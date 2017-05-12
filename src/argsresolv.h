#ifndef __ARGSRESOLV_H__
#define __ARGSRESOLV_H__

class UsersArgsResolver {
  public:
    UsersArgsResolver(const Nan::FunctionCallbackInfo<v8::Value>& callerInfo)
      : _info(callerInfo), _hostIdx(-1), _cbIdx(-1), _error(Nan::Null()) {}
    inline ~UsersArgsResolver() {}
    inline bool HasHostName() { return _hostIdx != -1; }
    inline bool HasCallback() { return _cbIdx != -1; }
    inline bool HasError() { return _error->IsNull() == false; }
    v8::Local<v8::String> GetHostName();
    Nan::Callback* GetCallback();
    inline const v8::Local<v8::Value>& GetError() { return _error; }

  protected:
    const Nan::FunctionCallbackInfo<v8::Value>& _info;
    v8::Local<v8::Value> _error;
    int _hostIdx;
    int _cbIdx;

    void init(int);

    // Prevent copy constructing - not in our best interest
    UsersArgsResolver(const UsersArgsResolver&);
    void operator=(const UsersArgsResolver&);
};

class UsersListArgsResolver : public UsersArgsResolver {
  public:
    UsersListArgsResolver(const Nan::FunctionCallbackInfo<v8::Value>&);
    inline bool DetailsWanted() { return _detailed; }
    inline unsigned GetFilterValue() { return _filterVal; }

  private:
    bool _detailed;
    unsigned _filterVal;

    UsersListArgsResolver(const UsersListArgsResolver&);
    void operator=(const UsersListArgsResolver&);
};

class UsersUserArgsResolver : public UsersArgsResolver {
  public:
    UsersUserArgsResolver(const Nan::FunctionCallbackInfo<v8::Value>&, bool);
    v8::Local<v8::String> GetUserName();
    inline bool FullDetailsWanted() { return _fullDetails; }

  private:
    bool _hasUser;
    bool _fullDetails;

    UsersUserArgsResolver(const UsersUserArgsResolver&);
    void operator=(const UsersUserArgsResolver&);
};

#endif

