#include <sys/stat.h>
#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>

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

void fill_header(const char* filepath) {
    struct stat st;
    struct tar_header header;
    
    // 0. Initialize header with nulls
    memset(&header, 0, sizeof(header));

    // 1. Get system stats
    if (stat(filepath, &st) != 0) {
        perror("stat");
        return;
    }

    // 2. Fill Filename
    strncpy(header.filename, filepath, 99);
    std::cout << header.filename << std::endl;
    // 3. Fill Mode (Octal string)
    // We only want the last 3 octal digits (permissions)
    sprintf(header.mode, "%07o", st.st_mode & 0777);
    std::cout << header.mode << std::endl;

    // 4. Fill UID/GID (Octal string)
    sprintf(header.uid, "%07o", st.st_uid);
    sprintf(header.gid, "%07o", st.st_gid);
    std::cout << header.uid << ' ' << header.gid << std::endl;

    // 5. Fill Size (11 octal digits + null/space)
    // Directories must have size 0 in tar
    if (S_ISDIR(st.st_mode)) {
        sprintf(header.size, "%011o", 0);
        header.typeflag[0] = '5'; // Directory
    } else {
        sprintf(header.size, "%011o", (unsigned int)st.st_size);
        header.typeflag[0] = '0'; // Normal file
    }
    
    std::cout << header.typeflag << std::endl;

    // 6. Fill Modification Time
    sprintf(header.mtime, "%011o", (unsigned int)st.st_mtime);
    
    std::cout << header.mtime << std::endl;
    
    // Now you would calculate the checksum and write to file...
}

int main()
{
    char filepath[1024];
    fill_header("robert.txt");
    
    return 0;
}
