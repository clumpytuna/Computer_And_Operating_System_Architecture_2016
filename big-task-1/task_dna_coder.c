#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
size_t SIZE_OF_BUFFER = 1000000;
/*
 Кодировки ошибок:
 1 - ошибка с  файлом открытым на чтение.
 2 - ошибка с файлом открытым на  запись.
 3 - ошибка с памятью.
 */
char symbol(int id)
{
    switch (id)
    {
        case 1:
            return 'a';
            break;
        case 2:
            return 't';
            break;
        case 3:
            return 'g';
            break;
        case 4:
            return 'c';
            break;
        case 5:
            return 'u';
            break;
        case 6:
            return 'e';
            break;
    }
    printf("ERROR IN DECODING");
    exit(1);
}

void bit_set_1(short int *var, int bit)
{
    *var |= 1 << (sizeof(short int)*8 - bit - 1);
}
void bit_set_0(short int *var, int bit)
{
    *var &= ~ (1 << (sizeof(short int)*8 - bit - 1));
}

int bit_look(short int *var, int bit)
{
    return ((*var) >> (bit))& 1;
}

void check_size(int ptr, short int *begin_ptr, size_t SIZE_OF_BUFFER)
{
    if ((begin_ptr + ptr) >= (begin_ptr + SIZE_OF_BUFFER - 1))
    {
        printf("MEMORY ERROR\n");
        exit(3);
    }
}

void check_bit(int *bit,int *ptr, short int *begin_ptr, size_t SIZE_OF_BUFFER)
{
    if (*bit == sizeof(short int) * 8 - 1)
    {
        (*ptr)++;
        check_size(*ptr, begin_ptr, SIZE_OF_BUFFER);
        *bit = 0;
    }
}

void write_a(int *bit, int *j, short int *res, size_t SIZE_OF_BUFFER)
{
    check_bit(bit, j, res, SIZE_OF_BUFFER);
    bit_set_0(res + (*j),*bit);
    (*bit)++;
    
    check_bit(bit, j, res, SIZE_OF_BUFFER);
    bit_set_0(res + (*j),*bit);
    (*bit)++;
    
    check_bit(bit, j, res, SIZE_OF_BUFFER);
    bit_set_1(res + (*j),*bit);
    (*bit)++;
}

void write_t(int *bit, int *j, short int *res, size_t SIZE_OF_BUFFER)
{
    check_bit(bit, j, res, SIZE_OF_BUFFER);
    bit_set_0(res + (*j),*bit);
    (*bit)++;
    
    check_bit(bit, j, res, SIZE_OF_BUFFER);
    bit_set_1(res + (*j),*bit);
    (*bit)++;
    
    check_bit(bit, j, res, SIZE_OF_BUFFER);
    bit_set_0(res + (*j),*bit);
    (*bit)++;
}

void write_g(int *bit, int *j, short int *res, size_t SIZE_OF_BUFFER)
{
    check_bit(bit, j, res, SIZE_OF_BUFFER);
    bit_set_0(res + (*j),*bit);
    (*bit)++;
    
    check_bit(bit, j, res, SIZE_OF_BUFFER);
    bit_set_1(res + (*j),*bit);
    (*bit)++;
    
    check_bit(bit, j, res, SIZE_OF_BUFFER);
    bit_set_1(res + (*j),*bit);
    (*bit)++;
}

void write_c(int *bit, int *j, short int *res, size_t SIZE_OF_BUFFER)
{
    check_bit(bit, j, res, SIZE_OF_BUFFER);
    bit_set_1(res + (*j),*bit);
    (*bit)++;
    
    check_bit(bit, j, res, SIZE_OF_BUFFER);
    bit_set_0(res + (*j),*bit);
    (*bit)++;
    
    check_bit(bit, j, res, SIZE_OF_BUFFER);
    bit_set_0(res + (*j),*bit);
    (*bit)++;
}

void write_u(int *bit, int *j, short int *res, size_t SIZE_OF_BUFFER)
{
    check_bit(bit, j, res, SIZE_OF_BUFFER);
    bit_set_1(res + (*j),*bit);
    (*bit)++;
    
    check_bit(bit, j, res, SIZE_OF_BUFFER);
    bit_set_0(res + (*j),*bit);
    (*bit)++;
    
    check_bit(bit, j, res, SIZE_OF_BUFFER);
    bit_set_1(res + (*j),*bit);
    (*bit)++;
}

void write_end(int *bit, int *j, short int *res, size_t SIZE_OF_BUFFER)
{
    check_bit(bit, j, res, SIZE_OF_BUFFER);
    bit_set_1(res + (*j),*bit);
    (*bit)++;
    
    check_bit(bit, j, res, SIZE_OF_BUFFER);
    bit_set_1(res + (*j),*bit);
    (*bit)++;
    
    check_bit(bit, j, res, SIZE_OF_BUFFER);
    bit_set_0(res + (*j),*bit);
    (*bit)++;
}

