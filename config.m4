dnl $Id$
dnl config.m4 for extension ffi

PHP_ARG_ENABLE(ffi, for ffi support,
[  --enable-ffi             Include ffi support])

if test "$PHP_FFI" != "no"; then
  PHP_LIBFFI_VERSION="2.00-beta"
  PHP_DEFINE(PHP_LIBFFI_VERSION)
	TARGETDIR="unknown"
	case "$host" in
		mips-sgi-irix5.* | mips-sgi-irix6.*) TARGET=MIPS; TARGETDIR=mips;;
		i*86-*-linux*) TARGET=X86; TARGETDIR=x86;;
		i*86-*-sco3.2v5*) TARGET=X86; TARGETDIR=x86;;
		i*86-*-solaris*) TARGET=X86; TARGETDIR=x86;;
		i*86-*-beos*) TARGET=X86; TARGETDIR=x86;;
		i*86-*-freebsd*) TARGET=X86; TARGETDIR=x86;;
		i*86-*-netbsdelf*) TARGET=X86; TARGETDIR=x86;;
		i*86-*-win32*) TARGET=X86_WIN32; TARGETDIR=x86;;
		i*86-*-cygwin*) TARGET=X86_WIN32; TARGETDIR=x86;;
		i*86-*-mingw*) TARGET=X86_WIN32; TARGETDIR=x86;;
		sparc-sun-4*) TARGET=SPARC; TARGETDIR=sparc;;
		sparc*-sun-*) TARGET=SPARC; TARGETDIR=sparc;;
		sparc-*-linux* | sparc-*-netbsdelf*) TARGET=SPARC; TARGETDIR=sparc;;
		sparc64-*-linux* | sparc64-*-netbsd*) TARGET=SPARC; TARGETDIR=sparc;;
		alpha*-*-linux* | alpha*-*-osf* | alpha*-*-freebsd* | alpha*-*-netbsd*) TARGET=ALPHA; TARGETDIR=alpha;;
		ia64*-*-*) TARGET=IA64; TARGETDIR=ia64;;
		m68k-*-linux*) TARGET=M68K; TARGETDIR=m68k;;
		mips64*-*);;
		mips*-*-linux*) TARGET=MIPS_LINUX; TARGETDIR=mips;;
		powerpc-*-linux* | powerpc-*-sysv*) TARGET=POWERPC; TARGETDIR=powerpc;;
		powerpc-*-beos*) TARGET=POWERPC; TARGETDIR=powerpc;;
		powerpc-*-darwin*) TARGET=POWERPC_DARWIN; TARGETDIR=powerpc;;
		powerpc-*-aix*) TARGET=POWERPC_AIX; TARGETDIR=powerpc;;
		rs6000-*-aix*) TARGET=POWERPC_AIX; TARGETDIR=powerpc;;
		arm*-*-linux-*) TARGET=ARM; TARGETDIR=arm;;
		s390-*-linux-*) TARGET=S390; TARGETDIR=s390;;
		s390x-*-linux-*) TARGET=S390; TARGETDIR=s390;;
		x86_64-*-linux*) TARGET=X86_64; TARGETDIR=x86;;
		sh-*-linux* | sh[[34]]*-*-linux*) TARGET=SH; TARGETDIR=sh;;
	esac

	if test $TARGETDIR = unknown; then
		AC_ERROR("libffi has not been ported to $host.")
	fi

	if test x$TARGET = xMIPS_LINUX; then
		TARGET=MIPS
	fi

  case "$TARGET" in
    MIPS)
      if "$ac_cv_prog_gcc" = "yes"; then
        ffi_sources="libffi/src/mips/ffi.c libffi/src/mips/o32.S libffi/src/mips/n32.S"
      else
        ffi_sources="libffi/src/mips/ffi.c libffi/src/mips/o32.s libffi/src/mips/n32.s"
      fi
      ;;
    MIPS_LINUX) ffi_sources="libffi/src/mips/ffi.c libffi/src/mips/o32.S";; 
    X86) ffi_sources="libffi/src/x86/ffi.c libffi/src/x86/sysv.S";;
    X86_WIN32) ffi_sources="libffi/src/x86/ffi.c libffi/src/x86/win32.S";;
    SPARC) ffi_sources="libffi/src/sparc/ffi.c libffi/src/sparc/v8.S libffi/src/sparc/v9.S";;
    ALPHA) ffi_sources="libffi/src/alpha/ffi.c libffi/src/alpha/osf.S";;
    IA64) ffi_sources="libffi/src/ia64/ffi.c libffi/src/ia64/unix.S";;
    M68K) ffi_sources="libffi/src/m68k/ffi.c libffi/src/m68k/sysv.S";;
    POWERPC) ffi_sources="libffi/src/powerpc/ffi.c libffi/src/powerpc/sysv.S libffi/src/powerpc/ppc_closure.S";;
    POWERPC_AIX) ffi_sources="libffi/src/powerpc/ffi_darwin.c libffi/src/powerpc/aix.S libffi/src/powerpc/aix_closures.S";;
    POWERPC_DARWIN) ffi_sources="libffi/src/powerpc/ffi_darwin.c libffi/src/powerpc/darwin.S libffi/src/powerpc/darwin_closure.S";;
    ARM)  ffi_sources="libffi/src/arm/sysv.S libffi/src/arm/ffi.c";;
    S390)  ffi_sources="libffi/src/s390/sysv.S libffi/src/s390/ffi.c";;
    X86_64) ffi_sources="libffi/src/x86/ffi64.c libffi/src/x86/unix64.S libffi/src/x86/ffi.c libffi/src/x86/sysv.S";;
    SH)  ffi_sources="libffi/src/sh/sysv.S libffi/src/sh/ffi.c";;

  esac 
  
	AC_CHECK_SIZEOF(short,2)
	AC_CHECK_SIZEOF(int,4)
	AC_CHECK_SIZEOF(long,4)
	AC_CHECK_SIZEOF(long long,8)
	AC_CHECK_SIZEOF(float,4)
	AC_CHECK_SIZEOF(double,8)
	AC_CHECK_SIZEOF(long double)
	
	AC_CHECK_SIZEOF(void *,4)

	if test x$TARGET = xSPARC; then
    AC_CACHE_CHECK([assembler and linker support unaligned pc related relocs],
		libffi_cv_as_sparc_ua_pcrel, [
			save_CFLAGS="$CFLAGS"
			save_LDFLAGS="$LDFLAGS"
			CFLAGS="$CFLAGS -fpic"
			LDFLAGS="$LDFLAGS -shared"
			AC_TRY_LINK([asm (".text; foo: nop; .data; .align 4; .byte 0; .uaword %r_disp32(foo); .text");],,
		    [libffi_cv_as_sparc_ua_pcrel=yes],
		    [libffi_cv_as_sparc_ua_pcrel=no])
			CFLAGS="$save_CFLAGS"
			LDFLAGS="$save_LDFLAGS"])
			if test "x$libffi_cv_as_sparc_ua_pcrel" = xyes; then
				AC_DEFINE(HAVE_AS_SPARC_UA_PCREL, 1,
					[Define if your assembler and linker support unaligned PC relative relocs.])
			fi

			AC_CACHE_CHECK([assembler .register pseudo-op support],
       libffi_cv_as_register_pseudo_op, [
       libffi_cv_as_register_pseudo_op=unknown
       # Check if we have .register
       AC_TRY_COMPILE([asm (".register %g2, #scratch");],,
                       [libffi_cv_as_register_pseudo_op=yes],
                       [libffi_cv_as_register_pseudo_op=no])
			])
			if test "x$libffi_cv_as_register_pseudo_op" = xyes; then
				AC_DEFINE(HAVE_AS_REGISTER_PSEUDO_OP, 1,
               [Define if your assembler supports .register.])
			fi
	fi


  PHP_NEW_EXTENSION(ffi, php_ffi.c ffi_struct.c ffi_library.c ffi_parser_util.c ffi_parser.c ffi_int64.c $ffi_sources libffi/src/debug.c libffi/src/prep_cif.c libffi/src/types.c libffi/src/raw_api.c libffi/src/java_raw_api.c, $ext_shared,,-I@ext_srcdir@/libffi/include)
  PHP_ADD_BUILD_DIR($ext_builddir/libffi/src/$TARGETDIR)
  PHP_ADD_BUILD_DIR($ext_builddir/libffi)
  PHP_ADD_BUILD_DIR($ext_builddir/libffi/src)
	
	sed -e "s/@TARGET@/$TARGET/" -e "s/@VERSION@/$PHP_LIBFFI_VERSION/"  < $ext_srcdir/libffi/include/ffi.h.in > $ext_srcdir/libffi/include/ffi.h

	PHP_ADD_MAKEFILE_FRAGMENT
 
fi
