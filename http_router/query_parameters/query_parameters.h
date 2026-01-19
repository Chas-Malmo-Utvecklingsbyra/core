#ifndef QUERY_PARAMETERS_H
#define QUERY_PARAMETERS_H

#include <string.h>

#define QUERY_PARAMETER_MAX_LENGTH 256

typedef enum QueryParameter_Result_t
{
    QUERY_PARAMETER_RESULT_MALFORMED = -3,
    QUERY_PARAMETER_RESULT_ALLOC_FAILURE = -2,
    QUERY_PARAMETER_RESULT_ERROR = -1,
    QUERY_PARAMETER_RESULT_OK = 0
} QueryParameter_Result_t;

/**
 * @brief Structure to hold query parameters parsed from a URL.
 * @param keys Array of parameter key strings
 * @param values Array of parameter value strings
 * @param count Number of parameters currently stored
 * @param capacity Maximum number of parameters that can be stored
 */
typedef struct QueryParameters_t
{
    char **keys;
    char **values;
    size_t count;
} QueryParameters_t;

/**
 * @brief Creates a QueryParameters_t structure with a specified capacity.
 * @param param Pointer to the QueryParameters_t structure to initialize.
 * @param capacity The maximum number of parameters that can be stored.
 * @return int 0 on success, negative value on error.
 */
QueryParameter_Result_t Query_Parameter_Create(QueryParameters_t *param, size_t param_count);

/**
 * @brief Parses query parameters from a URL path into a QueryParameters_t structure.
 * @param param Pointer to the QueryParameters_t structure to populate.
 * @param path The URL path containing query parameters.
 * @return int 0 on success, negative value on error.
 */
QueryParameter_Result_t Query_Parameter_Parse(QueryParameters_t *param, const char *path);

/**
 * @brief Counts the number of query parameters in a URL path.
 * @param path The URL path containing query parameters.
 * @return size_t The number of query parameters found.
 */
size_t Query_Parameter_Get_Param_Count(const char *path);

/**
 * @brief Retrieves the value of a query parameter by key.
 * @param param Pointer to the QueryParameters_t structure.
 * @param key The key of the query parameter to retrieve.
 * @return const char* The value of the query parameter, or NULL if not found.
 */
const char *Query_Parameter_Get(QueryParameters_t *param, const char *key);

/**
 * @brief Frees the memory allocated for a QueryParameters_t structure.
 * @param param Pointer to the QueryParameters_t structure to free.
 */
void Query_Parameter_Dispose(QueryParameters_t *param);

#endif // QUERY_PARAMETERS_H