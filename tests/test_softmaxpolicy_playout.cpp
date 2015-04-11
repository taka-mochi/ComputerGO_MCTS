#include "precomp.h"

#include "gtest/gtest.h"
#include <stdexcept>
#include "AI/SoftmaxPolicyPlayout.h"
#include "ML/StandardFeatureExtractor.h"

#include <ctime>
#include <execinfo.h>

using namespace std;
using namespace Common;
using namespace Go;
using namespace AI;
using namespace ML;

class Test_SoftmaxPolicyPlayout : public ::testing::Test {
protected:
  virtual void SetUp() {
  }

  virtual void TearDown() {
  }

  void test_init() {
    Board board(9);
    StandardFeatureExtractor extractor;
    FeatureWeights weights(extractor.getFeatureDimension()); weights.clear();
    std::vector<ML::PATTERN_3x3_HASH_VALUE> patterns_null;

    SoftmaxPolicyPlayout playout(&board, patterns_null, weights, Common::MTRandom::getInstance());

    for (size_t i=0; i<playout.m_featureWeights.size(); i++) {
      EXPECT_TRUE(fabs(0.0-playout.m_featureWeights[i])<0.0001);
    }

  }

  void test_initProb_emptyboard() {
    // test first initialization of am empty board. features are standardFeatures
    Board board(9);
    StandardFeatureExtractor extractor;
    std::vector<ML::PATTERN_3x3_HASH_VALUE> patterns;

    unordered_map<int, double> index_to_weight;
    index_to_weight[StandardFeatureExtractor::STATIC_FEATURE_DIST_FROM_LAST_IS_LESS_THAN_4] = 0;
    //index_to_weight[StandardFeatureExtractor::STATIC_FEATURE_PREVIOUS_MOVE_IS_PASS] = 0;
    index_to_weight[StandardFeatureExtractor::STATIC_FEATURE_CURRENT_MOVE_IS_PASS] = 0;

    for (int x=0; x<9; x++) {
      for (int y=0; y<9; y++) {
        int dist_x = std::min(x, 8-x);
        int dist_y = std::min(y, 8-y);
        int dist = std::min(dist_x, dist_y);
        double weight = 0;
        if (dist <= 1) {
          weight = -0.5;
        }
        PATTERN_3x3_HASH_VALUE pat = extractor.getPatternExtractor_size9().encode(board, board.xyToPoint(x,y), false);
        int index = extractor.registerPatternAsFeature(pat);
        index_to_weight[StandardFeatureExtractor::STATIC_FEATURE_SIZE + index] = weight;
        patterns.push_back(pat);

        pat = extractor.getPatternExtractor_size9().encode(board, board.xyToPoint(x,y), true);
        index = extractor.registerPatternAsFeature(pat);
        index_to_weight[StandardFeatureExtractor::STATIC_FEATURE_SIZE + index] = weight;
        patterns.push_back(pat);
      }
    }

    FeatureWeights weights(extractor.getFeatureDimension()); weights.clear();
    for (size_t i=0; i<weights.size(); i++) {
      weights[i] = index_to_weight[i];
    }

    SoftmaxPolicyPlayout policy(&board, patterns, weights, Common::MTRandom::getInstance());

    policy.initProbabilities(&board, BLACK, policy.m_probTableBlack, policy.m_featureTableBlack);
    board.put(PASS, BLACK);
    policy.initProbabilities(&board, WHITE, policy.m_probTableWhite, policy.m_featureTableWhite);

    // check probability table of both color
    for (int x=0; x<9; x++) {
      for (int y=0; y<9; y++) {
        int dist_x = std::min(x, 8-x);
        int dist_y = std::min(y, 8-y);
        int dist = std::min(dist_x, dist_y);
        double exp_weight = 1;
        if (dist <= 1) {
          exp_weight = exp(-0.5);
        }
        Point p = board.xyToPoint(x,y);
        if (exp_weight != policy.m_probTableBlack[p]) {
          //cerr << "pos = (" << x << "," << y << ")" << endl;
        }
        EXPECT_TRUE(fabs(exp_weight - policy.m_probTableBlack[p]) < 0.0001);
        EXPECT_TRUE(fabs(exp_weight - policy.m_probTableWhite[p]) < 0.0001);
      }
    }
  }

  void test_initProb_randomboard() {
    Board board(9);
    StandardFeatureExtractor extractor;
    std::vector<ML::PATTERN_3x3_HASH_VALUE> patterns;
    unordered_map<int, double> index_to_weight;
    index_to_weight[StandardFeatureExtractor::STATIC_FEATURE_DIST_FROM_LAST_IS_LESS_THAN_4] = 1;
    //index_to_weight[StandardFeatureExtractor::STATIC_FEATURE_PREVIOUS_MOVE_IS_PASS] = 1;
    //index_to_weight[StandardFeatureExtractor::STATIC_FEATURE_CURRENT_MOVE_IS_PASS] = 1;

    board.put(2,2,BLACK);
    board.put(5,4,WHITE);

    for (int x=0; x<9; x++) {
      for (int y=0; y<9; y++) {
        double weight = 0;
        if (!board.isColor(x,y,FREE)) continue;
        PATTERN_3x3_HASH_VALUE pat = extractor.getPatternExtractor_size9().encode(board, board.xyToPoint(x,y), false);
        if (pat != 0) {
          weight = -0.5;
        }
        int index = extractor.registerPatternAsFeature(pat);
        index_to_weight[StandardFeatureExtractor::STATIC_FEATURE_SIZE + index] = weight;
        patterns.push_back(pat);

        pat = extractor.getPatternExtractor_size9().encode(board, board.xyToPoint(x,y), true);
        index = extractor.registerPatternAsFeature(pat);
        index_to_weight[StandardFeatureExtractor::STATIC_FEATURE_SIZE + index] = weight;
        patterns.push_back(pat);
      }
    }

    FeatureWeights weights(extractor.getFeatureDimension()); weights.clear();
    for (size_t i=0; i<weights.size(); i++) {
      weights[i] = index_to_weight[i];
    }

    SoftmaxPolicyPlayout policy(&board, patterns, weights, Common::MTRandom::getInstance());

    policy.initProbabilities(&board, BLACK, policy.m_probTableBlack, policy.m_featureTableBlack);
    board.put(PASS, BLACK);
    policy.initProbabilities(&board, WHITE, policy.m_probTableWhite, policy.m_featureTableWhite);
    // check probability table of both color
    for (int x=0; x<9; x++) {
      for (int y=0; y<9; y++) {
        int dist_x = std::min(x, 8-x);
        int dist_y = std::min(y, 8-y);
        int dist = std::min(dist_x, dist_y);
        double exp_weight = 1;
        double exp_weight_white = 1;
        if (dist <= 1 || 
            (x>=1 && x<=3 && y>=1 && y<=3) ||
            (x>=4 && x<=6 && y>=3 && y<=5)) {
          exp_weight = exp(-0.5);
          exp_weight_white = exp(-0.5);
        }
        if (x>=4 && x<=6 && y>=3 && y<=5) {
          exp_weight *= exp(1);
        }
        if (!board.isColor(x,y,FREE)) exp_weight_white = exp_weight = 0;

        Point p = board.xyToPoint(x,y);
        if (fabs(exp_weight - policy.m_probTableBlack[p]) >= 0.001) {
          cerr << exp_weight << "," << policy.m_probTableBlack[p] << endl;
        }
        EXPECT_TRUE(fabs(exp_weight - policy.m_probTableBlack[p]) < 0.001);
        EXPECT_TRUE(fabs(exp_weight_white - policy.m_probTableWhite[p]) < 0.001);
      }
    }
  }

