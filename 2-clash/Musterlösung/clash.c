#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>

#include "plist.h"

#define MAX_LINE 1337

static void die(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

static void print_prompt(void)
{
    size_t len = 1024;
    char *cwd = NULL;

    while(true)
    {
        cwd = realloc(cwd, len);
        if(cwd == NULL)
            die("realloc");

        if(getcwd(cwd, len) != NULL)
            break;
        if(errno == ERANGE)
            len *= 2;
        else
            die("getcwd");
    }

    printf("%s: ", cwd);
    fflush(stdout);
    free(cwd);
}

static void collect_zombies(void)
{
    pid_t pid;
    int status;

    while ((pid = waitpid(-1, &status, WNOHANG)) != 0)
    {
        if (pid < 0)
        {
            if (errno == ECHILD)
                break;
            die("waitpid");
        }
        char buf[MAX_LINE+1];
        if (removeElement(pid, buf, sizeof(buf)) < 0)
            continue;
        if (WIFEXITED(status))
            printf("Exitstatus: [%s] = %d\n", buf, WEXITSTATUS(status));
    }
    
}

static int print_jobs(pid_t pid, const char *buf)
{
    printf("%d %s\n", pid, buf);
    return 0;
}

int main(void) 
{
    while(true)
    {
        collect_zombies();
        print_prompt();

        char buf[MAX_LINE+1];
        if (fgets(buf, sizeof(buf), stdin) == NULL)
        {
            if(feof(stdin))
                break;
            die("fgets");
        }

        if (strlen(buf) == MAX_LINE && buf[MAX_LINE-1] != '\n')
        {
            fprintf(stderr, "Input too long.\n");

            int c;
            while ((c = fgetc(stdin)) != EOF && c != '\n');
            continue;
        }
        
        if (strlen(buf) < 2)
            continue;

        buf[strlen(buf)-1] = '\0';

        bool background = false;
        if (buf[strlen(buf)-1] == '&')
        {
            buf[strlen(buf)-1] = '\0';
            background = true;
        }

        char cpy[sizeof(buf)];
        strcpy(cpy, buf);

        int i = 0;
        char *argv[MAX_LINE/2+1 + 1];

        argv[i++] = strtok(buf, " \t");
        while ((argv[i++] = strtok(NULL, " \t")) != NULL)
            continue;
        
        if (argv[0] == NULL)
            continue;

        if (strcmp(argv[0], "cd") == 0)
        {
            if (argv[1] == NULL || argv[2] != NULL)
                fprintf(stderr, "Usage: cd <path/to/directory>\n");
            if (chdir(argv[1]))
                perror("chdir");
        }

        if (strcmp(argv[0], "jobs") == 0)
        {
            if (argv[1] != NULL)
                fprintf(stderr, "Usage: jobs");
            else 
                walkList(print_jobs);
        }

        pid_t pid = fork();
        if (pid < 0)
            die("fork");
        else if (pid == 0) 
        {
            execvp(argv[0], argv);
            die("exec");
        }

        if (background)
        {
            if (insertElement(pid, cpy) == -2)
            {
                fprintf(stderr, "out of memory");
                exit(EXIT_FAILURE);
            }
        } else {
            int status;
            if (waitpid(pid, &status, 0) < 0)
                die("waitpid");

            if (WIFEXITED(status))
                printf("Exitstatus: [%s] = %d\n", buf, WEXITSTATUS(status));
        }
    }
}