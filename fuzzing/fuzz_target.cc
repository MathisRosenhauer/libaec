#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include "libaec.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    if (Size < 2)
        return 0;

    std::vector<unsigned char> dest(Size * 4);
    aec_stream strm;
    strm.bits_per_sample = (Data[0] & 0x1f) | 1;
    strm.block_size = 8 << (Data[1] & 3);
    strm.rsi = 2;

    strm.flags = AEC_DATA_PREPROCESS;
    if (Data[1] & 0x80)
        strm.flags |= AEC_DATA_MSB;
    if (Data[1] & 0x40)
        strm.flags |= AEC_DATA_SIGNED;
    if (strm.bits_per_sample <= 24 &&
        strm.bits_per_sample > 16 &&
        Data[1] & 0x10)
        strm.flags |= AEC_DATA_3BYTE;

    // Decode data
    strm.next_in = reinterpret_cast<const unsigned char *>(Data) + 2;
    strm.avail_in = Size - 2;
    strm.next_out = dest.data();
    strm.avail_out = dest.size();
    aec_buffer_decode(&strm);

    // Encode data
    strm.next_in = reinterpret_cast<const unsigned char *>(Data) + 2;
    strm.avail_in = Size - 2;
    strm.next_out = dest.data();
    strm.avail_out = dest.size();
    aec_buffer_encode(&strm);

    return 0;
}
