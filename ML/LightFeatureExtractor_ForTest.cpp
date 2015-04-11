#include "precomp.h"
#include "ML/LightFeatureExtractor_ForTest.h"

using namespace std;
using namespace Common;
using namespace Go;

namespace ML {

  // Light Feature Extractor for Test
  void LightFeatureExtractor_ForTest::extractFromStateForAllMoves(const Go::Board *state, Common::Color turn, Common::SparseVector *featureTableToExtract, Common::SparseVector &forPassFeature, Common::PointSet &extractedLegalMoves, bool clearOldFeatures) const {
    assert (state);
    assert (featureTableToExtract);

    if (clearOldFeatures) {
      for (int y=0; y<state->getSize(); y++) {
        for (int x=0; x<state->getSize(); x++) {
          featureTableToExtract[state->xyToPoint(x,y)].clear();
        }
      }
      //featureTableToExtract[0].clear();
      extractedLegalMoves.clear();
    }
    forPassFeature.clear();

    for (int y=0; y<state->getSize(); y++) {
      for (int x=0; x<state->getSize(); x++) {
        Point p = state->xyToPoint(x,y);

        if (state->isColor(p, FREE) &&
            (state->getNeighborEmptyCount(p) >= 1 || state->checkLegalHand(p, turn, Board::flipColor(turn)) == Board::PUT_LEGAL)) {
          extractFromStateAndAction(featureTableToExtract[p], state, p, turn);
          extractedLegalMoves.insert(p,p);
        }
      }
    }
  }

  void LightFeatureExtractor_ForTest::extractFromStateAndAction(FeatureValues &features, const Board *state, Common::Point action, Common::Color turn) const {
    assert(state != NULL);
    assert(features.size() == getFeatureDimension());

    int x,y;
    std::pair<int,int> xy(state->pointToXY(action));

    if (action != PASS) {
      x = xy.first; y = xy.second;

      // capture feature
      for (int i=0; i<4; i++) {
        BlockPtr b(state->getBelongBlock(action + state->getFourDirections()[i]));
        if (b && b->getColor() != turn && b->getLibertyCount() == 1) {
          // capture
          features[CAPTURE] = 1;
        }
      }

      // distance from the nearest border
      int minX = std::min(x, state->getSize()-(x+1));
      int minY = std::min(y, state->getSize()-(y+1));
      int minDist = std::min(minX, minY);
      if (minDist < 4) {
        features[DIST_FROM_BORDER_0+minDist] = 1;
      } else {
        features[DIST_FROM_BORDER_4_OR_MORE] = 1;
      }
    }
  }

  void LightFeatureExtractor_ForTest::extractFromStateAndAction(SparseVector &features, const Board *state, Common::Point action, Common::Color turn) const {
    FeatureValues values(getFeatureDimension()); values.clear();
    extractFromStateAndAction(values, state, action, turn);
    for (size_t i=0; i<getFeatureDimension(); i++) {
      if (values[i] != 0) {
        features.add(i);
      }
    }
  }

  size_t LightFeatureExtractor_ForTest::getFeatureDimension() const {
    return FEATURE_SIZE;
  }
}
