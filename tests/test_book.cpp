#include "../precomp.h"

#include "gtest/gtest.h"
#include <stdexcept>
#include <algorithm>
#include "Go/GoBook.h"

using namespace std;
using namespace Common;
using namespace Go;

TEST(GoBookTest, checkInitialized) {
  GoBook book;
  Board test(9);

  EXPECT_FALSE(book.existMoveInBook(test));

}

TEST(GoBookTest, readTestFile) {
  GoBook book;
  
  EXPECT_TRUE(book.readFromFile("book/testbook.book"));

  Board test(9), test13(13);
  cerr << "empty " << test.getHash().get() << endl;
  EXPECT_TRUE(book.existMoveInBook(test));
  EXPECT_FALSE(book.existMoveInBook(test13));

  EXPECT_EQ(1,  book.getMovesInBook(test).size());
  EXPECT_EQ(test.xyToPoint(4,4), book.getMovesInBook(test)[0]);

  test.put(2, 2, BLACK);
  EXPECT_EQ(2, book.getMovesInBook(test).size());
  EXPECT_EQ(test.xyToPoint(2,6), book.getMovesInBook(test)[0]);

  test.put(3,2, WHITE);
  EXPECT_FALSE(book.existMoveInBook(test));

  test.undo(); test.undo();
  test.put(3,3,BLACK);
  EXPECT_FALSE(book.existMoveInBook(test));
  test.put(3,2,WHITE);
  cerr << test.getHash().get() << endl;
  EXPECT_EQ(2, book.getMovesInBook(test).size());
  vector<Point> moves = book.getMovesInBook(test);
  EXPECT_TRUE(moves[0] == test.xyToPoint(2,2) || moves[0] == test.xyToPoint(4,2));
  EXPECT_TRUE(moves[1] == test.xyToPoint(2,2) || moves[1] == test.xyToPoint(4,2));

  test.undo(); test.undo();
  test.put(5,3,BLACK);
  test.put(5,2,WHITE);

  EXPECT_EQ(2, book.getMovesInBook(test).size());
  moves = book.getMovesInBook(test);
  if (moves[0] == test.xyToPoint(4,2)) {
    EXPECT_EQ(test.xyToPoint(6,2), moves[1]);
  } else {
    EXPECT_EQ(test.xyToPoint(6,2), moves[0]);
  }
}

