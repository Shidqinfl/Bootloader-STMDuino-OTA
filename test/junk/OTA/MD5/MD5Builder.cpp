#include "MD5Builder.h"
#include <string.h>

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21



static unsigned char PADDING[] = {
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void MD5Builder::MD5Init(MD5_CTX *context) {
    context->count[0] = context->count[1] = 0;
    context->state[0] = 0x67452301;
    context->state[1] = 0xefcdab89;
    context->state[2] = 0x98badcfe;
    context->state[3] = 0x10325476;
}

void MD5Builder::MD5Update(MD5_CTX *context, uint8_t *input, uint32_t inputLen) {
    uint32_t i, index, partLen;

    index = (uint32_t)((context->count[0] >> 3) & 0x3F);

    if ((context->count[0] += ((uint32_t)inputLen << 3)) < ((uint32_t)inputLen << 3)) {
        context->count[1]++;
    }
    context->count[1] += ((uint32_t)inputLen >> 29);

    partLen = 64 - index;

    if (inputLen >= partLen) {
        MD5_memcpy(&context->buffer[index], input, partLen);
        MD5Transform(context->state, context->buffer);

        for (i = partLen; i + 63 < inputLen; i += 64) {
            MD5Transform(context->state, &input[i]);
        }
        index = 0;
    } else {
        i = 0;
    }

    MD5_memcpy(&context->buffer[index], &input[i], inputLen - i);
}

void MD5Builder::MD5Final(uint8_t digest[16], MD5_CTX *context) {
    uint8_t bits[8];
    uint32_t index, padLen;

    Encode(bits, context->count, 8);

    index = (uint32_t)((context->count[0] >> 3) & 0x3f);
    padLen = (index < 56) ? (56 - index) : (120 - index);
    MD5Update(context, PADDING, padLen);

    MD5Update(context, bits, 8);

    Encode(digest, context->state, 16);

    MD5_memset((uint8_t *)context, 0, sizeof(*context));
}

void MD5Builder::MD5Transform(uint32_t state[4], uint8_t block[64]) {
    uint32_t a = state[0], b = state[1], c = state[2], d = state[3], x[16];

    Decode(x, block, 64);

    // Perform the MD5 algorithm main steps
    // (the F, G, H, and I transformations)

    // Update state variables a, b, c, and d using transformation functions
    // [...]

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;

    MD5_memset((uint8_t *)x, 0, sizeof(x));
}

void MD5Builder::Encode(uint8_t *output, uint32_t *input, uint32_t len) {
    uint32_t i, j;

    for (i = 0, j = 0; j < len; i++, j += 4) {
        output[j] = (uint8_t)(input[i] & 0xff);
        output[j+1] = (uint8_t)((input[i] >> 8) & 0xff);
        output[j+2] = (uint8_t)((input[i] >> 16) & 0xff);
        output[j+3] = (uint8_t)((input[i] >> 24) & 0xff);
    }
}

void MD5Builder::Decode(uint32_t *output, uint8_t *input, uint32_t len) {
    uint32_t i, j;

    for (i = 0, j = 0; j < len; i++, j += 4) {
        output[i] = ((uint32_t)input[j]) | (((uint32_t)input[j+1]) << 8) |
                    (((uint32_t)input[j+2]) << 16) | (((uint32_t)input[j+3]) << 24);
    }
}

void MD5Builder::MD5_memcpy(uint8_t *output, uint8_t *input, uint32_t len) {
    memcpy(output, input, len);
}

void MD5Builder::MD5_memset(uint8_t *output, int value, uint32_t len) {
    memset(output, value, len);
}
