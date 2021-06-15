// Korrekteur : Alex
// Punkte     : 1.5 von 10
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// +3 in reason of string syntax
#define MAX_LINE_LENGTH 103

/*I----> +--------------------------------------------------------------------+
         | Fehlende Fehlerbehandlung bei allen zusätzlichen Ausgaben.(-1.0)  |
         +-------------------------------------------------------------------*/

/*I----> +--------------------------------------------------------------------+
         | static vergessen. (-0.5) Alle Schnittstellen, die nicht in der     |
         | Aufgabenstellung verlangt sind, sollen nach außen hin nicht       |
         | sichtbar sein. Wenn du hier ein static drin hast, erlaubst du es   |
         | anderen Modulen, mit denen dein Modul womöglich zusammen gelinkt  |
         | wird, ebenfalls Funktionen mit diesem namen zu verwenden. So       |
         | verhinderst du einen erfolgreichen Link-Vorgang.                   |
         +-------------------------------------------------------------------*/
// compare strings alpha-numerical 
int compare(const void* a, const void* b) 
{
    return strcmp(*(const char**)a, *(const char**)b);
}


int main(int argc, char **argv)
{
        char** lines = 0;
        int line_count = 0, line_length;
        
        if (argc < 1)
        {
            fprintf(stderr, "Program start failed.\n");
            return EXIT_FAILURE;
        }
        
/*I----> +--------------------------------------------------------------------+
         | Laut Aufgabenstellung sind keine weiteren Ausgaben                 |
         | gefordert. (-0.5)                                                  |
         +-------------------------------------------------------------------*/
        fprintf(stdout, "Wait for input... \n");
        
        if(fflush(stdout) == EOF)
        {
            fprintf(stderr, "Output clearing goes wrong.\n");
            return EXIT_FAILURE;
        }
        
        // Read Input Loop
        while(!feof(stdin))
        {
                line_length = 0;
                
                // allocate memory of char pointer
                lines = (char**) realloc(lines, (line_count + 1) * sizeof(*lines));
                if(lines == NULL) 
                {
                    fprintf(stderr, "Memory allocation goes wrong.\n");
                    return EXIT_FAILURE;
                }    
                
                // allocate memory of line
                lines[line_count] = (char*) calloc(MAX_LINE_LENGTH, sizeof(char));
                if(lines[line_count] == NULL) 
                {
                    fprintf(stderr, "Memory allocation goes wrong.\n");
                    return EXIT_FAILURE;
                }
                
                // read input from stin in lines[line_count]
                if(fgets(lines[line_count], MAX_LINE_LENGTH, stdin) == NULL && !feof(stdin))
                {
/*I----> +--------------------------------------------------------------------+
         | Fehlerüberprüfung mit ferror() (-0.5)                            |
         +-------------------------------------------------------------------*/
            fprintf(stderr, "Input reading failed.\n");
                    free(lines[line_count]);
                    return EXIT_FAILURE;
                }
                
                // check if line is to long or empty
                line_length = strlen(lines[line_count]);
                        
                if(line_length >= MAX_LINE_LENGTH-1)
                {
                    fprintf(stderr, "Line size is limited.\n");
                    free(lines[line_count]);
                    continue;
/*I----> +--------------------------------------------------------------------+
         | Ihr erkennt überlange Zeilen und "ignoriert" diese mit continue.  |
         | Allerdings immer nur die ersten 103 Zeichen jeder zu langen Zeile. |
         | Im nächsten Schleifenaufruf werden so die restlichen Zeichen      |
         | dieser Zeile als nächste Zeile interpretiert. (-2.0)              |
         +-------------------------------------------------------------------*/
                }
/*I----> +--------------------------------------------------------------------+
         | Der Fall, dass in der letzten Zeile 1 Zeichen ohne \n steht, wird  |
         | hiermit auch ignoriert. (-0.5)                                     |
         +-------------------------------------------------------------------*/
                if(line_length == 1)
                {
                    fprintf(stderr, "Ignoring empty line.\n");
                    free(lines[line_count]);
                    continue;
                }
                
/*I----> +--------------------------------------------------------------------+
         | Laut Aufgabenstellung sind die \n-Zeichen am Ende einer            |
         | Zeile nicht Teil des Wortes. Bei eurer Implementierung             |
         | werden diese mitsortiert, was zu einer falschen                    |
         | Sortierreihenfolge führt. (-1.0)                                  |
         +-------------------------------------------------------------------*/
                line_count++;
        }
        
        // sort input
        qsort(lines, line_count, sizeof(*lines), compare);

/*I----> +--------------------------------------------------------------------+
         | Keine zusätzlichen Ausgaben!                                      |
         +-------------------------------------------------------------------*/
        // print sorted input and free memory
        fprintf(stdout, "\n\nSorted input...\n\n");
        for(int i = 0; i < line_count; i++)
        {
/*I----> +--------------------------------------------------------------------+
         | Hier fehlt der Zeilenumbruch nach jedem Wort. (-0.5)               |
         +-------------------------------------------------------------------*/
            if(fprintf(stdout, "%s", lines[i]) < 0)
            {
                fprintf(stderr, "Internal Error in 'printf' function occured.\n");
                return EXIT_FAILURE;
            }
            free(lines[i]);
        }
        free(lines);

/*I----> +--------------------------------------------------------------------+
         | Hier müsste ein fflush() plus Fehlerbehandlung stehen, damit      |
         | sicher die Ausgabe geschrieben wurde. (-2.0)                       |
         +-------------------------------------------------------------------*/
                
        return EXIT_SUCCESS;
}
/*P----> +--------------------------------------------------------------------+
         | Punktabzug in dieser Datei: 8.5 Punkte                             |
         +-------------------------------------------------------------------*/
