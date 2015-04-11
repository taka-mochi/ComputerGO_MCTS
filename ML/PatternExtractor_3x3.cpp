
#include "precomp.h"
#include "ML/PatternExtractor_3x3.h"
#include <iostream>

using namespace std;
using namespace Common;
using namespace Go;

namespace ML {

  const PATTERN_3x3_HASH_VALUE PatternExtractor_3x3::INVALID_HASH_VALUE = static_cast<PATTERN_3x3_HASH_VALUE>(-1);
  const PATTERN_3x3_HASH_VALUE PatternExtractor_3x3::VALUE_FOR_ROT[4] = {
    0x0, 0x01, 0x02, 0x03
  };
  const PATTERN_3x3_HASH_VALUE PatternExtractor_3x3::ROT_MASK = 0x3;
  
  const int PatternExtractor_3x3::BIT_OFFSET_FOR_PATTERN = 0;
  const int PatternExtractor_3x3::BIT_COUNT_FOR_ONE_POINT = 2;
  const PATTERN_3x3_HASH_VALUE PatternExtractor_3x3::PATTERN_MASK = 0x0ffff;

  const int PatternExtractor_3x3::BIT_OFFSET_FOR_BORDER = 16;
  const PATTERN_3x3_HASH_VALUE PatternExtractor_3x3::BORDER_MASK = 0xf0000;

  const unsigned int PatternExtractor_3x3::BIT_SIZE_FOR_PATTERN = 20;

  std::vector<PATTERN_3x3_HASH_VALUE> PatternExtractor_3x3::preComputedHashes;
  std::vector<PATTERN_3x3_HASH_VALUE> PatternExtractor_3x3::preComputedColorReversedHashes;

  enum Direction {
    UP, LEFT, RIGHT, DOWN
  };

  // Pattern Ordering
  // origin  090    180    270
  // 0 1 2  5 3 0  7 6 5  2 4 7
  // 3   4  6   1  4   3  1   6
  // 5 6 7  7 4 2  2 1 0  0 3 5
  const int PatternExtractor_3x3::PATTERN_BIT_ORDERING_IN_ROT_0_VIEW[4][8] = {
    {0,1,2,3,4,5,6,7},
    {5,3,0,6,1,7,4,2},
    {7,6,5,4,3,2,1,0},
    {2,4,7,1,6,0,3,5}
  };
  
  // Border Ordering in Rot 0 view
  //   0: DOWN RIGHT LEFT UP
  //  90: RIGHT UP DOWN LEFT
  // 180: UP LEFT RIGHT DOWN
  // 270: LEFT DOWN UP RIGHT
  const int PatternExtractor_3x3::BORDER_BIT_ORDERING_IN_ROT_0_VIEW[4][4] = {
    {DOWN, RIGHT, LEFT, UP},
    {RIGHT, UP, DOWN, LEFT},
    {UP, LEFT, RIGHT, DOWN},
    {LEFT, DOWN, UP, RIGHT}
  };


