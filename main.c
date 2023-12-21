#include <stdio.h>
#include <stdlib.h>
#include "collections.h"
#include "stringparser.h"
#include <string.h>
#include <ctype.h>

void error_exit(const char*);

//Default Commands
const char* PRINT_COMMAND = "PRINT";
const char* END_COMMAND = "END";
const char* CREATE_COMMAND = "CREATE";
const char* INSERT_COMMAND = "INSERT";
const char* VALUES_COMMAND = "VALUES";
const char* DISPLAY_COMMAND = "DISPLAY";
const char* DELETE_COMMAND = "DELETE";
const char* UPDATE_COMMAND = "UPDATE";
const char* ALL_COMMAND = "ALL";
const char* FROM_COMMAND = "FROM";
const char* WHERE_COMMAND = "WHERE";
const char* AT_COMMAND = "AT";
const char* SET_COMMAND = "SET";
const char* PEEK_COMMAND = "PEEK";
const char* SCRIPT_COMMAND = "SCRIPT";
const char* CLEAR_COMMAND = "CLEAR";

#define BUFFER_SIZE 1000
enum INPUT_TYPE
{
    EMPTY,
    CREATE,
    INSERT,
    DISPLAY,
    DELETE,
    UPDATE,
    VALUES,
    SET,
    PEEK,
    PRINT,
    END,
    UNKNOWN,
    WHERE,
    SCRIPT,
    CLEAR
};

//Create
enum VARIABLE_TYPE
{
    INT_TYPE,
    CHAR_TYPE,
    FLOAT_TYPE,
    VARCHAR_TYPE,
    UNKNOWN_TYPE
};

const char* INT_COMMAND = "INT";
const char* CHAR_COMMAND = "CHAR";
const char* FLOAT_COMMAND = "FLOAT";
const char* VARCHAR_COMMAND = "VARCHAR";
const char* PRIMARY_COMMAND = "PRIMARY";
const char* FOREIGN_COMMAND = "FOREIGN";
const char* PRIMARY_FOREIGN_COMMAND = "PRIMARY/FOREIGN";

typedef struct TABLE_ITEM
{
    int is_primary;
    int is_foreign;
    char* foreign_target_table;
    char* foreign_target_column;
    char* name;
    enum VARIABLE_TYPE type;
    int data_size;
    linked_list* rows;
} TABLE_ITEM;

typedef struct TABLE_DECLARATION
{
    char* name;
    linked_list* columns;
    int row_count;
} TABLE_DECLARATION;

typedef struct LOGIC_ITEM
{
    char* column_name;
    int logic_type;
    char* data;
    int data_size;
} LOGIC_ITEM;

typedef struct UPDATE_ITEM
{
    int column_index;
    char* data;
    int data_size;
} UPDATE_ITEM;

void free_logic(LOGIC_ITEM* item)
{
    free(item->column_name);
    free(item->data);
    free(item);
}

void free_table_declaration(TABLE_DECLARATION* table)
{
    for(int i = 0; i < table->columns->count; i++)
    {
        TABLE_ITEM* item = *((TABLE_ITEM**)get_linked_list_data(table->columns, i));
        free(item->foreign_target_table);
        free(item->foreign_target_column);
        free(item->name);

        free_linked_list(item->rows);
        free(item);
    }

    free_linked_list(table->columns);
    free(table->name);
    free(table);
}
void clear_all_table(linked_list* list)
{
    for(int i = 0; i < list->count; i++)
    {
        TABLE_DECLARATION* table = *((TABLE_DECLARATION**)get_linked_list_data(list, i));
        free_table_declaration(table);
    }

    clear_linked_list(list);
}

TABLE_ITEM* create_table_item(enum VARIABLE_TYPE type, char* name, int data_size)
{
    TABLE_ITEM* column = heapallocate(sizeof(TABLE_ITEM));
    column->type = type;
    column->name = name;
    column->is_primary = 0;
    column->is_foreign = 0;
    column->foreign_target_table = NULL;
    column->foreign_target_table = NULL;

    if(type == VARCHAR_TYPE)
    {
        if(data_size == 0)
        {
            error_exit("Fatal error when creating table item, data size for varchar is too small : 0\n");
        }
        column->rows = allocate_linked_list(data_size + 1);
        column->data_size = data_size;
    }
    else if(type == INT_TYPE)
    {
        column->rows = allocate_linked_list(sizeof(int));
        column->data_size = column->rows->data_size;
    }
    else if(type == CHAR_TYPE)
    {
        column->rows = allocate_linked_list(sizeof(char));
        column->data_size = column->rows->data_size;
    }
    else if(type == FLOAT_TYPE)
    {
        column->rows = allocate_linked_list(sizeof(float));
        column->data_size = column->rows->data_size;
    }
    else
    {
        error_exit("Fatal error when creating table item, type is unknown\n");
    }

    return column;
}

int get_column_index(TABLE_DECLARATION* table, const char* name)
{
    for(int i = 0; i < table->columns->count; i++)
    {
        TABLE_ITEM* column = *((TABLE_ITEM**)get_linked_list_data(table->columns, i));

        if(strcmp(column->name, name) == 0)
        {
            return i;
        }
    }

    return -1;
}
int get_table_index(linked_list* list, const char* name)
{
    for(int i = 0; i < list->count; i++)
    {
        TABLE_DECLARATION* table = *((TABLE_DECLARATION**)get_linked_list_data(list, i));

        if(strcmp(name, table->name) == 0)
        {
            return i;
        }
    }
    
    return -1;
}

TABLE_DECLARATION* create_table_declaration(char* name)
{
    TABLE_DECLARATION* table = heapallocate(sizeof(TABLE_DECLARATION));
    table->name = name;
    table->columns = allocate_linked_list(sizeof(TABLE_ITEM*));
    table->row_count = 0;

    return table;
}

void add_table_item(linked_list* tables, TABLE_DECLARATION* declaration, TABLE_ITEM* item, int is_primary, int is_foreign, char* foreign_target_table, char* foreign_target_column)
{
    for(int i = 0; i < declaration->columns->count; i++)
    {
        TABLE_ITEM* current = *((TABLE_ITEM**)get_linked_list_data(declaration->columns, i));
        if(strcmp(current->name, item->name) == 0)
        {
            printf("Fatal error when adding column : %s, in table %s\n", item->name, declaration->name);
            error_exit("Fatal error when adding column, a column with the same name already exist\n");
        }
    }

    void* buffer = heapallocate(item->data_size);
    for(int i = 0; i < declaration->row_count; i++)
    {
        add_linked_list_value(item->rows, buffer);
    }

    item->is_primary = is_primary;
    item->is_foreign = is_foreign;
    item->foreign_target_table = foreign_target_table;
    item->foreign_target_column = foreign_target_column;
    
    if(is_primary)
    {
        if(declaration->row_count > 0)
        {
            printf("Fatal error when adding column : %s, in table %s\n", item->name, declaration->name);
            error_exit("Fatal error when adding column, column cannot be made primary since row count is already higher than 0\n");
        }
    }
    if(is_foreign)
    {
        int table_index = get_table_index(tables, foreign_target_table);
        if(table_index == -1)
        {
            printf("Fatal error when adding column : %s, in table %s, foreign %s\n", item->name, declaration->name, foreign_target_table);
            error_exit("Fatal error when adding column, column cannot be made foreign because there is no table with such foreign name\n");
        }
        int column_index = get_column_index(*((TABLE_DECLARATION**)get_linked_list_data(tables, table_index)), foreign_target_column);
        if(column_index == -1)
        {
            printf("Fatal error when adding column : %s, in table %s, foreign %s column foreign %s\n", item->name, declaration->name, foreign_target_table, foreign_target_column);
            error_exit("Fatal error when adding column, column cannot be made foreign because there is no column with such foreign name in foreign table\n");
        }

        TABLE_DECLARATION* foreign_table = *((TABLE_DECLARATION**)get_linked_list_data(tables, table_index));
        TABLE_ITEM* foreign_column = *((TABLE_ITEM**)get_linked_list_data(foreign_table->columns, column_index));

        if(foreign_column->type != item->type || foreign_column->data_size != item->data_size)
        {
            printf("Fatal error when adding column : %s, in table %s, foreign %s column foreign %s\n", item->name, declaration->name, foreign_target_table, foreign_target_column);
            error_exit("Fatal error when adding column, incorect data type to foreign other table\n");
        }
        if(foreign_column->is_primary == 0)
        {
            printf("Fatal error when adding column : %s, in table %s, foreign %s column foreign %s\n", item->name, declaration->name, foreign_target_table, foreign_target_column);
            error_exit("Fatal error when adding column, foreign target is not a primary atribute\n");
        }

        if(declaration->row_count > 0)
        {
            printf("Fatal error when adding column : %s, in table %s\n", item->name, declaration->name);
            error_exit("Fatal error when adding column, column cannot be made foreign since row count is already higher than 0\n");
        }
    }

    add_linked_list_value(declaration->columns, &item);
}

