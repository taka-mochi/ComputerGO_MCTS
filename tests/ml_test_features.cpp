#include "../precomp.h"

#include "gtest/gtest.h"
#include <stdexcept>

#include "ML/FeatureArray.h"
#include "ML/FeatureExtractor.h"
#include "utility/SparseVector.h"
#include "ML/LightFeatureExtractor_ForTest.h"
#include "ML/StandardFeatureExtractor.h"

using namespace std;
using namespace Common;
using namespace Go;
using namespace ML;


TEST(FeatureTest, init) {
  FeatureValues features(210);
  ASSERT_EQ(210, (signed)features.size());

  features.clear();
  for (size_t i=0; i<features.size(); i++) {
    EXPECT_EQ(0, features[i]);
  }
  
  features.init(43);
  EXPECT_EQ(43, (signed)features.size());
  for (size_t i=0; i<features.size(); i++) {
    EXPECT_EQ(0, features[i]);
  }
}

TEST(FeatureTest, extraction_testModule) {
  LightFeatureExtractor_ForTest extractor;

  {
    int init_array[] = {
      3,3,3,3,3,3,3,3,3,3,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,3,3,3,3,3,3,3,3,3,3
    };
    Board board(9, init_array);

    shared_ptr<FeatureValues> features(new FeatureValues(extractor.getFeatureDimension()));
    extractor.extractFromStateAndAction(*features, &board, board.xyToPoint(4,4), BLACK);
    EXPECT_EQ(0, features->at(LightFeatureExtractor_ForTest::CAPTURE));
    EXPECT_EQ(0, features->at(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_0));
    EXPECT_EQ(0, features->at(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_1));
    EXPECT_EQ(0, features->at(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_2));
    EXPECT_EQ(0, features->at(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_3));
    EXPECT_EQ(1, features->at(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_4_OR_MORE));
    shared_ptr<SparseVector> sfeatures(new SparseVector(extractor.getFeatureDimension()));
    extractor.extractFromStateAndAction(*sfeatures, &board, board.xyToPoint(4,4), BLACK);
    EXPECT_FALSE(sfeatures->contains(LightFeatureExtractor_ForTest::CAPTURE));
    EXPECT_FALSE(sfeatures->contains(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_0));
    EXPECT_FALSE(sfeatures->contains(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_1));
    EXPECT_FALSE(sfeatures->contains(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_2));
    EXPECT_FALSE(sfeatures->contains(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_3));
    EXPECT_TRUE(sfeatures->contains(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_4_OR_MORE));

    features.reset(new FeatureValues(extractor.getFeatureDimension()));
    extractor.extractFromStateAndAction(*features, &board, board.xyToPoint(2,4), BLACK);
    EXPECT_EQ(0, features->at(LightFeatureExtractor_ForTest::CAPTURE));
    EXPECT_EQ(0, features->at(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_0));
    EXPECT_EQ(0, features->at(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_1));
    EXPECT_EQ(1, features->at(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_2));
    EXPECT_EQ(0, features->at(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_3));
    EXPECT_EQ(0, features->at(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_4_OR_MORE));
    sfeatures.reset(new SparseVector(extractor.getFeatureDimension()));
    extractor.extractFromStateAndAction(*sfeatures, &board, board.xyToPoint(2,4), BLACK);
    EXPECT_FALSE(sfeatures->contains(LightFeatureExtractor_ForTest::CAPTURE));
    EXPECT_FALSE(sfeatures->contains(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_0));
    EXPECT_FALSE(sfeatures->contains(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_1));
    EXPECT_TRUE(sfeatures->contains(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_2));
    EXPECT_FALSE(sfeatures->contains(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_3));
    EXPECT_FALSE(sfeatures->contains(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_4_OR_MORE));

    features.reset(new FeatureValues(extractor.getFeatureDimension()));
    extractor.extractFromStateAndAction(*features, &board, board.xyToPoint(3,0), BLACK);
    EXPECT_EQ(0, features->at(LightFeatureExtractor_ForTest::CAPTURE));
    EXPECT_EQ(1, features->at(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_0));
    EXPECT_EQ(0, features->at(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_1));
    EXPECT_EQ(0, features->at(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_2));
    EXPECT_EQ(0, features->at(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_3));
    EXPECT_EQ(0, features->at(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_4_OR_MORE));
    sfeatures.reset(new SparseVector(extractor.getFeatureDimension()));
    extractor.extractFromStateAndAction(*sfeatures, &board, board.xyToPoint(3,0), BLACK);
    EXPECT_FALSE(sfeatures->contains(LightFeatureExtractor_ForTest::CAPTURE));
    EXPECT_TRUE(sfeatures->contains(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_0));
    EXPECT_FALSE(sfeatures->contains(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_1));
    EXPECT_FALSE(sfeatures->contains(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_2));
    EXPECT_FALSE(sfeatures->contains(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_3));
    EXPECT_FALSE(sfeatures->contains(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_4_OR_MORE));
  }

  {
    int init_array[] = {
      3,3,3,3,3,3,3,3,3,3,3,
      3,0,0,1,0,0,0,0,0,0,3,
      3,0,1,2,1,0,0,0,0,0,3,
      3,1,2,2,1,0,0,0,0,0,3,
      3,0,0,1,1,0,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,3,3,3,3,3,3,3,3,3,3
    };
    Board board(9, init_array);

    shared_ptr<FeatureValues> features(new FeatureValues(extractor.getFeatureDimension()));
    extractor.extractFromStateAndAction(*features, &board, board.xyToPoint(1,3), 1);

    EXPECT_EQ(1, features->at(LightFeatureExtractor_ForTest::CAPTURE));
    EXPECT_EQ(0, features->at(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_0));
    EXPECT_EQ(1, features->at(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_1));
    EXPECT_EQ(0, features->at(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_2));
    EXPECT_EQ(0, features->at(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_3));
    EXPECT_EQ(0, features->at(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_4_OR_MORE));

    shared_ptr<SparseVector> sfeatures(new SparseVector(extractor.getFeatureDimension()));
    extractor.extractFromStateAndAction(*sfeatures, &board, board.xyToPoint(1,3), BLACK);

    EXPECT_TRUE(sfeatures->contains(LightFeatureExtractor_ForTest::CAPTURE));
    EXPECT_FALSE(sfeatures->contains(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_0));
    EXPECT_TRUE(sfeatures->contains(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_1));
    EXPECT_FALSE(sfeatures->contains(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_2));
    EXPECT_FALSE(sfeatures->contains(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_3));
    EXPECT_FALSE(sfeatures->contains(LightFeatureExtractor_ForTest::DIST_FROM_BORDER_4_OR_MORE));
  }

}

