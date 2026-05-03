#include "LZMAdec.h"

uint32_t BitTreeDecode(RangeDecoder & rc_, LzmaProb * tree, int bits)
{
    uint32_t m = 1;
    for (int i = 0; i < bits; ++i)
    {
        int bit = rc_.DecodeBit(tree[m]);
        // cout << "BTD m=" << m << " bit=" << bit << endl;
        m = (m << 1) | bit;
    }
    return m - (1 << bits);
}

static int decCallCount = 0;
int RangeDecoder::DecodeBit(LzmaProb & prob)
{
    int bit;
    uint32_t bound = (range_ >> 11) * prob;
    
    
    if (code_ < bound)
    {
        range_ = bound;
        prob += (2048 - prob) >> 5;
        bit = 0;
    }
    else
    {
        code_ -= bound;
        range_ -= bound;
        prob -= prob >> 5;
        bit = 1;
    }

    if (range_ < (1 << 24))
    {
        range_ <<= 8;
        code_ = (code_ << 8) | buf_[bufPos_++];
    }
    // decCallCount++;
    // cout << "DEC call#" << decCallCount << " range_=" << range_
    //      << " prob=" << prob << " bit=" << bit << endl;
    
    return bit;
}

uint32_t RangeDecoder::DecodeDirectBits(int numBits)
{
    uint32_t result = 0;

    for (int i = numBits - 1; i >= 0; i--)
    {
        range_ >>= 1;

        uint32_t bit = (code_ >= range_) ? 1 : 0;
        if (bit)
            code_ -= range_;

        result |= (bit << i);

        if (range_ < (1 << 24))
        {
            range_ <<= 8;
            code_   = (code_ << 8) | buf_[bufPos_++];
        }
    }
    return result;
}

uint32_t LenDec::Decode(RangeDecoder & rc, uint32_t posState)
{
    if (rc.DecodeBit(choice_) == 0)
        return BitTreeDecode(rc, low_[posState], 3) + 2;
    if (rc.DecodeBit(choice2_) == 0)
        return BitTreeDecode(rc, mid_[posState], 3) + 10;
    return BitTreeDecode(rc, high_, 8) + 18;
}

void LzmaDec::Decode()
{
    static const uint32_t litNext[12] = {0,0,0,0,1,2,3,4,5,6,4,5};
    bool finished = false;

    while (!finished && outPos_ < outSize_)
    {
        cout << outPos_ << endl;
        uint32_t posState = outPos_ & ((1 << pb_) - 1);
        
        int bit = rc_.DecodeBit(isMatch_[state_][posState]);
        cout << "bit " << bit << endl;
        if (bit == 0)
        {
            uint8_t prev    = outPos_ > 0 ? out_[outPos_ - 1] : 0;
            uint8_t literal = DecodeLiteral(prev);
            cout << "literal " << (int)literal << literal << endl;
            PutByte(literal);
            state_  = litNext[state_];
        }
        else
        {
            uint32_t len  = lenDec_.Decode(rc_, posState);
            uint32_t dist = DecodeDistance(len);
            cout << "length: " << len << "distance: " << dist << endl;
            if (dist == 0xFFFFFFFF) {
                finished = true;
                break;
            }

            for (uint32_t i = 0; i < len; i++)
            {
                PutByte(GetByte(dist + 1));
            }

            state_   = state_ < 7 ? 7 : 10;
        }
    }

    // write decoded output to file
    // File f("decompressed");
    // f.mywrite(out_, outPos_);
    // f.myclose();

    // // print decoded bytes
    // Pair p;
    // p.first  = out_;
    // p.second = outPos_;
    // std::cout << "[" << p << ']' << " decoded=" << outPos_ << " bytes" << std::endl;
}

uint8_t LzmaDec::DecodeLiteral(uint8_t prev)
{
    uint32_t litState = prev >> (8 - lc_);
    LzmaProb * probs = litProb_ + 0x300 * litState;
    return (uint8_t)BitTreeDecode(rc_, probs, 8);
}

uint32_t LzmaDec::DecodeDistance(uint32_t len)
{
    uint32_t lenState = len - 2;
    if (lenState > 3) lenState = 3;

    uint32_t slot = BitTreeDecode(rc_, distSlot_[lenState], 6);

    if (slot < 4) return slot;

    uint32_t directBits = (slot >> 1) - 1;
    uint32_t base = (2 | (slot & 1)) << directBits;
    uint32_t extra = 0;

    if (directBits > 4)
    {
        extra = rc_.DecodeDirectBits(directBits - 4) << 4;
        
    }

    extra |= BitTreeDecode(rc_, distAlign_, 4);

    return base + extra;
}
