#if !defined(UNICODE)
#define UNICODE
#endif

#pragma comment(lib, "netapi32.lib")

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <lm.h>
#include <wchar.h>
#include <stdexcept>
#include "userinfo.h"
#include "api2info.h"
#include "xferlist.h"
#include "usererrs.h"

using namespace mmrwinusers;

// -- getUserNameList() --------------------------------------------------------
// Fetch the account names of all users defined on this system, or from
// another host if its name is given.
// [in] pServerNameW: name of the system that is to be queried
//      (optional - local machine is the default)
// [in] filter: a bit-field that flags types of accounts to include
//      (see NetUserEnum page at MSDN)
//
XferList<char*>* getUserNameList(const wchar_t* pServerNameW, unsigned filter)
{
  XferList<char*>* pList = NULL;
  DWORD dwInfoLevel = 0;
  DWORD dwFilter = translateFilter(filter);

  // NetUserEnum() arg that must persist across multiple calls
  unsigned long resumeHandle = 0;

  NET_API_STATUS status; // NetUserEnum() result code

  do {
    // NetUserEnum() args
    LPUSER_INFO_0 pBuffer = NULL;
    unsigned long entriesReadArg = 0, totalEntriesArg = 0;

    status = NetUserEnum(
      pServerNameW,
      dwInfoLevel,
      dwFilter,
      (LPBYTE*) &pBuffer,
      MAX_PREFERRED_LENGTH,
      &entriesReadArg,
      &totalEntriesArg,
      &resumeHandle
    );

    if (status != NERR_Success && status != ERROR_MORE_DATA)
    {
      if (pBuffer != NULL) NetApiBufferFree(pBuffer);
      if (pList != NULL) delete pList;
      throw APIError(status);
    }

    if (pList == NULL) pList = new XferList<char*>();

    if (pBuffer == NULL) break;

    try { pList->Expand(entriesReadArg); }
    catch (...) {
      delete pList;
      NetApiBufferFree(pBuffer);
      throw;
    }

    // To index into the inbound list
    LPUSER_INFO_0 pInEntry = pBuffer;

    for (; 0 < entriesReadArg; entriesReadArg--, pInEntry++)
    {
      try {
        pList->AddItem(pInEntry->usri0_name);
      }
      catch (...) {
        delete pList;
        NetApiBufferFree(pBuffer);
        throw;
      }
    }

    NetApiBufferFree(pBuffer);

  } while (status == ERROR_MORE_DATA);

  return pList;
}

// -- getUserInfoList() --------------------------------------------------------
// Fetch modest amount of account data of all users defined on this system,
// or from another host if its name is given.
//
// [in] pServerNameW: name of the system that is to be queried
//      (optional - local machine is the default)
//
XferList<struct UserInfo>* getUserInfoList(const wchar_t* pServerNameW, unsigned filter)
{
  XferList<struct UserInfo>* pList = NULL;
  DWORD dwInfoLevel = 20;
  DWORD dwFilter = translateFilter(filter);

  // NetUserEnum() arg that must persist across multiple calls
  unsigned long resumeHandle = 0;

  NET_API_STATUS status; // NetUserEnum() result code

  do {
    // NetUserEnum() args
    LPUSER_INFO_20 pBuffer = NULL;
    unsigned long entriesReadArg = 0, totalEntriesArg = 0;

    status = NetUserEnum(
      pServerNameW,
      dwInfoLevel,
      dwFilter,
      (LPBYTE*) &pBuffer,
      MAX_PREFERRED_LENGTH,
      &entriesReadArg,
      &totalEntriesArg,
      &resumeHandle
    );

    if (status != NERR_Success && status != ERROR_MORE_DATA)
    {
      if (pList != NULL) delete pList;
      throw APIError(status);
    }

    if (pList == NULL) pList = new XferList<struct UserInfo>();

    if (pBuffer == NULL) break;

    try { pList->Expand(entriesReadArg); }
    catch (...) {
      delete pList;
      NetApiBufferFree(pBuffer);
      throw;
    }

    // To index into the inbound list
    LPUSER_INFO_20 pInEntry = pBuffer;

    for (; 0 < entriesReadArg; entriesReadArg--, pInEntry++)
    {
      try {
        struct UserInfo& user = pList->AddItem(pInEntry->usri20_name);
        translateUserInfo20(pInEntry, user);
      }
      catch (...) {
        delete pList;
        NetApiBufferFree(pBuffer);
        throw;
      }
    }

    NetApiBufferFree(pBuffer);

  } while (status == ERROR_MORE_DATA);

  return pList;
}

