#ifndef __USERERRS_2_H__
#define __USERERRS_2_H__

#include <stdexcept>

class WinUsersError : public std::exception {
  public:
    WinUsersError(unsigned long hcode) noexcept : _code(hcode) {}
    WinUsersError& operator=(const WinUsersError& other) noexcept {
      _code = other._code;
      return *this;
    }
    unsigned long code() const noexcept { return _code; }
    virtual const char* what() const noexcept { return ""; }
  protected:
    unsigned long _code;
};

class APIError : public WinUsersError {
  public:
    APIError(unsigned long hcode) : WinUsersError(hcode) {}
    virtual const char* what() const noexcept;
};

class SysError : public WinUsersError {
  public:
    SysError(unsigned long hcode) : WinUsersError(hcode) {}
    virtual const char* what() const noexcept;
};

class UsageError : public WinUsersError {
  public:
    UsageError(unsigned long hcode) : WinUsersError(hcode) {}
    virtual const char* what() const noexcept;
};

#endif

