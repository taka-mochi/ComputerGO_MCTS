#include "precomp.h"

#include "gtest/gtest.h"
#include <stdexcept>

#include "ML/Board_Move_Dataset_record.h"

using namespace std;
using namespace Common;
using namespace ML;
using namespace Go;

void check_single_record(Board_Move_Dataset_record &dataset, const vector<Point> &expectedPoints, int boardsize, Color init_turn, bool isLastRecord) {
  Board board(boardsize);

  const Board_Move_Data &init_data = dataset.get();
  shared_ptr<Board> init_board = init_data.state;
  for (int x=0; x<9; x++) {
    for (int y=0; y<9; y++) {
      EXPECT_TRUE(init_board->isColor(x,y,FREE));
    }
  }

  Color turn = init_turn;
  for (size_t i=0; i<expectedPoints.size(); i++) {
    EXPECT_FALSE(dataset.isEndOfData());
    const Board_Move_Data &movedata = dataset.get();
    // move verity
    EXPECT_EQ(expectedPoints[i], movedata.move);
    pair<int,int> expected(board.pointToXY(expectedPoints[i])), move(board.pointToXY(movedata.move));
    //cerr << "expected move = " << expected.first << "," << expected.second << " observed = " << move.first << "," << move.second << endl;
    // board state verity
    EXPECT_NE(shared_ptr<Board>(), movedata.state);
    if (movedata.state && i>=1 && expectedPoints[i-1] != PASS) {
      EXPECT_EQ(Board::flipColor(turn), movedata.state->getStone(expectedPoints[i-1]));
    }

    // iteration verity
    if (i+1<expectedPoints.size()) {
      EXPECT_TRUE(dataset.next());
    } else {
      if (isLastRecord) {
        EXPECT_FALSE(dataset.next());
      } else {
        EXPECT_TRUE(dataset.next());
      }
    }

    turn = Board::flipColor(turn);
  }
  if (isLastRecord) {
    EXPECT_TRUE(dataset.isEndOfData());
  } else {
    EXPECT_FALSE(dataset.isEndOfData());
  }

}

TEST(test_Board_Move_Dataset_record, read_record) {
  Board_Move_Dataset_record dataset;

  EXPECT_TRUE(dataset.readAndAddRecordFromSGF("Record/sgf_for_test/manually_test_simple.sgf"));

  EXPECT_FALSE(dataset.empty());

  vector<Point> expectedPoints;
  Board board(9);
  // ;B[gh];W[dd];B[fh];W[cd];B[hf];W[cc];B[bh];W[];B[gf];W[gi];B[tt];)
  expectedPoints.push_back(board.xyToPoint(6,7));
  expectedPoints.push_back(board.xyToPoint(3,3));
  expectedPoints.push_back(board.xyToPoint(5,7));
  expectedPoints.push_back(board.xyToPoint(2,3));
  expectedPoints.push_back(board.xyToPoint(7,5));
  expectedPoints.push_back(board.xyToPoint(2,2));
  expectedPoints.push_back(board.xyToPoint(1,7));
  expectedPoints.push_back(PASS);
  expectedPoints.push_back(board.xyToPoint(6,5));
  expectedPoints.push_back(board.xyToPoint(6,8));
  expectedPoints.push_back(PASS);

  // There are 11 legal moves in the test file: dataset.next() must return true 10 times, dataset.isEndOfData() must return false 11 times
  check_single_record(dataset, expectedPoints, 9, BLACK, true);

  dataset.seekToBegin();
  EXPECT_FALSE(dataset.isEndOfData());

  check_single_record(dataset, expectedPoints, 9, BLACK, true);
}

TEST(test_Board_Move_Dataset_record, no_record_access) {
  Board_Move_Dataset_record dataset;

  EXPECT_TRUE(dataset.empty());
  EXPECT_TRUE(dataset.isEndOfData());
  EXPECT_FALSE(dataset.next());
  const Board_Move_Data &data = dataset.get();
  EXPECT_EQ(shared_ptr<Board>(), data.state);
}

