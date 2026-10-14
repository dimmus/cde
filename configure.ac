AC_INIT([Common Desktop Environment],
        [2.5.2],
        [https://sourceforge.net/projects/cdesktopenv],
        [cde],
        [https://sourceforge.net/projects/cdesktopenv])
AC_CONFIG_HEADERS([include/cde_config.h])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIRS([m4])
AM_INIT_AUTOMAKE([foreign subdir-objects no-define])

CDE_VERSION_MAJOR=2
CDE_VERSION_MINOR=5
CDE_VERSION_MICRO=2
CDE_VERSION_DEV=99

AC_SUBST(CDE_VERSION_MAJOR)
AC_SUBST(CDE_VERSION_MINOR)
AC_SUBST(CDE_VERSION_MICRO)
AC_SUBST(CDE_VERSION_DEV)

AC_PREREQ([2.69])

AC_CANONICAL_HOST
AC_CANONICAL_BUILD

dnl Language standards.  C is GNU C11 (C99 plus the C11 additions and
dnl the GNU extensions the code base already relies on); C++ is pinned to
dnl GNU C++17 so that it does not silently follow the compiler default.
C_STD="gnu11"
CXX_STD="gnu++17"
AC_SUBST(C_STD)
AC_SUBST(CXX_STD)

dnl These must be up here for the compiler search list to actually work
AC_PROG_CC([cc gcc clang])
AC_PROG_CXX([c++ g++ clang++])

dnl Link-time optimisation (--enable-lto).  Archives of LTO objects must
dnl be made by the gcc-ar/gcc-ranlib/gcc-nm wrappers, which load the LTO
dnl plugin, so this comes before LT_INIT, which picks AR, RANLIB and NM.
AC_ARG_ENABLE([lto],
        AS_HELP_STRING([--enable-lto], [Build with link-time optimisation, -flto=auto -fno-semantic-interposition (default=no)]),
        [], [enable_lto=no]
)
LTO_CFLAGS=""
LTO_CXXFLAGS=""
LTO_LDFLAGS=""
if test "x$enable_lto" != xno; then
   CDE_CHECK_CFLAG([-flto=auto], [LTO_CFLAGS])
   CDE_CHECK_CXXFLAG([-flto=auto], [LTO_CXXFLAGS])
   CDE_CHECK_LDFLAG([-flto=auto], [LTO_LDFLAGS])
   if test -z "$LTO_CFLAGS" || test -z "$LTO_CXXFLAGS" || test -z "$LTO_LDFLAGS"; then
      AC_MSG_ERROR([--enable-lto: the compilers or the linker do not support -flto=auto])
   fi
   CDE_CHECK_CFLAG([-fno-semantic-interposition], [LTO_CFLAGS])
   CDE_CHECK_CXXFLAG([-fno-semantic-interposition], [LTO_CXXFLAGS])
   if test "x$GCC" = xyes; then
      AC_CHECK_TOOL([AR], [gcc-ar])
      AC_CHECK_TOOL([RANLIB], [gcc-ranlib])
      AC_CHECK_TOOL([NM], [gcc-nm])
   fi
fi

dnl Shared libraries only: nothing links the static archives, and
dnl building both compiles every libtool object twice.
LT_INIT([disable-static])
AC_PREFIX_DEFAULT(/usr/dt)

PKG_PROG_PKG_CONFIG

dnl systemd unit for dtlogin. By default it is installed only when systemd
dnl is installed (systemd.pc) and is the running init (/run/systemd/system,
dnl the sd_booted() test); pass a directory to install it regardless.
AC_ARG_WITH([systemdsystemunitdir],
	AS_HELP_STRING([--with-systemdsystemunitdir=DIR],
		[install dtlogin.service into DIR (default: systemd's unit dir if systemd is installed and running; "no" to skip)]),
	[], [with_systemdsystemunitdir=auto])
AC_MSG_CHECKING([where to install dtlogin.service])
if test "x$with_systemdsystemunitdir" = "xauto" || test "x$with_systemdsystemunitdir" = "xyes"; then
	def_systemdsystemunitdir=`$PKG_CONFIG --variable=systemdsystemunitdir systemd 2>/dev/null`
	if test -n "$def_systemdsystemunitdir" && test -d /run/systemd/system; then
		with_systemdsystemunitdir=$def_systemdsystemunitdir
	elif test "x$with_systemdsystemunitdir" = "xyes"; then
		AC_MSG_ERROR([systemd support requested but systemd is not installed or not running])
	else
		with_systemdsystemunitdir=no
	fi
fi
AC_MSG_RESULT([$with_systemdsystemunitdir])
if test "x$with_systemdsystemunitdir" != "xno"; then
	AC_SUBST([systemdsystemunitdir], [$with_systemdsystemunitdir])
fi
AM_CONDITIONAL([HAVE_SYSTEMD], [test "x$with_systemdsystemunitdir" != "xno"])

dnl SOURCE_DEFINES - start with CDE project default
SOURCE_CPP_DEFINES="-DANSICPP -DMULTIBYTE -DNLS16"

dnl CPP_COMPILER_FLAGS - CPP/C/C++ compiler flags
CPP_COMPILER_FLAGS=""

dnl CXX_COMPILER_FLAGS - C++ compiler flags
CXX_COMPILER_FLAGS=""

dnl C_COMPILER_FLAGS - C compiler flags
C_COMPILER_FLAGS=""


dnl These OS version checks are deprecated and should be replaced with
dnl feature checks where appropriate

build_linux=no
bsd=no
build_freebsd=no
build_openbsd=no
build_netbsd=no
build_solaris=no
build_aix=no

dnl locations of libs/includes if not in 'standard' places like on
dnl linux (/usr/...).  We build these up based on where X11 is, and
dnl other things as we go along.
EXTRA_LIBS=""
EXTRA_INCS=""

dnl Use Motif's advertised compiler interface when pkg-config metadata is
dnl available. Some Motif implementations expose dependencies such as Xft
dnl through their public headers. Keep the legacy library detection below
dnl for Motif implementations without pkg-config metadata and for CDE's
dnl existing linker ordering.
PKG_CHECK_MODULES([MOTIF], [motif],
    [CPP_COMPILER_FLAGS="${CPP_COMPILER_FLAGS} ${MOTIF_CFLAGS}"],
    [:])

dnl dtstyle's Font dialog lists scalable fonts with fontconfig and previews
dnl them with Xft.  Both are optional: without them the dialog offers only
dnl bitmap fonts.
PKG_CHECK_MODULES([XFT], [xft fontconfig], [have_xft=yes], [have_xft=no])
AM_CONDITIONAL([DTSTYLE_XFT], [test "x$have_xft" = xyes])
if test "x$have_xft" = xyes; then
   AC_MSG_NOTICE([Style Manager scalable (Xft) font support: yes])
else
   AC_MSG_NOTICE([Style Manager scalable (Xft) font support: no (xft and fontconfig not found)])
fi

# pam currently only works on netbsd (9.2 tested) and linux
supports_pam=no

case "${build_os}" in
	linux*)
		build_linux=yes
                dnl _DEFAULT_SOURCE replaces the deprecated _BSD_SOURCE and
                dnl _SVID_SOURCE (and implies _POSIX_SOURCE).  It is still
                dnl needed: many files define _XOPEN_SOURCE or _POSIX_SOURCE
                dnl themselves, which would otherwise hide the BSD/SVID API.
                SOURCE_CPP_DEFINES="${SOURCE_CPP_DEFINES} -D_DEFAULT_SOURCE"
                supports_pam=yes
                 ;;
	freebsd*)
		build_freebsd=yes
		bsd=yes
                # fbsd needs the iconv plug to avoid conflict with libiconv
                # and libc implementations.  We prefer the libc impl.
                SOURCE_CPP_DEFINES="${SOURCE_CPP_DEFINES} -DLIBICONV_PLUG"
		;;
	openbsd*)
		build_openbsd=yes
		bsd=yes
		;;
	netbsd*)
		build_netbsd=yes
		bsd=yes
                supports_pam=yes
		;;
	solaris*|sun*)
		build_solaris=yes
		;;
	aix*)
		build_aix=yes
		;;
esac

AM_CONDITIONAL([LINUX], [test "$build_linux" = "yes"])
AM_CONDITIONAL([BSD], [test "$bsd" = "yes"])
AM_CONDITIONAL([FREEBSD], [test "$build_freebsd" = "yes"])
AM_CONDITIONAL([OPENBSD], [test "$build_openbsd" = "yes"])
AM_CONDITIONAL([NETBSD], [test "$build_netbsd" = "yes"])
AM_CONDITIONAL([SOLARIS], [test "$build_solaris" = "yes"])
AM_CONDITIONAL([AIX], [test "$build_aix" = "yes"])

dnl set CSRG_BASED define for the BSD's.
if test "$bsd" = "yes"
then
        SOURCE_CPP_DEFINES="${SOURCE_CPP_DEFINES} -DCSRG_BASED"
fi

is_x86_64=no
is_i386=no
is_sparc=no
is_mips=no
is_arm=no
is_ppc=no
is_riscv=no
is_aarch64=no

case "${host_cpu}" in
	i[3456]86*)
		is_i386=yes
		;;
	x86_64* | amd64*)
		is_x86_64=yes
		;;
	arm*)
		is_arm=yes
		;;
	mips*)
		is_mips=yes
		;;
	sparc*)
		is_sparc=yes
		;;
	ppc* | powerpc*)
		is_ppc=yes
		;;
	riscv*)
		is_riscv=yes
		;;
	aarch64*)
		is_aarch64=yes
		;;