TEST(FeatureTest, arithmeticOp) {

  // =, *=, +=

  {  
    // operator=
    FeatureValues w1(10), w2(20);
    for (size_t i=0; i<w1.size(); i++) {
      w1[i] = i;
    }
    for (size_t i=0; i<w2.size(); i++) {
      w2[i] = i*i;
    }
    w1 = w2;
    EXPECT_EQ(20, (signed)w1.size());
    for (size_t i=0; i<w1.size(); i++) {
      EXPECT_TRUE(fabs(w1[i] - (i*i)) < 0.0001);
    }
  }
  {
    // operator *=
    FeatureValues w1(15);
    for (size_t i=0; i<w1.size(); i++) {
      w1[i] = i;
    }
    w1 *= 21;
    for (size_t i=0; i<w1.size(); i++) {
      EXPECT_EQ((signed)i*21, w1[i]);
    }
  }
  {
    // operator +=
    FeatureValues w1(10), w2(10);
    for (size_t i=0; i<w1.size(); i++) {
      w1[i] = i;
      w2[i] = i*i;
    }
    w1 += w2;
    for (size_t i=0; i<w1.size(); i++) {
      EXPECT_TRUE(fabs(w1[i] - (i+i*i)) < 0.0001);
    }
  }
  {
    // operator -=
    FeatureValues w1(10), w2(10);
    for (size_t i=0; i<w1.size(); i++) {
      w1[i] = i;
      w2[i] = i*i;
    }
    w1 -= w2;
    for (int i=0; i<(signed)w1.size(); i++) {
      EXPECT_EQ((i-i*i), w1[i]);
    }
  }
  {
    // operator =
    // operator *
    FeatureValues w1(10);
    for (size_t i=0; i<w1.size(); i++) {
      w1[i] = i;
    }
    FeatureValues w2 = w1 * 10;
    EXPECT_EQ(w1.size(), w2.size());
    for (size_t i=0; i<w2.size(); i++) {
      EXPECT_EQ(10*w1[i], w2[i]);
    }
    FeatureValues w3(10);
    for (size_t i=0; i<w3.size(); i++) {
      w3[i] = i*i;
    }
    w3 -= w1 * 10;
    EXPECT_EQ(w1.size(), w3.size());
    for (int i=0; i<(signed)w3.size(); i++) {
      EXPECT_EQ((i*i) - 10*w1[i], w3[i]);
    }
  }
  {
    // dot
    FeatureValues w1(10), w2(10);
    int expect = 0;
    for (size_t i=0; i<w1.size(); i++) {
      w1[i] = i;
      w2[i] = i*i;
      expect += i*i*i;
    }
    EXPECT_EQ(expect, w1.dot(w2));
  }
}

