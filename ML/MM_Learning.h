#pragma once

#include <vector>
#include "ML/GoLearning.h"
#include "ML/FeatureArray.h"
#include "ML/Board_Move_Dataset.h"

class MM_Learning_Test;

namespace ML {

  class FeatureExtractor;

  class MM_Learning : public GoLearning {
  public:
    // friend class definition: this is for Google Test
    friend class ::MM_Learning_Test;

  private:
    FeatureExtractor &m_featureExtractor;
    //int m_maxIterationCountForDataSet;
    double m_stepRate;  // this should be gradually decreased?

    // FeatureWeight: this should be implemented as one class
    //          because a mapping between a double array and feature descriptions is needed
    FeatureWeights m_featureWeights;
    FeatureWeights m_featureWeights_inverse;
    //std::vector<double> m_featureWeights;  // temporal implementation

    FeatureValues m_featureAppearCountsInLastTraining;
    FeatureWeights m_CijofEjsInLastTraining;

  private:
    // at once updater
    FeatureWeights calculateNewWeightsAtOnce(std::vector<Board_Move_Dataset *> &dataset);
    void updateForOneData(const Board_Move_Data &data, FeatureWeights &CijofEjs, FeatureValues &featureCounts, Common::Point *freePointBuffer);
    void updateForFeatureVectorsOfOneData(std::vector<Common::SparseVector *> &featureVectors, int index_of_action_in_data, FeatureWeights &CijofEjs, FeatureValues &featureCounts);

    // updaters for each dimension
    double calculateNewOneDimensionOfWeights(std::vector<Board_Move_Dataset *> &dataSet, size_t updateDim);
    void updateForOneData(const Board_Move_Data &data, size_t updateDim, double &CijofEj, int &featureCountAppearInTraining, Common::Point *freePointBuffer);
    void updateForFeatureVectorsOfOneData(std::vector<Common::SparseVector *> &featureVectors, int index_of_action_in_data, size_t updateDim, double &CijofEj, int &featureCountAppearInTraining);

    // common tools for update
    double calculateProductOfExistFeatureWeights(const Common::SparseVector &features);
    double calculateProductOfTeammateWeights(const Common::SparseVector &features, size_t index, double allProductOfExistFeatureWeights);
    static void releaseSparseVectors(std::vector<Common::SparseVector *> &featureVectors);
  public:
    explicit MM_Learning(FeatureExtractor &featureExtractor);
    explicit MM_Learning(FeatureExtractor &featureExtractor, double stepRate);
    virtual ~MM_Learning() {}

    void initWeights();
    void setParameters(double stepRate);

    const FeatureValues &getFeatureAppearCountsInLastTraining() const {return m_featureAppearCountsInLastTraining;}
    const FeatureWeights &getCijofEjsInLastTraining() const {return m_CijofEjsInLastTraining;}

    void setWeightByAppearCountAndCijofEj(size_t dimension, int appearCount, double CijofEj);
    void setWeightsByAppearCountsAndCijofEjs(const FeatureValues &appearCounts, const FeatureWeights &CijofEjs);

    void learn(std::vector<Board_Move_Dataset *> &dataset);
    void learn_at_once(std::vector<Board_Move_Dataset *> &dataSet);
    void learn_iterative(std::vector<Board_Move_Dataset *> &dataSet);
    void learn_one_dimension(std::vector<Board_Move_Dataset *> &dataSet, size_t updateDim);

    // temporal implementation
    //typedef std::vector<double> FeatureWeight;
    const FeatureWeights &getFeatureWeight() const {return m_featureWeights;}
  };
}