  PatternExtractor_3x3::PatternExtractor_3x3(int board_size)
    : m_boardSize(board_size)
  {
    m_dirs[INDEX_ROT_0][UP] =
      m_dirs[INDEX_ROT_90][LEFT] =
      m_dirs[INDEX_ROT_180][DOWN] =
      m_dirs[INDEX_ROT_270][RIGHT] = -(board_size+2);

    m_dirs[INDEX_ROT_0][DOWN] =
      m_dirs[INDEX_ROT_90][RIGHT] =
      m_dirs[INDEX_ROT_180][UP] =
      m_dirs[INDEX_ROT_270][LEFT] = +(board_size+2);

    m_dirs[INDEX_ROT_0][LEFT] =
      m_dirs[INDEX_ROT_90][DOWN] =
      m_dirs[INDEX_ROT_180][RIGHT] =
      m_dirs[INDEX_ROT_270][UP] = -1;

    m_dirs[INDEX_ROT_0][RIGHT] =
      m_dirs[INDEX_ROT_90][UP] =
      m_dirs[INDEX_ROT_180][LEFT] =
      m_dirs[INDEX_ROT_270][DOWN] = +1;

    for (int index=INDEX_ROT_0; index<=INDEX_ROT_270; index += 1) {
      m_encodeOrderArray[index][0] = m_dirs[index][UP] + m_dirs[index][LEFT];
      m_encodeOrderArray[index][1] = m_dirs[index][UP];
      m_encodeOrderArray[index][2] = m_dirs[index][UP] + m_dirs[index][RIGHT];
      m_encodeOrderArray[index][3] = m_dirs[index][LEFT];
      //m_encodeOrderArray[index][4] = 0;
      m_encodeOrderArray[index][4] = m_dirs[index][RIGHT];
      m_encodeOrderArray[index][5] = m_dirs[index][DOWN] + m_dirs[index][LEFT];
      m_encodeOrderArray[index][6] = m_dirs[index][DOWN];
      m_encodeOrderArray[index][7] = m_dirs[index][DOWN] + m_dirs[index][RIGHT];

      // cout << index*90 << endl;
      // for (int k=0; k<9; k++) {
      //   cout << m_encodeOrderArray[index][k] << ",";
      //   if ((k+1)%3 == 0) {
      //     cout << endl;
      //   }
      // }
    }

    if (preComputedHashes.size() == 0) {
      preComputeHashes();
    }

  }

  PATTERN_3x3_HASH_VALUE PatternExtractor_3x3::calcHashValueForRotatedPatternButInRot0RotationEncoding(PATTERN_3x3_HASH_VALUE original, RotationIndex index) {

    PATTERN_3x3_HASH_VALUE newhash = 0;

    // pattern ordering
    for (int i=0; i<8; i++) {
      PATTERN_3x3_HASH_VALUE value = (original >> (BIT_COUNT_FOR_ONE_POINT*PATTERN_BIT_ORDERING_IN_ROT_0_VIEW[index][i] + BIT_OFFSET_FOR_PATTERN)) & (0x3);
      newhash |= value << (BIT_COUNT_FOR_ONE_POINT*PATTERN_BIT_ORDERING_IN_ROT_0_VIEW[INDEX_ROT_0][i] + BIT_OFFSET_FOR_PATTERN);
    }

    // border ordering
    // if BORDER_BIT_ORDERING_IN_ROT_0_VIEW[index][i] is 1, 
    //  BORDER_BIT_ORDERING_IN_ROT_0_VIEW[INDEX_ROT_0] is 1
    // ROT_0 視点で index_rot を encode するとき、 BORDER_BIT_...0_VIEW[index][i] を参照して
    // [ROT_0][i] を設定する
    for (int i=0; i<4; i++) {
      PATTERN_3x3_HASH_VALUE value = (original >> (BORDER_BIT_ORDERING_IN_ROT_0_VIEW[index][i] + BIT_OFFSET_FOR_BORDER)) & (0x1);
      newhash |= value << (BORDER_BIT_ORDERING_IN_ROT_0_VIEW[INDEX_ROT_0][i] + BIT_OFFSET_FOR_BORDER);
    }

    return newhash;
  }

  PATTERN_3x3_HASH_VALUE PatternExtractor_3x3::calcHashValueForFlipedColor(PATTERN_3x3_HASH_VALUE value) {
    Color c[8];
    decode_pattern(value, c);
//    cerr << "normal " << c[0] << "," << c[1] << "," << c[2] << "," << c[3] << "," << c[4] << "," << c[5] << "," << c[6] << "," << c[7] << endl;
    for (int i=0; i<8; i++) {
      if (c[i] == BLACK) c[i] = WHITE;
      else if (c[i] == WHITE) c[i] = BLACK;
    }
//    cerr << "fliped " << c[0] << "," << c[1] << "," << c[2] << "," << c[3] << "," << c[4] << "," << c[5] << "," << c[6] << "," << c[7] << endl;
    PATTERN_3x3_HASH_VALUE flipedValue = encode_pattern(c);

    return (value & (~PATTERN_MASK)) | (flipedValue & PATTERN_MASK);
  }

