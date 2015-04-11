#include "precomp.h"

#include "gtest/gtest.h"
#include <stdexcept>

#include "ML/FeatureArray.h"
#include "ML/LightFeatureExtractor_ForTest.h"
#include "ML/MM_Learning.h"
#include "ML/LearningRateExponential.h"
#include "ML/ApprenticeshipLearning.h"

using namespace std;
using namespace Common;
using namespace ML;
using namespace Go;

void createTestDataset(Board_Move_Dataset_vector &dataset, LightFeatureExtractor_ForTest &light_extractor, ApprenticeshipLearning &learner) {
  {
    // data1
    int init_array[] = {
      3,3,3,3,3,3,3,
      3,2,2,2,1,1,3,
      3,2,2,2,1,1,3,
      3,2,2,1,0,0,3,
      3,2,2,2,1,1,3,
      3,0,2,1,0,0,3,
      3,3,3,3,3,3,3
    };
    // (0,1,0,0,0,0)
    // (0,1,0,0,0,0)
    // (0,1,0,0,0,0)
    // (0,0,1,0,0,0)
    // (1,1,0,0,0,0) <= training
    // (0,0,0,0,0,0): PASS
    shared_ptr<Board> board(new Board(5, init_array));
    Point train_move = board->xyToPoint(0,4);
    dataset.addData(board, train_move, 1, POINT_NULL);
    std::vector<SparseVector *> features;
    Point buffer[MAX_BOARD_SIZE+1]; int index;
    GoLearning::convertOneDataToFeatureVectors(light_extractor, dataset.get(), buffer, features, index);
    EXPECT_EQ(6, (signed)features.size());
    //ApprenticeshipLearning::releaseSparseVectors(features);
    // cout << "train index = " << index << endl;
    // for (int i=0; i<(signed)features.size(); i++) {
    //   cout << features[i]->toString() << endl;
    // }
  }
  {
    // data2
    int init_array[] = {
      3,3,3,3,3,3,3,
      3,1,1,0,2,1,3,
      3,1,2,2,1,1,3,
      3,1,2,0,0,1,3,
      3,1,2,2,1,1,3,
      3,1,1,0,2,1,3,
      3,3,3,3,3,3,3
    };
    // (0,0,0,1,0,0)
    // (0,0,1,0,0,0)
    // (1,1,0,0,0,0)
    // (1,1,0,0,0,0) <= training
    // (0,0,0,0,0,0): PASS
    shared_ptr<Board> board(new Board(5, init_array));
    Point train_move = board->xyToPoint(2,0);
    dataset.addData(board, train_move, 1, POINT_NULL);

    std::vector<SparseVector *> features;
    Point buffer[MAX_BOARD_SIZE+1]; int index;
    dataset.next();
    GoLearning::convertOneDataToFeatureVectors(light_extractor, dataset.get(), buffer, features, index);
    EXPECT_EQ(5, (signed)features.size());
    //ApprenticeshipLearning::releaseSparseVectors(features);
    // cout << "train index = " << index << endl;
    // for (int i=0; i<(signed)features.size(); i++) {
    //   cout << features[i]->toString() << endl;
    // }
  }
  dataset.seekToBegin();
}


TEST(SoftMax_CostFunc, calcWithInitWeight) {
  LearningRateExponential exp(0.9);
  LightFeatureExtractor_ForTest extractor;
  ApprenticeshipLearning learner(extractor, exp);

  Board_Move_Dataset_vector dataset;
  createTestDataset(dataset, extractor, learner);

  vector<Board_Move_Dataset *> dataset_wrap;
  dataset_wrap.push_back(&dataset);

  // calculate it
  double v = learner.calculateLoglikelihoodValue(dataset_wrap);

  EXPECT_TRUE((v-(log(6)+log(5)))<0.001);
}

TEST(SoftMax_CostFunc, calcWithRandomWeight) {
  LearningRateExponential e(0.9);
  LightFeatureExtractor_ForTest extractor;
  ApprenticeshipLearning learner(extractor, e);

  // set initial weights
  FeatureWeights weights(extractor.getFeatureDimension());
  for (int i=0; i<(signed)extractor.getFeatureDimension(); i++) {
    weights[i] = 0.1 - i/10.0;
  }
  // 0.1, 0.0, -0.1, -0.2, -0.3, -0.4

  Board_Move_Dataset_vector dataset;
  createTestDataset(dataset, extractor, learner);

  vector<Board_Move_Dataset *> dataset_wrap;
  dataset_wrap.push_back(&dataset);

  double expect_data_1 = +0.1 - log(4*exp(0.1) + exp(-0.1) + 1);
  double expect_data_2 = +0.1 - log(exp(-0.2) + exp(-0.1) + 2*exp(0.1) + 1);
  double expect = expect_data_1 + expect_data_2;

  // calculate it
  double v = learner.calculateLoglikelihoodValue(dataset_wrap);
  EXPECT_TRUE((v-expect) < 0.001);
}
