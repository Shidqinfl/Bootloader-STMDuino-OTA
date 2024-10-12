#ifndef _MD5_H_
#define _MD5_H_

#include <stdint.h>

typedef struct {
    uint32_t state[4];
    uint32_t count[2];
    uint8_t buffer[64];
} MD5_CTX;
extern MD5_CTX ctx_md5;
class MD5Builder
{
private:
    void MD5Transform(uint32_t state[4], uint8_t block[64]);
    void Encode(uint8_t *output, uint32_t *input, uint32_t len);
    void Decode(uint32_t *output, uint8_t *input, uint32_t len);
    void MD5_memcpy(uint8_t *output, uint8_t *input, uint32_t len);
    void MD5_memset(uint8_t *output, int value, uint32_t len);
public:
    void MD5Init(MD5_CTX *context);
    void MD5Update(MD5_CTX *context, uint8_t *input, uint32_t inputLen);
    void MD5Final(uint8_t digest[16], MD5_CTX *context);
};





#endif
