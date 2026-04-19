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
