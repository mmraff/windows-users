#include <stdlib.h>
#include <assert.h>
#include "userinfo.h"
#include "xferlist.h"
#include "usererrs.h"
#include "wstrutils.h"

using namespace mmrwinusers;
/*
 * Specializations of templates from xferlist.h
 */

XferList<char*>::~XferList()
{
  if (_pList != NULL)
  {
    for (size_t i = 0; i < _count; i++) free(_pList[i]);
    delete[] _pList;
  }
}

template<> void detachItems(char** pList, size_t count) {}

template<> void detachItems(struct UserInfo* pList, size_t count)
{
  for (size_t i = 0; i < count; i++) pList[i].ReleaseResources();
}

struct UserInfo& XferList<struct UserInfo>::AddItem(const wchar_t* pNameW)
{
  assert(0 < _space && "XferList::AddItem: no space; use Expand(n) first!");
  struct UserInfo& newInfo = _pList[_count];
  newInfo.SetString(UserField::name, pNameW);
  _count++;
  _space--;
  return newInfo;
}

char*& XferList<char*>::AddItem(const wchar_t* pNameW)
{
  assert(0 < _space && "XferList::AddItem: no space; use Expand(n) first!");
  char*& newName = _pList[_count] = getMultibyteStrCopy(pNameW);
  _count++;
  _space--;
  return newName;
}

