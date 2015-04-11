#include "precomp.h"

#include "AI/HistoryHeuristicsPlayout.h"
#include "utility/fmath.hpp"
#include "ML/PatternExtractor_3x3.h"

#include <iostream>
#include <fstream>

using namespace Common;
using namespace ML;
using namespace Go;
using namespace std;

namespace AI {

  const double HistoryHeuristicsPlayout::CONVERGED_VALUE_NO_CHANGE_HISTORY_VALUE = -999999;

  HistoryHeuristicsPlayout::HistoryHeuristicsPlayout(const Go::Board *initial_board, Common::MTRandom &rnd, const AI::HistoryHeuristics *history, int max_movecount_to_use_history, int max_depth_from_root_to_use_history, double convergence_value, double slope_of_history_value, bool pattern_check_enable)
    : m_sequence()
    , m_history(history)
    , m_pattern3x3Extractor(initial_board->getSize())
    , m_patternCheckEnable(pattern_check_enable)
    , m_rnd(rnd)
    , m_maxMovecountToUseHistory(max_movecount_to_use_history)
    , m_maxDepthFromRootToUseHistory(max_depth_from_root_to_use_history)
    , m_convergenceEnable(false)
    , m_convergedHistoryValue(convergence_value)
    , m_slopeOfHistoryValue(slope_of_history_value)
    , m_simulationCount(0)
    , m_moveCount(0)
    , m_patternOKCount(0)
  {
    for (int i=0; i<COLOR_NUM; i++) {
      for (int j=0; j<MAX_BOARD_SIZE; j++) {
        m_comparePatternHashes[i][j] = ML::PatternExtractor_3x3::INVALID_HASH_VALUE;
      }
    }
  }

  HistoryHeuristicsPlayout::~HistoryHeuristicsPlayout()
  {
  }

  void HistoryHeuristicsPlayout::setRequiredPatternToChangeProbabilityFromBoard(Common::Point m, Common::Color c, const Go::Board *board) {
    if (!m_patternCheckEnable) return;
    assert (m >= 0 && m < MAX_BOARD_SIZE);
    assert (c >= 0 && c < COLOR_NUM);
    assert (m != PASS);
    assert (m != POINT_NULL);
    PATTERN_3x3_HASH_VALUE v = m_pattern3x3Extractor.encode(*board, m, c == WHITE);
    setRequiredPatternToChangeProbability(m, c, v);
  }

  void HistoryHeuristicsPlayout::disableReuiredPatternToChangeProbability(Common::Point m, Common::Color c) {
    if (!m_patternCheckEnable) return;
    setRequiredPatternToChangeProbability(m, c, ML::PatternExtractor_3x3::INVALID_HASH_VALUE);
  }
      
  double HistoryHeuristicsPlayout::clampHistoryValue(double originalValue, double decreaseValue) {
    if (!m_convergenceEnable) return originalValue;
    if (m_convergedHistoryValue == CONVERGED_VALUE_NO_CHANGE_HISTORY_VALUE) return originalValue - decreaseValue;
    double decreasedValue = originalValue - decreaseValue;
    if ((originalValue - m_convergedHistoryValue) * (decreasedValue - m_convergedHistoryValue) <= 0) {
      return m_convergedHistoryValue;
    } else {
      return decreasedValue;
    }
    
  }

