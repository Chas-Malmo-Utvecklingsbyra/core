#include <stdarg.h>


typedef enum
{
    LOGGER_OUTPUT_TYPE_CONSOLE,
    LOGGER_OUTPUT_TYPE_FILE_JSON,
    LOGGER_OUTPUT_TYPE_FILE_XML,
    LOGGER_OUTPUT_TYPE_UNDEFINED
} Logger_Output_Type;

typedef enum
{
    LOGGER_RESULT_OK,
    LOGGER_RESULT_INVALID_PARAMETER
} Logger_Result;

typedef struct
{
    const char *id;
    Logger_Output_Type output_type;
} Logger;

Logger_Result Logger_Init(Logger *logger, const char *id, Logger_Output_Type output_type);
Logger_Result Logger_Write(Logger *logger, const char *format, ...);
void Logger_Dispose(Logger *logger);