/*
#include "../precomp.h"

#include "gtest/gtest.h"
#include <stdexcept>
#include "../AI/PureMCAI.h"
#include "../AI/RandomPlayer.h"
#include "../AI/NormalPlayout.h"
#include "../AI/UCTPlayer.h"
#include "../AI/UCTBoundCalculator.h"
#include "AI/SoftmaxPolicyPlayout.h"
#include "ML/StandardFeatureExtractor.h"

#include <ctime>
#include <execinfo.h>

using namespace std;
using namespace Common;
using namespace Go;
using namespace ML;

void battle(Board &board, AI::PlayerBase *players[])
{
  int turn = BLACK;
  int playerindex = 0;
  
  bool before_pass = false;

  int count = 0;
  while (true) {
    Point pos(players[playerindex]->selectBestMove(turn));

    int err = board.put(pos, turn);
    ASSERT_EQ(0, err);

    //board.printToErr();
    
    if (pos == PASS &&
        before_pass) {
      break;
    }

    turn = Board::flipColor(turn);
    playerindex = 1-playerindex;
    
    count++;
    before_pass = (pos == PASS);
  }

  double b, w;
  board.countScoreDetail(b, w);
  int win = board.countScore(BLACK);
  if (win > 0) {
    cerr << "Black wins: ";
  } else {
    cerr << "White wins: ";
  }
  cerr << "black_score(" << b << "), white_score(" << w << ")" << endl;
  int b_hama = board.getCapturedCount(BLACK);
  int w_hama = board.getCapturedCount(WHITE);
  cerr << "black_captured(" << b_hama << "), white_captured(" << w_hama << ")" << endl;
}
/*
TEST(AItest, uct_avoid_loop) {
  // check whether UCTPlayer avoids loop situation
  int init_array[] = {
    3,3,3,3,3,3,3,3,
    3,0,0,0,1,2,2,3,
    3,1,2,1,1,2,2,3,
    3,0,1,1,2,2,2,3,
    3,1,1,2,2,0,2,3,
    3,2,2,2,0,2,0,3,
    3,2,2,2,2,2,2,3,
    3,3,3,3,3,3,3,3,
  };
  Board board(6,init_array);
  ASSERT_EQ(Board::PUT_LEGAL, board.put(1,0,2));
  ASSERT_EQ(Board::PUT_LEGAL, board.put(0,0,1));
  ASSERT_EQ(Board::PUT_LEGAL, board.put(0,2,2));
  ASSERT_EQ(Board::PUT_LEGAL, board.put(0,1,1));
  ASSERT_EQ(Board::PUT_LEGAL, board.put(PASS,2));

  AI::Parameters params;
  params.setPlayoutLimit(1);
  AI::UCTPlayer<AI::NormalPlayout, AI::UCB_Calculator> uct(&board, AI::NormalPlayout(), params);
  try {
    for (int i=0; i<10; i++) {
      Point p = uct.selectBestMove(1);
      EXPECT_NE(board.xyToPoint(0,0), p);
    }
  } catch (exception &e) {
    const int maxTraces = 10240;
    void **traceBuffers = new void*[maxTraces];

    int numTraces = backtrace(traceBuffers, maxTraces);
    char **traceStrings = backtrace_symbols(traceBuffers, numTraces);
    for (int i=0; i<numTraces; i++) {
      std::cerr << traceStrings[i] << endl;
    }
    ASSERT_TRUE(false);
  }
}

*
TEST(AItest, MC_invalidMoveTest) {
  srand((unsigned)time(NULL));

  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,1,0,0,0,0,0,0,0,3,
    3,0,2,1,2,2,2,2,1,2,3,
    3,0,0,0,1,2,2,2,2,1,3,
    3,0,1,1,1,1,1,1,1,1,3,
    3,1,2,2,0,2,1,2,2,2,3,
    3,0,1,1,1,2,1,0,0,0,3,
    3,1,0,1,2,1,0,1,1,2,3,
    3,0,0,1,2,0,1,2,2,2,3,
    3,0,0,0,2,2,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  AI::PureMC<AI::NormalPlayout> ai(&board, AI::NormalPlayout(), BLACK);
  //ai.setPlayoutCountForEachHand(1);
  ai.setDebugLevel(1);

  Point pos(ai.selectBestMove(BLACK));

  int err = board.put(pos, BLACK);
  if (err != 0) {
    cerr << "turn:black" << " pos:" << board.pointToXY(pos).first << "," << board.pointToXY(pos).second << endl;
    board.print();
    cerr << "undo" << endl;
    board.undo();
    board.print();
  }
  EXPECT_EQ(0, err);

}

*/
/*
TEST(AItest, PlainUCTvsMC) {
  //srand((unsigned)time(NULL));
  srand(0);

  Board board(9);
  AI::UCTPlayer<AI::NormalPlayout, AI::UCB_Calculator> uct(&board, AI::NormalPlayout());
  uct.setDebugLevel(1);
  uct.setPlayoutLimit(1000);
  AI::PureMC<AI::NormalPlayout> mc(&board, AI::NormalPlayout());
  mc.setDebugLevel(1);
  mc.setPlayoutCountForEachHand(10);
  AI::PlayerBase *players[] = {&mc, &uct};

  battle(board, players);
}
*/
/*
TEST(AItest, PlainUCTvsMC_on_19) {
  srand((unsigned)time(NULL));

  Board board(19);
  AI::UCTPlayer<AI::NormalPlayout> uct(&board, AI::NormalPlayout());
  uct.setDebugLevel(1);
  uct.setPlayoutLimit(10);
  AI::PureMC<AI::NormalPlayout> mc(&board, AI::NormalPlayout());
  mc.setDebugLevel(1);
  mc.setPlayoutCountForEachHand(1);
  AI::PlayerBase *players[] = {&uct, &mc};

  battle(board, players);
}
*/
/*
TEST(AItest, PlainUCTvsSoftmaxUCT) {
  //unsigned int srand_seed = (unsigned)time(NULL);
  unsigned int srand_seed = 0;
  srand(srand_seed);
  cerr << "rand seed = " << srand_seed << endl;

  AI::Parameters params;

  Board board(9);
  StandardFeatureExtractor extractor;
  FeatureWeights init_weights(extractor.getFeatureDimension());
  vector<PATTERN_3x3_HASH_VALUE> patterns_null;
  init_weights[0] = 2.0;

  AI::SoftmaxPolicyPlayout softmaxPolicy(&board, patterns_null, init_weights, Common::MTRandom::getInstance());
  params.setPlayoutLimit(1000);
  AI::UCTPlayer<AI::SoftmaxPolicyPlayout, AI::UCB_Calculator> softmaxuct(&board, softmaxPolicy, params, Common::MTRandom::getInstance());
  softmaxuct.setDebugLevel(1);

  params.setPlayoutLimit(1000);
  AI::UCTPlayer<AI::NormalPlayout, AI::UCB_Calculator> plainuct(&board, AI::NormalPlayout(Common::MTRandom::getInstance()), params, Common::MTRandom::getInstance());
  plainuct.setDebugLevel(1);
  AI::PlayerBase *players[] = {&softmaxuct, &plainuct};

  battle(board, players);
}
*/
