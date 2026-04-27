#include "LZMA.h"

std::ostream & operator<<(std::ostream & cout, const Pair & p)
{
    cout << '[';
    string delim = "";
    for (int i = 0; i < p.second; ++i)
    {
        cout << delim << int((p.first)[i]);
        delim = ", ";
    }
    return cout;
}

void BitTreeEncode(RangeEncoder & rc, LzmaProb * tree, int bits, uint32_t sym)
{
    uint32_t m = 1;
    for (int i = bits - 1; i >= 0; i--)
    {
        uint32_t bit = (sym >> i) & 1;
        cout << tree[m] << ' ' << bit << endl;
        rc.EncodeBit(tree[m], bit);
        m = (m << 1) | bit;
    }
}

void RangeEncoder::ShiftLow()
{
    if ((uint32_t)low_ < 0xFF000000 || (low_ >> 32) != 0)
    {
        uint8_t carry = (uint8_t)(low_ >> 32);

        // emit the held cache byte with carry
        WriteByte(cache_ + carry);
        cacheSize_--;

        while (cacheSize_ != 0)
        {
            WriteByte(0xFF + carry);
            cacheSize_--;
        }
        
        cache_ = (uint8_t)((uint32_t)low_ >> 24);
    }
    else
    {
        cacheSize_++;
    }

    low_ = (low_ & 0xFFFFFFFF) << 8;
}

void RangeEncoder::EncodeDirectBits(uint32_t value, int numBits)
{
    for (int i = numBits - 1; i >= 0; i--)
    {
        range_ >>= 1;
        low_ += range_ & (0 - ((value >> i) & 1));
        if (range_ < (1 << 24))
        {
            range_ <<= 8;
            ShiftLow();
        }
    }
}

void RangeEncoder::Flush()
{
    std::cout << "Flush\n";
    for (int i = 0; i < 5; i++)
    {
        std::cout << i << std::endl;
        ShiftLow();
    }
}

void LenEnc::Encode(RangeEncoder & rc, uint32_t posState, uint32_t len)
{
    len -= 2;

    if (len < 8) {
        rc.EncodeBit(choice_, 0);
        BitTreeEncode(rc, low_[posState], 3, len);
    }
    else if (len < 16) {
        rc.EncodeBit(choice_,  1);
        rc.EncodeBit(choice2_, 0);
        BitTreeEncode(rc, mid_[posState], 3, len - 8);
    }
    else {
        rc.EncodeBit(choice_,  1);
        rc.EncodeBit(choice2_, 1);
        BitTreeEncode(rc, high_, 8, len - 16);
    }
}

LzmaEnc::LzmaEnc(uint8_t* in, size_t inSize)
    : lc_(3), lp_(0), pb_(2),
      src_(new uint8_t[inSize]),
      srcSize_(inSize),
      state_(0), rc_(inSize)
{
    // match finder
    memset(hashTable_, 0xFF, sizeof(hashTable_));
    memset(chain_,     0xFF, sizeof(chain_));

    // copy input
    for (int i = 0; i < (int)inSize; ++i)
        src_[i] = in[i];

    // isMatch
    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 16; j++)
            isMatch_[i][j] = 1024;

    // isRep
    for (int i = 0; i < 12; i++)
        isRep_[i] = 1024;

    // litProbs — all 6144 entries
    for (int i = 0; i < 0x300 * (1 << lc_); ++i)
        litProb_[i] = 1024;

    // distance slot trees
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 64; ++j)
            distSlot_[i][j] = 1024;

    // align tree
    for (int i = 0; i < 16; i++)
        distAlign_[i] = 1024;
}

uint32_t GetPosSlot(uint32_t dist)
{
    static const uint8_t kDistSlot_[128] = {
        0, 1, 2, 3, 4, 4, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7,
        8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9,
        10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
        11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,
        12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,
        12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,
        13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,
        13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,
    };
    
    if (dist < 128) return kDistSlot_[dist];   // lookup table for small distances

    uint32_t n = 0;
    uint32_t d = dist >> 7;
    if (d >= (1<<4)) { n += 4; d >>= 4; }
    if (d >= (1<<2)) { n += 2; d >>= 2; }
    n += (d >> 1);
    return (n << 1) + ((dist >> (n-1)) & 1) + 12;
}

