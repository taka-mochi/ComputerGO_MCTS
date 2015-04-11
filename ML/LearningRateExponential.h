#pragma once

#include "LearningRateCalculator.h"

namespace ML {
  class LearningRateExponential : public LearningRateCalculator {
    double m_stepBase;
  public:
    explicit LearningRateExponential(double initRate)
      : m_stepBase(initRate)
    {
    }
    virtual double operator()(int iteration, double lastRate) {
      if (iteration == 0) return lastRate; // a^0 = 1
      return lastRate*m_stepBase;
    }
  };
}
