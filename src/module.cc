#include <nan.h>

using namespace v8;

void addWinUsersConstants(Handle<Object>);
NAN_METHOD(usersList);
NAN_METHOD(usersGetDetails);
NAN_METHOD(usersGetLocalGroups);
NAN_METHOD(usersGetGlobalGroups);

void init(Handle<Object> exports) {

  exports->Set(Nan::New<String>("list").ToLocalChecked(),
               Nan::New<FunctionTemplate>(usersList)->GetFunction());

  exports->Set(Nan::New<String>("getDetails").ToLocalChecked(),
               Nan::New<FunctionTemplate>(usersGetDetails)->GetFunction());

  exports->Set(Nan::New<String>("getLocalGroups").ToLocalChecked(),
               Nan::New<FunctionTemplate>(usersGetLocalGroups)->GetFunction());

  exports->Set(Nan::New<String>("getGlobalGroups").ToLocalChecked(),
               Nan::New<FunctionTemplate>(usersGetGlobalGroups)->GetFunction());

  addWinUsersConstants(exports);
}

NODE_MODULE(winusers, init);