  PATTERN_3x3_HASH_VALUE PatternExtractor_3x3::calcHashValueForFlipedLeftRight(PATTERN_3x3_HASH_VALUE value) {
    Color c[8];
    decode_pattern(value, c);
//    cerr << "normal " << c[0] << "," << c[1] << "," << c[2] << "," << c[3] << "," << c[4] << "," << c[5] << "," << c[6] << "," << c[7] << endl;
    std::swap(c[0], c[2]);
    std::swap(c[3], c[4]);
    std::swap(c[5], c[7]);
//    cerr << "fliped " << c[0] << "," << c[1] << "," << c[2] << "," << c[3] << "," << c[4] << "," << c[5] << "," << c[6] << "," << c[7] << endl;
    bool left = value & ( 1 << (BIT_OFFSET_FOR_BORDER + LEFT)) ? true : false;
    bool right = value & ( 1 << (BIT_OFFSET_FOR_BORDER + RIGHT)) ? true : false;
    if (left != right) {
      value ^= (1 << (BIT_OFFSET_FOR_BORDER + LEFT)) | (1 << (BIT_OFFSET_FOR_BORDER + RIGHT));
    }
    PATTERN_3x3_HASH_VALUE flipedValue = encode_pattern(c);

    return (value & (~PATTERN_MASK)) | (flipedValue & PATTERN_MASK);
  }

