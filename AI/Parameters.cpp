#include "precomp.h"

#include "AI/Parameters.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdlib>

#include "AI/UCTPlayer.h"
#include "AI/SoftmaxPolicyPlayout.h"
#include "AI/NormalPlayout.h"
#include "AI/HistoryHeuristicsPlayout.h"
#include "AI/UCTBoundCalculator.h"
#include "utility/utility.h"

using namespace std;

namespace AI {

  static std::string trim(const std::string &s) {
    std::string str(s);
    for (int i=(signed)str.size()-1; i>=0; i--) {
      if (str.at(i) == ' ') {
        str.resize(str.size()-1);
      } else {
        break;
      }
    }

    while (str.size() > 0 && str.at(0) == ' ') {
      str = str.substr(1);
    }
    return str;
  }

  bool Parameters::readParametersFromFile(const std::string &paramFile) {
    std::ifstream ifs(paramFile.c_str());
    if (!ifs) return false;

    while (!ifs.eof()) {
      std::string str;
      std::getline(ifs, str);

      char tmp[1024];
      strcpy(tmp, str.c_str());
      char *tmp2 = strtok(tmp, "=");
      char *tmp3 = strtok(NULL, "");
      if (tmp2 == NULL) {
        continue;
      }
      string name = trim(tmp2);
      if (name.at(0) == '#') continue; // comment
      name = Common::toUpper(name);

      string val = tmp3 == NULL ? "" : trim(tmp3);
      //transform (name.begin (), name.end (), name.begin (), ToUpperFunction());


      if (name == "UCT C") {
        if (val == "0") Parameters::setUCT_C(0);
        else {
          double C = atof(val.c_str());
          if (C != 0) Parameters::setUCT_C(C);
        }
      } else if (name == "RAVE K") {
        if (val == "0") Parameters::setRave_K(0);
        else {
          double K = atof(val.c_str());
          if (K != 0) Parameters::setRave_K(K);
        }
      } else if (name == "RAVE M") {
        if (val == "0") Parameters::setRave_M(0);
        else {
          double M = atof(val.c_str());
          if (M != 0) Parameters::setRave_M(M);
        }
      } else if (name == "ALLOW RESIGN") {
        m_allowResign = true;
      } else if (name == "DISALLOW RESIGN") {
        m_allowResign = false;
      } else if (name == "UCT EXPAND THRESHOLD") {
        if (val == "0") Parameters::setThreshold_Expand(0);
        else {
          double T = atoi(val.c_str());
          if (T != 0) Parameters::setThreshold_Expand(T);
        }
      } else if (name == "PLAYOUT LIMIT") {
        if (val == "0") Parameters::setPlayoutLimit(0);
        else {
          int P = atoi(val.c_str());
          if (P != 0) Parameters::setPlayoutLimit(P);
        }
      } else if (name == "DYNAMIC UPDATE POLICY ALPHA") {
        if (val == "0") Parameters::setDynamicUpdatePolicyAlpha(0);
        else {
          double A = atof(val.c_str());
          if (A!=0) Parameters::setDynamicUpdatePolicyAlpha(A);
        }
      } else if (name == "DYNAMIC UPDATE POLICY LAMBDA") {
        if (val == "0") Parameters::setDynamicUpdatePolicyLambda(0);
        else {
          double L = atof(val.c_str());
          if (L!=0) Parameters::setDynamicUpdatePolicyLambda(L);
        }
      } else if (name == "STANDARD FEATURE WEIGHT FILE") {
        m_standardFeatureWeightFile = val;
      } else if (name == "PLAYER KIND") {
        m_playerKind = PLAYER_KIND::fromString(val);
      } else if (name == "OPENING BOOK") {
        m_openingBookFileName = val;
        //} else if (name == "CONVERGENCE VALUE") {
        // m_historyConvergenceValue = atof(val.c_str());
      } else if (name == "HISTORY HISTOGRAM PLUS VALUE") {
        m_historyHistogramPlusValue = atof(val.c_str());
      } else if (name == "HISTORY CONVERGENCE SLOPE") {
        m_historyConvergenceSlope = atof(val.c_str());
      } else if (name == "CHECK PATTERN ON HISTORY PLAYOUT") {
        m_historyCheckPattern = true;
      }
    }
    return true;
  }

  
  std::shared_ptr<AI::PlayerBase> Parameters::createPlayerAccordingToKind(Go::Board *board, int debug_level) const {
    shared_ptr<AI::PlayerBase> player;

    MTRandom &rnd = MTRandom::getInstance();

    switch (getPlayerKind()) {
      //case AI::Parameters::PLAYER_KIND::PURE_MC:
      //break;
    case AI::Parameters::PLAYER_KIND::UCT_PLAIN:
    {
      AI::UCTPlayer<AI::NormalPlayout, AI::UCB_Calculator> *uct_ucb = 
        new AI::UCTPlayer<AI::NormalPlayout, AI::UCB_Calculator>(board, AI::NormalPlayout(rnd), *this, rnd);
      //uct_ucb.setPlayoutLimit(AI::Parameters::getPlayoutLimit());
      uct_ucb->setDebugLevel(debug_level);
      player.reset(uct_ucb);
    }
    break;
    case AI::Parameters::PLAYER_KIND::UCT_SOFTMAX_STANDARD_FEATURES:
    {
      AI::SoftmaxPolicyPlayout softmaxpolicy(getStandardFeatureWeightFile(), rnd);
      AI::UCTPlayer<AI::SoftmaxPolicyPlayout, AI::UCB_Calculator> *uct_softmax =
        new AI::UCTPlayer<AI::SoftmaxPolicyPlayout, AI::UCB_Calculator>(board, softmaxpolicy, *this, rnd);
      //uct_softmax.setPlayoutLimit(AI::Parameters::getPlayoutLimit());
      uct_softmax->setDynamicPolicyUpdate(false);
      uct_softmax->setDebugLevel(debug_level);
      player.reset(uct_softmax);
      cerr << "Dynamic Update = " << uct_softmax->getDynamicPolicyUpdate() << endl;
      cerr << "Weight file = " << getStandardFeatureWeightFile() << endl;
    }
    break;
    case AI::Parameters::PLAYER_KIND::UCT_SOFTMAX_STANDARD_FEATURES_DYNAMIC_UPDATE:
    {
      AI::SoftmaxPolicyPlayout softmaxpolicy(getStandardFeatureWeightFile(), rnd);
      AI::UCTPlayer<AI::SoftmaxPolicyPlayout, AI::UCB_Calculator> *uct_softmax =
        new AI::UCTPlayer<AI::SoftmaxPolicyPlayout, AI::UCB_Calculator>(board, softmaxpolicy, *this, rnd);
      uct_softmax->setDynamicPolicyUpdate(true);
      uct_softmax->setDebugLevel(debug_level);
      player.reset(uct_softmax);
      cerr << "Dynamic Update = " << uct_softmax->getDynamicPolicyUpdate() << endl;
      cerr << "Weight file = " << getStandardFeatureWeightFile() << endl;
    }
    break;
    case AI::Parameters::PLAYER_KIND::HISTORY_HEURISTIC:
    {
      //AI::HistoryHeuristicsPlayout playout(board, rnd, NULL, 2, 4, // Settings for Preliminary Experiment
//                                           Common::HISTORY_TABLE_INIT_VALUE,
//                                           m_historyConvergenceSlope, m_historyCheckPattern); // Settings for Preliminary Experiment

      AI::HistoryHeuristicsPlayout playout(board, rnd, NULL, 100000, 100000,
                                            Common::HISTORY_TABLE_INIT_VALUE,
                                           m_historyConvergenceSlope, m_historyCheckPattern); // Settings for Preliminary Experiment

      cerr << "History Info: HistogramPlus=" << m_historyHistogramPlusValue << " Slope=" << m_historyConvergenceSlope << endl;
      cerr << "Check 3x3 Pattern = " << m_historyCheckPattern << endl;
      if (m_historyConvergenceSlope != 0) {
        cerr << "Converge to " << Common::HISTORY_TABLE_INIT_VALUE << endl;
      }

      AI::UCTPlayer<AI::HistoryHeuristicsPlayout, AI::UCB_Calculator> *uct_history =
        new AI::UCTPlayer<AI::HistoryHeuristicsPlayout, AI::UCB_Calculator>(board, playout, *this, rnd);
      uct_history->setDebugLevel(debug_level);
      player.reset(uct_history);
    }
    break;
    default:
      break;
    }

    return player;
  }
}
