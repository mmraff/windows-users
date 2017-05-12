#ifndef __DEEPINFO_H__
#define __DEEPINFO_H__

#include "userinfo.h"

namespace mmrwinusers {

// Styled after the struct USER_INFO_4; to be used by getDetails(true).
struct UserDeepInfo : public UserInfo
{
  UserDeepInfo();
  virtual ~UserDeepInfo();
  void SetString(UserField, const wchar_t*);
  void SetFlag(UserField, bool value);
  void SetNumeric(UserField, unsigned long);
  void SetLogonHours(const unsigned char*);

  char* sid;
  char* homeDir;
  char* homeDirDrive;
  char* profilePath;
  char* scriptPath;
  char* userComment;
  char* appParams;
  char* workstations;
  char* logonServer;
  unsigned* logonHours;
  unsigned long primaryGroupId;
  unsigned long maxStorage;
  unsigned long countryCode;
  unsigned long codePage;
  unsigned long passwdAge;
  unsigned long accountExpires;
  unsigned long lastLogon;
  unsigned long logonCount;
  unsigned long badPasswdCount;
  PrivLevel privLevel;
  bool isAccountsOperator;
  bool isServerOperator;
  bool isPrintOperator;
};

} // namespace mmrwinusers

#endif

