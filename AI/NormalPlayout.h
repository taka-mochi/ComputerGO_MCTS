
#pragma once

#include "common.h"
#include <vector>
#include "utility/utility.h"
#include "ML/FeatureArray.h"

namespace Go {
  class Board;
}
namespace ML {
  class StandardFeatureExtractor;
  //class FeatureWeights;
}

namespace AI {
  class NormalPlayout {
    std::vector<Common::Point> m_sequence;
    Common::Point candidates[MAX_BOARD_SIZE];
    Common::MTRandom &m_rnd;

  public:
    explicit NormalPlayout(Common::MTRandom &rnd);
    virtual ~NormalPlayout(){}
    std::string getName() const {
      return "NormalPlayout";
    }
    inline const std::vector<Common::Point> &getLastSequence() const {
      return m_sequence;
    }
    ML::StandardFeatureExtractor *getFeatureExtractor() {return NULL;}
    const ML::FeatureWeights *getFeatureWeights() const {return NULL;}
    const ML::FeatureWeights *getExpFeatureWeights() const {return NULL;}
    void setFeatureWeights(const ML::FeatureWeights &weights){}
    void addFeatureWeights(const ML::FeatureWeights &weights){}

    double operator()(Common::Color turn_color, Go::Board *board, int depth, bool doUndo = true);
  };
}
