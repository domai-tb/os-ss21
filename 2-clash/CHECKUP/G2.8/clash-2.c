// Korrekteur: Alex
// Punkte in dieser Datei: 0 von 10

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>    
#include <string.h>    
#include <stdbool.h>

#include ".clash/plist.h"
/*I----> +--------------------------------------------------------------------+
         | No such File or Directory -> Kompilierfehler (-3.0)                |
         +-------------------------------------------------------------------*/


#define MAX_PATH_LENGTH 100
#define MAX_LINE_LENGTH 1337
#define BUFFER_SIZE 128
#define TOKEN_BUFFER_SIZE 10
#define STATUS_BACKGROUND -42 // internal statuscode for not waiting

/* Get working directory:

    Parameters:
    -

    Locals:

    - buffer:           buffer for getting the working dir
    - working_dir:      working dir

    Return:

    - If Fail:    exit(EXIT_FAILURE)
    - If Succes:  char* working_dir

*/
static char* get_working_dir() 
{
    char buffer[MAX_PATH_LENGTH];  

    if(getcwd(buffer, sizeof(buffer)) == NULL) {
        perror("Can't read working directory.\n");
        exit(EXIT_FAILURE);
    }

    char* working_dir = NULL;
    working_dir = (char*) malloc(strlen(buffer) * sizeof(char));
    if(working_dir == NULL) {
        perror("Memory allocation goes wrong.\n");
        exit(EXIT_FAILURE);
    }
/*I----> +--------------------------------------------------------------------+
         | Das Arbeitsverzeichnis kann durchaus länger als 100 Byte sein.    |
         | Daher muss getcwd() in einer Schleife ausgeführt werden. (-3.0)   |
         +-------------------------------------------------------------------*/
    strcpy(working_dir, buffer);
    return working_dir;
}

