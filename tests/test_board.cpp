#include "../precomp.h"

#include "gtest/gtest.h"
#include <stdexcept>
#include <algorithm>

using namespace std;
using namespace Common;
using namespace Go;

void test_default_init(int board_size) {
  Board board(board_size);
  ASSERT_EQ(board_size, board.getSize());
  for (int x=0; x<board_size; x++) { for (int y=0; y<board_size; y++) {
    ASSERT_EQ(FREE, board.getStone(x, y));
    EXPECT_TRUE(board.isFirst(x,y));
  } }
  ASSERT_EQ(WALL, board.getStone(-1,0));
  ASSERT_EQ(WALL, board.getStone(0,-1));
  ASSERT_EQ(WALL, board.getStone(-1,-1));
  ASSERT_EQ(WALL, board.getStone(board_size,0));
  ASSERT_EQ(WALL, board.getStone(board_size,board_size));
  ASSERT_EQ(WALL, board.getStone(0,board_size));
}

TEST(Board, default_init_9) {
  test_default_init(9);
}
TEST(Board, default_init_13) {
  test_default_init(13);
}
TEST(Board, default_init_19) {
  test_default_init(19);
}

void test_init_copy(int board_size) {
  Board board(board_size);
/*
  Board copyboard(board_size);
  copyboard.copyFrom(board);
  
  ASSERT_EQ(board.getSize(), copyboard.getSize());
  for (int x=0; x<board_size; x++) { for (int y=0; y<board_size; y++) {
      ASSERT_EQ(copyboard.getStone(x,y) , board.getStone(x, y));
  } }
*/
  board.put(4,4,1);
//  ASSERT_NE(board.getStone(4,4), copyboard.getStone(4,4));
  ASSERT_NE(BlockPtr(), board.getBelongBlock(4,4));
//  ASSERT_EQ(BlockPtr(), copyboard.getBelongBlock(4,4));
}

TEST(Board, init_copy_9) {
  test_init_copy(9);
}
TEST(Board, init_copy_13) {
  test_init_copy(13);
}
TEST(Board, init_copy_19) {
  test_init_copy(19);
}

void test_init_array(int board_size, int *array, vector<pair<int,int> > &blackPoints, vector<pair<int,int> > &whitePoints) {
  Board board(board_size, array);
  ASSERT_EQ(board.getSize(), board_size);

  for (int x=0; x<9; x++) { for (int y=0; y<9; y++) {
      pair<int,int> p(x,y);
      if (find(blackPoints.begin(), blackPoints.end(), p) != blackPoints.end()) {
        EXPECT_EQ(BLACK, board.getStone(x,y));
      }
      else if (find(whitePoints.begin(), whitePoints.end(), p) != whitePoints.end()) {
        EXPECT_EQ(WHITE, board.getStone(x,y));
      }
      else {
        EXPECT_EQ(FREE, board.getStone(x, y));
      }
  } }

}

TEST(Board, init_array_9) {
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,2,0,0,0,0,0,2,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,1,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,1,0,0,0,0,1,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  vector<pair<int,int> > blacks, whites;
  blacks.push_back(pair<int,int>(2,7));
  blacks.push_back(pair<int,int>(7,7));
  blacks.push_back(pair<int,int>(4,4));
  whites.push_back(pair<int,int>(1,1));
  whites.push_back(pair<int,int>(7,1));

  test_init_array(9, init_array, blacks, whites);
}

/*
TEST(Board, copy_from_obj_9) {
  Board board(9);
  Board copyboard(19);
  
  copyboard.copyFrom(board);
  ASSERT_EQ(board.getSize(), copyboard.getSize());
  for (int x=0; x<9; x++) { for (int y=0; y<9; y++) {
      ASSERT_EQ(copyboard.getStone(x,y) , board.getStone(x, y));
  } }
}
*/


TEST(Board, put1) {
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,2,0,0,0,0,0,2,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,1,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,1,0,0,0,0,1,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);
  board.put(4,4,2);
  EXPECT_EQ(1, board.getStone(4,4));
  board.put(0, 1, 1);
  ASSERT_EQ(1, board.getStone(0,1));
  board.put(0,2,2);
  ASSERT_EQ(2, board.getStone(0,2));
  board.put(0,0,2);
  ASSERT_EQ(0, board.getStone(0,1));
}