int pack(const char *s, short int *res, size_t SIZE_OF_BUFFER)
{
    int j = 0;
    const char *i = s;
    int bit = 0;
    
    while(*i != '\0')
    {
        check_size(j, res, SIZE_OF_BUFFER);
        
        if ((*i == 'a') | (*i == 'A') )
        {
            write_a(&bit, &j, res, SIZE_OF_BUFFER);
            ++i;
            continue;
        }
        else if ((*i == 't') | (*i == 'T') )
        {
            write_t(&bit, &j, res, SIZE_OF_BUFFER);
            ++i;
            continue;
        }
        else if ((*i == 'g') | (*i == 'G'))
        {
            write_g(&bit, &j, res, SIZE_OF_BUFFER);
            ++i;
            continue;
        }
        else if ((*i == 'c') | (*i == 'C'))
        {
            write_c(&bit, &j, res, SIZE_OF_BUFFER);
            ++i;
            continue;
        }
        else if ((*i == 'u') | (*i == 'U'))
        {
            write_u(&bit, &j, res, SIZE_OF_BUFFER);
            ++i;
            continue;
        }
        ++i;
        continue;
    }
    write_end(&bit, &j, res, SIZE_OF_BUFFER);
    return 0;
}

void do_array(short int *res, size_t size)
{
    int i = 0;
    for (i = 0; i < size; ++i)
    {
        *(res + i) =(short int) NULL;
        
    }
}

void do_string(char *string, size_t size)
{
    int i = 0;
    for  (i = 0; i < size; ++i)
    {
        *(string + i) =(char)NULL;
        
    }
}

void write_to_file(short int *c, FILE *file)
{
    if (fwrite(c, sizeof(short int), 1, file) == EOF)
    {
        printf("ERROR WITH WRITING");
        exit(2);
    }
}


void write_to_file_printf(char *begin, FILE *file)
{
    int index = 0;
    while ((*(begin +index) != '\0'))
    {
        fprintf(file, "%c", *(begin + index));
        ++index;
    }
}

char *safe_read(FILE *file, char *s)
{
    int symbol;
    size_t size_of_buffer = 1000;
    size_t step = 100;
    size_t i = 0;
    free(s);
    s = (char *) malloc (sizeof(char) * size_of_buffer);
    
    while (( (symbol = fgetc(file)) != EOF ) &&(symbol != '\0') && (symbol != '\n'))
    {
        if (ferror(file))
        {
            perror("Ошибка при работе с файлом");
            exit(1);
        }
        s[i] = symbol;
        if (++i >= size_of_buffer)
        {
            size_of_buffer += step;
            s = (char*) realloc (s, sizeof(char) * size_of_buffer);
            if(s == NULL)
            {
                fprintf(stderr, "Ошибка выделения памяти");
                exit(1);
            }
        }
    }
    s[i] = '\0';
    return s;
}


char *safe_read_for_rna(FILE *file, int* code)
{
    int symbol;
    int mode = 0;
    size_t size_of_buffer = 1000;
    size_t step = 100;
    size_t i = 0;
    char *s = (char *) malloc (sizeof(char) * size_of_buffer);
    
    while (( (symbol = fgetc(file)) != EOF ))
    {
        if (ferror(file))
        {
            perror("Ошибка при работе с файлом");
            exit(1);
        }
        if ((symbol == '>')&&(i == 0)) {
            mode = '>';
        }
        if ((mode == '>')&&((symbol == '\0') | (symbol == '\n'))) break;
        if ((mode == 0) &&(symbol == '>')) break;
        if ((symbol == '\0') | (symbol == '\n'))
        {
            continue;
        }
        s[i] = symbol;
        if (++i >= size_of_buffer)
        {
            size_of_buffer += step;
            s = (char*) realloc (s, sizeof(char) * size_of_buffer);
            if(s == NULL)
            {
                fprintf(stderr, "Ошибка выделения памяти");
                exit(1);
            }
        }
    }
    s[i] = '\0';
    if (symbol == EOF){
        *code = 1;
    }
    if (symbol == '>'){
        fseek(file, -sizeof(char),SEEK_CUR);
    }
    return s;
}


