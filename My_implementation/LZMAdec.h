#ifndef LZMADEC_H
#define LZMADEC_H

#include "LZMAenc.h"

typedef uint16_t LzmaProb;

class RangeDecoder
{
public:
    RangeDecoder(uint8_t * in, size_t inSize)
        : buf_(in), bufSize_(inSize), bufPos_(0),
          code_(0), range_(0xFFFFFFFF)
    {
        bufPos_++;
        
        for (int i = 0; i < 5; i++)
        {
            code_ = (code_ << 8) | buf_[bufPos_++];
        }
        cout << "DEC code_=" << code_ << " bufPos_=" << bufPos_ << endl;
        cout << "DEC code_ hex=" << hex << code_ << dec << endl;
        cout << "bufPos" << bufPos_ << endl;
    }
    int DecodeBit(LzmaProb & prob);
    uint32_t DecodeDirectBits(int);
    
// private:
    size_t bufSize_;
    uint8_t * buf_;
    size_t bufPos_;
    uint32_t code_;
    
    uint32_t range_;
};

class LenDec
{
public:
    LenDec()
    {
        static const int kNumPosBitsMax = 4;
        choice_ = choice2_ = 1024;
        for (int i = 0; i < (1 << kNumPosBitsMax); i++)
            for (int j = 0; j < 8; j++)
                low_[i][j] = mid_[i][j] = 1024;
        for (int i = 0; i < 256; i++)
            high_[i] = 1024;
    }
    uint32_t Decode(RangeDecoder & rc, uint32_t posState);
// private:
    LzmaProb choice_, choice2_;
    LzmaProb low_[1 << 4][8];
    LzmaProb mid_[1 << 4][8];
    LzmaProb high_[256];
};

class LzmaDec
{
public:
    LzmaDec(uint8_t* compressed, size_t compSize, size_t outSize)
        : lc_(3), lp_(0), pb_(2),
          out_(new uint8_t[outSize]),
          outSize_(outSize), outPos_(0),
          srcSize_(compSize),
          state_(0),
          rc_(compressed, compSize)
    {
        pbMask_ = (1 << pb_) - 1;

        for (int i = 0; i < 12; i++)
            for (int j = 0; j < 16; j++)
                isMatch_[i][j] = 1024;

        for (int i = 0; i < 0x300 * (1 << lc_); ++i)
            litProb_[i] = 1024;

        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 64; ++j)
                distSlot_[i][j] = 1024;

        for (int i = 0; i < 16; i++)
            distAlign_[i] = 1024;
    }

    ~LzmaDec()
    {
        delete[] out_;
    }

    void     Decode();
    uint8_t* GetOutput()  { return out_; }
    size_t   GetOutSize() { return outPos_; }
    void     PutByte(uint8_t byte)  { out_[outPos_++] = byte; }
    uint8_t  GetByte(uint32_t dist)
    {
        cout << outPos_  << ' ' << dist << endl;
        return out_[outPos_ - dist];
    }
    uint8_t  DecodeLiteral(uint8_t prev);
    uint32_t DecodeDistance(uint32_t len);
    
// private:
    uint32_t  lc_, lp_, pb_, pbMask_;
    uint8_t*  out_;
    size_t    outSize_;
    uint32_t  outPos_;
    size_t    srcSize_;
    uint32_t  state_;

    LzmaProb  isMatch_ [12][16];
    LzmaProb  litProb_ [0x300 * (1 << 3)];
    LenDec    lenDec_;
    LzmaProb  distSlot_[4][64];
    LzmaProb  distAlign_[16];

    RangeDecoder rc_;

};

#endif
