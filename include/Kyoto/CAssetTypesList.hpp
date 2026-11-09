#ifndef KYOTO_CASSETTYPESLIST_HPP
#define KYOTO_CASSETTYPESLIST_HPP

class CAssetTypesList {
public:
  static const unsigned int kInvalid = 0xffffffff;

  // Guessed name
  static unsigned int GetIndexForFourCC(unsigned int fourCC);
  static unsigned int GetFourCCForIndex(unsigned int index);
};

#endif
