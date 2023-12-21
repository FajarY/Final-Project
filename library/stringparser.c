#include "stringparser.h"
#include <malloc.h>
#include <string.h>
#include "collections.h"
#include <stdio.h>

char* copy_string(char* from, int size)
{
    char* string = (char*)malloc(size + 1);
    memcpy(string, from, size);
    *(string + size) = '\0';

    return string;
}

parsedata* parse(char* string, int buffer_size, char* parse_seperator, int seperator_size, char* parse_joinner, int joinner_size)
{
    int index = 0;
    
    linked_list* list = allocate_linked_list(sizeof(parseindex));

    parseindex current;
    current.start = 0;
    current.end = -1;

    char current_join = 0;

    while (index < buffer_size)
    {
        char character = *(string + index);
        if(character == '\0')
        {
            buffer_size = index + 1;
            break;
        }

        if(current_join == 0)
        {
            int found_seperator = 0;
            int found_joinner = 0;

            for(int i = 0; i < seperator_size; i++)
            {
                char seperator = *(parse_seperator + i);

                if(seperator == character)
                {
                    found_seperator = 1;
                    break;
                }
            }
            for(int i = 0; i < joinner_size; i++)
            {
                char joinner = *(parse_joinner + i);

                if(joinner == character)
                {
                    found_joinner = 1;
                    break;
                }
            }

            if(found_joinner)
            {
                current_join = character;
                if(current.start != index)
                {
                    current.end = index;
                    add_linked_list_value(list, &current);
                }

                current.start = index + 1;
                current.end = index;
            }
            else
            {
                if(found_seperator == 1)
                {
                    if(current.start == index)
                    {
                        current.start = index + 1;
                    }
                    else
                    {
                        current.end = index;
                        add_linked_list_value(list, &current);

                        current.start = index + 1;
                        current.end = index;
                    }
                }
            }
        }
        else
        {
            if(character == current_join)
            {
                current.end = index;
                current_join = 0;

                add_linked_list_value(list, &current);

                current.start = index + 1;
                current.end = index;
            }
        }

        index++;
    }

    if(current.start < buffer_size && current.start != index)
    {
        current.end = index;
        add_linked_list_value(list, &current);
    }

    parsedata* data = malloc(sizeof(parsedata));
    data->length = list->count;
    char** char_array = malloc(sizeof(char*) * data->length);
    int* count_array = malloc(sizeof(int) * data->length);
    data->data = char_array;
    data->size = count_array;

    for(int i = 0; i < list->count; i++)
    {
        current = *((parseindex*)get_linked_list_data(list, i));

        int len = current.end - current.start + 1;
        *(count_array + i) = len - 1;

        char* buffer = malloc(len);
        *(buffer + len - 1) = '\0';

        int buffer_index = 0;

        for(int j = 0; j < len - 1; j++)
        {
            *(buffer + j) = *(string + j + current.start);
        }

        *(char_array + i) = buffer;
    }

    free_linked_list(list);

    return data;
}

void free_parsedata(parsedata* data)
{
    for(int i = 0; i < data->length; i++)
    {
        free(data->data[i]);
    }
    free(data->size);
    free(data);
}
