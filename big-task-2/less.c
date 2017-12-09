#include <stdlib.h>
#include <sys/stat.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <getopt.h>

static struct termios stored_settings;


int length_of_number(int n)
{
    int i = 0;
    if (n == 0)
    {
        return 1;
    }
    while (n != 0)
    {
        ++i;
        n /= 10;
    }
    return i;
}

void print_digit(int digit)
{
    int n = length_of_number(digit);
    int i = 0;
    wchar_t *str = (wchar_t *)malloc( (n * 10) * sizeof(wchar_t) );
    if (digit == 0)
    {
        fputwc(L'0',stdout);
        free(str);
        return;
    }
    while (digit != 0)
    {
        *( str + n - i - 1 ) = L'0' + (digit % 10);
        ++i;
        digit /= 10;
    }
    for (i = 0; i < n; ++i)
    {
        fputwc(*(str + i), stdout);
    }
    free(str);
    return;
}

void get()
{
    tcgetattr(0,&stored_settings);
}

void memory_pointer_check(void *ptr)
{
    if (ptr == NULL)
    {
        perror("Ошибка выделения памяти");
        exit(3);
    }
}


void reset_keypress()
{
    tcsetattr(0,TCSANOW,&stored_settings);
    return;
}

void file_check(FILE *file)
{
    if ((file == NULL) | (ferror(file) != 0))
    {
        perror("Ошибка при работе с файлом");
        reset_keypress();
        exit(-1);
    }
}

int length(wchar_t *string)
{
    int length = 0;
    while (*string != '\0' && *string != '\n')
    {
        ++length;
        ++string;
    }
    return length;
}

void counter(FILE *file,  int *str_count)
{
    wchar_t symbol;
    *str_count = 1;
    rewind(file);
    
    while ((symbol = fgetwc(file)) != WEOF)
    {
        if (ferror(file))
        {
            perror("Ошибка при работе с файлом");
            exit(1);
        }
        
        if((symbol == L'\n') || (symbol == L'\0'))
        {
            ++(*str_count);
        }
    }
}

void file_read(FILE *file, wchar_t ***ptr_to_strings_array, int *number_of_str)
{
    wchar_t symbol;
    int str_index = 0;
    int index = 0;
    int jndex = 0;
    int size_of_buffer = 1000;
    size_t step = 100;
    
    wchar_t **strings_array;
    strings_array = (wchar_t **) malloc( (*number_of_str) * (sizeof(wchar_t *)));
    memory_pointer_check(strings_array);
    rewind(file);
    strings_array[str_index] = (wchar_t *)malloc(size_of_buffer * sizeof(wchar_t));
    while ((symbol = fgetwc(file)) != WEOF)
    {
        file_check(file);
        
        if (symbol == L'\n' || symbol == L'\0')
        {
            strings_array[str_index][index] = (wchar_t)symbol;
            str_index++;
            index = 0;
            strings_array[str_index] = (wchar_t*)malloc(size_of_buffer * sizeof(wchar_t));
            memory_pointer_check(strings_array[str_index]);
        }
        else
        {
            if (symbol == L'\t')
            {
                if (index >= size_of_buffer - 4)
                {
                    size_of_buffer += step;
                    strings_array[str_index] = (wchar_t*)realloc (strings_array[index], size_of_buffer * sizeof(wchar_t));
                    memory_pointer_check(strings_array[str_index]);
                }
                for (jndex = 0; jndex < 4; ++jndex)
                {
                    strings_array[str_index][index] = L' ';
                    ++index;
                }
                continue;
            }
            strings_array[str_index][index] = symbol;
            if (++index >= size_of_buffer)
            {
                size_of_buffer += step;
                strings_array[str_index] = (wchar_t*)realloc (strings_array[index], size_of_buffer * sizeof(wchar_t));
                memory_pointer_check(strings_array[str_index]);
            }
        }
    }
    strings_array[str_index][index] = L'\n';
    str_index = str_index + 1;
    *number_of_str = str_index;
    *ptr_to_strings_array = strings_array;
}


int string_read(FILE *file, wchar_t **ptr_to_str, int *length)
{
    wchar_t symbol = 0;
    size_t size_of_buffer = 1000;
    size_t step = 100;
    size_t i = 0;
    wchar_t *s = *ptr_to_str;
    s = (wchar_t*) realloc (s, sizeof(wchar_t) * size_of_buffer);
    *length = 0;
    while (( (symbol = fgetwc(file)) != EOF ) &&(symbol != L'\0') && (symbol != L'\n'))
    {
        file_check(file);
        (s[i]) = symbol;
        ++(*length);
        if (++i >= size_of_buffer)
        {
            size_of_buffer += step;
            s = (wchar_t*) realloc (s, sizeof(wchar_t) * size_of_buffer);
            memory_pointer_check(s);
        }
    }
    s[i] = L'\n';
    ++(*length);
    *ptr_to_str = s;
    if (symbol == WEOF)
    {
        return -1;
    }
    return 0;
}

