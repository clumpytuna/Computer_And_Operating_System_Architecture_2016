#ifndef SAFE_READ_H
#define SAFE_READ_H

#include <stdio.h>
#include <stdlib.h>

/* returns char* if everything is OK and a NULL pointer if an error occured

   error equals to: 0 if everything is OK
	 	    1 if file is NULL
	 	    2 if ferror has occured
                    3 if cursor is already at eof
*/

char* safe_get_string(FILE* file, int* error)
{
    char ch;
    char* string = NULL;
    int size;

    if (file == NULL || ferror(file) || feof(file))
    {
	if (file == NULL)
	{
	    *error = 1;
	}
	else if (ferror(file))
	{
	    *error = 2;
	}
	else if (feof(file))
	{
	    *error = 3;
	}

        return NULL;
    }

    string = (char*) malloc(sizeof(char) * 1);

    size = 0;
    ch = 'p'; /* random symbol, will be override */

    while (1)
    {
	ch = fgetc(file);
	if (feof(file) || ch == '\n')
	{
	    string[size] = '\0';
            break;   
	}

	size++;	
        string = (char*) realloc(string, sizeof(char) * (size + 1));

        string[size - 1] = ch;
    }
    
    *error = 0;
    return string;
}

#endif /* SAFE_READ_H */
