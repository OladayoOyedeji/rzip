#ifndef FILE_H
#define FILE_H

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <unordered_map>
#include <cmath>
#include <list>
#include <fstream>
#include <stdexcept>
#include <cstdint>
#include <cctype>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cerrno>
#include <format>

#include <filesystem>
#include <stdlib.h>

#ifdef _WIN32
#include <direct.h> // For _getcwd on Windows
#define GET_CURRENT_DIR _getcwd
#else
#include <unistd.h> // For getcwd on POSIX systems
#define GET_CURRENT_DIR getcwd
#endif

/* #include "include.h" */

const int MAX_BUF = 1024;

class FileError
{
public:
    FileError()
        : errno_(errno)
    {
        std::cout << "errno: " << errno_ << std::endl;
    }
    int errno_;
};

class OpenError: public FileError
{};
class ReadError: public FileError
{};
class WriteError: public FileError
{};

// const int max_int = 18446744073709551616;
class File
{
public:
    File(const std::string & filename)
    {
        fd_ = open(filename.c_str(), O_RDWR | O_CREAT, 0600);
        if (fd_ < 0) throw OpenError();
    }
    File(const char * filename)
    {
        fd_ = open(filename, O_RDWR | O_CREAT, 0600);
        if (fd_ < 0) throw OpenError();
    }
    ssize_t myread(unsigned char buff[], ssize_t size);
    ssize_t mywrite(unsigned char buff[], ssize_t size);
    off_t mylseek(off_t offset, int whence=SEEK_SET);
    void myclose()
    {
        close(fd_);
    }
    int fd_;
};

#endif
