#include <nan.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "argsresolv.h"
#include "wstrutils.h"
#include "deepinfo.h"
#include "wraputils.h"
#include "usererrs.h"

using namespace v8;
using namespace mmrwinusers;

UserInfo* getUserDetails(const wchar_t*, const wchar_t*, bool);

class UserInfoWorker : public Nan::AsyncWorker {
  public:
    UserInfoWorker(UsersUserArgsResolver& inArgs)
      : Nan::AsyncWorker(inArgs.GetCallback()),
        _getDetailed(inArgs.FullDetailsWanted()), _pUserNameW(NULL),
        _pSrvNameW(NULL), _pData(NULL), _errCode(0)
    {
      try {
        _pUserNameW = getWideStrCopy(*(Nan::Utf8String(inArgs.GetUserName())));
      }
      catch (WinUsersError& er) {
        _errCode = er.code();
        SetErrorMessage(er.what());
        return;
      }

      if (inArgs.HasHostName())
      {
        try { 
          _pSrvNameW = getWideStrCopy(*(Nan::Utf8String(inArgs.GetHostName())));
        }
        catch (WinUsersError& er) {
          _errCode = er.code();
          SetErrorMessage(er.what());
          free(_pUserNameW);
        }
      }
    }

    ~UserInfoWorker() {}

    void Execute()
    {
      if (_errCode != 0) return;

      try {
        _pData = getUserDetails(_pUserNameW, _pSrvNameW, _getDetailed);
      }
      catch (WinUsersError& er)
      {
        _errCode = er.code();
        SetErrorMessage(er.what());
      }
      free(_pUserNameW);
      if (_pSrvNameW) free(_pSrvNameW);
    }

    void HandleErrorCallback()
    {
      const unsigned argc = 1;
      Local<Value> exc = (this->ErrorMessage() == NULL) ?
        Nan::ErrnoException(_errCode, NULL, "Unknown error") :
        Nan::Error(this->ErrorMessage());
      Local<Value> argv[argc] = { exc };
      callback->Call(argc, argv);
    }

    void HandleOKCallback()
    {
      const unsigned argc = 2;
      Local<Value> argv[argc] = {
        Nan::Null(),
        transformUserInfoPlus(*_pData, _getDetailed)
      };

      if (_getDetailed) delete (struct UserDeepInfo*) _pData;
      else delete _pData;
      _pData = NULL;

      callback->Call(argc, argv);
    }

  private:
    wchar_t* _pUserNameW;
    wchar_t* _pSrvNameW;
    bool _getDetailed;
    UserInfo* _pData;
    unsigned long _errCode;
};

NAN_METHOD(usersGetDetails) {

  UsersUserArgsResolver args(info, true);
  if (args.HasError()) return Nan::ThrowError(args.GetError());

  if (args.HasCallback())
  {
    UserInfoWorker* pWorker = new UserInfoWorker(args);
    Nan::AsyncQueueWorker(pWorker);
  }
  else
  {
    UserInfo* pData = NULL;
    wchar_t* pUserNameW = NULL;
    bool wantFull = args.FullDetailsWanted();
    try {
      pUserNameW = getWideStrCopy(*(Nan::Utf8String(args.GetUserName())));
      pData = getUserDetails(pUserNameW, NULL, wantFull);
      free(pUserNameW);
      pUserNameW = NULL;
    }
    catch (WinUsersError& er)
    {
      Local<Value> exc = (er.what() == NULL) ?
        Nan::ErrnoException(er.code(), NULL, "Unknown error") :
        Nan::Error(er.what());

      if (pUserNameW) free(pUserNameW);
      return Nan::ThrowError(exc);
    }

    info.GetReturnValue().Set(transformUserInfoPlus(*pData, wantFull));

    if (wantFull) delete (struct UserDeepInfo*) pData;
    else delete pData;
  }
}

