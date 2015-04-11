
#pragma once

#include "common.h"
#include <string>

namespace AI {
  class PlayerBase {
  public:
    virtual ~PlayerBase(){}
    virtual std::string getAIName() const = 0;
    virtual bool shouldResign() const {return false;}
    virtual Common::Point selectBestMove(Common::Color turn) = 0;
  };
}
