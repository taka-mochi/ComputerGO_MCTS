#pragma once

#include "common.h"

namespace Go {
  class BoardHash {
    Common::BOARD_HASH_VALUE m_value;

    static bool isHashTableInitialized;
    static Common::BOARD_HASH_VALUE eachColorAndMoveHash[4][MAX_BOARD_SIZE];
    static Common::BOARD_HASH_VALUE eachColorAndMoveHashForKou[4][MAX_BOARD_SIZE];
    static Common::BOARD_HASH_VALUE toPlayHash[4];

  public:
    static void InitHashTable();
    BoardHash();
    BoardHash(const BoardHash &rhs);
    BoardHash &operator =(const BoardHash &rhs); 
    BoardHash &operator ^=(const BoardHash &rhs);
    bool operator ==(const BoardHash &rhs);
    ~BoardHash();

    void clear();
    void fromBoard(const Board &board, bool includeKou = true, bool includeToPlay = true);

    void xorMove(Common::Point move, Common::Color color);
    void xorKou(Common::Point move, Common::Color color);
    void xorToPlay(Common::Color toPlay);
    
    Common::BOARD_HASH_VALUE get() const {return m_value;}
  };
}
