#include "precomp.h"

#include "gtest/gtest.h"
#include <vector>
#include <stdexcept>

#include "ML/FeatureArray.h"
#include "ML/StandardFeatureExtractor.h"

using namespace std;
using namespace Common;
using namespace ML;
using namespace Go;

TEST(Test_StandardFeatureExtractor, init) {
  StandardFeatureExtractor extractor;

  Board board(9);

  vector<Point> moveSequence;

  moveSequence.push_back(board.xyToPoint(2,2)); // BLACK
  moveSequence.push_back(board.xyToPoint(4,5)); // WHITE
  moveSequence.push_back(board.xyToPoint(3,6)); // BLACK
  moveSequence.push_back(PASS); // WHITE
  moveSequence.push_back(board.xyToPoint(5,6)); // BLACK
  moveSequence.push_back(board.xyToPoint(5,5)); // WHITE

  // last state
  // 0,0,0,0,0,0,0,0,0
  // 0,0,0,0,0,0,0,0,0
  // 0,0,1,0,0,0,0,0,0
  // 0,0,0,0,0,0,0,0,0
  // 0,0,0,0,0,0,0,0,0
  // 0,0,0,0,2,2,0,0,0
  // 0,0,0,1,0,1,0,0,0
  // 0,0,0,0,0,0,0,0,0
  // 0,0,0,0,0,0,0,0,0 

  // Pre-process
  Color turn = BLACK;
  unordered_map<PATTERN_3x3_HASH_VALUE, int> hashToIndex;
  for (size_t i=0; i<moveSequence.size(); i++) {

    // extract patterns for all legal moves
    for (int x=0; x<9; x++) {
      for (int y=0; y<9; y++) {
        if (board.checkLegalHand(board.xyToPoint(x,y), turn, Board::flipColor(turn)) ==Board::PUT_LEGAL) {
          PATTERN_3x3_HASH_VALUE value = extractor.getPatternExtractor_size9().encode(board, board.xyToPoint(x,y), turn == WHITE);
          EXPECT_NE(PatternExtractor_3x3::INVALID_HASH_VALUE, value);
          hashToIndex[value] = extractor.registerPatternAsFeature(value);
        }
      }
    }
    
    ASSERT_EQ(Board::PUT_LEGAL, board.put(moveSequence[i], turn));
    turn = Board::flipColor(turn);
  }

  // set expected features
  board.clear();
  turn = BLACK;
  vector<shared_ptr<SparseVector> > expectedFeatures(6);
  // pattern features
  for (size_t i=0; i<expectedFeatures.size(); i++) {
    expectedFeatures[i] = shared_ptr<SparseVector>(new SparseVector());
    expectedFeatures[i]->clear();
    if (moveSequence[i] != PASS) {
      PATTERN_3x3_HASH_VALUE value = extractor.getPatternExtractor_size9().encode(board, moveSequence[i], turn == WHITE);
      EXPECT_NE(hashToIndex.end(), hashToIndex.find(value));
      expectedFeatures[i]->add(StandardFeatureExtractor::STATIC_FEATURE_SIZE + hashToIndex[value]);
    }
    EXPECT_EQ(Board::PUT_LEGAL, board.put(moveSequence[i], turn));
    turn = Board::flipColor(turn);
  }

  // static features
  expectedFeatures[2]->add(StandardFeatureExtractor::STATIC_FEATURE_DIST_FROM_LAST_IS_LESS_THAN_4);
  //expectedFeatures[3]->set(StandardFeatureExtractor::STATIC_FEATURE_CURRENT_MOVE_IS_PASS, 1);
  //expectedFeatures[4]->set(StandardFeatureExtractor::STATIC_FEATURE_PREVIOUS_MOVE_IS_PASS, 1);
  expectedFeatures[5]->add(StandardFeatureExtractor::STATIC_FEATURE_DIST_FROM_LAST_IS_LESS_THAN_4);

  // do move sequence and check
  board.clear();
  turn = BLACK;

  SparseVector featureTable[MAX_BOARD_SIZE]; PointSet legalMoves;
  SparseVector passFeature;

  // pattern features
  for (size_t i=0; i<moveSequence.size(); i++) {
    shared_ptr<SparseVector> expectedFeature = expectedFeatures[i];

    extractor.extractFromStateForAllMoves(&board, turn, featureTable, passFeature, legalMoves);
    SparseVector &result = featureTable[moveSequence[i]];

    //ASSERT_EQ(expectedFeature->size(), result.size());

    for (size_t j=0; j<expectedFeature->size(); j++) {
      EXPECT_TRUE(result.contains((*expectedFeature)[j]));
    }
    // for (size_t j=0; j<expectedFeature.size(); j++) {
    //   EXPECT_TRUE(expectedFeature.contains(result[j]));
    // }

    EXPECT_EQ(Board::PUT_LEGAL, board.put(moveSequence[i], turn));
    turn = Board::flipColor(turn);
  }
}

