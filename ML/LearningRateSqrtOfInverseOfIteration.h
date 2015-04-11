#pragma once

#include <cmath>

namespace ML {
  class LearningRateSqrtOfInverseOfIteration : public LearningRateCalculator {
    double m_A, m_B;
  public:
    LearningRateSqrtOfInverseOfIteration(double convergeSlowness, double initialMove)
      : m_A(convergeSlowness)
      , m_B(initialMove)
    {
    }

    virtual double operator()(int iteration, double lastRate) {
      return m_A / sqrt(m_B + iteration);
    }
  };
}
