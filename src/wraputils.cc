#include <nan.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "wstrutils.h"
#include "deepinfo.h"
#include "xferlist.h"
#include "usererrs.h"

using namespace v8;
using namespace mmrwinusers;

#define SET_NAN_STR_OR_NULL(obj, src, field) \
  obj->Set(Nan::New<String>(#field).ToLocalChecked(), src.field == NULL ? \
           Nan::Null() : Nan::New<String>(src.field).ToLocalChecked())

#define SET_NAN_BOOLEAN(obj, src, field) \
  obj->Set(Nan::New<String>(#field).ToLocalChecked(), \
           Nan::New<Boolean>(src.field));

#define SET_NAN_UINT_OR_DBL(obj, src, field) \
if (src.field < 0x100000000) { \
  obj->Set(Nan::New<String>(#field).ToLocalChecked(), \
           Nan::New<Uint32>((uint32_t) src.field)); \
} else { \
  obj->Set(Nan::New<String>(#field).ToLocalChecked(), \
           Nan::New<Number>((double) src.field)); \
}

#define SET_NAN_UINT_OR_NULL(obj, src, field) \
if (src.field == (unsigned long) -1L) { \
  obj->Set(Nan::New<String>(#field).ToLocalChecked(), \
           Nan::Null()); \
} else SET_NAN_UINT_OR_DBL(obj, src, field)

Local<Array> takeNames(void* pV)
{
  XferList<char*>* pList = (XferList<char*>*) pV;
  Local<Array> results = Nan::New<Array>();
  size_t count = pList->Length();

  for (size_t i = 0; i < count; i++)
  {
    (*results)->Set(i, Nan::New<String>((*pList)[i]).ToLocalChecked());
  }

  return results;
}

Local<Object> transformUserInfo(const struct UserInfo& src)
{
  Local<Object> userObj = Nan::New<Object>();

  userObj->Set(Nan::New<String>("name").ToLocalChecked(),
                Nan::New<String>(src.name).ToLocalChecked());
    
  SET_NAN_STR_OR_NULL(userObj, src, fullName);
  SET_NAN_STR_OR_NULL(userObj, src, comment);

  char* acctType = "normal";
  switch (src.accountType)
  {
    case AccountType::normal: break;
    case AccountType::tempDuplicate:    acctType = "temp duplicate"; break;
    case AccountType::interdomainTrust: acctType = "interdomain trust"; break;
    case AccountType::workstationTrust: acctType = "workstation trust"; break;
    case AccountType::serverTrust:      acctType = "server trust"; break;
    default: // this should never happen!
      assert(NULL && "transformUserData: Illegal value for accountType");
  }
  userObj->Set(Nan::New<String>("accountType").ToLocalChecked(),
               Nan::New<String>(acctType).ToLocalChecked());

  SET_NAN_BOOLEAN(userObj, src, disabled);
  SET_NAN_BOOLEAN(userObj, src, lockedOut);
  SET_NAN_BOOLEAN(userObj, src, passwdRequired);
  SET_NAN_BOOLEAN(userObj, src, passwdCanChange);
  SET_NAN_BOOLEAN(userObj, src, passwdExpired);
  SET_NAN_BOOLEAN(userObj, src, passwdNeverExpires);
  SET_NAN_BOOLEAN(userObj, src, encryptedPasswdOK);
  SET_NAN_BOOLEAN(userObj, src, smartcardRequired);
  SET_NAN_BOOLEAN(userObj, src, useOnlyDES);
  SET_NAN_BOOLEAN(userObj, src, noPreauthRequired);
  SET_NAN_BOOLEAN(userObj, src, notDelegated);
  SET_NAN_BOOLEAN(userObj, src, trustedForDeleg);
  SET_NAN_BOOLEAN(userObj, src, trustedToAuthDeleg);

  return userObj;
}

Local<Object> transformUserInfoPlus(const struct UserInfo& src, bool full)
{
  Local<Object> userObj = transformUserInfo(src);
  if (!full) return userObj;

  const struct UserDeepInfo& deepSrc = (const struct UserDeepInfo&) src;

  userObj->Set(Nan::New<String>("sid").ToLocalChecked(),
               Nan::New<String>(deepSrc.sid).ToLocalChecked());
  SET_NAN_STR_OR_NULL(userObj, deepSrc, homeDir);
  SET_NAN_STR_OR_NULL(userObj, deepSrc, homeDirDrive);
  SET_NAN_STR_OR_NULL(userObj, deepSrc, profilePath);
  SET_NAN_STR_OR_NULL(userObj, deepSrc, scriptPath);
  SET_NAN_STR_OR_NULL(userObj, deepSrc, userComment);
  SET_NAN_STR_OR_NULL(userObj, deepSrc, appParams);
  SET_NAN_STR_OR_NULL(userObj, deepSrc, workstations);
  SET_NAN_STR_OR_NULL(userObj, deepSrc, logonServer);

  SET_NAN_BOOLEAN(userObj, deepSrc, isAccountsOperator);
  SET_NAN_BOOLEAN(userObj, deepSrc, isServerOperator);
  SET_NAN_BOOLEAN(userObj, deepSrc, isPrintOperator);

  char* priv = NULL;
  switch (deepSrc.privLevel)
  {
    case PrivLevel::guest: priv = "guest"; break;
    case PrivLevel::user:  priv = "user"; break;
    case PrivLevel::admin: priv = "administrator"; break;
    default: // this should never happen!
      assert(NULL && "transformUserInfoPlus: Illegal value for privLevel");
  }
  userObj->Set(Nan::New<String>("privilegeLevel").ToLocalChecked(),
               Nan::New<String>(priv).ToLocalChecked());

  // logonHours, the trickiest field
  int i = 0;
  bool allHours = true;
  for (i = 0; i < 7; i++)
    if (deepSrc.logonHours[i] != 0x00ffffff)
    {
       // not all 24 low bits set => not all hours allowed for current day
       allHours = false;
       break;
    }

  if (allHours)
    userObj->Set(Nan::New<String>("logonHours").ToLocalChecked(), Nan::Null());
  else
  {
    Local<Array> days = Nan::New<Array>();
    for (uint32_t d = 0; d < 7; d++)
    {
      Local<Array> currDayLogonHours = Nan::New<Array>();
      uint32_t elIdx = 0;
      unsigned currDayBits = deepSrc.logonHours[d];
      for (uint32_t h = 0; h < 24; h++)
      {
        if (currDayBits & 0x01)
          (*currDayLogonHours)->Set(elIdx++, Nan::New<Uint32>(h));
        currDayBits >>= 1;
      }
      (*days)->Set(d, currDayLogonHours);
    }
    userObj->Set(Nan::New<String>("logonHours").ToLocalChecked(), days);
  }

  // Numeric fields
  SET_NAN_UINT_OR_DBL(userObj, deepSrc, primaryGroupId);
  SET_NAN_UINT_OR_NULL(userObj, deepSrc, maxStorage);
  SET_NAN_UINT_OR_DBL(userObj, deepSrc, countryCode);
  SET_NAN_UINT_OR_DBL(userObj, deepSrc, codePage);
  SET_NAN_UINT_OR_DBL(userObj, deepSrc, passwdAge);
  SET_NAN_UINT_OR_NULL(userObj, deepSrc, accountExpires);
  SET_NAN_UINT_OR_DBL(userObj, deepSrc, lastLogon);
  SET_NAN_UINT_OR_NULL(userObj, deepSrc, logonCount);
  SET_NAN_UINT_OR_NULL(userObj, deepSrc, badPasswdCount);

  return userObj;
}

