#include "precomp.h"
#include "AI/HistoryHeuristics.h"

using namespace std;
using namespace Common;
using namespace Go;

namespace AI {
  HistoryHeuristics::HistoryHeuristics(size_t size, WEIGHT_TYPE type, int maxDepth, int initialValue)
    : m_historyMoves(NULL)
    , m_historyMovesSize(size)
    , m_type(type)
    , m_depthThresholdFromRoot(maxDepth)
    , m_histogramAddValue(1.0)
  {
    m_historyMoves = new double *[COLOR_NUM];
    for (size_t i=0; i<COLOR_NUM; i++) {
      m_historyMoves[i] = new double[size];
      clearHistory(i, initialValue);
    }
  }

  HistoryHeuristics::~HistoryHeuristics()
  {
    for (size_t i=0; i<COLOR_NUM; i++) {
      delete [] m_historyMoves[i];
    }
    delete [] m_historyMoves;
  }

  void HistoryHeuristics::recordHistory(Common::Point move, Color player, int depth) {
    assert (move >= 0 && move <(signed)m_historyMovesSize);
    assert (player >= 0 && player < (signed)COLOR_NUM);

    if (depth <= m_depthThresholdFromRoot) {
      cerr << "move " << move << " depth " << depth << endl;

      int inversedDepth = m_depthThresholdFromRoot + 1 - depth;

      switch (m_type) {
      case HISTOGRAM:
        m_historyMoves[player][move] += m_histogramAddValue;
        break;
      case SQUARE_DEPTH:
        m_historyMoves[player][move] += inversedDepth*inversedDepth;
        break;
      case TWO_POWER_DEPTH:
        m_historyMoves[player][move] += 1 << inversedDepth;
        break;
      }
    }
  }

  void HistoryHeuristics::clearHistory(Color player, double clearValue) {
    assert(player >=0 && player < (signed)COLOR_NUM);
    if (clearValue == 0) {
      memset(m_historyMoves[player], 0, sizeof(double)*m_historyMovesSize);
    } else {
      for (size_t i=0; i<m_historyMovesSize; i++) {
        m_historyMoves[player][i] = clearValue;
      }
    }
  }

  void HistoryHeuristics::showTable(Common::Color color, Go::Board *board) const {
    assert(color >=0 && color < (signed)COLOR_NUM);
    
    cerr << "TODO: implement HistoryHeuristics::showTable" << endl;
    
  }
}