esac

AM_CONDITIONAL([I386], [test "$is_i386" = yes])
AM_CONDITIONAL([X86_64], [test "$is_x86_64" = yes])
AM_CONDITIONAL([ARM], [test "$is_arm" = yes])
AM_CONDITIONAL([SPARC], [test "$is_sparc" = yes])
AM_CONDITIONAL([MIPS], [test "$is_mips" = yes])
AM_CONDITIONAL([PPC], [test "$is_ppc" = yes])
AM_CONDITIONAL([RISCV], [test "$is_riscv" = yes])
AM_CONDITIONAL([AARCH64], [test "$is_aarch64" = yes])

dnl The in-tree libraries ($(LIBSVC), $(LIBTT), ...) are defined in
dnl build-aux/cde-libs.am, which the Makefile.am files that link them
dnl include, so that automake makes the programs depend on them.

dnl set up come convenience replacements for global include dirs
dnl include/ in the source directory holds the headers, the one in the
dnl build directory the generated ones (cde_config.h, Dt/Dt.h).  In-tree
dnl they are the same directory, and the compiler drops the second -I.
AC_SUBST(DT_INCDIR, '-I$(top_builddir)/include/Dt -I$(top_srcdir)/include/Dt')
AC_SUBST(DTI_INCDIR, '-I$(top_srcdir)/include/DtI')
AC_SUBST(TT_INCDIR, '-I$(top_srcdir)/include/Tt')
AC_SUBST(XM_INCDIR, '-I$(top_srcdir)/include/Xm')
AC_SUBST(CSA_INCDIR, '-I$(top_srcdir)/include/csa')
AC_SUBST(SPC_INCDIR, '-I$(top_srcdir)/include/SPC')

dnl language enables.  AC_ARG_ENABLE sets enable_<feature> to "yes" for
dnl --enable-<feature> and to "no" for --disable-<feature>, so the
dnl action-if-given is left empty and only the default is filled in.

AC_ARG_ENABLE([german],
        AS_HELP_STRING([--enable-german], [Build German locale (default=no)]),
        [], [enable_german=no]
)
AM_CONDITIONAL([GERMAN], [test "x$enable_german" != xno])

AC_ARG_ENABLE([italian],
        AS_HELP_STRING([--enable-italian], [Build Italian locale (default=no)]),
        [], [enable_italian=no]
)
AM_CONDITIONAL([ITALIAN], [test "x$enable_italian" != xno])

AC_ARG_ENABLE([french],
        AS_HELP_STRING([--enable-french], [Build French locale (default=no)]),
        [], [enable_french=no]
)
AM_CONDITIONAL([FRENCH], [test "x$enable_french" != xno])

AC_ARG_ENABLE([spanish],
        AS_HELP_STRING([--enable-spanish], [Build Spanish locale (default=no)]),
        [], [enable_spanish=no]
)
AM_CONDITIONAL([SPANISH], [test "x$enable_spanish" != xno])

AC_ARG_ENABLE([japanese],
        AS_HELP_STRING([--enable-japanese], [Build Japanese locale (default=no)]),
        [], [enable_japanese=no]
)
AM_CONDITIONAL([JAPANESE], [test "x$enable_japanese" != xno])

AC_ARG_ENABLE([chinese],
        AS_HELP_STRING([--enable-chinese], [Build Chinese (zh_CN.UTF-8) locale (default=no)]),
        [enable_zh="yes"], [enable_zh=""]
)
AM_CONDITIONAL([CHINESE], [test -n "$enable_zh"])

AC_ARG_ENABLE([chinese-tw],
        AS_HELP_STRING([--enable-chinese-tw], [Build Chinese Traditional (zh_TW.UTF-8) locale (default=no)]),
        [enable_zt="yes"], [enable_zt=""]
)
AM_CONDITIONAL([CHINESE_TW], [test -n "$enable_zt"])

dnl interface font
AC_ARG_ENABLE([misc-fixed],
        AS_HELP_STRING([--enable-misc-fixed], [Set interface font to misc-fixed (default=no)]),
        [], [enable_misc_fixed=no]
)
AM_CONDITIONAL([MISC_FIXED], [test "x$enable_misc_fixed" != xno])

dnl Documentation: the help volumes, the man pages and the dtinfo
dnl information library.  --disable-docs changes the default of the three
dnl switches below it, which can still turn a part back on.
AC_ARG_ENABLE([docs],
        AS_HELP_STRING([--disable-docs], [Do not build the help volumes, the man pages or the dtinfo library (default=build them)]),
        [], [enable_docs=yes]
)
AC_ARG_ENABLE([help-docs],
        AS_HELP_STRING([--disable-help-docs], [Do not build the help volumes (default=as --enable-docs)]),
        [], [enable_help_docs=$enable_docs]
)
AC_ARG_ENABLE([man-pages],
        AS_HELP_STRING([--disable-man-pages], [Do not build the man pages (default=as --enable-docs)]),
        [], [enable_man_pages=$enable_docs]
)
AC_ARG_ENABLE([infolib],
        AS_HELP_STRING([--disable-infolib], [Do not build the dtinfo information library (default=as --enable-docs)]),
        [], [enable_infolib=$enable_docs]
)
AM_CONDITIONAL([BUILD_HELP_DOCS], [test "x$enable_help_docs" != xno])
AM_CONDITIONAL([BUILD_MAN_PAGES], [test "x$enable_man_pages" != xno])
AM_CONDITIONAL([BUILD_INFOLIB], [test "x$enable_infolib" != xno])
build_any_docs=no
if test "x$enable_help_docs" != xno || test "x$enable_man_pages" != xno ||
   test "x$enable_infolib" != xno; then
   build_any_docs=yes
fi
AM_CONDITIONAL([BUILD_DOCS], [test "x$build_any_docs" = xyes])

dnl The tools that build the documentation, which are also installed for
dnl writing new help volumes and information libraries: the help tag
dnl compilers (dthelptag), the DocBook tools (instant, dtdocbook2sdl,
dnl dtdocbook2man, dtdocbook2infolib) and the DtSearch index tools
dnl (dtsrcreate, dtsrload, dtsrindex, ...).  They need Tcl and onsgmls.
AC_ARG_ENABLE([doc-tools],
        AS_HELP_STRING([--enable-doc-tools], [Build and install the documentation tools (default=only when building documentation)]),
        [], [enable_doc_tools=$build_any_docs]
)
if test "x$build_any_docs" = xyes && test "x$enable_doc_tools" = xno; then
   AC_MSG_ERROR([building documentation needs the documentation tools; drop --disable-doc-tools or add --disable-docs])
fi
AM_CONDITIONAL([BUILD_DOC_TOOLS], [test "x$enable_doc_tools" != xno])

dnl Xrender usage
AC_ARG_ENABLE([xrender],
        AS_HELP_STRING([--disable-xrender], [Do not use the Xrender extension (default=auto)]),
        [], [enable_xrender=auto]
)

dnl assert() checks.  A release build defines NDEBUG.
AC_ARG_ENABLE([debug],
        AS_HELP_STRING([--enable-debug], [Keep the assert() checks (default=no, build with -DNDEBUG)]),
        [], [enable_debug=no]
)

dnl Hardening: _FORTIFY_SOURCE, stack protector, stack clash protection
dnl and full RELRO.
AC_ARG_ENABLE([hardening],
        AS_HELP_STRING([--disable-hardening], [Do not build with _FORTIFY_SOURCE, -fstack-protector-strong, -fstack-clash-protection and -z relro -z now (default=enabled)]),
        [], [enable_hardening=yes]
)