void packer(FILE *input, FILE *output)
{
    char *buffer = NULL;
    short int *res = (short int*) malloc(SIZE_OF_BUFFER * sizeof(short int));
    int f_end_of_file = 0;
    int index = 0;
    short int end = SHRT_MAX - 1;
    
    if (res == NULL)
    {
        printf("ERROR 2 MEMORY\n");
        exit (3);
    }
    
    do_array(res, SIZE_OF_BUFFER);
    while (!f_end_of_file)
    {
        buffer = safe_read_for_rna(input, &f_end_of_file);
        if (*buffer == '>')
        {
            write_to_file_printf(buffer, output);
            fprintf(output, "%c", '\0');
        } else
        {
            pack(buffer, res, SIZE_OF_BUFFER);
            index = 0;
            while (*(res +index) != (short int)NULL)
            {
                write_to_file ((res + index), output);
                index++;
            }
            write_to_file(&end, output);
            do_array(res, SIZE_OF_BUFFER);
        }
        free(buffer);
    }
    free(res);
    fclose(input);
    fclose(output);
    
}

char* decode(short int *word, char *string)
{
    int i = 15;
    int j = 0;
    int pow_2 = 4;
    int sum = 0;
    for (i = 15; i >= 0; --i)
    {
        if ((i % 3 == 0)&&(i != 15))
        {
            if (symbol(sum) == 'e') break;
            string[j] = symbol(sum);
            pow_2 = 4;
            sum = 0;
            j += 1;
        }
        sum += pow_2 * (((*word) >> (i))& 1);
        pow_2 /= 2;
    }
    
    return string;
}

int unpacker(FILE *input, FILE *output)
{
    char *input_str = (char*)malloc(sizeof(char) * 100);
    char *decoded_str = (char*)malloc(sizeof(char) * 6);
    int i = 0;
    short int *symbol = (short int *)malloc(sizeof(short int));
    *symbol = 0;
    if (decoded_str == NULL)
    {
        printf("ERROR 2 MEMORY\n");
        exit (3);
    }
    if (input_str == NULL)
    {
        printf("ERROR 2 MEMORY\n");
        exit (3);
    }
    while (1)
    {
        i = 0;
        input_str = safe_read(input, input_str);
        write_to_file_printf(input_str, output);
        fprintf(output, "\n");
        if ((*input_str) != '>') break;
        fread(symbol, sizeof(short int), 1, input);
        
        if (ferror(input))
        {
            perror("Ошибка при работе с файлом");
            exit(1);
        }
        
        while ((*symbol != SHRT_MAX - 1))
        {
            if ((i % 2 == 0) && (i != 0)) fprintf(output, " ");
            if ( (i % 6 == 0) && (i != 0)) fprintf(output, "\n");
            
            do_string(decoded_str, 6);
            fprintf(output, "%s", decode(symbol, decoded_str));
            fread(symbol, sizeof(short int), 1, input);
            if (ferror(input))
            {
                perror("Ошибка при работе с файлом");
                exit(1);
            }
            ++i;
        }
        fprintf(output,"\n");
    }
    
    free(input_str);
    free(decoded_str);
    free(symbol);
    
    return 0;
}

int main(int argc, char **argv)
{
    char c;
    char *output_str = NULL;
    char *mode = NULL;
    char *input_str = NULL;
    
    
    {
        while ((c = getopt (argc, argv, "o:i:hm:")) != -1)
            switch (c)
        {
            case 'm':
                mode = optarg;
                break;
            case 'i':
                input_str = optarg;
                break;
            case 'o':
                output_str = optarg;
                break;
            case 'h':
                printf("Программа на вход принимает строго 3 аргумента :\n 1) -m: направление перекодирования (направление перекодирования задаётся словами «pack», «unpack»).\n 2) -i: файл источник данных.\n 3) -o: файл  приёмник данных.\n(встретив -h, программа выводит это сообщение и завершает работу)\n");
                return 0;
                break;
            case '?':
                perror("Unkown options!");
                return 1;
        }
    }
    if (argc < 4)
    {
        perror("Неверно указаны параментры");
        exit(-1);
    }
    
    
    if (strcmp(mode,"unpack") == 0)
    {
        FILE *input = fopen(input_str,"r");
        FILE *output = fopen(output_str,"w");
        
        if (input == NULL)
        {
            printf("ERROR WITH INPUT FILE\n");
            exit(1);
        }
        
        
        if (output == NULL)
        {
            printf("ERROR WITH OUTPUT FILE\n");
            exit(2);
        }
        
        unpacker(input, output);
        fclose(input);
        fclose(output);
        
    } else if (strcmp(mode, "pack") == 0)
    {
        FILE *input = fopen(input_str,"r");
        FILE *output = fopen(output_str,"w");
        
        if (input == NULL)
        {
            printf("ERROR WITH INPUT FILE\n");
            exit(1);
        }
        
        
        if (output == NULL)
        {
            printf("ERROR WITH OUTPUT FILE\n");
            exit(2);
        }
        
        packer(input, output);
        fclose(input);
        fclose(output);
    }
    return 0;
}

