#ifndef FILE_HELPER_H
#define FILE_HELPER_H

#include <stddef.h>
#include <stdbool.h>
/**
 * @file file_helper.c
 * @brief Implementation of file and directory helper functions.
 */

typedef enum File_Helper_Mode
{
    FILE_HELPER_MODE_READ,
    FILE_HELPER_MODE_WRITE,
    FILE_HELPER_MODE_APPEND
} File_Helper_Mode;

typedef enum File_Helper_Result
{
    FILE_HELPER_RESULT_SUCCESS,
    FILE_HELPER_RESULT_FAILURE,
    FILE_HELPER_RESULT_ALREADY_EXISTS,
    FILE_HELPER_RESULT_NOT_FOUND,
    FILE_HELPER_RESULT_ALLOCATION_FAILURE
} File_Helper_Result;

/**
 * @brief Checks if a directory exists at the given path.
 * @param path The path to the directory.
 * @return true if the directory exists, false otherwise.
 */
bool File_Helper_Dir_Exists(const char *path);

/**
 * @brief Checks if a file exists at the given path.
 * @param path The path to the file.
 * @return true if the file exists, false otherwise.
 */
bool File_Helper_File_Exists(const char *path);

/**
 * @brief creates a new directory at the specified path.
 * @param path The path where the directory should be created.
 * @return FILE_HELPER_RESULT_SUCCESS on success, otherwise an error code.
 */
File_Helper_Result File_Helper_Create_Dir(const char *path);

/**
 * @brief deletes the directory at the specified path.
 * @param path The path of the directory to delete.
 * @return FILE_HELPER_RESULT_SUCCESS on success, otherwise an error code.
 */
File_Helper_Result File_Helper_Delete_Dir(const char *path);

/**
 * @brief Creates a new file at the specified path with the given filename.
 * @param path The directory path where the file should be created.
 * @param filename The name of the file to create.
 * @return FILE_HELPER_RESULT_SUCCESS on success, otherwise an error code.
 */
File_Helper_Result File_Helper_Create(const char *path, const char *filename);

/**
 * @brief Writes data to a file at the specified path with the given filename.
 * @param path The directory path where the file is located.
 * @param filename The name of the file to write to.
 * @param data The data to write to the file.
 * @param data_size The size of the data to write.
 * @param mode The mode to open the file (write or append).
 * @param overwrite Whether to overwrite the file if it already exists.
 * @return FILE_HELPER_RESULT_SUCCESS on success, otherwise an error code.
 */
File_Helper_Result File_Helper_Write(const char *path, const char *filename, const char *data, size_t data_size, File_Helper_Mode mode, bool overwrite);

/**
 * @brief Reads data from a file at the specified path with the given filename.
 * @param path The directory path where the file is located.
 * @param filename The name of the file to read from.
 * @param out_data Pointer to store the read data (caller is responsible for freeing).
 * @param out_data_size Pointer to store the size of the read data.
 * @return FILE_HELPER_RESULT_SUCCESS on success, otherwise an error code.
 */
File_Helper_Result File_Helper_Read(const char *path, const char *filename, char **out_data, size_t *out_data_size);

/**
 * @brief Deletes a file at the specified path with the given filename.
 * @param path The directory path where the file is located.
 * @param filename The name of the file to delete.
 * @return FILE_HELPER_RESULT_SUCCESS on success, otherwise an error code.
 */
File_Helper_Result File_Helper_Delete_File(const char *path, const char *filename);

/**
 * @brief Gets the path of the most recently modified file in a directory.
 * @param path The directory path to search for the most recent file.
 * @param out_filepath Pointer to store the full path of the most recent file (caller is responsible for freeing).
 * @return FILE_HELPER_RESULT_SUCCESS on success, otherwise an error code.
 */
File_Helper_Result File_Helper_Get_Most_Recent_File(const char *path, char **out_filepath);

/**
 * @brief Checks if a file is empty at the specified path with the given filename.
 * @param path The directory path where the file is located.
 * @param filename The name of the file to check.
 * @return true if the file is empty or does not exist, false otherwise.
 */
bool File_Helper_Is_File_Empty(const char *path, const char *filename);

#endif // FILE_HELPER_H