dnl Warnings as errors (the CI build uses it).  Only the flags of the
dnl tree's own code (CDE_CFLAGS, CDE_CXXFLAGS) get -Werror: ksh93's
dnl build takes CFLAGS, and the configure tests are not affected.
AC_ARG_ENABLE([werror],
        AS_HELP_STRING([--enable-werror], [Treat the compiler warnings enabled for the tree's code as errors (default=no)]),
        [], [enable_werror=no]
)

dnl Export maps: each library exports only the symbols listed in
dnl build-aux/symbols/lib<name>.sym (see build-aux/gen-exports.py, which
dnl needs a tree built with --disable-export-maps to regenerate them).
AC_ARG_ENABLE([export-maps],
        AS_HELP_STRING([--disable-export-maps], [Export every global symbol of the CDE libraries, not only those in build-aux/symbols (default=enabled)]),
        [], [enable_export_maps=yes]
)

dnl copied from xdm...

AC_ARG_WITH(utmp_file,
  AS_HELP_STRING([--with-utmp-file=<pathname>],
    [specify file to pass to sessreg -u for current logins])
AS_HELP_STRING([--without-utmp-file],
    [specify passing "none" to sessreg -u to not record logins in utmp]),
  [UTMP_FILE="$withval"])
if test "x$UTMP_FILE" = "xyes" ; then
  UTMP_FILE=""
elif test "x$UTMP_FILE" = "xno" ; then
  UTMP_FILE="none"
fi
AC_MSG_CHECKING([for path to file listing current logins for sessreg])
if test "x$UTMP_FILE" = "x" ; then
  AC_MSG_RESULT([use sessreg default])
else
  AC_MSG_RESULT([$UTMP_FILE])
fi
AM_CONDITIONAL(SET_UTMP_FILE, test x$UTMP_FILE != x)
AC_SUBST(UTMP_FILE)

AC_ARG_WITH(wtmp_file,
  AS_HELP_STRING([--with-wtmp-file=<pathname>],
    [specify file to pass to sessreg -w for login history])
AS_HELP_STRING([--without-wtmp-file],
    [specify passing "none" to sessreg -w to not record logins in wtmp]),
  [WTMP_FILE="$withval"])
if test "x$WTMP_FILE" = "xyes" ; then
  WTMP_FILE=""
elif test "x$WTMP_FILE" = "xno" ; then
  WTMP_FILE="none"
fi
AC_MSG_CHECKING([for path to file listing login history for sessreg])
if test "x$WTMP_FILE" = "x" ; then
  AC_MSG_RESULT([use sessreg default])
else
  AC_MSG_RESULT([$WTMP_FILE])
fi
AM_CONDITIONAL(SET_WTMP_FILE, test x$WTMP_FILE != x)
AC_SUBST(WTMP_FILE)

dnl hmmm...
RM="rm -f"
AC_SUBST(RM)
CP="cp -f"
AC_SUBST(CP)

dnl ----------------------------------------------------------------------
dnl Packager-overridable roots.
dnl
dnl Each "top" is exposed both as an automake-substituted variable (for
dnl Makefile.am) and as a C preprocessor macro in cde_config.h (so source
dnl code can use it directly without each Makefile.am having to add a
dnl -DCDE_*_TOP=... define). Defaults preserve the historical CDE layout.
dnl ----------------------------------------------------------------------

AC_ARG_WITH([cde-install-dir],
    AS_HELP_STRING([--with-cde-install-dir=DIR],
        [root of the CDE tree (config, appconfig, app-defaults, ... and the
         paths compiled into the programs) @<:@default=/usr/dt, whatever
         --prefix is@:>@]),
    [CDE_INSTALLATION_TOP="$withval"],
    [CDE_INSTALLATION_TOP="$ac_default_prefix"])

AC_ARG_WITH([cde-data-dir],
    AS_HELP_STRING([--with-cde-data-dir=DIR],
        [directory for CDE arch-independent data
         (backdrops, palettes, types, app-defaults, appconfig, infolib,
         lib/nls) @<:@default=the CDE installation directory@:>@]),
    [CDE_DATA_TOP="$withval"],
    [CDE_DATA_TOP='${CDE_INSTALLATION_TOP}'])

AC_ARG_WITH([cde-config-dir],
    AS_HELP_STRING([--with-cde-config-dir=DIR],
        [directory for CDE system configuration @<:@default=/etc/dt@:>@]),
    [CDE_CONFIGURATION_TOP="$withval"],
    [CDE_CONFIGURATION_TOP="/etc/dt"])

AC_ARG_WITH([cde-state-dir],
    AS_HELP_STRING([--with-cde-state-dir=DIR],
        [directory for CDE runtime state/logs @<:@default=/var/dt@:>@]),
    [CDE_LOGFILES_TOP="$withval"],
    [CDE_LOGFILES_TOP="/var/dt"])

AC_ARG_WITH([cde-libexec-dir],
    AS_HELP_STRING([--with-cde-libexec-dir=DIR],
        [directory for CDE-internal helper binaries
         (rpc.cmsd, ttsession, ...) @<:@default=BINDIR@:>@]),
    [CDE_LIBEXEC_DIR="$withval"],
    [CDE_LIBEXEC_DIR='${bindir}'])

dnl CDE_INSTALLATION_TOP is /usr/dt unless --with-cde-install-dir says
dnl otherwise.  (Upstream CDE makes it follow --prefix; here --prefix=/usr
dnl keeps putting the programs in /usr/bin and the CDE tree in /usr/dt, as
dnl it always did.  A distribution layout passes, for example,
dnl --with-cde-install-dir=/usr --with-cde-data-dir=/usr/share/cde
dnl --with-cde-config-dir=/etc/cde --with-cde-state-dir=/var/lib/cde.)
CDE_USER_TOP=.dt

AC_SUBST(CDE_INSTALLATION_TOP)
AC_SUBST(CDE_DATA_TOP)
AC_SUBST(CDE_CONFIGURATION_TOP)
AC_SUBST(CDE_LOGFILES_TOP)
AC_SUBST(CDE_LIBEXEC_DIR)
AC_SUBST(CDE_USER_TOP)

dnl Resolve the substitution vars now (expanding ${prefix}/${bindir})
dnl so the values land as concrete strings in cde_config.h. The
dnl "prefix=NONE" idiom is the autoconf-recommended way to do this
dnl from configure.ac without writing a custom m4 macro.
test "x$prefix"      = xNONE && prefix="$ac_default_prefix"
test "x$exec_prefix" = xNONE && exec_prefix="$prefix"
eval _cde_install_top="\"$CDE_INSTALLATION_TOP\""
eval _cde_data_top="\"$CDE_DATA_TOP\""
eval _cde_libexec_dir="\"$CDE_LIBEXEC_DIR\""
eval _cde_libexec_dir="\"$_cde_libexec_dir\""
dnl Substitute the resolved values too, so that the files config.status
dnl makes from .in templates (help files, contrib scripts) get real paths.
CDE_INSTALLATION_TOP="$_cde_install_top"
CDE_DATA_TOP="$_cde_data_top"

AC_DEFINE_UNQUOTED([CDE_INSTALLATION_TOP], ["$_cde_install_top"],
    [Top of the CDE installation tree])
AC_DEFINE_UNQUOTED([CDE_DATA_TOP], ["$_cde_data_top"],
    [Top of the CDE arch-independent data tree])
AC_DEFINE_UNQUOTED([CDE_CONFIGURATION_TOP], ["$CDE_CONFIGURATION_TOP"],
    [Top of the CDE system configuration tree])
AC_DEFINE_UNQUOTED([CDE_LOGFILES_TOP], ["$CDE_LOGFILES_TOP"],
    [Top of the CDE runtime state/log tree])
AC_DEFINE_UNQUOTED([CDE_LIBEXEC_DIR], ["$_cde_libexec_dir"],
    [Directory for CDE-internal helper binaries])

dnl This variable will contain a list of programs that were not found,
dnl but are required to build CDE.  At the end, if the variable is
dnl non-empty, an error message will be printed, listing the missing
dnl programs.  We don't bother with the simple expected commands like
dnl ln, cp, etc...

MISSING_PROGS=""

AC_PROG_CPP

dnl we need a real preprocessor, not gcc -E.  We will call it GENCPP.
dnl We will go with BSD's tradcpp here...  This is used for
dnl pre-processing during building of CDE - in the way imake used to
dnl do.  This is not a replacement for cpp, used at runtime by
dnl software such as tt_type_comp.
AC_SUBST(GENCPP, '$(top_builddir)/util/tradcpp/tradcpp')

dnl make sure it's installed
AC_PROG_YACC
if test -z "$ac_cv_prog_YACC"; then
   MISSING_PROGS="[bison or byacc] ${MISSING_PROGS}"
fi

AC_PROG_LEX(noyywrap)
if test -z "$ac_cv_prog_LEX"; then
   MISSING_PROGS="[flex or lex] ${MISSING_PROGS}"
fi

AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_SED
AC_PROG_AWK
AC_PROG_GREP

dnl AC_PROG_AR
AC_PROG_RANLIB

AC_C_CONST
AC_C_BIGENDIAN
AC_C_INLINE
AC_C_CHAR_UNSIGNED
AC_C_FLEXIBLE_ARRAY_MEMBER
AC_SYS_POSIX_TERMIOS

AX_PTHREAD

AC_PATH_X
AC_PATH_XTRA

dnl check some compiler flags
CDE_CHECK_CFLAG([-Wall])
CDE_CHECK_CXXFLAG([-Wall])

dnl C++ only: string literals passed as char * all over the old C++ code.
CDE_CHECK_CXXFLAG([-Wno-write-strings])

CDE_CHECK_CFLAG([-Wno-unused-result])
CDE_CHECK_CXXFLAG([-Wno-unused-result])

dnl A curated part of -Wextra, enabled warning by warning (so no -Wno-
dnl undoes anything), and the tree builds without any of them.  With GCC
dnl 16 and the whole tree (--disable-docs) -Wextra reported, counting each
dnl source line once:
dnl
dnl   enabled, and every site fixed:
dnl     -Wempty-body (101: debug macros that expanded to nothing in an if or
dnl     else, and the "...} else" statement macros), -Wimplicit-fallthrough
dnl     (99: each case checked; marked FALLTHROUGH, or a missing break
dnl     added where the fall-through was a bug), -Wenum-conversion (75),
dnl     -Wignored-qualifiers (204, nearly all "const int f()" in the C++
dnl     programs), -Wcalloc-transposed-args (34), -Wdeprecated-copy (37,
dnl     C++), -Wtype-limits (12), -Wclobbered (10), -Wshift-negative-value
dnl     (5), -Wunused-but-set-parameter (5), -Wabsolute-value (3),
dnl     -Wold-style-declaration (2), and the -Wextra warnings that reported
dnl     nothing: -Wmissing-parameter-type, -Woverride-init,
dnl     -Wstring-compare, -Wexpansion-to-defined, -Wredundant-move (C++).
dnl     Among the fixed sites were real bugs: an invalid search date never
dnl     rejected on LP64 (DtSearch), mblen() failures not seen (dtcm),
dnl     softkey strings wrapping at 256 bytes (DtTerm), wrong help or error
dnl     texts after a missing break (dtmail, dtstyle).
dnl
dnl   not enabled, with the reason:
dnl     -Wunused-parameter (6126): Xt, Motif, ToolTalk and RPC callbacks
dnl       must take the parameters their prototypes declare; marking each
dnl       one unused would touch every callback in the tree and find
dnl       nothing.
dnl     -Wsign-compare (687): int counters against Cardinal, size_t and
dnl       other unsigned values that are never negative there; each site
dnl       needs a cast or a type change, and type changes in this code
dnl       change behaviour where a value does wrap.  The real cases of this
dnl       kind that -Wtype-limits reports are fixed.
dnl     -Wcast-function-type (495): the casts the APIs are built on
dnl       ((xdrproc_t) xdr_foo, (XtCallbackProc), (XtActionProc), qsort
dnl       comparators, the DtEncap and ToolTalk dispatch tables); casting
dnl       through void (*)(void) instead would hide the same thing.
dnl     -Wmissing-field-initializers (147): tables (class records, locale
dnl       tables, resource lists) that leave their trailing members to the
dnl       zero initialisation C guarantees.
dnl     The few C++ diagnostics -Wextra has no separate option for (a base
dnl       class not initialised in a user copy constructor, 3) come only
dnl       with -Wextra itself.
dnl
dnl ksh93's own build (programs/dtksh/ksh93) does not get these flags, and
dnl neither does the copy of its init.c that dtksh compiles (see
dnl programs/dtksh/Makefile.am).  --enable-werror adds -Werror to them.
C_EXTRA_WARNINGS=""
CXX_EXTRA_WARNINGS=""
for cde_warning in -Wempty-body -Wimplicit-fallthrough -Wignored-qualifiers \
		   -Wcalloc-transposed-args -Wtype-limits -Wclobbered \
		   -Wshift-negative-value -Wunused-but-set-parameter \
		   -Wstring-compare -Wexpansion-to-defined
do
	CDE_CHECK_CFLAG([$cde_warning], [C_EXTRA_WARNINGS])
	CDE_CHECK_CXXFLAG([$cde_warning], [CXX_EXTRA_WARNINGS])
done
for cde_warning in -Wenum-conversion -Wabsolute-value -Wold-style-declaration \
		   -Wmissing-parameter-type -Woverride-init
do
	CDE_CHECK_CFLAG([$cde_warning], [C_EXTRA_WARNINGS])
done
for cde_warning in -Wdeprecated-copy -Wredundant-move
do
	CDE_CHECK_CXXFLAG([$cde_warning], [CXX_EXTRA_WARNINGS])
done
if test "x$enable_werror" = xyes; then
	CDE_CHECK_CFLAG([-Werror], [C_EXTRA_WARNINGS])
	CDE_CHECK_CXXFLAG([-Werror], [CXX_EXTRA_WARNINGS])
fi
C_EXTRA_WARNINGS=`echo ${C_EXTRA_WARNINGS}`
CXX_EXTRA_WARNINGS=`echo ${CXX_EXTRA_WARNINGS}`
AC_SUBST(C_EXTRA_WARNINGS)
AC_SUBST(CXX_EXTRA_WARNINGS)

dnl -fno-strict-aliasing stays tree-wide.  -Wstrict-aliasing=2 reports 529
dnl places in about 150 files of more than 30 directories (rpcgen's
dnl xdr_pointer((char **)&p->next), XGetWindowProperty((unsigned char **)
dnl &prop), Xt class records, the raima and ISAM record casts...), so the
dnl flag could not be limited to a few directories, and the warning also
dnl misses real cases (GifUtils.c read char * objects through pixel **,
dnl reported only by the noisy level 1).  Measured with the whole tree
dnl built both ways: strict aliasing changes the code of 889 of 1646
dnl objects but shrinks .text by only 0.13%, and dtsvcbench and ttbench
dnl show no difference beyond their run-to-run noise.
CDE_CHECK_CFLAG([-fno-strict-aliasing])
CDE_CHECK_CXXFLAG([-fno-strict-aliasing])

dnl Hardening flags (--enable-hardening, the default)
HARDEN_CPPFLAGS=""
HARDEN_CFLAGS=""
HARDEN_CXXFLAGS=""
HARDEN_LDFLAGS=""
FORTIFY_SOURCE_2=""
FORTIFY_SOURCE_1=""
if test "x$enable_hardening" != xno; then
   dnl _FORTIFY_SOURCE needs optimisation (see build-aux/cde-fortify.h).
   dnl Level 3 needs GCC 12 and glibc 2.34; older glibc treats it as
   dnl level 2.
   AC_MSG_CHECKING([whether _FORTIFY_SOURCE can be used])
   AC_LANG_PUSH([C])
   ac_saved_cppflags="$CPPFLAGS"
   CPPFLAGS="$CPPFLAGS -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3"
   ac_saved_ccflags="$CFLAGS"
   CFLAGS="-O2 -Werror"
   AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <string.h>
#include <stdio.h>]], [[char b[4]; snprintf(b, sizeof(b), "%d", 1); return (int)strlen(b);]])],
      [fortify=yes], [fortify=no])
   CPPFLAGS="$ac_saved_cppflags"
   CFLAGS="$ac_saved_ccflags"
   AC_LANG_POP([C])
   AC_MSG_RESULT([$fortify])
   if test "$fortify" = yes; then
      dnl build-aux/cde-fortify.h defines _FORTIFY_SOURCE=3 only when
      dnl optimising (single quotes: make expands $(top_srcdir))
      HARDEN_CPPFLAGS='-U_FORTIFY_SOURCE -include $(top_srcdir)/build-aux/cde-fortify.h'
      dnl for targets whose code trips level 3's -Wformat-overflow checks
      dnl until it is fixed (added to their _CFLAGS/_CXXFLAGS)
      FORTIFY_SOURCE_2="-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2"
      dnl for code that uses %n in a format string built at run time,
      dnl which level 2 and above abort on
      FORTIFY_SOURCE_1="-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=1"
   fi
   CDE_CHECK_CFLAG([-fstack-protector-strong], [HARDEN_CFLAGS])
   CDE_CHECK_CXXFLAG([-fstack-protector-strong], [HARDEN_CXXFLAGS])
   CDE_CHECK_CFLAG([-fstack-clash-protection], [HARDEN_CFLAGS])
   CDE_CHECK_CXXFLAG([-fstack-clash-protection], [HARDEN_CXXFLAGS])
   CDE_CHECK_LDFLAG([-Wl,-z,relro], [HARDEN_LDFLAGS])
   CDE_CHECK_LDFLAG([-Wl,-z,now], [HARDEN_LDFLAGS])
fi
AC_SUBST(FORTIFY_SOURCE_2)
AC_SUBST(FORTIFY_SOURCE_1)
dnl for code built by its own build system (ksh93)
AC_SUBST(HARDEN_CFLAGS)

dnl Linking.  Programs and libraries drop the libraries they do not use
dnl (--as-needed; older libtool releases move it after the libraries on a
dnl shared library's link line, where it does nothing, so each library's
dnl _LIBADD still names exactly what it uses).  The libraries must record
dnl every library they use (-no-undefined -Wl,-z,defs, CDE_LIB_LDFLAGS),
dnl or --as-needed on a program could drop a library that only a CDE
dnl library needs.
LINK_LDFLAGS=""
CDE_CHECK_LDFLAG([-Wl,-O1], [LINK_LDFLAGS])
CDE_CHECK_LDFLAG([-Wl,--as-needed], [LINK_LDFLAGS])
CDE_CHECK_LDFLAG([-Wl,--sort-common], [LINK_LDFLAGS])
CDE_LIB_LDFLAGS="-no-undefined"
CDE_CHECK_LDFLAG([-Wl,-z,defs], [CDE_LIB_LDFLAGS])
AC_SUBST(CDE_LIB_LDFLAGS)

dnl Another linker (--with-linker=mold, lld, gold or bfd): -fuse-ld=NAME.
dnl mold and lld link much faster than GNU ld, which shows in the ~100
dnl links (and libtool relinks at install time) of the tree.
AC_ARG_WITH([linker],
        AS_HELP_STRING([--with-linker=NAME], [Link with the linker NAME (mold, lld, gold, bfd) through -fuse-ld=NAME (default: the compiler's default linker)]),
        [], [with_linker=no]
)
if test "x$with_linker" != xno && test "x$with_linker" != xyes; then
   fuse_ld=""
   CDE_CHECK_LDFLAG([-fuse-ld=$with_linker], [fuse_ld])
   if test -z "$fuse_ld"; then
      AC_MSG_ERROR([--with-linker=$with_linker: $CC cannot link with -fuse-ld=$with_linker])
   fi
   LINK_LDFLAGS="$fuse_ld $LINK_LDFLAGS"
elif test "x$with_linker" = xyes; then
   AC_MSG_ERROR([--with-linker needs the name of a linker, e.g. --with-linker=mold])
fi

dnl Build-tree paths out of the objects (__FILE__, the debug information):
dnl the tree's directory is recorded as ".", which makes the build
dnl reproducible and lets ccache share objects between trees (with its
dnl base_dir set).  A debugger then finds the sources from the top of the
dnl tree, or through its source path ("directory" in gdb).
AC_ARG_ENABLE([file-prefix-map],
        AS_HELP_STRING([--disable-file-prefix-map], [Record the absolute build directory in the objects and debug information (default=record "." through -ffile-prefix-map)]),
        [], [enable_file_prefix_map=yes]
)
FILE_PREFIX_CFLAGS=""
FILE_PREFIX_CXXFLAGS=""
if test "x$enable_file_prefix_map" != xno; then
   prefix_map_c=""
   prefix_map_cxx=""
   CDE_CHECK_CFLAG([-ffile-prefix-map=/=/], [prefix_map_c])
   CDE_CHECK_CXXFLAG([-ffile-prefix-map=/=/], [prefix_map_cxx])
   dnl (single quotes: make expands the directories)
   prefix_map='-ffile-prefix-map=$(abs_top_srcdir)=. -ffile-prefix-map=$(abs_top_builddir)=.'
   if test -n "$prefix_map_c"; then
      FILE_PREFIX_CFLAGS="$prefix_map"
   fi
   if test -n "$prefix_map_cxx"; then
      FILE_PREFIX_CXXFLAGS="$prefix_map"
   fi
fi
dnl (also on its own: the bundled ksh93's build takes it, see
dnl programs/dtksh/Makefile.am)
AC_SUBST(FILE_PREFIX_CFLAGS)

dnl export maps need a linker that takes GNU version scripts
if test "x$enable_export_maps" != xno; then
   AC_MSG_CHECKING([if the linker supports version scripts])
   AC_LANG_PUSH([C])
   ac_saved_ldflags="$LDFLAGS"
   echo '{ global: main; local: *; };' > conftest.ver
   LDFLAGS="$LDFLAGS -Wl,--version-script=conftest.ver"
   AC_LINK_IFELSE([AC_LANG_PROGRAM([])], [ld_vscript=yes], [ld_vscript=no])
   LDFLAGS="$ac_saved_ldflags"
   rm -f conftest.ver
   AC_LANG_POP([C])
   AC_MSG_RESULT([$ld_vscript])
   if test "$ld_vscript" != yes; then
      if test "x$enable_export_maps" = xyes; then
         AC_MSG_WARN([the linker does not take version scripts; the libraries export every global symbol])
      fi
      enable_export_maps=no
   fi
fi
AM_CONDITIONAL([EXPORT_MAPS], [test "x$enable_export_maps" != xno])

dnl assert() checks (--enable-debug).  A release build defines NDEBUG.
dnl Code that is not ready for that yet (asserts guarding the only use of
dnl a variable, assert(0) in place of a return, its own NDEBUG-keyed
dnl assert macro) adds $(KEEP_ASSERTS) to its _CPPFLAGS, _CFLAGS or
dnl _CXXFLAGS, after $(CDE_CPPFLAGS), and keeps its checks.
DEBUG_CPPFLAGS=""
KEEP_ASSERTS=""
if test "x$enable_debug" = xno; then
   DEBUG_CPPFLAGS="-DNDEBUG"
   KEEP_ASSERTS="-UNDEBUG"
fi
AC_SUBST(KEEP_ASSERTS)

dnl Add X11 goodies here
EXTRA_LIBS="${X_LIBS} ${EXTRA_LIBS}"
EXTRA_INCS="${X_CFLAGS} ${EXTRA_INCS}"

AC_FUNC_FORK

dnl programs with full paths

dnl - KSH, some systems call it as ksh93
AC_PATH_PROG(KSH, ksh)
if test -z "$ac_cv_path_KSH"; then
   AC_PATH_PROG(KSH, ksh93)
   if test -z "$ac_cv_path_KSH"; then
      AC_PATH_PROG(KSH, mksh)
      if test -z "$ac_cv_path_KSH"; then
        MISSING_PROGS="[ksh or ksh93] ${MISSING_PROGS}"
      fi
   fi
fi

AC_PATH_PROG(XRDB, xrdb)
if test -z "$ac_cv_path_XRDB"; then
   MISSING_PROGS="xrdb ${MISSING_PROGS}"
fi

AC_PATH_PROG(PERL, perl)
if test -z "$ac_cv_path_PERL"; then
   MISSING_PROGS="perl ${MISSING_PROGS}"
fi


dnl we need to use cpp for some things, like tooltalk and other
dnl runtime uses.  So look for the system's cpp.  NOTE: this is NOT
dnl the CPP (gcc -E) set by AC_PROG_CPP.  At this point I don't know
dnl if we should even bother looking for that as we can't really use
dnl it.
AC_PATH_PROG(CPP_PROGRAM, cpp, ,
       [/lib:/usr/bin:/usr/ccs/lib/:/usr/lib:/usr/libexec:/opt/langtools/lbin:$PATH])
if test -z "$ac_cv_path_CPP_PROGRAM"; then
   MISSING_PROGS="cpp ${MISSING_PROGS}"
fi

dnl major external program dependencies
AC_CHECK_PROGS(BDFTOPCF, bdftopcf)
if test -z "$ac_cv_prog_BDFTOPCF"; then
   MISSING_PROGS="bdftopcf ${MISSING_PROGS}"
fi
AC_CHECK_PROGS(MKFONTDIR, mkfontdir)
if test -z "$ac_cv_prog_MKFONTDIR"; then
   MISSING_PROGS="mkfontdir ${MISSING_PROGS}"
fi
AC_CHECK_PROGS(GZIP, gzip)
if test -z "$ac_cv_prog_GZIP"; then
   MISSING_PROGS="gzip ${MISSING_PROGS}"
fi
AC_CHECK_PROGS(M4, m4)
if test -z "$ac_cv_prog_M4"; then
   MISSING_PROGS="m4 ${MISSING_PROGS}"
fi
AC_CHECK_PROGS(RPCGEN, rpcgen)
if test -z "$ac_cv_prog_RPCGEN"; then
   MISSING_PROGS="rpcgen ${MISSING_PROGS}"
fi
AC_CHECK_PROGS(GENCAT, gencat)
if test -z "$ac_cv_prog_GENCAT"; then
   MISSING_PROGS="gencat ${MISSING_PROGS}"
fi
if test "x$enable_doc_tools" != xno; then
   AC_CHECK_PROGS(ONSGMLS, onsgmls)
   if test -z "$ac_cv_prog_ONSGMLS"; then
      MISSING_PROGS="onsgmls ${MISSING_PROGS}"
   fi
fi
AC_CHECK_PROGS(SESSREG, sessreg)
if test -z "$ac_cv_prog_SESSREG"; then
   MISSING_PROGS="SESSREG ${MISSING_PROGS}"
fi

dnl Used to check if program 'tic' is available to install terminfo files
AC_CHECK_PROGS(TIC, tic, :)

dnl headers
AC_CHECK_HEADERS([locale.h poll.h sys/poll.h security/pam_appl.h utempter.h])

dnl Xrender header
if test "x$enable_xrender" != xno; then
   AC_CHECK_HEADERS([X11/extensions/Xrender.h], [],
       [if test "x$enable_xrender" = xyes; then
           AC_MSG_ERROR([--enable-xrender was given but X11/extensions/Xrender.h was not found])
        fi
        enable_xrender=no], [#include <X11/Xlib.h>])
fi

dnl libraries
dnl libm and libcrypt go only to the targets that use them ($(MATHLIB),
dnl $(CRYPTLIB)), not into LIBS, which every library and program links.
MATHLIB=""
AC_CHECK_LIB(m, cosf, [MATHLIB="-lm"])
AC_SUBST(MATHLIB)
AC_SEARCH_LIBS(dlopen, [dl dld], [], [])

CRYPTLIB=""
AC_CHECK_LIB(crypt, crypt, [CRYPTLIB="-lcrypt"])
AC_SUBST(CRYPTLIB)
dnl this should be configurable, for now it is required
AC_CHECK_LIB(Xinerama, XineramaQueryScreens,
                [SOURCE_CPP_DEFINES="${SOURCE_CPP_DEFINES} -DUSE_XINERAMA";
                 XINERAMALIB="-lXinerama"],
                [AC_MSG_ERROR([libXinerama not found])], $X_LIBS)

dnl libXrandr (1.3 or later) for dtrandr, which is only built if it is
dnl there
XRANDRLIB=""
have_xrandr=no
AC_CHECK_LIB(Xrandr, XRRGetScreenResourcesCurrent,
             [save_CPPFLAGS="${CPPFLAGS}"
              CPPFLAGS="${CPPFLAGS} ${X_CFLAGS}"
              AC_CHECK_HEADER([X11/extensions/Xrandr.h],
                              [XRANDRLIB="-lXrandr"; have_xrandr=yes], [],
                              [#include <X11/Xlib.h>])
              CPPFLAGS="${save_CPPFLAGS}"],
             [], [$X_LIBS -lX11])
if test "x$have_xrandr" != xyes; then
   AC_MSG_WARN([libXrandr (1.3 or later) not found, dtrandr will not be built])
fi
AC_SUBST(XRANDRLIB)
AM_CONDITIONAL([BUILD_DTRANDR], [test "x$have_xrandr" = xyes])

dnl Sun RPC: libtirpc where the C library has none (glibc 2.32 and
dnl later), found through pkg-config, with a plain library check as a
dnl fallback for systems without libtirpc.pc.
TIRPC_CPPFLAGS=""
TIRPCLIB=""
PKG_CHECK_MODULES([TIRPC], [libtirpc],
      [TIRPC_CPPFLAGS="-DOPT_TIRPC ${TIRPC_CFLAGS}"; TIRPCLIB="${TIRPC_LIBS}"],
      [AC_CHECK_LIB(tirpc, svc_register,
            [TIRPC_CPPFLAGS="-DOPT_TIRPC -I/usr/include/tirpc";
             TIRPCLIB=-ltirpc])])
AC_SUBST(TIRPCLIB)

dnl see if the rpc_inline_t type exists
save_CPPFLAGS="${CPPFLAGS}"
CPPFLAGS="${CPPFLAGS} ${TIRPC_CPPFLAGS}"
AC_CHECK_TYPE(rpc_inline_t, [],
   [AC_DEFINE_UNQUOTED([rpc_inline_t], [int32_t],
       [Define to int32_t if not defined by including rpc/rpc.h])],
[
AC_INCLUDES_DEFAULT
#include <rpc/rpc.h>
])
CPPFLAGS="${save_CPPFLAGS}"

dnl check sizeof time_t for RPC
AC_CHECK_SIZEOF([int])
AC_CHECK_SIZEOF([long])
AC_CHECK_SIZEOF([long long])
AC_CHECK_SIZEOF([time_t])

AC_STRUCT_TIMEZONE
AC_CHECK_DECLS([timezone],,,
        [#include <time.h>])
AC_CHECK_MEMBERS([struct tm.tm_gmtoff],,,
        [#include <time.h>])

dnl jpeg
AC_CHECK_LIB(jpeg, jpeg_read_header, [JPEGLIB="-ljpeg"],
             [AC_MSG_ERROR([libjpeg not found, please install it])],
             [${EXTRA_LIBS}])
AC_SUBST(JPEGLIB)

dnl lmdb
AC_CHECK_LIB(lmdb, mdb_version, [LMDBLIB="-llmdb"],
             [AC_MSG_ERROR([liblmdb not found, please install it])],
             [${EXTRA_LIBS}])
AC_SUBST(LMDBLIB)

dnl Setup XTOOLLIB - we do it in this specific order to avoid ordering
dnl issues.  XTOOLLIB is every X library.  The CDE libraries and programs
dnl name the ones they use with the single-library variables (X11LIB,
dnl XTLIB, XMLIB, ...).
XTOOLLIB=""
AC_CHECK_LIB(X11, XOpenDisplay, [XTOOLLIB="-lX11"; X11LIB="-lX11"], ,
                  [${EXTRA_LIBS}])
AC_CHECK_LIB(Xext, XextFindDisplay, [XTOOLLIB="-lXext ${XTOOLLIB}"; XEXTLIB="-lXext"], ,
                  [${EXTRA_LIBS}])
if test "x$enable_xrender" != "xno" ; then
   AC_CHECK_LIB(Xrender, XRenderComposite,
                         [XTOOLLIB="-lXrender ${XTOOLLIB}"; XRENDERLIB="-lXrender";
                          SOURCE_CPP_DEFINES="${SOURCE_CPP_DEFINES} -DUSE_XRENDER"],
                         [if test "x$enable_xrender" = xyes; then
                             AC_MSG_ERROR([--enable-xrender was given but libXrender was not found])
                          fi],
                         [${EXTRA_LIBS}])
fi
AC_CHECK_LIB(Xau, XauReadAuth, [XTOOLLIB="-lXau ${XTOOLLIB}"; XAULIB="-lXau"], ,
                  [${EXTRA_LIBS}])
AC_CHECK_LIB(Xpm, XpmLibraryVersion, [XTOOLLIB="-lXpm ${XTOOLLIB}"; XPMLIB="-lXpm"], ,
                  [${EXTRA_LIBS}])
AC_CHECK_LIB(Xt, XtInitialize, [XTOOLLIB="-lXt ${XTOOLLIB}"; XTLIB="-lXt"], ,
                 [${EXTRA_LIBS}])
AC_CHECK_LIB(Xmu, XmuMakeAtom, [XTOOLLIB="-lXmu ${XTOOLLIB}"; XMULIB="-lXmu"], ,
                 [${EXTRA_LIBS}])
AC_CHECK_LIB(ICE, IceCloseConnection, [XTOOLLIB="-lICE ${XTOOLLIB}"; ICELIB="-lICE"], ,
                  [${EXTRA_LIBS}])
dnl dtsession's session manager (XSMP) listens on local transports only,
dnl through libICE's private _IceTransNoListen() as xsm does
AC_CHECK_LIB(ICE, _IceTransNoListen,
             [AC_DEFINE([HAVE__ICETRANSNOLISTEN], [1],
                        [Define if libICE has _IceTransNoListen()])], ,
             [${EXTRA_LIBS}])
AC_CHECK_LIB(SM, SmcOpenConnection, [XTOOLLIB="-lSM ${XTOOLLIB}"; SMLIB="-lSM"], ,
                 [${EXTRA_LIBS}])
AC_CHECK_LIB(Xm, XmTextSetString, [XTOOLLIB="-lXm ${XTOOLLIB}"; XMLIB="-lXm"],
                 [AC_MSG_ERROR([libXm (Motif) not found, please install it])],
                 [${EXTRA_LIBS}])
XTOOLLIB="${EXTRA_LIBS} ${X_EXTRA_LIBS} ${XTOOLLIB}"
AC_SUBST([XTOOLLIB])
X11LIB="${X_EXTRA_LIBS} ${X11LIB}"
AC_SUBST([X11LIB])
AC_SUBST([XEXTLIB])
AC_SUBST([XRENDERLIB])
AC_SUBST([XAULIB])
AC_SUBST([XPMLIB])
AC_SUBST([XTLIB])
AC_SUBST([XMULIB])
AC_SUBST([ICELIB])
AC_SUBST([SMLIB])
AC_SUBST([XMLIB])
AC_SUBST([XINERAMALIB])

dnl Motif headers, the resource manager (libDtMrm) and the UIL compiler
dnl library (dtbuilder)
save_CPPFLAGS="${CPPFLAGS}"
CPPFLAGS="${CPPFLAGS} ${EXTRA_INCS}"
AC_CHECK_HEADER([Xm/Xm.h], [],
                [AC_MSG_ERROR([Motif headers (Xm/Xm.h) not found, please install them])])
AC_CHECK_HEADER([X11/bitmaps/xm_hour16], [],
                [AC_MSG_ERROR([Motif bitmaps (X11/bitmaps/xm_*) not found, please install them])])
CPPFLAGS="${save_CPPFLAGS}"
AC_CHECK_LIB(Mrm, MrmOpenHierarchyPerDisplay, [MRESOURCELIB="-lMrm"],
             [AC_MSG_ERROR([libMrm (Motif) not found, please install it])],
             [${XTOOLLIB}])
AC_SUBST(MRESOURCELIB)
AC_CHECK_LIB(Uil, Uil, [:],
             [AC_MSG_ERROR([libUil (Motif) not found, please install it])],
             [${MRESOURCELIB} ${XTOOLLIB}])

dnl only needed by dtlogin
AC_CHECK_LIB(Xdmcp, XdmcpFlush, [XDMCPLIB="-lXdmcp"], [XDMCPLIB=""],
                 [${EXTRA_LIBS}])
AC_SUBST(XDMCPLIB)

dnl iconv - part of libc on glibc, musl and the BSDs, a separate libiconv
dnl elsewhere.  Checked here rather than with gettext's AM_ICONV, which
dnl makes autoreconf run autopoint and copy gettext files into the tree.
AC_MSG_CHECKING([for library containing iconv])
LTLIBICONV=no
save_LIBS="${LIBS}"
for lib in "" -liconv; do
   LIBS="${save_LIBS} ${lib}"
   AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <stdlib.h>
#include <iconv.h>]], [[iconv_t cd = iconv_open("", "");
iconv(cd, NULL, NULL, NULL, NULL);
iconv_close(cd);]])], [LTLIBICONV="${lib}"; break])
done
LIBS="${save_LIBS}"
if test "x${LTLIBICONV}" = "xno"; then
   AC_MSG_RESULT([no])
   AC_MSG_ERROR([iconv not found, please install it])
fi
AC_MSG_RESULT([${LTLIBICONV:-none required}])
AC_SUBST(LTLIBICONV)

AC_MSG_CHECKING([whether iconv takes a const input buffer])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <stdlib.h>
#include <iconv.h>
extern size_t iconv(iconv_t cd, const char **inbuf, size_t *inbytesleft,
                    char **outbuf, size_t *outbytesleft);]], [])],
   [iconv_const=const; AC_MSG_RESULT([yes])],
   [iconv_const=; AC_MSG_RESULT([no])])
AC_DEFINE_UNQUOTED([ICONV_CONST], [${iconv_const}],
   [Define as const if the declaration of iconv() needs const.])

dnl figure out pam support
dnl Right now this only works on linux and netbsd (9.2 tested)

dnl PAM configuration directory - configurable for non-root installs
AC_ARG_WITH([pam-dir],
  AS_HELP_STRING([--with-pam-dir=DIR],
    [directory for PAM configuration files (default: /etc/pam.d; /usr/local/etc/pam.d on FreeBSD)]),
  [PAM_CONFDIR="$withval"],
  [if test "$build_freebsd" = "yes"; then
     PAM_CONFDIR="/usr/local/etc/pam.d"
   else
     PAM_CONFDIR="/etc/pam.d"
   fi])
AC_SUBST(PAM_CONFDIR)

if test "$supports_pam" = "yes"
then
        AC_CHECK_LIB(pam, pam_start,
             [SOURCE_CPP_DEFINES="${SOURCE_CPP_DEFINES} -DHAS_PAM_LIBRARY";
	      PAMLIB="-lpam";
              AC_MSG_NOTICE([PAM configuration directory: $PAM_CONFDIR])])
        AC_SUBST(PAMLIB)
else
        AC_CHECK_LIB(pam, NOTSUPPORTED)
fi

AM_CONDITIONAL([HAS_PAM_LIBRARY], [test "x$ac_cv_lib_pam_pam_start" = "xyes"])

dnl figure out utempter support

AC_CHECK_LIB(utempter, utempter_add_record,
             [SOURCE_CPP_DEFINES="${SOURCE_CPP_DEFINES} -DHAS_UTEMPTER_LIBRARY"])

AM_CONDITIONAL([HAS_UTEMPTER_LIBRARY], [test "x$ac_cv_lib_utempter_utempter_add_record" = "xyes"])

dnl libcups: dtprintinfo reads queues, queue states and jobs from the CUPS
dnl scheduler with it (objects/PrintObj/Cups.C) instead of running lpstat
dnl and lpq.  cups-config gives the flags (libcups has no .pc file before
dnl CUPS 2.5); --without-cups keeps lpstat and lpq.
AC_ARG_WITH([cups],
  AS_HELP_STRING([--without-cups],
    [do not use libcups in dtprintinfo (default: use it when cups-config is found)]),
  [], [with_cups=check])
have_cups=no
CUPS_CFLAGS=""
CUPS_LIBS=""
if test "x$with_cups" != xno; then
   AC_PATH_PROG([CUPS_CONFIG], [cups-config], [no])
   if test "x$CUPS_CONFIG" != xno; then
      CUPS_CFLAGS=`$CUPS_CONFIG --cflags`
      CUPS_LIBS=`$CUPS_CONFIG --libs`
      AC_MSG_CHECKING([whether libcups can be used])
      ac_saved_cppflags="$CPPFLAGS"
      ac_saved_libs="$LIBS"
      CPPFLAGS="$CPPFLAGS $CUPS_CFLAGS"
      LIBS="$CUPS_LIBS $LIBS"
      AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <cups/cups.h>]],
         [[cups_dest_t *d = cupsGetNamedDest(CUPS_HTTP_DEFAULT, NULL, NULL);
           cupsFreeDests(d ? 1 : 0, d); return 0;]])],
         [have_cups=yes])
      CPPFLAGS="$ac_saved_cppflags"
      LIBS="$ac_saved_libs"
      AC_MSG_RESULT([$have_cups])
   fi
   if test "x$have_cups" != xyes; then
      CUPS_CFLAGS=""
      CUPS_LIBS=""
      if test "x$with_cups" = xyes; then
         AC_MSG_ERROR([--with-cups was given, but cups-config or a usable libcups was not found])
      fi
   fi