void checkSelfAtari(Board &board, StandardFeatureExtractor &extractor, vector<Point> &selfatari_pos) {
  SparseVector featureTable[MAX_BOARD_SIZE]; PointSet legalMoves;
  SparseVector passFeature;
  extractor.extractFromStateForAllMoves(&board, BLACK, featureTable, passFeature, legalMoves);

  for (size_t i=0; i<legalMoves.size(); i++) {
    Point p = legalMoves[i];
    int x = board.pointToXY(p).first;
    int y = board.pointToXY(p).second;

    ASSERT_EQ (Board::PUT_LEGAL, board.checkLegalHand(p,BLACK,WHITE));

    SparseVector &result = featureTable[p];

    if (find(selfatari_pos.begin(), selfatari_pos.end(), p) != selfatari_pos.end()) {
      bool is = result.contains(StandardFeatureExtractor::STATIC_FEATURE_SELF_ATARI);
      if (!is) cerr << "x,y = " << x << "," << y << endl;
      EXPECT_TRUE(is);
    } else {
      bool is = result.contains(StandardFeatureExtractor::STATIC_FEATURE_SELF_ATARI);
      if (is) cerr << "x,y = " << x << "," << y << endl;
      EXPECT_FALSE(is);
    }
  }
}

TEST(Test_StandardFeatureExtractor, selfAtari1) {
  {
    StandardFeatureExtractor extractor;
    vector<Point> moveSequence;

    // no capture
    int init_array[] = {
      3,3,3,3,3,3,3,3,3,3,3,
      3,0,2,2,2,0,0,0,0,0,3,
      3,2,1,1,1,2,0,0,0,0,3,
      3,2,1,0,0,2,0,0,0,0,3,
      3,0,2,1,1,2,0,0,0,0,3,
      3,0,1,2,2,2,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,3,3,3,3,3,3,3,3,3,3
    };
    Board board(9, init_array);

    vector<Point> selfataris;
    selfataris.push_back(board.xyToPoint(2,2));
    selfataris.push_back(board.xyToPoint(3,2));

    checkSelfAtari(board, extractor, selfataris);
  }
}

TEST(Test_StandardFeatureExtractor, selfAtari2) {
  {
    StandardFeatureExtractor extractor;
    vector<Point> moveSequence;

    // no capture
    int init_array[] = {
      3,3,3,3,3,3,3,3,3,3,3,
      3,0,2,0,2,0,0,0,0,0,3,
      3,2,1,0,1,2,0,0,0,0,3,
      3,2,1,0,1,2,0,0,0,0,3,
      3,0,2,1,1,2,0,0,0,0,3,
      3,0,1,2,2,2,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,3,3,3,3,3,3,3,3,3,3
    };
    Board board(9, init_array);

    vector<Point> poses;
    poses.push_back(board.xyToPoint(2,2));

    checkSelfAtari(board, extractor, poses);
  }
}

