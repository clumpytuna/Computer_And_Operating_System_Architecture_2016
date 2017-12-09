#ifndef PRINT_ENDING_H
#define PRINT_ENDING_H

#include "safe_read.h"
#include <stdlib.h>
#include <stdio.h>

#define SIZE 30

void print_text_from(char* filename)
{
	FILE* file;
	char** text;
	int i, error;

	if (filename == NULL)
	{
		printf("Wrong name of file\n");
		return;
	}

	file = fopen(filename, "r");
	
	if (file == NULL)
	{
		printf("File is not opened\n");
		return;
	}


	text = (char**) malloc(SIZE * sizeof(char*));

	for (i = 0; i < SIZE; ++i)
	{
		text[i] = safe_get_string(file, &error);
	}

	for (i = 0; i < SIZE; ++i)
	{
		printf("%s\n", text[i]);
	}

	for (i = 0; i < SIZE; ++i)
	{
		free(text[i]);
	}

	free(text);
	fclose(file);
}


#endif /* PRINT_ENDING_H */
