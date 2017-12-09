#define _XOPEN_SOURCE 500

#include "variable.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <strings.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "read_map-2.h"
#include "find_way-2.h"
#include <signal.h>

#define MAX_LENTGH_OF_FLIGHT 20
#define LENGTH_OF_SERVER_NAME 255

const int view = 15;
pthread_mutex_t worker;
pthread_mutex_t bomb;
pthread_mutex_t moratory;

pthread_t statics;

struct statis
{
    int sock;
    pthread_t *threads;
};

struct statis stats;

unsigned int size_of_sockaddr_in = sizeof(struct sockaddr_in);

struct all_clients_info CLIENTS_INFO;
struct MapInfo MAPS_INFO;
struct threads_info *DATA_FOR_THREADS; /* !!!
                                        */
int WORK = 1;
int MORATORY = 1;
int WINNER = -1;
int EXIT = 0;
int FREE = 0;

int socket_checker(int socket)
{
    if (socket < 0)
    {
        printf("socket() failed: %d\n", errno);
        return -1;
    }
    else
    {
        return 0;
    }
}

void items_on_map()
{
    int i;
    for (i = 0; i < MAPS_INFO.number_of_items; ++i)
    {
        if (MAPS_INFO.items[i].on)
        {
            MAPS_INFO.map[ (MAPS_INFO.items[i].x)][ (MAPS_INFO.items[i].y)] = '+';
        }
    }
}

void make_location(char ***map)
{
    int i = 0;
    int i_;
    int j_;
    struct coordinates location;
    for (i = 0; i < CLIENTS_INFO.CLNT_NMBR; ++i)
    {
        i_ = rand() % MAPS_INFO.cols;
        j_ = rand() % MAPS_INFO.rows;
        while ((*map)[i_][j_] != ' ')
        {
            i_ = rand() % MAPS_INFO.cols;
            j_ = rand() % MAPS_INFO.rows;
        }
        location.i = i_;
        location.j = j_;
        MAPS_INFO.map[i_][j_] = 'X';
        CLIENTS_INFO.CLIENTS_ACTUAL_COORDINATES[i + 1] = location;
        items_on_map();
    }

}



int server_init(int port, struct sockaddr_in *serv_addr)
{
    memset((char *) serv_addr, 0, size_of_sockaddr_in);
    
    serv_addr->sin_family = AF_INET;
    serv_addr->sin_addr.s_addr = INADDR_ANY;

    serv_addr->sin_port = htons(port);

    if (bind(CLIENTS_INFO.SOCKET_FOR_CLIENTS_WAITING, (struct sockaddr *) serv_addr,
             size_of_sockaddr_in) < 0)
    {
        printf("bind() failed: %d\n", errno);
        return -1;
    }
    
    return 0;

}


void clients_info_init()
{
    /* Количество клиентов.
     */
    CLIENTS_INFO.CLNT_NMBR = 0;
    
    /* Свободные клетки на поле,
     * чтобы клиентов не было больше свободных клеток.
     */
    CLIENTS_INFO.FREE_CELLS = 0;
    
    /* Массив сокетов для клиентов. !!!
     */
    CLIENTS_INFO.CLIENTS_SOCKETS = NULL;
    
    /* Актуальное количество клиентов.
     */
    CLIENTS_INFO.CLIENTS_IN_ACTION = 0;
    
    /* Сокет для ожидания клиентов игры.
     */
    CLIENTS_INFO.SOCKET_FOR_CLIENTS_WAITING = 0;
    /* Сокет создателя игры.
     */
    CLIENTS_INFO.CREATOR_SOCK_ID = 0;
    
    
    CLIENTS_INFO.ACTUAL_NUMBER_OF_CLIENTS = 0;
    /* Имена игроков. !!!
     */
    CLIENTS_INFO.CLIENTS_LOGINS = NULL;
    
    /* Массив структур для хранения информации о подключившихся клиентах. !!!
     */
    CLIENTS_INFO.CLIENTS_INFO = NULL;
    
    /* Координаты всех клиентов в настоящий момент времени.
     */
    CLIENTS_INFO.CLIENTS_ACTUAL_COORDINATES = NULL;
    
    CLIENTS_INFO.mutex = NULL;
    
    CLIENTS_INFO.CLIENTS_ACTUAL_HEALTH = NULL;
    
    CLIENTS_INFO.ACTUAL_NUMBERS_OF_BOMBS = 0;
    
    CLIENTS_INFO.BOMBS = NULL;
}

