#include "Kyoto/CUnpredictableRandom.hpp"
#include "Kyoto/CRandom16.hpp"

#include "rstl/math.hpp"

static const bool skInitialBoolHistory[20] = {false, true,  true,  true,  false, false, true,
                                              true,  false, false, false, true,  false, true,
                                              false, false, true,  true,  false, false};
static const uchar skInitialIntHistory[10] = {3, 8, 0, 6, 7, 9, 7, 0, 3, 5};
static const float skInitialFloatHistory[10] = {0.9f, 0.3f, 0.1f, 0.4f, 0.6f,
                                                0.9f, 0.3f, 0.6f, 0.4f, 0.8f};

static const int skMaxTransitions[50] = {2,  4,  4,  4,  4,  4,  4,  4,  4,  6,  6,  6,  6,
                                         6,  6,  8,  8,  8,  8,  8,  10, 10, 10, 10, 10, 12,
                                         12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
                                         12, 12, 12, 12, 12, 12, 14, 14, 14, 14, 14};
static const int skMinTransitions[50] = {0, 0, 0, 0, 0, 1, 1, 1, 1,  1,  1,  1,  1,  4,  4,  4, 4,
                                         4, 4, 4, 4, 4, 4, 6, 6, 6,  6,  6,  6,  6,  8,  8,  8, 8,
                                         8, 8, 8, 8, 8, 8, 8, 8, 10, 10, 10, 12, 12, 12, 12, 12};
static const int skMaxMinorityRun[50] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2,
                                         2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                                         3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
static const int skMaxMajorityRun[50] = {25, 25, 25, 25, 25, 25, 25, 15, 15, 15, 14, 14, 14,
                                         12, 12, 12, 12, 10, 10, 10, 10, 10, 10, 10, 10, 10,
                                         10, 7,  7,  7,  7,  7,  7,  7,  7,  6,  6,  5,  5,
                                         5,  5,  5,  4,  4,  4,  4,  4,  3,  3,  3};
static const int skMaxIntegerRepeat[50] = {
    1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  3,  3,
    3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 7, 8, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10};

CUnpredictableRandomBool::CUnpredictableRandomBool() : mHistory() {
  for (int i = 0; i < 10; ++i) {
    mHistory[i] = skInitialBoolHistory[i];
  }
  for (int i = 0; i < 10; ++i) {
    mHistory[i + 10] = skInitialBoolHistory[i + 10];
  }
}

bool CUnpredictableRandomBool::Generate(CRandom16& random, float probability) {
  int index = int(100.f * probability + 0.5f) - 1;
  bool flipped = false;
  for (int i = 0; i < 19; ++i) {
    mHistory[i] = mHistory[i + 1];
  }
  if (probability <= 0.f) {
    mHistory[19] = false;
    return mHistory[19];
  }
  if (probability >= 1.f) {
    mHistory[19] = true;
    return mHistory[19];
  }
  if (probability > 0.5f) {
    flipped = true;
    index = int(100.f * (1.f - probability) + 0.5f) - 1;
  }
  if (index < 0) {
    index = 0;
  }
  mHistory[19] = random.Float() <= probability;
  int i;
  int changes = 0;
  for (i = 0; i < 19; ++i) {
    if (mHistory[i] != mHistory[i + 1]) {
      ++changes;
    }
  }
  if (changes > skMaxTransitions[index]) {
    mHistory[19] = mHistory[18];
  } else if (changes < skMinTransitions[index]) {
    mHistory[19] = !mHistory[18];
  }
  int repeats = 1;
  i = 18;
  for (; i >= 0; --i) {
    if (mHistory[i] != mHistory[i + 1]) {
      break;
    }
    ++repeats;
  }
  if ((mHistory[19] && repeats > (flipped ? skMaxMajorityRun[index] : skMaxMinorityRun[index])) ||
      (!mHistory[19] && repeats > (flipped ? skMaxMinorityRun[index] : skMaxMajorityRun[index]))) {
    mHistory[19] = !mHistory[19];
  }
  if (probability >= 0.4f && probability <= 0.6f) {
    if (mHistory[19] == mHistory[15] && mHistory[18] == mHistory[14] &&
        mHistory[17] == mHistory[13] && mHistory[16] == mHistory[12]) {
      if (!mHistory[19]) {
        mHistory[19] = true;
      } else {
        mHistory[19] = false;
      }
    }
  }
  if (probability >= 0.4f && probability <= 0.6f) {
    if ((!mHistory[19] && mHistory[16] == true && !mHistory[18] && mHistory[15] == true &&
         !mHistory[17] && mHistory[14] == true) ||
        (mHistory[19] == true && !mHistory[16] && mHistory[18] == true && !mHistory[15] &&
         mHistory[17] == true && !mHistory[14])) {
      if (!mHistory[19]) {
        mHistory[19] = true;
      } else {
        mHistory[19] = false;
      }
    }
  }
  return mHistory[19];
}

