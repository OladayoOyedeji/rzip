#include "LZMA.h"
#include "File.h"

int main()
{
    File f("test.txt");
    uint8_t src[1024];
    size_t size = f.myread(src, 1024);

    if (size == 0) {
        cout << "ERROR: failed to read file" << endl;
        return 1;
    }

    cout << "read " << size << " bytes" << endl;  // add this

    LzmaEnc* enc = new LzmaEnc(src, size);
    enc->Encode();
    delete enc;
    f.myclose();
    return 0;
}
