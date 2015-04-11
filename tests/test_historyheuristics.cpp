
#include "../precomp.h"

#include "gtest/gtest.h"
#include <stdexcept>
#include "AI/HistoryHeuristics.h"
#include "../AI/HistoryHeuristicsPlayout.h"
#include "../AI/UCTPlayer.h"
#include "../AI/UCTBoundCalculator.h"

#include <ctime>
#include <execinfo.h>

using namespace std;
using namespace Common;
using namespace Go;
using namespace ML;
using namespace AI;

class Test_HistoryHeuristicsPlayout : public ::testing::Test {
protected:
  virtual void SetUp() {}
  virtual void TearDown() {}

  void testOnePoint() {
    Board board(9);
    HistoryHeuristics history(11*11,HistoryHeuristics::HISTOGRAM, 2);
    HistoryHeuristicsPlayout playout(&board, MTRandom::getInstance(), &history);

    for (int i=0; i<10; i++)
      history.recordHistory(board.xyToPoint(3,4), BLACK, 0);

    for (int i=0; i<6; i++)
      history.recordHistory(board.xyToPoint(7,3), WHITE, 1);

    for (int i=0; i<100; i++)
      history.recordHistory(board.xyToPoint(7,1), WHITE, 3);

    Color c = BLACK;
    for (int i=0; i<2; i++, c = Board::flipColor(c)) {
      for (int x=0; x<board.getSize(); x++) {
        for (int y=0; y<board.getSize(); y++) {
          Point p = board.xyToPoint(x,y);
          if (c == BLACK && x == 3 && y == 4) {
            EXPECT_EQ(10, history.getHistoryValue(p, c));
          } else if (c == WHITE && x == 7 && y == 3) {
            EXPECT_EQ(6, history.getHistoryValue(p, c));
          } else {
            if (0 != history.getHistoryValue(p, c)) {
              cerr << "x,y = " << x << "," << y << endl;
              EXPECT_EQ(0, history.getHistoryValue(p, c));
            }
          }
        }
      }
    }

    //playout.printProbabilityTableToErr(&board);

    for (int i=0; i<10; i++) {
      EXPECT_EQ(board.xyToPoint(3,4), playout.selectOneMoveAccordingToHistoryTable(&board, BLACK));
      EXPECT_EQ(board.xyToPoint(7,3), playout.selectOneMoveAccordingToHistoryTable(&board, WHITE));
    }
  }

  void testTwoPoint() {
    Board board(9);
    HistoryHeuristics history(11*11,HistoryHeuristics::HISTOGRAM, 2);
    HistoryHeuristicsPlayout playout(&board, MTRandom::getInstance(), &history);

    for (int i=0; i<10; i++)
      history.recordHistory(board.xyToPoint(3,4), BLACK, 0);

    for (int i=0; i<10; i++)
      history.recordHistory(board.xyToPoint(6,1), BLACK, 0);

    Color c = BLACK;
    for (int i=0; i<2; i++, c = Board::flipColor(c)) {
      for (int x=0; x<board.getSize(); x++) {
        for (int y=0; y<board.getSize(); y++) {
          Point p = board.xyToPoint(x,y);
          if (c == BLACK && ((x == 3 && y == 4) || (x == 6 && y == 1))) {
            EXPECT_EQ(10, history.getHistoryValue(p, c));
          } else {
            if (0 != history.getHistoryValue(p, c)) {
              cerr << "x,y = " << x << "," << y << endl;
              EXPECT_EQ(0, history.getHistoryValue(p, c));
            }
          }
        }
      }
    }

    //playout.printProbabilityTableToErr(&board);

    Point p1 = board.xyToPoint(3,4);
    Point p2 = board.xyToPoint(6,1);
    int p1count = 0, p2count = 0;
    for (int i=0; i<1000; i++) {
      Point pselected = playout.selectOneMoveAccordingToHistoryTable(&board, BLACK);
      if (pselected == p1) p1count++;
      else if (pselected == p2) p2count++;
      else {
        cerr << "x,y" << board.pointToXY(pselected).first << "," << board.pointToXY(pselected).second << endl;
        EXPECT_TRUE(pselected == p1 || pselected == p2);
      }
    }

    EXPECT_NE(0, p1count);
    EXPECT_NE(0, p2count);
  }

  virtual void testIllegalMove() {
    Board board(9);
    HistoryHeuristics history(11*11,HistoryHeuristics::HISTOGRAM, 2);
    HistoryHeuristicsPlayout playout(&board, MTRandom::getInstance(), &history);

    board.put(3,4,BLACK);
    history.recordHistory(board.xyToPoint(3,4), BLACK, 0);

    for (int i=0; i<100; i++) {
      EXPECT_NE(board.xyToPoint(3,4), playout.selectOneMoveAccordingToHistoryTable(&board, BLACK));
    }

    history.recordHistory(board.xyToPoint(4,7), BLACK, 0);
    for (int i=0; i<100; i++) {
      EXPECT_EQ(board.xyToPoint(4,7), playout.selectOneMoveAccordingToHistoryTable(&board, BLACK));
    }
  }

};

TEST_F(Test_HistoryHeuristicsPlayout, tmp) {
  Board board(9);
  HistoryHeuristics history(11*11);
  HistoryHeuristicsPlayout playout(&board, MTRandom::getInstance(), &history);

  board.put(3,3,BLACK);
  board.put(4,2,BLACK);
  board.put(5,3,BLACK);
  board.put(4,4,BLACK);
  board.printToErr();

  playout.printProbabilityTableToErr(&board);
}

TEST_F(Test_HistoryHeuristicsPlayout, clear) {
  Board board(9);
  HistoryHeuristics history(11*11);
  HistoryHeuristicsPlayout playout(&board, MTRandom::getInstance(), &history);
  
  history.recordHistory(board.xyToPoint(2,2), BLACK, 0);
  EXPECT_NE(0, history.getHistoryValue(board.xyToPoint(2,2), BLACK));

  history.clearHistory(WHITE);
  EXPECT_NE(0, history.getHistoryValue(board.xyToPoint(2,2), BLACK));
  history.clearHistory(BLACK);
  EXPECT_EQ(0, history.getHistoryValue(board.xyToPoint(2,2), BLACK));
}

TEST_F(Test_HistoryHeuristicsPlayout, onePoint) {
  testOnePoint();
}

TEST_F(Test_HistoryHeuristicsPlayout, twoPoint) {
  testTwoPoint();
}

TEST_F(Test_HistoryHeuristicsPlayout, illegalPoint) {
  testIllegalMove();
}
