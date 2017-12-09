#ifndef variable_h
#define variable_h
#include <pthread.h>
/**********************************************************************/
/*************************Иницаилизация сервера************************/
/**********************************************************************/

/* A struct to store the info about items on field */
struct Item
{
    int x, y, value, on;
};

struct Bombs
{
    int i;
    int j;
    int clients_id;
    int on;
};

struct coordinates
{
    int i;
    int j;
};

struct all_clients_info
{
    /* Количество клиентов.
     */
    int CLNT_NMBR;
    
    /* Свободные клетки на поле,
     * чтобы клиентов не было больше свободных клеток.
     */
    int FREE_CELLS;
    
    /* Массив сокетов для клиентов. !!!
     */
    int *CLIENTS_SOCKETS;
    
    /* Актуальное количество клиентов.
     */
    int CLIENTS_IN_ACTION;
    
    /* Сокет для ожидания клиентов игры.
     */
    int SOCKET_FOR_CLIENTS_WAITING;
    /* Сокет создателя игры.
     */
    int CREATOR_SOCK_ID;
    
    /* Имена игроков. !!!
     */
    char **CLIENTS_LOGINS;
    
    int ACTUAL_NUMBERS_OF_BOMBS;
    
    int ACTUAL_NUMBER_OF_CLIENTS;
    
    int *CLIENTS_ACTUAL_HEALTH;
    
    /* Массив структур для хранения информации о подключившихся клиентах. !!!
     */
    struct sockaddr_in *CLIENTS_INFO;
    
    /* Координаты всех клиентов в настоящий момент времени.
     */
    struct coordinates *CLIENTS_ACTUAL_COORDINATES;
    
    struct Bombs  *BOMBS;
    
    pthread_mutex_t *mutex;
    
    int *CLIENTS_IN_GAME;
    
};


/* A struct for handy work with a map */
struct MapInfo
{
    /* A map visual representation */
    char** map;
    int rows, cols;
    
    /* Info for generating mobs */
    int empty_cells;
    
    /* Additional parameters */
    int initial_health;
    int hit_value;
    float recharge_duration;
    float mining_time;
    int stay_health_drop;
    int movement_health_drop;
    float step_standard_delay;
    float moratory_duration;
    
    /* Array of items */
    struct Item* items;
    int number_of_items;
};


/************************************************************************/
/**************************Ининиализация потоков!************************/
/************************************************************************/


struct threads_info
{
    int id;
    int socket_id;
    int initial_health;
    int hit_value;
    int recharge_duration;
    int mining_time;
    int stay_health_drop;
    int movement_health_drop;
    int step_standard_delay;
    int moratory_duration;
    int over_due;
    char **map;
    struct coordinates *coor;
    int length_of_way;
    int BOOM;
    int EAT;
    int i;
    int j;
};


#endif /* variable_h */
