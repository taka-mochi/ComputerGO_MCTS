#include "precomp.h"

#include <random>

#include "BoardHash.h"

using namespace std;
using namespace Go;
using namespace Common;

namespace Go {
  bool BoardHash::isHashTableInitialized = false;
  BOARD_HASH_VALUE BoardHash::eachColorAndMoveHash[4][MAX_BOARD_SIZE];
  BOARD_HASH_VALUE BoardHash::eachColorAndMoveHashForKou[4][MAX_BOARD_SIZE];
  BOARD_HASH_VALUE BoardHash::toPlayHash[4];

  class my_dummy_mt64 {
    std::mt19937 m_rnd;
  public:
    my_dummy_mt64() : m_rnd() {}
    BOARD_HASH_VALUE operator()() {
      BOARD_HASH_VALUE upper = (BOARD_HASH_VALUE)m_rnd();
      BOARD_HASH_VALUE lower = (BOARD_HASH_VALUE)m_rnd();
      return ((upper<<32) | lower);
    }
  };

  void BoardHash::InitHashTable() {
/*
    typedef std::mersenne_twister_engine<uint_fast64_t,
                                              64,312,156,31,0xb5026f5aa96619e9,
                                              29,0x5555555555555555,
                                              17,0x71d67fffeda60000,
                                              37,0xfff7eee000000000,
                                              43,6364136223846793005> mymt19937_64;
*/    
    my_dummy_mt64 rnd;

    toPlayHash[0] = rnd();
    toPlayHash[1] = rnd();
    toPlayHash[2] = rnd();
    toPlayHash[3] = rnd();

    for (int i=0; i<4; i++) {
      for (int j=0; j<MAX_BOARD_SIZE; j++) {
        eachColorAndMoveHash[i][j] = rnd();
        eachColorAndMoveHashForKou[i][j] = rnd();
      }
    }
    isHashTableInitialized = true;
  }

  BoardHash::BoardHash()
    : m_value(0)
  {
    if (!isHashTableInitialized) {
      InitHashTable();
    }
  }

  BoardHash::BoardHash(const BoardHash &rhs)
    : m_value(rhs.m_value)
  {
  }

  BoardHash &BoardHash::operator=(const BoardHash &rhs)
  {
    m_value = rhs.m_value;
    return *this;
  }

  bool BoardHash::operator==(const BoardHash &rhs) {
    return m_value == rhs.m_value;
  }

  BoardHash::~BoardHash() {}

  void BoardHash::clear() {
    m_value = 0;
  }

  void BoardHash::fromBoard(const Board &board, bool includeKou, bool includeToPlay) {
    m_value = 0;
    for (int x=0; x<board.getSize(); x++) {
      for (int y=0; y<board.getSize(); y++) {
        Point p = board.xyToPoint(x,y);
        xorMove(p, board.getStone(p));
      }
    }
    if (includeKou) {
      Point kou = board.getKou();
      if (kou>0) {
        xorKou(kou, board.getToPlay());
      }
    }
    if (includeToPlay) {
      if (board.getHistoryCount() > 0) {
        xorToPlay(board.getToPlay());
      } else {
        xorToPlay(BLACK);
      }
    }
  }

  BoardHash &BoardHash::operator ^=(const BoardHash &rhs) {
    m_value ^= rhs.m_value;
    return *this;
  }

  void BoardHash::xorMove(Common::Point move, Common::Color color) {
    assert (move>=0 && move<MAX_BOARD_SIZE);
    assert (color >= 0 && color < 4);
    assert (isHashTableInitialized);

    m_value ^= eachColorAndMoveHash[color][move];
  }

  void BoardHash::xorKou(Common::Point move, Common::Color color) {
    assert (move>=0 && move<MAX_BOARD_SIZE);
    assert (color >= 0 && color < 4);
    assert (isHashTableInitialized);

    m_value ^= eachColorAndMoveHashForKou[color][move];
  }

  void BoardHash::xorToPlay(Common::Color color) {
    assert (color >= 0 && color < 4);
    assert (isHashTableInitialized);

    m_value ^= toPlayHash[color];
  }
}
