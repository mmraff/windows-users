#if !defined(UNICODE)
#define UNICODE
#endif

#include <stdio.h>     // only for fprintf DEBUG output
//#include <stdlib.h>
#include <assert.h>
#include <windows.h>   // GetLastError, LocalFree, ...
#include <lm.h>        // USER_INFO_xx and various #defines
#include <Sddl.h>      // ConvertSidToStringSid
#include "userinfo.h"
#include "deepinfo.h"
#include "usererrs.h"

#pragma comment(lib, "advapi32.lib")  // ConvertSidToStringSid
#pragma comment(lib, "kernel32.lib")  // GetLastError, LocalFree

#define DW_TO_BOOL(v) (v ? true : false)

using namespace mmrwinusers;

DWORD translateFilter(unsigned filter)
{
  DWORD dwFilter = 0;

  if (0 < filter)
  {
    if (filter & AccountType::normal) dwFilter |= FILTER_NORMAL_ACCOUNT;
    if (filter & AccountType::tempDuplicate)
      dwFilter |= FILTER_TEMP_DUPLICATE_ACCOUNT;
    if (filter & AccountType::interdomainTrust)
      dwFilter |= FILTER_INTERDOMAIN_TRUST_ACCOUNT;
    if (filter & AccountType::workstationTrust)
      dwFilter |= FILTER_WORKSTATION_TRUST_ACCOUNT;
    if (filter & AccountType::serverTrust)
      dwFilter |= FILTER_SERVER_TRUST_ACCOUNT;
  }

  return dwFilter;
}

AccountType translateAccountType(DWORD flags)
{
  AccountType acctType = AccountType::normal;
  switch (flags & UF_ACCOUNT_TYPE_MASK)
  {
    case UF_NORMAL_ACCOUNT: break;
    case UF_TEMP_DUPLICATE_ACCOUNT:
      acctType = AccountType::tempDuplicate; break;
    case UF_INTERDOMAIN_TRUST_ACCOUNT:
      acctType = AccountType::interdomainTrust; break;
    case UF_SERVER_TRUST_ACCOUNT:
      acctType = AccountType::serverTrust; break;
    case UF_WORKSTATION_TRUST_ACCOUNT:
      acctType = AccountType::workstationTrust; break;
    default:
      assert(NULL && "User data with unrecognized account type");
  }

  return acctType;
}

PrivLevel translatePrivLevel(DWORD priv)
{
  PrivLevel level;
  switch (priv)
  {
    case USER_PRIV_GUEST: level = PrivLevel::guest; break;
    case USER_PRIV_USER:  level = PrivLevel::user; break;
    case USER_PRIV_ADMIN: level = PrivLevel::admin; break;
    default:
      assert(NULL && "User data with unrecognized privilege level");
  }

  return level;
}

