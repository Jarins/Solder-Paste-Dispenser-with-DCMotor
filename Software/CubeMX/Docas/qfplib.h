// Copyright 2015 Mark Owen
// http://www.quinapalus.com
// E-mail: qfp@quinapalus.com
//
// Thanks to Bill Westfield
//
// This file is free software: you can redistribute it and/or modify
// it under the terms of version 2 of the GNU General Public License
// as published by the Free Software Foundation.
//
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this file.  If not, see <http://www.gnu.org/licenses/> or
// write to the Free Software Foundation, Inc., 51 Franklin Street,
// Fifth Floor, Boston, MA  02110-1301, USA.

#ifndef _QFPLIB_H_
#define _QFPLIB_H_

#ifdef __cplusplus
  extern "C" {
#endif

#define M_E				2.7182818284590452354
#define M_LOG2E			1.4426950408889634074
#define M_LOG10E		0.43429448190325182765
#define M_LN2			_M_LN2
#define M_LN10			2.30258509299404568402
#define M_PI			3.14159265358979323846
#define M_PI_2			1.57079632679489661923
#define M_PI_4			0.78539816339744830962
#define M_1_PI			0.31830988618379067154
#define M_2_PI			0.63661977236758134308
#define M_2_SQRTPI		1.12837916709551257390
#define M_SQRT2			1.41421356237309504880
#define M_SQRT1_2		0.70710678118654752440
#define M_TWOPI         (M_PI * 2.0)
#define M_3PI_4			2.3561944901923448370E0
#define M_SQRTPI        1.77245385090551602792981
#define M_LN2LO         1.9082149292705877000E-10
#define M_LN2HI         6.9314718036912381649E-1
#define M_SQRT3			1.73205080756887719000
#define M_IVLN10        0.43429448190325182765		 /* 1 / log(10) */
#define M_LOG2_E        _M_LN2
#define M_INVLN2        1.4426950408889633870E0 	 /* 1 / log(2) */

extern          float qfp_fadd(float x,float y);
extern          float qfp_fsub(float x,float y);
extern          float qfp_fmul(float x,float y);
extern          float qfp_fdiv(float x,float y);
extern          float qfp_fdiv_fast(float x,float y);
extern          int   qfp_float2int(float x);
extern          int   qfp_float2fix(float x,int y);
extern unsigned int   qfp_float2uint(float x);
extern unsigned int   qfp_float2ufix(float x,int y);
extern          float qfp_int2float(int x);
extern          float qfp_fix2float(int x,int y);
extern          float qfp_uint2float(unsigned int x);
extern          float qfp_ufix2float(unsigned int x,int y);
extern          int   qfp_fcmp(float x,float y);
extern          float qfp_fcos(float x);
extern          float qfp_fsin(float x);
extern          float qfp_ftan(float x);
extern          float qfp_fatan2(float y,float x);
extern          float qfp_fexp(float x);
extern          float qfp_fln(float x);
extern          float qfp_fsqrt(float x);
extern          float qfp_fsqrt_fast(float x);


#ifdef __cplusplus
  } // extern "C"
#endif
#endif
