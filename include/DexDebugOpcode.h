/**
 * Copyright (c) 2016-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

#include <list>

#include "dexdefs.h"
#include "Gatherable.h"

class DexIdx;
class DexOutputIdx;
class DexString;
class DexType;

class DexDebugOpcode : public Gatherable {
 private:
  union {
    uint32_t m_uvalue;
    int32_t m_value;
  };
  bool m_signed;

 protected:
  DexDebugItemOpcode m_opcode;

 public:
  DexDebugOpcode(DexDebugItemOpcode op, uint32_t v = DEX_NO_INDEX)
      : Gatherable() {
    m_opcode = op;
    m_uvalue = v;
    m_signed = false;
  }

  DexDebugOpcode(DexDebugItemOpcode op, int32_t v) : Gatherable() {
    m_opcode = op;
    m_value = v;
    m_signed = true;
  }

 public:
  virtual void encode(DexOutputIdx* dodx, uint8_t*& encdata);
  static DexDebugOpcode* make_opcode(DexIdx* idx, const uint8_t*& encdata);
  virtual DexDebugOpcode* clone() const { return new DexDebugOpcode(*this); }

  DexDebugItemOpcode opcode() const { return m_opcode; }

  uint32_t uvalue() const { return m_uvalue; }

  int32_t value() const { return m_value; }

  void set_opcode(DexDebugItemOpcode op) { m_opcode = op; }

  void set_uvalue(uint32_t uv) { m_uvalue = uv; }

  void set_value(int32_t v) { m_value = v; }
};

class DexDebugOpcodeSetFile : public DexDebugOpcode {
 private:
  DexString* m_str;

 public:
  DexDebugOpcodeSetFile(DexString* str) : DexDebugOpcode(DBG_SET_FILE) {
    m_str = str;
  }

  virtual void encode(DexOutputIdx* dodx, uint8_t*& encdata);
  virtual void gather_strings(std::vector<DexString*>& lstring);

  virtual DexDebugOpcodeSetFile* clone() const {
    return new DexDebugOpcodeSetFile(*this);
  }

  DexString* file() const { return m_str; }

  void set_file(DexString* file) { m_str = file; }
};

class DexDebugOpcodeStartLocal : public DexDebugOpcode {
 private:
  DexString* m_name;
  DexType* m_type;
  DexString* m_sig;

 public:
  DexDebugOpcodeStartLocal(uint32_t rnum,
                           DexString* name,
                           DexType* type,
                           DexString* sig = nullptr)
      : DexDebugOpcode(DBG_START_LOCAL, rnum) {
    m_name = name;
    m_type = type;
    m_sig = sig;
    if (sig) m_opcode = DBG_START_LOCAL_EXTENDED;
  }

  virtual void encode(DexOutputIdx* dodx, uint8_t*& encdata);
  virtual void gather_strings(std::vector<DexString*>& lstring);
  virtual void gather_types(std::vector<DexType*>& ltype);

  virtual DexDebugOpcodeStartLocal* clone() const {
    return new DexDebugOpcodeStartLocal(*this);
  }

  DexString* name() const { return m_name; }

  DexType* type() const { return m_type; }

  DexString* sig() const { return m_sig; }
};
