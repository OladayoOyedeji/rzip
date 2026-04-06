#ifndef INCLUDE_H
#define INCLUDE_H

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

#endif

