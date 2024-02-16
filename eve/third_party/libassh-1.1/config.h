/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Version of the shared library. */
#define CONFIG_ASSH_ABI_VERSION 001001001

/* Enable warning at compile time for use of functions which are not ABI
   stable. */
#define CONFIG_ASSH_ABI_WARN /**/

/* Allow use of x86 AES-NI instructions */
#define CONFIG_ASSH_AES_NI /**/

/* Enable storage of temporary buffers on stack. This is not secur on
   platforms with memory swapping. */
/* #undef CONFIG_ASSH_ALLOCA */

/* Specifies maximum length of hostname for user authentication. */
#define CONFIG_ASSH_AUTH_HOSTNAME_LEN 128

/* Specifies maximum length of password for user authentication. */
#define CONFIG_ASSH_AUTH_PASSWORD_LEN 32

/* Specifies maximum length of username for user authentication. */
#define CONFIG_ASSH_AUTH_USERNAME_LEN 32

/* Enable bignum engine when defined. */
#define CONFIG_ASSH_BIGNUM_BUILTIN 1

/* Bignum uses 16 bits words when defined. */
#define CONFIG_ASSH_BIGNUM_WORD 64

/* Enable MD5 hash */
#define CONFIG_ASSH_BUILTIN_MD5 /**/

/* Enable SHA1 hash */
#define CONFIG_ASSH_BUILTIN_SHA1 /**/

/* Enable SHA2 hash */
#define CONFIG_ASSH_BUILTIN_SHA2 /**/

/* Enable SHA3 hash */
#define CONFIG_ASSH_BUILTIN_SHA3 /**/

/* Enable support for the client side of the SSH protocol when defined. */
#define CONFIG_ASSH_CLIENT /**/

/* Enable support for the @em{host based} user authentication method on server
   side when defined. */
#define CONFIG_ASSH_CLIENT_AUTH_HOSTBASED /**/

/* Enable support for the @em{keyboard interactive} user authentication method
   on server side when defined. */
#define CONFIG_ASSH_CLIENT_AUTH_KEYBOARD /**/

/* Enable support for the @em{password support} user authentication method on
   server side when defined. */
#define CONFIG_ASSH_CLIENT_AUTH_PASSWORD /**/

/* Enable support for the @em{public key support} user authentication method
   on server side when defined. */
#define CONFIG_ASSH_CLIENT_AUTH_PUBLICKEY /**/

/* Send a public key lookup packet first before sending the signature during
   user authentication when defined. */
#define CONFIG_ASSH_CLIENT_AUTH_USE_PKOK /**/

/* Use crypt_r when defined. */
#define CONFIG_ASSH_CRYPT_R 1

/* Use getpwnam_r when defined. */
#define CONFIG_ASSH_GETPWNAM_R 1

/* Use getspnam_r when defined. */
#define CONFIG_ASSH_GETSPNAM_R 1

/* Specifies the maximum length of the remote software identification string.
   rfc4253 section 4.2 requires 255 bytes which is almost never seen in
   practice. Using a lower value on embedded targets will reduce the size of
   the @ref assh_session_s structure. */
#define CONFIG_ASSH_IDENT_SIZE 255

/* Enable support for SSH key creation when defined. */
#define CONFIG_ASSH_KEY_CREATE /**/

/* Enable support for SSH key validation when defined. */
#define CONFIG_ASSH_KEY_VALIDATE /**/

/* Use malloc_usable_size from C lbirary when defined. */
#define CONFIG_ASSH_MALLOC_USABLE_SIZE 1

/* Specifies the maximum size of the ssh transport packet. */
#define CONFIG_ASSH_MAX_PACKET_LEN 35000

/* Specifies the maximum size of the ssh packet payload. rfc4253 section 6.1
   requires 32768 bytes. Using a lower value on embedded targets will reduce
   the memory usage and limit resources exhaustion attacks. */
#define CONFIG_ASSH_MAX_PAYLOAD_LEN 32768

/* Specifies the maximum number of registered services. */
#define CONFIG_ASSH_MAX_SERVICES 4

/* Disallow performing multiple key exchanges before user authentication. */
#define CONFIG_ASSH_NO_REKEX_BEFORE_AUTH /**/

/* Specifies the path to OpenSSH key files for use by helpers. */
#define CONFIG_ASSH_OPENSSH_PREFIX "/etc/ssh/"

/* Enable the packet pool allocator when defined. */
#define CONFIG_ASSH_PACKET_POOL /**/

/* Specifies the maximum byte amount of spare packets in the pool. */
#define CONFIG_ASSH_PACKET_POOL_SIZE 1048576

/* Use setgroups when defined. */
#define CONFIG_ASSH_POSIX_SETGROUPS 1

/* Value passed to the CRYPTO_secure_malloc_init function. */
#define CONFIG_ASSH_SECMEM_SIZE 0x10000

/* Enable support for the server side of the SSH protocol when defined. */
#define CONFIG_ASSH_SERVER /**/

/* Enable support for the @em{host based} user authentication method on server
   side when defined. */
#define CONFIG_ASSH_SERVER_AUTH_HOSTBASED /**/

