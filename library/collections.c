#include <string.h>
#include "collections.h"
#include <stdio.h>
#include <malloc.h>

//Linked List
void force_exit(const char* message)
{
    perror(message);
    system("pause");
    exit(-1);
}
int minimum(int l, int r)
{
    if(l < r)
    {
        return l;
    }
    return r;
}
void* stackheapallocate(int size)
{
    void* memory = _malloca(size);

    if(memory == NULL)
    {
        force_exit("Failed to allocate stackheap memory!\n");
    }

    return memory;
}
void* heapallocate(int size)
{
    void* memory = malloc(size);

    if(memory == NULL)
    {
        force_exit("Failed to allocate heap memory!\n");
    }

    return memory;
}

struct linked_list* allocate_linked_list(int data_size)
{
    struct linked_list* list = (struct linked_list*)heapallocate(sizeof(struct linked_list));
    list->count = 0;
    list->data_size = data_size;
    list->head = NULL;

    return list;
}

void add_linked_list_value(struct linked_list* list, void* data)
{
    if(list == NULL)
    {
        force_exit("Fatal error, adding a value to a null linked list!\n");
    }
    if(data == NULL)
    {
        force_exit("Fatal error, adding null value to linked list!\n");
    }

    void* copy_data = heapallocate(list->data_size);
    memcpy(copy_data, data, list->data_size);

    struct linked_list_node* node = (struct linked_list_node*)heapallocate(sizeof(struct linked_list_node));
    node->data = copy_data;
    node->next = NULL;

    if(list->count == 0)
    {
        list->head = node;
        list->tail = node;
        list->count = 1;
    }
    else
    {
        list->tail->next = node;
        list->tail = node;
        list->count = list->count + 1;
    }
}

void set_linked_list_value_at(linked_list* list, int index, void* data)
{
    if(list == NULL)
    {
        force_exit("Fatal error, atempting to set value on null list!\n");
    }
    if(index >= list->count || index < 0)
    {
        force_exit("Fatal error, index is out of bounds of linked list set value!\n");
    }

    linked_list_node* start = list->head;
    while (index != 0)
    {
        start = start->next;
        index--;
    }
    
    memcpy(start->data, data, list->data_size);
}

void remove_linked_list_value_at(struct linked_list* list, int index)
{
    if(list == NULL)
    {
        force_exit("Fatal error, atempting to remove value on null list!\n");
    }
    if(index >= list->count || index < 0)
    {
        force_exit("Fatal error, index is out of bounds of linked list remove value!\n");
    }

    linked_list_node* before = NULL;
    linked_list_node* current = list->head;

    while (index != 0)
    {
        before = current;
        current = current->next;

        index--;
    }
    
    list->count = list->count - 1;

    if(before == NULL)
    {
        list->head = current->next;

        if(current->next == NULL)
        {
            list->tail = NULL;
        }
        free(current->data);
        free(current);
        
        return;
    }
    if(current->next == NULL)
    {
        free(current->data);
        free(current);
        before->next = NULL;
        list->tail = before;

        return;
    }

    linked_list_node* next = current->next;
    free(current->data);
    free(current);

    before->next = next;
}

void* get_linked_list_data(struct linked_list* list, int index)
{
    if(list == NULL)
    {
        force_exit("Fatal error, atempting to access null list!\n");
    }
    if(index >= list->count || index < 0)
    {
        force_exit("Fatal error, index is out of bounds of linked list!\n");
    }

    linked_list_node* start = list->head;

    while (index > 0)
    {
        start = start->next;
        index--;
    }
    
    return start->data;
}

void free_linked_list(struct linked_list* list)
{
    if(list == NULL)
    {
        force_exit("Fatal error, atempting to free null list!\n");
    }

    struct linked_list_node* start = list->head;
    while (start != NULL)
    {
        struct linked_list_node* next = start->next;
        free(start->data);
        free(start);

        start = next;
    }
    
    free(list);
}

void clear_linked_list(linked_list* list)
{
    if(list == NULL)
    {
        force_exit("Fatal error, atempting to clear null list!\n");
    }

    linked_list_node* head = list->head;
    while (head != NULL)
    {
        free(head->data);
        
        linked_list_node* next = head->next;

        free(head);

        head = next;
    }
    
    list->count = 0;
    list->head = NULL;
    list->tail = NULL;
}

