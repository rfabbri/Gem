#include "SIMD.h"
#include "m_pd.h"
#include <string>

int GemSIMD::cpuid = GEM_SIMD_NONE;
int GemSIMD::realcpuid = GEM_SIMD_NONE;



GemSIMD :: GemSIMD()
{
 int compiledarchs=0;
 cpuid=simd_runtime_check();

 std::string compiledstr;

#ifdef __MMX__
 if(compiledarchs>0)compiledstr+="/";
 compiledstr+="MMX";
 compiledarchs++;
#endif
#ifdef __SSE2__
 if(compiledarchs>0)compiledstr+="/";
 compiledstr+="SSE2";
 compiledarchs++;
#endif

#ifdef __VEC__
 if(compiledarchs>0)compiledstr+="/";
 compiledstr+="AltiVec";
 compiledarchs++;
#endif

 if(compiledarchs>0) {
   verbose(-1, "GEM: compiled for %s architecture", compiledstr.c_str());
 }

 if(cpuid){
   std::string usingstr;
   switch (cpuid){
#ifdef __SSE2__
   case GEM_SIMD_SSE2:
     usingstr="SSE2";
     break;
#endif
#ifdef __MMX__
   case GEM_SIMD_MMX:
     usingstr="MMX";
     break;
#endif
#ifdef __VEC__
   case GEM_SIMD_ALTIVEC:
     usingstr="AltiVec";
     break;
#endif
   default:
     usingstr="no";
     break;
   case 0: /* this should never happen but is here for compilers who hate to "switch" with only one "case" */
     usingstr="invalid";
   }
   verbose(-1, "GEM: using %s optimization", usingstr.c_str());
 }
}

GemSIMD :: ~GemSIMD()
{
}

int GemSIMD :: requestCPU(int req_cpuid)
{
  if(GEM_SIMD_ALTIVEC==realcpuid && (GEM_SIMD_SSE2==req_cpuid || GEM_SIMD_MMX==req_cpuid))// invalid selection
    return cpuid;
  if(GEM_SIMD_ALTIVEC==req_cpuid && (GEM_SIMD_SSE2==realcpuid || GEM_SIMD_MMX==realcpuid))// invalid selection
    return cpuid;

  if(realcpuid<req_cpuid)
    { /* requested too much! */
      cpuid=realcpuid;
    }
  else
    { /* fine! */
      cpuid=req_cpuid;
    }

  return cpuid;
}

int GemSIMD :: getCPU() 
{
  return cpuid;
}

int GemSIMD :: simd_runtime_check(void)
{
    unsigned int eax=0, edx=0;

#if defined(_WIN32) && defined(_MSC_VER)
	unsigned int	feature;
	#define _MMX_FEATURE_BIT        0x00800000
    /* on w32 we assume that there is only x86 */
    /* _MSC_VER and __GNUC__ are different in how you inline assember */
    __asm
      {
		push ebx
		push ecx
		push edx
		xor	eax,eax
		cpuid

		mov   eax, 1
		cpuid
		
		mov      feature,edx

		pop     ebx
		pop		ecx
		pop     edx
       }

		if(feature & 1<<26) {
				realcpuid=GEM_SIMD_SSE2;
				return realcpuid;
		  }
		if(feature & 1<<23) {
				realcpuid=GEM_SIMD_MMX;
				return realcpuid;
		  }

#elif defined (__GNUC__) && defined (__POWERPC__)
    /* detecting whether a powerPC supports AltiVec or not seems to be complicated.
     * therefore we rely on the compile-time setting (preprocessor)
     * see also http://lists.debian.org/debian-powerpc/2004/01/msg00106.html
     */
/*
unsigned int OGPProcessorHasAltivec(void)
{
    static int _OGPProcessorHasAltivec = -1;

    if (_OGPProcessorHasAltivec < 0) {
        int name[] = {CTL_HW, HW_VECTORUNIT};
        size_t size;

        size = sizeof(_OGPProcessorHasAltivec);
        if (sysctl(name, 2, &_OGPProcessorHasAltivec, &size, NULL, 0) < 0) {
            perror("sysctl");
            _OGPProcessorHasAltivec = 0;
        }
    }

    return _OGPProcessorHasAltivec;
}
*/
# if defined __VEC__
    realcpuid=GEM_SIMD_ALTIVEC;
    return realcpuid;
# endif /* __VEC__ */

#elif defined (__GNUC__) && (defined(_X86_) || defined(__i386__) || defined(__i586__) || defined(__i686__))
    __asm__("push %%ebx \n" /* ebx might be used as PIC register  :-(  */
            "cpuid      \n"
            "pop  %%ebx \n"
	    : "=a"(eax),"=d"(edx) : "a" (1): "cx");
#elif defined (__GNUC__) && defined (__x86_64__)
/* for x86_64 */
    __asm__("mov  %%bx, %%si \n"
            "cpuid           \n"
            "xchg %%bx, %%si \n"
	    : "=a"(eax),"=d"(edx)
	    : "a" (1)
	    : "cx", "si");
# endif
    /* AltiVec should have been handled above */
    /* now comes the parsing of the cpuid on x86 hardware
     * see http://www.sandpile.org/ia32/cpuid.htm for what which bit is
     */
#ifdef __SSE2__
    if(edx & 1<<26){ // SSE2
      realcpuid=GEM_SIMD_SSE2;
      return realcpuid;
    }
#endif
#ifdef __SSE__
    if(edx & 1<<25){ // SSE
    }
#endif
#ifdef __MMX__
    if(edx & 1<<23){ // MMX
      realcpuid=GEM_SIMD_MMX;
      return realcpuid;
    }
#endif
    realcpuid=GEM_SIMD_NONE;
    return realcpuid;
}
