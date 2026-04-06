#include "TapeArchiver.h"

// void tar_file(const char * file_name, std::string & target_tar_file)
// {
//     struct stat file_info;

//     stat(file_name, &file_info);

//     std::cout << "size: " << file_info.st_size
//               << "mode: " << file_info.st_mode
//               << "modification time: " << file_info.st_mtime << std::endl;
// }

unsigned int getsize(const char *in)
{
    unsigned int size = 0;
    unsigned int j;
    unsigned int count = 1;

    for (j = 11; j > 0; j--, count *= 8)
        size += ((in[j - 1] - '0') * count);

    return size;
}

void concantenate(std::string & s, const std::string & s1)
{
    for (int i = 0; i < s1.size(); ++i)
    {
        s.push_back(s1[i]);
    }
}
