#include "logger.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>



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
    
    char internal_format_buffer[4096];
    snprintf(internal_format_buffer, sizeof(internal_format_buffer), "%ld [%s] %s\n", current_time, logger->id, format);
    
    char output_buffer[4096];
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