#include <stdlib.h>
#include <stdint.h>
#include "libaec.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    struct aec_stream strm;
    int32_t *source;
    unsigned char *dest;

    if (Size < 2)
        return 0;

    dest = (unsigned char *)malloc(Size * 4);
    strm.bits_per_sample = (Data[0] & 0x1f) | 1;
    strm.block_size = 8 << (Data[1] & 3);
    strm.rsi = 2;
    strm.flags = AEC_DATA_PREPROCESS;
    if (Data[1] & 0x80)
        strm.flags |= AEC_DATA_MSB;
    if (Data[1] & 0x40)
        strm.flags |= AEC_DATA_SIGNED;
    if (strm.bits_per_sample <= 24 && strm.bits_per_sample > 16)
        strm.flags |= AEC_DATA_3BYTE;
    strm.next_in = (unsigned char *)(Data + 2);
    strm.avail_in = Size - 2;
    strm.next_out = dest;
    strm.avail_out = (Size - 2) * 4;
    if (Data[1] & 0x20)
        aec_buffer_encode(&strm);
    else
        aec_buffer_decode(&strm);
    free(dest);
    return 0;
}