TEST(Test_StandardFeatureExtractor, selfAtari3) {
  {
    StandardFeatureExtractor extractor;
    vector<Point> moveSequence;

    // no capture
    int init_array[] = {
      3,3,3,3,3,3,3,3,3,3,3,
      3,0,2,2,2,0,0,0,0,0,3,
      3,2,2,1,1,2,2,0,0,0,3,
      3,2,2,0,2,1,2,0,0,0,3,
      3,0,2,1,1,1,2,0,0,0,3,
      3,0,1,2,2,2,2,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,3,3,3,3,3,3,3,3,3,3
    };
    Board board(9, init_array);

    vector<Point> poses;
    poses.push_back(board.xyToPoint(2,2));

    checkSelfAtari(board, extractor, poses);
  }
}

TEST(Test_StandardFeatureExtractor, selfAtari4) {
  {
    StandardFeatureExtractor extractor;
    vector<Point> moveSequence;

    // no capture
    int init_array[] = {
      3,3,3,3,3,3,3,3,3,3,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,1,2,0,0,0,0,0,3,
      3,2,1,2,1,2,0,0,0,0,3,
      3,2,2,0,1,2,0,0,0,0,3,
      3,0,2,1,1,2,0,0,0,0,3,
      3,0,1,2,2,2,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,3,3,3,3,3,3,3,3,3,3
    };
    Board board(9, init_array);

    vector<Point> poses;
    poses.push_back(board.xyToPoint(2,3));

    checkSelfAtari(board, extractor, poses);

  }
}

TEST(Test_StandardFeatureExtractor, selfAtari5) {
  {
    StandardFeatureExtractor extractor;
    vector<Point> moveSequence;

    // no capture
    int init_array[] = {
      3,3,3,3,3,3,3,3,3,3,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,1,2,0,0,0,0,0,3,
      3,2,1,2,1,2,0,0,0,0,3,
      3,2,2,0,1,2,0,0,0,0,3,
      3,0,2,1,1,2,0,0,0,0,3,
      3,0,1,2,2,2,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,3,3,3,3,3,3,3,3,3,3
    };
    Board board(9, init_array);

    vector<Point> poses;
    poses.push_back(board.xyToPoint(2,3));

    checkSelfAtari(board, extractor, poses);
  }
}

TEST(Test_StandardFeatureExtractor, selfAtari6) {
  StandardFeatureExtractor extractor;
  vector<Point> moveSequence;

  // no capture
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,2,1,2,0,0,0,0,0,3,
    3,1,1,1,2,1,0,0,0,0,3,
    3,2,2,2,1,1,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  vector<Point> poses;
  poses.push_back(board.xyToPoint(0,0));
  
  checkSelfAtari(board, extractor, poses);

}

void checkNotSelfAtari(Board &board, StandardFeatureExtractor &extractor) {
  SparseVector featureTable[MAX_BOARD_SIZE]; PointSet legalMoves;
  SparseVector passFeature;
  extractor.extractFromStateForAllMoves(&board, BLACK, featureTable, passFeature, legalMoves);

  for (size_t i=0; i<legalMoves.size(); i++) {
    Point p = legalMoves[i];
    int x = board.pointToXY(p).first;
    int y = board.pointToXY(p).second;

    ASSERT_EQ (Board::PUT_LEGAL, board.checkLegalHand(p,BLACK,WHITE));

    SparseVector &result = featureTable[p];

    bool is = result.contains(StandardFeatureExtractor::STATIC_FEATURE_SELF_ATARI);
    if (is) cerr << "x,y = " << x << "," << y << endl;
    EXPECT_FALSE(is);
  }
}

TEST(Test_StandardFeatureExtractor, notSelfAtari1) {
  {
    StandardFeatureExtractor extractor;

    // no capture
    int init_array[] = {
      3,3,3,3,3,3,3,3,3,3,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,1,2,0,0,0,0,0,3,
      3,2,1,2,1,2,0,0,0,0,3,
      3,2,1,0,1,2,0,0,0,0,3,
      3,0,2,1,1,2,0,0,0,0,3,
      3,0,1,2,2,2,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,3,3,3,3,3,3,3,3,3,3
    };
    Board board(9, init_array);

    checkNotSelfAtari(board, extractor);
  }
}

