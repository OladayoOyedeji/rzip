#ifndef LZMA_H
#define LZMA_H

#include <iostream>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <string>
#include "File.h"

using namespace std;

typedef uint16_t LzmaProb;

typedef std::pair<uint8_t *, size_t> Pair;

std::ostream & operator<<(std::ostream & cout, const Pair & p);

class RangeEncoder
{
public:
    RangeEncoder(size_t inputSize)
        : bufSize_(inputSize + (inputSize >> 3) + 128),
          buf_(new uint8_t[bufSize_]),
          bufPos_(0),
          low_(0), range_(0xFFFFFFFF),
          cacheSize_(1), cache_(0)
    {}
    void EncodeBit(uint16_t & prob, int bit);
    void Flush();
    void ShiftLow();
    void EncodeDirectBits(uint32_t, int);
    void WriteByte(uint8_t byte)
    {
        // if (bufPos_ >= bufSize_) {
    //     cout << "ERROR: output buffer overflow at " << bufPos_ << endl;
    //     return;
    // }
        // cout << bufPos_ << endl;
        buf_[bufPos_++] = byte;
    }
//private:
    size_t bufSize_;
    uint8_t * buf_;
    size_t bufPos_;
    uint64_t low_;
    
    uint32_t range_;
    uint32_t cacheSize_;
    uint8_t cache_;
};

class LenEnc
{
public:
    LenEnc()
    {
        static const int kNumPosBitsMax = 4;
        choice_ = choice2_ = 1024;
        for (int i = 0; i < (1 << kNumPosBitsMax); i++)
            for (int j = 0; j < 8; j++)
                low_[i][j] = mid_[i][j] = 1024;
        for (int i = 0; i < 256; i++)
            high_[i] = 1024;
    }
    void Encode(RangeEncoder & rc, uint32_t posState, uint32_t len);
private:
    LzmaProb choice_;
    LzmaProb choice2_;
    LzmaProb low_[1 << 4][8];
    LzmaProb mid_[1 << 4][8];
    LzmaProb high_[256];
};

class LzmaEnc
{
public:
    LzmaEnc(const char * name, uint8_t*, size_t);
    bool Encode();
    void EncodeLiteral(uint32_t);
    int  FindMatch(uint32_t, uint32_t&);
    void EncodeDistance(uint32_t, uint32_t);

private:
    std::string name_;
    uint8_t*     src_;
    size_t       srcSize_;
    RangeEncoder rc_;
    uint32_t     lc_ = 3, lp_ = 0, pb_ = 2;
    uint32_t     state_;
    LzmaProb     isMatch_[12][16];
    LzmaProb     isRep_[12];
    LzmaProb     litProb_[0x300 * (1 << 3)];
    LenEnc       lenEnc_;
    LzmaProb     distSlot_[4][64];
    LzmaProb     distAlign_[16];
    uint32_t     hashTable_[1 << 16];
    uint32_t     chain_[1 << 22];
    uint32_t     dictSize_ = 1 << 22;
};

#endif
