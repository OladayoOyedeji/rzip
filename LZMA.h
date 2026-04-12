#ifndef FILE_H
#define FILE_H

#include "File.h"

#define RANGE 0xFFFFFFFF
#define LOW 0


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

#endif
