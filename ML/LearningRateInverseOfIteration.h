#pragma once

#include "LearningRateCalculator.h"

namespace ML {
  
  class LearningRateInverseOfIteration : public LearningRateCalculator {
    double m_A,m_B;
  public:
    LearningRateInverseOfIteration(double convergeSlowness, double initialMove)
      : m_A(convergeSlowness)
      , m_B(initialMove)
    {
    }
    virtual double operator()(int iteration, double lastRate) {
      return m_A / (m_B + iteration);
    }
  };
}
