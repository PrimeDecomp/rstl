#ifndef _CINSTRUCTION
#define _CINSTRUCTION
#include "rstl/RstlVersions.h"

#include "types.h"

#include "Kyoto/CToken.hpp"

#include "rstl/vector.hpp"

class CFontRenderState;
class CTextRenderBuffer;

class CInstruction {
public:
  virtual ~CInstruction() = 0;
  virtual void Invoke(CFontRenderState& state, CTextRenderBuffer* buf) const = 0;
  virtual void PageInvoke(CFontRenderState& state, CTextRenderBuffer* buf) const {}
  virtual void GetAssets(rstl::vector< CToken >& assetsOut) const {}
  virtual uint GetAssetCount() const { return 0; }
// TODO: verify that this Prime/Echoes difference is real
#if RSTL_PRIME1
#if RSTL_VERSION >= VERSION_GM8P_00
  virtual bool IsLineInstruction() const { return false; }
#endif
#else
  virtual bool IsLineInstruction() const { return false; }
#endif

};

inline CInstruction::~CInstruction() {}

#endif // _CINSTRUCTION
