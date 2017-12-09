#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <locale.h>
#include <time.h>
#include <getopt.h>
#include <pthread.h>
#include <stdlib.h>
#include "print_ending.h"

#define BUF_SIZE 256
#define DIMENSION 30

int CMD = 0;
int MORATORIUM;
char MAP[12][12];
static struct termios stored_settings;
int WORK = 1;
pthread_mutex_t worker;
int WINNER = 0;

struct threads_info
{
    int sock;
    char **map;
    pthread_mutex_t mutex;
};
void get()
{
    tcgetattr(0, &stored_settings);
}

void reset_keypress()
{
    tcsetattr(0, TCSANOW, &stored_settings);
    return;
}

void make_non_canonical()
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

}


int input_checker(int symbol)
{
    if (symbol == 27 || symbol == 91 || symbol == 65 || symbol == 66 || symbol == 67 || symbol == 68 || symbol == 'q' || symbol == 'b' || symbol == 'm' || symbol == 'e')
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


void command_parser(int cmd, int sock)
{
    
    if (cmd == 1)
    {
        printf("Ввели вправо");
        write(sock, &cmd, sizeof(int));
        read(sock, &cmd, sizeof(int));
        printf("%d\n", cmd);
        
    }
    
    if (cmd == 2)
    {
        printf("Ввели вверх");
        write(sock, &cmd, sizeof(int));
        read(sock, &cmd, sizeof(int));
        printf("%d\n", cmd);

    }
    
    if (cmd == 3)
    {
        printf("Ввели влево");
        write(sock, &cmd, sizeof(int));
        read(sock, &cmd, sizeof(int));
        printf("%d\n", cmd);

    }
    
    if (cmd == 4)
    {
        printf("Ввели вниз");
        write(sock, &cmd, sizeof(int));
        read(sock, &cmd, sizeof(int));
        printf("%d\n", cmd);
    }
    
    if (cmd == 4)
    {
        printf("Ввели вниз");
        write(sock, &cmd, sizeof(int));
        read(sock, &cmd, sizeof(int));
        printf("%d\n", cmd);

    }
}


void print_map(char **map, int my_health)
{
    int i;
    int j;
    for (i = 0; i < DIMENSION; ++i)
    {
        printf("\n");
        for (j = 0; j < DIMENSION; ++j)
        {
            printf("%c", map[i][j]);
        }
    }
    printf("My health is: %d\n", my_health);
}

void* threads_write(void *arg)
{
    /*Микросекунда = 10 ^ (-6)
     */
    int i = 0;
    int j = 0;
    int end = -1;
    struct threads_info *info = arg;
    int my_health = 0;
    while (1)
    {
        if (!WORK)
        {
            break;
        }
        usleep(100000);
        if (!WORK)
        {
            break;
        }
        write(info->sock, &CMD, sizeof(int));
        for (i = 0; i < DIMENSION; ++i)
        {
            for (j = 0; j < DIMENSION; ++j)
            {
                read(info->sock, &info->map[i][j], sizeof(char));
            }
        }
        
        read(info->sock, &my_health, sizeof(int));
        if (my_health <= 0)
        {
            
            pthread_mutex_lock(&worker);
            WORK = 0;
            WINNER = -1;
            pthread_mutex_unlock(&worker);
            
            if (my_health == -1000)
            {
                WINNER = 1;
                
                printf("Игра завершена, нажмите любую клавишу, чтобы узнать результат\n");
                return (void *)0;
            }
            write(info->sock, &end, sizeof(int));
            
            printf("Игра завершена, нажмите любую клавишу, чтобы узнать результат\n");
            return (void *)0;
        }
        
        print_map(info->map, my_health);
        pthread_mutex_lock(&(info->mutex));
        CMD = 0;
        pthread_mutex_unlock(&(info->mutex));
    }
    
    return (void *)0;
}

void *threads_read(void *arg)
{
    int symbol;
    int symbols[4];
    int i = 1;
    int status = -1;
    struct threads_info *info = arg;
    
    while((symbol = fgetc(stdin)) != EOF)
    {
        
        pthread_mutex_lock(&worker);
        
        if (WORK == 0)
        {
            reset_keypress();
            return (void *)0;
        }
        pthread_mutex_unlock(&worker);
        
        if (!input_checker(symbol))
        {
            continue;
        }
        
        if (symbol == 'q')
        {
            pthread_mutex_lock(&(info->mutex));
            pthread_mutex_lock(&worker);
            CMD = -1;
            WORK = 0;
            write(info->sock, &status, sizeof(int));
            pthread_mutex_unlock(&worker);
            pthread_mutex_unlock(&(info->mutex));
            reset_keypress();
            break;
        }
        
        if (symbol == 'b')
        {
            pthread_mutex_lock(&(info->mutex));
            CMD = 5;
            pthread_mutex_unlock(&(info->mutex));
            continue;
        }
        
        if (symbol == 'e')
        {
            pthread_mutex_lock(&(info->mutex));
            CMD = 6;
            pthread_mutex_unlock(&(info->mutex));
            continue;
        }
        
        if (symbol == 'm')
        {
            pthread_mutex_lock(&(info->mutex));
            CMD = 7;
            pthread_mutex_unlock(&(info->mutex));
            continue;
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
                symbols[i] = symbol;
                pthread_mutex_lock(&(info->mutex));
                CMD = cmd(symbols);
                pthread_mutex_unlock(&(info->mutex));
                continue;
            }
            ++i;
            continue;
        }
    }
    

    return (void *)0;
}

