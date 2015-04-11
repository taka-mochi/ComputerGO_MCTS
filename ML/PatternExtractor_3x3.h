
#pragma once

#include "common.h"

namespace Go {
  class Board;
}

namespace ML {

  class PatternExtractor_3x3 {
  public:
    enum RotationIndex {
      INDEX_ROT_0, INDEX_ROT_90, INDEX_ROT_180, INDEX_ROT_270
    };
    static const PATTERN_3x3_HASH_VALUE INVALID_HASH_VALUE;

  private:
    static const PATTERN_3x3_HASH_VALUE VALUE_FOR_ROT[4];
    static const PATTERN_3x3_HASH_VALUE ROT_MASK;

    static const int BIT_OFFSET_FOR_PATTERN;
    static const int BIT_COUNT_FOR_ONE_POINT;
    static const PATTERN_3x3_HASH_VALUE PATTERN_MASK;

    static const int BIT_OFFSET_FOR_BORDER;
    static const PATTERN_3x3_HASH_VALUE BORDER_MASK;

    static const unsigned int BIT_SIZE_FOR_PATTERN;

    static const int PATTERN_BIT_ORDERING_IN_ROT_0_VIEW[4][8];
    static const int BORDER_BIT_ORDERING_IN_ROT_0_VIEW[4][4];

    static std::vector<PATTERN_3x3_HASH_VALUE> preComputedHashes;
    // a mapping from non-reversed pattern hashes to reversed pattern hases
    static std::vector<PATTERN_3x3_HASH_VALUE> preComputedColorReversedHashes;

    int m_boardSize;
    int m_encodeOrderArray[4][8];
    int m_dirs[4][4];

    static void preComputeHashes();
    static PATTERN_3x3_HASH_VALUE encode_pattern(Common::Color pattern[8]);
    PATTERN_3x3_HASH_VALUE encode_for_one_rot(const Go::Board &board, Common::Point centerPoint, RotationIndex rot_index) const;
    static void decode_pattern(PATTERN_3x3_HASH_VALUE value, Common::Color result_pattern[8]);
  public:
    explicit PatternExtractor_3x3(int board_size);
    PATTERN_3x3_HASH_VALUE encode(const Go::Board &board, Common::Point centerPoint, bool doFlipColor) const;
    static void decode(PATTERN_3x3_HASH_VALUE h, Common::Color pattern[8], bool &border_left, bool &border_right, bool &border_up, bool &border_down);

    static const std::vector<PATTERN_3x3_HASH_VALUE> &getAllPossiblePatterns() {return preComputedHashes;}

    static PATTERN_3x3_HASH_VALUE calcHashValueForRotatedPatternButInRot0RotationEncoding(PATTERN_3x3_HASH_VALUE original, RotationIndex index);
    static PATTERN_3x3_HASH_VALUE calcHashValueForFlipedColor(PATTERN_3x3_HASH_VALUE original);
    static PATTERN_3x3_HASH_VALUE calcHashValueForFlipedLeftRight(PATTERN_3x3_HASH_VALUE value);
  };
}
