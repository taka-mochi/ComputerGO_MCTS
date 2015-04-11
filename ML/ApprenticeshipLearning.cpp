#include "precomp.h"

#include <sys/time.h>
#include "ML/ApprenticeshipLearning.h"
#include "ML/FeatureExtractor.h"
#include "ML/Board_Move_Dataset.h"
#include "ML/LearningRateCalculator.h"
#include "ML/LearningRateConstant.h"

using namespace std;
using namespace Common;
using namespace Go;

namespace ML {

  static LearningRateConstant dummyCalc(0);

  ApprenticeshipLearning::ApprenticeshipLearning(FeatureExtractor &extractor, LearningRateCalculator &lrCalc)
    : m_featureExtractor(extractor)
    , m_featureWeights(extractor.getFeatureDimension())
    , m_learnRateCalc(lrCalc)
    , m_iteration(0)
    , m_currentStepRate(1.0)      
    , m_addReguralizationTerm(false)
    , m_regTermRateCalc(dummyCalc)
    , m_currentRegTermRate(1.0)
  {
    init();
  }

  ApprenticeshipLearning::ApprenticeshipLearning(FeatureExtractor &extractor, LearningRateCalculator &lrCalc, LearningRateCalculator &regTermRateCalc)
    : m_featureExtractor(extractor)
    , m_featureWeights(extractor.getFeatureDimension())
    , m_learnRateCalc(lrCalc)
    , m_iteration(0)
    , m_currentStepRate(1.0)      
    , m_addReguralizationTerm(true)
    , m_regTermRateCalc(regTermRateCalc)
    , m_currentRegTermRate(1.0)
  {
    init();
  }

  void ApprenticeshipLearning::init(FeatureWeights *init_weights) {
    m_currentStepRate = 1.0;
    m_currentRegTermRate = 1.0;
    m_iteration = 0;
    if (init_weights == NULL) {
      m_featureWeights.init(m_featureExtractor.getFeatureDimension());
    } else {
      m_featureWeights = *init_weights;
    }
  }

  // Entry point of AL
  void ApprenticeshipLearning::learn(vector<Board_Move_Dataset *> &dataset) {
    Point pointBuffer[MAX_BOARD_SIZE+1];

    FeatureWeights grad(m_featureExtractor.getFeatureDimension());
    FeatureWeights tmp_grad(m_featureExtractor.getFeatureDimension());

    timeval begin,end;
    gettimeofday(&begin, NULL);

    int t = m_iteration;
    for (size_t i=0; i<dataset.size(); i++) {
      Board_Move_Dataset &set = *dataset[i];
      set.seekToBegin();
      if (!set.empty()) {
        do {
          Board_Move_Data data(set.get());
          calculateGradientOfOneData(data, grad, pointBuffer);

          if (m_addReguralizationTerm) {
            double rate = m_regTermRateCalc(m_iteration, m_currentRegTermRate);
            tmp_grad = m_featureWeights;
            tmp_grad *= rate;
            grad -= tmp_grad;//m_featureWeights * rate;
            m_currentRegTermRate = rate;
          }
 
          //grad *= m_stepRateBase/(1+m_stepRateBase*t);
          double stepRate = m_learnRateCalc(m_iteration, m_currentStepRate);
          grad *= stepRate;
          m_iteration++;
          m_currentStepRate = stepRate;
          
          m_featureWeights += grad;
          
        } while (set.next());
      }
      cerr << "end dataset " << i << endl;
    }

    gettimeofday(&end, NULL);
    double begin_sec = (double)begin.tv_sec + (double)begin.tv_usec * 1e-6;
    double end_sec = (double)end.tv_sec + (double)end.tv_usec * 1e-6;
    cerr << "End Learning. Time (sec) = " << end_sec - begin_sec << " , position count = " << m_iteration - t << endl;
  }

  // internal methods

  // Calculate gradients of a given data
  void ApprenticeshipLearning::calculateGradientOfOneData(const Board_Move_Data &data, FeatureWeights &result, Point *pointBuffer) {
    static std::vector<SparseVector *> features;
    int index_of_training = -1;
    features.clear();
    features.reserve(MAX_BOARD_SIZE);
   
    GoLearning::convertOneDataToFeatureVectors(m_featureExtractor, data, pointBuffer, features, index_of_training);

    if (features.size() == 0) {
      // no training data
      result.clear();
      return;
    }
    assert(index_of_training>=0 && index_of_training < (signed)features.size());

    // calculate gradient
    calculateGradientOfFeatureVectors(features, index_of_training, result);

    // release temporal variables
    releaseSparseVectors(features);
  }

  void ApprenticeshipLearning::calculateGradientOfFeatureVectors(const std::vector<Common::SparseVector *> &features, 
                                                                 int index_of_training, FeatureWeights &result) {
    double totalOfExpValues = 0;
    result.clear();

    // calculate the gradients
    // 2nd term
    for (size_t i=0; i<features.size(); i++) {
      SparseVector *vec = features[i];
      double expdot = exp(m_featureWeights.dot(*features[i]));
      totalOfExpValues += expdot;

      for (size_t j=0; j<vec->size(); j++) {
        result[vec->at(j)] -= expdot;
      }
    }
    result *= (1.0/totalOfExpValues);
    
    // add 1st term
    SparseVector *correctVector = features[index_of_training];
    for (size_t i=0; i<correctVector->size(); i++) {
      result[correctVector->at(i)] += 1;
    }
  }

  void ApprenticeshipLearning::releaseSparseVectors(std::vector<Common::SparseVector *> &features) {
    for (size_t i=0; i<features.size(); i++) {
      features[i]->clear();
      SparseVectorAllocator::getInstance().release(features[i]);
    }    
  }

  double ApprenticeshipLearning::calculateLoglikelihoodValue(std::vector<Board_Move_Dataset *> &dataset) {
    // calculate soft-max policy values on 
    double value = 0;
    Point pointBuffer[MAX_BOARD_SIZE+1];

    for (size_t i=0; i<dataset.size(); i++) {
      Board_Move_Dataset &set = *dataset[i];
      set.seekToBegin();
      if (!set.empty()) {
        do {
          Board_Move_Data data(set.get());

          value += calculateLoglikelihoodOfOneData(data, pointBuffer);
        } while (set.next());
      }
    }

    return value;
  }
  double ApprenticeshipLearning::calculateLoglikelihoodOfOneData(const Board_Move_Data &data, Common::Point *pointBuffer) {
    std::vector<SparseVector *> features;
    int index_of_training = -1;
 
    GoLearning::convertOneDataToFeatureVectors(m_featureExtractor, data, pointBuffer, features, index_of_training);
    if (features.size() == 0) {
      // no training data
      return 0.0;
    }

    assert(index_of_training>=0 && index_of_training < (signed)features.size());

    // calculate log of policy
    double value = 0;
    double sum_of_2ndterm = 0;
    for (size_t i=0; i<features.size(); i++) {
      double dot_value = m_featureWeights.dot(*features[i]);
      sum_of_2ndterm += exp(dot_value);
    }
    value = m_featureWeights.dot(*features[index_of_training])
      - log(sum_of_2ndterm);

    releaseSparseVectors(features);

    return value;
  }
    
}