void init_clients(void)
{
    int i = 0;
    read(CLIENTS_INFO.CREATOR_SOCK_ID, &CLIENTS_INFO.CLNT_NMBR, sizeof(int));
    printf("We need to create : %d clients\n", CLIENTS_INFO.CLNT_NMBR);
    write(CLIENTS_INFO.CREATOR_SOCK_ID, "We are waiting for all clients", 31);
    
    CLIENTS_INFO.ACTUAL_NUMBER_OF_CLIENTS = CLIENTS_INFO.CLNT_NMBR;
    
    CLIENTS_INFO.CLIENTS_SOCKETS = (int *)malloc(sizeof(int) * CLIENTS_INFO.CLNT_NMBR );
    
    CLIENTS_INFO.CLIENTS_INFO = (struct sockaddr_in *)malloc(size_of_sockaddr_in * CLIENTS_INFO.CLNT_NMBR);
    
    CLIENTS_INFO.CLIENTS_LOGINS = (char **)malloc(sizeof(char *) * CLIENTS_INFO.CLNT_NMBR);
    
    DATA_FOR_THREADS = (struct threads_info *)malloc(sizeof(struct threads_info) * CLIENTS_INFO.CLNT_NMBR);
    
    for (i = 0; i < CLIENTS_INFO.CLNT_NMBR; ++i)
    {
        CLIENTS_INFO.CLIENTS_LOGINS[i] = malloc(sizeof(char) * 61);
    }
    
    CLIENTS_INFO.CLIENTS_ACTUAL_COORDINATES = (struct coordinates*)malloc(sizeof(struct coordinates) * (CLIENTS_INFO.CLNT_NMBR + 1));
    
    CLIENTS_INFO.CLIENTS_IN_ACTION = 0;
    
    CLIENTS_INFO.BOMBS = (struct Bombs *)malloc(sizeof(struct Bombs) * MAPS_INFO.empty_cells);
    
    for (i = 0; i < MAPS_INFO.empty_cells; ++i)
    {
        CLIENTS_INFO.BOMBS[i].on = 0;
    }
    
    for (i = 0; i < MAPS_INFO.empty_cells; ++i)
    {
        CLIENTS_INFO.BOMBS[i].clients_id = 0;
    }
    
    CLIENTS_INFO.mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t) * (CLIENTS_INFO.CLNT_NMBR + 1));
    
    CLIENTS_INFO.CLIENTS_ACTUAL_HEALTH = (int *)malloc(sizeof(int) * (CLIENTS_INFO.CLNT_NMBR + 1));
    
    for (i = 0; i < CLIENTS_INFO.CLNT_NMBR + 1; ++i)
    {
        CLIENTS_INFO.CLIENTS_ACTUAL_HEALTH[i] = MAPS_INFO.initial_health;
    }
    
    for (i = 0; i < CLIENTS_INFO.CLNT_NMBR + 1; ++i)
    {
        pthread_mutex_init(&CLIENTS_INFO.mutex[i], 0);
    }
    
    CLIENTS_INFO.FREE_CELLS = MAPS_INFO.empty_cells;
    make_location(&MAPS_INFO.map);
    
}

int init_socket(int *sock_id)
{
    /**
     * Recieve socked descriptor.
     */
    
    *sock_id = socket(AF_INET, SOCK_STREAM, 0);
    
    if (socket_checker(*sock_id) < 0) {
        printf("Error in creating socket!");
        return -1;
    }
    
    return 0;
}

int clients_waiting(void)
{
    int i = 0;
    char *message = "Please,enter your login and wait all over gamers:\n\0";
    char *ready = "All gamers are here! Let's get started!\n";

    for (i = 0; i < CLIENTS_INFO.CLNT_NMBR; ++i)
    {
        CLIENTS_INFO.CLIENTS_SOCKETS[i] = accept(CLIENTS_INFO.SOCKET_FOR_CLIENTS_WAITING,
                             (struct sockaddr *) &CLIENTS_INFO.CLIENTS_INFO[i],
                             &size_of_sockaddr_in);
        
        if (CLIENTS_INFO.CLIENTS_SOCKETS[i] < 0)
        {
            printf("accept() failed in creatind %d client: %d\n", i + 1, errno);
            return -1;
        }
        
        ++CLIENTS_INFO.CLIENTS_IN_ACTION;
        write(CLIENTS_INFO.CLIENTS_SOCKETS[i], message, sizeof(char) * (strlen(message)));
        read(CLIENTS_INFO.CLIENTS_SOCKETS[i], CLIENTS_INFO.CLIENTS_LOGINS[i], sizeof(char) * 61);
        printf("Clients connection!\n");
        /*printf("%d client name is %s \n",  i + 1, CLIENTS_INFO.CLIENTS_LOGINS[i]);*/
    }
    
    for ( i = 0; i < CLIENTS_INFO.CLNT_NMBR; ++i)
    {
        write(CLIENTS_INFO.CLIENTS_SOCKETS[i], ready, sizeof(char) * (strlen(ready)));
    }
    
    write(CLIENTS_INFO.CREATOR_SOCK_ID, ready, sizeof(char) * (strlen(ready)));
    
    return 0;
}


