#pragma once

#include "AI/UCTNode.h"
#include "AI/Parameters.h"
#include "utility/utility.h"

namespace AI {
  class UCB_Calculator {
    const Parameters &m_params;
    Common::ApproximatedMath m_math;
  public:
    UCB_Calculator(const Parameters &params)
      : m_params(params)
      , m_math()
    {
    }
    inline std::string getName() const {return "UCB";}
    //inline double operator()(UCTNode *node, UCTNode *parentNode) {
    inline double operator()(UCTNode *node, double parentGameCountLog, double parentRaveCountLog, UCTNode *parentNode) {
      double gameCountInv = 1.0/node->getGameCount();
      const double C = m_params.getUCT_C();
      //return node->getWinCount()*gameCountInv + C * m_math.sqrt(m_math.logint(parentNode->getGameCount())*gameCountInv);
      return node->getWinCount()*gameCountInv + C * Common::ApproximatedMath::sqrt(parentGameCountLog*gameCountInv);
     }
  };

  class RAVE_Calculator {
    const Parameters &m_params;
    Common::ApproximatedMath m_math;
  public:
    RAVE_Calculator(const Parameters &params)
      : m_params(params)
      , m_math()
    {
    }
    inline std::string getName() const {return "Rave";}
    //inline double operator()(UCTNode *node, UCTNode *parentNode) {
    inline double operator()(UCTNode *node, double parentGameCountLog, double parentRaveCountLog, UCTNode *parentNode) {
      const double C = m_params.getUCT_C();
      double raveCountInv = 1.0/node->getRaveCount();
      //return node->getRaveWinCount()*raveCountInv + C * m_math.sqrt(m_math.logint(parentNode->getChildrenTotalRaveCount())*raveCountInv);
      return node->getRaveWinCount()*raveCountInv + C * Common::ApproximatedMath::sqrt(parentRaveCountLog*raveCountInv);
    }
  };

  class UCB_RAVE_Calculator {
    UCB_Calculator ucb_calc;
    RAVE_Calculator rave_calc;

    const Parameters &m_params;
  public:
    UCB_RAVE_Calculator(const Parameters &params)
      : ucb_calc(params)
      , rave_calc(params)
      , m_params(params)
    {
    }

    inline std::string getName() const {return "UCB_RAVE";}
    //inline double operator()(UCTNode *node, UCTNode *parentNode) {
    inline double operator()(UCTNode *node, double parentGameCountLog, double parentRaveCountLog, UCTNode *parentNode) {
      const double K = m_params.getRave_K();
      //static const double C = Parameters::getUCT_C();
      const double M = m_params.getRave_M();
      double ucb = ucb_calc(node, parentGameCountLog, parentRaveCountLog, parentNode);
      double rave = rave_calc(node, parentGameCountLog, parentRaveCountLog, parentNode);
      double beta = sqrt(K/(K+M*parentNode->getGameCount()));
      return (1-beta)*ucb + beta*rave;
    }
  };
}
