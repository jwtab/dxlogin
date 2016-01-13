

#include "StdAfx.h"
#include "Base64.h"

#define DW_EOL  "\n"
#define MAXLINE  2048   //每多少字符换行

static char base64tab[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz0123456789+/";

static char base64idx[128] = {
    '\377','\377','\377','\377','\377','\377','\377','\377',
    '\377','\377','\377','\377','\377','\377','\377','\377',
    '\377','\377','\377','\377','\377','\377','\377','\377',
    '\377','\377','\377','\377','\377','\377','\377','\377',
    '\377','\377','\377','\377','\377','\377','\377','\377',
    '\377','\377','\377',    62,'\377','\377','\377',    63,
        52,    53,    54,    55,    56,    57,    58,    59,
        60,    61,'\377','\377','\377','\377','\377','\377',
    '\377',     0,     1,     2,     3,     4,     5,     6,
         7,     8,     9,    10,    11,    12,    13,    14,
        15,    16,    17,    18,    19,    20,    21,    22,
        23,    24,    25,'\377','\377','\377','\377','\377',
    '\377',    26,    27,    28,    29,    30,    31,    32,
        33,    34,    35,    36,    37,    38,    39,    40,
        41,    42,    43,    44,    45,    46,    47,    48,
        49,    50,    51,'\377','\377','\377','\377','\377'
};

static char hextab[] = "0123456789ABCDEF";

#ifdef __cplusplus
inline int isbase64(int a) {
    return ('A' <= a && a <= 'Z')
        || ('a' <= a && a <= 'z')
        || ('0' <= a && a <= '9')
        || a == '+' || a == '/';
}
#else
#define isbase64(a) (  ('A' <= (a) && (a) <= 'Z') \
                    || ('a' <= (a) && (a) <= 'z') \
                    || ('0' <= (a) && (a) <= '9') \
                    ||  (a) == '+' || (a) == '/'  )
#endif

int encode_base64(const char* aIn, size_t aInLen, char* aOut,
    size_t aOutSize, size_t* aOutLen)
{
    if (!aIn || !aOut || !aOutLen)
        return -1;
    size_t inLen = aInLen;
    char* out = aOut;
    size_t outSize = (inLen+2)/3*4;     /* 3:4 conversion ratio */
    outSize += strlen(DW_EOL)*outSize/MAXLINE + 2;  /* Space for newlines and NUL */
    if (aOutSize < outSize)
        return -1;
    size_t inPos  = 0;
    size_t outPos = 0;
    int c1, c2, c3;
    int lineLen = 0;
    /* Get three characters at a time and encode them. */
    for (size_t i=0; i < inLen/3; ++i) {
        c1 = aIn[inPos++] & 0xFF;
        c2 = aIn[inPos++] & 0xFF;
        c3 = aIn[inPos++] & 0xFF;
        out[outPos++] = base64tab[(c1 & 0xFC) >> 2];
        out[outPos++] = base64tab[((c1 & 0x03) << 4) | ((c2 & 0xF0) >> 4)];
        out[outPos++] = base64tab[((c2 & 0x0F) << 2) | ((c3 & 0xC0) >> 6)];
        out[outPos++] = base64tab[c3 & 0x3F];
        lineLen += 4;
        if (lineLen >= MAXLINE-3) {
            const char* cp = DW_EOL;
            out[outPos++] = *cp++;
            if (*cp) {
                out[outPos++] = *cp;
            }
            lineLen = 0;
        }
    }
    /* Encode the remaining one or two characters. */
    const char* cp;
    switch (inLen % 3) {
    case 0:
        cp = DW_EOL;
        out[outPos++] = *cp++;
        if (*cp) {
            out[outPos++] = *cp;
        }
        break;
    case 1:
        c1 = aIn[inPos] & 0xFF;
        out[outPos++] = base64tab[(c1 & 0xFC) >> 2];
        out[outPos++] = base64tab[((c1 & 0x03) << 4)];
        out[outPos++] = '=';
        out[outPos++] = '=';
        cp = DW_EOL;
        out[outPos++] = *cp++;
        if (*cp) {
            out[outPos++] = *cp;
        }
        break;
    case 2:
        c1 = aIn[inPos++] & 0xFF;
        c2 = aIn[inPos] & 0xFF;
        out[outPos++] = base64tab[(c1 & 0xFC) >> 2];
        out[outPos++] = base64tab[((c1 & 0x03) << 4) | ((c2 & 0xF0) >> 4)];
        out[outPos++] = base64tab[((c2 & 0x0F) << 2)];
        out[outPos++] = '=';
        cp = DW_EOL;
        out[outPos++] = *cp++;
        if (*cp) {
            out[outPos++] = *cp;
        }
        break;
    }
    out[outPos] = 0;
    *aOutLen = outPos;
    return 0;
}