void delete_table_row(TABLE_DECLARATION* table, int row)
{
    if(row >= table->row_count || row < 0)
    {
        printf("Fatal error when deleting row : %i, in table %s\n", row, table->name);
        error_exit("Fatal error when deleting row, index is outside bounds of the rows\n");
    }

    for(int i = 0; i < table->columns->count; i++)
    {
        TABLE_ITEM* table_item = *((TABLE_ITEM**)get_linked_list_data(table->columns, i));
        remove_linked_list_value_at(table_item->rows, row);
    }

    table->row_count -= 1;
}

LOGIC_ITEM* create_logic_item(TABLE_DECLARATION* table, char* column, char* logic, int logic_size, char* data, int data_size)
{
    int found_column = 0;
    for(int i = 0; i < table->columns->count; i++)
    {
        TABLE_ITEM* table_item = *((TABLE_ITEM**)get_linked_list_data(table->columns, i));
        if(strcmp(table_item->name, column) == 0)
        {
            found_column = 1;
            break;
        }
    }
    if(!found_column)
    {
        printf("Fatal error when creating logic item, cannot find column %s on table %s\n", column, table->name);
        error_exit("Fatal error when creating logic item, cannot find associated column in table\n");
    }
    if(logic_size > 2)
    {
        printf("Fatal error when creating logic item, syntax error for logic comparator %s\n", logic);
        error_exit("Fatal error when creating logic item, sytax error on comparator\n");
    }

    int logic_type = 0;
    char first_character = *(logic);
    if(first_character == '=' && logic_size == 1)
    {
        logic_type = 0;
    }
    else if(first_character == '>')
    {
        logic_type = 1;
        if(logic_size == 2)
        {
            if(*(logic + 1) == '=')
            {
                logic_type = 2;
            }
            else
            {
                printf("Fatal error when creating logic item, syntax error for logic comparator %s\n", logic);
                error_exit("Fatal error when creating logic item, sytax error on comparator\n");
            }
        }
    }
    else if(first_character == '<')
    {
        logic_type = 3;
        if(logic_size == 2)
        {
            if(*(logic + 1) == '=')
            {
                logic_type = 4;
            }
            else
            {
                printf("Fatal error when creating logic item, syntax error for logic comparator %s\n", logic);
                error_exit("Fatal error when creating logic item, sytax error on comparator\n");
            }
        }
    }
    else if(first_character == '!')
    {
        if(logic_size == 2 && *(logic + 1) == '=')
        {
            logic_type = 5;
        }
        else
        {
            printf("Fatal error when creating logic item, syntax error for logic comparator %s\n", logic);
            error_exit("Fatal error when creating logic item, sytax error on comparator\n");
        }
    }
    else
    {
        printf("Fatal error when creating logic item, syntax error for logic comparator %s\n", logic);
        error_exit("Fatal error when creating logic item, sytax error on comparator\n");
    }

    LOGIC_ITEM* item = heapallocate(sizeof(LOGIC_ITEM));

    int column_len = strlen(column);

    item->column_name = copy_string(column, column_len);
    item->data = copy_string(data, data_size);
    item->data_size = data_size;
    item->logic_type = logic_type;

    return item;
}

int is_true_on_logic(LOGIC_ITEM* logic, TABLE_DECLARATION* table, int index)
{
    int is_true = 1;
    char* dummy;

    for(int i = 0; i < table->columns->count; i++)
    {
        TABLE_ITEM* table_item = *((TABLE_ITEM**)get_linked_list_data(table->columns, i));

        if(strcmp(table_item->name, logic->column_name) != 0)
        {
            continue;
        }

        if(table_item->type == INT_TYPE)
        {
            int logic_data = strtol(logic->data, &dummy, 10);
            int table_data = get_linked_list_data_int(table_item->rows, index);

            if(logic->logic_type == 0)
            {
                if(!(logic_data == table_data))
                {
                    is_true = 0;
                    break;
                }
            }
            else if(logic->logic_type == 1)
            {
                if(!(table_data > logic_data))
                {
                    is_true = 0;
                    break;
                }
            }
            else if(logic->logic_type == 2)
            {
                if(!(table_data >= logic_data))
                {
                    is_true = 0;
                    break;
                }
            }
            else if(logic->logic_type == 3)
            {
                if(!(table_data < logic_data))
                {
                    is_true = 0;
                    break;
                }
            }
            else if(logic->logic_type == 4)
            {
                if(!(table_data <= logic_data))
                {
                    is_true = 0;
                    break;
                }
            }
            else if(logic->logic_type == 5)
            {
                if(!(table_data != logic_data))
                {
                    is_true = 0;
                    break;
                }
            }
        }
        else if(table_item->type == FLOAT_TYPE)
        {
            float logic_data = strtof(logic->data, &dummy);
            float table_data = get_linked_list_data_float(table_item->rows, index);

            if(logic->logic_type == 0)
            {
                if(!(logic_data == table_data))
                {
                    is_true = 0;
                    break;
                }
            }
            else if(logic->logic_type == 1)
            {
                if(!(table_data > logic_data))
                {
                    is_true = 0;
                    break;
                }
            }
            else if(logic->logic_type == 2)
            {
                if(!(table_data >= logic_data))
                {
                    is_true = 0;
                    break;
                }
            }
            else if(logic->logic_type == 3)
            {
                if(!(table_data < logic_data))
                {
                    is_true = 0;
                    break;
                }
            }
            else if(logic->logic_type == 4)
            {
                if(!(table_data <= logic_data))
                {
                    is_true = 0;
                    break;
                }
            }
            else if(logic->logic_type == 5)
            {
                if(!(table_data != logic_data))
                {
                    is_true = 0;
                    break;
                }
            }
        }
        else if(table_item->type == CHAR_TYPE)
        {
            char logic_data = ' ';
            if(logic->data_size > 0)
            {
                logic_data = *(logic->data);
            }
            char table_data = get_linked_list_data_char(table_item->rows, index);

            if(logic->logic_type == 0)
            {
                if(!(logic_data == table_data))
                {
                    is_true = 0;
                    break;
                }
            }
            else if(logic->logic_type == 1)
            {
                if(!(table_data > logic_data))
                {
                    is_true = 0;
                    break;
                }
            }
            else if(logic->logic_type == 2)
            {
                if(!(table_data >= logic_data))
                {
                    is_true = 0;
                    break;
                }
            }
            else if(logic->logic_type == 3)
            {
                if(!(table_data < logic_data))
                {
                    is_true = 0;
                    break;
                }
            }
            else if(logic->logic_type == 4)
            {
                if(!(table_data <= logic_data))
                {
                    is_true = 0;
                    break;
                }
            }
            else if(logic->logic_type == 5)
            {
                if(!(table_data != logic_data))
                {
                    is_true = 0;
                    break;
                }
            }
        }
        else if(table_item->type == VARCHAR_TYPE)
        {
            char* logic_data = logic->data;
            char* table_data = get_linked_list_data_string(table_item->rows, index);

            if(logic->logic_type == 0)
            {
                if(!(strcmp(table_data, logic_data) == 0))
                {
                    is_true = 0;
                    break;
                }
            }
            else if(logic->logic_type == 1)
            {
                if(!(strcmp(table_data, logic_data) > 0))
                {
                    is_true = 0;
                    break;
                }
            }
            else if(logic->logic_type == 2)
            {
                if(!(strcmp(table_data, logic_data) >= 0))
                {
                    is_true = 0;
                    break;
                }
            }
            else if(logic->logic_type == 3)
            {
                if(!(strcmp(table_data, logic_data) < 0))
                {
                    is_true = 0;
                    break;
                }
            }
            else if(logic->logic_type == 4)
            {
                if(!(strcmp(table_data, logic_data) <= 0))
                {
                    is_true = 0;
                    break;
                }
            }
            else if(logic->logic_type == 5)
            {
                if(!(strcmp(table_data, logic_data) != 0))
                {
                    is_true = 0;
                    break;
                }
            }
        }
    }

    return is_true;
}

int is_true_on_logics(linked_list* logic_list, TABLE_DECLARATION* table, int index)
{
    int is_true = 1;

    for(int i = 0; i < logic_list->count; i++)
    {
        LOGIC_ITEM* logic_item = *((LOGIC_ITEM**)get_linked_list_data(logic_list, i));
        if(!is_true_on_logic(logic_item, table, index))
        {
            is_true = 0;
            break;
        }
    }
    
    return is_true;
}

enum INPUT_TYPE get_command_type(const char* command)
{
    if(strcmp(command, PRINT_COMMAND) == 0)
    {
        return PRINT;
    }
    else if(strcmp(command, END_COMMAND) == 0)
    {
        return END;
    }
    else if(strcmp(command, CREATE_COMMAND) == 0)
    {
        return CREATE;
    }
    else if(strcmp(command, INSERT_COMMAND) == 0)
    {
        return INSERT;
    }
    else if(strcmp(command, VALUES_COMMAND) == 0)
    {
        return VALUES;
    }
    else if(strcmp(command, DISPLAY_COMMAND) == 0)
    {
        return DISPLAY;
    }
    else if(strcmp(command, WHERE_COMMAND) == 0)
    {
        return WHERE;
    }
    else if(strcmp(command, DELETE_COMMAND) == 0)
    {
        return DELETE;
    }
    else if(strcmp(command, UPDATE_COMMAND) == 0)
    {
        return UPDATE;
    }
    else if(strcmp(command, SET_COMMAND) == 0)
    {
        return SET;
    }
    else if(strcmp(command, PEEK_COMMAND) == 0)
    {
        return PEEK;
    }
    else if(strcmp(command, SCRIPT_COMMAND) == 0)
    {
        return SCRIPT;
    }
    else if(strcmp(command, CLEAR_COMMAND) == 0)
    {
        return CLEAR;
    }

