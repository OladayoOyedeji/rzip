#include "TapeArchiver.h"

void tar_file(const char * file_name, std::string & target_tar_file)
{
    struct stat file_info;

    stat(file_name, &file_info);

    std::cout << "size: " << file_info.st_size
              << "mode: " << file_info.st_mode
              << "modification time: " << file_info.st_mtime << std::endl;
}