TEST(Board, block_test1) {
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,1,1,1,0,0,0,0,0,3,
    3,1,2,2,1,0,0,0,2,0,3,
    3,0,1,2,2,1,0,0,0,0,3,
    3,0,1,2,0,0,0,0,0,0,3,
    3,0,0,1,0,1,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,1,0,0,0,0,1,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };

  Board board(9, init_array);
  BlockPtr block_1_1(board.getBelongBlock(1,1));
  BlockPtr block_2_1(board.getBelongBlock(2,1));
  BlockPtr block_2_2(board.getBelongBlock(2,2));
  BlockPtr block_3_2(board.getBelongBlock(3,2));
  BlockPtr block_2_3(board.getBelongBlock(2,3));

  BlockPtr block_2_0(board.getBelongBlock(1,0));
  BlockPtr block_3_0(board.getBelongBlock(2,0));
  BlockPtr block_1_0(board.getBelongBlock(3,0));

  EXPECT_EQ(2, block_1_1->getColor());
  EXPECT_EQ(block_1_1, block_2_1);
  EXPECT_EQ(block_1_1, block_2_1);
  EXPECT_EQ(block_1_1, block_2_2);
  EXPECT_EQ(block_1_1, block_3_2);
  EXPECT_EQ(block_1_1, block_2_3);

  EXPECT_EQ(1, block_1_0->getColor());
  EXPECT_EQ(block_1_0, block_2_0);
  EXPECT_EQ(block_1_0, block_3_0);

  //board.printAllBlocks();
}

TEST(Board, put2) {
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,1,1,1,0,0,0,0,0,3,
    3,1,2,2,1,0,0,0,2,0,3,
    3,0,1,2,2,1,0,0,0,0,3,
    3,0,1,2,0,0,0,0,0,0,3,
    3,0,0,1,0,1,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,1,0,0,0,0,1,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  board.put(4,3,1);

  BlockPtr block_4_2(board.getBelongBlock(4,2));
  EXPECT_NE(block_4_2->getLiberties().end(), block_4_2->getLiberties().find(board.xyToPoint(3,3)));
  EXPECT_NE(block_4_2->getLiberties().end(), block_4_2->getLiberties().find(board.xyToPoint(5,3)));
  EXPECT_NE(block_4_2->getLiberties().end(), block_4_2->getLiberties().find(board.xyToPoint(3,4)));
  EXPECT_NE(block_4_2->getLiberties().end(), block_4_2->getLiberties().find(board.xyToPoint(5,4)));
  EXPECT_NE(block_4_2->getLiberties().end(), block_4_2->getLiberties().find(board.xyToPoint(4,5)));

  board.put(3,3,1);
  EXPECT_EQ(1, board.getStone(3,3));
  EXPECT_EQ(1, board.getStone(1,0));
  EXPECT_EQ(1, board.getStone(0,1));
  EXPECT_EQ(0, board.getStone(2,3));
  EXPECT_EQ(0, board.getStone(3,2));
  EXPECT_EQ(0, board.getStone(2,2));
  EXPECT_EQ(0, board.getStone(1,1));
  EXPECT_EQ(0, board.getStone(2,1));  

  EXPECT_EQ(BlockPtr(), board.getBelongBlock(2,3));
  EXPECT_EQ(BlockPtr(), board.getBelongBlock(3,2));
  EXPECT_EQ(BlockPtr(), board.getBelongBlock(2,2));
  EXPECT_EQ(BlockPtr(), board.getBelongBlock(1,1));
  EXPECT_EQ(BlockPtr(), board.getBelongBlock(2,1));
}

