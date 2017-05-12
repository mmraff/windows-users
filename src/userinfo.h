#ifndef __USERINFO_H__
#define __USERINFO_H__

namespace mmrwinusers {

enum AccountType {
  normal = 1,
  tempDuplicate = 2,
  interdomainTrust = 4,
  workstationTrust = 8,
  serverTrust = 16
};

enum PrivLevel {
  guest, user, admin
};

#if !(defined(_MSC_VER) && (_MSC_VER >= 1800))
enum UserField {
#else
enum class UserField {
#endif
  name,
  fullName,
  comment,
  accountType,
  disabled,
  passwdRequired,
  passwdCanChange,
  lockedOut,
  passwdNeverExpires,
  encryptedPasswdOK,
  notDelegated,
  smartcardRequired,
  useOnlyDES,
  noPreauthRequired,
  trustedForDeleg,
  passwdExpired,
  trustedToAuthDeleg,
  // boundary: USER_INFO_4 provides the following fields; USER_INFO_20 does not
  sid,
  primaryGroupId,
  homeDir,
  homeDirDrive,
  profilePath,
  scriptPath,
  maxStorage,
  privLevel,
  isAccountsOperator,
  isServerOperator,
  isPrintOperator,
  userComment,
  countryCode,
  codePage,
  appParams,
  passwdAge,
  accountExpires,
  logonHours,
  workstations,
  logonServer,
  lastLogon,
  logonCount,
  badPasswdCount
};

// Styled after the struct USER_INFO_20; to be used by list({detailed: true})
// and by getDetails()
struct UserInfo
{
  UserInfo();
  ~UserInfo();
  void SetString(UserField, const wchar_t*);
  void SetFlag(UserField, bool value);
  void SetNumeric(UserField, unsigned long);
  void ReleaseResources();

  char* name;
  char* fullName;
  char* comment;
  AccountType accountType;
  unsigned disabled: 1;
  unsigned lockedOut: 1;
  unsigned passwdRequired: 1;
  unsigned passwdCanChange: 1;
  unsigned passwdExpired: 1;
  unsigned passwdNeverExpires: 1;
  unsigned encryptedPasswdOK: 1;
  unsigned smartcardRequired: 1;
  unsigned useOnlyDES: 1;
  unsigned noPreauthRequired: 1;
  unsigned notDelegated: 1;
  unsigned trustedForDeleg: 1;
  unsigned trustedToAuthDeleg: 1;
};

} // namespace mmrwinusers

#endif