TEST(Test_StandardFeatureExtractor, notSelfAtari2) {
  {
    StandardFeatureExtractor extractor;

    // no capture
    int init_array[] = {
      3,3,3,3,3,3,3,3,3,3,3,
      3,2,2,2,2,0,0,0,0,0,3,
      3,2,1,1,1,2,0,0,0,0,3,
      3,2,1,2,1,2,0,0,0,0,3,
      3,2,2,0,1,2,0,0,0,0,3,
      3,2,1,1,2,0,0,0,0,0,3,
      3,2,1,2,2,2,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,3,3,3,3,3,3,3,3,3,3
    };
    Board board(9, init_array);

    checkNotSelfAtari(board, extractor);
  }
}

void checkAtari(Board &board, StandardFeatureExtractor &extractor, vector<Point> &thereiskouPoses, vector<Point> &othersituations) {
  SparseVector featureTable[MAX_BOARD_SIZE]; PointSet legalMoves;
  SparseVector passFeature;
  extractor.extractFromStateForAllMoves(&board, BLACK, featureTable, passFeature, legalMoves);

  for (size_t i=0; i<legalMoves.size(); i++) {
    Point p = legalMoves[i];
    int x = board.pointToXY(p).first;
    int y = board.pointToXY(p).second;

    ASSERT_EQ (Board::PUT_LEGAL, board.checkLegalHand(p,BLACK,WHITE));

    SparseVector &result = featureTable[p];

    if (find(thereiskouPoses.begin(), thereiskouPoses.end(), p) != thereiskouPoses.end()) {
      bool is = result.contains(StandardFeatureExtractor::STATIC_FEATURE_ATARI_WHEN_THERE_IS_A_KOU);
      if (!is) cerr << "x,y = " << x << "," << y << endl;
      EXPECT_TRUE(is);
      
      is = result.contains(StandardFeatureExtractor::STATIC_FEATURE_ATARI_OTHER_SITUATIONS);
      if (is) cerr << "x,y = " << x << "," << y << endl;
      EXPECT_FALSE(is);
    }
    else if (find(othersituations.begin(), othersituations.end(), p) != othersituations.end()) {
      bool is = result.contains(StandardFeatureExtractor::STATIC_FEATURE_ATARI_WHEN_THERE_IS_A_KOU);
      if (is) cerr << "x,y = " << x << "," << y << endl;
      EXPECT_FALSE(is);
      
      is = result.contains(StandardFeatureExtractor::STATIC_FEATURE_ATARI_OTHER_SITUATIONS);
      if (!is) cerr << "x,y = " << x << "," << y << endl;
      EXPECT_TRUE(is);
    } else {
      bool is = result.contains(StandardFeatureExtractor::STATIC_FEATURE_ATARI_WHEN_THERE_IS_A_KOU);
      if (is) cerr << "x,y = " << x << "," << y << endl;
      EXPECT_FALSE(is);
      
        is = result.contains(StandardFeatureExtractor::STATIC_FEATURE_ATARI_OTHER_SITUATIONS);
        if (is) cerr << "x,y = " << x << "," << y << endl;
        EXPECT_FALSE(is);
    }

    bool is = result.contains(StandardFeatureExtractor::STATIC_FEATURE_SELF_ATARI);
    if (is) cerr << "x,y = " << x << "," << y << endl;
    EXPECT_FALSE(is);
  }
}

TEST(Test_StandardFeatureExtractor, checkAtari1) {
  StandardFeatureExtractor extractor;
  vector<Point> moveSequence;

  // no capture
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,1,0,0,0,0,0,3,
    3,0,0,1,2,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  vector<Point> kous;
  vector<Point> others;
  others.push_back(board.xyToPoint(3,5));
  others.push_back(board.xyToPoint(4,4));

  checkAtari(board, extractor, kous, others);
}

TEST(Test_StandardFeatureExtractor, checkAtari2) {
  StandardFeatureExtractor extractor;
  vector<Point> moveSequence;

  // no capture
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,2,1,0,0,0,3,
    3,0,0,0,2,1,0,1,0,0,3,
    3,0,0,0,0,2,1,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,1,1,0,0,0,0,0,3,
    3,0,1,2,2,1,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  board.put(5,2,WHITE);


  vector<Point> kous;
  vector<Point> others;
  kous.push_back(board.xyToPoint(2,8));
  kous.push_back(board.xyToPoint(3,8));

  checkAtari(board, extractor, kous, others);
}

