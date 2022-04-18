
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "Utils/MD5.h"
#include "openssl/md5.h"

namespace Uface {
namespace Application {

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define SWAP_LE32(x) (x)
#else
#define SWAP_LE32(x) bswap_32(x)
#endif

# define FF(b, c, d) (d ^ (b & (c ^ d)))
# define FG(b, c, d) FF (d, b, c)
# define FH(b, c, d) (b ^ c ^ d)
# define FI(b, c, d) (c ^ (b | ~d))

typedef struct {
    uint32_t    A;
    uint32_t    B;
    uint32_t    C;
    uint32_t    D;
    uint64_t    total;
    uint32_t    buffLen;
    char        buffer[128];
} MD5Ctx;

static void md5Begin(MD5Ctx *ctx);
static void md5Hash(const void *data, size_t length, MD5Ctx *ctx);
static void md5End(void *resbuf, MD5Ctx *ctx);
/* Hash a single block, 64 bytes long and 4-byte aligned. */
static void md5HashBlock(const void *buffer, MD5Ctx *ctx);
static int32_t md5InnerFile(const char *file, uint8_t *buffer, int32_t bufferLength, uint8_t *output, MD5Ctx *ctx);

int32_t md5File(const char *pathname, char *md5data) {

    const static int32_t FIX_ELEM_LENGTH = 16;
    MD5Ctx ctx;
    uint8_t buf[4096] = {0};
    uint8_t readbuf[FIX_ELEM_LENGTH] = {0};
    if (md5InnerFile(pathname,buf, sizeof(buf),readbuf,&ctx) < 0) {
        return -1;
    }

    char temp[3] = {0};
    for (int32_t index= 0; index < FIX_ELEM_LENGTH; index++) {
        memset(temp, 0, sizeof(temp));
        snprintf(temp,sizeof(temp) - 1,"%02x", readbuf[index]);
        strcat(md5data, temp);
    }

    return 0;
}

static void md5Begin(MD5Ctx *ctx) {
    ctx->A = 0x67452301;
    ctx->B = 0xefcdab89;
    ctx->C = 0x98badcfe;
    ctx->D = 0x10325476;

    ctx->total = 0;
    ctx->buffLen = 0;
    memset(ctx->buffer,0x00, sizeof(ctx->buffer));
}

static void md5Hash(const void *data, size_t length, MD5Ctx *ctx) {

    char *buffer = (char *)data;
    ctx->total += length;

    while (length > 0) {
        uint32_t index = 64 - ctx->buffLen;
        // Copy data into aligned buffer.

        if (index > length) {
            index = length;
        }

        memcpy(ctx->buffer + ctx->buffLen, buffer, index);
        length -= index;
        ctx->buffLen += index;
        buffer += index;

        // When buffer fills up, process it.
        if (ctx->buffLen == 64) {
            md5HashBlock(ctx->buffer, ctx);
            ctx->buffLen = 0;
        }
    }
}

static void md5End(void *resbuf, MD5Ctx *ctx) {
    char *buffer = ctx->buffer;
    int32_t i;

    /* Pad data to block size.  */

    buffer[ctx->buffLen++] = 0x80;
    memset(buffer + ctx->buffLen, 0, sizeof(ctx->buffer) - ctx->buffLen);

    /* Put the 64-bit file length in *bits* at the end of the buffer.  */
    ctx->total <<= 3;
    if (ctx->buffLen > 56) {
        buffer += 64;
    }

    for (i = 0; i < 8; i++) {
        buffer[56 + i] = ctx->total >> (i * 8);
    }

    /* Process last bytes.  */
    if (buffer != ctx->buffer) {
        md5HashBlock(ctx->buffer, ctx);
    }

    md5HashBlock(buffer, ctx);

    /* Put result from CTX in first 16 bytes following RESBUF.  The result is
     * always in little endian byte order, so that a byte-wise output yields
     * to the wanted ASCII representation of the message digest.
     *
     * IMPORTANT: On some systems it is required that RESBUF is correctly
     * aligned for a 32 bits value.
     */
    ((uint32_t *) resbuf)[0] = SWAP_LE32(ctx->A);
    ((uint32_t *) resbuf)[1] = SWAP_LE32(ctx->B);
    ((uint32_t *) resbuf)[2] = SWAP_LE32(ctx->C);
    ((uint32_t *) resbuf)[3] = SWAP_LE32(ctx->D);
}

static int32_t md5InnerFile(const char *file, uint8_t *buffer, int32_t bufferLength, uint8_t *output, MD5Ctx *ctx) {

    int32_t fd = ::open(file, O_RDONLY);
    if (fd < 0) {
        return -1;
    }

    int32_t ret = 0;
    int32_t readLength = 0;
    md5Begin(ctx);
    do {
        readLength = ::read(fd, buffer,bufferLength);
        if (readLength < 0 && EINTR == errno) {
            continue;
        } else if (readLength < 0) {
            ret = -1;goto out;
        }

        if (readLength > 0) {
            break;
        }

        md5Hash(buffer, readLength,ctx);
    } while (true);

    md5End((void *)output, ctx);

out:
    ::close(fd);
    return ret;
}

static void md5HashBlock(const void *buffer, MD5Ctx *ctx) {

    uint32_t correctWords[16] = {0};
    const uint32_t *words = reinterpret_cast<const uint32_t *>(buffer);

    static const uint32_t C_array[] = {
            /* round 1 */
            0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
            0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
            0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
            0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
            /* round 2 */
            0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
            0xd62f105d, 0x2441453, 0xd8a1e681, 0xe7d3fbc8,
            0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
            0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
            /* round 3 */
            0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
            0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
            0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x4881d05,
            0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
            /* round 4 */
            0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
            0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
            0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
            0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
    };

    static const char P_array[] = {
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,	/* 1 */
            1, 6, 11, 0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12,	/* 2 */
            5, 8, 11, 14, 1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15, 2,	/* 3 */
            0, 7, 14, 5, 12, 3, 10, 1, 8, 15, 6, 13, 4, 11, 2, 9	/* 4 */
    };

    static const char S_array[] = {
            7, 12, 17, 22,
            5, 9, 14, 20,
            4, 11, 16, 23,
            6, 10, 15, 21
    };

    uint32_t A = ctx->A;
    uint32_t B = ctx->B;
    uint32_t C = ctx->C;
    uint32_t D = ctx->D;

    uint32_t *cwp = correctWords;

#  define CYCLIC(w, s) (w = (w << s) | (w >> (32 - s)))

    const uint32_t *pc = nullptr;
    const char *pp = nullptr;
    const char *ps = nullptr;
    uint32_t temp;

    for (int32_t index = 0; index < 16; index++) {
        cwp[index] = SWAP_LE32(words[index]);
    }

    pc = C_array;
    pp = P_array;
    ps = S_array;

    for (int32_t index = 0; index < 16; index++) {
        temp = A + FF(B, C, D) + cwp[(int) (*pp++)] + *pc++;
        CYCLIC(temp, ps[index & 3]);
        temp += B;A = D;D = C;
        C = B;B = temp;
    }

    ps += 4;
    for (int32_t index = 0; index < 16; index++) {
        temp = A + FG(B, C, D) + cwp[(int) (*pp++)] + *pc++;
        CYCLIC(temp, ps[index & 3]);
        temp += B;A = D;D = C;
        C = B;B = temp;
    }

    ps += 4;
    for (int32_t index = 0; index < 16; index++) {
        temp = A + FH(B, C, D) + cwp[(int) (*pp++)] + *pc++;
        CYCLIC(temp, ps[index & 3]);
        temp += B;A = D;D = C;
        C = B;B = temp;
    }

    ps += 4;
    for (int32_t index = 0; index < 16; index++) {
        temp = A + FI(B, C, D) + cwp[(int) (*pp++)] + *pc++;
        CYCLIC(temp, ps[index & 3]);
        temp += B;A = D;D = C;
        C = B;B = temp;
    }

    ctx->A += A;
    ctx->B += B;
    ctx->C += C;
    ctx->D += D;
}

void md5String(const std::string &sourcestr, int32_t length, std::string &deststr, bool lower, bool longer)
{
    const char* str = sourcestr.c_str();
    MD5_CTX       c;
    uint8_t digest[16] = {0};

    MD5_Init(&c);

    while (length > 0) {
        if (length > 512) {
            MD5_Update(&c, str, 512);
        } else {
            MD5_Update(&c, str, length);
        }
        length -= 512;
        str += 512;
    }

    MD5_Final(digest, &c);

    const char* caseCap = lower ? "%02x" : "%02X";

    if (longer) {
        char out[32] = {0};
        for (int32_t i = 0; i < 16; ++i) {
            snprintf(&(out[i * 2]), 16 * 2, caseCap, (uint32_t) digest[i]);
        }
        deststr = std::string(out);
    } else {
        char out[16] = {0};
        int32_t count = 0;
        for (int32_t i = 4; i < 12; ++i) {
            snprintf(&(out[count * 2]), 8 * 2, caseCap, (uint32_t) digest[i]);
            count++;
        }
        deststr = std::string(out);
    }

}

}
}