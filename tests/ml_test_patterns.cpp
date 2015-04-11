
#include "precomp.h"
#include "gtest/gtest.h"

#include "ML/PatternExtractor_3x3.h"

using namespace std;
using namespace Common;
using namespace Go;
using namespace ML;


TEST(PatternExtractor, hashForRotatedPatterns) {
  // int init_array[] = {
  //   3,3,3,3,3,3,3,3,3,3,3,
  //   3,0,1,1,1,0,0,0,0,0,3,
  //   3,1,2,2,1,0,0,0,2,0,3,
  //   3,0,1,2,2,1,0,0,0,0,3,
  //   3,0,1,2,0,0,0,0,0,0,3,
  //   3,0,0,1,0,1,0,0,0,0,3,
  //   3,0,0,1,0,0,0,0,0,0,3,
  //   3,0,0,0,0,0,0,2,1,1,3,
  //   3,0,0,0,2,0,0,0,0,2,3,
  //   3,0,0,0,0,0,0,0,0,0,3,
  //   3,3,3,3,3,3,3,3,3,3,3
  // };
  // extracting pattern from (1,4) (no flip) 
  // b,0,1,2,
  // b,0,0,1,
  // b,0,0,1,
  // expected hash (in 2bit/16bit)
  // border bit order: DOWN RIGHT LEFT UP
  // rot0  : 00100001100001000001/0x21841
  // rot90 : 10001001010100000000/0x89500
  // rot180: 01000100000100100100/0x44124
  // rot270: 00010000000001010110/0x10056
  //  min is rot270: 0x10056
  PATTERN_3x3_HASH_VALUE original = 0x21841;

  PATTERN_3x3_HASH_VALUE val90 = PatternExtractor_3x3::calcHashValueForRotatedPatternButInRot0RotationEncoding(original, PatternExtractor_3x3::INDEX_ROT_90);
  EXPECT_EQ(static_cast<PATTERN_3x3_HASH_VALUE>(0x10056), val90);
  PATTERN_3x3_HASH_VALUE val180 = PatternExtractor_3x3::calcHashValueForRotatedPatternButInRot0RotationEncoding(original, PatternExtractor_3x3::INDEX_ROT_180);
  EXPECT_EQ(static_cast<PATTERN_3x3_HASH_VALUE>(0x44124), val180);
  PATTERN_3x3_HASH_VALUE val270 = PatternExtractor_3x3::calcHashValueForRotatedPatternButInRot0RotationEncoding(original, PatternExtractor_3x3::INDEX_ROT_270);
  EXPECT_EQ(static_cast<PATTERN_3x3_HASH_VALUE>(0x89500), val270);
}

TEST(PatternExtractor, hashForFlipedPatterns) {
  // b,0,1,2,
  // b,0,0,1,
  // b,3,3,3,
  // b,b,b,b,
  // expected hash (in 2bit/16bit)
  // border bit order: DOWN RIGHT LEFT UP
  // rot0  : 10100001100001111111/0xA187F => flip color 10100010010010111111/0xA24BF
  // rot90 : 11001001110111000011/0xC9DC3 => flip color 11000110111011000011/0xC6EC3
  // rot180: 01011111110100100100/0x5FD24 => flip color 01011111111000011000/0x5FE18
  // rot270: 00111100001101110110/0x3C376 => flip color 00111100001110111001/0x3C3B9
  PATTERN_3x3_HASH_VALUE originals [] = {
    0xA187F, 0xC9DC3, 0x5FD24, 0x3C376
  };
  PATTERN_3x3_HASH_VALUE fliped [] = {
    0xA24BF, 0xC6EC3, 0x5FE18, 0x3C3B9
  };
  
  for (int i=0; i<4; i++) {
    EXPECT_EQ(fliped[i], PatternExtractor_3x3::calcHashValueForFlipedColor(originals[i]));
  }
}

TEST(PatternExtractor, hashForFlipedLeftRightPatterns) {
  // b,0,1,2,
  // b,0,0,1,
  // b,3,3,3,
  // b,b,b,b,
  // expected hash (in 2bit/16bit)
  // border bit order: DOWN RIGHT LEFT UP
  // rot0  : 10100001100001111111/0xA187F => flip lr 11001001000100111111/0xC913F
  // rot90 : 11001001110111000011/0xC9DC3 => flip lr 10101101101101110000/0xADB70
  // rot180: 01011111110100100100/0x5FD24 => flip lr 00111111110001000110/0x3FC46
  // rot270: 00111100001101110110/0x3C376 => flip lr 01010000110111100111/0x50DE7
  PATTERN_3x3_HASH_VALUE originals [] = {
    0xA187F, 0xC9DC3, 0x5FD24, 0x3C376
  };
  PATTERN_3x3_HASH_VALUE fliped [] = {
    0xC913F, 0xADB70, 0x3FC46, 0x50DE7
  };
  
  for (int i=0; i<4; i++) {
    if (fliped[i] != PatternExtractor_3x3::calcHashValueForFlipedLeftRight(originals[i])) {
      printf("orig = 0x%X, calced = 0x%X\n", fliped[i], PatternExtractor_3x3::calcHashValueForFlipedLeftRight(originals[i]));
    }
    EXPECT_EQ(fliped[i], PatternExtractor_3x3::calcHashValueForFlipedLeftRight(originals[i]));
  }
}