  void PatternExtractor_3x3::preComputeHashes() {
    cerr << "------------------------------- begin pattern precomputation" << endl;
    unsigned int size = 1;
    for (unsigned int i=0; i<BIT_SIZE_FOR_PATTERN; i++) size *= 2;
    preComputedHashes.resize(size, INVALID_HASH_VALUE);
    preComputedColorReversedHashes.resize(size, INVALID_HASH_VALUE);

    // all BORDER patterns
    PATTERN_3x3_HASH_VALUE border_patterns[9] = {
      0x0, // no border
      // one border
      1 << (UP + BIT_OFFSET_FOR_BORDER),
      1 << (LEFT + BIT_OFFSET_FOR_BORDER),
      1 << (RIGHT + BIT_OFFSET_FOR_BORDER),
      1 << (DOWN + BIT_OFFSET_FOR_BORDER),
      (1 << (UP + BIT_OFFSET_FOR_BORDER)) | (1 << (LEFT + BIT_OFFSET_FOR_BORDER)),
      (1 << (UP + BIT_OFFSET_FOR_BORDER)) | (1 << (RIGHT + BIT_OFFSET_FOR_BORDER)),
      (1 << (DOWN + BIT_OFFSET_FOR_BORDER)) | (1 << (LEFT + BIT_OFFSET_FOR_BORDER)),
      (1 << (DOWN + BIT_OFFSET_FOR_BORDER)) | (1 << (RIGHT + BIT_OFFSET_FOR_BORDER))
    };

    Color init_patterns[9][4][8] = {
      {{0,0,0,0,0,0,0,0},{-1,-1,-1,-1,-1,-1,-1,-1},
       {-1,-1,-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1,-1,-1}},
      {{0,0,0,0,0,0,0,0},{WALL,WALL,WALL,0,0,0,0,0},
       {-1,-1,-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1,-1,-1}},
      {{0,0,0,0,0,0,0,0},{WALL,0,0,WALL,0,WALL,0,0},
       {-1,-1,-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1,-1,-1}},
      {{0,0,0,0,0,0,0,0},{0,0,WALL,0,WALL,0,0,WALL},
       {-1,-1,-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1,-1,-1}},
      {{0,0,0,0,0,0,0,0},{0,0,0,0,0,WALL,WALL,WALL},
       {-1,-1,-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1,-1,-1}},
      {{0,0,0,0,0,0,0,0},{WALL,WALL,WALL,WALL,0,WALL,0,0},
       {WALL,WALL,WALL,0,0,0,0,0},{WALL,0,0,WALL,0,WALL,0,0}},
      {{0,0,0,0,0,0,0,0},{WALL,WALL,WALL,0,WALL,0,0,WALL},
       {WALL,WALL,WALL,0,0,0,0,0},{0,0,WALL,0,WALL,0,0,WALL}},
      {{0,0,0,0,0,0,0,0},{WALL,0,0,WALL,0,WALL,WALL,WALL},
       {WALL,0,0,WALL,0,WALL,0,0},{0,0,0,0,0,WALL,WALL,WALL}},
      {{0,0,0,0,0,0,0,0},{0,0,WALL,0,WALL,WALL,WALL,WALL},
       {0,0,WALL,0,WALL,0,0,WALL},{0,0,0,0,0,WALL,WALL,WALL}}
    };

    int possible_patterns = 0;

    Color patterns[8] = {0};

    for (int border_index=0; border_index<9; border_index++) {
      for (int init_index=0; init_index<4; init_index++) {
        if (init_patterns[border_index][init_index][0]==-1) continue;
        for (int i=0; i<8; i++) patterns[i] = init_patterns[border_index][init_index][i];

        while (true) {
          PATTERN_3x3_HASH_VALUE value = border_patterns[border_index];
          
          // generate hash
          value |= encode_pattern(patterns);
          PATTERN_3x3_HASH_VALUE min_value = value, rot_values[4], flipped_values[4], reversed_values[4], reversed_flipped_values[4], reversed_min = INVALID_HASH_VALUE;
          rot_values[0] = value;
          
          // calc for other rotation
          for (int rot_index = INDEX_ROT_0; rot_index <= INDEX_ROT_270; rot_index++) {
            rot_values[rot_index] = calcHashValueForRotatedPatternButInRot0RotationEncoding(value, static_cast<RotationIndex>(rot_index));
            flipped_values[rot_index] = calcHashValueForFlipedLeftRight(rot_values[rot_index]);
            if (rot_values[rot_index] < min_value) {
              min_value = rot_values[rot_index];
            }
            if (flipped_values[rot_index] < min_value) {
              min_value = flipped_values[rot_index];
            }

            reversed_values[rot_index] = calcHashValueForFlipedColor(rot_values[rot_index]);
            reversed_flipped_values[rot_index] = calcHashValueForFlipedLeftRight(reversed_values[rot_index]);
            if (reversed_values[rot_index] < reversed_min) {
              reversed_min = reversed_values[rot_index];
            }
            if (reversed_flipped_values[rot_index] < reversed_min) {
              reversed_min = reversed_flipped_values[rot_index];
            }
          }
          
          // register
          for (int i=0; i<4; i++) {
            if (preComputedHashes[rot_values[i]] == INVALID_HASH_VALUE) {
              possible_patterns += 2;
              preComputedHashes[rot_values[i]] = min_value;
              preComputedHashes[flipped_values[i]] = min_value;
            } else {
              assert(preComputedHashes[rot_values[i]]==min_value);
              assert(preComputedHashes[flipped_values[i]]==min_value);
            }

            if (preComputedColorReversedHashes[rot_values[i]] == INVALID_HASH_VALUE) {
              //possible_patterns++;
              preComputedColorReversedHashes[rot_values[i]] = reversed_min;
              preComputedColorReversedHashes[flipped_values[i]] = reversed_min;
            } else {
              assert(preComputedColorReversedHashes[rot_values[i]]==reversed_min);
              assert(preComputedColorReversedHashes[flipped_values[i]]==reversed_min);
            }
          }
          
          // get next pattern
          int index = 0;
          while (index < 8) {
            if (patterns[index] == WALL) {
              index++;
            } else {
              patterns[index]++;
              if (patterns[index] > 2) {
                patterns[index] = 0;
                index++;
              } else {
                break;
              }
            }
          }
          if (index >= 8 ) break;
        }
      }
    }
    cerr << "---- possible pattern count = " << possible_patterns << endl;
  }

