#include "Kyoto/Audio/CBitcrusher.hpp"
#include "Kyoto/Math/CMath.hpp"

#include "math.h"
#include "stdio.h"
#include "string.h"

CBitcrusher::CBitcrusher(AudioMasterCallback audioMaster)
: AudioEffectX(audioMaster, 1, 4)
, mDistortionType(0.f)
, mGain(1.f)
, mBitDepth(0.f)
, mSampleRateReduction(1.f) {
  mHeldSamples[1] = 0.f;
  mHeldSamples[0] = 0.f;
  mSampleCounter = 0;
  // The native constructor does not initialize the surround held sample.
  canMono(true);
  setNumInputs(2);
  setNumOutputs(2);
  setUniqueID('Bits');
  canProcessReplacing(true);
  strcpy(mProgramName, "Bitcrusher VST");
}

CBitcrusher::~CBitcrusher() {}

void CBitcrusher::setProgramName(char* name) { strcpy(mProgramName, name); }

void CBitcrusher::getProgramName(char* name) { strcpy(name, mProgramName); }

void CBitcrusher::setParameter(long index, float value) {
  switch (index) {
  case 0:
    mDistortionType = value;
    break;
  case 1:
    mGain = value;
    break;
  case 2:
    mBitDepth = value;
    break;
  case 3:
    mSampleRateReduction = value;
    break;
  }
}

float CBitcrusher::getParameter(long index) {
  float ret = 0.f;
  switch (index) {
  case 0:
    ret = mDistortionType;
    break;
  case 1:
    ret = mGain;
    break;
  case 2:
    ret = mBitDepth;
    break;
  case 3:
    ret = mSampleRateReduction;
    break;
  }
  return ret;
}

void CBitcrusher::getParameterName(long index, char* name) {
  switch (index) {
  case 0:
    strcpy(name, "Dist type");
    break;
  case 1:
    strcpy(name, "Gain");
    break;
  case 2:
    strcpy(name, "Bit-depth");
    break;
  case 3:
    strcpy(name, "Downsampling");
    break;
  }
}

// The native calls pass text as a stray trailing vararg.
void CBitcrusher::getParameterDisplay(long index, char* text) {
  switch (index) {
  case 0:
    if (mDistortionType < 0.33)
      sprintf(text, "%i", 1, text);
    if (mDistortionType >= 0.33 && mDistortionType <= 0.66)
      sprintf(text, "%i", 2, text);
    if (mDistortionType > 0.66)
      sprintf(text, "%i", 3, text);
    break;
  case 1:
    sprintf(text, "%f", 20.0 * static_cast< float >(log10(-16.f * mGain + 17.f)), text);
    break;
  case 2:
    sprintf(text, "%i", static_cast< int >(-23.f * mBitDepth + 24.f));
    break;
  case 3:
    sprintf(text, "%i", static_cast< int >(-39.f * mSampleRateReduction + 40.f));
    break;
  }
}

void CBitcrusher::getParameterLabel(long index, char* label) {
  switch (index) {
  case 0:
    strcpy(label, "");
    break;
  case 1:
    strcpy(label, "dB");
    break;
  case 2:
    strcpy(label, "Bits");
    break;
  case 3:
    strcpy(label, "x");
    break;
  }
}

void CBitcrusher::process(float** inputs, float** outputs, long sampleFrames) {}

void CBitcrusher::processReplacing(float** inputs, float** outputs, long sampleFrames) {
  float* in1 = inputs[0];
  float* in2 = inputs[1];
  float* in3 = inputs[2];
  float* out1 = outputs[0];
  float* out2 = outputs[1];
  float* out3 = outputs[2];
  const float quantization = powf(2.f, -23.f * mBitDepth + 24.f);
  const float reciprocal = 1.f / quantization;
  const float gain = -16.f * mGain + 17.f;

  // The middle distortion mode and accumulating callback are native no-ops.
  if (mDistortionType < 0.33) {
    while (--sampleFrames >= 0) {
      float left = gain * *in1++;
      float right = gain * *in2++;
      float surround = gain * *in3++;
      if (left > 1.0)
        left = 1.f;
      if (right > 1.0)
        right = 1.f;
      if (surround > 1.0)
        surround = 1.f;
      if (left < -1.0)
        left = -1.f;
      if (right < -1.0)
        right = -1.f;
      if (surround < -1.0)
        surround = -1.f;

      if (static_cast< float >(mSampleCounter) < -39.f * mSampleRateReduction + 39.f) {
        ++mSampleCounter;
      } else {
        mHeldSamples[0] = left;
        mHeldSamples[1] = right;
        mHeldSamples[2] = surround;
        mSampleCounter = 0;
      }
      *out1++ =
          reciprocal * static_cast< float >(static_cast< int >(mHeldSamples[0] * quantization));
      *out2++ =
          reciprocal * static_cast< float >(static_cast< int >(mHeldSamples[1] * quantization));
      *out3++ =
          reciprocal * static_cast< float >(static_cast< int >(mHeldSamples[2] * quantization));
    }
  }
  if (mDistortionType > 0.66) {
    while (--sampleFrames >= 0) {
      float left = gain * *in1++;
      float right = gain * *in2++;
      // The native surround path omits sine distortion.
      float surround = gain * *in3++;
      left = CMath::FastSinR(left);
      right = CMath::FastSinR(right);

      if (static_cast< float >(mSampleCounter) < -39.f * mSampleRateReduction + 39.f) {
        ++mSampleCounter;
      } else {
        mHeldSamples[0] = left;
        mHeldSamples[1] = right;
        mHeldSamples[2] = surround;
        mSampleCounter = 0;
      }
      *out1++ =
          reciprocal * static_cast< float >(static_cast< int >(mHeldSamples[0] * quantization));
      *out2++ =
          reciprocal * static_cast< float >(static_cast< int >(mHeldSamples[1] * quantization));
      *out3++ =
          reciprocal * static_cast< float >(static_cast< int >(mHeldSamples[2] * quantization));
    }
  }
}

