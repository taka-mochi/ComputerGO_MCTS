
#pragma once

namespace Go {
  class Board;
}

namespace AI {
  class HistoryHeuristics {
  public:
    enum WEIGHT_TYPE {
      HISTOGRAM, SQUARE_DEPTH, TWO_POWER_DEPTH
    };
    
  public:
    explicit HistoryHeuristics(size_t historySize, WEIGHT_TYPE type = HISTOGRAM, int maxDepthFromRoot = 10, int initialValue = 0);
    ~HistoryHeuristics();

    void setHistogramAddValue(double value) {m_histogramAddValue = value;};
    void recordHistory(Common::Point move, Common::Color color, int depthFromRoot);
    void clearHistory(Common::Color player, double clearValue = 0);

    inline size_t getTableSize() const {return m_historyMovesSize;}

    inline double getHistoryValue(Common::Point move, Common::Color player) const {
      assert(move>=0 && move<(signed)m_historyMovesSize);
      assert(player>=0 && player < (signed)Common::COLOR_NUM);
      return m_historyMoves[player][move];
    }

    void showTable(Common::Color color, Go::Board *board) const;

  private:
    
    
  private:
    double **m_historyMoves;
    size_t m_historyMovesSize;
    WEIGHT_TYPE m_type;
    int m_depthThresholdFromRoot;
    double m_histogramAddValue;
  };
}