void mem_free(wchar_t **strings_array, int number_of_str)
{
    int i = 0;
    for (i = 0; i < number_of_str; ++i)
    {
        free(strings_array[i]);
    }
    free(strings_array);
}

void terminal_size(int *width, int *height)
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    
    *height = w.ws_row;
    *width = w.ws_col;
}

FILE *file_maker(char *name_of_file)
{
    FILE *file;
    file = fopen(name_of_file, "r");
    file_check(file);
    return file;
}

void print_str(wchar_t *string,  int begin, int s_length)
{
    int i;
    if (length(string) <= begin)
    {
        fputwc(L'\n', stdout);
        return;
    }

    if (*string == L'\n' || *string == L'\0')
    {
        fputwc(L'\n', stdout);
        return;
    }
    for (i = begin; (i <= s_length) && (string[i] != L'\0') && (string[i] != L'\n'); ++i)
    {
        if (length(string) == i)
        {
            fputwc(L'\n', stdout);
            return;
        }
        fputwc(string[i], stdout);
    }
    fputwc(L'\n', stdout);
}

void s_view(int begin_index, int end_index, int begin_str, int end_str,wchar_t **input)
{
    int i;
    for (i = begin_str; i <= end_str; ++i)
    {
        print_str(input[i], begin_index, end_index);
    }
}

void n_view(int begin_index, int end_index, int begin_str, int end_str, int number_of_str, wchar_t **input)
{
    int i;
    int j;
    int len_of_str_numbr = length_of_number(number_of_str);
    for (i = begin_str; i <= end_str; ++i)
    {
        if(begin_index != 0)
        {
            fputwc(L'<', stdout);
        }
        else
        {
            fputwc(L'|', stdout);
        }
        print_digit(i);
        if (length_of_number(i) < len_of_str_numbr)
        {
            for (j = 0; j < len_of_str_numbr - length_of_number(i); ++j)
            {
                fputwc(L' ', stdout);
            }
        }
        fputwc(L':', stdout);
        print_str(input[i], begin_index, end_index);
    }
}

int max_len_on_secion(wchar_t **input, int begin_str, int end_str)
{
    int max_len = 1;
    int i = 0;
    for (i = begin_str; i <= end_str; ++i)
    {
        if (length(input[i]) > max_len)
        {
            max_len = length(input[i]);
        }
    }
    return max_len;
    
}

void window_maker(int *begin_index, int *end_index, int *begin_str, int *end_str, int number_of_str, wchar_t **input, int n_flag)
{
    if (n_flag)
    {
        n_view(*begin_index, *end_index, *begin_str, *end_str,number_of_str,input);
    }
    else
    {
        s_view(*begin_index, *end_index, *begin_str, *end_str, input);
    }
    return;
}

void coordinator(int *begin_index, int *end_index, int *begin_str, int *end_str, int cmd, int number_of_str, wchar_t **input, int n_flag)
{
    int max_len;
    if (cmd == 5)
    {
        if (*end_str >= number_of_str)
        {
            *end_str = number_of_str - 1;
        }
        window_maker(begin_index, end_index, begin_str, end_str,number_of_str, input, n_flag);
        return;
    }
    
    if (cmd == 4)
    {
        if (*end_str == number_of_str - 1)
        {
            window_maker(begin_index, end_index, begin_str, end_str,number_of_str, input, n_flag);
            return;
        }
        else
        {
            
            ++(*begin_str);
            ++(*end_str);
            window_maker(begin_index, end_index, begin_str, end_str,number_of_str, input, n_flag);
            return;
        }
    }
    
    if (cmd == 3)
    {
        if (*begin_index == 0)
        {
            window_maker(begin_index, end_index, begin_str, end_str,number_of_str, input, n_flag);
            return;
        }
        else
        {
            --(*begin_index);
            --(*end_index);
            window_maker(begin_index, end_index, begin_str, end_str,number_of_str, input, n_flag);
            return;
        }
    }
    
    if (cmd == 2)
    {
        if (*begin_str == 0)
        {
            window_maker(begin_index, end_index, begin_str, end_str,number_of_str, input, n_flag);
            return;
        }
        else
        {
            --(*begin_str);
            --(*end_str);
            window_maker(begin_index, end_index, begin_str, end_str,number_of_str, input, n_flag);
            return;
        }
    }
    
    if (cmd == 1)
    {
        max_len = max_len_on_secion(input, *begin_str, *end_str);
        if (*end_index == max_len - 1)
        {
            window_maker(begin_index, end_index, begin_str, end_str,number_of_str, input, n_flag);
            return;
        }
        else
        {
            if (max_len < *end_index)
            {
                window_maker(begin_index, end_index, begin_str, end_str,number_of_str, input, n_flag);
                return;
            }
            ++(*end_index);
            ++(*begin_index);
            window_maker(begin_index, end_index, begin_str, end_str,number_of_str, input, n_flag);
            return;
        }
    }
}

