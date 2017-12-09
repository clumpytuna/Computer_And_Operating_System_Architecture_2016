#ifndef READ_MAP_H
#define READ_MAP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "safe_read-2.h"
#include "variable.h"


/* A struct initialiser function */
void init_map(struct MapInfo* map_info)
{
	map_info->rows = 0;
	map_info->cols = 0;
	
	map_info->map = NULL;

	map_info->empty_cells = 0;

	map_info->initial_health = 0;
	map_info->hit_value = 0;
	map_info->recharge_duration = 0;
	map_info->mining_time = 0;
	map_info->stay_health_drop = 0;
	map_info->movement_health_drop = 0;
	map_info->step_standard_delay = 0;
	map_info->moratory_duration = 0;

	map_info->items = NULL;
	map_info->number_of_items = 0;
}

/* Function that append additional rows and columns */
void wall_builder(char* str, int pos, int size)
{
	int i;
	for (i = pos; i < pos + size; ++i)
	{
		str[i] = '#';
	}

	str[pos + size] = '\0';	
}

/* Main logic function here:
	- reads a map from a stream named <filename>
	- info stores in a struct MapInfo
	- handle errors 
*/
int read_map(char* filename, struct MapInfo* map_info, int n, int m)
{
	int i, j, error, command_size;
	char* command;
	char* non_space_part;
	char equal[2];
	char* buf_item;
	char* buffer;
	char ch;
    FILE* file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Ошибка при работе с файлом");
        exit(1);
    }
    
	init_map(map_info);

	while (1)
	{
		if (feof(file))
		{
			break;
		}

		error = 0;
		command = safe_get_string(file, &error);	
		
		if (error)
		{
		
			perror("An error has occured while reading map from file.\n");
			fclose(file);
			free(command);
			return -1;
		}

		command_size = (int)strlen(command);

		if (!command_size)
		{
			free(command);
			continue;
		}

		non_space_part = (char*) malloc((command_size + 1) * sizeof(char));
		sscanf(command, "%s", non_space_part);
		
		if (strcmp(non_space_part, "Map") == 0 || strcmp(non_space_part, "map") == 0)
		{
			sscanf(command, "%s %dx%d", non_space_part, &(map_info->rows), &(map_info->cols));
			
			map_info->rows += 2 * n + 2;
			map_info->cols += 2 * m + 2;
	
			map_info->map = (char**) malloc(map_info->rows * sizeof(char*));
			
			for (i = 0; i < n; ++i)
			{
				map_info->map[i] = (char*) malloc((map_info->cols + 1) * sizeof(char));
				wall_builder(map_info->map[i], 0, map_info->cols);
			}

			for (i = n; i < map_info->rows - n; ++i)
			{
				map_info->map[i] = (char*) malloc((map_info->cols + 1) * sizeof(char));

				wall_builder(map_info->map[i], 0, m);
				buffer = safe_get_string(file, &error);
				strcat(map_info->map[i], buffer);
				wall_builder(map_info->map[i], map_info->cols - m, m);

				if (error)
				{
					perror("An error has occured while reading map from file.\n");
					fclose(file);
					free(command);
					free(non_space_part);
					return -1;
				}
				free(buffer);
			}

			for (i = map_info->rows - n; i < map_info->rows; ++i)
			{
				map_info->map[i] = (char*) malloc((map_info->cols + 1) * sizeof(char));
				wall_builder(map_info->map[i], 0, map_info->cols);
			}

			for (i = 0; i < map_info->rows; ++i)
			{
				for (j = 0; j < map_info->cols; ++j)
				{
					if (map_info->map[i][j] == ' ')
					{
						map_info->empty_cells++;
					}
				}
			}

		} else if (strcmp(non_space_part, "items:") == 0)
		{
			while (1)
			{
				ch = fgetc(file);

				if (feof(file))
				{
					break;
				}

				if (!('1' <= ch && ch <= '9'))
				{
					ungetc(ch, file);
					break;
				}

				ungetc(ch, file);
				
				buf_item = safe_get_string(file, &error);
				
				if (error)
				{
					perror("An error has occured while reading map from file.\n");
					fclose(file);
					free(command);
					free(non_space_part);
					return -1;
				}
							
				if (map_info->number_of_items == 0)
				{
					map_info->items = (struct Item*) malloc(100 * sizeof(struct Item));
					map_info->number_of_items++;

					sscanf(buf_item, "%d %d %d", &map_info->items[0].x, 
								     &map_info->items[0].y, 
								     &map_info->items[0].value);
                    map_info->items[0].x += n;
                    map_info->items[0].y += m;
                    map_info->items[0].on = 1;
				} else
				{
					map_info->number_of_items++;

					if (map_info->number_of_items % 100 == 0)
					{					

						map_info->items = (struct Item*) realloc(map_info->items, 
									map_info->number_of_items * sizeof(struct Item));
					}
					
					sscanf(buf_item, "%d %d %d", &map_info->items[map_info->number_of_items - 1].x,
								     &map_info->items[map_info->number_of_items - 1].y,
								     &map_info->items[map_info->number_of_items - 1].value);
                    map_info->items[map_info->number_of_items - 1].x += n;
                    map_info->items[map_info->number_of_items - 1].y += m;
                    map_info->items[map_info->number_of_items - 1].on = 1;
				}

				free(buf_item);
			}
		
		} else if (strcmp(non_space_part, "initial_health") == 0)
		{
			sscanf(command, "%s %s %d", non_space_part, equal, &(map_info->initial_health));
		} else if (strcmp(non_space_part, "hit_value") == 0)
		{
			sscanf(command, "%s %s %d", non_space_part, equal, &(map_info->hit_value));
		} else if (strcmp(non_space_part, "recharge_duration") == 0)
		{
			sscanf(command, "%s %s %f", non_space_part, equal, &(map_info->recharge_duration));
		} else if (strcmp(non_space_part, "mining_time") == 0)
		{	
			sscanf(command, "%s %s %f", non_space_part, equal, &(map_info->mining_time));
		} else if (strcmp(non_space_part, "stay_health_drop") == 0)
		{
			sscanf(command, "%s %s %d", non_space_part, equal, &(map_info->stay_health_drop));
		} else if (strcmp(non_space_part, "movement_health_drop") == 0)
		{
			sscanf(command, "%s %s %d", non_space_part, equal, &(map_info->movement_health_drop));
		} else if (strcmp(non_space_part, "step_standard_delay") == 0)
		{
			sscanf(command, "%s %s %f", non_space_part, equal, &(map_info->step_standard_delay));
		} else if (strcmp(non_space_part, "moratory_duration") == 0)
		{
			sscanf(command, "%s %s %f", non_space_part, equal, &(map_info->moratory_duration));
		} else
		{
			perror("Wrong map info.\n");
			free(command);
			free(non_space_part);
			fclose(file);
			return -1;
		}

		free(non_space_part);
		free(command);
	}

	
	fclose(file);
	return 0;
}

