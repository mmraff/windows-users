#include <stdlib.h>
#include <assert.h>
#include "userinfo.h"
#include "wstrutils.h"

namespace mmrwinusers {

UserInfo::UserInfo()
{
  memset(this, NULL, sizeof(struct UserInfo));
}

UserInfo::~UserInfo()
{
  if (this->name) free(this->name);
  if (this->fullName) free(this->fullName);
  if (this->comment) free(this->comment);
}

// To be used after a memory move, after which the heap-allocated members
// of the old object must not be deleted.
void UserInfo::ReleaseResources()
{
  this->name = NULL;
  if (this->fullName) this->fullName = NULL;
  if (this->comment) this->comment = NULL;
}

void UserInfo::SetString(UserField field, const wchar_t* valueW)
{
  switch (field)
  {
    case UserField::name:
      this->name = getMultibyteStrCopy(valueW); break;
    case UserField::fullName:
      this->fullName = valueW ? getMultibyteStrCopy(valueW) : NULL; break;
    case UserField::comment:
      this->comment = valueW ? getMultibyteStrCopy(valueW) : NULL; break;
    default:
      assert(NULL && "Invalid fieldname passed to UserInfo::SetString()");
  }
}

void UserInfo::SetFlag(UserField field, bool flag)
{
  switch (field)
  {
    case UserField::disabled:
      this->disabled = flag ? 1 : 0; break;
    case UserField::passwdRequired:
      this->passwdRequired = flag ? 1 : 0; break;
    case UserField::passwdCanChange:
      this->passwdCanChange = flag ? 1 : 0; break;
    case UserField::lockedOut:
      this->lockedOut = flag ? 1 : 0; break;
    case UserField::passwdNeverExpires:
      this->passwdNeverExpires = flag ? 1 : 0; break;
    case UserField::encryptedPasswdOK:
      this->encryptedPasswdOK = flag ? 1 : 0; break;
    case UserField::notDelegated:
      this->notDelegated = flag ? 1 : 0; break;
    case UserField::smartcardRequired:
      this->smartcardRequired = flag ? 1 : 0; break;
    case UserField::useOnlyDES:
      this->useOnlyDES = flag ? 1 : 0; break;
    case UserField::noPreauthRequired:
      this->noPreauthRequired = flag ? 1 : 0; break;
    case UserField::trustedForDeleg:
      this->trustedForDeleg = flag ? 1 : 0; break;
    case UserField::passwdExpired:
      this->passwdExpired = flag ? 1 : 0; break;
    case UserField::trustedToAuthDeleg:
      this->trustedToAuthDeleg = flag ? 1 : 0; break;
    default:
      assert(NULL && "Invalid fieldname passed to UserInfo::SetFlag()");
  }
}

void UserInfo::SetNumeric(UserField field, unsigned long value)
{
  switch (field)
  {
    case UserField::accountType:
      this->accountType = (AccountType) value;
      break;
    default:
      assert(NULL && "Invalid fieldname passed to UserInfo::SetNumeric()");
  }
}

} // namespace mmrwinusers