fi
AC_SUBST(CUPS_CFLAGS)
AC_SUBST(CUPS_LIBS)
AM_CONDITIONAL([HAVE_CUPS], [test "x$have_cups" = xyes])

dnl Figure out TCL (only instant, one of the documentation tools, uses it)

if test "x$enable_doc_tools" != xno; then
   SC_PATH_TCLCONFIG
   SC_LOAD_TCLCONFIG
fi

dnl check MISSING_PROGS - error out here if there's stuff in it.

if test -n "$MISSING_PROGS"; then
   AC_MSG_ERROR([Please install the following REQUIRED programs: ${MISSING_PROGS}])
fi

dnl Project compiler and linker flags.  CPPFLAGS, CFLAGS, CXXFLAGS and
dnl LDFLAGS belong to the user (make CFLAGS=-O0, sanitizers, PGO), so the
dnl flags the build needs go to CDE_CPPFLAGS, CDE_CFLAGS, CDE_CXXFLAGS and
dnl CDE_LDFLAGS instead.  They reach every Makefile through AM_CPPFLAGS,
dnl AM_CFLAGS, AM_CXXFLAGS and AM_LDFLAGS, which are substituted below;
dnl a Makefile.am that sets one of those, or a per-target _CPPFLAGS,
dnl _CFLAGS, _CXXFLAGS or _LDFLAGS (which replace the AM_ ones), starts it
dnl with the matching $(CDE_...) variable.
dnl (-I$(top_srcdir)/include: automake's default include path has only
dnl the include/ of the build directory, which in a VPATH build holds just
dnl the generated headers; in-tree it is that same directory.)
CDE_CPPFLAGS="-I\$(top_srcdir)/include ${SOURCE_CPP_DEFINES} ${CPP_COMPILER_FLAGS} ${TIRPC_CPPFLAGS} ${EXTRA_INCS} ${HARDEN_CPPFLAGS} ${DEBUG_CPPFLAGS}"
CDE_CFLAGS="-std=${C_STD} ${C_COMPILER_FLAGS} ${C_EXTRA_WARNINGS} ${HARDEN_CFLAGS} ${PTHREAD_CFLAGS} ${LTO_CFLAGS} ${FILE_PREFIX_CFLAGS}"
CDE_CXXFLAGS="-std=${CXX_STD} ${CXX_COMPILER_FLAGS} ${CXX_EXTRA_WARNINGS} ${HARDEN_CXXFLAGS} ${PTHREAD_CFLAGS} ${LTO_CXXFLAGS} ${FILE_PREFIX_CXXFLAGS}"
CDE_LDFLAGS="${HARDEN_LDFLAGS} ${LINK_LDFLAGS} ${LTO_LDFLAGS}"
dnl squeeze the whitespace
CDE_CPPFLAGS=`echo ${CDE_CPPFLAGS}`
CDE_CFLAGS=`echo ${CDE_CFLAGS}`
CDE_CXXFLAGS=`echo ${CDE_CXXFLAGS}`
CDE_LDFLAGS=`echo ${CDE_LDFLAGS}`
AC_SUBST(CDE_CPPFLAGS)
AC_SUBST(CDE_CFLAGS)
AC_SUBST(CDE_CXXFLAGS)
AC_SUBST(CDE_LDFLAGS)
AC_SUBST([AM_CPPFLAGS], ['$(CDE_CPPFLAGS)'])
AC_SUBST([AM_CFLAGS], ['$(CDE_CFLAGS)'])
AC_SUBST([AM_CXXFLAGS], ['$(CDE_CXXFLAGS)'])
AC_SUBST([AM_LDFLAGS], ['$(CDE_LDFLAGS)'])