TEST(test_Board_Move_Dataset_record, read_multi_records) {
  Board_Move_Dataset_record dataset;
  Board board19(19), board9(9);

  EXPECT_TRUE(dataset.readAndAddRecordFromSGF("Record/sgf_for_test/manually_test_simple.sgf"));
  EXPECT_TRUE(dataset.readAndAddRecordFromSGF("Record/sgf_for_test/manually_test_simple_2.sgf"));
  EXPECT_TRUE(dataset.readAndAddRecordFromSGF("Record/sgf_for_test/manually_test_simple_3.sgf"));

  EXPECT_FALSE(dataset.empty());

  vector<Point> expectedPoints1;
  // ;B[gh];W[dd];B[fh];W[cd];B[hf];W[cc];B[bh];W[];B[gf];W[gi];B[tt];)
  expectedPoints1.push_back(board9.xyToPoint(6,7));
  expectedPoints1.push_back(board9.xyToPoint(3,3));
  expectedPoints1.push_back(board9.xyToPoint(5,7));
  expectedPoints1.push_back(board9.xyToPoint(2,3));
  expectedPoints1.push_back(board9.xyToPoint(7,5));
  expectedPoints1.push_back(board9.xyToPoint(2,2));
  expectedPoints1.push_back(board9.xyToPoint(1,7));
  expectedPoints1.push_back(PASS);
  expectedPoints1.push_back(board9.xyToPoint(6,5));
  expectedPoints1.push_back(board9.xyToPoint(6,8));
  expectedPoints1.push_back(PASS);

  // There are 11 legal moves in the first test file
  cerr << "---- begin test record 1 ----" << endl;
  check_single_record(dataset, expectedPoints1, 9, BLACK, false);

  vector<Point> expectedPoints2;
  expectedPoints2.push_back(board19.xyToPoint(6,7));
  expectedPoints2.push_back(board19.xyToPoint(3,5));
  expectedPoints2.push_back(board19.xyToPoint(5,7));
  expectedPoints2.push_back(board19.xyToPoint(2,3));
  expectedPoints2.push_back(board19.xyToPoint(10,5));
  expectedPoints2.push_back(board19.xyToPoint(2,2));
  expectedPoints2.push_back(board19.xyToPoint(1,7));
  expectedPoints2.push_back(board19.xyToPoint(6,12));
  expectedPoints2.push_back(PASS);

  // There are 9 legal moves in the second test file
  cerr << "---- begin test record 2 ----" << endl;
  check_single_record(dataset, expectedPoints2, 19, BLACK, false);

  // p:15, q:16
  vector<Point> expectedPoints3;
  expectedPoints3.push_back(board19.xyToPoint(16,15));
  expectedPoints3.push_back(board19.xyToPoint(3,3));
  expectedPoints3.push_back(board19.xyToPoint(5,16));
  expectedPoints3.push_back(board19.xyToPoint(2,13));
  expectedPoints3.push_back(board19.xyToPoint(16,5));
  expectedPoints3.push_back(board19.xyToPoint(13,2));
  expectedPoints3.push_back(board19.xyToPoint(11,16));
  expectedPoints3.push_back(board19.xyToPoint(16,4));
  expectedPoints3.push_back(board19.xyToPoint(15,5));
  expectedPoints3.push_back(board19.xyToPoint(15,8));
  expectedPoints3.push_back(board19.xyToPoint(16,10));
  expectedPoints3.push_back(board19.xyToPoint(16,7));

  // There are 9 legal moves in the second test file
  cerr << "---- begin test record 3 ----" << endl;
  check_single_record(dataset, expectedPoints3, 19, WHITE, true);

  dataset.seekToBegin();
  EXPECT_FALSE(dataset.isEndOfData());

  check_single_record(dataset, expectedPoints1, 9, BLACK, false);
  check_single_record(dataset, expectedPoints2, 19, BLACK, false);
  check_single_record(dataset, expectedPoints3, 19, WHITE, true);

  EXPECT_TRUE(dataset.isEndOfData());
}