CUnpredictableRandomInt::CUnpredictableRandomInt() : mMinRepeatLimit(1) {
  for (int i = 0; i < 10; ++i) {
    mHistory[i] = skInitialIntHistory[i];
  }
}

int CUnpredictableRandomInt::GetChoiceProbabilityPercent(int numChoices, int choice) const {
  return rstl::min_val(int(100.f / numChoices), 99);
}

int CUnpredictableRandomInt::GetConsecutiveRepeatLimit(int numChoices, int choice) const {
  return skMaxIntegerRepeat[GetChoiceProbabilityPercent(numChoices, choice) / 2];
}

int CUnpredictableRandomInt::SelectCandidate(CRandom16& random, int numChoices,
                                             int rejectedChoice) {
  rstl::reserved_vector< int, 16 >& candidates = *mCandidates;
  if (numChoices <= 16) {
    if (candidates.empty()) {
      for (int i = 0; i < numChoices; ++i) {
        candidates.push_back(i);
      }
    } else if (candidates.size() > 1) {
      int i = 0;
      for (; i < candidates.size(); ++i) {
        if (candidates[i] == rejectedChoice) {
          break;
        }
      }
      for (; i < candidates.size() - 1; ++i) {
        candidates[i] = candidates[i + 1];
      }
      candidates.pop_back();
    }
    return candidates[random.Range(0, candidates.size() - 1)];
  }
  return random.Range(0, numChoices - 1);
}

int CUnpredictableRandomInt::Generate(CRandom16& random, int numChoices) {
  for (int i = 0; i < 9; ++i) {
    mHistory[i] = mHistory[i + 1];
  }
  rstl::reserved_vector< int, 16 > candidates;
  mCandidates = &candidates;
  int attempts = 0;
  bool rejected = true;
  int maxAttempts = rstl::min_val(numChoices, 50);
  while (rejected && attempts < maxAttempts) {
    rejected = false;
    ++attempts;
    int choice = SelectCandidate(random, numChoices, mHistory[9]);
    int repeatLimit = rstl::max_val(mMinRepeatLimit, GetConsecutiveRepeatLimit(numChoices, choice));
    mHistory[9] = choice;
    int distance = 2;
    int repeats = 1;
    while (mHistory[9] == mHistory[10 - distance]) {
      if (++repeats > repeatLimit) {
        rejected = true;
        break;
      }
      if (++distance > 10) {
        break;
      }
    }
    if (rejected) {
      continue;
    }
    int i;
    int maxOccurrences = 2 * GetChoiceProbabilityPercent(numChoices, choice) / 10;
    if (maxOccurrences < 10) {
      int occurrences = 1;
      for (i = 1; i < 10; ++i) {
        if (mHistory[9 - i] == mHistory[9]) {
          ++occurrences;
        }
      }
      if (occurrences > maxOccurrences) {
        rejected = true;
        continue;
      }
    }
    if (mHistory[9] == mHistory[8] && mHistory[7] == mHistory[6]) {
      rejected = true;
      continue;
    }
    if (numChoices > 3 && mHistory[9] == mHistory[7] && mHistory[8] == mHistory[6]) {
      rejected = true;
      continue;
    }
    if (numChoices > 5) {
      for (int i = 3; i < 7; ++i) {
        if ((mHistory[9] == mHistory[9 - i] && mHistory[8] == mHistory[8 - i] &&
             mHistory[7] == mHistory[7 - i]) ||
            (mHistory[9] == mHistory[7 - i] && mHistory[8] == mHistory[8 - i] &&
             mHistory[7] == mHistory[9 - i])) {
          rejected = true;
          break;
        }
      }
    }
  }
  return mHistory[9];
}

