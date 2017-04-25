#ifndef _IA_TYPEDEF_H_
#define _IA_TYPEDEF_H_

#ifdef  __cplusplus
extern "C"{
#endif
#ifdef ARM_CorTexA8
#include <stdint.h>
#endif

typedef void            V;
#ifdef ARM_CorTexA8
typedef int8_t          S8 ;
#else
typedef char            S8 ;
#endif
typedef unsigned char   U8 ;
typedef short           S16;
typedef unsigned short  U16;
typedef int             S32;
typedef unsigned int    U32;
typedef long long       S64;
typedef unsigned long long       U64;
typedef float           FL;
typedef double          DB;

typedef void *          pV;
#ifdef ARM_CorTexA8
typedef int8_t *        pS8 ;
#else
typedef char *          pS8 ;
#endif
typedef unsigned char * pU8 ;
typedef short *         pS16;
typedef unsigned short* pU16;
typedef int *           pS32;
typedef unsigned int *  pU32;
typedef long long *      pS64;
typedef float *         pFL;
typedef double *        pDB;
typedef int             BOOL;

#ifndef TI_DSP
typedef unsigned short Bool;
#endif


#define WIDTHBYTES(bits)    ((((bits) + 31)>>5)<<2)
#define BYTES_ALIGN64(bits)    ((((bits) + 63) >>6)<<6)
#define BYTES_ALIGN32(bits)    ((((bits) + 31) >>5)<<5)
#define BYTES_ALIGN16(bits)    ((((bits) + 15) >>4)<<4)
#define BYTES_ALIGN128(bits)    ((((bits) + 127) >>7)<<7)


#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef INOUT
#define INOUT
#endif

#ifndef TRUE
#define TRUE            (1)
#define FALSE           (0)
#endif

#define IA_TRUE        ( 1)
#define IA_FALSE       ( 0)

#ifndef  IA_OK
#define IA_OK          ( 0)
#endif
#define IA_ERR         ( 1)


#ifdef _DM648_
#define L3_PROGRAM        "IA_LPR_PROG"
#define L2_DATA           "L2_DATA"
#define L3_DATA_CACHE_0   "L3_DATA0"
#define L3_DATA_CACHE_2   "L3_DATA2"
#endif

/*BEGIN Added by t00499 for DM814x*/
#ifdef _DM81XX_
#define L3_PROGRAM       "dspcode"
#define L2_DATA          "dspmeml2"
#define L3_DATA_CACHE_0  "dspmem_c"
#define L3_DATA_CACHE_2  "dspmem_c"
#endif

#ifdef  __cplusplus
}
#endif  /* end of __cplusplus */

#endif  /* end of _IA_TYPEDEF_H_ */

