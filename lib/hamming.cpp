#include "hamming.h"

#include <cstdint>

namespace hamarc
{

std::uint16_t EncodeByte(unsigned char value)
{
    int bits[13] = {};
    int data_pos[8] = {3, 5, 6, 7, 9, 10, 11, 12};

    for (int i = 0; i < 8; i++)
    {
        bits[data_pos[i]] = (value >> i) & 1;
    }

    for (int p = 1; p <= 8; p <<= 1)
    {
        int parity = 0;
        for (int i = 1; i <= 12; i++)
        {
            if ((i & p) != 0)
            {
                parity ^= bits[i];
            }
        }
        bits[p] = parity;
    }

    std::uint16_t code = 0;
    for (int i = 1; i <= 12; i++)
    {
        if (bits[i] != 0)
        {
            code |= (1u << (i - 1));
        }
    }

    return code;
}  // namespace hamarc

unsigned char DecodeByte(std::uint16_t code)
{
    int bits[13] = {};
    for (int i = 1; i <= 12; i++)
    {
        bits[i] = (code >> (i - 1)) & 1;
    }

    int error_pos = 0;
    for (int p = 1; p <= 8; p <<= 1)
    {
        int parity = 0;
        for (int i = 1; i <= 12; i++)
        {
            if ((i & p) != 0)
            {
                parity ^= bits[i];
            }
        }
        if (parity != 0)
        {
            error_pos += p;
        }
    }

    if (error_pos >= 1 && error_pos <= 12)
    {
        bits[error_pos] ^= 1;
    }

    int data_pos[8] = {3, 5, 6, 7, 9, 10, 11, 12};
    unsigned char value = 0;
    for (int i = 0; i < 8; i++)
    {
        value |= (bits[data_pos[i]] << i);
    }

    return value;
}

}