TEST(Board, put3) {
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,2,0,1,0,1,2,0,0,3,
    3,2,2,0,1,1,2,2,0,1,3,
    3,0,2,0,1,2,0,2,2,0,3,
    3,2,1,1,1,2,2,0,2,0,3,
    3,0,2,2,2,2,2,2,0,0,3,
    3,1,2,0,1,1,2,1,1,0,3,
    3,2,1,1,0,1,0,2,0,0,3,
    3,0,1,1,1,0,1,1,1,2,3,
    3,0,0,0,1,1,0,1,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3,
  };
  Board board(9, init_array);

  EXPECT_EQ(0,board.put(4,0,2));
  EXPECT_EQ(0,board.getStone(5,0));
}

TEST(Board, put_eye) {
  {
    int init_array[] = {
      3,3,3,3,3,3,3,3,3,3,3,
      3,0,1,0,1,0,1,0,1,0,3,
      3,1,1,0,1,1,1,0,1,1,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,1,1,0,1,1,1,1,1,1,3,
      3,0,1,0,1,0,1,0,0,1,3,
      3,1,1,0,0,1,1,1,1,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,1,1,1,1,1,1,0,1,1,3,
      3,0,1,0,1,0,1,0,1,0,3,
      3,3,3,3,3,3,3,3,3,3,3
    };
    Board board(9, init_array);
  
    EXPECT_EQ(Board::PUT_EYE, board.put(0,0,1));
    EXPECT_EQ(Board::PUT_EYE, board.put(0,8,1));
    EXPECT_EQ(Board::PUT_EYE, board.put(8,0,1));
    EXPECT_EQ(Board::PUT_EYE, board.put(8,8,1));
    EXPECT_EQ(Board::PUT_EYE, board.put(4,0,1));
    EXPECT_EQ(Board::PUT_EYE, board.put(0,4,1));
    EXPECT_EQ(Board::PUT_EYE, board.put(4,4,1));
    EXPECT_EQ(Board::PUT_LEGAL, board.put(6,4,1));
    EXPECT_EQ(Board::PUT_EYE, board.put(7,4,1));
  }
  {
    int init_array[] = {
      3,3,3,3,3,3,3,3,3,3,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,0,0,0,2,2,2,0,0,0,3,
      3,0,0,2,1,1,1,2,0,0,3,
      3,0,0,2,1,0,1,2,0,0,3,
      3,0,0,0,2,1,0,1,2,0,3,
      3,0,0,0,2,1,1,1,2,0,3,
      3,0,0,0,0,2,2,2,2,0,3,
      3,0,0,0,0,0,0,0,0,0,3,
      3,3,3,3,3,3,3,3,3,3,3
    };
    Board board(9, init_array);
    EXPECT_EQ(Board::PUT_EYE, board.put(4,4,1));
    EXPECT_EQ(Board::PUT_EYE, board.put(5,5,1));

  }
}

TEST(Board, put_false_eye) {
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,1,0,1,0,1,0,0,0,3,
    3,1,2,0,2,1,1,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,1,1,0,2,1,1,2,1,1,3,
    3,0,1,0,1,0,1,1,0,1,3,
    3,1,1,0,2,1,1,1,1,2,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,1,0,0,0,1,1,0,0,0,3,
    3,0,1,0,1,0,1,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);
  
  EXPECT_EQ(Board::PUT_LEGAL, board.put(0,0,1));
  EXPECT_EQ(Board::PUT_EYE, board.put(0,8,1));
  EXPECT_EQ(Board::PUT_LEGAL, board.put(4,0,1));
  EXPECT_EQ(Board::PUT_LEGAL, board.put(4,4,1));
  EXPECT_EQ(Board::PUT_LEGAL, board.put(7,4,1));
  EXPECT_EQ(Board::PUT_EYE, board.put(4,8,1));
}

TEST(Board, put_suicide) {
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,1,2,0,0,0,0,0,0,3,
    3,1,1,2,0,0,0,0,2,0,3,
    3,2,2,2,2,2,2,0,0,0,3,
    3,0,2,2,1,1,2,0,0,0,3,
    3,0,2,1,0,1,2,0,0,0,3,
    3,0,2,1,1,1,2,0,0,0,3,
    3,0,0,2,2,2,2,2,2,0,3,
    3,0,0,2,1,1,1,1,2,0,3,
    3,0,0,2,1,0,0,1,2,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);
  board.put(3,3,1);
  EXPECT_EQ(Board::PUT_SUICIDE, board.put(0,0,1));
  EXPECT_EQ(Board::PUT_LEGAL, board.put(0,0,2));
  EXPECT_EQ(Board::PUT_LEGAL, board.put(5,8,1));
  EXPECT_EQ(Board::PUT_SUICIDE, board.put(4,8, 1));
  EXPECT_EQ(Board::PUT_SUICIDE, board.put(3,4, 1));
  
}

