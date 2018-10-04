#include <nan.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "argsresolv.h"
#include "wstrutils.h"
#include "xferlist.h"
#include "wraputils.h"
#include "usererrs.h"

using namespace v8;

XferList<char*>* getUserGroupsList(const wchar_t*, const wchar_t*, bool);

class GroupListWorker : public Nan::AsyncWorker {
  public:
    GroupListWorker(UsersUserArgsResolver& inArgs, bool global = false)
      : Nan::AsyncWorker(inArgs.GetCallback()), _global(global),
        _pUserNameW(NULL), _pSrvNameW(NULL), _pList(NULL), _errCode(0)
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
        }
      }
    }

    ~GroupListWorker() {}

    void Execute()
    {
      if (_errCode != 0) return;

      try {
        _pList = getUserGroupsList(_pUserNameW, _pSrvNameW, _global);
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
        takeNames(_pList)
      };
      delete _pList; _pList = NULL;
      callback->Call(argc, argv);
    }

  private:
    bool _global;
    wchar_t* _pUserNameW;
    wchar_t* _pSrvNameW;
    void* _pList;
    unsigned long _errCode;
};

void _usersGetGroups(const Nan::FunctionCallbackInfo<v8::Value>& info, bool global)
{
  UsersUserArgsResolver args(info, false);
  if (args.HasError()) return Nan::ThrowError(args.GetError());

  if (args.HasCallback())
  {
    GroupListWorker* pWorker = new GroupListWorker(args, global);
    Nan::AsyncQueueWorker(pWorker);
  }
  else
  {
    void* pList = NULL;
    wchar_t* pUserNameW = NULL;
    try {
      pUserNameW = getWideStrCopy(*(Nan::Utf8String(args.GetUserName())));
      pList = getUserGroupsList(pUserNameW, NULL, global);
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

    info.GetReturnValue().Set(takeNames(pList));

    delete pList;
  }
}

NAN_METHOD(usersGetLocalGroups)
{
  return _usersGetGroups(info, false);
}

NAN_METHOD(usersGetGlobalGroups)
{
  return _usersGetGroups(info, true);
}