void in_items(int i, int j, struct threads_info *info)
{
    int index;
    for (index = 0; index < MAPS_INFO.number_of_items; ++index)
    {
        if (MAPS_INFO.items[index].on && MAPS_INFO.items[index].x == i && MAPS_INFO.items[index].y == j)
        {
            pthread_mutex_lock(&CLIENTS_INFO.mutex[info->id]);
            MAPS_INFO.items[index].on = 0;
            CLIENTS_INFO.CLIENTS_ACTUAL_HEALTH[info->id] += MAPS_INFO.items[index].value;
            MAPS_INFO.map[i][j] = ' ';
            pthread_mutex_unlock(&CLIENTS_INFO.mutex[info->id]);
        }
    }
}


int blows_on_bobms(int client_id, int i, int j)
{
    int index;
    pthread_mutex_lock(&bomb);
    for (index = 0; index < CLIENTS_INFO.FREE_CELLS; ++index)
    {
        if (CLIENTS_INFO.BOMBS[index].on == 1 &&
            CLIENTS_INFO.BOMBS[index].i == i &&
            CLIENTS_INFO.BOMBS[index].j == j && CLIENTS_INFO.BOMBS[index].clients_id != client_id)
        {
            CLIENTS_INFO.BOMBS[index].on = 0;
            CLIENTS_INFO.ACTUAL_NUMBERS_OF_BOMBS -= 1;
            pthread_mutex_unlock(&bomb);
            return 1;
        }
    }
    
    pthread_mutex_unlock(&bomb);
    return 0;
}


int in_bobms(int client_id, int i, int j)
{
    int index;
    pthread_mutex_lock(&bomb);
    for (index = 0; index < CLIENTS_INFO.FREE_CELLS; ++index)
    {
        if (CLIENTS_INFO.BOMBS[index].on == 1 &&
            CLIENTS_INFO.BOMBS[index].i == i &&
            CLIENTS_INFO.BOMBS[index].j == j && CLIENTS_INFO.BOMBS[index].clients_id == client_id)
        {
            pthread_mutex_unlock(&bomb);
            return 1;
        }
    }
    
    pthread_mutex_unlock(&bomb);
    return 0;
}


int move_checker(int *i, int *j, int cmd, struct threads_info *info)
{
    int i_ = *i;
    int j_ = *j;
    pthread_mutex_lock(&CLIENTS_INFO.mutex[0]);
    if (cmd == 1)
    {
        j_ += 1;
    }
    if (cmd == 2)
    {
        i_ -= 1;
    }
    if (cmd == 3)
    {
        j_ -= 1;
    }
    if (cmd == 4)
    {
        i_ += 1;
    }
    if ((j_) < 0 || (j_) > MAPS_INFO.cols || (i_) < 0 || (i_) > MAPS_INFO.rows)
    {
        pthread_mutex_unlock(&CLIENTS_INFO.mutex[0]);
        return 0;
    } else
    {
        if (MAPS_INFO.map[i_][j_] != '#' && MAPS_INFO.map[i_][j_] != 'X')
        {
            
            MAPS_INFO.map[*i][*j] = ' ';
            items_on_map();
            *i = i_;
            *j = j_;
            MAPS_INFO.map[*i][*j] = 'X';
            CLIENTS_INFO.CLIENTS_ACTUAL_COORDINATES[info->id].i = *i;
            CLIENTS_INFO.CLIENTS_ACTUAL_COORDINATES[info->id].j = *j;
            
            pthread_mutex_lock(&CLIENTS_INFO.mutex[info->id]);
            CLIENTS_INFO.CLIENTS_ACTUAL_HEALTH[info->id] -= abs(MAPS_INFO.stay_health_drop
                                                                - MAPS_INFO.movement_health_drop);
            pthread_mutex_unlock(&CLIENTS_INFO.mutex[info->id]);
        }
    }
    
    if (blows_on_bobms(info->id, *i, *j))
    {
        pthread_mutex_lock(&CLIENTS_INFO.mutex[info->id]);
        CLIENTS_INFO.CLIENTS_ACTUAL_HEALTH[info->id] -= MAPS_INFO.hit_value;
        pthread_mutex_unlock(&CLIENTS_INFO.mutex[info->id]);
    }
    
    pthread_mutex_unlock(&CLIENTS_INFO.mutex[0]);

    return 0;
}

void add_bombs(int client_id, int i, int j)
{
    int index = 0;
    pthread_mutex_lock(&bomb);
    for (index = 0;  index < CLIENTS_INFO.FREE_CELLS; ++index)
    {
        if (CLIENTS_INFO.BOMBS[index].on == 0)
        {
            CLIENTS_INFO.BOMBS[index].on = 1;
            CLIENTS_INFO.BOMBS[index].clients_id = client_id;
            CLIENTS_INFO.BOMBS[index].i = i;
            CLIENTS_INFO.BOMBS[index].j = j;
            CLIENTS_INFO.ACTUAL_NUMBERS_OF_BOMBS++;
            break;
        }
    }
    pthread_mutex_unlock(&bomb);
}


