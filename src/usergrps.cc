#if !defined(UNICODE)
#define UNICODE
#endif

#pragma comment(lib, "netapi32.lib")

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <lm.h>
#include <wchar.h>
#include "xferlist.h"
#include "usererrs.h"

// -- getUserGroupsList() ----------------------------------------------------
// Fetch the names of all local OR global groups of which the named user is a
// member on this system, or on another host if its name is given.
// [in] pUserNameW: name of the user account that is to be queried
// [in] pServerNameW: name of the system that is to be queried
//      (optional - local machine is the default)
// [in] global: if true, request names of global groups instead of local groups
//
XferList<char*>* getUserGroupsList(
  const wchar_t* pUserNameW,
  const wchar_t* pServerNameW,
  bool global
)
{
  XferList<char*>* pList = NULL;

  DWORD dwInfoLevel = 0; // 0 is only level available for NetUserGetLocalGroups,
                         // and we're not interested in global group attributes
  DWORD dwFlags = 0; // the only alternate value opens a can of worms
  DWORD entriesReadArg = 0, totalEntriesArg = 0;
  LPBYTE pBuffer = NULL;
  NET_API_STATUS status; // to hold the result code

  if (global)
  {
    status = NetUserGetGroups(
      pServerNameW,
      pUserNameW,
      dwInfoLevel,
      &pBuffer,
      MAX_PREFERRED_LENGTH,
      &entriesReadArg,
      &totalEntriesArg
    );
  }
  else
  {
    status = NetUserGetLocalGroups(
      pServerNameW,
      pUserNameW,
      dwInfoLevel,
      dwFlags,
      &pBuffer,
      MAX_PREFERRED_LENGTH,
      &entriesReadArg,
      &totalEntriesArg
    );
  }

  if (status != NERR_Success)
    throw APIError(status);

  pList = new XferList<char*>();

  if (pBuffer != NULL)
  {
    try { pList->Expand(entriesReadArg); }
    catch (...) {
      delete pList;
      NetApiBufferFree(pBuffer);
      throw;
    }

    if (global)
    {
      LPGROUP_USERS_INFO_0 pInEntry = (LPGROUP_USERS_INFO_0)pBuffer;

      for (; 0 < entriesReadArg; entriesReadArg--, pInEntry++)
      {
        try {
          // The API is really really ridiculous in this case...
          if (wcscmp(pInEntry->grui0_name, L"None") != 0)
            pList->AddItem(pInEntry->grui0_name);
        }
        catch (...) {
          delete pList;
          NetApiBufferFree(pBuffer);
          throw;
        }
      }
    }
    else
    {
      LPLOCALGROUP_USERS_INFO_0 pInEntry = (LPLOCALGROUP_USERS_INFO_0)pBuffer;

      for (; 0 < entriesReadArg; entriesReadArg--, pInEntry++)
      {
        try {
          // TODO: can local group membership ever be "None"?
          pList->AddItem(pInEntry->lgrui0_name);
        }
        catch (...) {
          delete pList;
          NetApiBufferFree(pBuffer);
          throw;
        }
      }
    }

    NetApiBufferFree(pBuffer);
  }

  return pList;
}

