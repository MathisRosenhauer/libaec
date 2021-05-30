# libaec - Adaptive Entropy Coding library

Libaec provides fast lossless compression of 1 up to 32 bit wide
signed or unsigned integers (samples). The library achieves best
results for low entropy data as often encountered in space imaging
instrument data or numerical model output from weather or climate
simulations. While floating point representations are not directly
supported, they can also be efficiently coded by grouping exponents
and mantissa.

## Scope

Libaec implements extended
[Golomb-Rice](http://en.wikipedia.org/wiki/Golomb_coding) coding as
defined in the CCSDS recommended standard [121.0-B-3][1]. The library
covers the adaptive entropy coder and the preprocessor discussed in
sections 1 to 5.2.6 of the [standard][1].

## Downloads

Source code and binary installer can be [downloaded here](https://gitlab.dkrz.de/k202009/libaec/tags) [or here](https://github.com/MathisRosenhauer/libaec).

## Patent considerations

As stated in section A3 of the current [standard][1]

> At time of publication, the specifications of this Recommended
> Standard are not known to be the subject of patent rights.

## Installation

See [INSTALL.md](INSTALL.md) for details.

## SZIP Compatibility

[Libaec can replace SZIP](README.SZIP).

## Encoding

In this context efficiency refers to the size of the encoded
data. Performance refers to the time it takes to encode data.

Suppose you have an array of 32 bit signed integers you want to
compress. The pointer pointing to the data shall be called `source`,
output goes into `dest`.

```c
#include <libaec.h>

...
    struct aec_stream strm;
    int32_t *source;
    unsigned char *dest;

    /* input data is 32 bits wide */
    strm.bits_per_sample = 32;

    /* define a block size of 16 */
    strm.block_size = 16;

    /* the reference sample interval is set to 128 blocks */
    strm.rsi = 128;

    /* input data is signed and needs to be preprocessed */
    strm.flags = AEC_DATA_SIGNED | AEC_DATA_PREPROCESS;

    /* pointer to input */
    strm.next_in = (unsigned char *)source;

    /* length of input in bytes */
    strm.avail_in = source_length * sizeof(int32_t);

    /* pointer to output buffer */
    strm.next_out = dest;

    /* length of output buffer in bytes */
    strm.avail_out = dest_length;

    /* initialize encoding */
    if (aec_encode_init(&strm) != AEC_OK)
        return 1;

    /* Perform encoding in one call and flush output. */
    /* In this example you must be sure that the output */
    /* buffer is large enough for all compressed output */
    if (aec_encode(&strm, AEC_FLUSH) != AEC_OK)
        return 1;

    /* free all resources used by encoder */
    aec_encode_end(&strm);
...
```

`block_size` can vary from 8 to 64 samples. Smaller blocks allow the
compression to adapt more rapidly to changing source
statistics. Larger blocks create less overhead but can be less
efficient if source statistics change across the block.

`rsi` sets the reference sample interval in blocks. A large RSI will
improve performance and efficiency. It will also increase memory
requirements since internal buffering is based on RSI size. A smaller
RSI may be desirable in situations where errors could occur in the
transmission of encoded data and the resulting propagation of errors
in decoded data has to be minimized.

### Flags:

* `AEC_DATA_SIGNED`: input data are signed integers. Specifying this
  correctly increases compression efficiency. Default is unsigned.

* `AEC_DATA_PREPROCESS`: preprocessing input will improve compression
  efficiency if data samples are correlated. It will only cost
  performance for no gain in efficiency if the data is already
  uncorrelated.

* `AEC_DATA_MSB`: input data is stored most significant byte first
  i.e. big endian. Default is little endian on all architectures.

* `AEC_DATA_3BYTE`: the 17 to 24 bit input data is stored in three
  bytes. This flag has no effect for other sample sizes.

* `AEC_RESTRICTED`: use a restricted set of code options. This option is
  only valid for `bits_per_sample` <= 4.

### Data size:

The following rules apply for deducing storage size from sample size
(`bits_per_sample`):

 **sample size**  | **storage size**
--- | ---
 1 -  8 bits  | 1 byte
 9 - 16 bits  | 2 bytes
17 - 24 bits  | 3 bytes (only if `AEC_DATA_3BYTE` is set)
25 - 32 bits  | 4 bytes (if `AEC_DATA_3BYTE` is set)
17 - 32 bits  | 4 bytes (if `AEC_DATA_3BYTE` is not set)

If a sample requires less bits than the storage size provides, then
you have to make sure that unused bits are not set. Libaec does not
enforce this for performance reasons and will produce undefined output
if unused bits are set. All input data must be a multiple of the
storage size in bytes. Remaining bytes which do not form a complete
sample will be ignored.

Libaec accesses `next_in` and `next_out` buffers only bytewise. There
are no alignment requirements for these buffers.

### Flushing:

`aec_encode` can be used in a streaming fashion by chunking input and
output and specifying `AEC_NO_FLUSH`. The function will return if either
the input runs empty or the output buffer is full. The calling
function can check `avail_in` and `avail_out` to see what occurred. The
last call to `aec_encode()` must set `AEC_FLUSH` to drain all
output. [aec.c](src/aec.c) is an example of streaming usage of encoding and
decoding.

### Output:

Encoded data will be written to the buffer submitted with
`next_out`. The length of the compressed data is `total_out`.

See libaec.h for a detailed description of all relevant structure
members and constants.


## Decoding

Using decoding is very similar to encoding, only the meaning of input
and output is reversed.

```c
#include <libaec.h>

...
    struct aec_stream strm;
    /* this is now the compressed data */
    unsigned char *source;
    /* here goes the uncompressed result */
    int32_t *dest;

    strm.bits_per_sample = 32;
    strm.block_size = 16;
    strm.rsi = 128;
    strm.flags = AEC_DATA_SIGNED | AEC_DATA_PREPROCESS;
    strm.next_in = source;
    strm.avail_in = source_length;
    strm.next_out = (unsigned char *)dest;
    strm.avail_out = dest_lenth * sizeof(int32_t);
    if (aec_decode_init(&strm) != AEC_OK)
        return 1;
    if (aec_decode(&strm, AEC_FLUSH) != AEC_OK)
        return 1;
    aec_decode_end(&strm);
...
```

It is strongly recommended that the size of the output buffer
(`next_out`) is a multiple of the storage size in bytes. If the buffer
is not a multiple of the storage size and the buffer gets filled to
the last sample, the error code `AEC_MEM_ERROR` is returned.

It is essential for decoding that parameters like `bits_per_sample`,
`block_size`, `rsi`, and `flags` are exactly the same as they were for
encoding. Libaec does not store these parameters in the coded stream
so it is up to the calling program to keep the correct parameters
between encoding and decoding.

The actual values of coding parameters are in fact only relevant for
efficiency and performance. Data integrity only depends on consistency
of the parameters.

The exact length of the original data is not preserved and must also be
transmitted out of band. The decoder can produce additional output
depending on whether the original data ended on a block boundary or on
zero blocks. The output data must therefore be truncated to the
correct length. This can also be achieved by providing an output
buffer of just the correct length.

## References

[Lossless Data Compression. Recommendation for Space Data System
Standards, CCSDS 121.0-B-3. Blue Book. Issue 3. Washington, D.C.:
CCSDS, August 2020.][1]

[1]: https://public.ccsds.org/Pubs/121x0b3.pdf
