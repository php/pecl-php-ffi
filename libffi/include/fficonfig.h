#define ZEND_ACCONFIG_H_NO_C_PROTOS
#ifdef PHP_WIN32
# define TARGET X86_WIN32
# include "config.w32.h"
# define SIZEOF_DOUBLE 8
# define SIZEOF_FLOAT 4
# define SIZEOF_INT 4
# define SIZEOF_LONG 4
# define SIZEOF_LONG_DOUBLE 12
# define SIZEOF_PTRDIFF_T 4
# define SIZEOF_SHORT 2
# define SIZEOF_SIZE_T 4
# define SIZEOF_SSIZE_T 4
# define SIZEOF_VOID_P 4
# define HAVE_MEMCPY 1
# define STDC_HEADERS 1
# define __i386__ 1
#elif HAVE_CONFIG_H
# include "config.h"
#else
# include "php_config.h"
#endif