void decrease_health(int i, int j, int k)
{
    int index;
    
    if (k == 0)
    {
        k = 1;
    }
    
    for (index = 1; index < CLIENTS_INFO.CLNT_NMBR + 1; ++index)
    {
        if (CLIENTS_INFO.CLIENTS_ACTUAL_COORDINATES[index].i == i &&
            CLIENTS_INFO.CLIENTS_ACTUAL_COORDINATES[index].j == j)
        {
                pthread_mutex_lock(&CLIENTS_INFO.mutex[index]);
                CLIENTS_INFO.CLIENTS_ACTUAL_HEALTH[index] -= MAPS_INFO.hit_value * 1/k;
                pthread_mutex_unlock(&CLIENTS_INFO.mutex[index]);
        }
    }
}


void show_file(int i, int j, int socket ,struct threads_info *info , int should_do)
{

    const int n = 30;
    const int m = 30;
    int i_1;
    int j_1;
    int k;
    int i_0 = i - view;
    int j_0 = j - view;
    int cmd;
    int my_health;
    int offset_x = 0;
    int offset_y = 0;
    pthread_mutex_lock(&worker);
    pthread_mutex_lock(&CLIENTS_INFO.mutex[0]);
    /*TODO*/
    pthread_mutex_lock(&CLIENTS_INFO.mutex[info->id]);
    my_health = CLIENTS_INFO.CLIENTS_ACTUAL_HEALTH[info->id];
    
    if (WINNER != info -> id && WINNER != -1)
    {
        my_health = -2000;
        for (i_1 = 0; i_1 < n; ++i_1)
        {
            for(j_1 = 0; j_1 < m; ++j_1)
            {
                write(socket, &((info->map)[i_1][j_1]), sizeof(char));
            }
        }
        
        write(socket, &my_health, sizeof(int));
        pthread_mutex_unlock(&CLIENTS_INFO.mutex[info->id]);
        pthread_mutex_unlock(&worker);
        pthread_mutex_unlock(&CLIENTS_INFO.mutex[0]);
        return;
    }
    
    if (WINNER == info -> id)
    {
        my_health = -1000;
        for (i_1 = 0; i_1 < n; ++i_1)
        {
            for(j_1 = 0; j_1 < m; ++j_1)
            {
                write(socket, &((info->map)[i_1][j_1]), sizeof(char));
            }
        }
        
        write(socket, &my_health, sizeof(int));
        pthread_mutex_unlock(&CLIENTS_INFO.mutex[info->id]);
        pthread_mutex_unlock(&CLIENTS_INFO.mutex[0]);
        pthread_mutex_unlock(&worker);
        return;
    }
    
    pthread_mutex_unlock(&CLIENTS_INFO.mutex[info->id]);
    pthread_mutex_unlock(&worker);
    
    for (i_1 = 0; i_1 < n; ++i_1)
    {
        for(j_1 = 0; j_1 < m; ++j_1)
        {
            info->map[i_1][j_1] = MAPS_INFO.map[i_0 + i_1][j_0 + j_1];

            if (in_bobms(info->id, i_0 + i_1, j_0 + j_1) && MAPS_INFO.map[i_0 + i_1][j_0 + j_1] != 'O' &&
                 MAPS_INFO.map[i_0 + i_1][j_0 + j_1] != 'X')
            {
               info->map[i_1][j_1] = '*';
            }
        }
    }
    
    info->map[15][15] = 'O';
    
    if (info->EAT)
    {
        in_items(i, j, info);
        info->EAT = 0;
        
        for (i_1 = 0; i_1 < n; ++i_1)
        {
            for(j_1 = 0; j_1 < m; ++j_1)
            {
                write(socket, &((info->map)[i_1][j_1]), sizeof(char));
            }
        }
        write(socket, &my_health, sizeof(int));
        pthread_mutex_unlock(&CLIENTS_INFO.mutex[0]);
        usleep(MAPS_INFO.step_standard_delay * 1000000);
        return;
    }
    if (info->BOOM)
    {
        find_way(info->coor, &(info->length_of_way), info->map, 15, 15, 30, 30, &offset_x, &offset_y);
        
        if (info -> length_of_way != -1 && info->length_of_way <= MAX_LENTGH_OF_FLIGHT)
        {
            /*TODO*/
            decrease_health(offset_x + i, offset_y + j, info->length_of_way);
            for (i_1 = 1; i_1 < info->length_of_way; ++i_1)
            {
                info->map[info->coor[i_1].i][info->coor[i_1].j] = '.';
            }
            info->map[info->coor[0].i][info->coor[0].j] = '@';
        }
        for (k = 0; k < 4; ++k)
        {
            if (k >0)
            {
                read(socket, &cmd, sizeof(int));
            }
            for (i_1 = 0; i_1 < n; ++i_1)
            {
                for(j_1 = 0; j_1 < m; ++j_1)
                {
                    write(socket, &((info->map)[i_1][j_1]), sizeof(char));
                }
            }
            write(socket, &my_health, sizeof(int));
        }
        info -> BOOM = 0;
        pthread_mutex_unlock(&CLIENTS_INFO.mutex[0]);
        usleep(MAPS_INFO.recharge_duration * 1000000);
        return;
    }
    
    if (should_do == 7)
    {
        add_bombs(info->id, i, j);
        
        for (k = 0; k < 10; ++k)
        {
            usleep((1000000 * MAPS_INFO.mining_time) / 10);
            
            pthread_mutex_lock(&CLIENTS_INFO.mutex[info->id]);
            my_health = CLIENTS_INFO.CLIENTS_ACTUAL_HEALTH[info->id];
            pthread_mutex_unlock(&CLIENTS_INFO.mutex[info->id]);
            
            if (k >0)
            {
                read(socket, &cmd, sizeof(int));
            }
            for (i_1 = 0; i_1 < n; ++i_1)
            {
                for(j_1 = 0; j_1 < m; ++j_1)
                {
                    write(socket, &((info->map)[i_1][j_1]), sizeof(char));
                }
            }
            write(socket, &my_health, sizeof(int));
        }
        
        pthread_mutex_unlock(&CLIENTS_INFO.mutex[0]);
        return;
    }
    
    
    for (i_1 = 0; i_1 < n; ++i_1)
    {
        for(j_1 = 0; j_1 < m; ++j_1)
        {
            write(socket, &((info->map)[i_1][j_1]), sizeof(char));
        }
    }
    
    write(socket, &my_health, sizeof(int));
    pthread_mutex_unlock(&CLIENTS_INFO.mutex[info->id]);
    pthread_mutex_unlock(&CLIENTS_INFO.mutex[0]);
    usleep(MAPS_INFO.step_standard_delay * 1000000);
}



