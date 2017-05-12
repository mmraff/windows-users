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
        _pUserNameW(NULL), _pSrvNameW(NULL), _pList(NULL), _pSnag(NULL)
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

    ~GroupListWorker()
    {
      if (_pUserNameW) free(_pUserNameW);
      if (_pSrvNameW) free(_pSrvNameW);
      if (_pList) delete _pList;
      if (_pSnag) delete _pSnag;
    }

    void Execute()
    {
      if (_pSnag != NULL) return;

      try {
        _pList = getUserGroupsList(_pUserNameW, _pSrvNameW, _global);
      }
      catch (Snag* pS)
      {
        _pSnag = pS;
        SetErrorMessage("OH NO");
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
    Snag* _pSnag;
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
    catch (Snag* pSnag)
    {
      Local<Value> exc = (pSnag->message() == NULL) ?
        Nan::ErrnoException(pSnag->code(), NULL, "Unknown error") :
        Nan::Error(pSnag->message());
      delete pSnag;
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