TEST(Test_StandardFeatureExtractor, checkAtari3) {
  StandardFeatureExtractor extractor;
  vector<Point> moveSequence;

  // no capture
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,2,1,0,0,0,3,
    3,0,0,0,2,0,2,1,0,0,3,
    3,0,0,0,0,2,1,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,1,1,0,0,0,0,0,3,
    3,0,1,2,2,1,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  board.put(4,2,BLACK);
  board.put(3,3,WHITE);

  vector<Point> kous;
  vector<Point> others;
  others.push_back(board.xyToPoint(2,8));
  others.push_back(board.xyToPoint(3,8));
  others.push_back(board.xyToPoint(4,0));
  others.push_back(board.xyToPoint(3,1));

  checkAtari(board, extractor, kous, others);
}

void checkReCapture(Board &board, StandardFeatureExtractor &extractor, Point action, Color turn) {
  StandardFeatureExtractor::NeighborBlockSetCache myCache[MAX_BOARD_SIZE], enemyCache[MAX_BOARD_SIZE];
  for (int x=0; x<9; x++) {
    for (int y=0; y<9; y++) {
      Point p = board.xyToPoint(x,y);
      if (board.checkLegalHand(p,turn,Board::flipColor(turn)) != Board::PUT_LEGAL) continue;
      SparseVector result(extractor.getFeatureDimension());
      bool is = extractor.isRecapture(&board, p, turn, myCache, enemyCache);
      //extractor.extractFromStateAndAction(result, &board, p, turn);

      if (action == p) {
        //bool is = result.contains(StandardFeatureExtractor::STATIC_FEATURE_CAPTURE_RE_CAPTURE);
        if (!is) cerr << "x,y = " << x << "," << y << endl;
        EXPECT_TRUE(is);

        EXPECT_TRUE(extractor.isCapture(&board, p, turn, myCache, enemyCache)); // if Re-Capture feature is on, Capture feature must be on
      } else {
        //bool is = result.contains(StandardFeatureExtractor::STATIC_FEATURE_CAPTURE_RE_CAPTURE);
        if (is) cerr << "x,y = " << x << "," << y << endl;
        EXPECT_FALSE(is);
      }
    }
  }
}


TEST(Test_StandardFeatureExtractor, checkReCapture1) {
  StandardFeatureExtractor extractor;
  vector<Point> moveSequence;

  // no capture
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,2,1,1,0,0,0,3,
    3,0,0,2,0,2,2,1,0,0,3,
    3,0,0,0,2,1,1,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  board.put(3,2,BLACK);

  checkReCapture(board, extractor, board.xyToPoint(4,2), WHITE);
}

TEST(Test_StandardFeatureExtractor, checkReCapture2) {
  StandardFeatureExtractor extractor;
  vector<Point> moveSequence;

  // no capture
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,2,1,1,0,0,0,3,
    3,0,0,2,0,2,1,1,0,0,3,
    3,0,0,0,2,1,1,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  board.put(3,2,BLACK);

  checkReCapture(board, extractor, POINT_NULL, WHITE);
}

TEST(Test_StandardFeatureExtractor, checkReCapture3) {
  StandardFeatureExtractor extractor;
  vector<Point> moveSequence;

  // no capture
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,2,1,1,0,0,0,3,
    3,0,0,2,0,2,1,1,0,0,3,
    3,0,0,0,2,1,1,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,2,0,0,0,3,
    3,0,0,0,0,0,1,2,0,0,3,
    3,0,0,0,0,0,2,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  board.put(3,2,BLACK);

  checkReCapture(board, extractor, POINT_NULL, WHITE);

  StandardFeatureExtractor::NeighborBlockSetCache myCache[MAX_BOARD_SIZE], enemyCache[MAX_BOARD_SIZE];
  EXPECT_TRUE(StandardFeatureExtractor::isCapture(&board, board.xyToPoint(4,6), WHITE, myCache, enemyCache));
  EXPECT_FALSE(StandardFeatureExtractor::isCapture(&board, board.xyToPoint(4,2), WHITE, myCache, enemyCache));
}

