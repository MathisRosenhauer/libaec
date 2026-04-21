/**
 * @file fuzz_target_streaming.cc
 *
 * Exercises the streaming encode/decode APIs, RSI offset capture,
 * aec_buffer_seek, and aec_decode_range — the public API functions not
 * covered by fuzz_target.cc.
 *
 * Four phases per input:
 *
 *   1. Streaming decode of raw fuzz bytes on to a small output buffer.
 *      Forces the resumable decoder states (m_split_output, m_zero_output,
 *      m_se_decode, m_uncomp_copy, …).  Captured RSI offsets are then used
 *      to exercise aec_buffer_seek.
 *
 *   2. Streaming encode of the fuzz payload.  Captures RSI offsets via the
 *      encode offset API.
 *
 *   3. Streaming decode of the encoded output.  Validates the round-trip and
 *      collects decode-side RSI offsets needed for phase 4.
 *
 *   4. aec_decode_range called with two different (pos, size) pairs using the
 *      offsets captured in phase 3.
 *
 * APIs exercised
 *   aec_encode_init, aec_encode_enable_offsets, aec_encode,
 *   aec_encode_count_offsets, aec_encode_get_offsets, aec_encode_end,
 *   aec_decode_init, aec_decode_enable_offsets, aec_decode,
 *   aec_decode_count_offsets, aec_decode_get_offsets, aec_decode_end,
 *   aec_decode_range, aec_buffer_seek
 */

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>
#include "libaec.h"
#include <fuzzer/FuzzedDataProvider.h>

