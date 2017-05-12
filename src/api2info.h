#ifndef __API2INFO_H__
#define __API2INFO_H__

#include <lm.h>
#include "deepinfo.h"

DWORD translateFilter(unsigned);

void translateUserInfo20(LPUSER_INFO_20, struct mmrwinusers::UserInfo&);

void translateUserInfo4(LPUSER_INFO_4, struct mmrwinusers::UserDeepInfo&);

#endif