/* Enable support for the @em{keyboard interactive} user authentication method
   on server side when defined. */
#define CONFIG_ASSH_SERVER_AUTH_KEYBOARD /**/

/* Enable support for the @em{none} user authentication method on server side
   when defined. */
#define CONFIG_ASSH_SERVER_AUTH_NONE /**/

/* Enable support for the @em{password} user authentication method on server
   side when defined. */
#define CONFIG_ASSH_SERVER_AUTH_PASSWORD /**/

/* Enable support for the @em{public key} user authentication method on server
   side when defined. */
#define CONFIG_ASSH_SERVER_AUTH_PUBLICKEY /**/

/* Version of the source package. */
#define CONFIG_ASSH_SRC_VERSION 00010001

/* Use POSIX standard buffered streams */
#define CONFIG_ASSH_STDIO 1

/* Enable builtin cipher algorithms when defined. */
#define CONFIG_ASSH_USE_BUILTIN_CIPHERS /**/

/* Enable builtin key-exchange algorithms when defined. */
#define CONFIG_ASSH_USE_BUILTIN_KEX /**/

/* Enable builtin MAC algorithms when defined. */
#define CONFIG_ASSH_USE_BUILTIN_MACS /**/

/* Enable support for builtin PRNG. */
#define CONFIG_ASSH_USE_BUILTIN_PRNG /**/

/* Enable builtin signature algorithms when defined. */
#define CONFIG_ASSH_USE_BUILTIN_SIGN /**/

/* Enable support for unix @tt {/dev/u?random} random generator when defined.
   */
#define CONFIG_ASSH_USE_DEV_RANDOM /**/

/* Enable the Libgcrypt support when defined. */
/* #undef CONFIG_ASSH_USE_GCRYPT */

/* Enable Libgcrypt secur memory allocator when defined. */
/* #undef CONFIG_ASSH_USE_GCRYPT_ALLOC */

/* Enable Libgcrypt cipher algorithms when defined. */
/* #undef CONFIG_ASSH_USE_GCRYPT_CIPHERS */

/* Use Libgcrypt hash algorithms implementations when defined. */
/* #undef CONFIG_ASSH_USE_GCRYPT_HASH */

/* Enable Libgcrypt MAC algorithms when defined. */
/* #undef CONFIG_ASSH_USE_GCRYPT_MACS */

/* Enable Libgcrypt random number generator when defined. */
/* #undef CONFIG_ASSH_USE_GCRYPT_PRNG */

/* Enable support for the C library allocator which is not secur on some
   platforms. */
/* #undef CONFIG_ASSH_USE_LIBC_ALLOC */

/* Enable the openssl support when defined. */
#define CONFIG_ASSH_USE_OPENSSL /**/

/* Enable openssl secur memory allocator when defined. */
#define CONFIG_ASSH_USE_OPENSSL_ALLOC /**/

/* Enable openssl cipher algorithms when defined. */
#define CONFIG_ASSH_USE_OPENSSL_CIPHERS /**/

/* Use openssl hash algorithms implementations when defined. */
/* #undef CONFIG_ASSH_USE_OPENSSL_HASH */

/* Enable openssl MAC algorithms when defined. */
#define CONFIG_ASSH_USE_OPENSSL_MACS /**/

/* Enable openssl random number generator when defined. */
#define CONFIG_ASSH_USE_OPENSSL_PRNG /**/

/* Enable openssl signature algorithms when defined. */
#define CONFIG_ASSH_USE_OPENSSL_SIGN /**/

/* Enable the libsodium support when defined. */
#define CONFIG_ASSH_USE_SODIUM /**/

/* Enable libsodium key-exchange algorithms when defined. */
#define CONFIG_ASSH_USE_SODIUM_KEX /**/

/* Enable libsodium based prng when defined. */
#define CONFIG_ASSH_USE_SODIUM_PRNG /**/

/* Enable libsodium signature algorithms when defined. */
#define CONFIG_ASSH_USE_SODIUM_SIGN /**/

/* Enable the zlib library when defined. */
#define CONFIG_ASSH_USE_ZLIB /**/

/* Use valgrind headers for better checking when defined. */
/* #undef CONFIG_ASSH_VALGRIND */

/* Include error strings in the library */
#define CONFIG_ASSH_VERBOSE_ERROR /**/

/* Specifies the allocator storage type used for the zlib context. */
#define CONFIG_ASSH_ZLIB_ALLOC ASSH_ALLOC_INTERNAL

/* Define to 1 if you have the <crypt.h> header file. */
#define HAVE_CRYPT_H 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the <gcrypt.h> header file. */
/* #undef HAVE_GCRYPT_H */

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <minix/config.h> header file. */
/* #undef HAVE_MINIX_CONFIG_H */

/* Define to 1 if you have the <openssl/crypto.h> header file. */
#define HAVE_OPENSSL_CRYPTO_H 1

/* Define to 1 if you have the <openssl/opensslconf.h> header file. */
#define HAVE_OPENSSL_OPENSSLCONF_H 1

/* Use posix_openpt when defined. */
#define HAVE_POSIX_OPENPT 1