int move_maker(struct threads_info *info, int turn)
{
    move_checker(&(info->i), &(info->j), turn, info);
    show_file((info->i), (info->j), info->socket_id, info, turn);
    return 0;
}



int command_parser(int cmd, int sock, struct threads_info *info)
{
    
    
    pthread_mutex_lock(&moratory);
    if (MORATORY == 1 && cmd > 4)
    {
        cmd = 0;
    }
    pthread_mutex_unlock(&moratory);
    
    pthread_mutex_lock(&worker);
    pthread_mutex_lock(&CLIENTS_INFO.mutex[0]);
    
    if (CLIENTS_INFO.ACTUAL_NUMBER_OF_CLIENTS == 1)
    {
        WINNER = info ->id;
        
        pthread_mutex_unlock(&CLIENTS_INFO.mutex[0]);
        pthread_mutex_unlock(&worker);
        
        if (move_maker(info, cmd) < 0)
        {
            return -1;
        }
        
        return -1;
    }
    pthread_mutex_unlock(&CLIENTS_INFO.mutex[0]);
    pthread_mutex_unlock(&worker);
    
    if (cmd == -1)
    {
        pthread_mutex_lock(&worker);
        pthread_mutex_lock(&CLIENTS_INFO.mutex[0]);
        pthread_mutex_lock(&CLIENTS_INFO.mutex[info->id]);
        MAPS_INFO.map[info->i][info->j] = ' ';
        CLIENTS_INFO.CLIENTS_ACTUAL_HEALTH[info->id] = -2000;
        --CLIENTS_INFO.ACTUAL_NUMBER_OF_CLIENTS;
        pthread_mutex_unlock(&CLIENTS_INFO.mutex[info->id]);
        pthread_mutex_unlock(&CLIENTS_INFO.mutex[0]);
        pthread_mutex_unlock(&worker);
        return -1;
    }
    
    if (cmd == 0)
    {
        if (move_maker(info, cmd) < 0)
        {
            return -1;
        }
        
    }
    if (cmd == 1)
    {
        if (move_maker(info, cmd) < 0)
        {
            return -1;
        }
        
    }
    if (cmd == 2)
    {

        if (move_maker(info, cmd) < 0)
        {
            return -1;
        }
    }
    if (cmd == 3)
    {
        if (move_maker(info, cmd) < 0)
        {
            return -1;
        }
    }
    if (cmd == 4)
    {
        if (move_maker(info, cmd) < 0)
        {
            return -1;
        }
    }
    /*BOOM maker*/
    if (cmd == 5)
    {
        info->BOOM = 1;
        if (move_maker(info, cmd) < 0)
        {
            return -1;
        }
    }

    if (cmd == 6)
    {
        info->EAT = 1;
        if (move_maker(info, cmd) < 0)
        {
            return -1;
        }
    }
    
    /*Mine maker*/
    if (cmd == 7)
    {
        if (move_maker(info, cmd) < 0 )
        {
            return -1;
        }
    }
    
    return 0;
}

