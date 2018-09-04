if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  message(FATAL_ERROR "You need to build with Clang for fuzzing to work")
endif()

add_library(aec_fuzz STATIC ${libaec_SRCS})
add_executable(fuzz_target fuzz_target.cc)

set(FUZZ_TARGET_LINK_FLAGS "-fsanitize=address,undefined")
set(FUZZ_TARGET_COMPILE_FLAGS
  "-g"
  "-fsanitize=address,undefined"
  "-fno-sanitize-recover=undefined"
  "-fsanitize-coverage=trace-pc-guard,indirect-calls,edge")
set_target_properties(fuzz_target aec_fuzz PROPERTIES
  LINK_FLAGS ${FUZZ_TARGET_LINK_FLAGS}
  COMPILE_OPTIONS "${FUZZ_TARGET_COMPILE_FLAGS}")

target_link_libraries(fuzz_target aec_fuzz Fuzzer)