CUnpredictableRandomWeighted::CUnpredictableRandomWeighted(int count, const float* weights) {
  mWeights.reserve(count);
  for (int i = 0; i < count; ++i) {
    float weight = weights[i];
    mWeights.push_back(weight);
  }
  RecomputeTotalWeight();
}

int CUnpredictableRandomWeighted::GetChoiceProbabilityPercent(int numChoices, int choice) const {
  return rstl::min_val(int(100.f * mWeights[choice] / mTotalWeight), 99);
}

void CUnpredictableRandomWeighted::RecomputeTotalWeight() {
  mTotalWeight = 0.f;
  for (int i = 0; i < mWeights.size(); ++i) {
    mTotalWeight += mWeights[i];
  }
}

int CUnpredictableRandomWeighted::SelectCandidate(CRandom16& random, int numChoices,
                                                  int rejectedChoice) {
  rstl::reserved_vector< float, 16 >& candidates = *mCandidateWeights;
  if (numChoices <= 16) {
    if (candidates.empty()) {
      for (int i = 0; i < numChoices; ++i) {
        candidates.push_back(mWeights[i]);
      }
      mRemainingWeight = mTotalWeight;
    } else {
      mRemainingWeight -= candidates[rejectedChoice];
      candidates[rejectedChoice] = 0.f;
    }
    float threshold = mRemainingWeight * random.Float();
    float sum = 0.f;
    int i = 0;
    for (; i < candidates.size() - 1; ++i) {
      sum += candidates[i];
      if (sum >= threshold) {
        break;
      }
    }
    return i;
  }
  float threshold = mTotalWeight * random.Float();
  float sum = 0.f;
  int i = 0;
  for (; i < mWeights.size() - 1; ++i) {
    sum += mWeights[i];
    if (sum >= threshold) {
      break;
    }
  }
  return i;
}

int CUnpredictableRandomWeighted::Generate(CRandom16& random) {
  rstl::reserved_vector< float, 16 > candidates;
  mCandidateWeights = &candidates;
  return CUnpredictableRandomInt::Generate(random, mWeights.size());
}

CUnpredictableRandomFloat::CUnpredictableRandomFloat() {
  for (int i = 0; i < 5; ++i) {
    mHistory[i] = skInitialFloatHistory[i];
  }
  mInteger.SetMinRepeatLimit(2);
}

float CUnpredictableRandomFloat::Generate(CRandom16& random) {
  for (int i = 0; i < 4; ++i) {
    mHistory[i] = mHistory[i + 1];
  }
  mRejected = true;
  int attempts = 0;
  while (mRejected && attempts < 50) {
    mRejected = false;
    ++attempts;
    float bucket = float(mInteger.Generate(random, 10));
    float value = 0.1f * (bucket + random.Float());
    if (value > 1.f) {
      value = 1.f;
    }
    mHistory[4] = value;
    float d1 = mHistory[4] - mHistory[3];
    float d2 = mHistory[4] - mHistory[2];
    float d3 = mHistory[3] - mHistory[2];
    if (d1 <= 0.1f && d1 >= -0.1f && d2 <= 0.1f && d2 >= -0.1f && d3 <= 0.1f && d3 >= -0.1f) {
      mRejected = true;
    } else if (d1 <= 0.02f && d1 >= -0.02f) {
      mRejected = true;
    } else if ((mHistory[4] > mHistory[3] && mHistory[3] > mHistory[2] &&
                mHistory[2] > mHistory[1] && mHistory[1] > mHistory[0]) ||
               (mHistory[4] < mHistory[3] && mHistory[3] < mHistory[2] &&
                mHistory[2] < mHistory[1] && mHistory[1] < mHistory[0])) {
      mRejected = true;
    }
  }
  return mHistory[4];
}

