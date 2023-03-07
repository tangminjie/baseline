
#ifndef APPLICATE_INCLUDE_INFRA_DEFINE_H
#define APPLICATE_INCLUDE_INFRA_DEFINE_H

#include <cstdint>

#ifdef __cplusplus

#if __cplusplus < 201103L
#ifndef nullptr
#define nullptr    0
#endif
#endif

#endif

#ifndef __GNUC__
#define __attribute__(x)
#endif

#define ARCFACE_JOIN(X,Y)         ARCFACE_DO_JOIN(X,Y)
#define ARCFACE_DO_JOIN(X,Y)      ARCFACE_DO_JOIN2(X,Y)
#define ARCFACE_DO_JOIN2(X,Y)     X##Y

#endif //APPLICATE_INCLUDE_INFRA_DEFINE_H
