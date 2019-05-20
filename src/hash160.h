#ifndef _BCRYPTO_HASH160_HH
#define _BCRYPTO_HASH160_HH
#include <node.h>
#include <nan.h>
#include "nettle/sha2.h"
#include "nettle/ripemd160.h"

class BHash160 : public Nan::ObjectWrap {
public:
  static NAN_METHOD(New);
  static void Init(v8::Local<v8::Object> &target);

  BHash160();
  ~BHash160();

  struct sha256_ctx ctx;

private:
  static NAN_METHOD(Init);
  static NAN_METHOD(Update);
  static NAN_METHOD(Final);
  static NAN_METHOD(Digest);
  static NAN_METHOD(Root);
  static NAN_METHOD(Multi);
};
#endif
