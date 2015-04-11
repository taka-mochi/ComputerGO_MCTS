
#include "precomp.h"
#include "Record/Record.h"

#include <string>

using namespace std;
using namespace Go;

namespace Common {
  int Record::calcSkillDifference(const string &skill1, const string &skill2) {
    if (skill1.size() < 2 || skill2.size() < 2) return -1;
    
    int len1 = skill1.size();
    int len2 = skill2.size();
    // 10d is 0
    int val1 = 0, val2 = 0;

    if (skill1[len1-1] == 'd' || skill1[len1-1] == 'D') {
      val1 = 10-atoi(skill1.substr(0, len1-1).c_str());
    } else if (skill1[len1-1] == 'k' || skill1[len1-1] == 'K') {
      // 1k => 10
      val1 = 9 + atoi(skill1.substr(0, len1-1).c_str());
    } else return -1;

    if (skill2[len2-1] == 'd' || skill2[len2-1] == 'D') {
      val2 = 10-atoi(skill2.substr(0, len2-1).c_str());
    } else if (skill2[len2-1] == 'k' || skill2[len2-1] == 'K') {
      // 1k => 10
      val2 = 9 + atoi(skill2.substr(0, len2-1).c_str());
    } else return -1;

    return abs(val1-val2);
  }

  Record::BoardIterator Record::createBoardIterator() const {
    return Record::BoardIterator(this);
  }

  Record::BoardIterator::BoardIterator(const Record *record)
    : m_record(record)
    , m_board(new Board(record->getBoardSize()))
  {
    m_moveIndex = -1;
    // apply handicap
    const vector<Record::Move> &moves = m_record->getHandicapBlacks();
    for (size_t i=0; i<moves.size(); i++) {
      m_board->put(moves[i].x, moves[i].y, BLACK);
    }
  }

  Record::BoardIterator::BoardIterator()
    : m_record(NULL)
    , m_board(shared_ptr<Go::Board>())
    , m_moveIndex(0)
  {
  }

  Record::BoardIterator::~BoardIterator()
  {
    m_board = shared_ptr<Board>();
  }

  void Record::BoardIterator::clear()
  {
    //cerr << "releasing!!" << endl;
    m_board = shared_ptr<Board>();
  }

  void Record::BoardIterator::moveNext()
  {
    //return;

    if (!isLast()) {
      bool lastEyeRule = Rules::isPutEyeAllowed();
      Rules::setPutEyeAllowed(true);
      Color turn = getNextTurn();
      m_moveIndex++;
      Move next = m_record->getMoveSequence()[m_moveIndex];
      if ((next.x >= 0 && next.y >= 0) || next.x == PASS) {
        Board::PutType res = m_board->put(next.x, next.y, turn);
        if (res != Board::PUT_LEGAL) {
          cerr << "error type: " << res << endl;
          cerr << "index: " << m_moveIndex << endl;
          cerr << "pos: " << next.x << "," << next.y << endl;
          m_board->printToErr();
        }
        assert(res == Board::PUT_LEGAL);
        //cerr << "history size = " << m_board->m_changeHistory.size() << endl;
      } else {
        // resign or something
      }
      Rules::setPutEyeAllowed(lastEyeRule);
    }
  }
  
  const Board &Record::BoardIterator::getCurrentBoard() const {
    return *m_board;
  }

  const shared_ptr<Board> Record::BoardIterator::getCurrentBoardPtr() const {
    return m_board;
  }

  Color Record::BoardIterator::getLastTurn() const {
    Color turn = m_moveIndex%2 == 0 ? BLACK : WHITE;
    if (m_record->getHandicapCount() >= 2) {
      turn = Board::flipColor(turn);
    }
    return turn;
  }

  Color Record::BoardIterator::getNextTurn() const {
    return Board::flipColor(getLastTurn());
  }

  Record::Move Record::BoardIterator::getNextMove() const {
    if (isLast()) {
      return Record::Move(POINT_NULL, POINT_NULL);
    } else {
      return m_record->getMoveSequence()[m_moveIndex+1];
    }
  }

  bool Record::BoardIterator::isLast() const {
    //return true;
    return m_moveIndex+1 >= (signed)m_record->getMoveSequence().size();
  }
}