    return UNKNOWN;
}

enum VARIABLE_TYPE get_variable_type(const char* variable_type)
{
    if(strcmp(variable_type, INT_COMMAND) == 0)
    {
        return INT_TYPE;
    }
    else if(strcmp(variable_type, CHAR_COMMAND) == 0)
    {
        return CHAR_TYPE;
    }
    else if(strcmp(variable_type, FLOAT_COMMAND) == 0)
    {
        return FLOAT_TYPE;
    }
    else if(strcmp(variable_type, VARCHAR_COMMAND) == 0)
    {
        return VARCHAR_TYPE;
    }

    return UNKNOWN_TYPE;
}

int is_referenced_foreign_to_index(linked_list* tables, TABLE_DECLARATION* table, int index)
{
    for(int i = 0; i < table->columns->count; i++)
    {
        TABLE_ITEM* item = *((TABLE_ITEM**)get_linked_list_data(table->columns, i));
        if(!item->is_primary)
        {
            continue;
        }
        for(int j = 0; j < tables->count; j++)
        {
            TABLE_DECLARATION* other_declaration = *((TABLE_DECLARATION**)get_linked_list_data(tables, j));
            if(strcmp(other_declaration->name, table->name) == 0)
            {
                continue;
            }

            for(int k = 0; k < other_declaration->columns->count; k++)
            {
                TABLE_ITEM* other_item = *((TABLE_ITEM**)get_linked_list_data(other_declaration->columns, k));

                if(!other_item->is_foreign)
                {
                    continue;
                }
                if(strcmp(other_item->foreign_target_table, table->name) == 0 && strcmp(other_item->foreign_target_column, item->name) == 0)
                {
                    if(item->type == INT_TYPE)
                    {
                        int item_data = get_linked_list_data_int(item->rows, index);
                        for(int w = 0; w < other_item->rows->count; w++)
                        {
                            int other_data = get_linked_list_data_int(other_item->rows, w);

                            if(other_data == item_data)
                            {
                                return 1;
                            }
                        }
                    }
                    else if(item->type == FLOAT_TYPE)
                    {
                        float item_data = get_linked_list_data_float(item->rows, index);
                        for(int w = 0; w < other_item->rows->count; w++)
                        {
                            float other_data = get_linked_list_data_float(other_item->rows, w);

                            if(other_data == item_data)
                            {
                                return 1;
                            }
                        }
                    }
                    else if(item->type == CHAR_TYPE)
                    {
                        char item_data = get_linked_list_data_char(item->rows, index);
                        for(int w = 0; w < other_item->rows->count; w++)
                        {
                            char other_data = get_linked_list_data_char(other_item->rows, w);

                            if(other_data == item_data)
                            {
                                return 1;
                            }
                        }
                    }
                    else if(item->type == VARCHAR_TYPE)
                    {
                        char* item_data = get_linked_list_data_string(item->rows, index);
                        for(int w = 0; w < other_item->rows->count; w++)
                        {
                            char* other_data = get_linked_list_data_string(other_item->rows, w);

                            if(strcmp(item_data, other_data) == 0)
                            {
                                return 1;
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}

int is_referenced_foreign_to_index_update(linked_list* tables, TABLE_DECLARATION* table, int index, linked_list* update_items)
{
    for(int i = 0; i < table->columns->count; i++)
    {
        TABLE_ITEM* item = *((TABLE_ITEM**)get_linked_list_data(table->columns, i));
        if(!item->is_primary)
        {
            continue;
        }

        int found_ref = 0;
        for(int j = 0; j < update_items->count; j++)
        {
            UPDATE_ITEM* update_item = *((UPDATE_ITEM**)get_linked_list_data(update_items, j));

            if(update_item->column_index == i)
            {
                found_ref = 1;
            }
        }
        if(found_ref == 0)
        {
            continue;
        }
        for(int j = 0; j < tables->count; j++)
        {
            TABLE_DECLARATION* other_declaration = *((TABLE_DECLARATION**)get_linked_list_data(tables, j));
            if(strcmp(other_declaration->name, table->name) == 0)
            {
                continue;
            }

            for(int k = 0; k < other_declaration->columns->count; k++)
            {
                TABLE_ITEM* other_item = *((TABLE_ITEM**)get_linked_list_data(other_declaration->columns, k));

                if(!other_item->is_foreign)
                {
                    continue;
                }
                if(strcmp(other_item->foreign_target_table, table->name) == 0 && strcmp(other_item->foreign_target_column, item->name) == 0)
                {
                    if(item->type == INT_TYPE)
                    {
                        int item_data = get_linked_list_data_int(item->rows, index);
                        for(int w = 0; w < other_item->rows->count; w++)
                        {
                            int other_data = get_linked_list_data_int(other_item->rows, w);

                            if(other_data == item_data)
                            {
                                return 1;
                            }
                        }
                    }
                    else if(item->type == FLOAT_TYPE)
                    {
                        float item_data = get_linked_list_data_float(item->rows, index);
                        for(int w = 0; w < other_item->rows->count; w++)
                        {
                            float other_data = get_linked_list_data_float(other_item->rows, w);

                            if(other_data == item_data)
                            {
                                return 1;
                            }
                        }
                    }
                    else if(item->type == CHAR_TYPE)
                    {
                        char item_data = get_linked_list_data_char(item->rows, index);
                        for(int w = 0; w < other_item->rows->count; w++)
                        {
                            char other_data = get_linked_list_data_char(other_item->rows, w);

                            if(other_data == item_data)
                            {
                                return 1;
                            }
                        }
                    }
                    else if(item->type == VARCHAR_TYPE)
                    {
                        char* item_data = get_linked_list_data_string(item->rows, index);
                        for(int w = 0; w < other_item->rows->count; w++)
                        {
                            char* other_data = get_linked_list_data_string(other_item->rows, w);

                            if(strcmp(item_data, other_data) == 0)
                            {
                                return 1;
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}

int is_primary_referenced_on_table(TABLE_DECLARATION* table, char** data, int* data_size)
{
    char* dummy;
    for(int i = 0; i < table->row_count; i++)
    {
        int primary_count = 0;
        int primary_check = 0;
        for(int j = 0; j < table->columns->count; j++)
        {
            char* raw_data = *(data + j);
            TABLE_ITEM* table_item = *((TABLE_ITEM**)get_linked_list_data(table->columns, j));

            if(!table_item->is_primary)
            {
                continue;
            }
            primary_count += 1;

            if(table_item->type == INT_TYPE)
            {
                int check_data = strtol(raw_data, &dummy, 10);
                int item_data = get_linked_list_data_int(table_item->rows, i);

                if(check_data == item_data)
                {
                    primary_check += 1;
                }
            }
            else if(table_item->type == FLOAT_TYPE)
            {
                float check_data = strtof(raw_data, &dummy);
                float item_data = get_linked_list_data_float(table_item->rows, i);

                if(check_data == item_data)
                {
                    primary_check += 1;
                }
            }
            else if(table_item->type == CHAR_TYPE)
            {
                char check_data = ' ';
                if(*(data_size + j) > 0)
                {
                    check_data = *(raw_data);
                }
                char item_data = get_linked_list_data_char(table_item->rows, i);

                if(check_data == item_data)
                {
                    primary_check += 1;
                }
            }
            else if(table_item->type == VARCHAR_TYPE)
            {
                char* check_data = raw_data;
                char* item_data = get_linked_list_data_string(table_item->rows, i);

                if(strcmp(check_data, item_data) == 0)
                {
                    primary_check += 1;
                }
            }
        }

        if(primary_count != 0 && primary_count == primary_check)
        {
            return 1;
        }
    }

    return 0;
}

int is_primary_referenced_on_update(TABLE_DECLARATION* table, linked_list* update_items, int index)
{
    char* dummy;
    for(int i = 0; i < table->row_count; i++)
    {
        if(i == index)
        {
            continue;
        }

        int primary_count = 0;
        int primary_check = 0;

        for(int j = 0; j < table->columns->count; j++)
        {
            char* raw_data = NULL;
            int raw_size;
            TABLE_ITEM* table_item = *((TABLE_ITEM**)get_linked_list_data(table->columns, j));

            if(!table_item->is_primary)
            {
                continue;
            }
            
            int use_raw = 0;

            for(int k = 0; k < update_items->count; k++)
            {
                UPDATE_ITEM* current_update = *((UPDATE_ITEM**)get_linked_list_data(update_items, k));
                if(current_update->column_index == j)
                {
                    use_raw = 1;
                    raw_data = current_update->data;
                    raw_size = current_update->data_size;
                }
            }

            primary_count += 1;

            if(table_item->type == INT_TYPE)
            {
                int check_data;
                if(use_raw)
                {
                    check_data = strtol(raw_data, &dummy, 10);
                }
                else
                {
                    check_data = get_linked_list_data_int(table_item->rows, index);
                }
                int item_data = get_linked_list_data_int(table_item->rows, i);

                if(check_data == item_data)
                {
                    primary_check += 1;
                }
            }
            else if(table_item->type == FLOAT_TYPE)
            {
                float check_data;
                if(use_raw)
                {
                    check_data = strtof(raw_data, &dummy);
                }
                else
                {
                    check_data = get_linked_list_data_float(table_item->rows, index);
                }
                float item_data = get_linked_list_data_float(table_item->rows, i);

                if(check_data == item_data)
                {
                    primary_check += 1;
                }
            }
            else if(table_item->type == CHAR_TYPE)
            {
                char check_data = ' ';
                if(use_raw)
                {
                    if(raw_size > 0)
                    {
                        check_data = *(raw_data);
                    }
                }
                else
                {
                    check_data = get_linked_list_data_char(table_item->rows, index);
                }
                char item_data = get_linked_list_data_char(table_item->rows, i);

                if(check_data == item_data)
                {
                    primary_check += 1;
                }
            }
            else if(table_item->type == VARCHAR_TYPE)
            {
                char* check_data;
                if(use_raw)
                {
                    check_data = raw_data;
                }
                else
                {
                    check_data = get_linked_list_data_string(table_item->rows, index);
                }
                char* item_data = get_linked_list_data_string(table_item->rows, i);

                if(strcmp(check_data, item_data) == 0)
                {
                    primary_check += 1;
                }
            }
        }

        if(primary_count != 0 && primary_count == primary_check)
        {
            return 1;
        }
    }

    return 0;
}

int is_foreign_data_valid(linked_list* tables, TABLE_DECLARATION* table, char** data, int* data_size)
{
    char* dummy;
    for(int i = 0; i < table->columns->count; i++)
    {
        TABLE_ITEM* table_item = *((TABLE_ITEM**)get_linked_list_data(tables, i));
        char* raw_data = *(data + i);
        int raw_size = *(data_size + i);

        if(!table_item->is_foreign)
        {
            continue;
        }

        int target_table = get_table_index(tables, table_item->foreign_target_table);
        TABLE_DECLARATION* table_declaration = *((TABLE_DECLARATION**)get_linked_list_data(tables, target_table));
        int target_column = get_column_index(table_declaration, table_item->foreign_target_column);
        TABLE_ITEM* other_item = *((TABLE_ITEM**)get_linked_list_data(table_declaration->columns, target_column));

        int found_foreign = 0;
        if(table_item->type == INT_TYPE)
        {
            int item_data = strtol(raw_data, &dummy, 10);
            for(int j = 0; j < table_declaration->row_count; j++)
            {
                int other_data = get_linked_list_data_int(other_item->rows, j);
                if(other_data == item_data)
                {
                    found_foreign = 1;
                }
            }
        }
        else if(table_item->type == FLOAT_TYPE)
        {
            float item_data = strtof(raw_data, &dummy);
            for(int j = 0; j < table_declaration->row_count; j++)
            {
                float other_data = get_linked_list_data_float(other_item->rows, j);
                if(other_data == item_data)
                {
                    found_foreign = 1;
                }
            }
        }
        else if(table_item->type == CHAR_TYPE)
        {
            char item_data = ' ';
            if(raw_size > 0)
            {
                item_data = *(raw_data);
            }
            for(int j = 0; j < table_declaration->row_count; j++)
            {
                char other_data = get_linked_list_data_char(other_item->rows, j);
                if(other_data == item_data)
                {
                    found_foreign = 1;
                }
            }
        }
        else if(table_item->type = VARCHAR_TYPE)
        {
            char* item_data = raw_data;
            for(int j = 0; j < table_declaration->row_count; j++)
            {
                char* other_data = get_linked_list_data_string(other_item->rows, j);
                if(strcmp(other_data, item_data) == 0)
                {
                    found_foreign = 1;
                }
            }
        }

        if(found_foreign == 0)
        {
            return 0;
        }
    }

    return 1;
}

int is_foreign_data_valid_on_update(linked_list* tables, TABLE_DECLARATION* table, linked_list* update_items, int index)
{
    char* dummy;
    for(int i = 0; i < table->columns->count; i++)
    {
        TABLE_ITEM* table_item = *((TABLE_ITEM**)get_linked_list_data(table->columns, i));
        char* raw_data = NULL;
        int raw_size;
        int use_raw = 0;

        if(!table_item->is_foreign)
        {
            continue;
        }

        int target_table = get_table_index(tables, table_item->foreign_target_table);
        TABLE_DECLARATION* table_declaration = *((TABLE_DECLARATION**)get_linked_list_data(tables, target_table));
        int target_column = get_column_index(table_declaration, table_item->foreign_target_column);
        TABLE_ITEM* other_item = *((TABLE_ITEM**)get_linked_list_data(table_declaration->columns, target_column));

        for(int j = 0; j < update_items->count; j++)
        {
            UPDATE_ITEM* current_item = *((UPDATE_ITEM**)get_linked_list_data(update_items, j));
            if(current_item->column_index == i)
            {
                use_raw = 1;
                raw_data = current_item->data;
                raw_size = current_item->data_size;
            }
        }

        int found_foreign = 0;
        if(table_item->type == INT_TYPE)
        {
            int item_data;
            if(use_raw)
            {
                item_data = strtol(raw_data, &dummy, 10);
            }
            else
            {
                item_data = get_linked_list_data_int(table_item->rows, index);
            }

            for(int j = 0; j < table_declaration->row_count; j++)
            {
                int other_data = get_linked_list_data_int(other_item->rows, j);
                if(other_data == item_data)
                {
                    found_foreign = 1;
                }
            }
        }
        else if(table_item->type == FLOAT_TYPE)
        {
            float item_data;
            if(use_raw)
            {
                item_data = strtof(raw_data, &dummy);
            }
            else
            {
                item_data = get_linked_list_data_float(table_item->rows, index);
            }

            for(int j = 0; j < table_declaration->row_count; j++)
            {
                float other_data = get_linked_list_data_float(other_item->rows, j);
                if(other_data == item_data)
                {
                    found_foreign = 1;
                }
            }
        }
        else if(table_item->type == CHAR_TYPE)
        {
            char item_data = ' ';
            if(use_raw)
            {
                if(raw_size > 0)
                {
                    item_data = *(raw_data);
                }
            }
            else
            {
                item_data = get_linked_list_data_char(table_item->rows, index);
            }

            for(int j = 0; j < table_declaration->row_count; j++)
            {
                char other_data = get_linked_list_data_char(other_item->rows, j);
                if(other_data == item_data)
                {
                    found_foreign = 1;
                }
            }
        }
        else if(table_item->type = VARCHAR_TYPE)
        {
            char* item_data = raw_data;
            if(!use_raw)
            {
                item_data = get_linked_list_data_string(table_item->rows, index);
            }
            for(int j = 0; j < table_declaration->row_count; j++)
            {
                char* other_data = get_linked_list_data_string(other_item->rows, j);
                if(strcmp(other_data, item_data) == 0)
                {
                    found_foreign = 1;
                }
            }
        }

        if(found_foreign == 0)
        {
            return 0;
        }
    }

    return 1;
}

void update_table_column_at(linked_list* tables, TABLE_DECLARATION* update_table, linked_list* update_items, int index)
{
    char* dummy;
    if(is_referenced_foreign_to_index_update(tables, update_table, index, update_items))
    {
        error_exit("Fatal error when updating column, row is foreign referenced\n");
    }
    if(is_primary_referenced_on_update(update_table, update_items, index))
    {
        error_exit("Fatal error when updating column, the same column with the same primary keys is detected\n");
    }
    if(!is_foreign_data_valid_on_update(tables, update_table, update_items, index))
    {
        error_exit("Fatal error when updating column, cannot find reference foreign key on target\n");
    }

    for(int i = 0; i < update_items->count; i++)
    {
        UPDATE_ITEM* current_update = *((UPDATE_ITEM**)get_linked_list_data(update_items, i));
        TABLE_ITEM* update_column = *((TABLE_ITEM**)get_linked_list_data(update_table->columns, current_update->column_index));

        if(update_column->type == INT_TYPE)
        {
            int val = strtol(current_update->data, &dummy, 10);
            set_linked_list_value_int_at(update_column->rows, index, val);
        }
        else if(update_column->type == FLOAT_TYPE)
        {
            float val = strtof(current_update->data, &dummy);
            set_linked_list_value_float_at(update_column->rows, index, val);
        }
        else if(update_column->type == CHAR_TYPE)
        {
            char val = ' ';
            if(update_column->data_size > 0)
            {
                val = *(current_update->data);
            }
            set_linked_list_value_char_at(update_column->rows, index, val);
        }
        else if(update_column->type == VARCHAR_TYPE)
        {
            char* val = current_update->data;
            set_linked_list_value_string_at_safe(update_column->rows, index, val);
        }
    }
}

FILE* script = NULL;
void run()
{
    char* buffer = heapallocate(BUFFER_SIZE);
    char* dummy = heapallocate(5);

    enum INPUT_TYPE input_type = EMPTY;

    int loop = 1;
    int error = 0;
    const char* error_message = "";

    //Logics
    linked_list* logics = allocate_linked_list(sizeof(LOGIC_ITEM*));

    //Tables
    linked_list* tables = allocate_linked_list(sizeof(TABLE_DECLARATION*));

    //Create
    TABLE_DECLARATION* create_table = NULL;

    //Insert
    TABLE_DECLARATION* insert_table = NULL;

    //Display
    linked_list* display_column = allocate_linked_list(sizeof(char*));
    TABLE_DECLARATION* display_table = NULL;

    //Delete
    TABLE_DECLARATION* delete_table_where = NULL;

    //Update
    TABLE_DECLARATION* update_table = NULL;
    linked_list* update_item = allocate_linked_list(sizeof(UPDATE_ITEM*));
    int update_table_at = -1;

    while (loop)
    {
        memset(buffer, 0, BUFFER_SIZE);

        if(script != NULL)
        {
            if(fscanf_s(script, "%[^\n]", buffer, BUFFER_SIZE) != EOF)
            {
                fscanf(script, "%c", dummy);
            }
            else
            {
                fclose(script);
                script = NULL;

                printf("Finished reading script\n");
            }
        }
        else
        {
            scanf_s("%[^\n]", buffer, BUFFER_SIZE);
            scanf("%c", dummy);
        }

        parsedata* tokens = parse(buffer, BUFFER_SIZE, " \t", 2, "\"", 1);

        if(input_type == EMPTY)
        {
            if(tokens->length == 0)
            {
                goto pass;
            }
            
            char* command = tokens->data[0];
            
            enum INPUT_TYPE command_type = get_command_type(command);
            if(command_type == PRINT)
            {
                if(tokens->length != 2)
                {
                    error = 1;
                    error_message = "Syntax error for command : PRINT\n";
                    goto pass;
                }
                printf("%s\n", tokens->data[1]);
            }
            else if(command_type == CREATE)
            {
                if(tokens->length != 2)
                {
                    error = 1;
                    error_message = "Syntax error for command : CREATE\n";
                    goto pass;
                }
                input_type = CREATE;

                int name_len = tokens->size[1];
                if(name_len == 0 || name_len > 255)
                {
                    error = 1;
                    printf("Error when creating table, name length must atleast be 1 and no more than 255 : %i", name_len);
                    error_message = "Error when creating table, incorrect name count\n";
                    goto pass;
                }
                if(get_table_index(tables, tokens->data[1]) >= 0)
                {
                    error = 1;
                    printf("Error when creating table %s\n", tokens->data[1]);
                    error_message = "Error when creating table, the same name is already exist\n";
                    goto pass;
                }

                char* create_name = heapallocate(tokens->size[1] + 1);
                memcpy(create_name, tokens->data[1], tokens->size[1]);
                *(create_name + tokens->size[1]) = '\0';

                create_table = create_table_declaration(create_name);
            }
            else if(command_type == INSERT)
            {
                if(tokens->length != 2)
                {
                    error = 1;
                    error_message = "Syntax error for command : INSERT\n";
                    goto pass;
                }
                input_type = command_type;
                int index = get_table_index(tables, tokens->data[1]);

                if(index == -1)
                {
                    error = 1;
                    printf("Error when inserting, cannot find table : %s\n", tokens->data[1]);
                    error_message = "Error when inserting, cannot find table\n";
                    goto pass;
                }

                insert_table = *((TABLE_DECLARATION**)get_linked_list_data(tables, index));
            }
            else if(command_type == DISPLAY)
            {
                input_type = DISPLAY;
                if(tokens->length < 4)
                {
                    error = 1;
                    error_message = "Syntax error for command : DISPLAY\n";
                    goto pass;
                }
                if(strcmp(tokens->data[tokens->length - 2], FROM_COMMAND) != 0)
                {
                    error = 1;
                    error_message = "Syntax error for FROM command : DISPLAY\n";
                    goto pass;
                }
                int table_index = get_table_index(tables, tokens->data[tokens->length - 1]);
                if(table_index == -1)
                {
                    error = 1;
                    printf("Cannot find table : %s\n", tokens->data[tokens->length - 1]);
                    error_message = "Cannot find table for DISPLAY\n";
                    goto pass;
                }
                TABLE_DECLARATION* get_display_table = *((TABLE_DECLARATION**)get_linked_list_data(tables, table_index));

                if(tokens->length == 4 && strcmp(tokens->data[1], ALL_COMMAND) == 0)
                {
                    display_table = get_display_table;
                    for(int i = 0; i < get_display_table->columns->count; i++)
                    {
                        TABLE_ITEM* table_item = *((TABLE_ITEM**)get_linked_list_data(get_display_table->columns, i));
                        add_linked_list_value(display_column, &table_item->name);
                    }
                }
                else
                {
                    for(int i = 1; i < tokens->length - 2; i++)
                    {
                        int column_index = get_column_index(get_display_table, tokens->data[i]);
                        if(column_index == -1)
                        {
                            error = 1;
                            printf("Cannot find column : %s\n", tokens->data[i]);
                            error_message = "Cannot find column for DISPLAY\n";
                            goto pass;
                        }
                    }

                    display_table = get_display_table;
                    for(int i = 1; i < tokens->length - 2; i++)
                    {
                        int column_index = get_column_index(get_display_table, tokens->data[i]);
                        TABLE_ITEM* table_item = *((TABLE_ITEM**)get_linked_list_data(get_display_table->columns, column_index));
                        add_linked_list_value(display_column, &table_item->name);
                    }
                }
            }
            else if(command_type == DELETE)
            {
                if(tokens->length < 3)
                {
                    error = 1;
                    error_message = "Syntax error for command : DELETE\n";
                    goto pass;
                }
                if(tokens->length == 5)
                {
                    if(strcmp(tokens->data[1], FROM_COMMAND) == 0 && strcmp(tokens->data[3], AT_COMMAND) == 0)
                    {
                        int table_index = get_table_index(tables, tokens->data[2]);
                        if(table_index == -1)
                        {
                            error = 1;
                            printf("Error when deleting, cannot find table : %s\n", tokens->data[2]);
                            error_message = "Error when deleting, cannot find target table\n";
                            goto pass;
                        }

                        if(strcmp(tokens->data[4], ALL_COMMAND) == 0)
                        {
                            TABLE_DECLARATION* delete_table = *((TABLE_DECLARATION**)get_linked_list_data(tables, table_index));

                            for(int i = 0; i < delete_table->row_count; i++)
                            {
                                int is_foreign_referenced = is_referenced_foreign_to_index(tables, delete_table, i);
                                if(is_foreign_referenced)
                                {
                                    error = 1;
                                    printf("Error when deleting, row index is referenced to foreign\n");
                                    error_message = "Error when deleting, cannot delete a foreign referenced row\n";
                                    goto pass;
                                }
                            }
                            while (delete_table->row_count > 0)
                            {
                                delete_table_row(delete_table, 0);
                            }
                        }
                        else
                        {
                            char* dummy_ptr;
                            int row_index = strtol(tokens->data[4], &dummy_ptr, 10);

                            TABLE_DECLARATION* delete_table = *((TABLE_DECLARATION**)get_linked_list_data(tables, table_index));

                            if(row_index >= delete_table->row_count)
                            {
                                error = 1;
                                printf("Error when deleting, index is outside of bounds on table : %s, at %i\n", tokens->data[2], row_index);
                                error_message = "Error when deleting, cannot find target index on table\n";
                                goto pass;
                            }
                            
                            int is_foreign_referenced = is_referenced_foreign_to_index(tables, delete_table, row_index);

                            if(is_foreign_referenced)
                            {
                                error = 1;
                                printf("Error when deleting, row index is referenced to foreign\n");
                                error_message = "Error when deleting, cannot delete a foreign referenced row\n";
                                goto pass;
                            }

                            delete_table_row(delete_table, row_index);
                        }
                    }
                    else
                    {
                        error = 1;
                        error_message = "Syntax error for command : DELETE FROM [TABLE] AT [INDEX]\n";
                        goto pass;
                    }
                }
                else if(tokens->length == 3)
                {
                    char* dummy_ptr;
                    int table_index = get_table_index(tables, tokens->data[2]);

                    if(table_index == -1)
                    {
                        if(table_index == -1)
                        {
                            error = 1;
                            printf("Error when deleting, cannot find table : %s\n", tokens->data[2]);
                            error_message = "Error when deleting, cannot find target table\n";
                            goto pass;
                        }
                    }

                    delete_table_where = *((TABLE_DECLARATION**)get_linked_list_data(tables, table_index));

                    input_type = DELETE;
                }
                else
                {
                    error = 1;
                    error_message = "Syntax error for command : DELETE FROM [TABLE]\n";
                    goto pass;
                }
            }
            else if(command_type == UPDATE)
            {
                if(tokens->length < 2 || tokens->length > 4)
                {
                    error = 1;
                    error_message = "Syntax error for command : UPDATE [TABLE] / [AT [INDEX]]\n";
                    goto pass;
                }

                if(tokens->length == 2)
                {
                    int table_index = get_table_index(tables, tokens->data[1]);
                    if(table_index == -1)
                    {
                        error = 1;
                        printf("Error when updating, cannot find table %s\n", tokens->data[1]);
                        error_message = "Error when updating, target table does not exist\n";
                        goto pass;
                    }
                    update_table = *((TABLE_DECLARATION**)get_linked_list_data(tables, table_index));

                    update_table_at = -1;
                    input_type = UPDATE;
                }
                else if(tokens->length == 4)
                {
                    if(strcmp(tokens->data[2], AT_COMMAND) != 0)
                    {
                        error = 1;
                        error_message = "Syntax error for command : UPDATE [TABLE] AT [INDEX]\n";
                        goto pass;
                    }

                    int table_index = get_table_index(tables, tokens->data[1]);
                    if(table_index == -1)
                    {
                        error = 1;
                        printf("Error when updating, cannot find table %s\n", tokens->data[1]);
                        error_message = "Error when updating, target table does not exist\n";
                        goto pass;
                    }
                    char* dummy_ptr;
                    int row_index = strtol(tokens->data[3], &dummy_ptr, 10);

                    TABLE_DECLARATION* temp_table = *((TABLE_DECLARATION**)get_linked_list_data(tables, table_index));
                    if(row_index < 0 || row_index >= temp_table->row_count)
                    {
                        error = 1;
                        printf("Error when updating, table row is outside of bounds of row count : %i\n", row_index);
                        error_message = "Error when updating, table row is outside of bounds\n";
                        goto pass;
                    }

                    update_table_at = row_index;
                    update_table = temp_table;
                    input_type = UPDATE;
                }
                else
                {
                    error = 1;
                    error_message = "Syntax error for command : UPDATE [TABLE] / [AT [INDEX]]\n";
                    goto pass;
                }
            }
            else if(command_type == PEEK)
            {
                if(tokens->length != 1)
                {
                    error = 1;
                    error_message = "Syntax error for command : PEEK\n";
                    goto pass;
                }

                for(int i = 0; i < tables->count; i++)
                {
                    TABLE_DECLARATION* table = *((TABLE_DECLARATION**)get_linked_list_data(tables, i));
                    printf("%s\n", table->name);
                }
            }
            else if(command_type == SCRIPT)
            {
                if(tokens->length != 2)
                {
                    error = 1;
                    error_message = "Syntax error for command : SCRIPT\n";
                    goto pass;
                }
                if(script != NULL)
                {
                    error = 1;
                    error_message = "Cannot open script while another script is running\n";
                    goto pass;
                }
                FILE* open_file = fopen(tokens->data[1], "r");

                if(open_file != NULL)
                {
                    script = open_file;
                    printf("Reading script %s\n", tokens->data[1]);
                }
                else
                {
                    printf("Failed opening script %s\n", tokens->data[1]);
                }
            }
            else if(command_type == CLEAR)
            {
                if(tokens->length != 1)
                {
                    error = 1;
                    error_message = "Syntax error for command : CLEAR\n";
                    goto pass;
                }

                clear_all_table(tables);
            }
            else if(command_type == END)
            {
                loop = 0;
                goto pass;
            }
            else if(command_type == UNKNOWN)
            {
                printf("Unrecognized command : %s\n", command);

                error = 1;
                error_message = "Syntax error of unrecognized command\n";
                goto pass;
            }
        }
        else if(input_type == CREATE)
        {
            if(tokens->length == 0)
            {
                goto pass;
            }

            enum VARIABLE_TYPE variable_type = get_variable_type(tokens->data[0]);
            enum INPUT_TYPE command_type = get_command_type(tokens->data[0]);

            if(variable_type == VARCHAR_TYPE)
            {
                if(tokens->length < 3 || tokens->length > 6)
                {
                    error = 1;
                    error_message = "Error when creating table, syntax error for variable VARCHAR size name [PRIMARY/FOREIGN/EMPTY] [FOREIGN TARGET TABLE] [FOREIGN TARGET COLUMN]\n";
                    goto pass;
                }

                int column_name_size = tokens->size[2];
                if(column_name_size == 0 || column_name_size > 255)
                {
                    error = 1;
                    printf("Error when creating table %s, column name size must atleast be 1 and no more than 255 : %i", create_table->name, column_name_size);
                    error_message = "Error when creating table, incorect column name length\n";
                    goto pass;
                }
                if(get_column_index(create_table, tokens->data[2]) >= 0)
                {
                    error = 1;
                    printf("Error when creating table %s, column %s\n", create_table->name, tokens->data[2]);
                    error_message = "Error when creating table, the same column name is already exist\n";
                    goto pass;
                }

                char* end_ptr;

                long int varchar_size = strtol(tokens->data[1], &end_ptr, 10);

                if(varchar_size == 0 || varchar_size > 255)
                {
                    printf("VARCHAR size must be atleast 0 and no higher than 255 : %i\n", varchar_size);
                    error = 1;
                    error_message = "Error when creating table, incorrect VARCHAR size\n";
                    goto pass;
                }

                int is_primary = 0;
                int is_foreign = 0;
                char* target_foreign_table = NULL;
                char* target_foreign_column = NULL;

                if(tokens->length == 4)
                {
                    if(strcmp(PRIMARY_COMMAND, tokens->data[3]) != 0)
                    {
                        error = 1;
                        error_message = "Error when creating table, syntax error for primary variable VARCHAR size name [PRIMARY]\n";
                        goto pass;
                    }
                    is_primary = 1;
                }
                else if(tokens->length > 3)
                {
                    if(tokens->length != 6)
                    {
                        error = 1;
                        error_message = "Error when creating table, syntax error for foreign variable VARCHAR size name [PRIMARY/FOREIGN/EMPTY] [FOREIGN TARGET TABLE] [FOREIGN TARGET COLUMN]\n";
                        goto pass;
                    }

                    if(strcmp(FOREIGN_COMMAND, tokens->data[3]) == 0)
                    {
                        is_foreign = 1;
                    }
                    else if(strcmp(PRIMARY_FOREIGN_COMMAND, tokens->data[3]) == 0)
                    {
                        is_primary = 1;
                        is_foreign = 1;
                    }
                    else
                    {
                        error = 1;
                        error_message = "Error when creating table, syntax error for foreign variable VARCHAR size name [PRIMARY/FOREIGN/EMPTY] [FOREIGN TARGET TABLE] [FOREIGN TARGET COLUMN]\n";
                        goto pass;
                    }
                    target_foreign_table = copy_string(tokens->data[4], tokens->size[4]);
                    target_foreign_column = copy_string(tokens->data[5], tokens->size[5]);
                }
                
                TABLE_ITEM* item = create_table_item(variable_type, copy_string(tokens->data[2], tokens->size[2]), varchar_size);
                add_table_item(tables, create_table, item, is_primary, is_foreign, target_foreign_table, target_foreign_column);
            }
            else if(variable_type != UNKNOWN_TYPE)
            {
                if(tokens->length < 2 || tokens->length > 5)
                {
                    error = 1;

                    if(variable_type == INT_TYPE)
                    {
                        error_message = "Error when creating table, syntax error for variable INT name [PRIMARY/FOREIGN/EMPTY] [FOREIGN TARGET TABLE] [FOREIGN TARGET COLUMN]\n";
                    }
                    else if(variable_type == CHAR_TYPE)
                    {
                        error_message = "Error when creating table, syntax error for variable CHAR name [PRIMARY/FOREIGN/EMPTY] [FOREIGN TARGET TABLE] [FOREIGN TARGET COLUMN]\n";
                    }
                    else if(variable_type == FLOAT_TYPE)
                    {
                        error_message = "Error when creating table, syntax error for variable FLOAT name [PRIMARY/FOREIGN/EMPTY] [FOREIGN TARGET TABLE] [FOREIGN TARGET COLUMN]\n";
                    }
                    goto pass;
                }

                int column_name_size = tokens->size[1];
                if(column_name_size == 0 || column_name_size > 255)
                {
                    error = 1;
                    printf("Error when creating table %s, column name size must atleast be 1 and no more than 255 : %i", create_table->name, column_name_size);
                    error_message = "Error when creating table, incorect column name length\n";
                    goto pass;
                }
                if(get_column_index(create_table, tokens->data[1]) >= 0)
                {
                    error = 1;
                    printf("Error when creating table %s, column %s\n", create_table->name, tokens->data[1]);
                    error_message = "Error when creating table, the same column name is already exist\n";
                    goto pass;
                }

                int is_primary = 0;
                int is_foreign = 0;
                char* target_foreign_table = NULL;
                char* target_foreign_column = NULL;

                if(tokens->length == 3)
                {
                    if(strcmp(PRIMARY_COMMAND, tokens->data[2]) != 0)
                    {
                        error = 1;
                        error_message = "Error when creating table, syntax error for primary variable TYPE size name [PRIMARY]\n";
                        goto pass;
                    }
                    is_primary = 1;
                }
                else if(tokens->length > 2)
                {
                    if(tokens->length != 5)
                    {
                        error = 1;
                        error_message = "Error when creating table, syntax error for foreign variable TYPE size name [PRIMARY/FOREIGN/EMPTY] [FOREIGN TARGET TABLE] [FOREIGN TARGET COLUMN]\n";
                        goto pass;
                    }

                    if(strcmp(FOREIGN_COMMAND, tokens->data[2]) == 0)
                    {
                        is_foreign = 1;
                    }
                    else if(strcmp(PRIMARY_FOREIGN_COMMAND, tokens->data[2]) == 0)
                    {
                        is_primary = 1;
                        is_foreign = 1;
                    }
                    else
                    {
                        error = 1;
                        error_message = "Error when creating table, syntax error for foreign variable TYPE size name [PRIMARY/FOREIGN/EMPTY] [FOREIGN TARGET TABLE] [FOREIGN TARGET COLUMN]\n";
                        goto pass;
                    }
                    target_foreign_table = copy_string(tokens->data[3], tokens->size[3]);
                    target_foreign_column = copy_string(tokens->data[4], tokens->size[4]);
                }

                TABLE_ITEM* item = create_table_item(variable_type, copy_string(tokens->data[1], tokens->size[1]), 0);
                add_table_item(tables, create_table, item, is_primary, is_foreign, target_foreign_table, target_foreign_column);
            }
            else
            {
                if(command_type == END)
                {
                    if(tokens->length != 1)
                    {
                        error = 1;
                        error_message = "Error when creating table, syntax error when ending declaration\n";
                        goto pass;
                    }
                    if(create_table->columns->count == 0)
                    {
                        error = 1;
                        error_message = "Error when creating table, a table must atleast have one column\n";
                        goto pass;
                    }
                    
                    input_type = EMPTY;
                    add_linked_list_value(tables, &create_table);
                    create_table = NULL;
                }
                else
                {
                    error = 1;
                    printf("Unrecognized type when creating table : %s\n", tokens->data[0]);
                    error_message = "Error when creating table, unrecognized variable type\n";
                    goto pass;
                }
            }
        }
        else if(input_type == INSERT)
        {
            if(tokens->length == 0)
            {
                goto pass;
            }

            enum INPUT_TYPE command_type = get_command_type(tokens->data[0]);

            if(command_type == VALUES)
            {
                if(tokens->length - 1 != insert_table->columns->count)
                {
                    error = 1;
                    printf("Incorect argument count : %i\n", tokens->length - 1);
                    error_message = "Error when inserting table, incorect argument count\n";
                    goto pass;
                }

                //Check Primary
                for(int x = 0; x < insert_table->row_count; x++)
                {
                    int primary_count = 0;
                    int primary_check = 0;

                    for(int i = 0; i < insert_table->columns->count; i++)
                    {
                        TABLE_ITEM* column = *((TABLE_ITEM**)get_linked_list_data(insert_table->columns, i));
                        char* data = tokens->data[i + 1];
                        int data_len = tokens->size[i + 1];
                        char* residual;

                        if(column->is_primary)
                        {
                            primary_count += 1;

                            switch (column->type)
                            {
                                case INT_TYPE:
                                int integer = strtol(data, &residual, 10);
                                if(integer == get_linked_list_data_int(column->rows, x))
                                {
                                    primary_check += 1;
                                }
                                break;
                                case FLOAT_TYPE:
                                float floating = strtof(data, &residual);
                                if(integer == get_linked_list_data_float(column->rows, x))
                                {
                                    primary_check += 1;
                                }
                                break;
                                case CHAR_TYPE:
                                char character = ' ';
                                if(data_len > 0)
                                {
                                    character = *(data);
                                }
                                if(character == get_linked_list_data_char(column->rows, x))
                                {
                                    primary_check += 1;
                                }
                                break;
                                case VARCHAR_TYPE:
                                char* string = get_linked_list_data_string(column->rows, x);
                                if(strcmp(string, data))
                                {
                                    primary_check += 1;
                                }
                                break;
                            }
                        }
                    }
                    if(primary_count == primary_check && primary_check != 0)
                    {
                        error = 1;
                        printf("Error inserting value, same primary keys detected\n");
                        error_message = "Error when inserting table, all primary keys are equal\n";
                        goto pass;
                    }
                }

                //Check Foreign
                for(int x = 0; x < insert_table->columns->count; x++)
                {
                    TABLE_ITEM* column = *((TABLE_ITEM**)get_linked_list_data(insert_table->columns, x));
                    char* data = tokens->data[x + 1];
                    int data_len = tokens->size[x + 1];
                    char* residual;

                    if(column->is_foreign)
                    {
                        int foreign_table_index = get_table_index(tables, column->foreign_target_table);
                        TABLE_DECLARATION* foreign_table = *((TABLE_DECLARATION**)get_linked_list_data(tables, foreign_table_index));
                        int foreign_column_index = get_column_index(foreign_table, column->foreign_target_column);
                        TABLE_ITEM* foreign_column = *((TABLE_ITEM**)get_linked_list_data(foreign_table->columns, foreign_column_index));

                        int found_foreign = 0;
                        switch (column->type)
                        {
                            case INT_TYPE:
                            int integer = strtol(data, &residual, 10);
                            for(int i = 0; i < foreign_table->row_count; i++)
                            {
                                int check_int = get_linked_list_data_int(foreign_column->rows, i);

                                if(check_int == integer)
                                {
                                    found_foreign = 1;
                                    break;
                                }
                            }
                            if(!found_foreign)
                            {
                                error = 1;
                                printf("Cannot find foreign key on target table, value : %i\n", integer);
                                error_message = "Error when inserting table, cannot find foreign key on target table\n";
                                goto pass;
                            }
                            break;
                            case FLOAT_TYPE:
                            float floating = strtof(data, &residual);
                            for(int i = 0; i < foreign_table->row_count; i++)
                            {
                                float check_float = get_linked_list_data_float(foreign_column->rows, i);
                                if(floating == check_float)
                                {
                                    found_foreign = 1;
                                    break;
                                }
                            }
                            if(!found_foreign)
                            {
                                error = 1;
                                printf("Cannot find foreign key on target table, value : %f\n", floating);
                                error_message = "Error when inserting table, cannot find foreign key on target table\n";
                                goto pass;
                            }
                            break;
                            case CHAR_TYPE:
                            char character = ' ';
                            if(data_len > 0)
                            {
                                character = *(data);
                            }
                            for(int i = 0; i < foreign_table->row_count; i++)
                            {
                                char check_char = get_linked_list_data_char(foreign_column->rows, i);
                                if(check_char == character)
                                {
                                    found_foreign = 1;
                                    break;
                                }
                            }
                            if(!found_foreign)
                            {
                                error = 1;
                                printf("Cannot find foreign key on target table, value : %c\n", character);
                                error_message = "Error when inserting table, cannot find foreign key on target table\n";
                                goto pass;
                            }
                            break;
                            case VARCHAR_TYPE:
                            for(int i = 0; i < foreign_table->row_count; i++)
                            {
                                char* check_string = get_linked_list_data_string(foreign_column->rows, i);
                                if(strcmp(check_string, data) == 0)
                                {
                                    found_foreign = 1;
                                    break;
                                }
                            }
                            if(!found_foreign)
                            {
                                error = 1;
                                printf("Cannot find foreign key on target table, value : %s\n", data);
                                error_message = "Error when inserting table, cannot find foreign key on target table\n";
                                goto pass;
                            }
                            break;
                        }
                    }
                }

                //Insert
                for(int i = 0; i < insert_table->columns->count; i++)
                {
                    TABLE_ITEM* column = *((TABLE_ITEM**)get_linked_list_data(insert_table->columns, i));
                    char* data = tokens->data[i + 1];
                    int data_len = tokens->size[i + 1];
                    char* residual;

                    switch (column->type)
                    {
                        case INT_TYPE:
                        int integer = strtol(data, &residual, 10);
                        add_linked_list_int(column->rows, integer);
                        break;
                        case FLOAT_TYPE:
                        float floating = strtof(data, &residual);
                        add_linked_list_float(column->rows, floating);
                        break;
                        case CHAR_TYPE:
                        if(data_len < 1)
                        {
                            add_linked_list_char(column->rows, ' ');
                        }
                        else
                        {
                            add_linked_list_char(column->rows, *(data));
                        }
                        break;
                        case VARCHAR_TYPE:
                        add_linked_list_string_safe(column->rows, data);
                        break;
                    }
                }
                insert_table->row_count += 1;
            }
            else if(command_type == END)
            {
                input_type = EMPTY;
                insert_table = NULL;
                goto pass;
            }
            else
            {
                error = 1;
                printf("Unrecognized command when inserting table : %s\n", tokens->data[0]);
                error_message = "Error when inserting table, unrecognized command type\n";
                goto pass;
            }
        }
        else if(input_type == DISPLAY)
        {
            if(tokens->length == 0)
            {
                goto pass;
            }
            enum INPUT_TYPE command_type = get_command_type(tokens->data[0]);
            if(command_type == END)
            {
                if(tokens->length != 1)
                {
                    error = 1;
                    error_message = "Sytax error when END on DISPLAY\n";
                    goto pass;
                }

                printf("=======================%s=======================\n", display_table->name);

                int add_space = 0;
                for(int i = 0; i < display_column->count; i++)
                {
                    char* column_name = *((char**)get_linked_list_data(display_column, i));
                    int column_index = get_column_index(display_table, column_name);
                    if(column_index == -1)
                    {
                        printf("Error when displaying, cannot find column index of %s", column_name);
                    }

                    TABLE_ITEM* table_item = *((TABLE_ITEM**)get_linked_list_data(display_table->columns, column_index));
                    if(add_space)
                    {
                        printf(" | ");
                    }
                    printf("%s", table_item->name);
                    add_space = 1;
                }
                printf("\n");
                for(int i = 0; i < display_table->row_count; i++)
                {
                    add_space = 0;
                    if(is_true_on_logics(logics, display_table, i))
                    {
                        for(int j = 0; j < display_column->count; j++)
                        {
                            char* column_name = *((char**)get_linked_list_data(display_column, j));
                            int column_index = get_column_index(display_table, column_name);

                            if(add_space)
                            {
                                printf(" | ");
                            }
                            TABLE_ITEM* table_item = *((TABLE_ITEM**)get_linked_list_data(display_table->columns, column_index));
                            if(table_item->type == INT_TYPE)
                            {
                                printf("%i", get_linked_list_data_int(table_item->rows, i));
                            }
                            else if(table_item->type == FLOAT_TYPE)
                            {
                                printf("%f", get_linked_list_data_float(table_item->rows, i));   
                            }
                            else if(table_item->type == CHAR_TYPE)
                            {
                                printf("%c", get_linked_list_data_char(table_item->rows, i));   
                            }
                            else if(table_item->type == VARCHAR_TYPE)
                            {
                                printf("%s", get_linked_list_data_string(table_item->rows, i));   
                            }
                            add_space = 1;
                        }
                        printf("\n");
                    }
                }
                for(int i = 0; i < logics->count; i++)
                {
                    LOGIC_ITEM* logic_item = *((LOGIC_ITEM**)get_linked_list_data(logics, i));

                    free_logic(logic_item);
                }
                clear_linked_list(logics);
                clear_linked_list(display_column);

                input_type = EMPTY;
            }
            else if(command_type == WHERE)
            {
                if(tokens->length != 4)
                {
                    error = 1;
                    error_message = "Sytax error when WHERE on DISPLAY\n";
                    goto pass;
                }

                LOGIC_ITEM* logic_item = create_logic_item(display_table, tokens->data[1], tokens->data[2], tokens->size[2], tokens->data[3], tokens->size[3]);
                add_linked_list_value(logics, &logic_item);
            }
            else
            {
                error = 1;
                error_message = "Unrecognized command for DISPLAY\n";
                goto pass;
            }
        }
        else if(input_type == DELETE)
        {
            if(tokens->length == 0)
            {
                goto pass;
            }
            enum INPUT_TYPE command_type = get_command_type(tokens->data[0]);

            if(command_type == END)
            {
                if(tokens->length != 1)
                {
                    error = 1;
                    error_message = "Sytax error when END on DELETE\n";
                    goto pass;
                }

                for(int i = 0; i < delete_table_where->row_count; i++)
                {
                    if(is_true_on_logics(logics, delete_table_where, i))
                    {
                        if(is_referenced_foreign_to_index(tables, delete_table_where, i))
                        {
                            error = 1;
                            printf("Error when deleting, row index is referenced to foreign\n");
                            error_message = "Error when deleting, cannot delete a foreign referenced row\n";
                            goto pass;
                        }
                    }
                }

                int current_index = 0;
                int current_row_count = delete_table_where->row_count;
                while (current_index < current_row_count)
                {
                    int index_adder = 1;

                    if(is_true_on_logics(logics, delete_table_where, current_index))
                    {
                        delete_table_row(delete_table_where, current_index);
                        index_adder = 0;
                    }

                    current_index += index_adder;
                    current_row_count = delete_table_where->row_count;
                }
                
                delete_table_where = NULL;

                for(int i = 0; i < logics->count; i++)
                {
                    LOGIC_ITEM* logic_item = *((LOGIC_ITEM**)get_linked_list_data(logics, i));

                    free_logic(logic_item);
                }
                clear_linked_list(logics);

                input_type = EMPTY;
            }
            else if(command_type == WHERE)
            {
                if(tokens->length != 4)
                {
                    error = 1;
                    error_message = "Sytax error when WHERE on DELETE\n";
                    goto pass;
                }

                LOGIC_ITEM* logic_item = create_logic_item(delete_table_where, tokens->data[1], tokens->data[2], tokens->size[2], tokens->data[3], tokens->size[3]);
                add_linked_list_value(logics, &logic_item);
            }
        }
        else if(input_type == UPDATE)
        {
            enum INPUT_TYPE command_type = get_command_type(tokens->data[0]);
            if(command_type == SET)
            {
                if(tokens->length != 3)
                {
                    error = 1;
                    error_message = "Sytax error when updating on command SET [COLUMN] [VALUES]\n";
                    goto pass;
                }

                int column_index = get_column_index(update_table, tokens->data[1]);
                if(column_index == -1)
                {
                    error = 1;
                    printf("Error when updating, Cannot find column %s on table %s\n", update_table->name, tokens->data[1]);
                    error_message = "Error when updating, cannot find specified column on table\n";
                    goto pass;
                }

                UPDATE_ITEM* current_item = NULL;
                for(int i = 0; i < update_item->count; i++)
                {
                    UPDATE_ITEM* loop_item = *((UPDATE_ITEM**)get_linked_list_data(update_item, i));

                    if(loop_item->column_index == column_index)
                    {
                        current_item = loop_item;
                        break;
                    }
                }

                if(current_item == NULL)
                {
                    current_item = heapallocate(sizeof(UPDATE_ITEM));
                    current_item->column_index = column_index;
                    current_item->data = copy_string(tokens->data[2], tokens->size[2]);
                    current_item->data_size = tokens->size[2];

                    add_linked_list_value(update_item, &current_item);
                }
                else
                {
                    free(current_item->data);
                    current_item->data = copy_string(tokens->data[2], tokens->size[2]);
                    current_item->data_size = tokens->size[2];
                }
            }
            else if(command_type == WHERE)
            {
                if(tokens->length != 4)
                {
                    error = 1;
                    error_message = "Sytax error when WHERE on UPDATE\n";
                    goto pass;
                }
                if(update_table_at != -1)
                {
                    error = 1;
                    error_message = "Cannot add where condition when updating table at a certain index\n";
                    goto pass;
                }

                LOGIC_ITEM* logic_item = create_logic_item(update_table, tokens->data[1], tokens->data[2], tokens->size[2], tokens->data[3], tokens->size[3]);
                add_linked_list_value(logics, &logic_item);
            }
            else if(command_type == END)
            {
                if(tokens->length != 1)
                {
                    error = 1;
                    error_message = "Sytax error when END on UPDATE\n";
                    goto pass;
                }
                if(update_table_at != -1)
                {
                    update_table_column_at(tables, update_table, update_item, update_table_at);
                }
                else
                {
                    for(int i = 0; i < update_table->row_count; i++)
                    {
                        if(!is_true_on_logics(logics, update_table, i))
                        {
                            continue;
                        }
                        
                        update_table_column_at(tables, update_table, update_item, i);
                    }
                }

                for(int i = 0; i < update_item->count; i++)
                {
                    UPDATE_ITEM* current_update = *((UPDATE_ITEM**)get_linked_list_data(update_item, i));
                    
                    free(current_update->data);
                    free(current_update);
                }
                for(int i = 0; i < logics->count; i++)
                {
                    LOGIC_ITEM* logic_item = *((LOGIC_ITEM**)get_linked_list_data(logics, i));

                    free_logic(logic_item);
                }
                clear_linked_list(logics);

                input_type = EMPTY;

                clear_linked_list(update_item);
                update_table = NULL;
                update_table_at = -1;
            }
            else
            {
                error = 1;
                error_message = "Unrecognized command for UPDATE\n";
                goto pass;
            }
        }

        pass:
        free_parsedata(tokens);

        if(error)
        {
            error_exit(error_message);
        }
    }
    
}
int main(int argc, char* argv[])
{
    if(argc == 2)
    {
        script = fopen(argv[1], "r");
        if(script == NULL)
        {
            printf("Error opening script %s\n", argv[1]);
            error_exit("Cleaning up\n");
        }
    }

    printf("SQITS By Kadek Fajar Pramartha Yasodana / 5025231185\n");
    printf("====================================================\n");
    run();

    system("pause");

    return 0;
}

void error_exit(const char* message)
{
    if(script != NULL)
    {
        fclose(script);
        script = NULL;
    }

    printf(message);
    system("pause");
    exit(-1);
}