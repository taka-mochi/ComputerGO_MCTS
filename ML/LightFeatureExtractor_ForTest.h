#pragma once

#include "ML/FeatureExtractor.h"

namespace ML {
  class LightFeatureExtractor_ForTest : public FeatureExtractor {
  public:
    enum FEATURE_INDEX {
      CAPTURE = 0,
      DIST_FROM_BORDER_0,
      DIST_FROM_BORDER_1,
      DIST_FROM_BORDER_2,
      DIST_FROM_BORDER_3,
      DIST_FROM_BORDER_4_OR_MORE,

      FEATURE_SIZE
    };

    void extractFromStateAndAction(FeatureValues &features, const Go::Board *state, Common::Point action, Common::Color turn) const;
    void extractFromStateAndAction(Common::SparseVector &features, const Go::Board *state, Common::Point action, Common::Color turn) const;

  public:
    //std::shared_ptr<FeatureValues> extractFromStateAndAction(Go::Board *state, Common::Point action, Common::Color turn) {
    //  return FeatureExtractor::extractFromStateAndAction(state, action, turn);
    //}
    virtual void extractFromStateForAllMoves(const Go::Board *state, Common::Color turn, Common::SparseVector *featureTableToExtract, Common::SparseVector &forPassFeature, Common::PointSet &extractedLegalMoves, bool clearOldFeatures = true) const;
    size_t getFeatureDimension() const;
  };
}