//int
void add_linked_list_int(linked_list* list, int val)
{
    if(list == NULL)
    {
        force_exit("Fatal error, atempting to access null list!\n");
    }
    if(list->data_size != sizeof(int))
    {
        force_exit("Fatal error, incorrect linked list data size for int!\n");
    }
    add_linked_list_value(list, &val);
}
void set_linked_list_value_int_at(linked_list* list, int index, int value)
{
    if(list == NULL)
    {
        force_exit("Fatal error, atempting to access null list!\n");
    }
    if(list->data_size != sizeof(int))
    {
        force_exit("Fatal error, incorrect linked list data size for int!\n");
    }
    set_linked_list_value_at(list, index, &value);
}
int get_linked_list_data_int(linked_list* list, int index)
{
    void* data = get_linked_list_data(list, index);

    if(list->data_size != sizeof(int))
    {
        force_exit("Fatal error, incorrect linked list data size for int!\n");
    }

    int* parse = (int*)data;

    return *parse;
}

//float
void add_linked_list_float(linked_list* list, float val)
{
    if(list == NULL)
    {
        force_exit("Fatal error, atempting to access null list!\n");
    }
    if(list->data_size != sizeof(float))
    {
        force_exit("Fatal error, incorrect linked list data size for float!\n");
    }
    add_linked_list_value(list, &val);
}
void set_linked_list_value_float_at(linked_list* list, int index, float value)
{
    if(list == NULL)
    {
        force_exit("Fatal error, atempting to access null list!\n");
    }
    if(list->data_size != sizeof(float))
    {
        force_exit("Fatal error, incorrect linked list data size for float!\n");
    }
    set_linked_list_value_at(list, index, &value);
}
float get_linked_list_data_float(linked_list* list, int index)
{
    void* data = get_linked_list_data(list, index);

    if(list->data_size != sizeof(4))
    {
        force_exit("Fatal error, incorrect linked list data size for float!\n");
    }

    float* parse = (float*)data;

    return *parse;
}

//char
void add_linked_list_char(linked_list* list, char val)
{
    if(list == NULL)
    {
        force_exit("Fatal error, atempting to access null list!\n");
    }
    if(list->data_size != sizeof(char))
    {
        force_exit("Fatal error, incorrect linked list data size for char!\n");
    }
    add_linked_list_value(list, &val);
}
void set_linked_list_value_char_at(linked_list* list, int index, char value)
{
    if(list == NULL)
    {
        force_exit("Fatal error, atempting to access null list!\n");
    }
    if(list->data_size != sizeof(char))
    {
        force_exit("Fatal error, incorrect linked list data size for char!\n");
    }
    set_linked_list_value_at(list, index, &value);
}
char get_linked_list_data_char(linked_list* list, int index)
{
    void* data = get_linked_list_data(list, index);

    if(list->data_size != sizeof(char))
    {
        force_exit("Fatal error, incorrect linked list data size for char!\n");
    }

    char* parse = (char*)data;

    return *parse;
}

//string
void add_linked_list_string(linked_list* list, char* val)
{
    if(list == NULL)
    {
        force_exit("Fatal error, atempting to access null list!\n");
    }
    add_linked_list_value(list, val);
}
void add_linked_list_string_safe(linked_list* list, char* val)
{
    if(list == NULL)
    {
        force_exit("Fatal error, atempting to access null list!\n");
    }

    int len = minimum(strlen(val), list->data_size);

    char* buffer = stackheapallocate(list->data_size);

    memset(buffer, 0, list->data_size);
    memcpy(buffer, val, len);
    *(buffer + list->data_size - 1) = '\0';

    add_linked_list_value(list, buffer);

    _freea(buffer);
}
void set_linked_list_value_string_at(linked_list* list, int index, char* value)
{
    if(list == NULL)
    {
        force_exit("Fatal error, atempting to access null list!\n");
    }

    set_linked_list_value_at(list, index, value);
}
void set_linked_list_value_string_at_safe(linked_list* list, int index, char* value)
{
    if(list == NULL)
    {
        force_exit("Fatal error, atempting to access null list!\n");
    }

    int len = minimum(strlen(value), list->data_size);
    char* buffer = stackheapallocate(list->data_size);
    memset(buffer, 0, list->data_size);
    memcpy(buffer, value, len);
    *(buffer + list->data_size - 1) = '\0';

    set_linked_list_value_at(list, index, buffer);

    _freea(buffer);
}
char* get_linked_list_data_string(linked_list* list, int index)
{
    void* data = get_linked_list_data(list, index);

    char* string = (char*)data;

    return string;
}