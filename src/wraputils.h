#ifndef __WRAPUTILS_H__
#define __WRAPUTILS_H__

#include "userinfo.h"

v8::Local<v8::Array> takeNames(void*);
v8::Local<v8::Object> transformUserInfo(const struct mmrwinusers::UserInfo&);
v8::Local<v8::Object> transformUserInfoPlus(const struct mmrwinusers::UserInfo&, bool);

#endif
