dnl Compiler and linker flag probes.
dnl
dnl GCC (and clang) accept any unknown -Wno-<warning> silently, and only
dnl complain about it if some other diagnostic is emitted.  Probing the
dnl -Wno- form therefore always succeeds; the probes below test the
dnl positive -W<warning> form instead and then add the flag as given.

dnl CDE_FLAG_PROBE_FORM(FLAG)
dnl Expands to shell code that sets cde_flag_probe to the form of FLAG
dnl that is actually tested.
AC_DEFUN([CDE_FLAG_PROBE_FORM],
[dnl
  cde_flag_probe=`echo "$1" | sed 's/^-Wno-/-W/'`
])

dnl CDE_CHECK_CFLAG(FLAG, [VARIABLE = C_COMPILER_FLAGS])
dnl Append FLAG to VARIABLE if the C compiler accepts it.
AC_DEFUN([CDE_CHECK_CFLAG],
[dnl
  CDE_FLAG_PROBE_FORM([$1])
  AC_MSG_CHECKING([if $CC supports $1])
  AC_LANG_PUSH([C])
  ac_saved_ccflags="$CFLAGS"
  CFLAGS="-Werror $cde_flag_probe"
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
    [cc_flag_check=yes],
    [cc_flag_check=no]
  )
  AC_MSG_RESULT([$cc_flag_check])
  CFLAGS="$ac_saved_ccflags"
  if test "$cc_flag_check" = yes
  then
    m4_default([$2], [C_COMPILER_FLAGS])="${m4_default([$2], [C_COMPILER_FLAGS])} $1"
  fi
  AC_LANG_POP([C])
])

dnl CDE_CHECK_CXXFLAG(FLAG, [VARIABLE = CXX_COMPILER_FLAGS])
dnl Append FLAG to VARIABLE if the C++ compiler accepts it.
AC_DEFUN([CDE_CHECK_CXXFLAG],
[dnl
  CDE_FLAG_PROBE_FORM([$1])
  AC_MSG_CHECKING([if $CXX supports $1])
  AC_LANG_PUSH([C++])
  ac_saved_cxxflags="$CXXFLAGS"
  CXXFLAGS="-Werror $cde_flag_probe"
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
    [cxx_flag_check=yes],
    [cxx_flag_check=no]
  )
  AC_MSG_RESULT([$cxx_flag_check])
  CXXFLAGS="$ac_saved_cxxflags"
  if test "$cxx_flag_check" = yes
  then
    m4_default([$2], [CXX_COMPILER_FLAGS])="${m4_default([$2], [CXX_COMPILER_FLAGS])} $1"
  fi
  AC_LANG_POP([C++])
])

dnl CDE_CHECK_LDFLAG(FLAG, VARIABLE)
dnl Append FLAG to VARIABLE if a C program links with it.
AC_DEFUN([CDE_CHECK_LDFLAG],
[dnl
  AC_MSG_CHECKING([if the linker supports $1])
  AC_LANG_PUSH([C])
  ac_saved_ldflags="$LDFLAGS"
  LDFLAGS="$LDFLAGS -Werror $1"
  AC_LINK_IFELSE([AC_LANG_PROGRAM([])],
    [ld_flag_check=yes],
    [ld_flag_check=no]
  )
  AC_MSG_RESULT([$ld_flag_check])
  LDFLAGS="$ac_saved_ldflags"
  if test "$ld_flag_check" = yes
  then
    $2="${$2} $1"
  fi
  AC_LANG_POP([C])
])

dnl Old names, kept for compatibility.
AC_DEFUN([C_FLAG_CHECK], [CDE_CHECK_CFLAG([$1])])
AC_DEFUN([CXX_FLAG_CHECK], [CDE_CHECK_CXXFLAG([$1])])