TEST(Test_StandardFeatureExtractor, enumerateMovesForCaptureStringToNewAtariFeature1) {
  StandardFeatureExtractor extractor;

  // no capture
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,2,1,0,0,0,0,0,0,3,
    3,1,1,1,2,1,0,0,0,0,3,
    3,2,2,2,1,1,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  board.put(3,0,WHITE);

  PointSet moves, extensions;
  StandardFeatureExtractor::NeighborBlockSetCache myCache[MAX_BOARD_SIZE], enemyCache[MAX_BOARD_SIZE];
  extractor.enumerateMovesForCaptureStringToNewAtariFeatureAndExtensionFeature(&board, BLACK, moves, extensions, myCache, enemyCache);

  EXPECT_EQ((unsigned)2, moves.size());
  EXPECT_TRUE(moves.contains(board.xyToPoint(0,0)));
  EXPECT_TRUE(extractor.isCapture(&board, board.xyToPoint(0,0), BLACK, myCache, enemyCache));
  EXPECT_TRUE(moves.contains(board.xyToPoint(4,0)));
  EXPECT_TRUE(extractor.isCapture(&board, board.xyToPoint(4,0), BLACK, myCache, enemyCache));
  EXPECT_EQ((unsigned)0, extensions.size());
}

TEST(Test_StandardFeatureExtractor, enumerateMovesForCaptureStringToNewAtariFeature2) {
  StandardFeatureExtractor extractor;

  // no capture
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,2,1,2,1,0,0,3,
    3,0,0,0,2,1,1,0,1,0,3,
    3,0,0,0,0,2,2,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  board.put(6,3,WHITE);

  PointSet moves, extensions;
  StandardFeatureExtractor::NeighborBlockSetCache myCache[MAX_BOARD_SIZE], enemyCache[MAX_BOARD_SIZE];
  extractor.enumerateMovesForCaptureStringToNewAtariFeatureAndExtensionFeature(&board, BLACK, moves, extensions, myCache, enemyCache);

  EXPECT_EQ((unsigned)2, moves.size());
  EXPECT_TRUE(moves.contains(board.xyToPoint(5,1)));
  EXPECT_TRUE(extractor.isCapture(&board, board.xyToPoint(5,1), BLACK, myCache, enemyCache));
  EXPECT_TRUE(moves.contains(board.xyToPoint(6,4)));
  EXPECT_TRUE(extractor.isCapture(&board, board.xyToPoint(6,4), BLACK, myCache, enemyCache));
  EXPECT_EQ((unsigned)1, extensions.size());
  EXPECT_TRUE(extensions.contains(board.xyToPoint(4,1)));
}

TEST(Test_StandardFeatureExtractor, enumerateMovesForCaptureStringToNewAtariFeature3) {
  StandardFeatureExtractor extractor;

  // no capture
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,2,1,0,1,0,0,3,
    3,0,0,0,2,1,1,0,0,0,3,
    3,0,0,0,0,2,2,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  board.put(6,3,WHITE);

  PointSet moves, extensions;
  StandardFeatureExtractor::NeighborBlockSetCache myCache[MAX_BOARD_SIZE], enemyCache[MAX_BOARD_SIZE];
  extractor.enumerateMovesForCaptureStringToNewAtariFeatureAndExtensionFeature(&board, BLACK, moves, extensions, myCache, enemyCache);

  EXPECT_EQ((unsigned)0, moves.size());
  EXPECT_EQ((unsigned)0, extensions.size());
}

