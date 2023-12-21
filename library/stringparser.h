#ifndef STRINGPARSER
#define STRINGPARSER

typedef struct parsedata
{
    char** data;
    int length;
    int* size;
} parsedata;
typedef struct parseindex
{
    int start;
    int end;
} parseindex;

char* copy_string(char* from, int size);
parsedata* parse(char* string, int buffer_size, char* parse_seperator, int seperator_size, char* parse_joinner, int joinner_size);
void free_parsedata(parsedata* data);

#endif