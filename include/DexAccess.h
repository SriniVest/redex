/**
 * Copyright (c) 2016-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

enum DexAccessBits {
  DEX_ACCESS_ABSTRACT      = 0x0400,
  DEX_ACCESS_INTERFACE     = 0x0200,
  DEX_ACCESS_NATIVE        = 0x0100,
};

#define ACCESSFLAGS                         \
  AF(PUBLIC,       public,           0x1)   \
  AF(PRIVATE,      private,          0x2)   \
  AF(PROTECTED,    protected,        0x4)   \
  AF(STATIC,       static,           0x8)   \
  AF(FINAL,        final,           0x10)   \
  AF(SYNCHRONIZED, synchronized,    0x20)   \
  AF(VOLATILE,     volatile,        0x40)   \
  AF(BRIDGE,       bridge,          0x40)   \
  AF(TRANSIENT,    transient,       0x80)   \
  AF(VARARGS,      varargs,         0x80)   \
  AF(NATIVE,       native,         0x100)   \
  AF(INTERFACE,    interface,      0x200)   \
  AF(ABSTRACT,     abstract,       0x400)   \
  AF(STRICT,       strict,         0x800)   \
  AF(SYNTHETIC,    synthetic,     0x1000)   \
  AF(ANNOTATION,   annotation,    0x2000)   \
  AF(ENUM,         enum,          0x4000)   \
  AF(CONSTRUCTOR,  constructor,  0x10000)   \
  AF(DECLARED_SYNCHRONIZED, declared_synchronized, 0x2000)

enum DexAccessFlags : uint32_t {
#define AF(uc, lc, val) ACC_ ## uc = val,
  ACCESSFLAGS
#undef AF
};

inline DexAccessFlags operator&(const DexAccessFlags a,
                                const DexAccessFlags b) {
  return (DexAccessFlags)((uint32_t)a & (uint32_t)b);
}

inline DexAccessFlags operator|(const DexAccessFlags a,
                                const DexAccessFlags b) {
  return (DexAccessFlags)((uint32_t)a | (uint32_t)b);
}

inline DexAccessFlags operator~(const DexAccessFlags a) {
  return (DexAccessFlags)(~(uint32_t)a);
}

#define AF(uc, lc, val)                         \
  inline bool is_ ## lc(DexAccessFlags flags) { \
    return (flags & ACC_ ## uc) == ACC_ ## uc;  \
  }                                             \
                                                \
  template<class DexMember>                     \
  bool is_ ## lc(DexMember* m) {                \
    return is_ ## lc(m->get_access());          \
  }
ACCESSFLAGS
#undef AF

//
// DexAccessFlags visibility accessors
//

const DexAccessFlags VISIBILITY_MASK =
    static_cast<DexAccessFlags>(ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED);

inline bool is_package_protected(DexAccessFlags flags) {
  return
      (flags & (DexAccessFlags::ACC_PRIVATE | DexAccessFlags::ACC_PUBLIC)) == 0;
}

template<class DexMember>
bool is_package_protected(DexMember* m) {
  return is_package_protected(m->get_access());
}

template<class DexMember>
void set_public(DexMember* m) {
  m->set_access((m->get_access() & ~VISIBILITY_MASK) | ACC_PUBLIC);
}

template<class DexMember>
void set_public_final(DexMember* m) {
  m->set_access((m->get_access() & ~VISIBILITY_MASK) | ACC_PUBLIC | ACC_FINAL);
}
