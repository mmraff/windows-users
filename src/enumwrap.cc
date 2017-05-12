#include <nan.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "argsresolv.h"
#include "wstrutils.h"
#include "userinfo.h"
#include "xferlist.h"
#include "wraputils.h"
#include "usererrs.h"

using namespace v8;
using namespace mmrwinusers;

XferList<char*>* getUserNameList(const wchar_t*, unsigned);
XferList<struct UserInfo>* getUserInfoList(const wchar_t*, unsigned);

Local<Array> transformUserInfoList(void* pV)
{
  XferList<struct UserInfo>* pList = (XferList<struct UserInfo>*) pV;
  Local<Array> results = Nan::New<Array>();
  size_t count = pList->Length();

  for (size_t i = 0; i < count; i++)
    (*results)->Set(i, transformUserInfo((*pList)[i]));

  return results;
}

class UserEnumWorker : public Nan::AsyncWorker {
  public:
    UserEnumWorker(UsersListArgsResolver& inArgs)
      : Nan::AsyncWorker(inArgs.GetCallback()),
        _getDetailed(inArgs.DetailsWanted()),
        _filterVal(inArgs.GetFilterValue()),
        _pSrvNameW(NULL), _pList(NULL), _pSnag(NULL)
    {
      if (inArgs.HasHostName())
      {
        Nan::Utf8String* pName = new Nan::Utf8String(inArgs.GetHostName());
        try { 
          _pSrvNameW = getWideStrCopy(*(*pName));
        }
        catch (Snag* pS) {
          _pSnag = pS;
          SetErrorMessage("ERROR");
        }
        delete pName;
      }
    }

    ~UserEnumWorker()
    {
      if (_pSrvNameW) free(_pSrvNameW);
      if (_pList) delete _pList;
      if (_pSnag) delete _pSnag;
    }

    void Execute()
    {
      if (_pSnag != NULL) return;

      try {
        if (_getDetailed) _pList = getUserInfoList(_pSrvNameW, _filterVal);
        else _pList = getUserNameList(_pSrvNameW, _filterVal);
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
        _getDetailed ? transformUserInfoList(_pList) : takeNames(_pList)
      };
      delete _pList; _pList = NULL;
      callback->Call(argc, argv);
    }

  private:
    bool _getDetailed;
    unsigned _filterVal;
    wchar_t* _pSrvNameW;
    void* _pList;
    Snag* _pSnag;
};

NAN_METHOD(usersList) {

  UsersListArgsResolver args(info);
  if (args.HasError()) return Nan::ThrowError(args.GetError());

  if (args.HasCallback())
  {
    UserEnumWorker* pWorker = new UserEnumWorker(args);
    Nan::AsyncQueueWorker(pWorker);
  }
  else
  {
    void* pList = NULL;
    try {
      if (args.DetailsWanted())
        pList = getUserInfoList(NULL, args.GetFilterValue());
      else pList = getUserNameList(NULL, args.GetFilterValue());
    }
    catch (Snag* pSnag)
    {
      Local<Value> exc = (pSnag->message() == NULL) ?
        Nan::ErrnoException(pSnag->code(), NULL, "Unknown error") :
        Nan::Error(pSnag->message());
      delete pSnag;
      return Nan::ThrowError(exc);
    }

    if (args.DetailsWanted())
      info.GetReturnValue().Set(transformUserInfoList(pList));
    else info.GetReturnValue().Set(takeNames(pList));

    delete pList;
  }
}

