
#pragma once

#include "AI/PlayerBase.h"

namespace Go {
  class Board;
}

namespace AI {

  class RandomPlayer : public PlayerBase {
    Go::Board *m_board;
  public:
    RandomPlayer(Go::Board *board);
    std::string getAIName() const;
    Common::Point selectBestMove(Common::Color turn);
  };
}
