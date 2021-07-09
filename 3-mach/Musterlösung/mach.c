#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <pthread.h>
#include <stdbool.h>
#include <string.h>

#include "queue.h"
#include "run.h"
#include "sem.h"

#define FLAG_RUNNING    1
#define FLAG_OUTPUT     2
#define FLAG_POISON     3

static SEM *sem_wait;
static SEM *sem_limit;

static void die(const char *s) {
    perror(s);
    exit(EXIT_FAILURE);
}

static int parse_positive_int_or_die(char *str) {
    errno = 0;
    char *endptr;
    

    long x = strtol(str, &endptr, 10);
    if (errno != 0) {
        die("invalid number");
    }
    // Non empty string was fully parsed
    if (str == endptr || *endptr != '\0') {
        fprintf(stderr, "invalid number\n");
        exit(EXIT_FAILURE);
    }
    if (x <= 0) {
        fprintf(stderr, "number not positive\n");
        exit(EXIT_FAILURE);
    }
    if (x > INT_MAX) {
        fprintf(stderr, "number too large\n");
        exit(EXIT_FAILURE);
    }
    return (int)x;
}

static void *thread_output(void *unused) {
    (void)unused;

    // Whileschleife, die die Ausgabe aller Arbeiterthreads ausgegeben werden soll
    while (true) {
        char *cmd, *out;
        int flags;
      
        if (queue_get(&cmd, &out, &flags)) {
            die("queue_get");
        }

        /* poison pill */
        if (flags == FLAG_POISON) {
            break;
        }
        
        if (flags == FLAG_RUNNING) {
            if (printf("Running `%s` ...\n", cmd) < 0) {
                die("printf");
            }
            
        } else {
            if (printf("Completed `%s`: \"%s\"\n", cmd, out) < 0) {
                die("printf");
            }
        }
        free(cmd);
        free(out);
    }
    return NULL;
}

static void *thread_work(void *arg) {
    
    char *cmd = arg;
    
    errno = pthread_detach(pthread_self());
    if (errno) {
        die("pthread_detach");
    }
    
    char *cmd2 = strdup(cmd);
    if (!cmd2) {
        die("strdup");
    }
    
    if (queue_put(cmd2, NULL, FLAG_RUNNING)) {
        die("queue_put");
    }

    char *out;
    run_cmd(cmd, &out);
   
    if (queue_put(cmd, out, FLAG_OUTPUT)) {
        die("queue_put");
    }

    V(sem_wait);
    V(sem_limit);
    return NULL;
}

int main(int argc, char **argv) {

    if (argc != 3) {
        fprintf(stderr, "usage: %s <threads> <mach file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int threads = parse_positive_int_or_die(argv[1]);

    if (queue_init()) {
        die("queue");
    }

    sem_wait = semCreate(0);
    if (!sem_wait) {
        die("semCreate");
    }
    
    sem_limit = semCreate(threads);
    if (!sem_limit) {
        die("semCreate");
    }

    FILE *fh = fopen(argv[2], "r");
    if (!fh) {
        die(argv[2]);
    }

    pthread_t tid_output;
    errno = pthread_create(&tid_output, NULL, thread_output, NULL); // liefert errno-Wert oder 0 zurück
    if (errno) {
        die("pthread_create");
    }

    // Zähler, wie viele Threads gestartet wurden
    size_t threads_started = 0;
    char buf[4096+1];
    
    while (fgets(buf, sizeof(buf), fh)) {
        // falls es eine Leerzeile ist, sind alle Befehle der aktuellen Gruppe eingelesen
        if (!strcmp(buf, "\n")) {
            while (threads_started > 0) {
                P(sem_wait);
                threads_started--;
            }
            continue;
        }

        strtok(buf, "\n"); // strtok entfernt das \n aus dem Buf
        char *x = strdup(buf);
        if (!x) {
            die("strdup");
        }

        P(sem_limit); // Diese Semaphore wird inkrementiert. Ein Thread arbeitet nun in einem kritischen Bereich bzw. es ist festgelegt, wie viele Threads parallel arbeiten dürfen.
        threads_started++;

        pthread_t tid;
        errno = pthread_create(&tid, NULL, thread_work, x);
        if (errno) {
            die("pthread_create");
        }
    }//end-while
    
    if (ferror(fh)) {
        die("fgets");
    }
    fclose(fh);

    while (threads_started > 0) {
        P(sem_wait); //
        threads_started--;
    }


    // Alle Ausgaben fertig gespeichert, wird so nun in der Warteschlange vermerkt.
    if (queue_put(NULL, NULL, FLAG_POISON)) {
        die("queue_put");
    }

    // Warten, bis der Ausgabethread fertig ist.
    errno = pthread_join(tid_output, NULL);
    if (errno) {
        die("pthread_join");
    }

    // optional
    if (fflush(stdout)) {
        die("fflush");
    }

    queue_deinit();
    semDestroy(sem_wait);
    semDestroy(sem_limit);

    return EXIT_SUCCESS;
    
}