TEST(Board, put_kou) {
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,1,1,1,0,0,0,0,0,3,
    3,1,2,2,2,2,0,0,2,0,3,
    3,0,1,2,0,2,0,0,0,0,3,
    3,1,2,1,2,2,0,0,0,0,3,
    3,0,0,1,0,1,0,0,0,0,3,
    3,0,0,0,0,0,0,2,0,0,3,
    3,0,0,0,0,0,2,0,2,0,3,
    3,0,0,1,0,0,1,2,1,1,3,
    3,0,0,0,0,0,0,1,0,2,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  EXPECT_EQ(Board::PUT_LEGAL, board.put(0,2,2));
  EXPECT_EQ(Board::PUT_KOU, board.put(1,2,1));
  EXPECT_EQ(Board::PUT_LEGAL, board.put(0,8,1));
  EXPECT_EQ(Board::PUT_LEGAL, board.put(1,2,1));
}


TEST(Board, scoreCount) {
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,1,1,1,1,0,1,0,1,1,3,
    3,1,2,2,1,1,0,1,0,1,3,
    3,1,1,2,2,1,1,1,1,1,3,
    3,0,1,2,2,2,2,2,2,2,3,
    3,1,1,1,1,1,2,2,0,2,3,
    3,1,0,1,1,1,1,2,2,2,3,
    3,1,1,1,0,1,2,2,2,0,3,
    3,0,1,1,1,1,1,2,2,2,3,
    3,1,1,0,1,0,1,1,1,1,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  EXPECT_EQ(1, board.countScore(1));
  EXPECT_EQ(-1, board.countScore(2));

  int init_array2[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,2,2,2,2,0,2,0,2,2,3,
    3,2,1,1,2,2,0,2,0,2,3,
    3,2,2,1,1,2,2,2,2,2,3,
    3,0,2,1,1,1,1,2,2,2,3,
    3,2,2,2,2,2,1,1,0,2,3,
    3,2,0,2,2,2,2,1,1,1,3,
    3,2,2,2,0,2,2,1,1,0,3,
    3,0,2,2,2,2,2,1,1,1,3,
    3,2,2,0,2,0,2,2,2,2,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board2(9, init_array2);

  EXPECT_EQ(0, board2.countScore(1));
  EXPECT_EQ(0, board2.countScore(2));
}

TEST(Board, undo_normal) {
int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,1,1,1,0,0,0,0,0,3,
    3,1,2,2,1,0,0,0,2,0,3,
    3,0,1,2,2,1,0,0,0,0,3,
    3,0,1,2,0,0,0,0,0,0,3,
    3,0,0,1,0,1,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,1,0,0,0,0,1,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  board.put(5,1,1);
  EXPECT_TRUE(board.undo());
  EXPECT_EQ(0,board.getStone(5,1));
  EXPECT_FALSE(board.undo());
}

TEST(Board, undo_kou) {
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,1,1,1,0,0,0,0,0,3,
    3,1,2,2,1,0,0,0,2,0,3,
    3,0,1,2,2,1,1,0,0,0,3,
    3,0,1,2,0,0,0,0,0,0,3,
    3,0,0,1,1,1,0,0,0,0,3,
    3,0,0,0,0,0,0,2,0,0,3,
    3,0,0,0,0,0,2,1,2,0,3,
    3,0,0,1,0,0,1,0,1,0,3,
    3,0,0,0,0,0,0,1,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  ASSERT_EQ(0,board.put(6,7,2));
  EXPECT_EQ(0, board.getStone(6,6));
  ASSERT_NE(0, board.put(6,6,1));
  ASSERT_EQ(0, board.put(3,7,1));
  ASSERT_TRUE(board.undo());
  ASSERT_NE(0, board.put(6,6,1));
  board.put(PASS, PASS, 1);
  ASSERT_EQ(0, board.put(6,6,2));
}

TEST(Board, undo_add_group) {
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,1,1,1,0,0,0,0,0,3,
    3,1,2,2,1,0,0,0,2,0,3,
    3,0,1,2,2,1,0,0,0,0,3,
    3,0,1,2,0,0,0,0,0,0,3,
    3,0,0,1,0,1,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,1,0,0,0,0,1,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  board.put(5,2,1);
  EXPECT_TRUE(board.undo());
  EXPECT_EQ(0,board.getStone(5,2));

  BlockPtr block_4_2 = board.getBelongBlock(4,2);
  EXPECT_EQ(block_4_2->getLiberties().end(), block_4_2->getLiberties().find(board.xyToPoint(5,1)));
  EXPECT_EQ(block_4_2->getLiberties().end(), block_4_2->getLiberties().find(board.xyToPoint(6,2)));
  EXPECT_EQ(block_4_2->getLiberties().end(), block_4_2->getLiberties().find(board.xyToPoint(5,3)));

  EXPECT_EQ(BlockPtr(), board.getBelongBlock(5,2));
  EXPECT_NE(board.getBelongBlock(4,2), board.getBelongBlock(5,2));
}

TEST(Board, undo_joint_group) {
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,1,1,1,0,0,0,0,0,3,
    3,1,2,2,1,0,0,0,2,0,3,
    3,0,1,2,2,1,1,0,0,0,3,
    3,0,1,2,0,0,1,0,0,0,3,
    3,0,0,1,1,1,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,1,0,0,0,0,1,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  board.put(4,3,1);
  EXPECT_EQ(board.getBelongBlock(4,2), board.getBelongBlock(4,3));
  EXPECT_EQ(board.getBelongBlock(4,3), board.getBelongBlock(4,4));
  EXPECT_TRUE(board.undo());
  BlockPtr block_5_2(board.getBelongBlock(5,2));
  BlockPtr block_3_4(board.getBelongBlock(3,4));
  EXPECT_EQ(0,board.getStone(4,3));
  EXPECT_NE(block_5_2, block_3_4);
  EXPECT_EQ(BlockPtr(), board.getBelongBlock(4,3));
  
  // check liberty undo
  EXPECT_NE(block_5_2->getLiberties().end(), block_5_2->getLiberties().find(board.xyToPoint(5,4)));
  EXPECT_NE(block_3_4->getLiberties().end(), block_3_4->getLiberties().find(board.xyToPoint(3,3)));
  EXPECT_NE(block_3_4->getLiberties().end(), block_3_4->getLiberties().find(board.xyToPoint(5,4)));

}

TEST(Board, undo_kill_block) {
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,1,1,1,0,0,0,0,0,3,
    3,1,2,2,1,0,0,0,2,0,3,
    3,0,1,2,2,1,1,0,0,0,3,
    3,0,1,2,0,0,0,0,0,0,3,
    3,0,0,1,1,1,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,1,0,0,0,0,1,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  board.put(3,3,2);
  board.put(4,3,1);
  EXPECT_TRUE(board.undo());
  EXPECT_EQ(0,board.getStone(4,3));
  EXPECT_EQ(2,board.getStone(3,3));
  EXPECT_EQ(2,board.getStone(2,3));
  EXPECT_EQ(2,board.getStone(3,2));
  EXPECT_EQ(2,board.getStone(2,2));
  EXPECT_EQ(2,board.getStone(2,1));
  EXPECT_EQ(2,board.getStone(1,1));
  EXPECT_EQ(board.getBelongBlock(3,3), board.getBelongBlock(2,3));
  EXPECT_EQ(board.getBelongBlock(3,3), board.getBelongBlock(3,2));
  EXPECT_EQ(board.getBelongBlock(3,3), board.getBelongBlock(2,2));
  EXPECT_EQ(board.getBelongBlock(3,3), board.getBelongBlock(2,1));
  EXPECT_EQ(board.getBelongBlock(3,3), board.getBelongBlock(1,1));
}

TEST(Board, undo_kill_blocks2) {
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

  board.put(4,7,1);
  board.put(3,4,1);
  board.undo();
  EXPECT_EQ((unsigned)2,board.getBelongBlock(4,6)->getLibertyCount()); // ここはOK
  board.undo();
  ASSERT_EQ((unsigned)2,board.getBelongBlock(4,6)->getLibertyCount()); // ここでだめ
}


TEST(Board, undo_by_snapshot) {
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,1,1,1,0,0,0,0,0,3,
    3,1,2,2,1,0,0,0,2,0,3,
    3,0,1,2,2,1,1,0,0,0,3,
    3,0,1,2,0,0,0,0,0,0,3,
    3,0,0,1,1,1,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,1,0,0,0,0,1,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  board.put(5,3,2);
  board.takeSnapshot();
  board.put(3,3,2);
  board.put(4,3,1);
  board.restoreStateFromSnapshot();

  EXPECT_TRUE(board.isFirst(3,3));
  EXPECT_TRUE(board.isFirst(4,3));
  EXPECT_FALSE(board.isFirst(5,3));

  EXPECT_EQ(2,board.getStone(5,3));
  EXPECT_EQ(0,board.getStone(4,3));
  EXPECT_EQ(0,board.getStone(3,3));
  EXPECT_EQ(2,board.getStone(2,3));
  EXPECT_EQ(2,board.getStone(3,2));
  EXPECT_EQ(2,board.getStone(2,2));
  EXPECT_EQ(2,board.getStone(2,1));
  EXPECT_EQ(2,board.getStone(1,1));
  EXPECT_EQ(BlockPtr(NULL), board.getBelongBlock(3,3));
  EXPECT_EQ(board.getBelongBlock(2,3), board.getBelongBlock(3,2));
  EXPECT_EQ(board.getBelongBlock(2,3), board.getBelongBlock(2,2));
  EXPECT_EQ(board.getBelongBlock(2,3), board.getBelongBlock(2,1));
  EXPECT_EQ(board.getBelongBlock(2,3), board.getBelongBlock(1,1));
}

TEST(Board, undo_after_snapshot) {
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,1,1,1,0,0,0,0,0,3,
    3,1,2,2,1,0,0,0,2,0,3,
    3,0,1,2,2,1,1,0,0,0,3,
    3,0,1,2,0,0,0,0,0,0,3,
    3,0,0,1,1,1,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,1,0,0,0,0,1,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  board.put(5,3,2);
  board.put(5,4,1);
  board.put(3,3,2);
  board.put(4,3,1);
  board.takeSnapshot();
  board.put(3,3,2);
  board.put(3,2,1);
  board.restoreStateFromSnapshot();

  EXPECT_FALSE(board.isFirst(3,3));
  EXPECT_FALSE(board.isFirst(3,2));
  EXPECT_FALSE(board.isFirst(4,3));

  EXPECT_TRUE(board.undo());
  EXPECT_TRUE(board.isFirst(4,3));
  EXPECT_EQ(0,board.getStone(4,3));
  EXPECT_EQ(1,board.getStone(5,4));
  EXPECT_EQ(2,board.getStone(5,3));
  EXPECT_EQ(2,board.getStone(3,3));
  EXPECT_EQ(2,board.getStone(2,3));
  EXPECT_EQ(2,board.getStone(3,2));
  EXPECT_EQ(2,board.getStone(2,2));
  EXPECT_EQ(2,board.getStone(2,1));
  EXPECT_EQ(2,board.getStone(1,1));
  EXPECT_EQ(BlockPtr(NULL), board.getBelongBlock(4,3));
  EXPECT_EQ(board.getBelongBlock(2,3), board.getBelongBlock(3,3));
  EXPECT_EQ(board.getBelongBlock(2,3), board.getBelongBlock(3,2));
  EXPECT_EQ(board.getBelongBlock(2,3), board.getBelongBlock(2,2));
  EXPECT_EQ(board.getBelongBlock(2,3), board.getBelongBlock(2,1));
  EXPECT_EQ(board.getBelongBlock(2,3), board.getBelongBlock(1,1));

  EXPECT_NE(board.getBelongBlock(4,2)->getLiberties().end(), board.getBelongBlock(4,2)->getLiberties().find(board.xyToPoint(4,3)));
  EXPECT_NE(board.getBelongBlock(4,4)->getLiberties().end(), board.getBelongBlock(4,4)->getLiberties().find(board.xyToPoint(4,3)));
  EXPECT_EQ(board.getBelongBlock(4,2)->getStones().end(), board.getBelongBlock(4,2)->getStones().find(board.xyToPoint(4,3)));
  EXPECT_EQ(board.getBelongBlock(4,4)->getStones().end(), board.getBelongBlock(4,4)->getStones().find(board.xyToPoint(4,3)));
}

TEST(Board, check_3kou_repetition) {
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,1,2,0,0,0,0,0,0,3,
    3,1,2,0,2,0,0,0,2,0,3,
    3,0,1,2,0,0,0,2,1,2,3,
    3,0,0,0,2,0,0,1,0,1,3,
    3,0,0,0,0,1,2,0,1,0,3,
    3,0,0,0,1,0,1,2,0,0,3,
    3,0,0,0,0,0,2,0,2,0,3,
    3,0,0,1,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,1,0,2,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  EXPECT_EQ(0, board.put(4,6,1));
  EXPECT_FALSE(board.isRepeatedPosition());
  EXPECT_EQ(0, board.put(4,5,2));
  EXPECT_FALSE(board.isRepeatedPosition());
  EXPECT_EQ(0, board.put(2,1,1));
  EXPECT_FALSE(board.isRepeatedPosition());
  EXPECT_EQ(0, board.put(7,3,2));
  EXPECT_FALSE(board.isRepeatedPosition());
  EXPECT_EQ(0, board.put(5,5,1));
  EXPECT_FALSE(board.isRepeatedPosition());
  EXPECT_EQ(0, board.put(1,1,2));
  EXPECT_FALSE(board.isRepeatedPosition());
  EXPECT_EQ(0, board.put(7,2,1));
  EXPECT_FALSE(board.isRepeatedPosition());  
  EXPECT_EQ(0, board.put(4,5,2));
  EXPECT_TRUE(board.isRepeatedPosition());  
}

TEST(Board, check_sending_two_returning_one) {
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,2,0,1,2,0,0,0,0,3,
    3,1,2,1,1,2,0,0,2,0,3,
    3,0,1,1,2,2,0,0,0,0,3,
    3,1,1,2,2,0,0,0,0,0,3,
    3,2,2,2,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  EXPECT_EQ(0, board.put(7,7,2));
  cout << "Black's turn: kou " << board.getKou() << endl;
  board.print();
  EXPECT_EQ(0, board.put(0,0,1));
  EXPECT_EQ(0, board.put(0,2,2));
  EXPECT_EQ(0, board.put(0,1,1));
  EXPECT_FALSE(board.isRepeatedPosition());
  EXPECT_EQ(0, board.put(PASS,2));
  cout << "Black's turn: kou" << board.getKou() << endl;
  EXPECT_TRUE(board.isRepeatedPosition());
  board.print();
}

TEST(Board, enumerateFreeMoves) {
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,2,0,1,2,0,0,0,0,3,
    3,1,2,1,1,2,0,0,2,0,3,
    3,0,1,1,2,2,0,0,0,0,3,
    3,1,1,2,2,0,0,0,0,0,3,
    3,2,2,2,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  Point moves[MAX_BOARD_SIZE+1];
  int free_counts = 0;

  board.enumerateFreeMoves(moves, free_counts, 1);
  EXPECT_EQ(61, free_counts);
  for (int i=0; i<free_counts; i++) {
    EXPECT_TRUE(board.isColor(moves[i], FREE));
  }
}

TEST(Board, capturedCount) {
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,1,1,1,0,0,0,0,0,3,
    3,1,2,2,1,0,0,0,2,0,3,
    3,0,1,2,2,1,0,0,0,0,3,
    3,0,1,2,0,0,0,0,0,0,3,
    3,0,0,1,0,1,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,2,2,2,0,0,0,0,0,0,3,
    3,1,1,1,2,0,0,0,1,0,3,
    3,1,2,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  board.put(3,3,1);
  EXPECT_EQ(5, board.getCapturedCount(2));
  EXPECT_EQ(0, board.getCapturedCount(1));

  board.put(2,8,2);
  EXPECT_EQ(5, board.getCapturedCount(2));
  EXPECT_EQ(4, board.getCapturedCount(1));
}

TEST(Board, capturedCountAfterUndo) {
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,1,1,1,0,0,0,0,0,3,
    3,1,2,2,1,0,0,0,2,0,3,
    3,0,1,2,2,1,0,0,0,0,3,
    3,0,1,2,0,0,0,0,0,0,3,
    3,0,0,1,0,1,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,2,2,2,0,0,0,0,0,0,3,
    3,1,1,1,2,0,0,0,1,0,3,
    3,1,2,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  board.put(3,3,1);
  board.put(2,8,2);
  board.undo();
  EXPECT_EQ(5, board.getCapturedCount(2));
  EXPECT_EQ(0, board.getCapturedCount(1));
  board.undo();
  EXPECT_EQ(0, board.getCapturedCount(2));
  EXPECT_EQ(0, board.getCapturedCount(1));
}

TEST(Board, block_refCount) {
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,1,1,1,0,0,0,0,0,3,
    3,1,2,2,1,0,0,0,2,0,3,
    3,0,1,2,2,1,0,0,0,0,3,
    3,0,1,2,0,0,0,0,0,0,3,
    3,0,0,1,0,1,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,2,2,2,0,0,0,0,0,0,3,
    3,1,1,1,2,0,0,0,1,0,3,
    3,1,2,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  EXPECT_EQ(1, board.getBelongBlock(0,1)->refCount());
  EXPECT_EQ(4, board.getBelongBlock(1,0)->refCount());
  EXPECT_EQ(1, board.getBelongBlock(2,4)->refCount());
  EXPECT_EQ(1, board.getBelongBlock(4,4)->refCount());

  board.put(3,4,1);
  EXPECT_EQ(4, board.getBelongBlock(3,4)->refCount());
  board.undo();
  EXPECT_EQ(1, board.getBelongBlock(2,4)->refCount());
  EXPECT_EQ(1, board.getBelongBlock(4,4)->refCount());

  BlockPtr oldBlock = board.getBelongBlock(1,1);
  EXPECT_EQ(5, oldBlock->refCount());
  board.put(3,3,1);
  EXPECT_EQ(1, oldBlock->refCount());
  board.undo();
  EXPECT_EQ(5, oldBlock->refCount());
}

TEST(Board, refCount_test_for_snapshot) {
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,1,1,1,0,0,0,0,0,3,
    3,1,2,2,1,0,0,0,2,0,3,
    3,0,1,2,2,1,1,0,0,0,3,
    3,0,1,2,0,0,0,0,0,0,3,
    3,0,0,1,1,1,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,0,0,1,0,0,0,0,1,0,3,
    3,0,0,0,0,0,0,0,0,0,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Board board(9, init_array);

  board.put(5,3,2);
  EXPECT_EQ(2, board.getBelongBlock(5,3)->refCount());
  board.put(5,4,1);
  EXPECT_EQ(5, board.getBelongBlock(5,4)->refCount());
  board.put(3,3,2);
  EXPECT_EQ(7, board.getBelongBlock(3,3)->refCount());
  BlockPtr old = board.getBelongBlock(3,3);
  board.put(4,3,1);
  EXPECT_EQ(9, board.getBelongBlock(4,3)->refCount());
  EXPECT_EQ(2, old->refCount());
  board.takeSnapshot();
  board.put(3,3,2);
  board.put(3,2,1);
  board.restoreStateFromSnapshot();
  EXPECT_EQ(9, board.getBelongBlock(4,3)->refCount());
  board.undo();
  EXPECT_EQ(2, board.getBelongBlock(4,2)->refCount());
  EXPECT_EQ(5, board.getBelongBlock(4,4)->refCount());
  EXPECT_EQ(7, board.getBelongBlock(3,3)->refCount());
}
