
#pragma once

#include "common.h"
#include "AI/HistoryHeuristics.h"
#include "ML/FeatureArray.h"
#include "ML/StandardFeatureExtractor.h"
#include "utility/PointMap.h"
#include "utility/utility.h"
#include <vector>

namespace Go {
  class Board;
}

class Test_HistoryHeuristicsPlayout;

namespace AI {
  class HistoryHeuristicsPlayout {

    friend class ::Test_HistoryHeuristicsPlayout;

  private:
    Common::Point selectOneMoveAccordingToHistoryTable(Go::Board *board, Common::Color color, int simulationDepth);
    double clampHistoryValue(double originalValue, double decreaseValue);
    void setRequiredPatternToChangeProbability(Common::Point m, Common::Color c, ML::PATTERN_3x3_HASH_VALUE pattern) {
      assert (m >= 0 && m < MAX_BOARD_SIZE);
      assert (c >= 0 && c < Common::COLOR_NUM);
      m_comparePatternHashes[c][m] = pattern;
    }

  public:
    explicit HistoryHeuristicsPlayout(const Go::Board *initial_board, Common::MTRandom &rnd, const AI::HistoryHeuristics *history = NULL, int max_movecount_to_use_history = -1, int max_depth_from_root_to_use_history = -1, double convergence_value = CONVERGED_VALUE_NO_CHANGE_HISTORY_VALUE, double slope_of_history_value = 0.0, bool pattern_check_enable = false);
    virtual ~HistoryHeuristicsPlayout();
    static std::string getName() {
      return "HistoryHeuristicPlayout";
    }
    inline const std::vector<Common::Point> &getLastSequence() const {
      return m_sequence;
    }
    ML::StandardFeatureExtractor *getFeatureExtractor() {return NULL;}
    const ML::FeatureWeights *getFeatureWeights() const {return NULL;}

    void setFeatureWeights(const ML::FeatureWeights &weights) {}
    void addFeatureWeights(const ML::FeatureWeights &weights) {}
    void setExpFeatureWeights(const ML::FeatureWeights &expWeights) {}
    double operator()(Common::Color turn_color, Go::Board *board, int currentDepthFromRoot, bool doUndo = true);

    void setHistoryHeuristics(const HistoryHeuristics *history) {
      m_history = history;
    }
    void setAvailabilityOfHistoryValueConvergence(bool enable) {
      m_convergenceEnable = enable;
    }
    void setRequiredPatternToChangeProbabilityFromBoard(Common::Point m, Common::Color c, const Go::Board *board);
    void disableReuiredPatternToChangeProbability(Common::Point m, Common::Color c);

    void printProbabilityTableToErr(const Go::Board *board) const;

    static const double CONVERGED_VALUE_NO_CHANGE_HISTORY_VALUE;

    void printStatisticsToErr() const;
    void clearStatistics() const;

  private:
    std::vector<Common::Point> m_sequence;
    const HistoryHeuristics *m_history;
    ML::PATTERN_3x3_HASH_VALUE m_comparePatternHashes[4][MAX_BOARD_SIZE];
    ML::PatternExtractor_3x3 m_pattern3x3Extractor;
    bool m_patternCheckEnable;
    Common::MTRandom &m_rnd;
    int m_maxMovecountToUseHistory;
    int m_maxDepthFromRootToUseHistory;
    bool m_convergenceEnable;
    double m_convergedHistoryValue;
    double m_slopeOfHistoryValue;

    mutable int m_simulationCount;
    mutable int m_moveCount;
    mutable int m_patternOKCount;
  };
}
