#pragma once

#include <vector>
#include "ML/GoLearning.h"
#include "ML/FeatureArray.h"
#include "utility/SparseVector.h"

class AL_Test;

namespace ML {
  class FeatureExtractor;
  class Board_Move_Dataset;
  class Board_Move_Data;
  class LearningRateCalculator;

  class ApprenticeshipLearning : public GoLearning {
  public:
    friend class ::AL_Test;

  private:
    FeatureExtractor &m_featureExtractor;

    FeatureWeights m_featureWeights;

    LearningRateCalculator &m_learnRateCalc;

    int m_iteration;
    double m_currentStepRate;

    bool m_addReguralizationTerm;
    LearningRateCalculator &m_regTermRateCalc;
    double m_currentRegTermRate;

  private:
    // internal methods
    void calculateGradientOfOneData(const Board_Move_Data &data, FeatureWeights &result, Common::Point *pointBuffer);
    void calculateGradientOfFeatureVectors(const std::vector<Common::SparseVector *> &features, 
                                           int index_of_training, FeatureWeights &result);
    static void releaseSparseVectors(std::vector<Common::SparseVector *> &features);

    double calculateLoglikelihoodOfOneData(const Board_Move_Data &data, Common::Point *pointBuffer);

  public:
    explicit ApprenticeshipLearning(FeatureExtractor &extractor, LearningRateCalculator &lrCalc);
    explicit ApprenticeshipLearning(FeatureExtractor &extractor, LearningRateCalculator &lrCalc, LearningRateCalculator &regTermRateCalc);
    virtual ~ApprenticeshipLearning() {}

    void init(FeatureWeights *init_weights = NULL);

    void learn(std::vector<Board_Move_Dataset *> &dataset);

    double calculateLoglikelihoodValue(std::vector<Board_Move_Dataset *> &dataset);

    bool isReguralizationTermEnabled() const {return m_addReguralizationTerm;}

    const FeatureWeights &getFeatureWeight() const {return m_featureWeights;}
  };
}