TEST(Test_StandardFeatureExtractor, doubleExtensions) {
  StandardFeatureExtractor extractor;

  // no capture
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,0,0,0,0,2,2,0,0,3,
    3,0,0,0,2,2,1,1,2,0,3,
    3,0,0,0,2,1,0,0,1,0,3,
    3,0,0,0,2,1,2,2,0,0,3,
    3,0,0,0,2,1,0,0,0,0,3,
    3,0,0,0,0,2,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  board.put(5,2,WHITE);

  PointSet moves, extensions;
  StandardFeatureExtractor::NeighborBlockSetCache myCache[MAX_BOARD_SIZE], enemyCache[MAX_BOARD_SIZE];
  extractor.enumerateMovesForCaptureStringToNewAtariFeatureAndExtensionFeature(&board, BLACK, moves, extensions, myCache, enemyCache);

  EXPECT_EQ((unsigned)0, moves.size());
  EXPECT_EQ((unsigned)2, extensions.size());
  EXPECT_TRUE(extensions.contains(board.xyToPoint(5,4)));
  EXPECT_TRUE(extensions.contains(board.xyToPoint(6,2)));

  EXPECT_FALSE(extractor.isSelfAtari(&board, board.xyToPoint(5,4), BLACK, myCache, enemyCache));
  EXPECT_FALSE(extractor.isSelfAtari(&board, board.xyToPoint(6,2), BLACK, myCache, enemyCache));
}

TEST(Test_StandardFeatureExtractor, merginExtensions) {
  StandardFeatureExtractor extractor;

  // no capture
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,2,1,0,0,0,0,3,
    3,0,0,0,2,0,2,0,0,0,3,
    3,0,0,0,2,1,0,0,0,0,3,
    3,0,0,0,0,2,2,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  board.put(5,3,WHITE);

  PointSet moves, extensions;
  StandardFeatureExtractor::NeighborBlockSetCache myCache[MAX_BOARD_SIZE], enemyCache[MAX_BOARD_SIZE];
  extractor.enumerateMovesForCaptureStringToNewAtariFeatureAndExtensionFeature(&board, BLACK, moves, extensions, myCache, enemyCache);

  EXPECT_EQ((unsigned)0, moves.size());
  EXPECT_EQ((unsigned)1, extensions.size());
  EXPECT_TRUE(extensions.contains(board.xyToPoint(4,2)));

  EXPECT_FALSE(extractor.isSelfAtari(&board, board.xyToPoint(4,2), BLACK, myCache, enemyCache));
}

TEST(Test_StandardFeatureExtractor, notExtensionButAtari1) {
  StandardFeatureExtractor extractor;

  // no capture
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,2,0,0,0,0,3,
    3,0,0,0,2,0,0,0,0,0,3,
    3,0,0,0,2,1,0,0,0,0,3,
    3,0,0,0,0,2,2,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  board.put(5,3,WHITE);

  PointSet moves, extensions;
  StandardFeatureExtractor::NeighborBlockSetCache myCache[MAX_BOARD_SIZE], enemyCache[MAX_BOARD_SIZE];
  extractor.enumerateMovesForCaptureStringToNewAtariFeatureAndExtensionFeature(&board, BLACK, moves, extensions, myCache, enemyCache);

  EXPECT_EQ((unsigned)0, moves.size());
  EXPECT_EQ((unsigned)0, extensions.size());

  EXPECT_TRUE(extractor.isSelfAtari(&board, board.xyToPoint(4,2), BLACK, myCache, enemyCache));

  board.undo();
  board.put(4,2,WHITE);
  extractor.enumerateMovesForCaptureStringToNewAtariFeatureAndExtensionFeature(&board, BLACK, moves, extensions, myCache, enemyCache);
  EXPECT_EQ((unsigned)0, moves.size());
  EXPECT_EQ((unsigned)1, extensions.size());
  EXPECT_TRUE(extensions.contains(board.xyToPoint(5,3)));
}

TEST(Test_StandardFeatureExtractor, notExtensionButAtari2) {
  StandardFeatureExtractor extractor;

  // no capture
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,2,2,0,0,0,0,3,
    3,0,0,0,1,0,2,0,0,0,3,
    3,0,0,0,0,1,2,0,0,0,3,
    3,0,0,0,0,2,2,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  board.put(3,3,WHITE);

  PointSet moves, extensions;
  StandardFeatureExtractor::NeighborBlockSetCache myCache[MAX_BOARD_SIZE], enemyCache[MAX_BOARD_SIZE];
  extractor.enumerateMovesForCaptureStringToNewAtariFeatureAndExtensionFeature(&board, BLACK, moves, extensions, myCache, enemyCache);

  EXPECT_EQ((unsigned)0, moves.size());
  EXPECT_EQ((unsigned)0, extensions.size());

  EXPECT_TRUE(extractor.isSelfAtari(&board, board.xyToPoint(4,2), BLACK, myCache, enemyCache));
}

