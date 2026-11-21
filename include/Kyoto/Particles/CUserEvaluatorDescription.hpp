#ifndef _CUSEREVALUATORDESCRIPTION
#define _CUSEREVALUATORDESCRIPTION

#include "rstl/static_assert.hpp"
#include "rstl/types.h"

#include "Kyoto/Particles/IElement.hpp"

#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"



// Guessed name
class CUserEvaluatorDescription {
public:
  CUserEvaluatorDescription();
  ~CUserEvaluatorDescription();

  // Guessed name
  CRealElement* GetEvaluator() const { return mRealElements[0].get(); }

  rstl::reserved_vector< rstl::single_ptr< CRealElement >, 10 > mRealElements;
  rstl::reserved_vector< rstl::single_ptr< CIntElement >, 5 > mIntElements;
  rstl::reserved_vector< rstl::single_ptr< CColorElement >, 2 > mColorElements;
  rstl::reserved_vector< rstl::single_ptr< CVectorElement >, 3 > mVectorElements;
  rstl::reserved_vector< rstl::single_ptr< CEmitterElement >, 2 > mEmitterElements;
  rstl::reserved_vector< rstl::single_ptr< CModVectorElement >, 2 > mModVectorElements;
  rstl::reserved_vector< rstl::single_ptr< CUVElement >, 2 > mTextureElements;
};
CHECK_SIZEOF(CUserEvaluatorDescription, 0x84)


#endif // _CUSEREVALUATORDESCRIPTION