void* gameplay_maker(void *arg)
{
    int cmnd;
    
    struct threads_info *info = arg;
    while (1)
    {
        read(info->socket_id, &cmnd, sizeof(int));
        if (command_parser(cmnd, info->socket_id, info) < 0)
        {
            return (void *)0;
        }
    }
    return (void *)0;
}

void threads_data_init(pthread_t **threads)
{
    int i;
    int j;
    /*С единицы клиенты*/
    for (i = 0; i < CLIENTS_INFO.CLNT_NMBR; ++i)
    {
        DATA_FOR_THREADS[i].id = i + 1;
        DATA_FOR_THREADS[i].socket_id = CLIENTS_INFO.CLIENTS_SOCKETS[i];
        DATA_FOR_THREADS[i].i = CLIENTS_INFO.CLIENTS_ACTUAL_COORDINATES[i + 1].i;
        DATA_FOR_THREADS[i].j = CLIENTS_INFO.CLIENTS_ACTUAL_COORDINATES[i + 1].j;
        DATA_FOR_THREADS[i].hit_value = MAPS_INFO.hit_value;
        DATA_FOR_THREADS[i].initial_health = MAPS_INFO.initial_health;
        DATA_FOR_THREADS[i].mining_time = MAPS_INFO.mining_time;
        DATA_FOR_THREADS[i].moratory_duration = MAPS_INFO.moratory_duration;
        DATA_FOR_THREADS[i].movement_health_drop = MAPS_INFO.movement_health_drop;
        DATA_FOR_THREADS[i].recharge_duration = MAPS_INFO.recharge_duration;
        DATA_FOR_THREADS[i].stay_health_drop = MAPS_INFO.stay_health_drop;
        DATA_FOR_THREADS[i].step_standard_delay = MAPS_INFO.step_standard_delay;
        DATA_FOR_THREADS[i].coor = (struct coordinates *)malloc(sizeof(struct coordinates) * 500);
        DATA_FOR_THREADS[i].BOOM = 0;
        DATA_FOR_THREADS[i].EAT = 0;
        DATA_FOR_THREADS[i].map = (char **)malloc(sizeof(char *) * 40);
        for (j = 0; j < 40; ++j)
        {
            DATA_FOR_THREADS[i].map[j] = (char *)malloc(sizeof(char ) * 40);
        }
    }
    (*threads) = (pthread_t *) malloc(sizeof(pthread_t) * CLIENTS_INFO.CLNT_NMBR);
}

void destructor()
{
    int i;
    int j;
    
    if (CLIENTS_INFO.CREATOR_SOCK_ID != 0)
    {
        close(CLIENTS_INFO.CREATOR_SOCK_ID);
    }
    
    if (CLIENTS_INFO.CLIENTS_SOCKETS != NULL)
    {
        for (i = 0; i < CLIENTS_INFO.CLNT_NMBR; ++i)
        {
            close(CLIENTS_INFO.CLIENTS_SOCKETS[i]);
        }
    }
    if (CLIENTS_INFO.CLIENTS_SOCKETS != NULL)
    {
        free(CLIENTS_INFO.CLIENTS_SOCKETS);
    }
    
    for (i = 0; i < CLIENTS_INFO.CLNT_NMBR; ++i)
    {
        if (CLIENTS_INFO.CLIENTS_LOGINS[i] != NULL)
        {
            free(CLIENTS_INFO.CLIENTS_LOGINS[i]);
        }
    }
    
    if (CLIENTS_INFO.CLIENTS_LOGINS != NULL)
    {
        free(CLIENTS_INFO.CLIENTS_LOGINS);
    }
    
    if (CLIENTS_INFO.CLIENTS_INFO != NULL)
    {
        free(CLIENTS_INFO.CLIENTS_INFO);
    }
    
    if (CLIENTS_INFO.CLIENTS_ACTUAL_COORDINATES != NULL)
    {
        free(CLIENTS_INFO.CLIENTS_ACTUAL_COORDINATES);
    }
    
    if (CLIENTS_INFO.BOMBS != NULL)
    {
         free(CLIENTS_INFO.BOMBS);
    }
    
    if (CLIENTS_INFO.CLNT_NMBR > 0)
    {
        for (i = 0; i < CLIENTS_INFO.CLNT_NMBR + 1; ++i)
        {
                pthread_mutex_destroy(&CLIENTS_INFO.mutex[i]);
        }
    }
    
    pthread_mutex_destroy(&worker);
    pthread_mutex_destroy(&bomb);
    pthread_mutex_destroy(&moratory);
    
    for (i = 0 ; i < CLIENTS_INFO.CLNT_NMBR; ++i)
    {
         if (DATA_FOR_THREADS[i].coor != NULL)
         {
             free(DATA_FOR_THREADS[i].coor);
         }
        
        if (DATA_FOR_THREADS[i].map != NULL)
        {
            for (j = 0; j < 40; ++j)
            {
                if (DATA_FOR_THREADS[i].map[j] != NULL)
                {
                    free(DATA_FOR_THREADS[i].map[j]);
                }
            }
            free(DATA_FOR_THREADS[i].map);
        }
    }
    
    free(CLIENTS_INFO.mutex);
    
    close(CLIENTS_INFO.SOCKET_FOR_CLIENTS_WAITING);
    close(CLIENTS_INFO.CREATOR_SOCK_ID);
    destroy_map(&MAPS_INFO);
    
}

