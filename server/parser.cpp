#include "parser.h"

string int_to_fb(const uint32_t n)
{
    string result = "";
    result += (unsigned char)((n >> 24) & 0xFF);
    result += (unsigned char)((n >> 16) & 0xFF);
    result += (unsigned char)((n >> 8) & 0xFF);
    result += (unsigned char)(n & 0xFF);
    return result;
}

string long_to_eb(const uint64_t n)
{
    string result = "";
    result += (unsigned char)((n >> 56) & 0xFF);
    result += (unsigned char)((n >> 48) & 0xFF);
    result += (unsigned char)((n >> 40) & 0xFF);
    result += (unsigned char)((n >> 32) & 0xFF);
    result += (unsigned char)((n >> 24) & 0xFF);
    result += (unsigned char)((n >> 16) & 0xFF);
    result += (unsigned char)((n >> 8) & 0xFF);
    result += (unsigned char)(n & 0xFF);
    return result;
}

uint32_t fb_to_int(const string n)
{
    if (n.length() != 4)
        return 0;
    uint32_t res = 0;
    for (uint32_t i = 0; i < 4; i++)
    {
        res << 8;
        res += (uint32_t)n[i];
    }
    return res;
}

uint64_t eb_to_long(const string n)
{
    if (n.length() != 8)
        return 0;
    uint64_t res = 0;
    for (uint32_t i = 0; i < 8; i++)
    {
        res << 8;
        res += (uint64_t)n[i];
    }
    return res;
}

int control_sum(const string s)
{
    //TODO
    return 0;
}