static void fill_stream(struct aec_stream *strm,
                        unsigned int bits_per_sample,
                        unsigned int block_size,
                        unsigned int rsi,
                        unsigned int flags)
{
    memset(strm, 0, sizeof(*strm));
    strm->bits_per_sample = bits_per_sample;
    strm->block_size      = block_size;
    strm->rsi             = rsi;
    strm->flags           = flags;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
{
    FuzzedDataProvider fdp(Data, Size);

    const unsigned int bits_per_sample =
        fdp.ConsumeIntegralInRange<unsigned int>(1, 32);
    const unsigned int block_size =
        fdp.PickValueInArray<unsigned int>({8, 16, 32, 64});
    const unsigned int rsi =
        fdp.ConsumeIntegralInRange<unsigned int>(1, 4096);
    /* Only the flags that aec_decode_init/aec_encode_init actually honour. */
    const unsigned int flags =
        fdp.ConsumeIntegral<uint8_t>() &
        (AEC_DATA_SIGNED | AEC_DATA_MSB | AEC_DATA_PREPROCESS | AEC_PAD_RSI);

    /* Output chunk size used in phase 1 to exercise resumable states. */
    const size_t chunk_out =
        fdp.ConsumeIntegralInRange<size_t>(1, 128);

    std::vector<uint8_t> payload = fdp.ConsumeRemainingBytes<uint8_t>();
    if (payload.empty())
        return 0;

    /* ── Phase 1: streaming decode of raw fuzz bytes ────────────────────────
     *
     * Intent: exercise the decoder on arbitrary, likely-invalid compressed
     * data and verify it handles errors cleanly.  The small per-call avail_out
     * forces the resumable partial-output states.
     * After decoding, use any captured RSI offsets with aec_buffer_seek.       */
    {
        struct aec_stream dec;
        fill_stream(&dec, bits_per_sample, block_size, rsi, flags);
        dec.next_in  = payload.data();
        dec.avail_in = payload.size();

        /* Generous output buffer; we expose it in small slices below. */
        std::vector<uint8_t> out(payload.size() * 4 + 1024);
        size_t out_written = 0;

        if (aec_decode_init(&dec) == AEC_OK) {
            aec_decode_enable_offsets(&dec);

            /* Feed output in chunk_out-sized pieces to hit resumable paths. */
            bool decode_error = false;
            while (out_written + chunk_out <= out.size() && dec.avail_in > 0) {
                dec.next_out  = out.data() + out_written;
                dec.avail_out = chunk_out;
                int st = aec_decode(&dec, AEC_NO_FLUSH);
                out_written += chunk_out - dec.avail_out;
                if (st == AEC_DATA_ERROR) { decode_error = true; break; }
                if (st != AEC_OK) break;
            }
            /* Drain whatever remains (only if no hard error above). */
            if (!decode_error && out_written < out.size()) {
                dec.next_out  = out.data() + out_written;
                dec.avail_out = out.size() - out_written;
                aec_decode(&dec, AEC_FLUSH);
            }

            size_t count = 0;
            if (aec_decode_count_offsets(&dec, &count) == AEC_OK && count > 0) {
                std::vector<size_t> offsets(count);
                if (aec_decode_get_offsets(&dec, offsets.data(), count) == AEC_OK) {
                    /* Exercise aec_buffer_seek at up to 4 captured offsets. */
                    for (size_t i = 0; i < count && i < 4; i++) {
                        struct aec_stream sk;
                        fill_stream(&sk, bits_per_sample, block_size, rsi, flags);
                        sk.next_in  = payload.data();
                        sk.avail_in = payload.size();
                        if (aec_decode_init(&sk) == AEC_OK) {
                            std::vector<uint8_t> sk_out(256);
                            sk.next_out  = sk_out.data();
                            sk.avail_out = sk_out.size();
                            if (aec_buffer_seek(&sk, offsets[i]) == AEC_OK)
                                aec_decode(&sk, AEC_FLUSH);
                            aec_decode_end(&sk);
                        }
                    }
                }
            }
            aec_decode_end(&dec);
        }
    }

    /* ── Phase 2: streaming encode of fuzz payload ──────────────────────────
     *
     * Intent: exercise the encoder streaming API and RSI offset capture.      */
    std::vector<uint8_t> encoded(payload.size() * 2 + 1024);
    size_t encoded_bytes = 0;

    {
        struct aec_stream enc;
        fill_stream(&enc, bits_per_sample, block_size, rsi, flags);
        enc.next_in   = payload.data();
        enc.avail_in  = payload.size();
        enc.next_out  = encoded.data();
        enc.avail_out = encoded.size();

        if (aec_encode_init(&enc) != AEC_OK)
            return 0;
        aec_encode_enable_offsets(&enc);
        aec_encode(&enc, AEC_FLUSH);
        encoded_bytes = enc.total_out;

        size_t count = 0;
        if (aec_encode_count_offsets(&enc, &count) == AEC_OK && count > 0) {
            std::vector<size_t> enc_offsets(count);
            aec_encode_get_offsets(&enc, enc_offsets.data(), count);
        }
        aec_encode_end(&enc);
    }

    if (encoded_bytes == 0)
        return 0;

    /* ── Phase 3: streaming decode of encoded output ────────────────────────
     *
     * Intent: validate the encode→decode round-trip and collect RSI offsets
     * in the decoded domain for phase 4.                                       */
    std::vector<uint8_t> decoded(payload.size() + 1024);
    size_t decoded_bytes = 0;
    std::vector<size_t> dec_offsets;

    {
        struct aec_stream dec;
        fill_stream(&dec, bits_per_sample, block_size, rsi, flags);
        dec.next_in   = encoded.data();
        dec.avail_in  = encoded_bytes;
        dec.next_out  = decoded.data();
        dec.avail_out = decoded.size();

        if (aec_decode_init(&dec) != AEC_OK)
            return 0;
        aec_decode_enable_offsets(&dec);
        aec_decode(&dec, AEC_FLUSH);
        decoded_bytes = dec.total_out;

        size_t count = 0;
        if (aec_decode_count_offsets(&dec, &count) == AEC_OK && count > 0) {
            dec_offsets.resize(count);
            aec_decode_get_offsets(&dec, dec_offsets.data(), count);
        }
        aec_decode_end(&dec);
    }

    /* ── Phase 4: aec_decode_range ──────────────────────────────────────────
     *
     * Intent: exercise random-access decode with valid compressed data and
     * captured RSI offsets.  Calls the function twice:
     *   (a) full range from position 0
     *   (b) from the start of the second RSI (if one exists)               */
    if (dec_offsets.empty() || decoded_bytes == 0)
        return 0;

    struct aec_stream rng;
    fill_stream(&rng, bits_per_sample, block_size, rsi, flags);
    rng.next_in  = encoded.data();
    rng.avail_in = encoded_bytes;

    if (aec_decode_init(&rng) != AEC_OK)
        return 0;

    std::vector<uint8_t> rng_out(decoded_bytes);

    /* (a) Full range. */
    rng.next_out  = rng_out.data();
    rng.avail_out = decoded_bytes;
    aec_decode_range(&rng, dec_offsets.data(), dec_offsets.size(),
                     0, decoded_bytes);

    /* (b) Second RSI boundary onwards. */
    if (dec_offsets.size() >= 2) {
        /* Mirror the bytes_per_sample calculation in aec_decode_init.
         * Note: AEC_DATA_3BYTE is excluded from the flags mask above so
         * bits_per_sample 17-24 always maps to 4 bytes here. */
        size_t bytes_per_sample =
            (bits_per_sample > 16) ? 4 : (bits_per_sample > 8) ? 2 : 1;
        size_t rsi_bytes = (size_t)rsi * block_size * bytes_per_sample;
        if (rsi_bytes > 0 && rsi_bytes < decoded_bytes) {
            size_t sz = decoded_bytes - rsi_bytes;
            rng.next_out  = rng_out.data();
            rng.avail_out = sz;
            aec_decode_range(&rng, dec_offsets.data(), dec_offsets.size(),
                             rsi_bytes, sz);
        }
    }

    aec_decode_end(&rng);
    return 0;
}