  PATTERN_3x3_HASH_VALUE PatternExtractor_3x3::encode_pattern(Color pattern[8]) {
    PATTERN_3x3_HASH_VALUE value = 0;
    for (int i=0; i<8; i++) {
      value |= pattern[i] << ((8-i-1)*BIT_COUNT_FOR_ONE_POINT + BIT_OFFSET_FOR_PATTERN);
    }
    return value;
  };

  void PatternExtractor_3x3::decode_pattern(PATTERN_3x3_HASH_VALUE value, Color result_pattern[8]) {
    for (int i=0; i<8; i++) {
      result_pattern[i] = (value >> ((8-i-1)*BIT_COUNT_FOR_ONE_POINT + BIT_OFFSET_FOR_PATTERN)) & 0x3;
    }
  }

  PATTERN_3x3_HASH_VALUE PatternExtractor_3x3::encode_for_one_rot(const Board &board, Common::Point centerPoint, RotationIndex rot_index) const {
    PATTERN_3x3_HASH_VALUE value = 0;//VALUE_FOR_ROT[rot_index];
    // rot をHASHに入れてると、違う回転で同じパターン、というのを別個に扱ってしまう

    // stones encoding
    for (int i=0; i<8; i++) {
      assert(rot_index>=0&&rot_index<=3);
      int look_index_offset = m_encodeOrderArray[rot_index][i];
      Color stone = board.getStone(centerPoint + look_index_offset);
      //if (doFlipColor && (stone == BLACK || stone == WHITE)) {
      //  stone = Board::flipColor(stone);
      //}
      // stone = [0,3]
      int offset_bits = BIT_OFFSET_FOR_PATTERN + BIT_COUNT_FOR_ONE_POINT*(8-i-1);
      value |= stone<<offset_bits;
    }

    // border encoding
    for (int dir=0; dir<4; dir++) {
      if (board.isColor(centerPoint+m_dirs[rot_index][dir], WALL) ||
          board.isColor(centerPoint+m_dirs[rot_index][dir]*2, WALL)) {
        value |= (1<<dir) << BIT_OFFSET_FOR_BORDER;
      }
    }
    return value;
  }

  PATTERN_3x3_HASH_VALUE PatternExtractor_3x3::encode(const Board &board, Common::Point centerPoint, bool doFlipColor) const {
    assert(board.isColor(centerPoint, FREE));

    PATTERN_3x3_HASH_VALUE value = encode_for_one_rot(board, centerPoint, INDEX_ROT_0);
    return doFlipColor == false ? preComputedHashes[value] : preComputedColorReversedHashes[value];

    //
    // TODO: consider whether a last move is in this pattern or not
    //     I think it is OK that the feature is independently taken into account
    //      => [a feature when distance from the last move is 2 or less]

    // TODO: consider atari
    //     This is same as the last move above?
    //     Independent feature is enough I think

  }

  void PatternExtractor_3x3::decode(PATTERN_3x3_HASH_VALUE h, Common::Color pattern[8], bool &border_left, bool &border_right, bool &border_up, bool &border_down) {
    decode_pattern(h, pattern);

    border_up = h & (1<<(UP+BIT_OFFSET_FOR_BORDER)) ? true : false;
    border_down = h & (1<<(DOWN+BIT_OFFSET_FOR_BORDER)) ? true : false;
    border_left = h & (1<<(LEFT+BIT_OFFSET_FOR_BORDER)) ? true : false;
    border_right = h & (1<<(RIGHT+BIT_OFFSET_FOR_BORDER)) ? true : false;
  }
}
