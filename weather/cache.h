#ifndef __cache_h__
#define __cache_h__

typedef enum Cache_Result
{
    Cache_Result_File_Does_Not_Exist = 1,
    Cache_Result_OK = 0,
    Cache_Result_Error = -1

} Cache_Result;

Cache_Result cache_save_to_file(const char* file_name, const char* data);
Cache_Result cache_create_directory();

#endif