// TODO:
// * consider combining all the Win API stuff in one cc file
// * consider putting the template specializations in the places where they're used,
//  rather than having a separate file for them.
#if !defined(UNICODE)
#define UNICODE
#endif

#pragma comment(lib, "netapi32.lib")

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <lm.h>
#include <wchar.h>
#include "wstrutils.h"
#include "api2info.h"
#include "deepinfo.h"
#include "usererrs.h"

using namespace mmrwinusers;

// -- getUserDetails() --------------------------------------------------------
// Fetch information about the named user account on this system, or from
// another host if its name is given. The request can indicate that full data
// is desired, or just a modest set of data.
//
// [in] pUserNameW: name of the user account from which to fetch data
// [in] pServerNameW: name of the system that is to be queried
//      (optional - local machine is the default)
// [in] full: flag to request full details if true, else moderate details
//
UserInfo* getUserDetails(
  const wchar_t* pUserNameW,
  const wchar_t* pServerNameW,
  bool full
)
{
  UserInfo* pInfo = NULL;
  DWORD dwInfoLevel = full ? 4 : 20; // for struct USER_INFO_4 / USER_INFO_20

  NET_API_STATUS status; // Result code from NetUserGetInfo()
  LPBYTE pBuffer; // NetUserGetInfo() will put results here

  status = NetUserGetInfo(
    pServerNameW,
    pUserNameW,
    dwInfoLevel,
    &pBuffer
  );

  if (status != NERR_Success) throw new APISnag(status);
  else
  {
    if (pBuffer)
    {
      try {
        if (full)
        {
          USER_INFO_4* pUI4 = (USER_INFO_4*)pBuffer;
          pInfo = new UserDeepInfo();
          translateUserInfo4(pUI4, (UserDeepInfo&)*pInfo);
        }
        else
        {
          USER_INFO_20* pUI20 = (USER_INFO_20*)pBuffer;
          pInfo = new UserInfo();
          pInfo->SetString(UserField::name, pUI20->usri20_name);
          translateUserInfo20(pUI20, *pInfo);
        }
      }
      catch (Snag* pS) {
        NetApiBufferFree(pBuffer);
        throw pS;
      }
    }

    NetApiBufferFree(pBuffer);
  }

  return pInfo;
}

