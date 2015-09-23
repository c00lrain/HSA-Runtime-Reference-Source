////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2014 ADVANCED MICRO DEVICES, INC.
//
// AMD is granting you permission to use this software and documentation (if
// any) (collectively, the "Materials") pursuant to the terms and conditions of
// the Software License Agreement included with the Materials. If you do not
// have a copy of the Software License Agreement, contact your AMD
// representative for a copy.
//
// You agree that you will not reverse engineer or decompile the Materials, in
// whole or in part, except as allowed by applicable law.
//
// WARRANTY DISCLAIMER: THE SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND. AMD DISCLAIMS ALL WARRANTIES, EXPRESS, IMPLIED, OR STATUTORY,
// INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE, NON - INFRINGEMENT, THAT THE
// SOFTWARE WILL RUN UNINTERRUPTED OR ERROR - FREE OR WARRANTIES ARISING FROM
// CUSTOM OF TRADE OR COURSE OF USAGE. THE ENTIRE RISK ASSOCIATED WITH THE USE
// OF THE SOFTWARE IS ASSUMED BY YOU. Some jurisdictions do not allow the
// exclusion of implied warranties, so the above exclusion may not apply to You.
//
// LIMITATION OF LIABILITY AND INDEMNIFICATION: AMD AND ITS LICENSORS WILL NOT,
// UNDER ANY CIRCUMSTANCES BE LIABLE TO YOU FOR ANY PUNITIVE, DIRECT,
// INCIDENTAL, INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES ARISING FROM USE OF
// THE SOFTWARE OR THIS AGREEMENT EVEN IF AMD AND ITS LICENSORS HAVE BEEN
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGES. In no event shall AMD's total
// liability to You for all damages, losses, and causes of action (whether in
// contract, tort (including negligence) or otherwise) exceed the amount of $100
// USD. You agree to defend, indemnify and hold harmless AMD and its licensors,
// and any of their directors, officers, employees, affiliates or agents from
// and against any and all loss, damage, liability and other expenses (including
// reasonable attorneys' fees), resulting from Your use of the Software or
// violation of the terms and conditions of this Agreement.
//
// U.S.GOVERNMENT RESTRICTED RIGHTS: The Materials are provided with
// "RESTRICTED RIGHTS." Use, duplication, or disclosure by the Government is
// subject to the restrictions as set forth in FAR 52.227 - 14 and DFAR252.227 -
// 7013, et seq., or its successor. Use of the Materials by the Government
// constitutes acknowledgement of AMD's proprietary rights in them.
//
// EXPORT RESTRICTIONS: The Materials may be subject to export restrictions as
//                      stated in the Software License Agreement.
//
////////////////////////////////////////////////////////////////////////////////

// The following set of header files provides definitions for AMD GPU
// Architecture:
//   - amd_hsa_common.h
//   - amd_hsa_elf.h
//   - amd_hsa_kernel_code.h
//   - amd_hsa_queue.h
//   - amd_hsa_signal.h
//
// Refer to "HSA Application Binary Interface: AMD GPU Architecture" for more
// information.

#ifndef AMD_HSA_COMMON_H
#define AMD_HSA_COMMON_H

#include <stddef.h>
#include <stdint.h>

// Descriptive version of the HSA Application Binary Interface.
#define AMD_HSA_ABI_VERSION "AMD GPU Architecture v0.35 (June 25, 2015)"

// Alignment attribute that specifies a minimum alignment (in bytes) for
// variables of the specified type.
#if defined(__GNUC__)
#  define __ALIGNED__(x) __attribute__((aligned(x)))
#elif defined(_MSC_VER)
#  define __ALIGNED__(x) __declspec(align(x))
#elif defined(RC_INVOKED)
#  define __ALIGNED__(x)
#else
#  error
#endif

// Creates enumeration entries for packed types. Enumeration entries include
// bit shift amount, bit width, and bit mask.
#define AMD_HSA_BITS_CREATE_ENUM_ENTRIES(name, shift, width)                   \
  name ## _SHIFT = (shift),                                                    \
  name ## _WIDTH = (width),                                                    \
  name = (((1 << (width)) - 1) << (shift))                                     \

// Gets bits for specified mask from specified src packed instance.
#define AMD_HSA_BITS_GET(src, mask)                                            \
  ((src & mask) >> mask ## _SHIFT)                                             \

// Sets val bits for specified mask in specified dst packed instance.
#define AMD_HSA_BITS_SET(dst, mask, val)                                       \
  dst &= (~(1 << mask ## _SHIFT) & ~mask);                                     \
  dst |= (((val) << mask ## _SHIFT) & mask)                                    \

#endif // AMD_HSA_COMMON_H
