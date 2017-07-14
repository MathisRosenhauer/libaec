AC_PREREQ([2.64])
AC_INIT([libaec], [1.0.1], [rosenhauer@dkrz.de])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_SRCDIR([src/libaec.h])
AC_CONFIG_HEADERS(config/config.h)

LT_INIT
gl_VISIBILITY
AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE([enable])

AC_PROG_CC
AC_PROG_CXX

AC_HEADER_STDC

AC_C_BIGENDIAN
AC_C_INLINE
AC_TYPE_INT64_T
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_C_INLINE
AC_C_RESTRICT

AC_CHECK_FUNCS([memset strstr])
AC_CHECK_DECLS(__builtin_clzll)

AM_EXTRA_RECURSIVE_TARGETS([bench benc bdec])

AC_ARG_ENABLE([fuzzing],
  AS_HELP_STRING([--enable-fuzzing], [Enable fuzzing of libaec]))

AS_IF([test "x$enable_fuzzing" = "xyes"], [
  AX_COMPILER_VENDOR
  AS_IF([test "x$ax_cv_c_compiler_vendor" = "xclang"], [
    AC_LANG_PUSH([C++])
    AX_COMPILER_VENDOR
    AC_LANG_POP([C++])
    AS_IF([test "x$ax_cv_cxx_compiler_vendor" = "xclang"], [], [
      AC_MSG_ERROR([You need to build with Clang as CXX for fuzzing to work])
    ])
  ], [
    AC_MSG_ERROR([You need to build with Clang as CC for fuzzing to work])
  ])
])
AM_CONDITIONAL([AEC_FUZZING], [test x$enable_fuzzing = xyes])

AC_CONFIG_FILES([Makefile src/Makefile tests/Makefile fuzzing/Makefile])
AC_OUTPUT
