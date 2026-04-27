// Everything you actually need, nothing else
class LzmaEncSimple {
public:
    LzmaEncSimple(uint8_t* in, size_t inSize, uint8_t* out, size_t outSize);
    bool Encode();

private:
    // --- Input ---
    uint8_t*  src;
    size_t    srcSize;
    uint32_t  pos;

    // --- Output ---
    RangeEncoder rc;

    // --- LZMA properties ---
    uint32_t lc = 3, lp = 0, pb = 2;
    uint32_t pbMask;

    // --- State ---
    uint32_t state;
    uint32_t reps[4];

    // --- Probability tables ---
    LzmaProb isMatch  [12][16];
    LzmaProb isRep    [12];
    LzmaProb isRepG0  [12];
    LzmaProb isRepG1  [12];
    LzmaProb isRepG2  [12];
    LzmaProb isRep0Long[12][16];
    LzmaProb litProbs [1 << (8 + 3)]; // fixed lc=3,lp=0: 0x300 * 8
    LenEnc   lenEnc;
    LzmaProb distSlot [4][64];         // pos slot trees
    LzmaProb distAlign[16];            // 4-bit align tree

    // --- Hash chain match finder ---
    static const uint32_t HASH_SIZE = 1 << 16;
    uint32_t hashTable[HASH_SIZE];
    uint32_t chain    [1 << 22];       // size to your max dict
    uint32_t dictSize = 1 << 22;

    // --- Internal ---
    void     WriteHeader();
    void     WriteEndMark();
    void     ResetProbs();
    uint32_t Hash2(uint32_t pos);
    int      FindMatch(uint32_t pos, uint32_t& outDist);  // returns best len
    void     EncodeLiteral (uint32_t pos, uint8_t byte);
    void     EncodeNewMatch(uint32_t dist, uint32_t len, uint32_t posState);
    void     EncodePosSlot (uint32_t dist, uint32_t lenState);
    void     EncodeDistance(uint32_t dist, uint32_t len);
    void     BitTreeEncode (LzmaProb* tree, int bits, uint32_t sym);
};

class RangeEncoder
{
public:
    RangeEncoder()
        : buf(nullptr), bufSize(0), bufPos(0),
          low(0), range(0xFFFFFFFF), cacheSize(1), cache(0)
    {}

    void Init(uint8_t* outBuf, size_t outSize)
    {
        buf     = outBuf;
        bufSize = outSize;
        bufPos  = 0;
        low      = 0;
        range    = 0xFFFFFFFF;
        cacheSize = 1;
        cache    = 0;
    }

    void EncodeBit(uint16_t& prob, int bit)
    {
        uint32_t bound = (range >> 11) * prob;

        if (bit == 0) {
            range  = bound;
            prob  += (2048 - prob) >> 5;
        } else {
            low   += bound;
            range -= bound;
            prob  -= prob >> 5;
        }

        if (range < (1 << 24)) {
            range <<= 8;
            ShiftLow();
        }
    }

    void EncodeDirectBits(uint32_t value, int numBits)
    {
        for (int i = numBits - 1; i >= 0; i--) {
            range >>= 1;
            low   += range & (0 - ((value >> i) & 1));
            if (range < (1 << 24)) {
                range <<= 8;
                ShiftLow();
            }
        }
    }

    void Flush()
    {
        for (int i = 0; i < 5; i++)
            ShiftLow();
    }

    size_t GetOutputSize() { return bufPos; }

private:
    uint8_t*  buf;
    size_t    bufSize;
    size_t    bufPos;

    uint64_t  low;        // 64-bit to safely absorb carries
    uint32_t  range;
    uint32_t  cacheSize;  // how many 0xFF bytes are pending
    uint8_t   cache;      // last byte waiting to be emitted

    void WriteByte(uint8_t byte)
    {
        buf[bufPos++] = byte;
    }

    void ShiftLow()
    {
        if ((uint32_t)low < 0xFF000000 || (low >> 32) != 0)
        {
            uint8_t temp = cache;
            do {
                WriteByte(temp + (uint8_t)(low >> 32));
                temp = 0xFF;
                cacheSize--;
            } while (cacheSize != 0);

            cache = (uint8_t)((uint32_t)low >> 24);
        }
        else
        {
            cacheSize++;
        }

        low = (uint32_t)(low << 8);
    }
};