LIBS="${EXTRA_LIBS} ${LIBS} ${PTHREAD_LIBS}"

dnl All of the makefiles we need to generate go here...
AC_CONFIG_FILES([
Makefile

util/Makefile
util/tradcpp/Makefile

lib/Makefile

lib/DtXinerama/Makefile

lib/tt/Makefile
lib/tt/mini_isam/Makefile
lib/tt/slib/Makefile
lib/tt/lib/Makefile
lib/tt/bin/Makefile
lib/tt/bin/shell/Makefile
lib/tt/bin/ttauth/Makefile
lib/tt/bin/tttar/Makefile
lib/tt/bin/tt_type_comp/Makefile
lib/tt/bin/tttrace/Makefile
lib/tt/bin/dbck/Makefile
lib/tt/bin/ttdbserverd/Makefile
lib/tt/bin/ttsession/Makefile

lib/DtSvc/Makefile

lib/DtSearch/Makefile
lib/DtSearch/raima/Makefile

lib/DtWidget/Makefile

lib/DtHelp/Makefile
lib/DtHelp/il/Makefile

lib/DtPrint/Makefile

lib/DtTerm/Term/Makefile
lib/DtTerm/Makefile
lib/DtTerm/TermView/Makefile
lib/DtTerm/util/Makefile
lib/DtTerm/TermPrim/Makefile

lib/DtMrm/Makefile

lib/csa/Makefile

lib/DtMmdb/Makefile

programs/Makefile

programs/backdrops/Makefile

programs/palettes/Makefile

programs/icons/Makefile

programs/dsdm/Makefile

programs/dthelp/Makefile
programs/dthelp/dthelpview/Makefile
programs/dthelp/dthelpgen/Makefile
programs/dthelp/dthelpprint/Makefile
programs/dthelp/parser/Makefile

programs/dtmail/Makefile
programs/dtmail/dtmail/Makefile
programs/dtmail/MotifApp/Makefile
programs/dtmail/dtmailpr/Makefile
programs/dtmail/libDtMail/Makefile
programs/dtmail/libDtMail/RFC/Makefile
programs/dtmail/libDtMail/Common/Makefile

programs/dtpad/Makefile

programs/dtfile/Makefile
programs/dtfile/dtcopy/Makefile

programs/dtwm/Makefile

programs/dtlogin/Makefile
programs/dtlogin/config/Makefile

programs/dtsession/Makefile
programs/dtsession/config/Makefile

programs/dthello/Makefile

programs/dtstyle/Makefile

programs/dtexec/Makefile

programs/dtdbcache/Makefile
programs/dtrandr/Makefile

programs/dticon/Makefile

programs/dtterm/Makefile

programs/dtcalc/Makefile

programs/dtaction/Makefile

programs/dtspcd/Makefile

programs/dtscreen/Makefile

programs/dtksh/Makefile
programs/dtksh/examples/Makefile

programs/dtcm/Makefile
programs/dtcm/libDtCmP/Makefile
programs/dtcm/server/Makefile
programs/dtcm/dtcm/Makefile

programs/dtsearchpath/Makefile
programs/dtsearchpath/libCliSrv/Makefile
programs/dtsearchpath/dtsp/Makefile
programs/dtsearchpath/dtappg/Makefile

programs/dtappbuilder/Makefile
programs/dtappbuilder/src/Makefile
programs/dtappbuilder/src/libAButil/Makefile
programs/dtappbuilder/src/libABobj/Makefile
programs/dtappbuilder/src/libABobjXm/Makefile
programs/dtappbuilder/src/libABil/Makefile
programs/dtappbuilder/src/abmf/Makefile
programs/dtappbuilder/src/ab/Makefile

programs/dtappintegrate/Makefile

programs/dtprintegrate/Makefile

programs/dtconfig/Makefile
programs/dtconfig/sun/Makefile

programs/dtcreate/Makefile

programs/dtprintinfo/Makefile

programs/fontaliases/Makefile

programs/dtdspmsg/Makefile

programs/ttsnoop/Makefile

programs/dtimsstart/Makefile

programs/dtpdm/Makefile

programs/dtsr/Makefile

programs/dtpdmd/Makefile

programs/dtinfo/Makefile
programs/dtinfo/tools/Makefile
programs/dtinfo/tools/misc/Makefile
programs/dtinfo/clients/Makefile
programs/dtinfo/clients/dtinfo_start/Makefile

programs/dtinfo/dtinfo/Makefile
programs/dtinfo/dtinfo/wwl/Makefile
programs/dtinfo/dtinfo/wwl/src/Makefile
programs/dtinfo/dtinfo/src/Makefile
programs/dtinfo/dtinfo/install/Makefile
programs/dtinfo/dtinfo/install/lib/Makefile
programs/dtinfo/dtinfo/install/lib/bitmaps/Makefile

programs/types/Makefile

programs/tttypes/Makefile

programs/util/Makefile
programs/util/dttypes/Makefile

programs/dtopen/Makefile

programs/dtdocbook/Makefile
programs/dtdocbook/dtdocbook2man
programs/dtdocbook/dtdocbook2sdl
programs/dtdocbook/doc_utils/Makefile
programs/dtdocbook/dtsr/Makefile
programs/dtdocbook/infolib/Makefile
programs/dtdocbook/instant/Makefile
programs/dtdocbook/locales/Makefile
programs/dtdocbook/locales/de_DE.UTF-8/Makefile
programs/dtdocbook/locales/el_GR.UTF-8/Makefile
programs/dtdocbook/locales/en_US.UTF-8/Makefile
programs/dtdocbook/locales/es_ES.UTF-8/Makefile
programs/dtdocbook/locales/fr_FR.UTF-8/Makefile
programs/dtdocbook/locales/it_IT.UTF-8/Makefile
programs/dtdocbook/locales/ja_JP.UTF-8/Makefile
programs/dtdocbook/sgml/Makefile
programs/dtdocbook/spec/Makefile
programs/dtdocbook/tcl/Makefile
programs/dtdocbook/tpt/Makefile

programs/localized/Makefile
programs/localized/util/Makefile
programs/localized/C/Makefile
programs/localized/C/app-defaults/Makefile
programs/localized/C/config/Makefile
programs/localized/C/backdrops/Makefile
programs/localized/C/types/Makefile
programs/localized/C/palettes/Makefile
programs/localized/C/msg/Makefile
programs/localized/C/appmanager/Makefile
programs/localized/de_DE.UTF-8/Makefile
programs/localized/de_DE.UTF-8/app-defaults/Makefile
programs/localized/de_DE.UTF-8/config/Makefile
programs/localized/de_DE.UTF-8/backdrops/Makefile
programs/localized/de_DE.UTF-8/types/Makefile
programs/localized/de_DE.UTF-8/palettes/Makefile
programs/localized/de_DE.UTF-8/msg/Makefile
programs/localized/de_DE.UTF-8/appmanager/Makefile
programs/localized/fr_FR.UTF-8/Makefile
programs/localized/fr_FR.UTF-8/app-defaults/Makefile
programs/localized/fr_FR.UTF-8/config/Makefile
programs/localized/fr_FR.UTF-8/backdrops/Makefile
programs/localized/fr_FR.UTF-8/types/Makefile
programs/localized/fr_FR.UTF-8/palettes/Makefile
programs/localized/fr_FR.UTF-8/msg/Makefile
programs/localized/fr_FR.UTF-8/appmanager/Makefile
programs/localized/it_IT.UTF-8/Makefile
programs/localized/it_IT.UTF-8/app-defaults/Makefile
programs/localized/it_IT.UTF-8/config/Makefile
programs/localized/it_IT.UTF-8/backdrops/Makefile
programs/localized/it_IT.UTF-8/types/Makefile
programs/localized/it_IT.UTF-8/palettes/Makefile
programs/localized/it_IT.UTF-8/msg/Makefile
programs/localized/it_IT.UTF-8/appmanager/Makefile
programs/localized/es_ES.UTF-8/Makefile
programs/localized/es_ES.UTF-8/app-defaults/Makefile
programs/localized/es_ES.UTF-8/config/Makefile
programs/localized/es_ES.UTF-8/backdrops/Makefile
programs/localized/es_ES.UTF-8/types/Makefile
programs/localized/es_ES.UTF-8/palettes/Makefile
programs/localized/es_ES.UTF-8/msg/Makefile
programs/localized/es_ES.UTF-8/appmanager/Makefile
programs/localized/ja_JP.UTF-8/Makefile
programs/localized/ja_JP.UTF-8/app-defaults/Makefile
programs/localized/ja_JP.UTF-8/config/Makefile
programs/localized/ja_JP.UTF-8/backdrops/Makefile
programs/localized/ja_JP.UTF-8/types/Makefile
programs/localized/ja_JP.UTF-8/palettes/Makefile
programs/localized/ja_JP.UTF-8/msg/Makefile
programs/localized/ja_JP.UTF-8/appmanager/Makefile
programs/localized/zh_CN.UTF-8/Makefile
programs/localized/zh_CN.UTF-8/app-defaults/Makefile
programs/localized/zh_CN.UTF-8/config/Makefile
programs/localized/zh_CN.UTF-8/backdrops/Makefile
programs/localized/zh_CN.UTF-8/types/Makefile
programs/localized/zh_CN.UTF-8/palettes/Makefile
programs/localized/zh_CN.UTF-8/msg/Makefile
programs/localized/zh_CN.UTF-8/appmanager/Makefile
programs/localized/zh_TW.UTF-8/Makefile
programs/localized/zh_TW.UTF-8/app-defaults/Makefile
programs/localized/zh_TW.UTF-8/config/Makefile
programs/localized/zh_TW.UTF-8/backdrops/Makefile
programs/localized/zh_TW.UTF-8/types/Makefile
programs/localized/zh_TW.UTF-8/palettes/Makefile
programs/localized/zh_TW.UTF-8/msg/Makefile
programs/localized/zh_TW.UTF-8/appmanager/Makefile

doc/Makefile

doc/en_US.UTF-8/help/Makefile
doc/en_US.UTF-8/help/common/Desktop.hf
doc/en_US.UTF-8/help/common/DesktopIntro.hf
doc/en_US.UTF-8/man/Makefile
doc/en_US.UTF-8/guides/Makefile
doc/en_US.UTF-8/m-guides/Makefile

doc/de_DE.UTF-8/guides/Makefile
doc/de_DE.UTF-8/help/Makefile
doc/de_DE.UTF-8/help/common/Desktop.hf
doc/de_DE.UTF-8/help/common/DesktopIntro.hf

doc/es_ES.UTF-8/guides/Makefile
doc/es_ES.UTF-8/help/Makefile
doc/es_ES.UTF-8/help/common/Desktop.hf
doc/es_ES.UTF-8/help/common/DesktopIntro.hf

doc/fr_FR.UTF-8/guides/Makefile
doc/fr_FR.UTF-8/help/Makefile
doc/fr_FR.UTF-8/help/common/Desktop.hf
doc/fr_FR.UTF-8/help/common/DesktopIntro.hf

doc/it_IT.UTF-8/guides/Makefile
doc/it_IT.UTF-8/help/Makefile
doc/it_IT.UTF-8/help/common/Desktop.hf
doc/it_IT.UTF-8/help/common/DesktopIntro.hf

doc/ja_JP.UTF-8/guides/Makefile
doc/ja_JP.UTF-8/help/Makefile
doc/ja_JP.UTF-8/help/common/Desktop.hf
doc/ja_JP.UTF-8/help/common/DesktopIntro.hf

include/Makefile

copyright
doc/common/help/HELPEnt.sgm
lib/tt/bin/ttauth/ttauth.man

contrib/desktopentry/cde.desktop
contrib/rc/linux/dtlogin
contrib/rc/freebsd/dtlogin
contrib/rc/netbsd/dtlogin
contrib/rc/openbsd/dtlogin
contrib/rc/openbsd/cmsd
contrib/xinetd/cmsd
contrib/xinetd/ttdbserver
contrib/desktop2dt/desktop2dt
])

