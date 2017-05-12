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
        _pSrvNameW(NULL), _pData(NULL), _pSnag(NULL)
    {
      try {
        _pUserNameW = getWideStrCopy(*(Nan::Utf8String(inArgs.GetUserName())));
      }
      catch (Snag* pS) {
        _pSnag = pS;
        SetErrorMessage("ERROR");
      }
      if (_pSnag) return;

      if (inArgs.HasHostName())
      {
        try { 
          _pSrvNameW = getWideStrCopy(*(Nan::Utf8String(inArgs.GetHostName())));
        }
        catch (Snag* pS) {
          _pSnag = pS;
          SetErrorMessage("ERROR");
        }
      }
    }

    ~UserInfoWorker()
    {
      if (_pUserNameW) free(_pUserNameW);
      if (_pSrvNameW) free(_pSrvNameW);
      if (_pData) workaroundDeleteData();
      if (_pSnag) delete _pSnag;
    }

    void Execute()
    {
      if (_pSnag) return;

      try {
        _pData = getUserDetails(_pUserNameW, _pSrvNameW, _getDetailed);
      }
      catch (Snag* pS)
      {
        _pSnag = pS;
        SetErrorMessage("AARGH");
      }
    }

    void HandleErrorCallback()
    {
      const unsigned argc = 1;
      Local<Value> exc = (_pSnag->message() == NULL) ?
        Nan::ErrnoException(_pSnag->code(), NULL, "Unknown error") :
        Nan::Error(_pSnag->message());
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
      workaroundDeleteData();
      _pData = NULL;
      callback->Call(argc, argv);
    }

  private:
    void workaroundDeleteData()
    {
      if (_getDetailed) delete (struct UserDeepInfo*) _pData;
      else delete _pData;
    }

    wchar_t* _pUserNameW;
    wchar_t* _pSrvNameW;
    bool _getDetailed;
    UserInfo* _pData;
    Snag* _pSnag;
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
    catch (Snag* pSnag)
    {
      Local<Value> exc = (pSnag->message() == NULL) ?
        Nan::ErrnoException(pSnag->code(), NULL, "Unknown error") :
        Nan::Error(pSnag->message());
      delete pSnag;
      if (pUserNameW) free(pUserNameW);
      return Nan::ThrowError(exc);
    }

    info.GetReturnValue().Set(transformUserInfoPlus(*pData, wantFull));

    if (wantFull) delete (struct UserDeepInfo*) pData;
    else delete pData;
  }
}

