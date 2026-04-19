void LzmaEncSimple::BitTreeEncode(LzmaProb* tree, int bits, uint32_t sym)
{
    uint32_t m = 1;
    for (int i = bits - 1; i >= 0; i--) {
        uint32_t bit = (sym >> i) & 1;
        rc.EncodeBit(tree[m], bit);
        m = (m << 1) | bit;
    }
}

void LzmaEncSimple::EncodeLiteral(uint32_t pos, uint8_t byte)
{
    uint8_t prev = pos > 0 ? src[pos - 1] : 0;
    uint32_t litState = (prev >> (8 - lc));  // lp=0 so no position term
    LzmaProb* probs = litProbs + 0x300 * litState;
    BitTreeEncode(probs, 8, byte);
}

void LenEnc::Encode(RangeEncoder& rc, uint32_t posState, uint32_t len) {
    len -= 2;  // minimum match length is 2
    if (len < 8) {
        rc.EncodeBit(choice, 0);
        BitTreeEncode(rc, low[posState], 3, len);
    } else if (len < 16) {
        rc.EncodeBit(choice,  1);
        rc.EncodeBit(choice2, 0);
        BitTreeEncode(rc, mid[posState], 3, len - 8);
    } else {
        rc.EncodeBit(choice,  1);
        rc.EncodeBit(choice2, 1);
        BitTreeEncode(rc, high, 8, len - 16);
    }
}

// Distance → slot mapping table (standard LZMA)
static const uint8_t kDistSlot[128] = {
     0, 1, 2, 3, 4, 4, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7,
     8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9,
    10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
    11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,
    12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,
    12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,
    13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,
    13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,
};

uint32_t GetPosSlot(uint32_t dist) {
    if (dist < 128) return kDistSlot[dist];
    // for larger distances, count leading zeros
    uint32_t n = 0;
    uint32_t d = dist >> 7;
    if (d >= (1<<4)) { n += 4; d >>= 4; }
    if (d >= (1<<2)) { n += 2; d >>= 2; }
    n += (d >> 1);
    return n * 2 + ((dist >> (n - 1)) & 1) + 12;
}

void LzmaEncSimple::EncodeDistance(uint32_t dist, uint32_t len)
{
    uint32_t lenState = len - 2;
    if (lenState > 3) lenState = 3;  // clamped to 4 tables

    uint32_t slot = GetPosSlot(dist);
    BitTreeEncode(distSlot[lenState], 6, slot);

    if (slot >= 4) {
        uint32_t directBits = (slot >> 1) - 1;
        uint32_t base       = (2 | (slot & 1)) << directBits;
        uint32_t extra      = dist - base;

        if (slot < 14) {
            // encode direct bits raw (no prob model)
            rc.EncodeDirectBits(extra >> 4, directBits - 4);
        }
        // always encode low 4 bits through align tree
        BitTreeEncode(distAlign, 4, extra & 0xF);
    }
}

int LzmaEncSimple::FindMatch(uint32_t pos, uint32_t& outDist)
{
    uint32_t bestLen  = 0;
    uint32_t bestDist = 0;
    uint32_t limit    = pos < dictSize ? pos : dictSize;

    uint16_t h = ((src[pos] << 8) | src[pos + 1]);  // 2-byte hash key
    uint32_t cur = hashTable[h];
    hashTable[h] = pos;

    for (int depth = 0; depth < 32 && cur != 0xFFFFFFFF; depth++) {
        uint32_t dist = pos - cur;
        if (dist > limit) break;

        // Count match length
        uint32_t maxLen = srcSize - pos;
        if (maxLen > 273) maxLen = 273;
        uint32_t len = 0;
        while (len < maxLen && src[pos + len] == src[cur + len]) len++;

        if (len > bestLen) {
            bestLen  = len;
            bestDist = dist;
            if (len == 273) break;  // max length, stop
        }
        cur = chain[cur % dictSize];
    }

    chain[pos % dictSize] = hashTable[h];
    outDist = bestDist;
    return (int)bestLen;
}

bool LzmaEncSimple::Encode()
{
    WriteHeader();

    while (pos < srcSize) {
        uint32_t posState = pos & pbMask;
        uint32_t dist, len;
        len = FindMatch(pos, dist);

        if (len < 2) {
            // Literal
            rc.EncodeBit(isMatch[state][posState], 0);
            EncodeLiteral(pos, src[pos]);
            // state: literal after literal stays low, after match goes to 0
            static const uint32_t litNext[12] = {0,0,0,0,1,2,3,4,5,6,4,5};
            state = litNext[state];
            pos++;
        } else {
            // Match
            rc.EncodeBit(isMatch[state][posState], 1);
            rc.EncodeBit(isRep[state], 0);  // not a rep, always new match

            lenEnc.Encode(rc, posState, len);
            EncodeDistance(dist - 1, len);  // LZMA stores dist-1

            // update rep history
            reps[3] = reps[2]; reps[2] = reps[1];
            reps[1] = reps[0]; reps[0] = dist - 1;

            state = (state < 7) ? 7 : 10;
            pos += len;
        }
    }

    WriteEndMark();
    rc.Flush();
    return true;
}

void LzmaEncSimple::WriteHeader()
{
    // Properties byte: (pb*5 + lp)*9 + lc
    uint8_t props = (uint8_t)((pb * 5 + lp) * 9 + lc);
    rc.WriteByte(props);  // raw byte, before range encoding starts

    // Dict size (4 bytes LE)
    for (int i = 0; i < 4; i++)
        rc.WriteByte((dictSize >> (i * 8)) & 0xFF);

    // Uncompressed size: -1 means unknown (stream has end mark)
    for (int i = 0; i < 8; i++)
        rc.WriteByte(0xFF);
}

void LzmaEncSimple::WriteEndMark()
{
    // A match with dist=0xFFFFFFFF signals end of stream to the decoder
    uint32_t posState = pos & pbMask;
    rc.EncodeBit(isMatch[state][posState], 1);
    rc.EncodeBit(isRep[state], 0);
    lenEnc.Encode(rc, posState, 2);          // minimum length
    uint32_t slot = 63;                      // slot for 0xFFFFFFFF
    BitTreeEncode(distSlot[0], 6, slot);
    rc.EncodeDirectBits(0x1FFFFFF, 26);      // direct bits
    BitTreeEncode(distAlign, 4, 0xF);        // align bits
}
