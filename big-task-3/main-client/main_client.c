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

#define BUF_SIZE 256

static struct termios stored_settings;

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


void text_reader(int sock)
{
    int i;
    int cmd_1 = 1;
    int cmd_2 = 2;
    int coord[2];
    int get_clnt_nmbr = 0;
    char name[61];
    int health;
    int symbol;
    while((symbol = fgetc(stdin)) != EOF)
    {
        if (symbol == 'q')
        {
            reset_keypress();
            write(sock, &cmd_2, sizeof(int));
            break;
        }
        
        if (symbol == 's')
        {
            write(sock, &cmd_1, sizeof(int));
            read(sock, &get_clnt_nmbr, sizeof(int));
            if (get_clnt_nmbr == -1)
            {
                printf("The game is finished!");
                reset_keypress();
                break;
            }
            for (i = 0; i < get_clnt_nmbr; ++i)
            {
                read(sock, name, sizeof(char) * 61);
                read(sock, coord, sizeof(int) * 2);
                read(sock, &health, sizeof(int));
                
                if (health < 0)
                {
                    continue;
                }
                
                printf("Player:  %s in point: (%d %d) with health: %d\n", name, coord[0], coord[1], health);
            }
            continue;
        }
        
    }
}


int main(int argc, char ** argv)
{
    int clnt_nmbr;
    int sock, port;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buf[BUF_SIZE];
    if (argc != 3)
    {
        fprintf(stderr, "Wrong options!");
        return 1;
    }
    
    port = atoi(argv[2]);
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        printf("socket() failed: %d", errno);
        return 1;
    }
    
    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        printf("Host not found\n");
        return 1;
    }
    
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    
    serv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    serv_addr.sin_port = htons(port);
    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("connect() failed: %d", errno);
        return 1;
    }
    printf("Please, enter a number of clients:\n");
    scanf("%d", &clnt_nmbr);
    write(sock, &clnt_nmbr, sizeof(int));
    
    memset(buf, 0, BUF_SIZE);
    read(sock, buf, BUF_SIZE-1);
    printf("%s\n", buf);
    
    
    
    
    memset(buf, 0, BUF_SIZE);
    read(sock, buf, BUF_SIZE-1);
    printf("%s\n", buf);
    
    printf("You can press 's' to watch statistic about players.\n Can press 'q' to quit.\n");
    
    make_non_canonical();
    text_reader(sock);
    
    close(sock);
    return 0;
}