TEST(PatternExtractor, noBorderPattern) {

  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,1,1,1,0,0,0,0,0,3,
    3,1,2,2,1,0,0,0,2,0,3,
    3,0,1,2,2,1,0,0,0,0,3,
    3,0,1,2,0,0,0,0,0,0,3,
    3,0,0,1,0,1,0,0,0,0,3,
    3,0,0,1,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,2,0,0,0,1,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  // extracting pattern from (3,3) 
  // 2,2,1,
  // 2,0,0,
  // 1,0,1,
  // expected hash (in 2bit/16bit)
  // rot0  : 1010011000010001        -> flip -> 0110100010010001
  // rot90 : 0100011000101001        -> flip -> 0100010010011010
  // rot180: 0100010010011010/0x449A -> flip -> 0100011000101001/0x4629
  // rot270: 0110100010010001        -> flip -> 1010011000010001
  //  min is rot180: 0x449A

  Board board(9, init_array);
  PatternExtractor_3x3 extractor(board.getSize());

  PATTERN_3x3_HASH_VALUE val = extractor.encode(board, board.xyToPoint(3,3), false);
  //printf("val = 0x%x\n", val);
  EXPECT_EQ(static_cast<PATTERN_3x3_HASH_VALUE>(0x449A), val);

  // extracting pattern (flipped) from (3,6) 
  // 2,0,0,
  // 0,0,0,
  // 0,1,0,
  // expected hash (in 2bit/16bit)
  // rot0  : 1000000000000100      -> flip -> 0000100000000100
  // rot90 : 0000000001100000/0x60 -> flip -> 0000000100000010
  // rot180: 0001000000000010      -> flip -> 0001000000100000
  // rot270: 0000100100000000      -> flip -> 1000000001000000
  //  min is rot90: 0x60
  val = extractor.encode(board, board.xyToPoint(3,6), true);
  //printf("val = 0x%x\n", val);
  EXPECT_EQ(static_cast<PATTERN_3x3_HASH_VALUE>(0x60), val);
}

TEST(PatternExtractor, withBorderPattern) {

  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,1,1,1,0,0,0,0,0,3,
    3,1,2,2,1,0,0,0,0,0,3,
    3,0,1,2,2,1,0,0,0,0,3,
    3,0,1,2,0,0,0,0,0,0,3,
    3,0,0,1,0,1,0,0,0,0,3,
    3,0,0,1,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,2,1,1,3,
    3,0,0,0,2,0,0,0,0,2,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  // extracting pattern from (1,4) (no flip) 
  // b,0,1,2,
  // b,0,0,1,
  // b,0,0,1,
  // expected hash (in 2bit/16bit)
  // border bit order: DOWN RIGHT LEFT UP
  // rot0  : 00100001100001000001/
  // rot90 : 10001001010100000000/
  // rot180: 01000100000100100100/
  // rot270: 00010000000001010110/0x10056 -> flip -> 00010000000100100101
  //  min is rot270: 0x10056

  Board board(9, init_array);
  PatternExtractor_3x3 extractor(board.getSize());

  PATTERN_3x3_HASH_VALUE val = extractor.encode(board, board.xyToPoint(1,4), false);
  //printf("with border val = 0x%x\n", val);
  EXPECT_EQ(static_cast<PATTERN_3x3_HASH_VALUE>(0x10056), val);

  // extracting pattern from (7,7)
  // 2,1,1,b
  // 0,0,2,b
  // 0,0,0,b
  // b,b,b,b
  // expected hash (in 2bit/16bit)
  // border bit order: DOWN RIGHT LEFT UP
  // rot0  : 11001001010010000000         -> flip -> 1010...
  // rot90 : 01010110000100100000         -> flip -> 00110010010001000010
  // rot180: 00110000001000010110/0x30216 -> flip -> 01010000000010100101/0x500A5
  // rot270: 10100000100001001001         -> flip -> 1100...
  //  min is rot180: 0x30216
  val = extractor.encode(board, board.xyToPoint(7,7), false);
  //printf("with border val = 0x%x\n", val);
  EXPECT_EQ(static_cast<PATTERN_3x3_HASH_VALUE>(0x30216), val);

  // extracting pattern (flipped) from (7,0)
  // b,b,b,b
  // 3,3,3,b
  // 0,0,0,b
  // 0,0,0,b
  // expected hash (in 2bit/16bit)
  // border bit order: DOWN RIGHT LEFT UP
  // rot0  : 01011111110000000000         -> flip -> 00111111110000000000
  // rot90 : 00111100001100110000/0x3C330 -> flip -> 0101...
  // rot180: 10100000000000111111         -> flip -> 1100...
  // rot270: 11000000110011000011         -> flip -> 1010...
  //  min is rot90:0x3C330
  val = extractor.encode(board, board.xyToPoint(7,0), true);
  //printf("with border val = 0x%x\n", val);
  EXPECT_EQ(static_cast<PATTERN_3x3_HASH_VALUE>(0x3C330), val);

  // extracting pattern (flipped) from (4,0)
  // b,b,b,
  // 3,3,3,
  // 2,0,0,
  // 2,0,0,
  // expected hash (in 2bit/16bit)
  // border bit order: DOWN RIGHT LEFT UP
  // rot0  : 00011111111000100000/0x1fe20 -> flip -> 00011111110010000010/0x1fc82
  // rot90 : 00101100001100111010         -> flip -> 01001100001100111010/0x4c33A
  // rot180: 10000000100010111111         -> flip -> 10001000001000111111
  // rot270: 01001010110011000011         -> flip -> 00101110101100110000
  //  min is rot0:0x1fc12
  val = extractor.encode(board, board.xyToPoint(4,0), true);
  //printf("with border val = 0x%x\n", val);
  EXPECT_EQ(static_cast<PATTERN_3x3_HASH_VALUE>(0x1fc82), val);

  // extracting pattern from (8,8)
  // 0,2,3,b
  // 0,0,3,b
  // 3,3,3,b
  // b,b,b,b
  // expected hash (in 2bit/16bit)
  // border bit order: DOWN RIGHT LEFT UP
  // rot0  : 11000010110011111111         -> flip -> 1010...
  // rot90 : 01011111111011000011         -> flip -> 00111111111110110000/0x3ffB0
  // rot180: 00111111111100111000/0x3ff38 -> flip -> 0101...
  // rot270: 10101100001110111111         -> flip -> 1100...
  //  min is rot180:0xcff38
  val = extractor.encode(board, board.xyToPoint(8,8), false);
  //printf("with border val = 0x%x\n", val);
  EXPECT_EQ(static_cast<PATTERN_3x3_HASH_VALUE>(0x3ff38), val);

}

