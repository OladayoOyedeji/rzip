#include "LZMA.h"
#include <filesystem>

namespace fs = std::filesystem;

int main()
{
    File f("test.txt");
    fs::path p = "test.txt";
    size_t size = fs::file_size(p);
    uint8_t src[size];
    size = f.myread(src, size);

    if (size == 0)
    {
        cout << "ERROR: failed to read file" << endl;
        return 1;
    }

    cout << "read " << size << " bytes" << endl;  // add this

    LzmaEnc* enc = new LzmaEnc("test.txt", src, size);
    enc->Encode();
    delete enc;
    f.myclose();
    return 0;
}
