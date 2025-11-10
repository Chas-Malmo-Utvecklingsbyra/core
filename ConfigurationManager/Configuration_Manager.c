#include "Configuration_Manager.h"
#include <stdlib.h>
#include "../json/fileHelper/fileHelper.h"
#include <string.h>

/* TODO: Move this to utils? */
char* strdup(const char* str){
	char* copy = (char*)malloc(strlen(str) + 1);
	if(copy == NULL)
		return NULL;
	strcpy(copy, str);
	/*printf("[ALLOC] strdup: %s -> %p\n", str, (void*)copy);*/
	return copy;
}

/* TODO: Add datatypes? */
int Configuration_Manager_Init(Linked_List* configuration_list)
{
    if(Linked_List_Init(configuration_list) == LL_FAILURE) return -1;

    cJSON* root = json_read_from_file(CONFIGURATION_FILE_PATH);

    if (root == NULL) return -1;

    cJSON* current = cJSON_GetArrayItem(root, 0);

    if(current == NULL) return -1;

    while(current != NULL)
    {
        Configuration_Field* new_field = (Configuration_Field*)malloc(sizeof(Configuration_Field));
        new_field->field_name = (void*)strdup(current->string);
        
        switch (current->type)
        {
            case cJSON_String:
                new_field->field_datatype = "string";
                new_field->field_value = (char*)malloc(sizeof(current->valuestring));
                new_field->field_value = (void*)strdup(current->valuestring);
                break;
        }
        /* printf("new_field: %s\n%s\n%s\n", new_field->field_name, new_field->field_datatype, (char*)new_field->field_value); */
        Linked_List_Add_Item(configuration_list, (void*)new_field);
        current = current->next;
    }
    return 0;
}

Configuration_Field* Configuration_Get_Field(Linked_List* configuration_list, char* get_field_name)
{
    if(configuration_list == NULL)
    {
        return NULL;
    }
    if(configuration_list->head == NULL)
    { 
        return NULL;
    }

    Linked_List_Node* current = configuration_list->head;
    while (current != NULL)
    {
        if(current->object == NULL)
        {
            continue;
        } 
        Configuration_Field* temp = (Configuration_Field*)current->object;
        if(strcmp(temp->field_name, get_field_name))
            return temp;
    }
    return NULL;
}

void Configuration_Field_Dispose(Configuration_Field* field)
{
    if(field == NULL) return;
    if(field->field_name != NULL)
    {
        free(field->field_name);
    }
    if(field->field_value != NULL)
    {
        free(field->field_value);
    }
}