/* Change Working directory:

    Parameters:
    
    - directory:        targeted directory

    Locals:
    -

    Return:

    - If Fail:          EXIT_FAILURE
    - If Success:       EXIT_SUCCESS

*/
static int change_directory(char* directory)
{
    if(directory == NULL) {
        perror("Syntax: cd <path>\n");
/*I----> +--------------------------------------------------------------------+
         | Die errno wurde in diesem Fall nicht gesetzt, daher sollte die     |
         | Fehlerbehandlung mit fprintf(stderr,"") statt perror()             |
         | erfolgen. (-0.5)                                                   |
         +-------------------------------------------------------------------*/
        return EXIT_FAILURE;

/*I----> +--------------------------------------------------------------------+
         | Blick auf die main(): wenn dort execute_command() aufgerufen wird  |
         | und dort dann change_directory() aufgerufen wird, erhält im       |
         | Fehlerfall "status" einen Wert ungleich Null. Wenn dieser Wert     |
         | dann zufällig == STATUS_BACKGROUND ist, wird insertElement()      |
         | aufgerufen.                                                        |
         +-------------------------------------------------------------------*/
    }
    else if(chdir(directory) != EXIT_SUCCESS) {
        perror("Unable to change directory.");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


/* Show all inputed commands:

    Parameters:
    
    - pid:              PID of Process
    - cmdline           Command of Process

    Locals:
    -

    Return:

    - If Fail:          -
    - If Success:       EXIT_SUCCESS

*/
static int print_job_info(pid_t pid, const char* cmdline)
{
    printf("[%d]: '%s'\n", pid, cmdline);
    return EXIT_SUCCESS;
}

/* Try to find a '&' at the end of user input:

    Parameters:

    - line:             input line (from user)  

    Locals:
    -

    Return:

    - If Fail:          -
    - If Success:       -

*/
static bool is_background(char* line)
{
    size_t length = strlen(line);
    return line[length-1] == '&';
}

/* Reading the command from user input:

    Parameters:
    
    - command_line:     user input

    Locals:

    - command_line:     buffer for getting the user input
    - index:            array index of buffer
    - c:                character of user input
    - buffer_size:      max. size of buffer

    Return:    

    - If Fail:    EXIT_FAILURE
    - If Succes:  char* command_line
    - If EOF:     exit programm

*/
static int read_command(char** command_line)
{
    int index = 0;
    int c;
    int buffer_size = BUFFER_SIZE;

    *command_line = (char*) malloc(sizeof(char) * buffer_size);
    if(*command_line == NULL) {
        perror("Memory allocation goes wrong.\n");
        exit(EXIT_FAILURE);
    }

    while(true) {
        // Read a character
        c = getchar();

        // Replace '\n' with a null character and return.
        if(c == '\n') {
            (*command_line)[index] = '\0';
            // Return failure, if line is too long
            if(index > MAX_LINE_LENGTH) {
                perror("Input line too long.\n");
/*I----> +--------------------------------------------------------------------+
         | Der Rest der überlangen Zeile muss noch weggelesen werden. (-1.0) |
         +-------------------------------------------------------------------*/
                return EXIT_FAILURE;
            }
            if(index == 0) {
                return EXIT_FAILURE;
            }
       
            return EXIT_SUCCESS;
        }
        // Exit at EOF
        if(c == EOF) {
            if(EOF == fflush(stdout)) {
                perror("fflush");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }
        // Ignore leading spaces
        if(index == 0) {
            if(c == ' ' || c == '\t') {
                continue;
            }
        }
        
        // Write character in buffer
        (*command_line)[index] = c;
        index++;
        
        // Realloc, if buffer size exceeded
        if(index >= buffer_size) {
            buffer_size += BUFFER_SIZE;
            *command_line = (char*) realloc(*command_line, buffer_size);
            if(command_line == NULL) {
                perror("Memory allocation goes wrong.\n");
                exit(EXIT_FAILURE);
            }
        }
        
    }
}

/* Parse parameters from input line:

    Parameters:

    - line:             input line (from user)
    - type:             if background process, true, else false

    Locals:

    - buffer_size:      size of buffer to write parameters
    - token:            single word from input line
    - delimit:          array of delimiters for strtok
    - type:             background process? -> '&' at last position
    - index:            parameter array index 

    Return:             
    
    - If Fail:          exit(EXIT_FAILURE)
    - If Succes:        char** parameters

*/
static char** get_parameters(char* line, bool type)
{
    int buffer_size = TOKEN_BUFFER_SIZE;
    int index = 0;
    char* token;
    char delimit[3] = " \t";
    
    char** parameters = (char**) malloc(buffer_size * sizeof(char*));
    if(parameters == NULL) {
        perror("Memory allocation goes wrong.\n");
        exit(EXIT_FAILURE);
    }

    // parse (user) input into parameter array
    token = strtok(line, delimit);
    while(token != NULL) {
        parameters[index] = token;
        index++;

        if(index >= buffer_size) {
            buffer_size += TOKEN_BUFFER_SIZE;
            parameters = realloc(parameters, buffer_size * sizeof(char*));
            if(parameters == NULL) {
                perror("Memory allocation goes wrong.\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, delimit);
    }

    
    // if background process, overwrite '&'
/*I----> +--------------------------------------------------------------------+
         | Aufgabenstellung spezifiziert nicht genau, ob letztes Zeichen oder |
         | letztes Token -> daher ok so.                                      |
         +-------------------------------------------------------------------*/
/*I----> +--------------------------------------------------------------------+
         | EIgentlich müsste nur das allerletzte Zeichen (was das & wäre)    |
         | mit \0 überschrieben werden.                                      |
         +-------------------------------------------------------------------*/
/*I----> +--------------------------------------------------------------------+
         | So kommt es z.B. bei Eingabe "&" zum SegFault.                     |
         +-------------------------------------------------------------------*/
    if(type) {
        parameters[index-1]= NULL ;
    }
    else {
        parameters[index] = NULL;
    }

    return parameters;
}

/* Execute inputed command:

    Parameters:

    - parameters:       parsed user input
    - type:             background process? => true
    - _pid:             PID of forked process

    Locals:

    - wait_status:      status of child process

    Return:             
    
    - If Fail:          EXIT_FAILURE
    - If Succes:        int wait_status

*/
static int execute_command(char** parameters, bool type, pid_t* _pid)
{
    int wait_status;

    if(strcmp(parameters[0], "cd") == 0) {
        return change_directory(parameters[1]);
    }
    else if(strcmp(parameters[0], "jobs") == 0) {
        walkList(print_job_info);
        return EXIT_SUCCESS;
    }

    *_pid = fork();
    if (*_pid == -1) {
        // Error forking
        perror("Unable to fork.\n");
        exit(EXIT_FAILURE);
    } 
    else if (*_pid == 0) {
        // Child process:   Execute command
        if (execvp(parameters[0], parameters) == -1) {
            perror("exec");
            return EXIT_FAILURE;
/*I----> +--------------------------------------------------------------------+
         | In diesem Fall sollte mit exit() beendet werden. (-0.5)            |
         +-------------------------------------------------------------------*/
        }
    } 
    else {
        // Parent process:  Wait for child, if foreground process
        if(type) {
            return STATUS_BACKGROUND;
        } 
        else {
            do {
                *_pid = waitpid(*_pid, &wait_status, WUNTRACED);
            } while (!WIFEXITED(wait_status) && !WIFSIGNALED(wait_status));
        }
    }

    return wait_status;
/*I----> +--------------------------------------------------------------------+
         | In wait_status ist mehr als nur eine Variable, daher kann in der   |
         | Main nicht einfach geprüft werden, ob status !=                   |
         | STATUS_BACKGROUND (-0.5)                                           |
         +-------------------------------------------------------------------*/
}

/* find, print and remove all Zombies in plist

    Parameters:
    -

    Locals:

    - wstatus:          status of process
    - w:                process id

    Return:             
    
    - If Fail:          EXIT_FAILURE
    - If Succes:        EXIT_SUCCESS

*/
static int cleanup_zombies() 
{
    int wstatus;
    int w = waitpid(-1, &wstatus, WNOHANG);
  
    
    while(w != 0 && w != -1) {
        char pid_cmd[MAX_LINE_LENGTH];
        if(WIFEXITED(wstatus)) {
            if(removeElement(w, pid_cmd, MAX_LINE_LENGTH) == -1) {
                perror("Unable to remove Element\n");
                return EXIT_FAILURE;
            }
/*I----> +--------------------------------------------------------------------+
         | wstatus ist nicht der Exitstatus, da sind noch mehr Werte drin     |
         | gespeichert. Du brauchst WEXITSTATUS(wstatus), siehe man           |
         | waitpid. (-1.0)                                                    |
         +-------------------------------------------------------------------*/
            printf("Exisstatus [ %s ] = %d\n", pid_cmd, wstatus);
        }    
        w = waitpid(-1, &wstatus, WNOHANG);
    }
/*I----> +--------------------------------------------------------------------+
         | Im Fall, dass w == -1 ist, muss errno == ECHILD überprüft werden |
         | (kein Fehler), sonst mit perror + exit abbrechen. (-0.5)           |
         +-------------------------------------------------------------------*/
    return EXIT_SUCCESS;
}

/* How to use: 

    - compile:            make -rR
    - run:                <path>/clash
    - type:                <name> <param 1> ... <param n>

    Locals:

    - commmand_line:    user input
    - _line:            copy of user input 
    - working_dir:      working directory
    - parameters:       array of single words in user input
    - status:           status of processes; abort programm at error 
    - job_type:         background process? Default: False => Forground process
    - pid:              PID of Backgroundprocess

*/
int main(int argc, char **argv)
{
    char* command_line;
    char* _line;
    char* working_dir;
    char** parameters;
    bool job_type = false;
    pid_t pid;
    int status;

    // input loop
    do {
        // print working directory
        working_dir = get_working_dir();
        printf("%s: ", working_dir);

/*I----> +--------------------------------------------------------------------+
         | Hier fehlt fflush(), damit die Ausgabe sicher aufs Terminal        |
         | geschrieben wurde. Stdout ist standardmäßig gepuffered           |
         | (meist zeilenweise, aber das ist laut POSIX nicht zwingend):       |
         | Erst mit einem \n oder fflush() wird die Zeile sicher              |
         | geschrieben werden (-0-5)                                          |
         +-------------------------------------------------------------------*/

        // read user input
        if(read_command(&command_line) == EXIT_FAILURE) {
            // skip if reading command fails
            continue;
        }

        // copy user input (because strtok)
        _line = (char*) malloc(strlen(command_line) * sizeof(char));
        if (_line == NULL) {
            perror("Memory allocation goes wrong.\n");
            return EXIT_FAILURE;
        }
        strcpy(_line, command_line);

        // serach for '&' at the end of user input
        job_type = is_background(_line);

        // get parameters from user input
        parameters = get_parameters(_line, job_type);

        /* execute command and return status code.
           status code -42 target the background process. */
        status = execute_command(parameters, job_type, &pid);
/*I----> +--------------------------------------------------------------------+
         | Im Fall, dass execvp einen Fehler hat, wird an status EXIT_FAILURE |
         | übergeben. -> status != STATUS_BACKGROUND So wird                 |
         | fälschlicherweise der Exitstatus ausgegeben, auch wenn exec       |
         | gefailt ist. (-0.5)                                                |
         +-------------------------------------------------------------------*/

/*I----> +--------------------------------------------------------------------+
         | Der status ist nicht der Exitstatus, da sind noch mehr Werte drin  |
         | gespeichert. Du bauchst noch WIFEXITED plus WEXITSTATUS; siehe man |
         | waitpid (-1.0)                                                     |
         +-------------------------------------------------------------------*/
        if(status != STATUS_BACKGROUND) {
            printf("Exisstatus [ %s ] = %d\n", command_line, status);
        }
        else {
            insertElement(pid, command_line);
/*I----> +--------------------------------------------------------------------+
         | Rückgabewert insertElement nicht beachtet. (s. plist.h) (-1.0)    |
         +-------------------------------------------------------------------*/
        }
        
        //cleanup zombies
        cleanup_zombies();

        // free allocated memory
        free(working_dir);
        free(command_line);
        free(_line);
        free(parameters);
        
    } while (true);

    return EXIT_SUCCESS;
}
/*P----> +--------------------------------------------------------------------+
         | Punktabzug in dieser Datei: 12.5 Punkte                            |
         +-------------------------------------------------------------------*/
