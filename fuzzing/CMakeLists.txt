add_executable(fuzz_target fuzz_target.cc)
target_link_libraries(fuzz_target PUBLIC libaec::aec)

# Actually link libFuzzer
target_link_options(fuzz_target PRIVATE -fsanitize=fuzzer)
