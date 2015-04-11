#include "precomp.h"

#include <gtest/gtest.h>

#include "Record/Record.h"

using namespace Common;
using namespace std;
using namespace Go;

TEST(Record_Test, setter_getter) {
  Record rec;

  rec.setHandicapCount(3);
  rec.setBoardSize(9);
  rec.setWinner(BLACK);
  rec.setKomi(5.5);
  rec.setBlackPlayerName("hage");
  rec.setWhitePlayerName("hoge");
  rec.setResultToDraw();
  rec.addHandicapPoint(Record::Move(2,3));
  rec.addHandicapPoint(Record::Move(4,5));
  rec.addHandicapPoint(Record::Move(1,6));

  EXPECT_EQ(3, rec.getHandicapCount());
  EXPECT_EQ(9, rec.getBoardSize());
  EXPECT_EQ(BLACK, rec.getWinner());
  EXPECT_EQ(5.5, rec.getKomi());
  EXPECT_EQ(std::string("hage"), rec.getBlackPlayerName());
  EXPECT_EQ(std::string("hoge"), rec.getWhitePlayerName());
  EXPECT_EQ(std::string("Draw"), rec.getResult());
  EXPECT_EQ(3, (signed)rec.getHandicapBlacks().size());
  EXPECT_TRUE(rec.checkConsistencyForHandicap());

  rec.addHandicapPoint(Record::Move(2,4));
  rec.setHandicapCount(-1);
  EXPECT_EQ(3, rec.getHandicapCount());
  EXPECT_EQ(4, (signed)rec.getHandicapBlacks().size());
  EXPECT_FALSE(rec.checkConsistencyForHandicap());
  rec.setBoardSize(20);
  EXPECT_EQ(9, rec.getBoardSize());
  rec.setBoardSize(0);
  EXPECT_EQ(9, rec.getBoardSize());
  rec.setWinner(FREE);
  EXPECT_EQ(BLACK, rec.getWinner());
  rec.setKomi(-0.1);
  EXPECT_EQ(5.5, rec.getKomi());
}

TEST(Record_Test, boardIterator) {
  // make this board
  // int init_array[][9] = {
  //   {0,1,1,1,0,0,0,0,0},
  //   {1,2,2,2,0,0,0,2,0},
  //   {0,1,2,2,1,0,0,0,0},
  //   {0,1,2,0,0,0,0,0,0},
  //   {0,0,1,0,0,0,0,0,0},
  //   {0,0,0,0,0,0,0,0,0},
  //   {0,0,0,0,0,0,0,0,0},
  //   {0,0,0,0,0,0,0,0,0},
  //   {0,0,0,0,0,0,0,0,0}
  // };
  Record rec;
  rec.setBoardSize(9);

  vector<Record::Move> move_list;

  move_list.push_back(Record::Move(1,0));
  move_list.push_back(Record::Move(1,1));
  move_list.push_back(Record::Move(2,0));
  move_list.push_back(Record::Move(2,1));
  move_list.push_back(Record::Move(3,0));
  move_list.push_back(Record::Move(3,1));
  move_list.push_back(Record::Move(0,1));
  move_list.push_back(Record::Move(7,1));
  move_list.push_back(Record::Move(1,2));
  move_list.push_back(Record::Move(2,2));
  move_list.push_back(Record::Move(4,2));
  move_list.push_back(Record::Move(3,2));
  move_list.push_back(Record::Move(1,3));
  move_list.push_back(Record::Move(2,3));
  move_list.push_back(Record::Move(2,4));

  for (size_t i=0; i<move_list.size(); i++) {
    rec.addMove(move_list[i]);
  }

  Record::BoardIterator board_it = rec.createBoardIterator();
  EXPECT_EQ(9, board_it.getCurrentBoard().getSize());

  for (int x=0; x<9; x++) for (int y=0; y<9; y++) {
      EXPECT_EQ(FREE, board_it.getCurrentBoard().getStone(x,y));
  }

  int i=0;
  do {
    board_it.moveNext();
    Color c = i%2 == 0 ? BLACK : WHITE;

    const Board &board = board_it.getCurrentBoard();
    EXPECT_EQ(c, board.getStone(move_list[i].x, move_list[i].y));
    if (i+1 < (signed)move_list.size()) {
      EXPECT_EQ(FREE, board.getStone(move_list[i+1].x, move_list[i+1].y));
    }

    i++;
  } while (!board_it.isLast());

  board_it.getCurrentBoard().printToErr();
}

TEST(Record_Test, boardIterator_with_Handicap) {
  // make this board
  // int init_array[][9] = {
  //   {0,1,1,1,0,0,0,0,0},
  //   {1,2,2,2,0,0,0,2,0},
  //   {0,1,2,2,1,0,0,0,0},
  //   {0,1,2,0,0,0,0,0,0},
  //   {0,0,1,0,0,0,0,0,0},
  //   {0,0,0,0,0,0,0,0,0},
  //   {0,0,0,0,0,0,0,0,0},
  //   {0,0,0,0,0,0,0,0,0},
  //   {0,0,0,0,0,0,0,0,0}
  // };
  Record rec;
  rec.setBoardSize(9);
  rec.setHandicapCount(2);
  rec.addHandicapPoint(Record::Move(6,2));
  rec.addHandicapPoint(Record::Move(2,6));
  EXPECT_TRUE(rec.checkConsistencyForHandicap());

  vector<Record::Move> move_list;

  move_list.push_back(Record::Move(1,0));
  move_list.push_back(Record::Move(1,1));
  move_list.push_back(Record::Move(2,0));
  move_list.push_back(Record::Move(2,1));
  move_list.push_back(Record::Move(3,0));
  move_list.push_back(Record::Move(3,1));
  move_list.push_back(Record::Move(0,1));
  move_list.push_back(Record::Move(7,1));
  move_list.push_back(Record::Move(1,2));
  move_list.push_back(Record::Move(2,2));
  move_list.push_back(Record::Move(4,2));
  move_list.push_back(Record::Move(3,2));
  move_list.push_back(Record::Move(1,3));
  move_list.push_back(Record::Move(2,3));
  move_list.push_back(Record::Move(2,4));

  for (size_t i=0; i<move_list.size(); i++) {
    rec.addMove(move_list[i]);
  }

  Record::BoardIterator board_it = rec.createBoardIterator();
  EXPECT_EQ(9, board_it.getCurrentBoard().getSize());

  for (int x=0; x<9; x++) for (int y=0; y<9; y++) {
      if ((x==2 && y==6) || (x==6 && y==2)) {
        EXPECT_EQ(BLACK, board_it.getCurrentBoard().getStone(x,y));
      } else {
        EXPECT_EQ(FREE, board_it.getCurrentBoard().getStone(x,y));
      }
  }

  int i=0;
  do {
    board_it.moveNext();
    Color c = i%2 == 1 ? BLACK : WHITE;

    const Board &board = board_it.getCurrentBoard();
    EXPECT_EQ(c, board.getStone(move_list[i].x, move_list[i].y));
    if (i+1 < (signed)move_list.size()) {
      EXPECT_EQ(FREE, board.getStone(move_list[i+1].x, move_list[i+1].y));
    }

    i++;
  } while (!board_it.isLast());

  board_it.getCurrentBoard().printToErr();
}
