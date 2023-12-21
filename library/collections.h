#ifndef COLLECTIONS
#define COLLECTIONS

typedef struct linked_list_node
{
    void* data;
    struct linked_list_node* next;
} linked_list_node;
typedef struct linked_list
{
    int data_size;
    int count;
    struct linked_list_node* head;
    struct linked_list_node* tail;
} linked_list;

void* stackheapallocate(int size);
void* heapallocate(int size);

struct linked_list* allocate_linked_list(int data_size);
void add_linked_list_value(struct linked_list* list, void* data);
void set_linked_list_value_at(linked_list* list, int index, void* data);
void remove_linked_list_value_at(struct linked_list* list, int index);
void* get_linked_list_data(struct linked_list* list, int index);
void free_linked_list(struct linked_list* list);
void clear_linked_list(linked_list* list);

void add_linked_list_int(linked_list* list, int val);
void set_linked_list_value_int_at(linked_list* list, int index, int value);
int get_linked_list_data_int(linked_list* list, int index);

void add_linked_list_float(linked_list* list, float val);
void set_linked_list_value_float_at(linked_list* list, int index, float value);
float get_linked_list_data_float(linked_list* list, int index);

void add_linked_list_char(linked_list* list, char val);
void set_linked_list_value_char_at(linked_list* list, int index, char value);
char get_linked_list_data_char(linked_list* list, int index);

void add_linked_list_string(linked_list* list, char* val);
void add_linked_list_string_safe(linked_list* list, char* val);
void set_linked_list_value_string_at(linked_list* list, int index, char* value);
void set_linked_list_value_string_at_safe(linked_list* list, int index, char* value);
char* get_linked_list_data_string(linked_list* list, int index);

#endif