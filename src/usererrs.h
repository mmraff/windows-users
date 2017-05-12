#ifndef __USERERRS_H__
#define __USERERRS_H__

class Snag {
  public:
    Snag(unsigned long hcode) : _code(hcode) {}
    ~Snag() {}
    unsigned long code() { return _code; }
    virtual const char* message() = 0;
  protected:
    unsigned long _code;
};

class APISnag : virtual public Snag {
  public:
    APISnag(unsigned long hcode) : Snag(hcode) {}
    const char* message();
};

class SystemSnag : virtual public Snag {
  public:
    SystemSnag(unsigned long hcode) : Snag(hcode) {}
    const char* message();
};

class UsageSnag : virtual public Snag {
  public:
    UsageSnag(unsigned long hcode) : Snag(hcode) {}
    const char* message();
};

#endif