  Common::Point HistoryHeuristicsPlayout::selectOneMoveAccordingToHistoryTable(Go::Board *board, Common::Color color, int simulationDepth) {
    assert (m_history);
    double probTable[40][40] = {{0}};
    Point legalMoves[500]; int legalMovesCount = 0;
    
    int maxSize = board->getSize();
   
    static const double illegalMoveValue = -1010101010;

    double decreaseValue = simulationDepth * m_slopeOfHistoryValue;    
 
    for (int y=0; y<maxSize; y++) {
      Point p = board->xyToPoint(0,y);
      for (int x=0; x <board->getSize() ; x++) {
        if (board->isColor(p, FREE) && board->checkLegalHand(p, color, Board::flipColor(color)) == Board::PUT_LEGAL) {
          double origValue = m_history->getHistoryValue(p, color);
          if (m_patternCheckEnable &&
              m_comparePatternHashes[color][p] != ML::PatternExtractor_3x3::INVALID_HASH_VALUE &&
              m_pattern3x3Extractor.encode(*board, p, color == WHITE) != m_comparePatternHashes[color][p]) {
            // different pattern. converge the probability to flat
            //cerr << "value is not changed!!!!! at " << p << endl;
            //board->printToErr();
            assert (m_convergedHistoryValue != CONVERGED_VALUE_NO_CHANGE_HISTORY_VALUE);
            probTable[y][x] = m_convergedHistoryValue;            
          } else {
            // pattern is not specified, or same pattern. change the probability
            //if (m_comparePatternHashes[color][p] != ML::PatternExtractor_3x3::INVALID_HASH_VALUE) {
            //  cerr << "pattern is not changed at " << p << endl;
            //  board->printToErr();
            //}
            if (m_patternCheckEnable && 
                m_comparePatternHashes[color][p] != ML::PatternExtractor_3x3::INVALID_HASH_VALUE) {
              // pattern OK
              m_patternOKCount++;
            }
            probTable[y][x] = clampHistoryValue(origValue, decreaseValue);
          }
          //if (origValue != 0 && origValue != probTable[y][x]) cerr << "orig: " << origValue << " clamped:" << probTable[y][x] << endl;
          probTable[y][maxSize] += probTable[y][x];
          legalMoves[legalMovesCount++] = p;
        } else {
          probTable[y][x] = illegalMoveValue;
        }
        p++;
      }
      probTable[maxSize][maxSize] += probTable[y][maxSize];
    }

    // if all value is zero, select a move completely randomly
    // select the y
    if (probTable[maxSize][maxSize] < 0.000001) {
      //assert (legalMovesCount > 0);
      if (legalMovesCount > 0) {
        return legalMoves[m_rnd()%legalMovesCount];
      } else {
        return PASS;
      }
    } else {
      double vy = m_rnd.from_0_to_1()*probTable[maxSize][maxSize];
      int selected_y = -1;
      double sum = 0;
      for (int y=0; y<maxSize; y++) {
        if (sum + probTable[y][maxSize]>=vy) {
          selected_y = y;
          break;
        }
        sum += probTable[y][maxSize];
      }
      if (selected_y == -1) {
        int y = maxSize-1;
        for (; y>=0 && probTable[y][maxSize] < 0.00001; y--);
        selected_y = y;
      }
      assert (selected_y != -1);

      double vx = m_rnd.from_0_to_1()*probTable[selected_y][maxSize];
      int selected_x = -1;

      sum = 0;
      for (int x=0; x<maxSize; x++) {        
        if (probTable[selected_y][x] != illegalMoveValue) {
          if (sum + probTable[selected_y][x] >= vx) {
            selected_x = x;
            break;
          }
          sum += probTable[selected_y][x];
        }
      }
      if (selected_x == -1) {
        int x = maxSize-1;
        for (; x>=0 &&
               (probTable[selected_y][x] == illegalMoveValue || probTable[selected_y][x] < 0.00001);
             x--);
        selected_x = x;
      }
      assert (selected_x != -1);

      return board->xyToPoint(selected_x, selected_y);
    }
  }

