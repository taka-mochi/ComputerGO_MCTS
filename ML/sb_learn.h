#pragma once

#include <vector>
#include "ML/go_learning.h"

namespace ML {
  class SB_Learning : public GO_Learning {
  private:
    int m_maxIterationCountForDataSet;
    int m_iterForEstimatedValue;
    int m_iterForGradientValue;
    double m_stepRate;  // this should be gradually decreased?

    // FeatureWeight: this should be implemented as one class
    //          because a mapping between a double array and feature descriptions is needed
    std::vector<double> m_featureWeights;  // temporal implementation

  public:
    explicit SB_Learning();
    explicit SB_Learning(int maxIter, int iterForEstimateValue, int iterForGradValue, double stepRate);
    virtual ~SB_Learning();

    void setParameters(int maxIter, int iterForEstimateValue, int iterForGradValue, double stepRate);

    //void learn(dataset, board, simulator);

    // temporal implementation
    typedef std::vector<double> FeatureWeight;
    const FeatureWeight &getFeatureWeight() const {return m_featureWeights;}
  };
}