void LzmaEnc::EncodeDistance(uint32_t dist, uint32_t len)
{
    uint32_t lenState = len - 2;
    if (lenState > 3) lenState = 3;

    uint32_t slot = GetPosSlot(dist);
    BitTreeEncode(rc_, distSlot_[lenState], 6, slot);

    if (slot >= 4)
    {
        uint32_t directBits = (slot >> 1) - 1;
        uint32_t base = (2 | (slot & 1)) << directBits;
        uint32_t extra = dist - base;

        if (slot < 14)
        {
            // Encode Direct Bits
            rc_.EncodeDirectBits(extra >> 4, directBits-4);
        }

        BitTreeEncode(rc_, distAlign_, 4, extra & 0xF);
        
    }
}

void LzmaEnc::EncodeLiteral(uint32_t pos)
{
    uint8_t byte = src_[pos];
    uint8_t prev = pos > 0 ? src_[pos - 1] : 0;
    uint32_t litState = (prev >> (8 - lc_));
    
    LzmaProb * probs = litProb_ + 0x300 * litState;
    cout << "here?\n";
    BitTreeEncode(rc_, probs, 8, byte);
}

bool LzmaEnc::Encode()
{
    // transitions litNext[state_]
    static const uint32_t transNext[12] = {0,0,0,0,1,2,3,4,5,6,4,5};
    state_ = 0;
    int pos = 0;
    while (pos < srcSize_-1)
    {
        uint32_t posState = pos & ((1 << pb_) - 1);
        uint32_t dist, len;
        len = FindMatch(pos, dist);
        if (len < 2)
        {
            std::cout << state_ << ' ' << posState << std::endl;
            rc_.EncodeBit(isMatch_[state_][posState], 0);
            EncodeLiteral(pos);
            cout << "literal: " << (int)src_[pos] << endl;

            state_ = transNext[state_];
            pos++;
        }
        else
        {
            cout << "matched: <dist: " << dist
                 << " len: " << len << "> index: "
                 << pos << " to " << pos - dist
                 << " \n";
            // encode <len, dist>
            rc_.EncodeBit(isMatch_[state_][posState], 1);
            lenEnc_.Encode(rc_, posState, len);
            EncodeDistance(dist - 1, len);

            // reps[3] = reps[2]; reps[2] = reps[1];
            // reps[1] = reps[0]; reps[0] = dist - 1;

            state_ = (state_ < 7) ? 7 : 10;
            
            pos += len;
        }
    }
    rc_.Flush();
    Pair p;
    p.first = rc_.buf_;
    p.second = rc_.bufPos_;
    std::cout << "hello[" << p << ']' << std::endl;
    return true;
}

void RangeEncoder::EncodeBit(uint16_t & prob, int bit)
{
    uint32_t bound = (range_ >> 11) * prob;

    if (bit == 0)
    {
        range_ = bound;
        prob += (2048 - prob) >> 5;
    }
    else
    {
        low_ += bound;
        range_ -= bound;
        prob -= prob >> 5;
    }

    if (range_ < (1 << 24))
    {
        range_ <<= 8;
        cout << "encode bit " << low_ << ' ' << range_ << endl;
        ShiftLow();
    }
}

int LzmaEnc::FindMatch(uint32_t pos, uint32_t & outDist)
{
    uint32_t bestLen = 0;
    uint32_t bestDist = 0;
    uint32_t limit = pos < dictSize_ ? pos: dictSize_;

    uint16_t h = (src_[pos] << 8) | src_[pos + 1];
    uint32_t head = hashTable_[h];
    uint32_t cur = head;
    hashTable_[h] = pos;

    // you are basically checking every possible matches
    for (int depth = 0; depth < 32; depth++)
    {
        if (cur == 0xFFFFFFFF) // -1
            break;

        // 
        uint32_t dist = pos - cur;
        if (dist > limit)
            break;

        uint32_t maxLen = srcSize_ - pos;

        if (maxLen > 273)
            maxLen = 273;

        uint32_t len = 0;
        while (len < maxLen && src_[pos+len] == src_[cur + len])
            len++;

        if (len > bestLen)
        {
            bestLen = len;
            bestDist = dist;
            if (len == 273) break;
        }
        cur = chain_[cur % dictSize_];
    }

    chain_[pos % dictSize_] = head;
    outDist = bestDist;
    return bestLen;
}
