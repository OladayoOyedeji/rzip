#include "LZMAdec.h"
#include <string>

int main()
{
    uint8_t * input = new uint8_t[1024];
    File f("file/test.txt");
    f.myread(input, 1024);

    // cout << "Original Size: 1024\n" << input << endl;
    
    size_t inputSize = 1024;
    LzmaEnc * enc = new LzmaEnc(input, inputSize);
    enc->Encode();
    
    uint8_t * comp = enc->rc_.buf_;
    size_t compSize = enc->rc_.bufPos_;
    cout << "Compressed Size: [" << compSize << "]\n";
    for (int i = 0; i < compSize; ++i)
    {
        cout << (int)comp[i] << ' ';
    }
    cout << endl;
    
    File fdec("file_encoded/test.txt.rz");
    fdec.mywrite(comp, compSize);
    
    delete enc;
    delete [] input;

    LzmaDec * dec = new LzmaDec(comp, compSize, inputSize);
    dec->Decode();

    uint8_t * decode = dec->out_;
    
    File fout("file_decoded/test.txt");
    size_t decodeSize = dec->outSize_;

    cout << decodeSize << endl;
    fout.mywrite(decode, decodeSize);
    for (int i = 0; i < decodeSize; ++i)
    {
        cout << (int)decode[i] << ' ';
    }
    cout << endl;

    cout << "comp size: " << compSize << " normal size: " << inputSize << endl;

    delete dec;
    
    return 0;
}