TEST(PatternExtractor, decoding) {

  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,1,1,1,0,0,0,0,0,3,
    3,1,2,2,1,0,0,0,0,0,3,
    3,0,1,2,2,1,0,0,0,0,3,
    3,0,1,2,0,0,0,0,0,0,3,
    3,0,0,1,0,1,0,0,0,0,3,
    3,0,0,1,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,2,1,1,3,
    3,0,0,0,2,0,0,0,0,2,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  // extracting pattern from (1,4) (no flip) 
  // b,0,1,2,
  // b,0,0,1,
  // b,0,0,1,
  // expected hash (in 2bit/16bit)
  // border bit order: DOWN RIGHT LEFT UP
  // rot0  : 00100001100001000001/
  // rot90 : 10001001010100000000/
  // rot180: 01000100000100100100/
  // rot270: 00010000000001010110/0x10056 => flipped 00010000000010101001/0x100A9
  //  min is rot270: 0x10056

  Board board(9, init_array);
  PatternExtractor_3x3 extractor(board.getSize());

  PATTERN_3x3_HASH_VALUE val = extractor.encode(board, board.xyToPoint(1,4), false);
  //printf("with border val = 0x%x\n", val);
  Color pattern[8];
  bool border_left, border_right, border_up, border_down;
  extractor.decode(val, pattern, border_left, border_right, border_up, border_down);
  EXPECT_TRUE(border_up);
  EXPECT_FALSE(border_right | border_left | border_down);
  EXPECT_EQ(0, pattern[0]); EXPECT_EQ(0, pattern[1]); EXPECT_EQ(0, pattern[2]); EXPECT_EQ(0, pattern[3]);
  EXPECT_EQ(1, pattern[4]); EXPECT_EQ(1, pattern[5]); EXPECT_EQ(1, pattern[6]); EXPECT_EQ(2, pattern[7]);

  // extracting pattern (flipped) from (7,0)
  // b,b,b,b
  // 3,3,3,b
  // 0,0,0,b
  // 0,0,0,b
  // expected hash (in 2bit/16bit)
  // border bit order: DOWN RIGHT LEFT UP
  // rot0  : 01011111110000000000
  // rot90 : 00111100001100110000/0x3C330
  // rot180: 10100000000000111111
  // rot270: 11000000110011000011
  //  min is rot90:0x3C330
  val = extractor.encode(board, board.xyToPoint(7,0), true);
  //printf("with border val = 0x%x\n", val);
  extractor.decode(val, pattern, border_left, border_right, border_up, border_down);
  EXPECT_TRUE(border_left & border_up);
  EXPECT_FALSE(border_right | border_down);
  EXPECT_EQ(3, pattern[0]); EXPECT_EQ(0, pattern[1]); EXPECT_EQ(0, pattern[2]); EXPECT_EQ(3, pattern[3]);
  EXPECT_EQ(0, pattern[4]); EXPECT_EQ(3, pattern[5]); EXPECT_EQ(0, pattern[6]); EXPECT_EQ(0, pattern[7]);

}
