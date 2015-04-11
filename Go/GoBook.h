#pragma once

#include "BoardHash.h"

namespace Go {
  class Board;

  class GoBook {
    typedef std::unordered_map<Common::BOARD_HASH_VALUE, std::vector<Common::Point> > HashToMoves;
    std::vector<HashToMoves> m_boardSizeToHashEntry;

    void addEntryForSequence(const std::vector<std::pair<int,int> > &sequence, const std::vector<std::pair<int,int> > &moves, int boardsize);
    std::vector<std::pair<int,int> > readPoints(std::istream &in);
  public:
    GoBook();
    ~GoBook();

    void clear();
    bool readFromFile(const std::string &file);

    bool existMoveInBook(const Board &board) const;
    std::vector<Common::Point> getMovesInBook(const Board &board) const;
    bool addEntry(const Board &board, Common::Point move);
  };
}