  double HistoryHeuristicsPlayout::operator()(Common::Color turn_color, Go::Board *board, int currentDepthFromRoot, bool doUndo) {

    const static int playing_count_offset = 200;

    m_sequence.resize(0);
    
    int currentTurn = turn_color;
    int board_size = board->getSize();
    int max_playing = board_size*board_size + playing_count_offset;
    Common::Point prev_move(POINT_NULL);

    int playoutHandCount = 0;
    bool doublepass = false;

    Point freeMoves[MAX_BOARD_SIZE+1];

    m_simulationCount++;

    for (playoutHandCount=0; playoutHandCount<max_playing && !doublepass; playoutHandCount++) {
      Point selected_move = POINT_NULL;
      m_moveCount++;

      if ((m_maxMovecountToUseHistory < 0 || playoutHandCount < m_maxMovecountToUseHistory) &&
          (m_maxDepthFromRootToUseHistory < 0 || playoutHandCount + currentDepthFromRoot < m_maxDepthFromRootToUseHistory)) {
        // select by history table
        selected_move = selectOneMoveAccordingToHistoryTable(board, currentTurn, playoutHandCount);
        // if (currentTurn == BLACK) {
        //   cerr << "selected = " << selected_move << " move count = " << playoutHandCount + 1 << "depth = " << playoutHandCount + currentDepthFromRoot + 1 << endl;
        // }
      } else {
        // random select
        int freeMovesCount = 0;
        board->enumerateFreeMoves(freeMoves, freeMovesCount, currentTurn);
        volatile int index;
        do {
          if (freeMovesCount == 0) {
            selected_move = PASS;
          } else {
            index = m_rnd()%freeMovesCount;
            selected_move = freeMoves[index];
            freeMoves[index] = freeMoves[freeMovesCount-1];
            freeMovesCount--;
          }
        } while (board->checkLegalHand(selected_move, currentTurn) != Board::PUT_LEGAL);
      }

      // checks for the selected_move if the mode is DEBUG
      Board::PutType err;
#ifdef DEBUG
      if ((err = board->checkLegalHand(selected_move, currentTurn, Board::flipColor(currentTurn))) != Board::PUT_LEGAL) {
        board->printToErr();
        cerr << "selected move = " << board->pointToXY(selected_move).first << "," << board->pointToXY(selected_move).second << " turn = " << (currentTurn == BLACK ? "black" : "white") << " error = " << err << endl;
        cerr << "sequence new to old" << endl;
        for (int i=board->getHistoryCount(); i>=0; i--) {
          const Board::MoveChangeEntry *entry = board->getHistory(i);
          if (entry) {
            int killedBlocks = 0;
            for (killedBlocks=0; entry->m_killedBlocks[killedBlocks] && killedBlocks < 4; killedBlocks++) {}
            if (killedBlocks > 0) {
              int x = board->pointToXY(entry->m_putPos).first;
              int y = board->pointToXY(entry->m_putPos).second;
              cerr << "last " << i << " move = " << x << "," << y << " killed blocks = " << killedBlocks << " put color = " << entry->m_putColor << endl;
            }
          }
        }
        printProbabilityTableToErr(board);
      }
#endif // DEBUG


      err = board->put(selected_move, currentTurn, true);
      assert (err == Board::PUT_LEGAL);
      m_sequence.push_back(selected_move);

      if (selected_move == PASS && prev_move == PASS) {
        doublepass = true;
      } else {
        prev_move = selected_move;
        currentTurn = Board::flipColor(currentTurn);
      }
    }

    double score = board->countScore(turn_color);
    if (doUndo) {
      // undo changes
      for (int i=0; i<playoutHandCount; i++) {
        board->undo();
      }
    }
    return score;    
  }

  void HistoryHeuristicsPlayout::printProbabilityTableToErr(const Go::Board *board) const {
    cerr << "'-0' means 'an illegal move'" << endl;
    assert (m_history);
    
    Color c = BLACK;
    string color_str[] = {"BLACK","WHITE"};
    bool isPutEyeAllowed = Rules::isPutEyeAllowed();
    Rules::setPutEyeAllowed(true);
    setprecision(6);
    for (int i=0; i<2; i++) {
      cerr << "--- " << color_str[i] << " ---" << endl;
      for (int y=0; y<board->getSize(); y++) {
        Point p = board->xyToPoint(0,y);
        for (int x=0; x<board->getSize(); x++) {
          if (!board->isColor(p, FREE) || board->checkLegalHand(p, c, Board::flipColor(c)) != Board::PUT_LEGAL) {
            cerr << "-0 ";
          } else {
            fprintf(stderr, "%lf ", m_history->getHistoryValue(p,c));
            //cerr << m_history->getHistoryValue(p, c) << " ";
          }
          p++;
        }
        cerr << endl;
      }
      c = Board::flipColor(c);
      cerr << endl;
    }
    Rules::setPutEyeAllowed(isPutEyeAllowed);
  }

  void HistoryHeuristicsPlayout::printStatisticsToErr() const {
    cerr << "SimulationCount," << m_simulationCount << ",MoveCount," << m_moveCount << ",OKPatternCount," << m_patternOKCount << endl;
  }

  void HistoryHeuristicsPlayout::clearStatistics() const {
    m_simulationCount = 0;
    m_moveCount = 0;
    m_patternOKCount = 0;
  }
}

