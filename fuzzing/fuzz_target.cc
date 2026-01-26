#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include "libaec.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    if (Size < 4)
        return 0;

    std::vector<unsigned char> dest(Size);
    aec_stream strm;
    strm.bits_per_sample = Data[0];
    strm.block_size = Data[1];
    strm.flags = Data[2];
    strm.rsi = Data[3];

    // Decode data
    strm.next_in = reinterpret_cast<const unsigned char *>(Data) + 4;
    strm.avail_in = Size - 4;
    strm.next_out = dest.data();
    strm.avail_out = dest.size();
    aec_buffer_decode(&strm);

    // Encode data
    strm.next_in = reinterpret_cast<const unsigned char *>(Data) + 4;
    strm.avail_in = Size - 4;
    strm.next_out = dest.data();
    strm.avail_out = dest.size();
    aec_buffer_encode(&strm);

    return 0;
}