void print_hosts_name(char* name, int length_of_server_name)
{
    gethostname((char *)name, length_of_server_name);
    printf("Host name is %s\n", name);
}



void health_droper()
{
    float k = MAPS_INFO.moratory_duration / MAPS_INFO.step_standard_delay;
    int i = 0;
    static float j = 0;
    usleep(MAPS_INFO.step_standard_delay * 1000000);
    j += 1;
    if (j > k)
    {
        pthread_mutex_lock(&moratory);
        MORATORY = 0;
        pthread_mutex_unlock(&moratory);
        
    }
    for (i = 1; i < CLIENTS_INFO.CLNT_NMBR + 1; ++i)
    {
        pthread_mutex_lock(&CLIENTS_INFO.mutex[i]);
        if (CLIENTS_INFO.CLIENTS_ACTUAL_HEALTH[i] >= 0)
        {
            CLIENTS_INFO.CLIENTS_ACTUAL_HEALTH[i] -= MAPS_INFO.stay_health_drop;
        }
        pthread_mutex_unlock(&CLIENTS_INFO.mutex[i]);
    }
}

void handler(int sig)
{
    destructor();
    exit(0);
}

void stat_view(int cmd, int sock)
{
    int i = 0;
    int coord[2];
    if (cmd == 1)
    {
        

        write(sock, &CLIENTS_INFO.ACTUAL_NUMBER_OF_CLIENTS, sizeof(int));
        for (i = 0; i < CLIENTS_INFO.ACTUAL_NUMBER_OF_CLIENTS; ++i)
        {
            if (CLIENTS_INFO.CLIENTS_ACTUAL_HEALTH[i + 1] < 0)
            {
                continue;
            }
            write(sock, CLIENTS_INFO.CLIENTS_LOGINS[i], sizeof(char) * 61);
            coord[0] = CLIENTS_INFO.CLIENTS_ACTUAL_COORDINATES[i + 1].i;
            coord[1] = CLIENTS_INFO.CLIENTS_ACTUAL_COORDINATES[i + 1].j;
            write(sock, coord, sizeof(int) * 2);
            write(sock, &CLIENTS_INFO.CLIENTS_ACTUAL_HEALTH[i + 1], sizeof(int));
        }

    }
}

void *statistics(void *arg)
{
    struct statis *data = (struct statis *)arg;
    int cmd;
    int i = 0;
    int coord[2];
    int end = -1;
    while (1)
    {
        read(data->sock, &cmd, sizeof(int));
        if (cmd == 1)
        {
            
            pthread_mutex_lock(&CLIENTS_INFO.mutex[0]);
            if (CLIENTS_INFO.ACTUAL_NUMBER_OF_CLIENTS <= 1)
            {
                write(data->sock, &end, sizeof(int));
                return (void *)0;
            }
            
            write(data->sock, &CLIENTS_INFO.CLNT_NMBR, sizeof(int));
            for (i = 0; i < CLIENTS_INFO.CLNT_NMBR; ++i)
            {
                write(data->sock, CLIENTS_INFO.CLIENTS_LOGINS[i], sizeof(char) * 61);
                coord[0] = CLIENTS_INFO.CLIENTS_ACTUAL_COORDINATES[i + 1].i;
                coord[1] = CLIENTS_INFO.CLIENTS_ACTUAL_COORDINATES[i + 1].j;
                write(data->sock, coord, sizeof(int) * 2);
                write(data->sock, &CLIENTS_INFO.CLIENTS_ACTUAL_HEALTH[i + 1], sizeof(int));
            }
            pthread_mutex_unlock(&CLIENTS_INFO.mutex[0]);
        }
        
        if (cmd == 3)
        {
            pthread_mutex_lock(&CLIENTS_INFO.mutex[0]);
            for (i = 0; i < CLIENTS_INFO.CLNT_NMBR;++i)
            {
                close(CLIENTS_INFO.CLIENTS_SOCKETS[i]);
            }
            pthread_mutex_unlock(&CLIENTS_INFO.mutex[0]);
                        return (void *)0;

        }
        
        if (cmd == 2)
        {
            return (void *)0;
        }

    }
    return (void *)0;
}

