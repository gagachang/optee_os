/* SPDX-License-Identifier: BSD-3-Clause */
/*
   Copyright (c) 2011, 2012 ARM Ltd
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   3. The name of the company may not be used to endorse or promote
      products derived from this software without specific prior written
      permission.

   THIS SOFTWARE IS PROVIDED BY ARM LTD ``AS IS'' AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL ARM LTD BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <asm.S>

#define GPR_LAYOUT			\
	REG_PAIR (x19, x20,  0);	\
	REG_PAIR (x21, x22, 16);	\
	REG_PAIR (x23, x24, 32);	\
	REG_PAIR (x25, x26, 48);	\
	REG_PAIR (x27, x28, 64);	\
	REG_PAIR (x29, x30, 80);	\
	REG_ONE (x16,      96)

#define FPR_LAYOUT			\
	REG_PAIR ( d8,  d9, 112);	\
	REG_PAIR (d10, d11, 128);	\
	REG_PAIR (d12, d13, 144);	\
	REG_PAIR (d14, d15, 160);

// int setjmp (jmp_buf)
	.global	setjmp
	.type	setjmp, %function
setjmp:
BTI(	bti	c)
	mov	x16, sp
#define REG_PAIR(REG1, REG2, OFFS)	stp REG1, REG2, [x0, OFFS]
#define REG_ONE(REG1, OFFS)		str REG1, [x0, OFFS]
	GPR_LAYOUT
	FPR_LAYOUT
#ifdef CFG_FTRACE_SUPPORT
	stp	x29, x30, [sp, #-16]!
	mov	x29, sp
	add	x0, x0, #104
	bl	ftrace_setjmp
	ldp	x29, x30, [sp], #16
#endif
#undef REG_PAIR
#undef REG_ONE
	mov	w0, #0
	ret
	.size	setjmp, .-setjmp

// void longjmp (jmp_buf, int) __attribute__ ((noreturn))
	.global	longjmp
	.type	longjmp, %function
longjmp:
BTI(	bti	c)
#define REG_PAIR(REG1, REG2, OFFS)	ldp REG1, REG2, [x0, OFFS]
#define REG_ONE(REG1, OFFS)		ldr REG1, [x0, OFFS]
#ifdef CFG_FTRACE_SUPPORT
	stp	x0, x1, [sp, #-16]!
	stp	x29, x30, [sp, #-16]!
	mov	x29, sp
	add	x0, x0, #104
	bl	ftrace_longjmp
	ldp	x29, x30, [sp], #16
	ldp	x0, x1, [sp], #16
#endif
#if defined(__KERNEL__) && defined(CFG_CORE_SANITIZE_KADDRESS) && \
    !defined(CFG_DYN_CONFIG)
	stp	x0, x1, [sp, #-16]!
	ldr	x0, [x0, 96]
	bl	asan_handle_longjmp
	ldp	x0, x1, [sp], #16
#endif
	GPR_LAYOUT
	FPR_LAYOUT
#undef REG_PAIR
#undef REG_ONE
	mov	sp, x16
	cmp	w1, #0
	cinc	w0, w1, eq
/*
 * clang has a bug and doesn't insert bti after setjmp
 * causing BTI ecxception. Remove this when the bug is fixed.
 * https://bugs.llvm.org/show_bug.cgi?id=49544
 */
#if defined(__clang__) && defined(CFG_TA_BTI)
	ret
#else
	// use br not ret, as ret is guaranteed to mispredict
	br	x30
#endif
	.size	longjmp, .-longjmp

BTI(emit_aarch64_feature_1_and     GNU_PROPERTY_AARCH64_FEATURE_1_BTI)