void translateUserInfo20(LPUSER_INFO_20 pApiInfo, struct UserInfo& userInfo)
{
  if (pApiInfo->usri20_full_name)
    userInfo.SetString(UserField::fullName, pApiInfo->usri20_full_name);
  else // DEBUG ONLY!
    fprintf(stderr, "*** NULL returned for full_name field *** (api2info.cc)\n");
  if (pApiInfo->usri20_comment)
    userInfo.SetString(UserField::comment, pApiInfo->usri20_comment);

  userInfo.SetNumeric(
    UserField::accountType,
    translateAccountType(pApiInfo->usri20_flags)
  );

  userInfo.SetFlag(UserField::disabled,
    DW_TO_BOOL(pApiInfo->usri20_flags & UF_ACCOUNTDISABLE));
  userInfo.SetFlag(UserField::passwdRequired,
    !DW_TO_BOOL(pApiInfo->usri20_flags & UF_PASSWD_NOTREQD));
  userInfo.SetFlag(UserField::passwdCanChange,
    !DW_TO_BOOL(pApiInfo->usri20_flags & UF_PASSWD_CANT_CHANGE));
  userInfo.SetFlag(UserField::lockedOut,
    DW_TO_BOOL(pApiInfo->usri20_flags & UF_LOCKOUT));
  userInfo.SetFlag(UserField::passwdNeverExpires,
    DW_TO_BOOL(pApiInfo->usri20_flags & UF_DONT_EXPIRE_PASSWD));
  userInfo.SetFlag(UserField::encryptedPasswdOK,
    DW_TO_BOOL(pApiInfo->usri20_flags & UF_ENCRYPTED_TEXT_PASSWORD_ALLOWED));
  userInfo.SetFlag(UserField::notDelegated,
    DW_TO_BOOL(pApiInfo->usri20_flags & UF_NOT_DELEGATED));
  userInfo.SetFlag(UserField::smartcardRequired,
    DW_TO_BOOL(pApiInfo->usri20_flags & UF_SMARTCARD_REQUIRED));
  userInfo.SetFlag(UserField::useOnlyDES,
    DW_TO_BOOL(pApiInfo->usri20_flags & UF_USE_DES_KEY_ONLY));
  userInfo.SetFlag(UserField::noPreauthRequired,
    DW_TO_BOOL(pApiInfo->usri20_flags & UF_DONT_REQUIRE_PREAUTH));
  userInfo.SetFlag(UserField::trustedForDeleg,
    DW_TO_BOOL(pApiInfo->usri20_flags & UF_TRUSTED_FOR_DELEGATION));
  userInfo.SetFlag(UserField::passwdExpired,
    DW_TO_BOOL(pApiInfo->usri20_flags & UF_PASSWORD_EXPIRED));
  userInfo.SetFlag(UserField::trustedToAuthDeleg,
    DW_TO_BOOL(pApiInfo->usri20_flags & UF_TRUSTED_TO_AUTHENTICATE_FOR_DELEGATION));
}

