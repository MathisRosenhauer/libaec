add_executable(fuzz_target fuzz_target.cc)
target_link_libraries(fuzz_target PUBLIC libaec::aec)

# Actually link libFuzzer
target_link_options(fuzz_target PRIVATE -fsanitize=fuzzer)

add_executable(fuzz_target_sz fuzz_target_sz.cc)
target_link_libraries(fuzz_target_sz PUBLIC libaec::sz)

# Actually link libFuzzer
target_link_options(fuzz_target_sz PRIVATE -fsanitize=fuzzer)
