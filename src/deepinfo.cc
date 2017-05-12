#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <errno.h>
#include <assert.h>
#include "userinfo.h"
#include "deepinfo.h"
#include "wstrutils.h"
#include "usererrs.h"

namespace mmrwinusers {

UserDeepInfo::UserDeepInfo()
{
  size_t szBase = sizeof(struct UserInfo);
  memset((char*)this + szBase, NULL, sizeof(struct UserDeepInfo) - szBase);
}

UserDeepInfo::~UserDeepInfo()
{
  // char* values:
  if (this->sid) free(this->sid);
  if (this->homeDir) free(this->homeDir);
  if (this->homeDirDrive) free(this->homeDirDrive);
  if (this->profilePath) free(this->profilePath);
  if (this->scriptPath) free(this->scriptPath);
  if (this->userComment) free(this->userComment);
  if (this->appParams) free(this->appParams);
  if (this->workstations) free(this->workstations);
  if (this->logonServer) free(this->logonServer);

  // special: unsigned*
  if (this->logonHours) free(this->logonHours);
}

void UserDeepInfo::SetString(UserField field, const wchar_t* valueW)
{
  char** pMb = NULL;
  switch (field)
  {
    case UserField::sid:          pMb = &this->sid; break;
    case UserField::homeDir:      pMb = &this->homeDir; break;
    case UserField::homeDirDrive: pMb = &this->homeDirDrive; break;
    case UserField::profilePath:  pMb = &this->profilePath; break;
    case UserField::scriptPath:   pMb = &this->scriptPath; break;
    case UserField::userComment:  pMb = &this->userComment; break;
    case UserField::appParams:    pMb = &this->appParams; break;
    case UserField::workstations: pMb = &this->workstations; break;
    case UserField::logonServer:  pMb = &this->logonServer; break;
    default:
      UserInfo::SetString(field, valueW); return;
  }

  *pMb = valueW ? getMultibyteStrCopy(valueW) : NULL;
}

void UserDeepInfo::SetFlag(UserField field, bool flag)
{
  switch (field)
  {
    case UserField::isAccountsOperator:
      this->isAccountsOperator = flag; break;
    case UserField::isServerOperator:
      this->isServerOperator = flag; break;
    case UserField::isPrintOperator:
      this->isPrintOperator = flag; break;
    default:
      UserInfo::SetFlag(field, flag);
  }
}

void UserDeepInfo::SetNumeric(UserField field, unsigned long value)
{
  switch (field)
  {
    case UserField::primaryGroupId: this->primaryGroupId = value; break;
    case UserField::maxStorage:     this->maxStorage = value; break;
    case UserField::countryCode:    this->countryCode = value; break;
    case UserField::codePage:       this->codePage = value; break;
    case UserField::passwdAge:      this->passwdAge = value; break;
    case UserField::accountExpires: this->accountExpires = value; break;
    case UserField::lastLogon:      this->lastLogon = value; break;
    case UserField::logonCount:     this->logonCount = value; break;
    case UserField::badPasswdCount: this->badPasswdCount = value; break;

    case UserField::privLevel:      this->privLevel = (PrivLevel)value; break;

    default: UserInfo::SetNumeric(field, value);
  }
}

// Special case field 'logonHours': requires conversion from array of 21 bytes
// to array of 7 unsigned ints, with only the low 24 bits used in each.
// Looking at only the low 24 bits of each uint, we get a representation of the
// 24 hours of a day that can be processed in a slightly more intuitive way.
// Example: 10101010 11001100 11110000 11001100 10101010 10011001 ...
// becomes: 00000000111100001100110010101010 00000000100110011010101011001100 ...
// (where Sunday and Monday are represented before the ellipsis)
void UserDeepInfo::SetLogonHours(const unsigned char* p)
{
  // PBYTE value is returned by WinAPI for the field in question;
  // PBYTE is a near pointer to BYTE; BYTE is unsigned char (WinDef.h).
  // Therefore, we receive the source as unsigned char*, and access elements
  // with array notation, expecting to get 21 unsigned char values.
  size_t sz = sizeof(unsigned) * 7;
  this->logonHours = (unsigned*)malloc(sz);
  if (this->logonHours == NULL) throw new SystemSnag(ENOMEM);
  memset(this->logonHours, NULL, sz);

  int i = 0, o = 0;
  for (; o < 7; o++)
  {
    this->logonHours[o]  = (unsigned)p[i++];
    this->logonHours[o] |= (unsigned)p[i++] << 8;
    this->logonHours[o] |= (unsigned)p[i++] << 16;
  }
}

} // namespace mmrwinusers

