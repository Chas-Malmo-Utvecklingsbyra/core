#include "../Datatypes/LinkedList.h"

#ifndef Configuration_Manager_H
#define Configuration_Manager_H

#define CONFIGURATION_FILE_PATH "settings.json"

typedef struct Configuration_Field Configuration_Field;

/*
* settings.json:
* {
*   "host" : "127.0.0.1",
*   "port" : "80",
*   "postgresqlApiKey" : "asdasdd"
* }
*/

struct Configuration_Field
{
    char* field_name;
    char* field_datatype;
    void* field_value;
};

int Configuration_Manager_Init(Linked_List* configuration_list);

/* Should maybe be a local function */
Configuration_Field* Configuration_Get_Field(Linked_List* configuration_list, char* get_field_name);

void Configuration_Field_Dispose(Configuration_Field* field);

#endif /* Configuration_Manager_H */