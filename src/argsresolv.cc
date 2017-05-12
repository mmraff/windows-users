#include <nan.h>
#include <assert.h>
#include "argsresolv.h"

using namespace v8;

// --- UsersArgsResolver (base class) ------------------------------------

// All of the JS functions optionally take a hostname and/or callback argument.
// The derived args-resolver classes call init when they're done looking for
// their particular first arguments, but only if there are any arguments left.
void UsersArgsResolver::init(int currIdx)
{
  assert(currIdx < _info.Length() && "init called with index beyond end of args");

  if (_info[currIdx]->IsFunction())
  {
    // Saw callback first, so no hostname
    _cbIdx = currIdx;
  }
  else if (_info[currIdx]->IsString() || _info[currIdx]->IsStringObject())
  {
    if (_info[currIdx]->ToString()->Length() != 0)
    {
      _hostIdx = currIdx;
    }
    // else this arg is empty, so we leave hostname arg NULL.
  }
  else if (!_info[currIdx]->IsUndefined() && !_info[currIdx]->IsNull())
  {
    char* errMsg;
    switch (currIdx) {
      case 0: errMsg = "Invalid type for first argument"; break;
      case 1: errMsg = "Invalid type for second argument"; break;
      case 2: errMsg = "Invalid type for third argument"; break;
      default: errMsg = "Processing beyond end of expected arguments";
    }
    _error = Nan::TypeError(errMsg);
    return;
  }
  currIdx++;

  if (currIdx < _info.Length()  && _cbIdx == -1)
  {
    if (_info[currIdx]->IsFunction())
    {
      _cbIdx = currIdx;
    }
    else
      fprintf(stderr, "Warning: ignoring extra argument(s)\n");
  }

  // It's only OK to leave out the callback arg when hostname arg is empty.
  if (_hostIdx != -1 && _cbIdx == -1)
    _error = Nan::SyntaxError("Must provide callback when giving a hostname");
}

Local<String> UsersArgsResolver::GetHostName()
{
  assert(_hostIdx != -1 && "Attempt to GetHostName() when there is none!");

  return _info[_hostIdx].As<String>();
}

Nan::Callback* UsersArgsResolver::GetCallback()
{
  assert(_cbIdx != -1 && "Attempt to GetCallback() when there is none!");

  return new Nan::Callback(_info[_cbIdx].As<Function>());
}

// --- UsersListArgsResolver -------------------------------------------------

UsersListArgsResolver::UsersListArgsResolver(
  const Nan::FunctionCallbackInfo<v8::Value>& callerInfo
) : UsersArgsResolver(callerInfo), _detailed(false), _filterVal(0)
{
  int currIdx = 0;

  if (_info.Length() == 0) return;

  if (_info[0]->IsObject()) {

    Local<Object> opts = _info[0].As<Object>();
    Local<Object> refObj = Nan::New<v8::Object>();
    Nan::Utf8String argProtoName(opts->ObjectProtoToString());
    Nan::Utf8String refProtoName(refObj->ObjectProtoToString());
    // Nan code for Utf8String uses null termination, so this is OK:
    if (strcmp(*argProtoName, *refProtoName) == 0)
    {
      // This arg is a plain old JS object; we expect it to be
      // the container for options.

      // Check for the 'detailed' option
      Local<String> propDetailed = Nan::New<String>("detailed").ToLocalChecked();
      if (Nan::Has(opts, propDetailed).FromJust())
      {
        Local<Value> valDetailed = Nan::Get(opts, propDetailed).ToLocalChecked();
        if (valDetailed->IsBoolean() || valDetailed->IsBooleanObject())
          _detailed = valDetailed->IsTrue();
        else {
          _error = Nan::TypeError("Invalid type for option 'detailed'");
          return;
        }
      }
      // else, _detailed stays default value of false

      // Check for the 'filter' option
      // Validating this numerical value is more complicated...
      Local<String> propFilter = Nan::New<String>("filter").ToLocalChecked();
      if (Nan::Has(opts, propFilter).FromJust())
      {
        Local<Value> valFilter = Nan::Get(opts, propFilter).ToLocalChecked();
        if (!valFilter->IsNumber() && !valFilter->IsNumberObject())
        {
          _error = Nan::TypeError("Only numeric values allowed for option 'filter'");
          return;
        }
        double rawNumber = valFilter->NumberValue();
        if (rawNumber < 0 || UINT32_MAX < rawNumber)
        {
          _error = Nan::RangeError("Out-of-range value for option 'filter'");
          return;
        }

        if (valFilter->IsNumber())
        {
          if (valFilter->IsUint32()) _filterVal = valFilter->Uint32Value();
          else // This should never happen...
            fprintf(stderr,
              "Warning: number %f could not be converted to uint32!\n",
              rawNumber);
        }
        else if (valFilter->IsNumberObject())
        {
          Nan::Maybe<uint32_t> uintMaybe = Nan::To<uint32_t>(valFilter);
          if (uintMaybe.IsJust()) _filterVal = uintMaybe.FromJust();
          else // This should never happen...
            fprintf(stderr,
              "Warning: Number object %f could not be converted to uint32!\n",
              rawNumber);
        }
      } // end: if options object has a 'filter' property
      // else, _filterVal stays default value of 0

      currIdx++;

    } // end: if arg is a plain old JS object
  } // end: if 1st arg IsObject()

  if (currIdx < _info.Length()) init(currIdx);
}

// --- UsersUserArgsResolver -------------------------------------------------

// Specific to functions that require a username
UsersUserArgsResolver::UsersUserArgsResolver(
  const Nan::FunctionCallbackInfo<Value>& callerInfo,
  bool flagAllowed = false
) : UsersArgsResolver(callerInfo), _hasUser(false), _fullDetails(false)
{
  int currIdx = 0;

  if (_info.Length() == 0 || _info[0]->IsUndefined() || _info[0]->IsNull()) {
    _error = Nan::SyntaxError("Must provide username");
    return;
  }

  if (!(_info[0]->IsString() || _info[0]->IsStringObject())) {
    _error = Nan::TypeError("Invalid type for username argument");
    return;
  }

  if (_info[0]->ToString()->Length() == 0) {
    _error = Nan::SyntaxError("Username cannot be empty");
    return;
  }

  _hasUser = true;
  currIdx++;

  if (_info.Length() > 1)
  {
    if (flagAllowed && (_info[1]->IsBoolean() || _info[1]->IsBooleanObject())) {
      if (_info[1]->IsTrue()) _fullDetails = true;
      currIdx++;
    }
    if (currIdx < _info.Length()) init(currIdx);
  }
}

Local<String> UsersUserArgsResolver::GetUserName()
{
  assert(_hasUser && "Attempt to GetUserName() when there is none!");

  return _info[0].As<String>();
}

