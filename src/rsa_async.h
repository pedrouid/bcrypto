#ifndef _BCRYPTO_RSA_ASYNC_HH
#define _BCRYPTO_RSA_ASYNC_HH

#include <node.h>
#include <nan.h>
#include "rsa/rsa.h"

class BRSAWorker : public Nan::AsyncWorker {
public:
  BRSAWorker (
    int bits,
    int exp,
    Nan::Callback *callback
  );

  virtual ~BRSAWorker();
  virtual void Execute();
  void HandleOKCallback();

private:
  int bits;
  int exp;
  bcrypto_rsa_key_t *key;
};

#endif