int decode_base64(const char* aIn, size_t aInLen, char* aOut,
    size_t aOutSize, size_t* aOutLen)
{
    if (!aIn || !aOut || !aOutLen)
        return -1;
    size_t inLen = aInLen;
    char* out = aOut;
    size_t outSize = (inLen+3)/4*3;
    if (aOutSize < outSize)
        return -1;
    /* Get four input chars at a time and decode them. Ignore white space
     * chars (CR, LF, SP, HT). If '=' is encountered, terminate input. If
     * a char other than white space, base64 char, or '=' is encountered,
     * flag an input error, but otherwise ignore the char.
     */
    int isErr = 0;
    int isEndSeen = 0;
    int b1, b2, b3;
    int a1, a2, a3, a4;
    size_t inPos = 0;
    size_t outPos = 0;
    while (inPos < inLen) {
        a1 = a2 = a3 = a4 = 0;
        while (inPos < inLen) {
            a1 = aIn[inPos++] & 0xFF;
            if (isbase64(a1)) {
                break;
            }
            else if (a1 == '=') {
                isEndSeen = 1;
                break;
            }
            else if (a1 != '\r' && a1 != '\n' && a1 != ' ' && a1 != '\t') {
                isErr = 1;
            }
        }
        while (inPos < inLen) {
            a2 = aIn[inPos++] & 0xFF;
            if (isbase64(a2)) {
                break;
            }
            else if (a2 == '=') {
                isEndSeen = 1;
                break;
            }
            else if (a2 != '\r' && a2 != '\n' && a2 != ' ' && a2 != '\t') {
                isErr = 1;
            }
        }
        while (inPos < inLen) {
            a3 = aIn[inPos++] & 0xFF;
            if (isbase64(a3)) {
                break;
            }
            else if (a3 == '=') {
                isEndSeen = 1;
                break;
            }
            else if (a3 != '\r' && a3 != '\n' && a3 != ' ' && a3 != '\t') {
                isErr = 1;
            }
        }
        while (inPos < inLen) {
            a4 = aIn[inPos++] & 0xFF;
            if (isbase64(a4)) {
                break;
            }
            else if (a4 == '=') {
                isEndSeen = 1;
                break;
            }
            else if (a4 != '\r' && a4 != '\n' && a4 != ' ' && a4 != '\t') {
                isErr = 1;
            }
        }
        if (isbase64(a1) && isbase64(a2) && isbase64(a3) && isbase64(a4)) {
            a1 = base64idx[a1] & 0xFF;
            a2 = base64idx[a2] & 0xFF;
            a3 = base64idx[a3] & 0xFF;
            a4 = base64idx[a4] & 0xFF;
            b1 = ((a1 << 2) & 0xFC) | ((a2 >> 4) & 0x03);
            b2 = ((a2 << 4) & 0xF0) | ((a3 >> 2) & 0x0F);
            b3 = ((a3 << 6) & 0xC0) | ( a4       & 0x3F);
            out[outPos++] = char(b1);
            out[outPos++] = char(b2);
            out[outPos++] = char(b3);
        }
        else if (isbase64(a1) && isbase64(a2) && isbase64(a3) && a4 == '=') {
            a1 = base64idx[a1] & 0xFF;
            a2 = base64idx[a2] & 0xFF;
            a3 = base64idx[a3] & 0xFF;
            b1 = ((a1 << 2) & 0xFC) | ((a2 >> 4) & 0x03);
            b2 = ((a2 << 4) & 0xF0) | ((a3 >> 2) & 0x0F);
            out[outPos++] = char(b1);
            out[outPos++] = char(b2);
            break;
        }
        else if (isbase64(a1) && isbase64(a2) && a3 == '=' && a4 == '=') {
            a1 = base64idx[a1] & 0xFF;
            a2 = base64idx[a2] & 0xFF;
            b1 = ((a1 << 2) & 0xFC) | ((a2 >> 4) & 0x03);
            out[outPos++] = char(b1);
            break;
        }
        else {
            break;
        }
        if (isEndSeen) {
            break;
        }
    } /* end while loop */
    *aOutLen = outPos;
    return (isErr) ? -1 : 0;
}