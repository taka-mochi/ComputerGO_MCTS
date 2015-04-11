
#pragma once

#include "common.h"
#include "ML/FeatureArray.h"
#include "utility/SparseVector.h"

namespace Go {
  class Board;
}

namespace ML {

  class FeatureExtractor {
    
  public:
    //virtual std::shared_ptr<FeatureValues> extractFromStateAndAction(Go::Board *state, Common::Point action, Common::Color turn) const {
    //  std::shared_ptr<FeatureValues> values(new FeatureValues(getFeatureDimension()));
    //  extractFromStateAndAction(*values, state, action, turn);
    //  return values;
    //}
    virtual void extractFromStateForAllMoves(const Go::Board *state, Common::Color turn, Common::SparseVector *featureTableToExtract, Common::SparseVector &forPassFeature, Common::PointSet &extractedLegalMoves, bool clearOldFeatures = true) const = 0;
    //virtual void extractFromStateAndAction(FeatureValues &result, const Go::Board *state, Common::Point action, Common::Color turn) const = 0;
    //virtual void extractFromStateAndAction(Common::SparseVector &sparse_result, const Go::Board *state, Common::Point action, Common::Color turn) const = 0;
    virtual size_t getFeatureDimension() const = 0;
  };
}