TEST(Test_StandardFeatureExtractor, notExtensionButAtari3) {
  StandardFeatureExtractor extractor;

  // no capture
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,0,0,0,2,0,0,0,0,3,
    3,0,0,0,2,0,2,0,0,0,3,
    3,0,0,0,2,0,2,0,0,0,3,
    3,0,0,0,0,1,2,0,0,0,3,
    3,0,0,0,0,2,2,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  board.put(3,3,WHITE);

  PointSet moves, extensions;
  StandardFeatureExtractor::NeighborBlockSetCache myCache[MAX_BOARD_SIZE], enemyCache[MAX_BOARD_SIZE];
  extractor.enumerateMovesForCaptureStringToNewAtariFeatureAndExtensionFeature(&board, BLACK, moves, extensions, myCache, enemyCache);

  EXPECT_EQ((unsigned)0, moves.size());
  EXPECT_EQ((unsigned)0, extensions.size());

  EXPECT_TRUE(extractor.isSelfAtari(&board, board.xyToPoint(4,2), BLACK, myCache, enemyCache));
}

TEST(Test_StandardFeatureExtractor, notExtensionButAtari4) {
  StandardFeatureExtractor extractor;

  // no capture
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,0,0,0,1,1,0,0,0,3,
    3,0,0,0,0,2,2,1,0,0,3,
    3,0,0,0,1,0,2,1,0,0,3,
    3,0,0,0,1,2,1,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  board.put(4,4,BLACK);

  PointSet moves, extensions;
  StandardFeatureExtractor::NeighborBlockSetCache myCache[MAX_BOARD_SIZE], enemyCache[MAX_BOARD_SIZE];
  extractor.enumerateMovesForCaptureStringToNewAtariFeatureAndExtensionFeature(&board, WHITE, moves, extensions, myCache, enemyCache);

  EXPECT_EQ((unsigned)0, moves.size());
  EXPECT_EQ((unsigned)0, extensions.size());

  EXPECT_TRUE(extractor.isSelfAtari(&board, board.xyToPoint(4,2), WHITE, myCache, enemyCache));
}

TEST(Test_StandardFeatureExtractor, notExtensionAndNotNewAtari1) {
  StandardFeatureExtractor extractor;

  // no capture
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,0,0,0,2,0,0,0,0,3,
    3,0,0,0,2,0,2,0,0,0,3,
    3,0,0,0,2,0,2,0,0,0,3,
    3,0,0,0,0,1,2,0,0,0,3,
    3,0,0,0,0,2,2,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  board.put(4,2,WHITE);

  PointSet moves, extensions;
  StandardFeatureExtractor::NeighborBlockSetCache myCache[MAX_BOARD_SIZE], enemyCache[MAX_BOARD_SIZE];
  extractor.enumerateMovesForCaptureStringToNewAtariFeatureAndExtensionFeature(&board, BLACK, moves, extensions, myCache, enemyCache);

  EXPECT_EQ((unsigned)0, moves.size());
  EXPECT_EQ((unsigned)1, extensions.size());
  EXPECT_TRUE(extensions.contains(board.xyToPoint(3,3)));

  board.put(0,0,BLACK);
  EXPECT_TRUE(extractor.isCapture(&board, board.xyToPoint(3,3), WHITE, myCache, enemyCache));

  board.put(1,1,WHITE);
  moves.clear(); extensions.clear();
  extractor.enumerateMovesForCaptureStringToNewAtariFeatureAndExtensionFeature(&board, BLACK, moves, extensions, myCache, enemyCache);
  EXPECT_EQ((unsigned)0, moves.size());
  EXPECT_EQ((unsigned)0, extensions.size());
}
