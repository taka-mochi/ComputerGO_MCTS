#pragma once

#include "LearningRateCalculator.h"

namespace ML {
  class LearningRateConstant : public LearningRateCalculator {
    double m_constant;
  public:
    explicit LearningRateConstant(double constant)
      : m_constant(constant)
    {
    }
    virtual double operator()(int iteration, double lastRate) {
      return m_constant;
    }
  };
}
