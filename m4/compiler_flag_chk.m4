dnl From stackoverflow, with some changes
dnl @synopsis CXX_FLAGS_CHECK [compiler flags]
dnl @summary check whether compiler supports given C++ flags or not
AC_DEFUN([CXX_FLAG_CHECK],
[dnl
  AC_MSG_CHECKING([if $CXX supports $1])
  AC_LANG_PUSH([C++])
  ac_saved_cxxflags="$CXXFLAGS"
  CXXFLAGS="-Werror $1"
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
    [cxx_flag_check=yes],
    [cxx_flag_check=no]
  )
  AC_MSG_RESULT([$cxx_flag_check])
  CXXFLAGS="$ac_saved_cxxflags"
  if test "$cxx_flag_check" = yes
  then
    CXX_COMPILER_FLAGS="$1 $CXX_COMPILER_FLAGS "
  fi
  AC_LANG_POP([C++])
])

AC_DEFUN([C_FLAG_CHECK],
[dnl
  AC_MSG_CHECKING([if $CC supports $1])
  AC_LANG_PUSH([C])
  ac_saved_ccflags="$CFLAGS"
  CFLAGS="-Werror $1"
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
    [cc_flag_check=yes],
    [cc_flag_check=no]
  )
  AC_MSG_RESULT([$cc_flag_check])
  CFLAGS="$ac_saved_ccflags"
  if test "$cc_flag_check" = yes
  then
    C_COMPILER_FLAGS="$1 $C_COMPILER_FLAGS "
  fi
  AC_LANG_POP([C])
])
