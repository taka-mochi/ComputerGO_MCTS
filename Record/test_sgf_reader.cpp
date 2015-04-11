#include "precomp.h"

#include <gtest/gtest.h>

#include "Record/Record.h"
#include "Record/SgfReader.h"

using namespace std;
using namespace Common;
using namespace Go;

void test_board_moves(bool isHandicaped, Record::BoardIterator &board_it, const vector<Record::Move> &move_list) {
  for (int x=0; x<9; x++) for (int y=0; y<9; y++) {
    EXPECT_EQ(FREE, board_it.getCurrentBoard().getStone(x,y));
  }

  size_t i = 0;
  size_t res_v = isHandicaped ? 1 : 0;
  do {
    board_it.moveNext();
    const Board &board = board_it.getCurrentBoard();
    Color turn = i%2 == res_v ? BLACK : WHITE;

    if (move_list[i].x != PASS && move_list[i].y != PASS) {
      EXPECT_EQ(turn, board.getStone(move_list[i].x, move_list[i].y));
    }
    if (i+1<move_list.size()) {
      if (move_list[i+1].x != PASS && move_list[i+1].y != PASS) {
        EXPECT_EQ(FREE, board.getStone(move_list[i+1].x, move_list[i+1].y));
      }
    }
    
    i++;
  } while (!board_it.isLast());
}

TEST(SgfReaderTest, readFromSimpleTestFile) {
  // read sgf file made manually
  string test_simple_file("sgf_for_test/manually_test_simple.sgf");

  Record record;
  EXPECT_EQ(SgfReader::SGF_READ_OK, SgfReader::readFromFile(test_simple_file, record));

  Record::BoardIterator board_it = record.createBoardIterator();
  const Board &init_board = board_it.getCurrentBoard();

  EXPECT_EQ(9, init_board.getSize());
  EXPECT_EQ(string("itamochi_test_black"), record.getBlackPlayerName());
  EXPECT_EQ(string("itamochi_test_white"), record.getWhitePlayerName());
  EXPECT_EQ(string("2k"), record.getBlackRank());
  EXPECT_EQ(string("3k"), record.getWhiteRank());
  EXPECT_EQ(0, record.getHandicapCount());
  EXPECT_EQ(string("2012-05-16"), record.getDate());
  EXPECT_EQ(BLACK, record.getWinner());
  EXPECT_EQ(string("R"), record.getResult());
  EXPECT_EQ(6.5, record.getKomi());

  vector<Record::Move> move_list;
  move_list.push_back(Record::Move(6,7));
  move_list.push_back(Record::Move(3,3));
  move_list.push_back(Record::Move(5,7));
  move_list.push_back(Record::Move(2,3));
  move_list.push_back(Record::Move(7,5));
  move_list.push_back(Record::Move(2,2));
  move_list.push_back(Record::Move(1,7));
  move_list.push_back(Record::Move(PASS,PASS));
  move_list.push_back(Record::Move(6,5));
  move_list.push_back(Record::Move(6,8));
  move_list.push_back(Record::Move(PASS,PASS));

  test_board_moves(false, board_it, move_list);
}

TEST(SgfReaderTest, readFromRealSgfFile) {
  vector<std::string> test_files;
  test_files.push_back("sgf_for_test/2013-04-01-4.sgf");
  test_files.push_back("sgf_for_test/2013-04-02-3.sgf");
  test_files.push_back("sgf_for_test/2013-04-02-7.sgf");
  test_files.push_back("sgf_for_test/2013-04-01-10.sgf");

  std::string black_names[] = {
    "RoyalCrown", "ben0", "feamed", "penpen88"
  };
  std::string white_names[] = {
    "pestilence", "sfreedom", "Drakemite", "hallstein2"
  };
  std::string white_ranks[] = {
    "7d","7d","7d","7d"
  };
  std::string black_ranks[] = {
    "5d","6d","4d","7d"
  };
  int handicaps[] = {
    2,0,3,0
  };
  vector<Record::Move> handiPoints[] = {
    vector<Record::Move>(), vector<Record::Move>(), vector<Record::Move>(), vector<Record::Move>()
  };
  handiPoints[0].push_back(Record::Move(15, 3));
  handiPoints[0].push_back(Record::Move(3, 15));
  handiPoints[2].push_back(Record::Move(15, 3));
  handiPoints[2].push_back(Record::Move(3, 15));
  handiPoints[2].push_back(Record::Move(15,15));
  
  Color winners[] = {
    BLACK, WHITE, BLACK, WHITE
  };
  string results[] = {
    "Resign", "Resign", "Resign", "7.50"
  };
  double komi[] = {
    0.5, 0.5, 0.5, 6.5
  };
  string dates[] = {
    "2013-04-01", "2013-04-02", "2013-04-02", "2013-04-01"
  };

  for (size_t i=0; i<4; i++) {
    Record record;
    EXPECT_EQ(SgfReader::SGF_READ_OK, SgfReader::readFromFile(test_files[i], record));

    EXPECT_EQ(19, record.getBoardSize());
    EXPECT_EQ(dates[i], record.getDate());
    EXPECT_EQ(komi[i], record.getKomi());
    EXPECT_EQ(results[i], record.getResult());
    EXPECT_EQ(winners[i], record.getWinner());
    EXPECT_EQ(handicaps[i], record.getHandicapCount());
    EXPECT_EQ(black_names[i], record.getBlackPlayerName());
    EXPECT_EQ(white_names[i], record.getWhitePlayerName());
    EXPECT_EQ(black_ranks[i], record.getBlackRank());
    EXPECT_EQ(white_ranks[i], record.getWhiteRank());

    vector<Record::Move> &handis = handiPoints[i];
    const vector<Record::Move> &getHandis = record.getHandicapBlacks();
    EXPECT_EQ(handis.size(), getHandis.size());
    for (size_t j=0; j<getHandis.size(); j++) {
      const Record::Move &move = getHandis[j];
      size_t k=0;
      for (; k<handis.size(); k++) {
        if (move.x == handis[k].x && move.y == handis[k].y) {
          break;
        }
      }
      EXPECT_NE(handis.size(), k);
    }
  }
}
