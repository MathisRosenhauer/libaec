add_executable(fuzz_target fuzz_target.cc)
target_link_libraries(fuzz_target PUBLIC libaec::aec)
target_link_options(fuzz_target PRIVATE -fsanitize=fuzzer)

add_executable(fuzz_target_sz fuzz_target_sz.cc)
target_link_libraries(fuzz_target_sz PUBLIC libaec::sz)
target_link_options(fuzz_target_sz PRIVATE -fsanitize=fuzzer)

# Covers the streaming encode/decode APIs, RSI offset capture,
# aec_buffer_seek, and aec_decode_range.
add_executable(fuzz_target_streaming fuzz_target_streaming.cc)
target_link_libraries(fuzz_target_streaming PUBLIC libaec::aec)
target_link_options(fuzz_target_streaming PRIVATE -fsanitize=fuzzer)
