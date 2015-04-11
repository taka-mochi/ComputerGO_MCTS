#pragma once

#include "common.h"
#include "utility/SingletonMemoryPool.h"
#include "utility/SparseVector.h"
#include "ML/Board_Move_Dataset.h"
#include "ML/FeatureExtractor.h"

namespace ML {

  typedef Common::SingletonMemoryPool<Common::SparseVector, 1000> SparseVectorAllocator;

  class GoLearning {
  public:
    virtual ~GoLearning() {}
    
    // virtual void learn(dataset) = 0;
    // virtual const FeatureWeight &getFeatureWeight() const = 0;

    static void convertOneDataToFeatureVectors(const FeatureExtractor &featureExtractor, const Board_Move_Data &data, Common::Point *freePointBuffer, std::vector<Common::SparseVector *> &result_buffer, int &index_of_action_in_data, size_t *checkAppearDim = NULL, bool *isAppear = NULL);
  };
}
