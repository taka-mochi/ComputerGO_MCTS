
#pragma once

#include "precomp.h"
#include "common.h"
#include "HistoryHeuristicsPlayout.h"

namespace Go {
  class Board;
}

namespace AI {
  
  class PlayerBase;

  class Parameters {
  public:
    struct PLAYER_KIND {
      enum KIND {
        PURE_MC, UCT_PLAIN, UCT_PLAIN_RAVE, UCT_SOFTMAX_STANDARD_FEATURES, UCT_SOFTMAX_STANDARD_FEATURES_RAVE, UCT_SOFTMAX_STANDARD_FEATURES_DYNAMIC_UPDATE, HISTORY_HEURISTIC, 
        PLAYER_KIND_NUM, INVALID_PLAYER_KIND = -1
      };
      static KIND fromString(const std::string &str) {
        std::string upper(Common::toUpper(str));
        std::cerr << upper << std::endl;
        if (upper == "PURE MC") {
          return PURE_MC;
        } else if (upper == "PLAIN UCT") {
          return UCT_PLAIN;
        } else if (upper == "PLAIN UCT RAVE") {
          return UCT_PLAIN_RAVE;
        } else if (upper == "SOFTMAX UCT STANDARD FEATURES") {
          return UCT_SOFTMAX_STANDARD_FEATURES;
        } else if (upper == "SOFTMAX UCT RAVE STANDARD FEATURES") {
          return UCT_SOFTMAX_STANDARD_FEATURES_RAVE;
        } else if (upper == "SOFTMAX UCT STANDARD FEATURES DYNAMIC UPDATE") {
          return UCT_SOFTMAX_STANDARD_FEATURES_DYNAMIC_UPDATE;
        } else if (upper == "HISTORY HEURISTIC") {
          return HISTORY_HEURISTIC;
        } else {
          return INVALID_PLAYER_KIND;
        }
      }
      static std::string toString(KIND kind) {
        switch (kind) {
        case PURE_MC:
          return "Pure MC";
        case UCT_PLAIN:
          return "Plain UCT";
        case UCT_PLAIN_RAVE:
          return "Plain UCT Rave";
        case UCT_SOFTMAX_STANDARD_FEATURES:
          return "Softmax UCT Standard Features";
        case UCT_SOFTMAX_STANDARD_FEATURES_RAVE:
          return "Softmax UCT Rave Standard Features";
        case UCT_SOFTMAX_STANDARD_FEATURES_DYNAMIC_UPDATE:
          return "Softmax UCT Standard Features Dynamic Update";
        case HISTORY_HEURISTIC:
          return "History Heuristic";
        default:
          return "";
        }
      }
    };

  private:
    bool m_allowResign;
    double m_resignThresholdOfWinRate;
    double m_uctC, m_raveK, m_raveM;
    int m_thresholdExpand;
    int m_playoutLimit;
    double m_dynamicUpdatePolicyAlpha;
    double m_dynamicUpdatePolicyLambda;
    double m_historyHistogramPlusValue;
    double m_historyConvergenceSlope;
    bool m_historyCheckPattern;
    std::string m_standardFeatureWeightFile;
    PLAYER_KIND::KIND m_playerKind;
    std::string m_openingBookFileName;

  public:
  Parameters()
    : m_allowResign(true)
      , m_resignThresholdOfWinRate(0.05)
      , m_uctC(0.031)
      , m_raveK(3)
      , m_raveM(100)
      , m_thresholdExpand(10)
      , m_playoutLimit(10000)
      , m_dynamicUpdatePolicyAlpha(1.0)
      , m_dynamicUpdatePolicyLambda(1.0)
      , m_historyHistogramPlusValue(0)
      , m_historyConvergenceSlope(0)
      , m_historyCheckPattern(false)
      , m_standardFeatureWeightFile()
      , m_playerKind(PLAYER_KIND::INVALID_PLAYER_KIND)
      , m_openingBookFileName("")
    {}

    bool isResignAllowed() const {return m_allowResign;}
    void setResignDisabled() {m_allowResign = false;}
    double getResignThresholdOfWinRate() const {return m_resignThresholdOfWinRate;}

    void setPlayoutLimit(int limit) {m_playoutLimit = limit;}
    int getPlayoutLimit() const {return m_playoutLimit;}

    void setUCT_C(double C) {m_uctC = C;}
    double getUCT_C() const {return m_uctC;}

    void setRave_K(double K) {m_raveK = K;}
    double getRave_K() const {return m_raveK;}
    void setRave_M(double M) {m_raveM = M;}
    double getRave_M() const {return m_raveM;}

    int getThreshold_Expand() const {return m_thresholdExpand;}
    void setThreshold_Expand(int s) {m_thresholdExpand = s;}

    double getDynamicUpdatePolicyAlpha() const {return m_dynamicUpdatePolicyAlpha;}
    double getDynamicUpdatePolicyLambda() const {return m_dynamicUpdatePolicyLambda;}
    void setDynamicUpdatePolicyAlpha(double alpha) {m_dynamicUpdatePolicyAlpha = alpha;}
    void setDynamicUpdatePolicyLambda(double lambda) {m_dynamicUpdatePolicyLambda = lambda;}

    const std::string &getStandardFeatureWeightFile() const {return m_standardFeatureWeightFile;}

    bool readParametersFromFile(const std::string &file);

    PLAYER_KIND::KIND getPlayerKind() const {return m_playerKind;}

    static Parameters &getInstanceOfColor(Common::Color color) {
      static Parameters params[4];
      return params[color];
    }

    std::shared_ptr<AI::PlayerBase> createPlayerAccordingToKind(Go::Board *board, int debug_level) const;

    bool getUseOpeningBook() const {return m_openingBookFileName != "";}
    std::string getOpeningBookFilename() const {return m_openingBookFileName;}

    double getHistoryHistogramAddValue() const {return m_historyHistogramPlusValue;}
  };
}
