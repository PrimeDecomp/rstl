#ifndef _CUNPREDICTABLERANDOM
#define _CUNPREDICTABLERANDOM

#include "rstl/static_assert.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"
#include "rstl/types.h"

class CRandom16;

class CUnpredictableRandomBool { // Guessed name
public:
  CUnpredictableRandomBool();
  bool Generate(CRandom16& random, float probability); // Guessed name

private:
  bool mHistory[20]; // Guessed name
};
CHECK_SIZEOF(CUnpredictableRandomBool, 0x14)

class CUnpredictableRandomInt { // Guessed name
public:
  CUnpredictableRandomInt();
  virtual ~CUnpredictableRandomInt() {}
  virtual int SelectCandidate(CRandom16& random, int numChoices,
                              int rejectedChoice);                           // Guessed name
  virtual int GetChoiceProbabilityPercent(int numChoices, int choice) const; // Guessed name
  virtual int GetConsecutiveRepeatLimit(int numChoices, int choice) const;   // Guessed name

  int Generate(CRandom16& random, int numChoices);               // Guessed name
  void SetMinRepeatLimit(int limit) { mMinRepeatLimit = limit; } // Guessed name

private:
  int mMinRepeatLimit; // Guessed name
  int mHistory[10]; // Guessed name
  rstl::reserved_vector< int, 16 >* mCandidates; // Guessed name
};
CHECK_SIZEOF(CUnpredictableRandomInt, 0x34)

class CUnpredictableRandomWeighted : public CUnpredictableRandomInt { // Guessed name
public:
  CUnpredictableRandomWeighted(int count, const float* weights);
  ~CUnpredictableRandomWeighted() {}
  int SelectCandidate(CRandom16& random, int numChoices, int rejectedChoice); // Guessed name
  int GetChoiceProbabilityPercent(int numChoices, int choice) const;          // Guessed name

  int Generate(CRandom16& random); // Guessed name
  void RecomputeTotalWeight();     // Guessed name
  const rstl::vector< float >& GetWeights() const { return mWeights; } // Guessed name

private:
  rstl::vector< float > mWeights; // Guessed name
  float mTotalWeight; // Guessed name
  rstl::reserved_vector< float, 16 >* mCandidateWeights; // Guessed name
  float mRemainingWeight; // Guessed name
};
CHECK_SIZEOF(CUnpredictableRandomWeighted, 0x50)

class CUnpredictableRandomFloat { // Guessed name
public:
  CUnpredictableRandomFloat();
  float Generate(CRandom16& random); // Guessed name

private:
  bool mRejected; // Guessed name
  float mHistory[5]; // Guessed name
  CUnpredictableRandomInt mInteger; // Guessed name
};
CHECK_SIZEOF(CUnpredictableRandomFloat, 0x4c)

#endif // _CUNPREDICTABLERANDOM
