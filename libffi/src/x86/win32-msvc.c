/* -----------------------------------------------------------------------
   win32.S - Copyright (c) 1996, 1998, 2001, 2002  Red Hat, Inc.
	     Copyright (c) 2001  John Beniton
	     Copyright (c) 2002  Ranjit Mathew
	
	Ported to MSVC Intel syntax by Wez Furlong <wez@php.net>		
 
   X86 Foreign Function Interface
 
   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   ``Software''), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:
 
   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.
 
   THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL CYGNUS SOLUTIONS BE LIABLE FOR ANY CLAIM, DAMAGES OR
   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
   OTHER DEALINGS IN THE SOFTWARE.
   ----------------------------------------------------------------------- */

#include "ffi.h"

void __declspec(naked) ffi_call_STDCALL()
{
	__asm {
			push   ebp
			mov    ebp,esp
			/* Make room for all of the new args. */
			mov    ecx,DWORD PTR [ebp+16]
			sub    esp,ecx
			mov    eax,esp
			/* Place all of the ffi_prep_args in position */
			push   DWORD PTR [ebp+12]
			push   eax
			/* call ffi_prep_args() */
			call   DWORD PTR [ebp+8]
			/* Return stack to previous state and call the function */
			add    esp,0x8

			/* FIXME: Align the stack to a 128-bit boundary to avoid
			   potential performance hits. */

			call   DWORD PTR [ebp+28]
			/* stdcall functions pop arguments off the stack themselves
			   Load ecx with the return type code */

			mov    ecx,DWORD PTR [ebp+20]
			/* If the return value pointer is NULL, assume no return value. */
			cmp    DWORD PTR [ebp+24],0x0
			jne    sc_retint

			/* Even if there is no space for the return value, we are
			   obliged to handle floating-point values. */

			cmp    ecx, FFI_TYPE_FLOAT
			jne    sc_epilogue
			fstp   st(0)
			jmp    sc_epilogue

sc_retint:
			cmp    ecx, FFI_TYPE_INT
			jne    sc_retfloat
			/* Load ecx with the pointer to storage for the return value */
			mov    ecx,DWORD PTR [ebp+24]
			mov    DWORD PTR [ecx],eax
			jmp    sc_epilogue

sc_retfloat:
			cmp    ecx,FFI_TYPE_FLOAT
			jne    sc_retdouble
			/* Load ecx with the pointer to storage for the return value */
			mov    ecx,DWORD PTR [ebp+24]
			fstp   DWORD PTR [ecx]
			jmp    sc_epilogue

sc_retdouble:
			cmp    ecx,FFI_TYPE_DOUBLE
			jne    sc_retlongdouble
			/* Load ecx with the pointer to storage for the return value */
			mov    ecx,DWORD PTR [ebp+24]
			fstp   QWORD PTR [ecx]
			jmp    sc_epilogue

sc_retlongdouble:
			cmp    ecx,FFI_TYPE_LONGDOUBLE
			jne    sc_retint64
			mov    ecx,DWORD PTR [ebp+24]
			/* Load ecx with the pointer to storage for the return value
			 * WARNING: this was XWORD; if you have problems with
			 * long double return values (should be very rare!) then
			 * you need to figure this out */
			fstp   TBYTE PTR [ecx]
			jmp    sc_epilogue

sc_retint64:
			cmp    ecx,FFI_TYPE_SINT64
			jne    sc_epilogue
			/* Load ecx with the pointer to storage for the return value */
			mov    ecx,DWORD PTR [ebp+24]
			mov    DWORD PTR [ecx],eax
			mov    DWORD PTR [ecx+4],edx

sc_epilogue:
			mov    esp,ebp
			pop    ebp
			ret    
	}
}

