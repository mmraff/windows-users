#include <nan.h>
#include <assert.h>
#include "userinfo.h"

using v8::String;
using v8::Uint32;
using namespace mmrwinusers;

#define WINUSERS_DEFINE_CONSTANT(target, name, val)                           \
do {                                                                          \
  v8::Local<String> cName = Nan::New<String>(name).ToLocalChecked();      \
  v8::Local<Uint32> cVal = Nan::New<Uint32>(val);                         \
  v8::PropertyAttribute cAttribs =                                            \
    static_cast<v8::PropertyAttribute>(v8::ReadOnly | v8::DontDelete);        \
  Nan::ForceSet(target, cName, cVal, cAttribs).FromJust();                    \
}                                                                             \
while (0)

void addWinUsersConstants(v8::Handle<v8::Object> target)
{
  v8::Local<v8::Object> constants = Nan::New<v8::Object>();

  WINUSERS_DEFINE_CONSTANT(constants, "NORMAL", AccountType::normal);
  WINUSERS_DEFINE_CONSTANT(constants, "TEMP_DUPLICATE", AccountType::tempDuplicate);
  WINUSERS_DEFINE_CONSTANT(constants, "INTERDOMAIN_TRUST", AccountType::interdomainTrust);
  WINUSERS_DEFINE_CONSTANT(constants, "WORKSTATION_TRUST", AccountType::workstationTrust);
  WINUSERS_DEFINE_CONSTANT(constants, "SERVER_TRUST", AccountType::serverTrust);

  target->Set(Nan::New<String>("constants").ToLocalChecked(), constants); // experiment!
}

