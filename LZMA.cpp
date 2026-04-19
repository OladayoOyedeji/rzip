
void RangeEncoder::EncodeBit(uint16_t &prob, bool bit)
{
    // 1. Calculate the split point (bound)
    // range >> 11 is essentially dividing by 2048
    uint32_t bound = (range >> 11) * prob;

    if (!bit) // Encoding a 0
    {
        // Keep the lower part of the range
        range = bound;
        
        // Increase probability: Move prob closer to 2048
        // This makes '0' more likely next time
        prob += (2048 - prob) >> 5;
    }
    else // Encoding a 1
    {
        // Keep the upper part of the range
        low += bound;
        range -= bound;
        
        // Decrease probability: Move prob closer to 0
        // This makes '1' more likely next time
        prob -= prob >> 5;
    }

    // 3. Normalization
    // If range gets too small (< 2^24), we must shift out a byte
    while (range < (1 << 24))
    {
        // Shift the top 8 bits of 'low' to the output
        output_byte((uint8_t)(low >> 24));
        
        range <<= 8;
        low <<= 8;
    }
}

void LzmaEnc::ResetProbs() {
    // 1. Reset the general decision tables
    // These are 12 states x 16 posStates
    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 16; j++) {
            isMatch[i][j] = 1024;
            isRep0Long[i][j] = 1024;
        }
        isRep[i] = 1024;
        isRepG0[i] = 1024;
        isRepG1[i] = 1024;
        isRepG2[i] = 1024;
    }

    // 2. Reset the Length Encoders
    // You must reset the choice bits and the bit-trees (low, mid, high)
    ResetLenEncoder(lenEnc);
    ResetLenEncoder(repLenEnc);

    // 3. Reset Distance/Position Slot probabilities
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 64; j++) {
            posSlotEncoder[i][j] = 1024;
        }
    }
    
    for (int i = 0; i < 114; i++) posEncoders[i] = 1024;
    for (int i = 0; i < 16; i++) posAlignEncoder[i] = 1024;

    // 4. Reset Literals
    // This is your dynamically allocated array
    uint32_t numLitProbs = 0x300 << (lc + lp);
    for (uint32_t i = 0; i < numLitProbs; i++) {
        litProbs[i] = 1024;
    }
}

LzmaEnc::LzmaEnc() 
    : state(0), processedSize(0), additionalOffset(0) 
{
    // 1. Set default properties (can be changed later via a SetProps method)
    lc = 3; 
    lp = 0; 
    pb = 2;
    pbMask = (1 << pb) - 1;

    // 2. Initial History: LZMA starts with all distances at 1
    for (int i = 0; i < 4; i++) reps[i] = 1;

    // 3. Reset all general probability tables to 1024 (50% chance)
    // You can use a helper function to 'Fill' these with 1024
    ResetProbs();

    // 4. Allocate Literals (0x300 is the size of one literal tree)
    uint32_t numLitProbs = 0x300 << (lc + lp);
    litProbs = new uint16_t[numLitProbs];
    
    for (uint32_t i = 0; i < numLitProbs; i++) {
        litProbs[i] = 1024;
    }
}

void LzmaEnc::EncodeOneToken()
{
    uint32_t posState = processedSize & pbMask;
    uint32_t len;
    uint32_t dist;

    // 1. Get the best match from your BST MatchFinder
    // If no match found, mf returns len=0 and dist=0xFFFFFFFF
    mf.GetBestMatch(len, dist);

    if (dist == 0xFFFFFFFF)
    { 
        // --- LITERAL PATH ---
        rc.EncodeBit(isMatch[state][posState], 0);
        
        uint8_t curByte = mf.GetByte(0);
        uint8_t prevByte = mf.GetByte(-1);
        
        EncodeLiteral(processedSize, curByte, prevByte);
        
        state = kLiteralNextStates[state];
        processedSize++;
    } 
    else
    {
        // --- MATCH PATH ---
        rc.EncodeBit(isMatch[state][posState], 1);

        // Is it a Repeated Distance?
        bool isRepMatch = false;
        int repIndex = -1;
        
        for (int i = 0; i < 4; i++)
        {
            if (dist == reps[i])
            {
                isRepMatch = true;
                repIndex = i;
                break;
            }
        }

        if (isRepMatch)
            EncodeRepMatch(repIndex, len, posState);
        else
            EncodeNewMatch(dist, len, posState);

        processedSize += len;
    }
}

void LzmaEnc::EncodeLiteral(LzmaProb & probs, uint32_t sym)
{
    uint32_t range = rangeEncoder.range;
    sym |= 0x100;
    while (sym < 0x10000)
    {
        LzmaProb * prob = probs + (sym >> 8);
        uint32_t bit = (sym >> 7) & 1;
        sym <<= 1;
        rangeEncoder.EncodeBit(prob, bit);
        
    }
    rangeEncoder.range = range;
}

void Lzma::EncodeMatched(LzmaProb & probs, uint32_t sym, uint32_t matchbyte)
{
    uint32_t range = rangeEncoder.range;
    uint32_t offs = 0x100;
    sym |= 0x100;
    do
    {
        matchByte <<= 1;

        LzmaProb prob = probs + (offs + (matchByte & offs) + (symm >> 8));
        uint32_t bit = (sym >> 7) & 1;
        sym <<= 1;
        offs &= ~(matchByte ^ sym);
        rangeEncoder.EncodeBit(prob, bit);
    }
    while (sym < 0x100000);
    rangeEncoder.range = range;
}

int LzmaEnc::Encode()
{
    int res = SZ_OK;

    while (!finished) {
        // Compress a chunk of data
        res = CodeOneBlock(); 
        
        if (res != SZ_OK) return res;

        // Optional: Print status to your 32-bit system's console
        printf("Processed: %llu bytes\n", nowPos64);
    }

    // Wrap everything up
    Finish(); 
    return SZ_OK;
}

void encode_bit(bool bit, uint16_t & prob_1,
                uint32_t range, uint32_t low)
{
    uint32_t b = (range >> 11) * prob_1;
    if (bit)
    {
        low +=  b;
        range -= b;
        prob -= prob >> 5;
    }
    else
    {
        prob_1 = (2048 - prob) >> 5;
        range = b;
    }

    while (range < (i << 24))
    {
        output_byte(low >> 24);
        range <<= 8;
        low <<= 8;
    }
}
