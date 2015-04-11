#include "../precomp.h"

#include "gtest/gtest.h"
#include <stdexcept>
#include <algorithm>
#include "Go/BoardHash.h"

using namespace std;
using namespace Common;
using namespace Go;

TEST(BoardHashTest, checkInitialized) {
  BoardHash hash;

  EXPECT_EQ(0, hash.get());
  hash.xorMove(13, 1);
  hash.xorMove(19, 2);
  EXPECT_NE(0, hash.get());
}

TEST(BoardHashTest, checkBoardInitialize) {
  BoardHash hash;

  Board board(9);

  BoardHash hashList[MAX_BOARD_SIZE];
  for (int i=0; i<MAX_BOARD_SIZE; i++) {
    hashList[i].xorMove(i, FREE);
  }

  hash.fromBoard(board);

  BOARD_HASH_VALUE value = hash.get();

  for (int x=0; x<board.getSize(); x++) {
    for (int y=0; y<board.getSize(); y++) {
      value ^= hashList[board.xyToPoint(x,y)].get();
    }
  }
  BoardHash toPlayBlack; toPlayBlack.xorToPlay(BLACK);
  value ^= toPlayBlack.get();
  EXPECT_EQ(0, value);
}

TEST(BoardHashTest, checkMovedBoard) {
  BoardHash hash;

  Board board(19);

  BoardHash toPlay[4];
  BoardHash hashList[MAX_BOARD_SIZE];
  for (int i=0; i<MAX_BOARD_SIZE; i++) {
    hashList[i].xorMove(i, FREE);
  }
  toPlay[FREE].xorToPlay(FREE);
  toPlay[BLACK].xorToPlay(BLACK);
  toPlay[WHITE].xorToPlay(WHITE);
  toPlay[WALL].xorToPlay(WALL);

  hash.fromBoard(board);

  board.put(1,3,BLACK);
  hash.xorMove(board.xyToPoint(1,3), FREE);
  hash.xorMove(board.xyToPoint(1,3), BLACK);
  hash.xorToPlay(BLACK);
  hash.xorToPlay(WHITE);
  BOARD_HASH_VALUE value = hash.get();

  value ^= toPlay[WHITE].get();

  for (int x=0; x<board.getSize(); x++) {
    for (int y=0; y<board.getSize(); y++) {
      if (x==1 && y==3) {
        BoardHash v; v.xorMove(board.xyToPoint(x,y),BLACK);
        value ^= v.get();
      } else {
        value ^= hashList[board.xyToPoint(x,y)].get();
      }
    }
  }
   EXPECT_EQ(0, value);
}

TEST(BoardHashTest, checkKou) {
  BoardHash hash;

  Board board(19);

  BoardHash toPlay[4];
  BoardHash hashList[MAX_BOARD_SIZE];
  for (int i=0; i<MAX_BOARD_SIZE; i++) {
    hashList[i].xorMove(i, FREE);
  }
  toPlay[FREE].xorToPlay(FREE);
  toPlay[BLACK].xorToPlay(BLACK);
  toPlay[WHITE].xorToPlay(WHITE);
  toPlay[WALL].xorToPlay(WALL);

  hash.fromBoard(board);

  std::vector<Point> moves;
  moves.push_back(board.xyToPoint(13,14)); // BLACK 
  moves.push_back(board.xyToPoint(12,14)); // WHITE
  moves.push_back(board.xyToPoint(12,15)); // BLACK 
  moves.push_back(board.xyToPoint(11,15)); // WHITE
  moves.push_back(board.xyToPoint(13,16)); // BLACK 
  moves.push_back(board.xyToPoint(12,16)); // WHITE
  moves.push_back(board.xyToPoint(14,15)); // BLACK 
  moves.push_back(board.xyToPoint(13,15)); // WHITE


  Color color = BLACK;
  for (size_t i=0; i<moves.size(); i++) {
    hash.xorMove(moves[i], board.getStone(moves[i]));
    hash.xorToPlay(color);
    if (board.getKou() > 0) {
      hash.xorKou(board.getKou(), color);
    }

    board.put(moves[i], color);

    hash.xorMove(moves[i], color);
    color = Board::flipColor(color);
    hash.xorToPlay(color);
    if (board.getKou() > 0) {
      hash.xorKou(board.getKou(), color);
    }
  }
  // catch
  hash.xorMove(board.xyToPoint(12,15), BLACK);
  hash.xorMove(board.xyToPoint(12,15), FREE);

  BoardHash fromBoardHash;
  fromBoardHash.fromBoard(board);

  EXPECT_EQ(hash.get(), fromBoardHash.get());
}

TEST(BoardHashTest, checkRandomMove) {
  BoardHash hash;

  srand(0);

  for (int i=0; i<10; i++) {
    Board board(19);

    BoardHash h;
    h.fromBoard(board);
    if (h.get() != board.getHash().get()) {
      board.printToErr();
    }
    EXPECT_EQ(h.get(), board.getHash().get());

    Color c = BLACK;
    for (int j=0; j<30; j++) {
      int x;
      int y;
      do {
        x = rand()%board.getSize();
        y = rand()%board.getSize();
      } while (board.checkLegalHand(board.xyToPoint(x,y),c,Board::flipColor(c)) != Board::PUT_LEGAL);

      board.put(x,y, c);

      c = Board::flipColor(c);
      BoardHash h;
      h.fromBoard(board);
      if (h.get() != board.getHash().get()) {
        board.printToErr();
      }
      EXPECT_EQ(h.get(), board.getHash().get());
    }
  }

}