  // void test_addUpdateCandidates() {
  //   Board board(9);
  //   StandardFeatureExtractor extractor;
  //   std::vector<ML::PATTERN_3x3_HASH_VALUE> patterns_null;
    
  //   SparseVector featureTable[MAX_BOARD_SIZE];

  //   board.put(2,2,BLACK);
  //   board.put(5,4,WHITE);
  //   board.put(5,5,BLACK);
  //   board.put(3,4,WHITE);
  //   board.put(6,4,BLACK);
  //   board.put(5,3,WHITE);
  //   board.put(6,3,BLACK);
  //   board.put(3,3,WHITE);
  //   board.put(5,2,BLACK);
  //   board.put(3,2,WHITE);
  //   board.put(4,3,BLACK);
  //   board.put(6,6,WHITE);
  //   board.put(4,4,BLACK);
  //   board.put(6,5,WHITE);

  //   const Board::MoveChangeEntry *lastMove = board.getHistory(board.getHistoryCount()-1);
  //   const Board::MoveChangeEntry *secondLastMove = board.getHistory(board.getHistoryCount()-2);
  //   ASSERT_EQ(WHITE, lastMove->m_putColor);
  //   ASSERT_EQ(board.xyToPoint(6,5), lastMove->m_putPos);
  //   ASSERT_EQ(BLACK, secondLastMove->m_putColor);
  //   ASSERT_EQ(board.xyToPoint(4,4), secondLastMove->m_putPos);

  //   FeatureWeights weights(extractor.getFeatureDimension());
  //   SoftmaxPolicyPlayout policy(&board, patterns_null, weights);

  //   PointSet pointSet;
  //   policy.addUpdateCandidateMovesAndAddFeaturesForPreviousMoveFeatures(pointSet, &board, lastMove, featureTable, BLACK);
  //   policy.enumerateMovesOfPatternChanged(pointSet, &board, BLACK, lastMove, NULL);
  //   EXPECT_EQ((unsigned)5, pointSet.size());
  //   EXPECT_NE(pointSet.end(), pointSet.find(board.xyToPoint(5,4)));
  //   EXPECT_NE(pointSet.end(), pointSet.find(board.xyToPoint(7,4)));
  //   EXPECT_NE(pointSet.end(), pointSet.find(board.xyToPoint(7,5)));
  //   EXPECT_NE(pointSet.end(), pointSet.find(board.xyToPoint(5,6)));
  //   EXPECT_NE(pointSet.end(), pointSet.find(board.xyToPoint(7,6)));
    
  //   pointSet.clear();
  //   policy.addUpdateCandidateMovesAndAddFeaturesForPreviousMoveFeatures(pointSet, &board, lastMove, featureTable, WHITE);
  //   policy.enumerateMovesOfPatternChanged(pointSet, &board, WHITE, lastMove, NULL);
  //   EXPECT_EQ((unsigned)5, pointSet.size());
  //   EXPECT_NE(pointSet.end(), pointSet.find(board.xyToPoint(5,4)));
  //   EXPECT_NE(pointSet.end(), pointSet.find(board.xyToPoint(7,4)));
  //   EXPECT_NE(pointSet.end(), pointSet.find(board.xyToPoint(7,5)));
  //   EXPECT_NE(pointSet.end(), pointSet.find(board.xyToPoint(5,6)));
  //   EXPECT_NE(pointSet.end(), pointSet.find(board.xyToPoint(7,6)));

  //   pointSet.clear();
  //   //policy.addUpdateCandidateMoves(pointSet, &board, secondLastMove, BLACK);
  //   policy.enumerateMovesOfPatternChanged(pointSet, &board, BLACK, NULL, secondLastMove);
  //   EXPECT_EQ((unsigned)6, pointSet.size());
  //   EXPECT_NE(pointSet.end(), pointSet.find(board.xyToPoint(4,5)));
  //   EXPECT_NE(pointSet.end(), pointSet.find(board.xyToPoint(5,4)));
  //   EXPECT_NE(pointSet.end(), pointSet.find(board.xyToPoint(5,3)));
  //   EXPECT_NE(pointSet.end(), pointSet.find(board.xyToPoint(4,2)));
  //   EXPECT_NE(pointSet.end(), pointSet.find(board.xyToPoint(6,2)));
  //   EXPECT_NE(pointSet.end(), pointSet.find(board.xyToPoint(3,5)));

  //   pointSet.clear();
  //   //policy.addUpdateCandidateMoves(pointSet, &board, secondLastMove, WHITE);
  //   policy.enumerateMovesOfPatternChanged(pointSet, &board, WHITE, NULL, secondLastMove);
  //   EXPECT_EQ((unsigned)6, pointSet.size());
  //   EXPECT_NE(pointSet.end(), pointSet.find(board.xyToPoint(4,5)));
  //   EXPECT_NE(pointSet.end(), pointSet.find(board.xyToPoint(5,4)));
  //   EXPECT_NE(pointSet.end(), pointSet.find(board.xyToPoint(5,3)));
  //   EXPECT_NE(pointSet.end(), pointSet.find(board.xyToPoint(4,2)));
  //   EXPECT_NE(pointSet.end(), pointSet.find(board.xyToPoint(6,2)));
  //   EXPECT_NE(pointSet.end(), pointSet.find(board.xyToPoint(3,5)));
  // }

