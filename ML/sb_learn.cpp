#include "precomp.h"

#include "ML/sb_learn.h"

using namespace std;

namespace ML {
  SB_Learning::SB_Learning()
    : m_maxIterationCountForDataSet(0)
    , m_iterForEstimatedValue(0)
    , m_iterForGradientValue(0)
    , m_stepRate(0.0)
    , m_featureWeights()
  {
  }

  SB_Learning::SB_Learning(int maxIter, int iterForEstimateValue, int iterForGradValue, double stepRate)
    : m_featureWeights()
  {
    setParameters(maxIter, iterForEstimateValue, iterForGradValue, stepRate);
  }

  void SB_Learning::setParameters(int maxIter, int iterForEstimateValue, int iterForGradValue, double stepRate) {
    m_maxIterationCountForDataSet = maxIter;
    m_iterForEstimatedValue = iterForEstimateValue;
    m_iterForGradientValue = iterForGradValue;
    m_stepRate = stepRate;
  }

  //void SB_Learning::::learn(somedataset, board, simulator)
  //{
  //}
}
