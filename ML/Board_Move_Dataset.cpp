#include "precomp.h"

#include "ML/Board_Move_Dataset.h"

using namespace std;
using namespace Common;
using namespace Go;

typedef shared_ptr<Board> BoardPtr;

namespace ML {
  Board_Move_Dataset_vector::Board_Move_Dataset_vector()
    : m_index(0)
    , m_dataSet()
  {
  }

  Board_Move_Dataset_vector::~Board_Move_Dataset_vector()
  {
    m_dataSet.clear();
  }

  void Board_Move_Dataset_vector::addData(const Board_Move_Data &d) {
    m_dataSet.push_back(d);
  }

  void Board_Move_Dataset_vector::addData(BoardPtr state, Point move, Color turn, Point kou) {
    addData(Board_Move_Data(state,move, turn, kou));
  }
}