void translateUserInfo4(LPUSER_INFO_4 pApiInfo, struct UserDeepInfo& userInfo)
{
  userInfo.SetString(UserField::name, pApiInfo->usri4_name);
  if (pApiInfo->usri4_full_name)
    userInfo.SetString(UserField::fullName, pApiInfo->usri4_full_name);
  else // DEBUG ONLY!
    fprintf(stderr, "*** NULL returned for full_name field *** (api2info.cc)\n");
  if (pApiInfo->usri4_comment)
    userInfo.SetString(UserField::comment, pApiInfo->usri4_comment);

  userInfo.SetNumeric(
    UserField::accountType,
    translateAccountType(pApiInfo->usri4_flags)
  );

  userInfo.SetFlag(UserField::disabled,
    DW_TO_BOOL(pApiInfo->usri4_flags & UF_ACCOUNTDISABLE));
  userInfo.SetFlag(UserField::passwdRequired,
    !DW_TO_BOOL(pApiInfo->usri4_flags & UF_PASSWD_NOTREQD));
  userInfo.SetFlag(UserField::passwdCanChange,
    !DW_TO_BOOL(pApiInfo->usri4_flags & UF_PASSWD_CANT_CHANGE));
  userInfo.SetFlag(UserField::lockedOut,
    DW_TO_BOOL(pApiInfo->usri4_flags & UF_LOCKOUT));
  userInfo.SetFlag(UserField::passwdNeverExpires,
    DW_TO_BOOL(pApiInfo->usri4_flags & UF_DONT_EXPIRE_PASSWD));
  userInfo.SetFlag(UserField::encryptedPasswdOK,
    DW_TO_BOOL(pApiInfo->usri4_flags & UF_ENCRYPTED_TEXT_PASSWORD_ALLOWED));
  userInfo.SetFlag(UserField::notDelegated,
    DW_TO_BOOL(pApiInfo->usri4_flags & UF_NOT_DELEGATED));
  userInfo.SetFlag(UserField::smartcardRequired,
    DW_TO_BOOL(pApiInfo->usri4_flags & UF_SMARTCARD_REQUIRED));
  userInfo.SetFlag(UserField::useOnlyDES,
    DW_TO_BOOL(pApiInfo->usri4_flags & UF_USE_DES_KEY_ONLY));
  userInfo.SetFlag(UserField::noPreauthRequired,
    DW_TO_BOOL(pApiInfo->usri4_flags & UF_DONT_REQUIRE_PREAUTH));
  userInfo.SetFlag(UserField::trustedForDeleg,
    DW_TO_BOOL(pApiInfo->usri4_flags & UF_TRUSTED_FOR_DELEGATION));
  userInfo.SetFlag(UserField::passwdExpired,
    DW_TO_BOOL(pApiInfo->usri4_flags & UF_PASSWORD_EXPIRED));
  userInfo.SetFlag(UserField::trustedToAuthDeleg,
    DW_TO_BOOL(pApiInfo->usri4_flags & UF_TRUSTED_TO_AUTHENTICATE_FOR_DELEGATION));

  // The above mirrors translateUserInfo20.
  // The following is unique to USER_INFO_4-sourced data.

  wchar_t* pSidW = NULL;
  if (!ConvertSidToStringSid(pApiInfo->usri4_user_sid, &pSidW))
    throw APIError(GetLastError());
  userInfo.SetString(UserField::sid, pSidW);
  if (LocalFree(pSidW) != NULL)
    throw SysError(GetLastError());

  if (pApiInfo->usri4_home_dir)
    userInfo.SetString(UserField::homeDir, pApiInfo->usri4_home_dir);
  if (pApiInfo->usri4_home_dir_drive)
    userInfo.SetString(UserField::homeDirDrive, pApiInfo->usri4_home_dir_drive);
  if (pApiInfo->usri4_profile)
    userInfo.SetString(UserField::profilePath, pApiInfo->usri4_profile);
  if (pApiInfo->usri4_script_path)
    userInfo.SetString(UserField::scriptPath, pApiInfo->usri4_script_path);
  if (pApiInfo->usri4_usr_comment)
    userInfo.SetString(UserField::userComment, pApiInfo->usri4_usr_comment);
  if (pApiInfo->usri4_parms)
    userInfo.SetString(UserField::appParams, pApiInfo->usri4_parms);
  if (pApiInfo->usri4_workstations)
    userInfo.SetString(UserField::workstations, pApiInfo->usri4_workstations);
  if (pApiInfo->usri4_logon_server)
    userInfo.SetString(UserField::logonServer, pApiInfo->usri4_logon_server);

  userInfo.SetFlag(UserField::isAccountsOperator,
    DW_TO_BOOL(pApiInfo->usri4_auth_flags & AF_OP_ACCOUNTS));
  userInfo.SetFlag(UserField::isServerOperator,
    DW_TO_BOOL(pApiInfo->usri4_auth_flags & AF_OP_SERVER));
  userInfo.SetFlag(UserField::isPrintOperator,
    DW_TO_BOOL(pApiInfo->usri4_auth_flags & AF_OP_PRINT));

  userInfo.SetNumeric(UserField::primaryGroupId, pApiInfo->usri4_primary_group_id);
  userInfo.SetNumeric(UserField::maxStorage, pApiInfo->usri4_max_storage);
  userInfo.SetNumeric(UserField::privLevel,
                      translatePrivLevel(pApiInfo->usri4_priv));
  userInfo.SetNumeric(UserField::countryCode, pApiInfo->usri4_country_code);
  userInfo.SetNumeric(UserField::codePage, pApiInfo->usri4_code_page);
  userInfo.SetNumeric(UserField::passwdAge, pApiInfo->usri4_password_age);
  userInfo.SetNumeric(UserField::accountExpires, pApiInfo->usri4_acct_expires);
  userInfo.SetNumeric(UserField::lastLogon, pApiInfo->usri4_last_logon);
  userInfo.SetNumeric(UserField::logonCount, pApiInfo->usri4_num_logons);
  userInfo.SetNumeric(UserField::badPasswdCount, pApiInfo->usri4_bad_pw_count);

  userInfo.SetLogonHours(pApiInfo->usri4_logon_hours);
}

