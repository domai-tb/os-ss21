#include "queue.h"
#include <errno.h>
#include <stdlib.h>

#include "sem.h"

static SEM *sem_lock;
static SEM *sem_notify;

struct elem {
    struct elem *next;  // Verkettungszeiger
    char *cmd;          // Speicher für aktuellen Befehl
    char *out;          // Speicher für Ausgabe
    int flags;          // Optionale Flags
};
static struct elem *queue; // Listenkopf

int queue_init(void) {

    // Ein Thread gleichzeitig darf auf der Queue arbeiten.
    sem_lock = semCreate(1);
    if (!sem_lock) {
        return -1;
    }
    
    // Hilfs-Semaphor, Elemente können erst aus der Queue entfernt werden, sobald welche hinzugefügt wurden.
    sem_notify = semCreate(0);
    if (!sem_notify) {
       
        int old_errno = errno;
        semDestroy(sem_lock);
        errno = old_errno;
        return -1;
    }
    return 0;
}

void queue_deinit(void) {

    semDestroy(sem_lock);
    semDestroy(sem_notify);

}

// Diese Funktion fügt ein Element in die Warteschlange ein.
// Ein Warteschlangenelement entspricht einer Ausgabe.
int queue_put(char *cmd, char *out, int flags) {

    struct elem *x = malloc(sizeof(*x));
    if (!x) {
        return -1;
    }
    x->next = NULL;
    x->cmd = cmd;
    x->out = out;
    x->flags = flags;

    // hier beginnt ein kritischer Bereich
    P(sem_lock);
    if (queue == NULL) {
        queue = x;
    } else {
        struct elem *y = queue;
        while (y->next != NULL) {
            y = y->next;
        }
        y->next = x;
    }
    V(sem_lock);    // Kritischer Bereich wird freigegeben. Thread verlässt diesen Bereich.
    V(sem_notify);  // In queue_get() kann erst ein Element entfernt werden, sobald ein Element hinzugefügt wurde. P(sem_notify) wartet, bis V(sem_notify) aufgerufen wurde.

    return 0;
}

// Diese Funktion holt ein Element (das älteste = Kopf) aus der Queue und schreibt die Werte in die übergebenen Pointer.
int queue_get(char **cmd, char **out, int *flags) {

    P(sem_notify);  // siehe queue_put()
    P(sem_lock);    // Kritischer Bereich wird betreten, sem_lock wird nun dekrementiert und blockiert.
    // Dies ist ein kritischer Bereich, da nicht mehrere Threads gleichzeitig Elemente aus der Queue löschen sollten.
    struct elem *x = queue;
    queue = x->next;
    V(sem_lock);
    // Ende kritischer Bereich. sem_lock wird wieder erhöht und hat Platz für nächsten Thread.

    *cmd = x->cmd;
    *flags = x->flags;
    free(x);

    return 0;
}