int main(int argc, char ** argv)
{
    int result;
    int i;
    int sock, port;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char name[61];
    char message_from_server[60];
    pthread_t threads[2];
    struct threads_info info;
    pthread_mutex_t mutex;

    pthread_mutex_init(&mutex, 0);
    memset(name, 0, 60);
    memset(message_from_server, 0, 60);
    
    if (argc < 3)
    {
        fprintf(stderr,"usage: %s <hostname> <port_number>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    port = atoi(argv[2]);
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        printf("socket() failed: %d", errno);
        return EXIT_FAILURE;
    }
    
    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        printf("Host not found\n");
        return EXIT_FAILURE;
    }
    
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    serv_addr.sin_port = htons(port);
    
    if (connect(sock, (struct sockaddr *)&(serv_addr), sizeof(serv_addr)) < 0)
    {
        printf("connect() failed: %d", errno);
        return EXIT_FAILURE;
    }
    
    read(sock, message_from_server, sizeof(char) * 60);
    print_text_from("doom.txt");
    printf("%s\n", message_from_server);
    
    scanf("%s", name);
    write(sock, name, sizeof(char) * (strlen(name) + 1));
    
    memset(message_from_server, 0, sizeof(char) * 60);
    read(sock, message_from_server, sizeof(char) * 60);
    printf("%s\n", message_from_server);
    
    make_non_canonical();
    
    
    result = pthread_mutex_init(&mutex, NULL);
    if (result < 0)
    {
        perror("mutex_init failed!");
        return -1;
    }
    
    info.sock = sock;
    info.mutex = mutex;
    info.map = (char **)malloc(sizeof(char *) * DIMENSION);
    for (i = 0; i < DIMENSION; ++i)
    {
        info.map[i] = (char *)malloc(sizeof(char) * DIMENSION);
    }
    
    
    result = pthread_create(&threads[0], NULL, threads_read, &info);
    if (result != 0)
    {
        perror("Creating the thread ERROR");
        
        return -1;
    }
    
    result = pthread_create(&threads[1], NULL, threads_write, &info);
    if (result != 0)
    {
        perror("Creating the thread ERROR");
        
        return -1;
    }
    
    for (i = 0; i < 2; ++i)
    {
        result = pthread_join(threads[i], NULL);
        
        
        if (result != 0)
        {
            perror("Joining the thread ERROR");
            return -1;
        }
    }
    
    if (WINNER == 1)
    {
        print_text_from("win.txt");
    }
    else if (WINNER == -1)
    {
        print_text_from("lose.txt");
    }
    else if (WINNER == 0)
    {
        print_text_from("disconnection.txt");
    }
    
    for (i = 0; i < DIMENSION; ++i)
    {
        free(info.map[i]);
    }
    
    free((info.map));
    close(sock);
    return 0;
}


