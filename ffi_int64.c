/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2004 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.0 of the PHP license,       |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_0.txt.                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Wez Furlong  <wez@php.net>                                   |
   +----------------------------------------------------------------------+
 */

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_ffi.h"
#include "php_ffi_internal.h"

SINT64 php_ffi_strto_int64(const char *nptr, char **endptr, int base, int is_unsigned)
{
	int negative;
	register UINT64 cutoff;
	register unsigned int cutlim;
	register UINT64 i;
	register const char *s;
	register unsigned char c;
	const char *save;
	int overflow;

	if (base < 0 || base == 1 || base > 36)
		base = 10;

	s = nptr;

	/* Skip white space.  */
	while (isspace (*s))
		++s;

	if (*s == '\0') {
		goto noconv;
	}
	/* Check for a sign.  */
	if (*s == '-') {
		negative = 1;
		++s;
	} else if (*s == '+') {
		negative = 0;
		++s;
	} else {
		negative = 0;
	}

	if (base == 16 && s[0] == '0' && toupper (s[1]) == 'X') {
		s += 2;
	}

	/* If BASE is zero, figure it out ourselves.  */
	if (base == 0) {
		if (*s == '0') {
			if (toupper (s[1]) == 'X') {
				s += 2;
				base = 16;
			} else {
				base = 8;
			}
		} else {
			base = 10;
		}
	}

	/* Save the pointer so we can check later if anything happened.  */
	save = s;
	cutoff = UPHP_FFI_SINT64_MAX / (unsigned long int) base;
	cutlim = (unsigned int) (UPHP_FFI_SINT64_MAX % (unsigned long int) base);

	overflow = 0;
	i = 0;
	for (c = *s; c != '\0'; c = *++s) {
		if (isdigit(c)) {
			c -= '0';
		} else if (isalpha(c)) {
			c = toupper(c) - 'A' + 10;
		} else {
			break;
		}

		if (c >= base) {
			break;
		}

		/* Check for overflow.  */
		if (i > cutoff || (i == cutoff && c > cutlim)) {
			overflow = 1;
		} else {
			i *= (UINT64) base;
			i += c;
		}
	}

	/* Check if anything actually happened.  */
	if (s == save) {
		goto noconv;
	}

	/* Store in ENDPTR the address of one character
	   past the last character we converted.  */
	if (endptr != NULL) {
		*endptr = (char *) s;
	}

	if (is_unsigned) {
		/* Check for a value that is within the range of
		   `unsigned long int', but outside the range of `long int'.  */
		if (negative) {
			if (i  > (UINT64) PHP_FFI_SINT64_MIN) {
				overflow = 1;
			}
		} else if (i > (UINT64) PHP_FFI_SINT64_MAX) {
			overflow = 1;
		}
	}

	if (overflow) {
		if (is_unsigned) {
			return UPHP_FFI_SINT64_MAX;
		} else {
			return negative ? PHP_FFI_SINT64_MIN : PHP_FFI_SINT64_MAX;
		}
	}

	/* Return the result of the appropriate sign.  */
	return (negative ? -((SINT64) i) : (SINT64) i);

noconv:
	/* There was no number to convert.  */
	if (endptr != NULL) {
		*endptr = (char *) nptr;
	}
	return 0L;
}

static char _dig_vec[] =
  "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";


char *php_ffi_int64_tostr(SINT64 val, char *dst, int radix)
{
	char buffer[65];
	register char *p;
	long long_val;

	if (radix < 0) {
		if (val < 0) {
			*dst++ = '-';
			val = -val;
		}
	}

	if (val == 0) {
		*dst++='0';
		*dst='\0';
		return dst;
	}
	p = &buffer[sizeof(buffer)-1];
	*p = '\0';

	while ((UINT64) val > (UINT64) LONG_MAX) {
		UINT64 quo=(UINT64) val/(unsigned int) 10;
		uint rem= (unsigned int) (val- quo* (unsigned int) 10);
		*--p = _dig_vec[rem];
		val= quo;
	}
	long_val= (long) val;
	while (long_val != 0) {
		long quo= long_val/10;
		*--p = _dig_vec[(unsigned char) (long_val - quo*10)];
		long_val= quo;
	}
	while ((*dst++ = *p++) != 0) 
		;
	return dst-1;
}