int cmd(int input[4])
{
    /*  1 - вправо
     *  2 - вверх
     *  3 - влево
     *  4 - вниз
     */
    if (input[1] == 27 && input[2] == 91 && input[3] == 65)
    {
        return 2;
    }
    
    if (input[1] == 27 && input[2] == 91 && input[3] == 67)
    {
        
        return 1;
    }
    
    if (input[1] == 27 && input[2] == 91 && input[3] == 66)
    {
        return 4;
    }
    
    if (input[1] == 27 && input[2] == 91 && input[3] == 68)
    {
        return 3;
    }
    
    return 0;
}

void size_recalc(int *width_o, int *height_o, int *width, int *height, int *end_index, int *end_str)
{
    terminal_size(width_o, height_o);
    *end_index = (*width_o - *width) + *end_index;
    *end_str = (*height_o - *height) + *end_str;
    *height = *height_o;
    *width = *width_o;
    return;
}

int input_checker(int symbol)
{
    if (symbol == 27 || symbol == 91 || symbol == 65 || symbol == 66 || symbol == 67 || symbol == 68 || symbol == 'q')
    {
        return 1;
    }
    else 
    {
        return 0;
    }
}

int arrow_checker(int symbol, int index)
{
    if (symbol == 27 && (index != 1))
    {
        return 0;
    }

    if (symbol == 91 && (index != 2))
    {
        return 0;
    }

    if (((symbol == 65) || (symbol == 66) || (symbol == 67) || (symbol == 68)) && (index != 3))
    {
        return 0;
    }
    return 1;
}

void show_file(char *name_of_file, int n_flag)
{
    wchar_t **input = NULL;
    int number_of_str = 0;
    int symbol;
    int symbols[4];
    int i = 1;
    int width;
    int height;
    int begin_index = 0;
    int end_index;
    int begin_str = 0;
    int end_str;
    int width_o;
    int height_o;
    FILE *file = file_maker(name_of_file);
    counter(file, &number_of_str);
    file_read(file, &input, &number_of_str);
    terminal_size(&width, &height);
    if (n_flag)
    {
        end_index = width - 1 - length_of_number(number_of_str) - 2;
    }
    else
    {
        end_index = width - 1;
    }
    
    end_str = height - 2;
    coordinator(&begin_index, &end_index, &begin_str, &end_str, 5, number_of_str, input, n_flag);
    
    
    while((symbol = fgetc(stdin)) != EOF)
    {
        if (!input_checker(symbol))
        {
            continue;
        }

        if (symbol == 'q')
        {
            break;
        }
        
        if (! arrow_checker(symbol, i))
        {
            continue;
        }
        else
        {
            symbols[i] = symbol;
            if (i % 3 == 0)
            {
                terminal_size(&width_o, &height_o);
                size_recalc(&width_o, &height_o, &width, &height, &end_index, &end_str);
                symbols[i] = symbol;
                coordinator(&begin_index, &end_index, &begin_str, &end_str, cmd(symbols), number_of_str, input, n_flag);
                i = 1;
                continue;
            }
            ++i;
            continue;
        }
    }
    mem_free(input, number_of_str);
    fclose(file);
    return;
}


void main_viewer(char *name_of_file, int n_flag)
{
    struct termios old_attributes,new_attributes;
    
    get();
    if(!isatty(0))
    {
        printf("Вы перенаправили ввод не из терминала.\n");
        printf("В таких условиях работа программы беcсмысленна.\n");
        exit(1);
    }
    
    tcgetattr(0,&old_attributes);
    memcpy(&new_attributes,&old_attributes, sizeof(struct termios));
    new_attributes.c_lflag &= ~ECHO;
    new_attributes.c_lflag &= ~ICANON;
    new_attributes.c_cc[VMIN] = 1;
    tcsetattr(0,TCSANOW, &new_attributes);
    
    show_file(name_of_file, n_flag);
    tcsetattr(0,TCSANOW, &old_attributes);
    return;
}

int main(int argc, char **argv)
{
    char c;
    char *name_of_file = NULL;
    int n_flag = 0;
    setlocale(LC_ALL, "");
    {
        while ((c = getopt (argc, argv, "nhf:")) != -1)
            switch (c)
        {
            case 'f':
                name_of_file = optarg;
                break;
            case 'h':
                printf("\nПрограмма показывает файл.\n Чтобы указать файл используйте аргумент флага f.\n Флаг n включает особый режим.\n");
                return 0;
                break;
            case 'n':
                n_flag = 1;
                break;
            case '?':
                perror("Unkown options!");
                return 1;
        }
    }
    if (name_of_file == NULL)
    {
        exit(0);
    }
    main_viewer(name_of_file, n_flag);
    return 0;
}