void destroy_map(struct MapInfo* map_info)
{
	int i;

	if (map_info->map)
	{
		for (i = 0; i < map_info->rows; ++i)
		{
			if (map_info->map[i])
			{	
				free(map_info->map[i]);
			}
		}

		free(map_info->map);
	}

	if (map_info->items)
	{
		free(map_info->items);
	}
}

void print_maps_info(struct MapInfo map_info)
{
    int i;
    int j;
    printf("%d rows and %d cols\n\n", map_info.rows, map_info.cols);
    
    printf("The map is:\n");
    for (i = 0; i < map_info.rows; ++i)
    {
        for (j = 0; j < map_info.cols; ++j)
        {
            printf("%c", map_info.map[i][j]);
        }
        printf("\n");
    }
    printf("\n");
    
    printf("Number of empty cells: %d\n\n", map_info.empty_cells);
    
    printf("initial_health: %d\n", map_info.initial_health);
    printf("hit_value: %d\n", map_info.hit_value);
    printf("recharge_duration: %f\n", map_info.recharge_duration);
    printf("mining_time: %f\n", map_info.mining_time);
    printf("stay_health_drop: %d\n", map_info.stay_health_drop);
    printf("movement_health_drop: %d\n", map_info.movement_health_drop);
    printf("step_standard_delay: %f\n", map_info.step_standard_delay);
    printf("moratory_duration: %f\n", map_info.moratory_duration);
    printf("\n");
    
    printf("Number of items: %d\n\n", map_info.number_of_items);
    
    printf("Items are:\n");
    for (i = 0; i < map_info.number_of_items; ++i)
    {
        printf("Item[%d] is: (%d, %d, %d)\n", i + 1, map_info.items[i].x, map_info.items[i].y, map_info.items[i].value);
    }
    printf("\n");
}

#endif /* READ_MAP_H */
