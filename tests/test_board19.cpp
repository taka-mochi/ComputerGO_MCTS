#include "../precomp.h"

#include "gtest/gtest.h"
#include <stdexcept>

using namespace std;
using namespace Common;

TEST(Board, default_init_9) {
  Common::Board board(9);
  ASSERT_EQ(9, board.getSize());
  for (int x=0; x<9; x++) { for (int y=0; y<9; y++) {
    ASSERT_EQ(Common::Board::FREE, board.getStone(x, y));
  } }
  ASSERT_EQ(Common::Board::WALL, board.getStone(-1,0));
  ASSERT_EQ(Common::Board::WALL, board.getStone(0,-1));
  ASSERT_EQ(Common::Board::WALL, board.getStone(-1,-1));
  ASSERT_EQ(Common::Board::WALL, board.getStone(9,0));
  ASSERT_EQ(Common::Board::WALL, board.getStone(9,9));
  ASSERT_EQ(Common::Board::WALL, board.getStone(0,9));
}

TEST(Board, init_copy_9) {
  Common::Board board(9);
  Common::Board copyboard(9);
  copyboard.copyFrom(board);
  
  ASSERT_EQ(board.getSize(), copyboard.getSize());
  for (int x=0; x<9; x++) { for (int y=0; y<9; y++) {
      ASSERT_EQ(copyboard.getStone(x,y) , board.getStone(x, y));
  } }

  board.put(4,4,1);
  ASSERT_NE(board.getStone(4,4), copyboard.getStone(4,4));
  ASSERT_NE(shared_ptr<Common::Block>(), board.getBelongBlock(4,4));
  ASSERT_EQ(shared_ptr<Common::Block>(), copyboard.getBelongBlock(4,4));
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
  Common::Board board(9, init_array);

  ASSERT_EQ(board.getSize(), 9);
  for (int x=0; x<9; x++) { for (int y=0; y<9; y++) {
      int val = 0;
      if ((x==1 && y==1) || (x==7 && y==1)) {
        val = 2;
      }
      if ((x==2 && y==7) || (x==7 && y==7) || (x==4 && y==4)) {
        val = 1;
      }
      ASSERT_EQ(val, board.getStone(x, y));
  } }
}

/*
TEST(Board, copy_from_obj_9) {
  Common::Board board(9);
  Common::Board copyboard(19);
  
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
  Common::Board board(9, init_array);
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

  Common::Board board(9, init_array);
  shared_ptr<Common::Block> block_1_1(board.getBelongBlock(1,1));
  shared_ptr<Common::Block> block_2_1(board.getBelongBlock(2,1));
  shared_ptr<Common::Block> block_2_2(board.getBelongBlock(2,2));
  shared_ptr<Common::Block> block_3_2(board.getBelongBlock(3,2));
  shared_ptr<Common::Block> block_2_3(board.getBelongBlock(2,3));

  shared_ptr<Common::Block> block_2_0(board.getBelongBlock(1,0));
  shared_ptr<Common::Block> block_3_0(board.getBelongBlock(2,0));
  shared_ptr<Common::Block> block_1_0(board.getBelongBlock(3,0));

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
  Common::Board board(9, init_array);

  board.put(4,3,1);
  shared_ptr<Block> block_4_2(board.getBelongBlock(4,2));
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

  EXPECT_EQ(NULL, board.getBelongBlock(2,3).get());
  EXPECT_EQ(NULL, board.getBelongBlock(3,2).get());
  EXPECT_EQ(NULL, board.getBelongBlock(2,2).get());
  EXPECT_EQ(NULL, board.getBelongBlock(1,1).get());
  EXPECT_EQ(NULL, board.getBelongBlock(2,1).get());
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
  Common::Board board(9, init_array);

  EXPECT_EQ(0,board.put(4,0,2));
  EXPECT_EQ(0,board.getStone(5,0));
  board.print();
}


TEST(Board, put_suicide) {
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
  Common::Board board(9, init_array);
  board.put(3,3,1);
  EXPECT_EQ(1, board.getStone(3,3));
  EXPECT_EQ(1, board.getStone(1,0));
  EXPECT_EQ(1, board.getStone(0,1));
  EXPECT_EQ(0, board.getStone(2,3));
}

TEST(Board, put_kou) {
  int init_array[] = {
    3,3,3,3,3,3,3,3,3,3,3,
    3,0,1,1,1,0,0,0,0,0,3,
    3,1,2,2,2,2,0,0,2,0,3,
    3,0,1,2,0,2,0,0,0,0,3,
    3,1,1,1,2,2,0,0,0,0,3,
    3,0,0,1,0,1,0,0,0,0,3,
    3,0,0,0,0,0,0,2,0,0,3,
    3,0,0,0,0,0,2,0,2,0,3,
    3,0,0,1,0,0,1,2,1,1,3,
    3,0,0,0,0,0,0,1,0,2,3,
    3,3,3,3,3,3,3,3,3,3,3
  };
  Common::Board board(9, init_array);

  ASSERT_NE(0, board.put(3,2,2));
  ASSERT_NE(0, board.put(0,2,1));
  ASSERT_NE(0, board.put(7,8,2));
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
  Common::Board board(9, init_array);

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
  Common::Board board2(9, init_array2);

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
  Common::Board board(9, init_array);

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
  Common::Board board(9, init_array);

  ASSERT_EQ(0,board.put(6,7,2));
  EXPECT_EQ(0, board.getStone(6,6));
  ASSERT_NE(0, board.put(6,6,1));
  ASSERT_EQ(0, board.put(3,7,1));
  ASSERT_TRUE(board.undo());
  ASSERT_NE(0, board.put(6,6,1));
  board.put(Board::PASS, Board::PASS, 1);
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
  Common::Board board(9, init_array);

  board.put(5,2,1);
  EXPECT_TRUE(board.undo());
  EXPECT_EQ(0,board.getStone(5,2));

  std::shared_ptr<Common::Block> block_4_2 = board.getBelongBlock(4,2);
  EXPECT_EQ(block_4_2->getLiberties().end(), block_4_2->getLiberties().find(board.xyToPoint(5,1)));
  EXPECT_EQ(block_4_2->getLiberties().end(), block_4_2->getLiberties().find(board.xyToPoint(6,2)));
  EXPECT_EQ(block_4_2->getLiberties().end(), block_4_2->getLiberties().find(board.xyToPoint(5,3)));

  EXPECT_EQ(NULL, board.getBelongBlock(5,2).get());
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
  Common::Board board(9, init_array);

  board.put(4,3,1);
  EXPECT_EQ(board.getBelongBlock(4,2), board.getBelongBlock(4,3));
  EXPECT_EQ(board.getBelongBlock(4,3), board.getBelongBlock(4,4));
  EXPECT_TRUE(board.undo());
  shared_ptr<Block> block_5_2(board.getBelongBlock(5,2));
  shared_ptr<Block> block_3_4(board.getBelongBlock(3,4));
  EXPECT_EQ(0,board.getStone(4,3));
  EXPECT_NE(block_5_2, block_3_4);
  EXPECT_EQ(NULL, board.getBelongBlock(4,3).get());
  
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
  Common::Board board(9, init_array);

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
  Common::Board board(9, init_array);

  board.put(4,7,1);
  board.put(3,4,1);
  board.undo();
  EXPECT_EQ(2,board.getBelongBlock(4,6)->getLibertyCount()); // ここはOK
  board.undo();
  ASSERT_EQ(2,board.getBelongBlock(4,6)->getLibertyCount()); // ここでだめ
}
