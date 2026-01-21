#include "logger.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define LOGGER_MAX_TIME_BUFFER_SIZE 20
#define LOGGER_MAX_LOG_BUFFER_SIZE 4096

Logger_Result Logger_Init(Logger *logger, const char *id, Logger_Output_Type output_type)
{
    if(logger == NULL || id == NULL)
    {
        return LOGGER_RESULT_INVALID_PARAMETER;
    }

    logger->id = id;
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
    snprintf(internal_format_buffer, sizeof(internal_format_buffer), "[%s] [%s] %s\n", time_buffer, logger->id, format);
    
    char output_buffer[LOGGER_MAX_LOG_BUFFER_SIZE];
    vsnprintf(output_buffer, sizeof(output_buffer), internal_format_buffer, args);
    printf("%s", output_buffer);

    // TODO: Decide how to write to file and when, and where should we do it

    va_end(args);

    return LOGGER_RESULT_OK;
}

void Logger_Dispose(Logger *logger)
{
    (void)logger;
    // TODO: Decide what we want to do here.
}