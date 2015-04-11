
#include "precomp.h"

#include "AI/NormalPlayout.h"
#include <iostream>

using namespace std;
using namespace Common;
using namespace AI;
using namespace Go;

namespace AI {
  
  NormalPlayout::NormalPlayout(Common::MTRandom &rnd)
    : m_sequence()
    , m_rnd(rnd)
  {
    m_sequence.reserve(MAX_BOARD_SIZE+1000);
  }
  //void performUndo(Board *board, int playoutHandCount) __attribute__((noinline));
  //void enumerateLegalHands(Board *board, Point *candidates, int &candidate_count) __attribute__((noinline));
  void performUndo(Board *board, int playoutHandCount)
  {
    // undo changes
    for (int i=0; i<playoutHandCount; i++) {
      board->undo();
    }
  }

/*
  void enumerateLegalHandCandidates(Board *board, Point *candidates, int &candidate_count, int board_size, int currentTurn) {
    candidate_count = 0;
    for (int x=0; x<board_size; x++) for (int y=0; y<board_size; y++) {
        Point p = board->xyToPoint(x,y);
        if (!board->isColor(p, Board::FREE)) continue;
        //if (board->checkLegalHand(p, currentTurn, Board::flipColor(currentTurn)) != Board::PUT_LEGAL) continue;
        candidates[candidate_count++] = p;
    }
  }
*/

  double NormalPlayout::operator()(Color turn_color, Board *board, int depth, bool doUndo) {
    const static int playing_count_offset = 200;

    m_sequence.resize(0);
    
    int currentTurn = turn_color;
    int board_size = board->getSize();
    int max_playing = board_size*board_size + playing_count_offset;
    Common::Point prev_hand(POINT_NULL);

    // enumerate hand candidates
    int candidate_count = 0;
    //candidates.resize(board_size*board_size);

    int playoutHandCount;
    for (playoutHandCount=0; playoutHandCount<max_playing; playoutHandCount++) {
      
      //enumerateLegalHandCandidates(board, candidates, candidate_count, board_size, currentTurn);
      board->enumerateFreeMoves(candidates, candidate_count, currentTurn);
//      cerr << "Legal moves = " << candidate_count << endl;

      // select a legal hand randomly
      Common::Point selected_hand(PASS);
      while (true) {
        int sel_index = -1;
        if (candidate_count!=0) {
          sel_index = m_rnd(candidate_count);
          selected_hand = candidates[sel_index];
        } else {
          selected_hand = PASS;
          break;
        }
        if (board->getNeighborEmptyCount(selected_hand)>=1) break;
        Board::PutType err = board->checkLegalHand(selected_hand, currentTurn, Board::flipColor(currentTurn));
        if (err == Board::PUT_LEGAL) {
          break;
        }
        assert(sel_index!=-1);
        candidates[sel_index] = candidates[candidate_count-1];
        candidate_count--;
      }
      board->put(selected_hand, currentTurn, true);
      m_sequence.push_back(selected_hand);

      // double pass
      if (selected_hand == PASS && prev_hand == PASS) {
        // end game
        playoutHandCount++;
        break;
      }
      
      prev_hand = selected_hand;
      currentTurn = Board::flipColor(currentTurn);
    }

    double score = board->countScore(turn_color);
    if (doUndo) performUndo(board, playoutHandCount);
    //std::cerr << playoutHandCount << endl;
    return score;    
  }
}