dnl include/Dt/Dt.h carries the version and is included by most of the
dnl tree.  Write it only when its content changes, so that rerunning
dnl configure does not rebuild everything that includes it.  (The
dnl top-level Makefile has the same rule for when Dt.h.in changes.)
AC_CONFIG_COMMANDS([include/Dt/Dt.h], [
  cde_dt_h_tmp=include/Dt/Dt.h.tmp
  sed -e "s|@CDE_VERSION_MAJOR@|$cde_version_major|g" \
      -e "s|@CDE_VERSION_MINOR@|$cde_version_minor|g" \
      -e "s|@CDE_VERSION_MICRO@|$cde_version_micro|g" \
      -e "s|@CDE_VERSION_DEV@|$cde_version_dev|g" \
      "$cde_srcdir/include/Dt/Dt.h.in" > "$cde_dt_h_tmp" || exit 1
  if cmp -s "$cde_dt_h_tmp" include/Dt/Dt.h; then
    AC_MSG_NOTICE([include/Dt/Dt.h is unchanged])
    rm -f "$cde_dt_h_tmp"
  else
    mv -f "$cde_dt_h_tmp" include/Dt/Dt.h || exit 1
  fi
], [
  cde_srcdir="$srcdir"
  cde_version_major="$CDE_VERSION_MAJOR"
  cde_version_minor="$CDE_VERSION_MINOR"
  cde_version_micro="$CDE_VERSION_MICRO"
  cde_version_dev="$CDE_VERSION_DEV"
])

AC_OUTPUT
