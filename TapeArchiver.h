#ifndef TAPEARCHIVER_H
#define TAPEARCHIVER_H
#include <iostream>
#include <sys/stat.h>
#include <string>
#include <cstdio>
#include <cstring>
#include "File.h"
// void tar_file(const char * file_name, std::string & target_tar_file);

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

unsigned int getsize(const char *in);
void concantenate(std::string & s, const std::string & s1);
class Tar
{
public:
    Tar()
    {}
    void writeHeader(const char * filepath)
    {
        struct stat st;
        tar_header header;
    
        // 0. Initialize header with nulls
        memset(&header, 0, sizeof(header));

        // 1. Get system stats
        if (stat(filepath, &st) != 0) {
            perror("stat");
            return;
        }

        // 2. Fill Filename
        strncpy(header.filename, filepath, 99);
        concantenate(buff, header.filename);
        // 3. Fill Mode (Octal string)
        // We only want the last 3 octal digits (permissions)
        sprintf(header.mode, "%07o", st.st_mode & 0777);
        concantenate(buff, header.mode);

        // 4. Fill UID/GID (Octal string)
        sprintf(header.uid, "%07o", st.st_uid);
        sprintf(header.gid, "%07o", st.st_gid);
        concantenate(buff, header.gid);
        concantenate(buff, header.uid);

        // 5. Fill Size (11 octal digits + null/space)
        // Directories must have size 0 in tar
        if (S_ISDIR(st.st_mode)) {
            sprintf(header.size, "%011o", 0);
            header.typeflag[0] = '5'; // Directory
        } else {
            sprintf(header.size, "%011o", (unsigned int)st.st_size);
            header.typeflag[0] = '0'; // Normal file
        }
    
        concantenate(buff, header.typeflag);

        // 6. Fill Modification Time
        sprintf(header.mtime, "%011o", (unsigned int)st.st_mtime);
        concantenate(buff, header.mtime);
        concantenate(buff, "\n");

    }
    void writeData(const std::string & path)
    {
        std::cout << path << std::endl;
        std::ifstream f(path, std::ios::in);
        std::string input;
        
        while (std::getline(f, input))
        {
            concantenate(buff, input);
            concantenate(buff, "\n");
        }
    }
    void addtoTar(const char * path)
    {
        if (fs::is_regular_file(path))
        {
            writeHeader(path);
            writeData(path);
            //writePadding();
        }
        else
        {
            writeHeader(path);
            
            for (const auto& entry : fs::directory_iterator(path))
            {
                std::string s = entry.path().filename().string();
                addtoTar(s.c_str());
            }
        }
    }
    std::string buff;
};

#endif
