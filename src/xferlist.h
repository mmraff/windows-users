#ifndef __XFERLIST_H__
#define __XFERLIST_H__

#include <assert.h>
#include <string.h>    // memcpy, memset
#include <errno.h>     // ENOMEM
#include "usererrs.h"

template<class T> class XferList
{
  public:
    XferList() : _pList(NULL), _count(0), _space(0) {}
    ~XferList() { if (_pList != NULL) delete[] _pList; }
    inline size_t Length() { return _count; }
    void Expand(size_t);
    T& AddItem(const wchar_t*);
    const T& operator[] (int);

  protected:
    T* _pList;
    size_t _count;
    size_t _space;
};

// Used by Expand().
template<class T>
void detachItems(T* pList, size_t);

template<class T>
void XferList<T>::Expand(size_t addCount)
{
  assert(0 < addCount && "Invalid value to Expand()");

  if (_pList == NULL) // This is the initial allocation of memory
  {
    try { _pList = new T[addCount]; }
    catch (...) {}
    if (_pList == NULL) throw new SystemSnag(ENOMEM);

    memset(_pList, NULL, addCount * sizeof(T));
  }
  else // This is not the first Expand() call
  {
    T* pTemp = NULL;
    try { pTemp = new T[_count + addCount]; }
    catch (...) {}
    if (pTemp == NULL) throw new SystemSnag(ENOMEM);

    memcpy(pTemp, _pList, _count * sizeof(T)); // pTemp <-- _pList
    memset(pTemp + _count, NULL, addCount * sizeof(T));
    detachItems<T>(_pList, _count);
    delete[] _pList;
    _pList = pTemp;
  }
  _space = addCount;
}

template<class T>
const T& XferList<T>::operator[] (int index)
{
  assert(0 <= index && index < (int)_count && "XferList[]: invalid index");
  assert(_pList != NULL && "XferList[]: count is set but pList is NULL!");

  return _pList[index];
}

#endif // __XFERLIST_H__

