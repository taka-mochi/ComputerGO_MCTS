#include "precomp.h"
#include "ML/Board_Move_Dataset_record.h"
#include "Record/SgfReader.h"

using namespace std;
using namespace Common;
using namespace Go;

namespace ML {
  Board_Move_Dataset_record::Board_Move_Dataset_record()
    : m_records()
    , m_currentRecord(0)
    , m_currentBoard()
      //, m_currentMoveData()
  {
  }

  Board_Move_Dataset_record::~Board_Move_Dataset_record() {
    m_records.clear();
  }

  bool Board_Move_Dataset_record::readAndAddRecordFromSGF(const std::string &filename) {
    shared_ptr<Record> record(new Record);
    if (SgfReader::readFromFile(filename, *record) != SgfReader::SGF_READ_OK) {
      cerr << "Failed to read record file: " << filename << endl;
      return false;
    }

    return addRecord(record);
  }

  bool Board_Move_Dataset_record::addRecord(std::shared_ptr<Common::Record> record) {
    m_records.push_back(record);
    if (m_records.size() == 1) {
      seekToBegin();
    }
    return true;
  }

  bool Board_Move_Dataset_record::isEndOfData() const {
    if (m_currentRecord >= static_cast<int>(m_records.size())) return true;
    if (m_currentRecord + 1 == static_cast<int>(m_records.size()) &&
        m_currentBoard.isLast()) return true;
    return false;
  }

  void Board_Move_Dataset_record::seekToBegin() {
    if (m_records.size() == 0) return;

    m_currentRecord = 0;
    m_currentBoard.clear();
    m_currentBoard = m_records[m_currentRecord]->createBoardIterator();
  }

  bool Board_Move_Dataset_record::empty() const {
    if (m_records.size() == 0) return true;
    if (m_records[0]->getMoveSequence().size() == 0) return true;
    return false;
  }

  bool Board_Move_Dataset_record::changeToNextRecord() {
    if (m_currentRecord >= static_cast<int>(m_records.size())) return false;

    //m_currentMoveData.state.reset();

    do {
      m_currentRecord++;
      if (m_currentRecord >= static_cast<int>(m_records.size())) {
        m_currentBoard.clear();
        m_currentBoard = Record::BoardIterator();
        return false;
      }
      m_currentBoard.clear();
      m_currentBoard = m_records[m_currentRecord]->createBoardIterator();
    } while (m_currentBoard.isLast());

    return true;
  }

  bool Board_Move_Dataset_record::next() {
    if (isEndOfData()) return false;

    m_currentBoard.moveNext();
    if (m_currentBoard.isLast()) {
      return changeToNextRecord();
    }
    
    return true;
  }

  Board_Move_Data Board_Move_Dataset_record::get() const {
    if (isEndOfData()) {
      return Board_Move_Data();
    }

    const shared_ptr<Board> board = m_currentBoard.getCurrentBoardPtr();
    //cerr << "board history size = " << board->m_changeHistory.size() << endl;
    Record::Move move(m_currentBoard.getNextMove());
    Point m;
    if (move.x == PASS || move.y == PASS) {
      m = PASS;
    } else {
      m = board->xyToPoint(move.x, move.y);
    }
    Color turn = m_currentBoard.getNextTurn();
    return Board_Move_Data(board, m, turn, POINT_NULL);
  }
}
