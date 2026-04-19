#ifndef FILE_H
#define FILE_H

#include "File.h"

#define RANGE 0xFFFFFFFF
#define LOW 0

typedef uint16_t LzmaProb;

class LenEnc
{
    LzmaProb choice;                          // 0: use low,  1: go to choice2
    LzmaProb choice2;                         // 0: use mid,  1: use high
    LzmaProb low [1 << kNumPosBitsMax][8];    // [posState][symbol], 3 bits
    LzmaProb mid [1 << kNumPosBitsMax][8];    // [posState][symbol], 3 bits
    LzmaProb high[256];                       // 8 bits, covers lengths 18-273
};

class LzmaEnc
{
public:
    LzmaEnc();
    ~LzmaEnc();
    void EncodeOneToken();
    void UpdateState(int matchType);
    void EncodeLiteral(uint32_t pos, uint8_t cur, uint8_t prev);
    void EncodePosSlot(uint32_t dist, int lenState);
    int SetProbs(const LzmaProb * probs);
    void SetDataSize(uint64_t expectedDataSize);
    int WriteProperties(unsigned char * properties, unsigned int * size);
    unsigned IsWriteEndMark();
    int Encode();
private:
    BST_MatchFinder mf;
    uint32_t dictSize;
    uint32_t fb;

    RangeEncoder rangeEncoder;

    uint32_t state;
    uint32_t lc, lp, pb;
    uint8_t prevByte;

    LzmaProb isMatch[kNumStates][1 << kNumPosBitsMax];
    LzmaProb isRep[kNumStates];
    LzmaProb isRepG0[kNumStates];
    LzmaProb isRepG1[kNumStates];
    LzmaProb isRepG2[kNumStates];
    LzmaProb isRep0Long[kNumStates][1 << kNumPosBitsMax]; // don't forget this one

    std::vector<LzmaProb> litProbs; // sized to (1 << (lc+lp)) * 0x300 on init

    LenEnc lenProbs;                // not uint16_t array — use the class
    LenEnc repLenProbs;

    LzmaProb distSlotProbs[kNumLenToPosStates][1 << kNumPosSlotBits];
    LzmaProb distProbs[kNumFullDistances];  // or name it alignProbs for the low bits

    uint64_t processedSize;
};

#endif
