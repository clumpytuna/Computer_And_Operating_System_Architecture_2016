#include <stdlib.h>

#define SIZE 30

/* Possible directions */
const int dx[4] = {1, 0, -1, 0};
const int dy[4] = {0, 1, 0, -1};



/* Function that finds shortest way to the nearest enemy */
void find_way(struct coordinates* coords, int* len_of_way, char** map, int i, int j,
              int n, int m, int *outer_x, int *outer_y)
{
	int it;
    
	/* Array of distances to cells */
	int distance[SIZE * SIZE];

	/* qstart - for pop(), qend - for push() */
	int qstart, qend;
	
	/* Queue of states */
	int queue[SIZE * SIZE];

	/* Array of parent states */
	int parent[SIZE * SIZE];

	/* Element that popped forom the queue */
	int front;

	/* Positions of states */
	int x, y, new_x = 0, new_y = 0;

	/* Statement that answer is found */
	int flag = 0;
	
	/* For getting the answer */
	int pos = 0;

	/* Initializing distance by "MAX_VALUE" */
	for (it = 0; it < SIZE * SIZE; ++it)
	{
        /**/
		distance[it] = 99;
 	        parent[it] = -1;
	}
	
	*len_of_way = -1;
    *outer_x = -1;
    *outer_y = -1;
    
	distance[SIZE * i + j] = 0;

	qstart = 0, qend = 0;
	queue[qend] = SIZE * i + j;
	qend++;
	
	while (qstart < qend)
	{
		front = queue[qstart];
		qstart++;

		x = front / SIZE;
		y = front % SIZE;

		for (it = 0; it < 4; ++it)
		{
			new_x = x + dx[it];
			new_y = y + dy[it];

			if (0 <= new_x && new_x < SIZE && 0 <= new_y && new_y < SIZE && map[new_x][new_y] != '#')
			{
				if (distance[SIZE * x + y] + 1 < distance[SIZE * new_x + new_y] && map[new_x][new_y] == 'X')
				{
					distance[SIZE * new_x + new_y] = distance[SIZE * x + y] + 1;
					*len_of_way = distance[SIZE * new_x + new_y];
                    parent[SIZE * new_x + new_y] = SIZE * x + y;
					flag = 1;
                    *outer_x = new_x - i;
                    *outer_y = new_y - j;
                    break;
				}
	
				if (distance[SIZE * x + y] + 1 < distance[SIZE * new_x + new_y])
				{
					distance[SIZE * new_x + new_y] = distance[SIZE * x + y] + 1;
					parent[SIZE * new_x + new_y] = SIZE * x + y;
					queue[qend] = SIZE * new_x + new_y;
					qend++;
				}
			}
		}
		
		if (flag)
		{
			break;
		}
	}

    if (*len_of_way == -1)
    {
        return;
    }
    
	coords[pos].i = new_x;
	coords[pos].j = new_y;
	pos++;	
    
	while (parent[SIZE * new_x + new_y] != -1)
	{
		x = parent[SIZE * new_x + new_y] / SIZE;
		y = parent[SIZE * new_x + new_y] % SIZE;

		new_x = x;
		new_y = y;

		coords[pos].i = new_x;
		coords[pos].j = new_y;
		pos++;	
	}
	
}

