#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include "szlib.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    if (Size < 4)
        return 0;

    std::vector<unsigned char> dest(Size);
    size_t dest_len;
    SZ_com_t sz_param;
    sz_param.bits_per_pixel = Data[0];
    sz_param.pixels_per_block = Data[1];
    sz_param.options_mask = Data[2];
    sz_param.pixels_per_scanline = Data[3];

    // Decode data
    SZ_BufftoBuffDecompress(dest.data(), &dest_len,
                            reinterpret_cast<const unsigned char *>(Data) + 4,
                            Size - 4, &sz_param);

    // Encode data
    SZ_BufftoBuffCompress(dest.data(), &dest_len,
                          reinterpret_cast<const unsigned char *>(Data) + 4,
                          Size - 4, &sz_param);
    return 0;
}