TEST(FeatureTest, sparseVectorBasic) {

  // =, *=, +=
  SparseVector vec(10);

  EXPECT_EQ(0, (signed)vec.size());
  vec.add(0); vec.add(2);
  EXPECT_EQ(2, (signed)vec.size());
  EXPECT_TRUE(vec.contains(0));
  EXPECT_TRUE(vec.contains(2));
  EXPECT_FALSE(vec.contains(4));
  vec.clear();
  EXPECT_EQ(0, (signed)vec.size());
  EXPECT_FALSE(vec.contains(0));
  EXPECT_FALSE(vec.contains(2));
  EXPECT_FALSE(vec.contains(4));
}

TEST(FeatureTest, dot_bt_ArrayAndSparse) {
  FeatureWeights w1(15);
  SparseVector s1(15);
  
  double expect = 0;
  for (size_t i=0; i<15; i+=3) {
    s1.add(i);
    w1[i] = (i+2)/(i+1);
    expect += w1[i];
  }
  EXPECT_TRUE(fabs(w1.dot(s1) - expect) < 0.001);
}

TEST(FeatureTest, standardFeatureExtractor) {
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,0,1,0,0,0,0,0,0,3,
    3,0,1,2,1,0,0,0,0,0,3,
    3,1,2,2,1,0,0,0,0,0,3,
    3,0,0,1,1,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  board.put(6,6,2);
    
  StandardFeatureExtractor extractor;

  map<Point, int> pointToPatternIndex;
  Go::Rules::setPutEyeAllowed(true);
  for (int x=0; x<9; x++) {
    for (int y=0; y<9; y++) {
      if (board.checkLegalHand(board.xyToPoint(x,y), 1, 2) == Board::PUT_LEGAL) {
        const PatternExtractor_3x3 &pat = extractor.getPatternExtractor_size9();
        PATTERN_3x3_HASH_VALUE value = pat.encode(board, board.xyToPoint(x,y), false);
        int index = extractor.registerPatternAsFeature(value);
        pointToPatternIndex[board.xyToPoint(x,y)] = index;
      }
    }
  }

  SparseVector featureTable[MAX_BOARD_SIZE];
  PointSet legalMoves;
  SparseVector passFeature;

  extractor.extractFromStateForAllMoves(&board, 1, featureTable, passFeature, legalMoves, true);

  for (size_t i=0; i<legalMoves.size(); i++) {
    Point p = legalMoves[i];

    EXPECT_EQ(Board::PUT_LEGAL, board.checkLegalHand(p, 1, 2));

    SparseVector &vec = featureTable[p];

    int x(board.pointToXY(p).first);
    int y(board.pointToXY(p).second);

    if (abs(x-6) <= 1 && abs(y-6) <= 1) {
      EXPECT_TRUE(vec.contains(StandardFeatureExtractor::STATIC_FEATURE_DIST_FROM_LAST_IS_LESS_THAN_4));
    } else {
      EXPECT_FALSE(vec.contains(StandardFeatureExtractor::STATIC_FEATURE_DIST_FROM_LAST_IS_LESS_THAN_4));
    }
    //EXPECT_FALSE(vec.contains(StandardFeatureExtractor::STATIC_FEATURE_CURRENT_MOVE_IS_PASS));
    //EXPECT_FALSE(vec.contains(StandardFeatureExtractor::STATIC_FEATURE_PREVIOUS_MOVE_IS_PASS));

    int index = pointToPatternIndex[board.xyToPoint(x,y)];
    for (int i=0; i<(signed)extractor.getFeatureDimension() - StandardFeatureExtractor::STATIC_FEATURE_SIZE; i++) {
      if (i != index){ 
        EXPECT_FALSE(vec.contains(i + StandardFeatureExtractor::STATIC_FEATURE_SIZE));
      } else {
        EXPECT_TRUE(vec.contains(i + StandardFeatureExtractor::STATIC_FEATURE_SIZE));
      }
    }
  }
  Go::Rules::setPutEyeAllowed(false);
}
