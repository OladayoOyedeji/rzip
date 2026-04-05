#ifndef TAPEARCHIVER_H
#define TAPEARCHIVER_H
#include <iostream>
#include <sys/stat.h>
#include <string>
void tar_file(const char * file_name, std::string & target_tar_file);

namespace fs = std::filesystem;

struct tar_header {
    char filename[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag[1];
};

unsigned int getsize(const char *in)
{
    unsigned int size = 0;
    unsigned int j;
    unsigned int count = 1;

    for (j = 11; j > 0; j--, count *= 8)
        size += ((in[j - 1] - '0') * count);

    return size;
}

class Tar
{
    Tar();
    void writeHeader(const char * file_name,
                     std::string & target_tar_file)
    {
        struct stat file_info;
        stat(file_name, &file_info);

        file_info.st_size; file_info.st_mode; file_info.st_mtime;
        file_info.st_uid;  file_info.st_gid;  file_info.st_size;
    }
    void addtoTar(const std::string & path)
    {
        if (fs::is_regular_file(path))
        {
            writeHeader(path);
            writeData(path);
            writePadding();
        }
        else
        {
            writeHeader(path + "/");
            
            for (const auto& entry : fs::directory_iterator(path))
            {
                addtoTar(entry.status());
            }
        }
    }
};

#endif