int main(int argc, const char * argv[])
{

    int i;
    /*int symbol;*/
    int port;
    int result;
    pthread_t *threads = NULL;
    /*size_t length_of_server_name = 255;*/
    const char name[LENGTH_OF_SERVER_NAME];
    struct sockaddr_in serv_addr, cli_addr;
    char *name_of_map_file = "/home/yar/Рабочий стол/Last Edition/Server/input.txt";
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = handler;
    sigaction(SIGINT, &act, 0);
    pthread_mutex_init(&worker, 0);
    pthread_mutex_init(&bomb, 0);
    pthread_mutex_init(&moratory, 0);
    /*
    while ((symbol = getopt (argc, argv, "phf:")) != -1)
        switch (symbol)
    {
        case 'f':
            name_of_map_file = optarg;
            break;
        case 'h':
            printf("Welcome to doom!\n");
            return 0;
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case '?':
            perror("Unkown options!");
            return 1;
    }
    */
    
    if (argc < 2)
    {
        fprintf(stderr,"usage:  <port_number>\n");
        return 1;
    }
    port = atoi(argv[1]);
    
    clients_info_init();
    
    if (init_socket(&CLIENTS_INFO.SOCKET_FOR_CLIENTS_WAITING) < 0)
    {
        exit(1);
    }
    
    if (server_init(port, &serv_addr) < 0)
    {
        exit(1);
    };
    
    
    if (read_map(name_of_map_file, &MAPS_INFO, 30, 30) < 0)
    {
        destructor();
        return 1;
    }


    gethostname((char *)name, LENGTH_OF_SERVER_NAME);
    printf("Host name is %s\n", name);
    items_on_map();
    print_maps_info(MAPS_INFO);
    
    listen(CLIENTS_INFO.SOCKET_FOR_CLIENTS_WAITING, 1);
    

    CLIENTS_INFO.CREATOR_SOCK_ID = accept(CLIENTS_INFO.SOCKET_FOR_CLIENTS_WAITING,
                         (struct sockaddr *) &cli_addr,
                         &size_of_sockaddr_in);
    
    if (CLIENTS_INFO.CREATOR_SOCK_ID < 0)
    {
        printf("accept() failed: %d\n", errno);
        return EXIT_FAILURE;
    }
    
    init_clients();
    print_maps_info(MAPS_INFO);

    listen(CLIENTS_INFO.SOCKET_FOR_CLIENTS_WAITING, CLIENTS_INFO.CLNT_NMBR);
    
    clients_waiting();
    threads_data_init(&threads);
    
    for (i = 0; i < CLIENTS_INFO.CLNT_NMBR; ++i)
    {
        result = pthread_create(&threads[i], NULL, gameplay_maker, &DATA_FOR_THREADS[i]);
        if (result != 0)
        {
            perror("Creating the thread failed");
            destructor();
            return -1;
        }
        
    }
    stats.sock = CLIENTS_INFO.CREATOR_SOCK_ID;
    stats.threads = threads;
    result = pthread_create(&statics, NULL, statistics, &stats);
    if (result != 0)
    {
        perror("Creating the thread failed");
        destructor();
        return -1;
    }
    
    
    while (1)
    {
        pthread_mutex_lock(&worker);
        pthread_mutex_lock(&CLIENTS_INFO.mutex[0]);
        if (CLIENTS_INFO.ACTUAL_NUMBER_OF_CLIENTS <= 1)
        {
            pthread_mutex_unlock(&worker);
            pthread_mutex_unlock(&CLIENTS_INFO.mutex[0]);
            break;
        }
        pthread_mutex_unlock(&CLIENTS_INFO.mutex[0]);
        if(WINNER != -1)
        {
            pthread_mutex_unlock(&worker);
            break;
        }
        pthread_mutex_unlock(&worker);
        health_droper();
    }
    
    for (i = 0; i < CLIENTS_INFO.CLNT_NMBR; ++i)
    {
        result = pthread_join(threads[i], NULL);
        if (result != 0)
        {
            perror("Joining the thread failed");
            destructor();
            return -1;
        }
    }
    
    result = pthread_join(statics, NULL);
    if (result != 0)
    {
        perror("Joining the thread failed");
        destructor();
        return -1;
    }
    
    destructor();
    free(threads);
    close(CLIENTS_INFO.CREATOR_SOCK_ID);
    close(CLIENTS_INFO.SOCKET_FOR_CLIENTS_WAITING);
    return 0;
}