/* Define to 1 if you have the <sodium/crypto_scalarmult_curve25519.h> header
   file. */
#define HAVE_SODIUM_CRYPTO_SCALARMULT_CURVE25519_H 1

/* Define to 1 if you have the <sodium/crypto_sign_ed25519.h> header file. */
#define HAVE_SODIUM_CRYPTO_SIGN_ED25519_H 1

/* Define to 1 if you have the <sodium/randombytes.h> header file. */
#define HAVE_SODIUM_RANDOMBYTES_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdio.h> header file. */
#define HAVE_STDIO_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Use GNU TIOCGWINSZ ioctl */
#define HAVE_TIOCGWINSZ 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the <valgrind/memcheck.h> header file. */
/* #undef HAVE_VALGRIND_MEMCHECK_H */

/* Define to 1 if you have the <wchar.h> header file. */
#define HAVE_WCHAR_H 1

/* Define to 1 if you have the <zlib.h> header file. */
#define HAVE_ZLIB_H 1

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#define LT_OBJDIR ".libs/"

/* Enable assert() */
/* #undef NDEBUG */

/* Name of package */
#define PACKAGE "libassh"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME "libassh"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "libassh 1.1"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "libassh"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "1.1"

/* The size of `long', as computed by sizeof. */
#define SIZEOF_LONG 8

/* Define to 1 if all of the C90 standard headers exist (not just the ones
   required in a freestanding environment). This macro is provided for
   backward compatibility; new code need not use it. */
#define STDC_HEADERS 1

/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
# define _ALL_SOURCE 1
#endif
/* Enable general extensions on macOS.  */
#ifndef _DARWIN_C_SOURCE
# define _DARWIN_C_SOURCE 1
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif
/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE 1
#endif
/* Enable X/Open compliant socket functions that do not require linking
   with -lxnet on HP-UX 11.11.  */
#ifndef _HPUX_ALT_XOPEN_SOCKET_API
# define _HPUX_ALT_XOPEN_SOCKET_API 1
#endif
/* Identify the host operating system as Minix.
   This macro does not affect the system headers' behavior.
   A future release of Autoconf may stop defining this macro.  */
#ifndef _MINIX
/* # undef _MINIX */
#endif
/* Enable general extensions on NetBSD.
   Enable NetBSD compatibility extensions on Minix.  */
#ifndef _NETBSD_SOURCE
# define _NETBSD_SOURCE 1
#endif
/* Enable OpenBSD compatibility extensions on NetBSD.
   Oddly enough, this does nothing on OpenBSD.  */
#ifndef _OPENBSD_SOURCE
# define _OPENBSD_SOURCE 1
#endif
/* Define to 1 if needed for POSIX-compatible behavior.  */
#ifndef _POSIX_SOURCE
/* # undef _POSIX_SOURCE */
#endif
/* Define to 2 if needed for POSIX-compatible behavior.  */
#ifndef _POSIX_1_SOURCE
/* # undef _POSIX_1_SOURCE */
#endif
/* Enable POSIX-compatible threading on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
# define _POSIX_PTHREAD_SEMANTICS 1
#endif
/* Enable extensions specified by ISO/IEC TS 18661-5:2014.  */
#ifndef __STDC_WANT_IEC_60559_ATTRIBS_EXT__
# define __STDC_WANT_IEC_60559_ATTRIBS_EXT__ 1
#endif
/* Enable extensions specified by ISO/IEC TS 18661-1:2014.  */
#ifndef __STDC_WANT_IEC_60559_BFP_EXT__
# define __STDC_WANT_IEC_60559_BFP_EXT__ 1
#endif
/* Enable extensions specified by ISO/IEC TS 18661-2:2015.  */
#ifndef __STDC_WANT_IEC_60559_DFP_EXT__
# define __STDC_WANT_IEC_60559_DFP_EXT__ 1
#endif
/* Enable extensions specified by ISO/IEC TS 18661-4:2015.  */
#ifndef __STDC_WANT_IEC_60559_FUNCS_EXT__
# define __STDC_WANT_IEC_60559_FUNCS_EXT__ 1
#endif
/* Enable extensions specified by ISO/IEC TS 18661-3:2015.  */
#ifndef __STDC_WANT_IEC_60559_TYPES_EXT__
# define __STDC_WANT_IEC_60559_TYPES_EXT__ 1
#endif
/* Enable extensions specified by ISO/IEC TR 24731-2:2010.  */
#ifndef __STDC_WANT_LIB_EXT2__
# define __STDC_WANT_LIB_EXT2__ 1
#endif
/* Enable extensions specified by ISO/IEC 24747:2009.  */
#ifndef __STDC_WANT_MATH_SPEC_FUNCS__
# define __STDC_WANT_MATH_SPEC_FUNCS__ 1
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
# define _TANDEM_SOURCE 1
#endif
/* Enable X/Open extensions.  Define to 500 only if necessary
   to make mbstate_t available.  */
#ifndef _XOPEN_SOURCE
/* # undef _XOPEN_SOURCE */
#endif


/* Version number of package */
#define VERSION "1.1"

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif
