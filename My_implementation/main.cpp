#include "LZMAdec.h"
#include <string>

size_t get_size(const char * filename)
{
    struct stat fileStatus;
    if (stat(filename, &fileStatus) == 0)
    {
        std::cout << "File size: " << fileStatus.st_size << " bytes\n";
        return fileStatus.st_size;
    } else {
        std::cerr << "Error reading file stats.\n";
        return -1;
    }
}

int main()
{
    size_t size;
    try
    {
        size = get_size("file/main.py");
    }
    catch (const std::runtime_error & e)
    {
        cout << e.what() << endl;
        return -1;
    }
    uint8_t * input = new uint8_t[size];
    File f("file/main.py");
    f.myread(input, size);
    
    size_t inputSize = size;
    LzmaEnc * enc = new LzmaEnc(input, inputSize);
    enc->Encode();
    
    uint8_t * comp = enc->rc_.buf_;
    size_t compSize = enc->rc_.bufPos_;
    cout << "Compressed Size: [" << compSize << "]\n";
    
    File fdec("file_encoded/main.py.rz");
    fdec.mywrite(comp, compSize);
    
    delete enc;
    delete [] input;

    LzmaDec * dec = new LzmaDec(comp, compSize, inputSize);
    dec->Decode();

    uint8_t * decode = dec->out_;
    
    File fout("file_decoded/main.py");
    size_t decodeSize = dec->outSize_;
    
    fout.mywrite(decode, decodeSize);

    cout << "comp size: " << compSize << " normal size: " << inputSize << endl;

    delete dec;
    
    return 0;
}
