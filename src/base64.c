
#include <string.h>
#include <malloc.h>
  
#ifdef __cplusplus
extern "C" {
#endif
  
static const char b64_tab[65] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  
char*
base64_encode(const unsigned char *inp, int in_len, int *out_len)
{
    int rval_len = (((in_len + 2) / 3) * 4);
    unsigned char* rval = calloc(1, rval_len + 1);
    unsigned char* pout = (unsigned char*)rval;
    unsigned int i, acc = 0, bits = 0;
    memset((void*)rval, '=', rval_len);
    *((char*)rval+rval_len) = '\0';
    for(i = 0; i < in_len; i++) {
        acc = (acc << 8) | (inp[i] & 0xFFU);
        bits += 8;
        while(bits >= 6) {
            bits -= 6;
            *pout = b64_tab[(acc >> bits) & 0x3FU];
            pout++;
            if(pout > (rval + rval_len)) {
                free((void*)rval);
                return NULL;
            }
        }
    }
    if(bits > 0) {
        acc <<= 6 - bits;
        *pout = b64_tab[acc & 0x3FU];
    }
    if(out_len) {
        *out_len = strlen(rval);
    }
    return (char*)rval;
}
  
static const char reverse_tab[128] = {
   64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
   64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
   64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
   52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
   64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
   15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
   64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
   41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64
};
  
char*
base64_decode(const unsigned char *inp, int in_len)
{
    char *rval = calloc(1, in_len), *pout = rval;
    unsigned int i, acc = 0, bits = 0, outpos = 0;
    memset(rval, 0, in_len);
    for(i = 0; i < in_len; i++) {
        if(inp[i] == ' ' || inp[i] == '=') continue;
        if((inp[i] > 127) || (inp[i] < 0) || (reverse_tab[inp[i]] > 63)) {
            free(rval);
            return NULL;
        }
        acc = (acc << 6) | reverse_tab[inp[i]];
        bits += 6;
        if(bits >= 8) {
            bits -= 8;
            *pout = (char)((acc >> bits) & 0xFFU);
            pout++;
        }
   
    }
    return rval;
}
  
#ifdef __cplusplus
}
#endif

