#include "File.h"

ssize_t File::myread(unsigned char buff[], ssize_t size)
{
    ssize_t total_size = 0; // total number of bytes read
    while (size > 0)
    {
        ssize_t result_size = read(fd_, buff, size);
        std::cout << result_size << std::endl;
        if (result_size == 0) break; // EOF
        if (result_size == -1) throw ReadError();
        buff += result_size;
        size -= result_size;
        total_size += result_size;
    }
    return total_size;
}

ssize_t File::mywrite(unsigned char buff[], ssize_t size)
{
    ssize_t total_size = 0; // total number of bytes written
    while (size > 0)
    {
        ssize_t result_size = write(fd_, buff, size);
        if (result_size == 0) break; // EOF
        if (result_size == -1) throw WriteError();
        buff += result_size;
        size -= result_size;
        total_size += result_size;
    }
    return total_size;
}

off_t File::mylseek(off_t offset, int whence)
{
    return lseek(fd_, offset, whence);
}
