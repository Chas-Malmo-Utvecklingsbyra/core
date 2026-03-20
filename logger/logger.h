#include <stdarg.h>

/**
 * @file logger.h
 * @brief Logger module for logging messages to console or files in various formats.
 */
typedef enum
{
    LOGGER_OUTPUT_TYPE_CONSOLE,
    LOGGER_OUTPUT_TYPE_FILE_JSON,
    LOGGER_OUTPUT_TYPE_FILE_XML,
    LOGGER_OUTPUT_TYPE_FILE_TEXT,
    LOGGER_OUTPUT_TYPE_UNDEFINED
} Logger_Output_Type;

typedef enum
{
    LOGGER_RESULT_OK,
    LOGGER_RESULT_INVALID_PARAMETER
} Logger_Result;

typedef enum
{
    LOGGER_LEVEL_INFO,
    LOGGER_LEVEL_WARNING,
    LOGGER_LEVEL_ERROR
} Logger_Level;

typedef struct
{
    const char *id;
    const char *path;
    const char *file_name;
    Logger_Output_Type output_type;
    Logger_Level level;
} Logger;

/**
 * @brief Initializes a Logger instance.
 * @param logger Pointer to the Logger instance to initialize.
 * @param id Identifier for the logger.
 * @param path File path for log output (if applicable).
 * @param output_type Type of output for the logger.
 * @return LOGGER_RESULT_OK on success, otherwise an error code.
 */
Logger_Result Logger_Init(Logger *logger, const char *id, const char *path, const char *file_name, Logger_Output_Type output_type);

/**
 * @brief Writes a log message using the specified logger.
 * @param logger Pointer to the Logger instance.
 * @param level What level of loginfo triggered.
 * @param format Format string for the log message (printf-style).
 * @param ... Additional arguments for the format string.
 * @return LOGGER_RESULT_OK on success, otherwise an error code.
 */
Logger_Result Logger_Write(Logger *logger, Logger_Level level, const char *format, ...);

/**
 * @brief Disposes of a Logger instance, releasing any allocated resources.
 * @param logger Pointer to the Logger instance to dispose.
 * @return void.
 */
void Logger_Dispose(Logger *logger);

/**
 * @brief Macro to write a log message with automatic null check.
 * @param logger Pointer to the Logger instance (can be NULL).
 * @param ... Format string and arguments for the log message.
 * 
 * Usage: LOG_WRITE(logger, "User %s logged in at %d", username, timestamp);
 */
#define LOG_WRITE(logger, level, ...) \
    do { \
        if ((logger) != NULL) { \
            Logger_Write((logger), (level), __VA_ARGS__); \
        } \
    } while(0)
