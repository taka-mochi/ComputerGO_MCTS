#pragma once

namespace ML {
  class LearningRateCalculator {
  public:
    virtual double operator()(int iteration, double lastRate) = 0;
  };
}
