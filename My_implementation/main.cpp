#include "LZMAdec.h"
#include <string>

int main()
{
    uint8_t * input = new uint8_t[1024];
    File f("test.txt");
    f.myread(input, 1024);

    cout << "Original Size: 1024\n" << input << endl;
    
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
    
    delete enc;
    delete input;

    // LzmaDec * dec = new LzmaDec(comp, compSize, inputSize);
    // dec->Decode();

    // uint8_t * decode = dec->out_;
    // size_t decodeSize = sizeof(decode);
    // for (int i = 0; i < decodeSize; ++i)
    // {
    //     cout << (int)decode[i] << ' ';
    // }
    // cout << endl;
    
    
    return 0;
}
