#if !defined(UNICODE)
#define UNICODE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include "usererrs.h"

wchar_t* getWideStrCopy(const char* pSrc)
{
  size_t sz = 0, retNum = 0;
  errno_t resCode = 0;

  assert(pSrc != NULL && "Null passed as src pointer!");

  resCode = mbstowcs_s(&sz, NULL, 0, pSrc, 0); // to get required buffer size
  if (resCode != 0)
  {
    if (resCode == EILSEQ) throw UsageError(EILSEQ);
    else throw SysError(resCode);
  }

  wchar_t* pDest = (wchar_t*)malloc(sizeof(wchar_t) * sz);
  if (pDest == NULL) throw SysError(ENOMEM);

  // We do all we can to avoid an error in the following, so if something goes
  // wrong, we throw mystery value resCode:
  resCode = mbstowcs_s(
    &retNum, // [out]
    pDest,   // [out]
    sz,      // [in]
    pSrc,    // [in]
    sz - 1   // [in]
  );
  if (resCode != 0) {
    free(pDest);
    throw SysError(resCode);
  }

  return pDest;
}

char* getMultibyteStrCopy(const wchar_t* pSrc)
{
  size_t sz = 0, retNum = 0;
  errno_t resCode = 0;

  assert(pSrc != NULL && "Null passed as src pointer!");

  resCode = wcstombs_s(&sz, NULL, 0, pSrc, 0); // to get required buffer size
  if (resCode != 0) throw SysError(resCode);
  
  char* pDest = (char*)malloc(sz);
  if (pDest == NULL) throw SysError(ENOMEM);

  resCode = wcstombs_s(
    &retNum, // [out]
    pDest,   // [out]
    sz,      // [in]
    pSrc,    // [in]
    sz       // [in]
  );
  if (resCode != 0) {
    free(pDest);
    throw SysError(resCode);
  }

  return pDest;
}