  void test_updateProbBeforeAction() {
    {
      Board board(9);
      StandardFeatureExtractor extractor;
      std::vector<ML::PATTERN_3x3_HASH_VALUE> patterns;

      board.put(2,2,BLACK);
      board.put(5,4,WHITE);
      board.put(5,5,BLACK);
      board.put(3,4,WHITE);
      board.put(6,4,BLACK);
      board.put(5,3,WHITE);
      board.put(6,3,BLACK);
      board.put(3,3,WHITE);
      board.put(5,2,BLACK);
      board.put(3,2,WHITE);
      board.put(4,3,BLACK);
      board.put(6,6,WHITE);
      board.put(4,4,BLACK);
      board.put(6,5,WHITE);

      board.printToErr();

      unordered_map<int, double> index_to_weight;
      index_to_weight[StandardFeatureExtractor::STATIC_FEATURE_DIST_FROM_LAST_IS_LESS_THAN_4] = 1;
      //index_to_weight[StandardFeatureExtractor::STATIC_FEATURE_PREVIOUS_MOVE_IS_PASS] = 1;
      index_to_weight[StandardFeatureExtractor::STATIC_FEATURE_CURRENT_MOVE_IS_PASS] = 1;
      //index_to_weight[StandardFeatureExtractor::STATIC_FEATURE_CAPTURE_RE_CAPTURE] = 2;

      for (int x=0; x<9; x++) {
        for (int y=0; y<9; y++) {
          double weight = -1;
          if (!board.isColor(x,y,FREE)) continue;
          PATTERN_3x3_HASH_VALUE pat = extractor.getPatternExtractor_size9().encode(board, board.xyToPoint(x,y), false);
          if (pat != 0) {
            weight = 0;
          }
          int index = extractor.registerPatternAsFeature(pat);
          index_to_weight[StandardFeatureExtractor::STATIC_FEATURE_SIZE + index] = weight;
          patterns.push_back(pat);

          pat = extractor.getPatternExtractor_size9().encode(board, board.xyToPoint(x,y), true);
          index = extractor.registerPatternAsFeature(pat);
          index_to_weight[StandardFeatureExtractor::STATIC_FEATURE_SIZE + index] = weight;
          patterns.push_back(pat);
        }
      }

      FeatureWeights weights(extractor.getFeatureDimension()); weights.clear();
      for (size_t i=0; i<weights.size(); i++) {
        weights[i] = index_to_weight[i];
      }

      SoftmaxPolicyPlayout policy(&board, patterns, weights, Common::MTRandom::getInstance());

      policy.initProbabilities(&board, BLACK, policy.m_probTableBlack, policy.m_featureTableBlack);
      //policy.updateProbabilitiesBeforeAction(&board, BLACK);
      EXPECT_EQ(0.0, policy.m_probTableBlack[board.xyToPoint(6,5)]);
      EXPECT_EQ(0.0, policy.m_probTableBlack[board.xyToPoint(5,5)]);
      EXPECT_EQ(0.0, policy.m_probTableBlack[board.xyToPoint(6,6)]);
      EXPECT_EQ(0.0, policy.m_probTableBlack[board.xyToPoint(6,4)]);
      EXPECT_TRUE(fabs(exp(1) - policy.m_probTableBlack[board.xyToPoint(5,4)]) < 0.001);
      EXPECT_TRUE(fabs(exp(1) - policy.m_probTableBlack[board.xyToPoint(7,4)]) < 0.001);
      EXPECT_TRUE(fabs(exp(1) - policy.m_probTableBlack[board.xyToPoint(7,5)]) < 0.001);
      EXPECT_TRUE(fabs(exp(1) - policy.m_probTableBlack[board.xyToPoint(7,6)]) < 0.001);
      EXPECT_TRUE(fabs(exp(1) - policy.m_probTableBlack[board.xyToPoint(5,6)]) < 0.001);

      board.put(4,5,BLACK);
      policy.initProbabilities(&board, WHITE, policy.m_probTableWhite, policy.m_featureTableWhite);
      //policy.updateProbabilitiesBeforeAction(&board, WHITE);
      EXPECT_TRUE(fabs(0.0 - policy.m_probTableWhite[board.xyToPoint(4,5)]) < 0.001);
      EXPECT_TRUE(fabs(0.0 - policy.m_probTableWhite[board.xyToPoint(4,4)]) < 0.001);
      EXPECT_TRUE(fabs(0.0 - policy.m_probTableWhite[board.xyToPoint(5,5)]) < 0.001);
      EXPECT_TRUE(fabs(exp(1) - policy.m_probTableWhite[board.xyToPoint(3,5)]) < 0.001);
      EXPECT_TRUE(fabs(1*exp(1) - policy.m_probTableWhite[board.xyToPoint(4,6)]) < 0.001);
      EXPECT_TRUE(fabs(1*exp(1) - policy.m_probTableWhite[board.xyToPoint(3,6)]) < 0.001);
      EXPECT_TRUE(fabs(1*exp(1) - policy.m_probTableWhite[board.xyToPoint(5,6)]) < 0.001);
      EXPECT_TRUE(fabs(1 - policy.m_probTableWhite[board.xyToPoint(7,4)]) < 0.001);
      EXPECT_TRUE(fabs(1 - policy.m_probTableWhite[board.xyToPoint(7,5)]) < 0.001);
      EXPECT_TRUE(fabs(1 - policy.m_probTableWhite[board.xyToPoint(7,6)]) < 0.001);
 
      board.put(2,3,WHITE);
      policy.updateProbabilitiesBeforeAction(&board, BLACK);
      EXPECT_TRUE(fabs(0.0 - policy.m_probTableBlack[board.xyToPoint(2,2)]) < 0.001);
      EXPECT_TRUE(fabs(0.0 - policy.m_probTableBlack[board.xyToPoint(2,3)]) < 0.001);
      EXPECT_TRUE(fabs(0.0 - policy.m_probTableBlack[board.xyToPoint(3,4)]) < 0.001);
      EXPECT_TRUE(fabs(0.0 - policy.m_probTableBlack[board.xyToPoint(3,3)]) < 0.001);
      EXPECT_TRUE(fabs(0.0 - policy.m_probTableBlack[board.xyToPoint(3,2)]) < 0.001);
      EXPECT_TRUE(fabs(1*exp(1) - policy.m_probTableBlack[board.xyToPoint(1,3)]) < 0.001);
      EXPECT_TRUE(fabs(1*exp(1) - policy.m_probTableBlack[board.xyToPoint(1,2)]) < 0.001);
      EXPECT_TRUE(fabs(1*exp(1) - policy.m_probTableBlack[board.xyToPoint(1,4)]) < 0.001);
      EXPECT_TRUE(fabs(1*exp(1) - policy.m_probTableBlack[board.xyToPoint(2,4)]) < 0.001);
      EXPECT_TRUE(fabs(1 - policy.m_probTableBlack[board.xyToPoint(3,6)]) < 0.001);
      EXPECT_TRUE(fabs(1 - policy.m_probTableBlack[board.xyToPoint(4,6)]) < 0.001);
      EXPECT_TRUE(fabs(1 - policy.m_probTableBlack[board.xyToPoint(5,4)]) < 0.001);
      EXPECT_TRUE(fabs(1 - policy.m_probTableBlack[board.xyToPoint(5,6)]) < 0.001);
      EXPECT_TRUE(fabs(1 - policy.m_probTableBlack[board.xyToPoint(7,4)]) < 0.001);
      EXPECT_TRUE(fabs(1 - policy.m_probTableBlack[board.xyToPoint(7,5)]) < 0.001);
      EXPECT_TRUE(fabs(1 - policy.m_probTableBlack[board.xyToPoint(7,6)]) < 0.001);

      board.put(0,3,BLACK);
      board.printToErr();
      policy.updateProbabilitiesBeforeAction(&board, WHITE);
      EXPECT_TRUE(fabs(0.0 - policy.m_probTableWhite[board.xyToPoint(0,3)]) < 0.001);
      EXPECT_TRUE(fabs(exp(1) - policy.m_probTableWhite[board.xyToPoint(0,2)]) < 0.001);
      EXPECT_TRUE(fabs(exp(1) - policy.m_probTableWhite[board.xyToPoint(0,4)]) < 0.001);
      EXPECT_TRUE(fabs(exp(1) - policy.m_probTableWhite[board.xyToPoint(1,2)]) < 0.001);
      EXPECT_TRUE(fabs(exp(1) - policy.m_probTableWhite[board.xyToPoint(1,3)]) < 0.001);
      EXPECT_TRUE(fabs(exp(1) - policy.m_probTableWhite[board.xyToPoint(1,4)]) < 0.001);
      EXPECT_TRUE(fabs(1 - policy.m_probTableWhite[board.xyToPoint(2,4)]) < 0.001);
      EXPECT_TRUE(fabs(1 - policy.m_probTableWhite[board.xyToPoint(3,6)]) < 0.001);
      EXPECT_TRUE(fabs(1 - policy.m_probTableWhite[board.xyToPoint(4,6)]) < 0.001);
      EXPECT_TRUE(fabs(1 - policy.m_probTableWhite[board.xyToPoint(5,4)]) < 0.001);
      EXPECT_TRUE(fabs(1 - policy.m_probTableWhite[board.xyToPoint(5,6)]) < 0.001);
      EXPECT_TRUE(fabs(1 - policy.m_probTableWhite[board.xyToPoint(7,4)]) < 0.001);
      EXPECT_TRUE(fabs(1 - policy.m_probTableWhite[board.xyToPoint(7,5)]) < 0.001);
      EXPECT_TRUE(fabs(1 - policy.m_probTableWhite[board.xyToPoint(7,6)]) < 0.001);

    }

    {
      Board board(9);
      StandardFeatureExtractor extractor;
      std::vector<ML::PATTERN_3x3_HASH_VALUE> patterns;

      board.put(2,2,BLACK);
      board.put(3,2,WHITE);
      board.put(3,3,BLACK);
      board.put(PASS,WHITE);
      board.put(4,2,BLACK);

      FeatureWeights weights(extractor.getFeatureDimension());
      SoftmaxPolicyPlayout policy(&board, patterns, weights, Common::MTRandom::getInstance());
      policy.initProbabilities(&board, WHITE, policy.m_probTableWhite, policy.m_featureTableWhite);
      board.put(PASS,WHITE);
      policy.initProbabilities(&board, BLACK, policy.m_probTableBlack, policy.m_featureTableBlack);
      board.put(3,1,BLACK);
      policy.updateProbabilitiesBeforeAction(&board, WHITE);
      EXPECT_EQ(0.0, policy.m_probTableWhite[board.xyToPoint(3,2)]);
      board.put(4,5,WHITE);
      policy.updateProbabilitiesBeforeAction(&board, BLACK);
      EXPECT_EQ(0.0, policy.m_probTableBlack[board.xyToPoint(3,2)]);
    }

    {
      Board board(9);
      StandardFeatureExtractor extractor;
      std::vector<ML::PATTERN_3x3_HASH_VALUE> patterns;

      board.put(4,4,WHITE);
      board.put(3,3,BLACK);
      board.put(3,2,WHITE);
      board.put(4,3,BLACK);
      board.put(4,2,WHITE);
      board.put(5,2,BLACK);
      board.put(2,3,WHITE);
      board.put(6,3,BLACK);
      board.put(3,4,WHITE);
      board.put(5,4,BLACK);
      board.put(5,3,WHITE);

      // 0,0,0,0,0,0,0,0,0
      // 0,0,0,0,0,0,0,0,0
      // 0,0,0,2,2,1,0,0,0
      // 0,0,2,0,0,2,1,0,0
      // 0,0,0,2,2,1,0,0,0
      // 0,0,0,0,0,0,0,0,0
      // 0,0,0,0,0,0,0,0,0
      // 0,0,0,0,0,0,0,0,0
      // 0,0,0,0,0,0,0,0,0

      FeatureWeights weights(extractor.getFeatureDimension());
      weights[StandardFeatureExtractor::STATIC_FEATURE_CAPTURE_RE_CAPTURE] = -0.5;
      weights[StandardFeatureExtractor::STATIC_FEATURE_DIST_FROM_LAST_IS_LESS_THAN_4] = 2;
      SoftmaxPolicyPlayout policy(&board, patterns, weights, Common::MTRandom::getInstance());

      policy.initProbabilities(&board, BLACK, policy.m_probTableBlack, policy.m_featureTableBlack);
      policy.updateProbabilitiesBeforeAction(&board, BLACK);
      EXPECT_TRUE(fabs(exp(2)*exp(-0.5) - policy.m_probTableBlack[board.xyToPoint(4,3)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableBlack[board.xyToPoint(6,2)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableBlack[board.xyToPoint(6,4)]) < 0.001);
      EXPECT_TRUE(fabs(0 - policy.m_probTableBlack[board.xyToPoint(5,2)]) < 0.001);
      EXPECT_TRUE(fabs(0 - policy.m_probTableBlack[board.xyToPoint(5,3)]) < 0.001);
      EXPECT_TRUE(fabs(0 - policy.m_probTableBlack[board.xyToPoint(5,4)]) < 0.001);
      EXPECT_TRUE(fabs(0 - policy.m_probTableBlack[board.xyToPoint(4,2)]) < 0.001);
      EXPECT_TRUE(fabs(0 - policy.m_probTableBlack[board.xyToPoint(4,4)]) < 0.001);

    }

    {
      // check STATIC_FEATURE_CAPTURE_STRING_TO_NEW_ATARI
      Board board(9);
      StandardFeatureExtractor extractor;
      std::vector<ML::PATTERN_3x3_HASH_VALUE> patterns;

      board.put(1,3,BLACK);
      board.put(PASS,WHITE);
      board.put(2,3,BLACK);
      board.put(PASS,WHITE);
      board.put(2,0,BLACK);
      board.put(1,0,WHITE);
      board.put(2,1,BLACK);
      board.put(3,1,WHITE);
      board.put(1,1,BLACK);
      board.put(2,2,WHITE);
      board.put(0,1,BLACK);
      board.put(1,2,WHITE);
      board.put(3,2,BLACK);
      board.put(0,2,WHITE);
      board.put(4,1,BLACK);
      board.printToErr();

      // 0,2,1,0,0,0,0,0,0
      // 1,1,1,2,1,0,0,0,0
      // 2,2,2,1,0,0,0,0,0
      // 0,1,1,0,0,0,0,0,0
      // 0,0,0,0,0,0,0,0,0
      // 0,0,0,0,0,0,0,0,0
      // 0,0,0,0,0,0,0,0,0
      // 0,0,0,0,0,0,0,0,0
      // 0,0,0,0,0,0,0,0,0

      FeatureWeights weights(extractor.getFeatureDimension());
      weights[StandardFeatureExtractor::STATIC_FEATURE_CAPTURE_STRING_TO_NEW_ATARI] = 3;
      weights[StandardFeatureExtractor::STATIC_FEATURE_DIST_FROM_LAST_IS_LESS_THAN_4] = 2;
      SoftmaxPolicyPlayout policy(&board, patterns, weights, Common::MTRandom::getInstance());

      policy.initProbabilities(&board, WHITE, policy.m_probTableWhite, policy.m_featureTableWhite);
      policy.updateProbabilitiesBeforeAction(&board, WHITE);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableWhite[board.xyToPoint(3,0)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableWhite[board.xyToPoint(4,0)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableWhite[board.xyToPoint(5,0)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableWhite[board.xyToPoint(5,1)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableWhite[board.xyToPoint(5,2)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableWhite[board.xyToPoint(4,2)]) < 0.001);
 
      board.put(3,0,WHITE);
      policy.initProbabilities(&board, BLACK, policy.m_probTableBlack, policy.m_featureTableBlack);
      policy.updateProbabilitiesBeforeAction(&board, BLACK);
      EXPECT_TRUE(fabs(fmath::expd(2)*fmath::expd(3) - policy.m_probTableBlack[board.xyToPoint(4,0)]) < 0.001);
      EXPECT_TRUE(fabs(exp(3) - policy.m_probTableBlack[board.xyToPoint(0,0)]) < 0.001);
      EXPECT_TRUE(fabs(exp(3) - policy.m_probTableBlack[board.xyToPoint(0,3)]) < 0.001);
      EXPECT_TRUE(fabs(0 - policy.m_probTableBlack[board.xyToPoint(3,0)]) < 0.001);
      EXPECT_TRUE(fabs(1 - policy.m_probTableBlack[board.xyToPoint(5,0)]) < 0.001);
      EXPECT_TRUE(fabs(1 - policy.m_probTableBlack[board.xyToPoint(5,1)]) < 0.001);
      EXPECT_TRUE(fabs(1 - policy.m_probTableBlack[board.xyToPoint(5,2)]) < 0.001);
      EXPECT_TRUE(fabs(1 - policy.m_probTableBlack[board.xyToPoint(4,2)]) < 0.001);
    }
    {
      // check STATIC_FEATURE_CAPTURE_STRING_TO_NEW_ATARI & STATIC_FEATURE_DIST_FROM_BEFORE_PREVIOUS_IS_LESS_THAN_4
      Board board(9);
      StandardFeatureExtractor extractor;
      std::vector<ML::PATTERN_3x3_HASH_VALUE> patterns;

      board.put(1,3,BLACK);
      board.put(PASS,WHITE);
      board.put(2,3,BLACK);
      board.put(PASS,WHITE);
      board.put(2,0,BLACK);
      board.put(1,0,WHITE);
      board.put(2,1,BLACK);
      board.put(3,1,WHITE);
      board.put(1,1,BLACK);
      board.put(2,2,WHITE);
      board.put(0,1,BLACK);
      board.put(1,2,WHITE);
      board.put(3,2,BLACK);
      board.put(0,2,WHITE);
      board.put(4,1,BLACK);
      board.printToErr();

      // 0,2,1,0,0,0,0,0,0
      // 1,1,1,2,1,0,0,0,0
      // 2,2,2,1,0,0,0,0,0
      // 0,1,1,0,0,0,0,0,0
      // 0,0,0,0,0,0,0,0,0
      // 0,0,0,0,0,0,0,0,0
      // 0,0,0,0,0,0,0,0,0
      // 0,0,0,0,0,0,0,0,0
      // 0,0,0,0,0,0,0,0,0

      FeatureWeights weights(extractor.getFeatureDimension());
      weights[StandardFeatureExtractor::STATIC_FEATURE_CAPTURE_STRING_TO_NEW_ATARI] = 3;
      weights[StandardFeatureExtractor::STATIC_FEATURE_DIST_FROM_LAST_IS_LESS_THAN_4] = 2;
      weights[StandardFeatureExtractor::STATIC_FEATURE_DIST_FROM_BEFORE_PREVIOUS_IS_LESS_THAN_4] = 0.5;
      SoftmaxPolicyPlayout policy(&board, patterns, weights, Common::MTRandom::getInstance());

      policy.initProbabilities(&board, WHITE, policy.m_probTableWhite, policy.m_featureTableWhite);
      policy.updateProbabilitiesBeforeAction(&board, WHITE);
      EXPECT_TRUE(fabs(exp(0.5) - policy.m_probTableWhite[board.xyToPoint(0,3)]) < 0.001);
      EXPECT_TRUE(fabs(0 - policy.m_probTableWhite[board.xyToPoint(0,1)]) < 0.001);
      EXPECT_TRUE(fabs(0 - policy.m_probTableWhite[board.xyToPoint(1,3)]) < 0.001);
      EXPECT_TRUE(fabs(0 - policy.m_probTableWhite[board.xyToPoint(1,2)]) < 0.001);
      EXPECT_TRUE(fabs(0 - policy.m_probTableWhite[board.xyToPoint(1,1)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableWhite[board.xyToPoint(3,0)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableWhite[board.xyToPoint(4,0)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableWhite[board.xyToPoint(5,0)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableWhite[board.xyToPoint(5,1)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableWhite[board.xyToPoint(5,2)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableWhite[board.xyToPoint(4,2)]) < 0.001);
 
      board.put(3,0,WHITE);
      policy.initProbabilities(&board, BLACK, policy.m_probTableBlack, policy.m_featureTableBlack);
      policy.updateProbabilitiesBeforeAction(&board, BLACK);
      EXPECT_TRUE(fabs(fmath::expd(0.5+2+3)-policy.m_probTableBlack[board.xyToPoint(4,0)]) < 0.001);
      EXPECT_TRUE(fabs(exp(3) - policy.m_probTableBlack[board.xyToPoint(0,0)]) < 0.001);
      EXPECT_TRUE(fabs(exp(3) - policy.m_probTableBlack[board.xyToPoint(0,3)]) < 0.001);
      EXPECT_TRUE(fabs(0 - policy.m_probTableBlack[board.xyToPoint(3,0)]) < 0.001);
      EXPECT_TRUE(fabs(exp(0.5) - policy.m_probTableBlack[board.xyToPoint(5,0)]) < 0.001);
      EXPECT_TRUE(fabs(exp(0.5) - policy.m_probTableBlack[board.xyToPoint(5,1)]) < 0.001);
      EXPECT_TRUE(fabs(exp(0.5) - policy.m_probTableBlack[board.xyToPoint(5,2)]) < 0.001);
      EXPECT_TRUE(fabs(exp(0.5) - policy.m_probTableBlack[board.xyToPoint(4,2)]) < 0.001);
    }

    {
      // check Kou & STATIC_FEATURE_DIST_FROM_BEFORE_PREVIOUS_IS_LESS_THAN_4
      Board board(9);
      StandardFeatureExtractor extractor;
      std::vector<ML::PATTERN_3x3_HASH_VALUE> patterns;

      board.put(3,2,BLACK);
      board.put(3,3,WHITE);
      board.put(4,2,BLACK);
      board.put(2,2,WHITE);
      board.put(4,3,BLACK);
      board.put(2,4,WHITE);
      board.put(3,4,BLACK);
      board.put(1,3,WHITE);
      board.printToErr();

      // 0,0,0,0,0,0,0,0,0
      // 0,0,0,0,0,0,0,0,0
      // 0,0,2,1,1,0,0,0,0
      // 0,2,0,2,1,0,0,0,0
      // 0,0,2,1,0,0,0,0,0
      // 0,0,0,0,0,0,0,0,0
      // 0,0,0,0,0,0,0,0,0
      // 0,0,0,0,0,0,0,0,0
      // 0,0,0,0,0,0,0,0,0

      FeatureWeights weights(extractor.getFeatureDimension());
      weights[StandardFeatureExtractor::STATIC_FEATURE_DIST_FROM_LAST_IS_LESS_THAN_4] = 2;
      weights[StandardFeatureExtractor::STATIC_FEATURE_DIST_FROM_BEFORE_PREVIOUS_IS_LESS_THAN_4] = 0.5;
      SoftmaxPolicyPlayout policy(&board, patterns, weights, Common::MTRandom::getInstance());

      policy.initProbabilities(&board, BLACK, policy.m_probTableBlack, policy.m_featureTableBlack);
      policy.updateProbabilitiesBeforeAction(&board, BLACK);
      EXPECT_TRUE(fabs(fmath::expd(2+0.5) - policy.m_probTableBlack[board.xyToPoint(2,3)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableBlack[board.xyToPoint(0,2)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableBlack[board.xyToPoint(1,2)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableBlack[board.xyToPoint(0,3)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableBlack[board.xyToPoint(0,4)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableBlack[board.xyToPoint(1,4)]) < 0.001);

      EXPECT_TRUE(fabs(exp(0.5) - policy.m_probTableBlack[board.xyToPoint(4,4)]) < 0.001);
      EXPECT_TRUE(fabs(exp(0.5) - policy.m_probTableBlack[board.xyToPoint(4,5)]) < 0.001);
      EXPECT_TRUE(fabs(exp(0.5) - policy.m_probTableBlack[board.xyToPoint(3,5)]) < 0.001);
      EXPECT_TRUE(fabs(exp(0.5) - policy.m_probTableBlack[board.xyToPoint(2,5)]) < 0.001);

      board.put(2,3,BLACK);
      policy.initProbabilities(&board, WHITE, policy.m_probTableWhite, policy.m_featureTableWhite);
      policy.updateProbabilitiesBeforeAction(&board, WHITE);
      EXPECT_TRUE(fabs(0 -  policy.m_probTableWhite[board.xyToPoint(3,3)]) < 0.001);
      EXPECT_TRUE(fabs(0 -  policy.m_probTableWhite[board.xyToPoint(2,3)]) < 0.001);
      EXPECT_TRUE(fabs(exp(0.5) - policy.m_probTableWhite[board.xyToPoint(0,2)]) < 0.001);
      EXPECT_TRUE(fabs(exp(0.5) - policy.m_probTableWhite[board.xyToPoint(0,3)]) < 0.001);
      EXPECT_TRUE(fabs(exp(0.5) - policy.m_probTableWhite[board.xyToPoint(0,4)]) < 0.001);
      EXPECT_TRUE(fabs(fmath::expd(2+0.5) - policy.m_probTableWhite[board.xyToPoint(1,2)]) < 0.001);
      EXPECT_TRUE(fabs(fmath::expd(2+0.5) - policy.m_probTableWhite[board.xyToPoint(1,4)]) < 0.001);

      board.put(1,4,WHITE);
      policy.updateProbabilitiesBeforeAction(&board, BLACK);
      EXPECT_TRUE(fabs(exp(0.5) - policy.m_probTableBlack[board.xyToPoint(3,3)]) < 0.001);
      EXPECT_TRUE(fabs(exp(0.5) - policy.m_probTableBlack[board.xyToPoint(1,2)]) < 0.001);
      EXPECT_TRUE(fabs(0 -  policy.m_probTableBlack[board.xyToPoint(1,4)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableBlack[board.xyToPoint(0,3)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableBlack[board.xyToPoint(0,4)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableBlack[board.xyToPoint(0,5)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableBlack[board.xyToPoint(2,5)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableBlack[board.xyToPoint(1,5)]) < 0.001);

      board.put(4,4,BLACK);
      policy.updateProbabilitiesBeforeAction(&board, WHITE);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableWhite[board.xyToPoint(3,3)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableWhite[board.xyToPoint(3,5)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableWhite[board.xyToPoint(4,5)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableWhite[board.xyToPoint(5,3)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableWhite[board.xyToPoint(5,4)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableWhite[board.xyToPoint(5,5)]) < 0.001);
      EXPECT_TRUE(fabs(0 -  policy.m_probTableWhite[board.xyToPoint(4,4)]) < 0.001);
      EXPECT_TRUE(fabs(exp(0.5) - policy.m_probTableWhite[board.xyToPoint(0,3)]) < 0.001);
      EXPECT_TRUE(fabs(exp(0.5) - policy.m_probTableWhite[board.xyToPoint(0,4)]) < 0.001);
      EXPECT_TRUE(fabs(exp(0.5) - policy.m_probTableWhite[board.xyToPoint(0,5)]) < 0.001);
      EXPECT_TRUE(fabs(exp(0.5) - policy.m_probTableWhite[board.xyToPoint(2,5)]) < 0.001);
      EXPECT_TRUE(fabs(exp(0.5) - policy.m_probTableWhite[board.xyToPoint(1,5)]) < 0.001);
    }

    {
      // check EXTENSION_NEW_ATARI
      Board board(9);
      StandardFeatureExtractor extractor;
      std::vector<ML::PATTERN_3x3_HASH_VALUE> patterns;

      board.put(2,3,BLACK);
      board.put(2,2,WHITE);
      board.put(3,2,BLACK);
      board.put(1,3,WHITE);
      board.put(4,2,BLACK);
      board.put(2,4,WHITE);
      board.printToErr();

      // 0,0,0,0,0,0,0,0,0
      // 0,0,0,0,0,0,0,0,0
      // 0,0,2,1,1,0,0,0,0
      // 0,2,1,0,0,0,0,0,0
      // 0,0,2,0,0,0,0,0,0
      // 0,0,0,0,0,0,0,0,0
      // 0,0,0,0,0,0,0,0,0
      // 0,0,0,0,0,0,0,0,0
      // 0,0,0,0,0,0,0,0,0

      FeatureWeights weights(extractor.getFeatureDimension());
      weights[StandardFeatureExtractor::STATIC_FEATURE_DIST_FROM_BEFORE_PREVIOUS_IS_LESS_THAN_4] = 2;
      weights[StandardFeatureExtractor::STATIC_FEATURE_EXTENSION_NEW_ATARI] = 1;
      SoftmaxPolicyPlayout policy(&board, patterns, weights, Common::MTRandom::getInstance());

      policy.initProbabilities(&board, BLACK, policy.m_probTableBlack, policy.m_featureTableBlack);
      policy.updateProbabilitiesBeforeAction(&board, BLACK);
      EXPECT_TRUE(fabs(exp(2)*exp(1) - policy.m_probTableBlack[board.xyToPoint(3,3)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableBlack[board.xyToPoint(3,1)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableBlack[board.xyToPoint(4,1)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableBlack[board.xyToPoint(5,1)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableBlack[board.xyToPoint(5,2)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableBlack[board.xyToPoint(5,3)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableBlack[board.xyToPoint(4,3)]) < 0.001);
      EXPECT_TRUE(fabs(0 - policy.m_probTableBlack[board.xyToPoint(4,2)]) < 0.001);

      board.put(3,4,BLACK);
      policy.initProbabilities(&board, WHITE, policy.m_probTableWhite, policy.m_featureTableWhite);
      board.put(1,4,WHITE);
      policy.updateProbabilitiesBeforeAction(&board, BLACK);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableBlack[board.xyToPoint(3,3)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableBlack[board.xyToPoint(4,3)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableBlack[board.xyToPoint(4,4)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableBlack[board.xyToPoint(4,5)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableBlack[board.xyToPoint(3,5)]) < 0.001);
      EXPECT_TRUE(fabs(exp(2) - policy.m_probTableBlack[board.xyToPoint(2,5)]) < 0.001);
    }

    {
      // check ATARI, SELF-ATARI, CAPTURE features
      // TODO: 
      Board board(9);
      StandardFeatureExtractor extractor;
      std::vector<ML::PATTERN_3x3_HASH_VALUE> patterns;

      board.put(3,2,BLACK);
      board.put(0,0,WHITE);
      board.put(2,3,BLACK);
      board.put(1,1,WHITE);
      board.put(2,4,BLACK);
      board.put(1,0,WHITE);
      board.put(3,5,BLACK);
      board.put(3,4,WHITE);
      board.put(4,5,BLACK);
      board.put(4,4,WHITE);
      board.put(5,4,BLACK);
      board.printToErr();

      // 2,2,2,0,0,0,0,0,0
      // 0,0,0,0,0,0,0,0,0
      // 0,0,0,1,0,0,0,0,0
      // 0,0,1,0,0,0,0,0,0
      // 0,0,1,2,2,1,0,0,0
      // 0,0,0,1,1,0,0,0,0
      // 0,0,0,0,0,0,2,0,0
      // 0,0,0,0,0,0,0,0,0
      // 0,0,0,0,0,0,0,0,0

      FeatureWeights weights(extractor.getFeatureDimension());
      weights[StandardFeatureExtractor::STATIC_FEATURE_SELF_ATARI] = 1;
      weights[StandardFeatureExtractor::STATIC_FEATURE_CAPTURE] = 2;
      weights[StandardFeatureExtractor::STATIC_FEATURE_ATARI_OTHER_SITUATIONS] = 3;
      weights[StandardFeatureExtractor::STATIC_FEATURE_ATARI_WHEN_THERE_IS_A_KOU] = 4;
      SoftmaxPolicyPlayout policy(&board, patterns, weights, Common::MTRandom::getInstance());

      policy.initProbabilities(&board, WHITE, policy.m_probTableWhite, policy.m_featureTableWhite);
      EXPECT_TRUE(fabs(exp(1) - policy.m_probTableWhite[board.xyToPoint(3,3)]) < 0.001);
      EXPECT_TRUE(fabs(1 - policy.m_probTableWhite[board.xyToPoint(4,3)]) < 0.001);
      EXPECT_TRUE(fabs(1 - policy.m_probTableWhite[board.xyToPoint(2,2)]) < 0.001);
      EXPECT_TRUE(fabs(1 - policy.m_probTableWhite[board.xyToPoint(4,2)]) < 0.001);
      EXPECT_TRUE(fabs(0 - policy.m_probTableWhite[board.xyToPoint(3,4)]) < 0.001);
      EXPECT_TRUE(fabs(0 - policy.m_probTableWhite[board.xyToPoint(4,4)]) < 0.001);

      board.put(6,6,WHITE);
      policy.initProbabilities(&board, BLACK, policy.m_probTableBlack, policy.m_featureTableBlack);

      EXPECT_TRUE(fabs(exp(3) - policy.m_probTableBlack[board.xyToPoint(3,3)]) < 0.001);
      EXPECT_TRUE(fabs(exp(3) - policy.m_probTableBlack[board.xyToPoint(4,3)]) < 0.001);
      EXPECT_TRUE(fabs(0 - policy.m_probTableBlack[board.xyToPoint(4,4)]) < 0.001);
      EXPECT_TRUE(fabs(0 - policy.m_probTableBlack[board.xyToPoint(4,5)]) < 0.001);

      board.put(5,6,BLACK);
      policy.updateProbabilitiesBeforeAction(&board, WHITE);
      EXPECT_TRUE(fabs(exp(1) - policy.m_probTableWhite[board.xyToPoint(3,3)]) < 0.001);
      EXPECT_TRUE(fabs(1 - policy.m_probTableWhite[board.xyToPoint(4,3)]) < 0.001);
      EXPECT_TRUE(fabs(1 - policy.m_probTableWhite[board.xyToPoint(2,2)]) < 0.001);
      EXPECT_TRUE(fabs(1 - policy.m_probTableWhite[board.xyToPoint(4,2)]) < 0.001);
      EXPECT_TRUE(fabs(0 - policy.m_probTableWhite[board.xyToPoint(3,4)]) < 0.001);
      EXPECT_TRUE(fabs(0 - policy.m_probTableWhite[board.xyToPoint(4,4)]) < 0.001);

      board.put(6,4,WHITE);
      policy.updateProbabilitiesBeforeAction(&board, BLACK);
      board.put(6,5,BLACK);
      policy.updateProbabilitiesBeforeAction(&board, WHITE);
      EXPECT_TRUE(fabs(exp(1) - policy.m_probTableWhite[board.xyToPoint(3,3)]) < 0.001);
      EXPECT_TRUE(fabs(exp(3) - policy.m_probTableWhite[board.xyToPoint(5,3)]) < 0.001);
      EXPECT_TRUE(fabs(exp(3) - policy.m_probTableWhite[board.xyToPoint(7,5)]) < 0.001);
      board.put(7,5,WHITE);
      policy.updateProbabilitiesBeforeAction(&board, BLACK);
      EXPECT_TRUE(fabs(exp(3) - policy.m_probTableBlack[board.xyToPoint(3,3)]) < 0.001);
      EXPECT_TRUE(fabs(exp(3) - policy.m_probTableBlack[board.xyToPoint(4,3)]) < 0.001);
      EXPECT_TRUE(fabs(exp(3) - policy.m_probTableBlack[board.xyToPoint(7,4)]) < 0.001);
      EXPECT_TRUE(fabs(exp(3) - policy.m_probTableBlack[board.xyToPoint(7,6)]) < 0.001);
      board.put(1,7,BLACK);
      policy.updateProbabilitiesBeforeAction(&board, WHITE);
      EXPECT_TRUE(fabs(fmath::expd(2)*fmath::expd(3) - policy.m_probTableWhite[board.xyToPoint(5,5)]) < 0.001);
      EXPECT_TRUE(fabs(exp(1) - policy.m_probTableWhite[board.xyToPoint(3,3)]) < 0.001);
      EXPECT_TRUE(fabs(exp(3) - policy.m_probTableWhite[board.xyToPoint(5,3)]) < 0.001);
      EXPECT_TRUE(fabs(0 - policy.m_probTableWhite[board.xyToPoint(7,5)]) < 0.001);
      board.put(5,5,WHITE);
      policy.updateProbabilitiesBeforeAction(&board, BLACK);
      EXPECT_TRUE(fabs(fmath::expd(4) - policy.m_probTableBlack[board.xyToPoint(3,3)]) < 0.001);
      EXPECT_TRUE(fabs(fmath::expd(4) - policy.m_probTableBlack[board.xyToPoint(4,3)]) < 0.001);
      EXPECT_TRUE(fabs(0 - policy.m_probTableBlack[board.xyToPoint(6,5)]) < 0.001);
    }

  }

  void test_selectOneMoveAccordingToProbabilityTable() {
    Board board(9);
    std::vector<ML::PATTERN_3x3_HASH_VALUE> patterns;

    double table[MAX_BOARD_SIZE];
    for (int x=0; x<9; x++) {
      for (int y=0; y<9; y++) {
        table[board.xyToPoint(x,y)] = x*x + y;
      }
    }

    for (int rand_seed = 0; rand_seed <= 10; rand_seed++) {
      MTRandom::getInstance().init(rand_seed);
      MTRandom &rnd = MTRandom::getInstance();
      int expected_y = -1;
      double sum = 0;
      double rnd_1 = rnd.from_0_to_1();
      double rnd_2 = rnd.from_0_to_1();
      double yselectors [] = {
        204, 213, 222,231,240,249,258,267,276
      };
      for (int i=0; i<9; i++) {
        if (sum + yselectors[i]/2160 > rnd_1) {
          expected_y = i;
          break;
        }
        sum += yselectors[i]/2160;
      }
      if (rnd_1 == 1) expected_y = 8;

      int expected_x = -1;
      sum = 0;
      for (int i=0; i<9; i++) {
        if (sum + (i*i + expected_y)/yselectors[expected_y] > rnd_2) {
          expected_x = i;
          break;
        }
        sum += (i*i + expected_y)/yselectors[expected_y];
      }
      if (rnd_2 == 1) expected_x = 8;

      MTRandom::getInstance().init(rand_seed);

      StandardFeatureExtractor ex;
      FeatureWeights weights(ex.getFeatureDimension()); weights.clear();
      
      SoftmaxPolicyPlayout policy(&board, patterns, weights, Common::MTRandom::getInstance());


      Point p = policy.selectOneMoveAccordingToProbabilityTable(&board, table);
      std::pair<int,int> observed(board.pointToXY(p));
      EXPECT_EQ(expected_x, observed.first);
      EXPECT_EQ(expected_y, observed.second);
    }
  }
};

TEST_F(Test_SoftmaxPolicyPlayout, init) {
  test_init();
}

TEST_F(Test_SoftmaxPolicyPlayout, initProb) {
  test_initProb_emptyboard();
  test_initProb_randomboard();
}


// TEST_F(Test_SoftmaxPolicyPlayout, addUpdateCandidates) {
//   test_addUpdateCandidates();
// }


TEST_F(Test_SoftmaxPolicyPlayout, updateProbBeforeAction) {
  test_updateProbBeforeAction();
}

TEST_F(Test_SoftmaxPolicyPlayout, selectOneMoveAccordingToProbabilityTable) {
  test_selectOneMoveAccordingToProbabilityTable();
}
