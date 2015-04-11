
#pragma once

#include "precomp.h"
#include "AI/PlayerBase.h"
#include "common.h"

#include <iostream>
#include <cstdlib>
#include <stdio.h>

using namespace std;
using namespace Common;
using namespace Go;

namespace AI {

  template <class PLAYOUT_POLICY_CLASS> class PureMC : public PlayerBase {
    mutable int m_playoutCount;
    int m_playoutCountForEachHand;
    Common::Color m_currentColor;
    Go::Board *m_board;
    PLAYOUT_POLICY_CLASS playoutPolicy;

    int m_debugLevel;
  public:
  PureMC(Go::Board *init_board, PLAYOUT_POLICY_CLASS policy, Common::Color init_turn = Common::BLACK)
    : m_playoutCount(0)
      , m_playoutCountForEachHand(30)
      , m_currentColor(init_turn)
      , m_board(init_board)
      , playoutPolicy(policy)
      , m_debugLevel(0)
    {
    }

    std::string getAIName() const {
      return "PureMC";
    }

    void setPlayoutCountForEachHand(int count) {
      m_playoutCountForEachHand = count;
    }

    void setDebugLevel(int level) {
      if (level <= 0) m_debugLevel = 0;
      else m_debugLevel = level;
    }

    // select a best move according to pure monte calro search
    Common::Point selectBestMove(Common::Color turn) {
      int board_size = m_board->getSize();
      double best_value = -100;
      Point best_move(PASS);  // default move

      clock_t start = 0, end = 0;
      int startPlayoutCount = 0;
      if (m_debugLevel >= 1) {
        start = clock();
        startPlayoutCount = m_playoutCount;
      }
    
      std::string str_tmp;

      for (int y=0; y<board_size; y++) {
        for (int x=0; x<board_size; x++) {
          if (m_board->getStone(x, y) != FREE) continue;

          Point p = m_board->xyToPoint(x,y);
          // try put
          Board::PutType err = m_board->put(p,turn);
          if (err != Board::PUT_LEGAL) continue; // illegal hand

          // check repetition
          bool isRepeated = m_board->isRepeatedPosition();
          m_board->undo();
          if (isRepeated) {
            continue;
          }

          double score_sum = 0;
          m_board->takeSnapshot();
          for (int i=0; i<m_playoutCountForEachHand; i++) {
            //int score = -playout(Board::flipColor(turn));
            m_playoutCount++;
            m_board->put(x, y, turn);
            double score = -playoutPolicy(Board::flipColor(turn), m_board, false);
            //int score = -playoutPolicy(Board::flipColor(turn), m_board);
            score_sum += score;
            // m_playoutBoard.print();
            m_board->restoreStateFromSnapshot();
          }
        
          double win_rate = ((double) score_sum) / m_playoutCountForEachHand;
          if (win_rate > best_value) {
            best_value = win_rate;
            best_move = m_board->xyToPoint(x, y);
            cerr << "best move updated(x,y)=(" << x << "," << y << "), value=" << best_value << ", try count=" << m_playoutCountForEachHand << endl;
            cerr << str_tmp;
          }

          //m_board->undo();
        }
      }
    
      if (m_debugLevel >= 1) {
        end = clock();
        int diffPlayoutCount = m_playoutCount - startPlayoutCount;
        double time_diff_msec = ((double)(end-start)/CLOCKS_PER_SEC*1000);
        cerr << "Executed playout count = " << diffPlayoutCount << endl;
        cerr << "Time per playout (msec) = " << time_diff_msec/diffPlayoutCount << endl;
        cerr << "Playout speed (playout/sec) = " << 1000*diffPlayoutCount/time_diff_msec << endl;
      }
    
      return best_move;
    }
    
  };
}
