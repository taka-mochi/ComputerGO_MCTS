
#include "precomp.h"

#include "AI/RandomPlayer.h"

using namespace Common;
using namespace std;

namespace AI {

  RandomPlayer::RandomPlayer(Go::Board *board)
    : m_board(board)
  {
  }

  string RandomPlayer::getAIName() const {
    return "Random";
  }

  Point RandomPlayer::selectBestMove(Color turn) {
    int board_size = m_board->getSize();
    vector<Common::Point> candidates;
    candidates.reserve(board_size*board_size);

    for (int x=0; x<board_size; x++) {
      for (int y=0; y<board_size; y++) {
        int stone = m_board->getStone(x, y);
        if (stone != FREE) {
          continue;
        }
        candidates.push_back(m_board->xyToPoint(x, y));
      }
    }

    // select a legal hand randomly
    Common::Point selected_hand(PASS);
    while (true) {
      int selected_index = -1;
      if (candidates.size()==0) {
        selected_hand = Common::Point(PASS);
      } else {
        selected_index = rand()%candidates.size();
        selected_hand = candidates[selected_index];
      }
      int err = m_board->put(selected_hand, turn);
      if (err == 0) {
        m_board->undo();
        break; // legal hand
      }
      candidates[selected_index] = candidates[candidates.size()-1];
      candidates.resize(candidates.size()-1);
    }

    return selected_hand;
  }
}
