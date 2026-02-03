#include "logger.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "file_helper/file_helper.h"

#define LOGGER_MAX_TIME_BUFFER_SIZE 20
#define LOGGER_MAX_LOG_BUFFER_SIZE 4096

Logger_Result Logger_Init(Logger *logger, const char *id, const char *path, Logger_Output_Type output_type)
{
    if(logger == NULL || id == NULL)
    {
        return LOGGER_RESULT_INVALID_PARAMETER;
    }

    logger->id = id;
    logger->path = path;
    logger->output_type = output_type;

    return LOGGER_RESULT_OK;
}

Logger_Result Logger_Write(Logger *logger, const char *format, ...)
{
    if(logger == NULL || format == NULL)
    {
        return LOGGER_RESULT_INVALID_PARAMETER;
    }

    va_list args;
    va_start(args, format);
    time_t current_time = time(NULL);
    
    /* Build time string with local time, prints YYYY-MM-DD HH:MM:SS */
    char time_buffer[LOGGER_MAX_TIME_BUFFER_SIZE];
    struct tm* tm_info = localtime(&current_time);
    if (strftime(time_buffer, LOGGER_MAX_TIME_BUFFER_SIZE, "%Y-%m-%d %H:%M:%S", tm_info) == 0)
    {
        snprintf(time_buffer, LOGGER_MAX_TIME_BUFFER_SIZE, "Unknown Time");
    }

    char internal_format_buffer[LOGGER_MAX_LOG_BUFFER_SIZE];
    char output_buffer[LOGGER_MAX_LOG_BUFFER_SIZE];

    /* Check if path is valid for file output */
    if (logger->output_type != LOGGER_OUTPUT_TYPE_CONSOLE && (logger->path == NULL || logger->path[0] == '\0'))
    {
        va_end(args);
        return LOGGER_RESULT_INVALID_PARAMETER;
    }
    
    switch (logger->output_type)
    {
        case LOGGER_OUTPUT_TYPE_CONSOLE:
            snprintf(internal_format_buffer, sizeof(internal_format_buffer), "[%s] [%s] %s\n", time_buffer, logger->id, format);
            vsnprintf(output_buffer, sizeof(output_buffer), internal_format_buffer, args);
            
            printf("%s", output_buffer);
            break;
        
        case LOGGER_OUTPUT_TYPE_FILE_JSON: /* WIP, DONT USE */
            /* TODO: Format to JSON, filename? */
            if(File_Helper_Is_File_Empty(logger->path, "log.json"))
            {
                /* If file is empty, start the JSON array */
                File_Helper_Write(logger->path, "log.json", "[\n", 2, FILE_HELPER_MODE_WRITE, true);
            }
            else
            {
                /* If file is not empty, append a comma to separate entries */
                File_Helper_Write(logger->path, "log.json", ",\n", 2, FILE_HELPER_MODE_APPEND, false);
            }
            snprintf(internal_format_buffer, sizeof(internal_format_buffer), "{\"time\":\"%s\", \"id\":\"%s\", \"message\":\"%s\"}\n", time_buffer, logger->id, format);
            vsnprintf(output_buffer, sizeof(output_buffer), internal_format_buffer, args);
            
            File_Helper_Write(logger->path, "log.json", output_buffer, strlen(output_buffer), FILE_HELPER_MODE_APPEND, false);
            break;

        case LOGGER_OUTPUT_TYPE_FILE_TEXT:
            snprintf(internal_format_buffer, sizeof(internal_format_buffer), "[%s] [%s] %s\n", time_buffer, logger->id, format);
            vsnprintf(output_buffer, sizeof(output_buffer), internal_format_buffer, args);
            
            File_Helper_Write(logger->path, "log.txt", output_buffer, strlen(output_buffer), FILE_HELPER_MODE_APPEND, false);
            break;

        default:
            break;
    }
    
    

    // TODO: Decide how to write to file and when, and where should we do it

    va_end(args);

    return LOGGER_RESULT_OK;
}

void Logger_Dispose(Logger *logger)
{
    if(!logger)
        return;
        
    printf("Disposing logger: %s\n", logger->id);
    if(logger->output_type == LOGGER_OUTPUT_TYPE_FILE_JSON)
    {
        // Close the JSON array properly
        if(!File_Helper_Is_File_Empty(logger->path, "log.json"))
        {
            printf("Closing JSON log file\n");
            File_Helper_Write(logger->path, "log.json", "\n]", 2, FILE_HELPER_MODE_APPEND, false);
        }
    }
    // TODO: Decide what we want to do here.
}