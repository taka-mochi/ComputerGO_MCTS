#include "precomp.h"

#include "gtest/gtest.h"
#include <stdexcept>

#include "ML/FeatureArray.h"
#include "ML/LightFeatureExtractor_ForTest.h"
#include "ML/ApprenticeshipLearning.h"
#include "ML/LearningRateExponential.h"
#include "ML/LearningRateConstant.h"

using namespace std;
using namespace Common;
using namespace ML;
using namespace Go;

class AL_Test : public ::testing::Test {
protected:
  virtual void SetUp() {
  }

  virtual void TearDown() {
  }

  void test_init() {
    learner.init();
    EXPECT_EQ(1, learner.m_currentStepRate);
    EXPECT_EQ(light_extractor.getFeatureDimension(), learner.m_featureWeights.size());
    for (size_t i=0; i<light_extractor.getFeatureDimension(); i++) {
      EXPECT_EQ(0, learner.m_featureWeights[i]);
    }

    FeatureWeights init_test(light_extractor.getFeatureDimension());
    for (size_t i=0; i<init_test.size(); i++) {
      init_test[i] = i*i;
    }
    learner.init(&init_test);
    for (size_t i=0; i<init_test.size(); i++) {
      EXPECT_EQ(init_test[i], learner.m_featureWeights[i]);
    }
  }

  void test_calcGradientOfFeatureVectors() {
    {
      // iter 1
      SparseVector f1,f2,f3;
      f1.add(0); f1.add(1); f1.add(4);             // (1,1,0,0,1,0)
      f2.add(1); f2.add(3); f2.add(4); f2.add(5);  // (0,1,0,1,1,1)
      f3.add(2); f3.add(4);                        // (0,0,1,0,1,0)

      double expected[6] = {
        -1.0/3, 1.0/3, -1.0/3, 2.0/3, 0.0/3, 2.0/3
      };

      learner.init();
      FeatureWeights result(6);

      // learn
      vector<SparseVector *> features; features.push_back(&f1); features.push_back(&f2); features.push_back(&f3);
      int correctIndex = 1;
      learner.calculateGradientOfFeatureVectors(features, correctIndex, result);

      // verity
      for (size_t i=0; i<6; i++) {
        if (!(fabs(result[i]-expected[i]) < 0.001)) {
          cerr << "dim " << i << endl << "expected = " << expected[i] << endl << "actual = " << result[i] << endl;
        }
        EXPECT_TRUE(fabs(result[i]-expected[i]) < 0.001);
      }

      learner.init(&result);
    }

    {
      // iter 2. another data
      FeatureWeights result(6);
      SparseVector f1,f2,f3,f4;
      f1.add(4);                       // (0,0,0,0,1,0)
      f2.add(0); f2.add(1); f2.add(2); // (1,1,1,0,0,0)
      f3.add(0); f3.add(5);            // (1,0,0,0,0,1)
      f4.add(0); f4.add(3); f4.add(5); // (1,0,0,1,0,1)

      // weights = (-1/3,1/3,-1/3,2/3,0,2/3)

      double normalizer = 1 + exp(1) + exp(1.0/3) + exp(-1.0/3);
      double expected[6] = {
        1 - (exp(-1.0/3) + exp(1.0/3) + exp(1))/normalizer,
        0 - (exp(-1.0/3))/normalizer,
        0 - (exp(-1.0/3))/normalizer,
        1 - (exp(1))/normalizer,
        0 - (1)/normalizer,
        1 - (exp(1.0/3) + exp(1))/normalizer
      };

      vector<SparseVector *> features; features.push_back(&f1); features.push_back(&f2); features.push_back(&f3); features.push_back(&f4);
      int correctIndex = 3;
      learner.calculateGradientOfFeatureVectors(features, correctIndex, result);

      // verity
      for (size_t i=0; i<6; i++) {
        if (!(fabs(result[i]-expected[i]) < 0.001)) {
          cerr << "dim " << i << endl << "expected = " << expected[i] << endl << "actual = " << result[i] << endl;
        }
        EXPECT_TRUE(fabs(result[i]-expected[i]) < 0.001);
      }
    }
  }

  void createTestDataset(Board_Move_Dataset_vector &dataset) {
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
      ApprenticeshipLearning::releaseSparseVectors(features);
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
      GoLearning::convertOneDataToFeatureVectors(learner.m_featureExtractor, dataset.get(), buffer, features, index);
      EXPECT_EQ(5, (signed)features.size());
      ApprenticeshipLearning::releaseSparseVectors(features);
      // cout << "train index = " << index << endl;
      // for (int i=0; i<(signed)features.size(); i++) {
      //   cout << features[i]->toString() << endl;
      // }
    }
    dataset.seekToBegin();
  }

  void test_calcGradientOfOneData() {
    Board_Move_Dataset_vector dataset;
    createTestDataset(dataset);

    {
      double normalizer_2 = exp(4) + 3*exp(3) + 1;
      double expected_weights_per_iterations[2][6] = {
        {5.0/6, 1.0/3, -1.0/6,0,0,0},
        {1 - 2*exp(3)/normalizer_2, 1 - 2*exp(3)/normalizer_2, -exp(3)/normalizer_2, -exp(4)/normalizer_2, 0, 0}
      };

      learner.init();
      dataset.seekToBegin();
      FeatureWeights result(6);
      Point buffer[MAX_BOARD_SIZE+1];

      learner.calculateGradientOfOneData(dataset.get(), result, buffer);
      for (size_t i=0; i<3; i++) {
        EXPECT_TRUE(fabs(result[i] - expected_weights_per_iterations[0][i]) < 0.001);
      }

      for (size_t i=0; i<6; i++) {
        learner.m_featureWeights[i] = i+1;
      }
      dataset.next();
      learner.calculateGradientOfOneData(dataset.get(), result, buffer);
      for (size_t i=0; i<4; i++) {
        EXPECT_TRUE(fabs(result[i] - expected_weights_per_iterations[1][i]) < 0.001);
      }
      
    }
  }

  void test_learn() {
    Board_Move_Dataset_vector dataset;
    createTestDataset(dataset);

    learner.init();

    vector<Board_Move_Dataset *> wrapper; wrapper.push_back(&dataset);
    learner.learn(wrapper);
    double normalizer = 2 + 2*exp(7.0/6) + exp(-1.0/6);
    double expected[6] = {
      // 5/6, 1/3, -1/6, 0,0,0: after iter 1
      5.0/6 + 0.9*(1 - 2*exp(7.0/6)/normalizer),
      1.0/3 + 0.9*(1 - 2*exp(7.0/6)/normalizer),
      -1.0/6+ 0.9*(-exp(-1.0/6)/normalizer),
      + 0.9*(-1/normalizer), 0 , 0 // : after iter 2
    };

    for (size_t i=0; i<6; i++) {
      if (!(fabs(learner.getFeatureWeight()[i] - expected[i]) < 0.001)) {
        cerr << "dim " << i << endl;
        cerr << "expected: " << expected[i] << endl;
        cerr << "actual: " << learner.getFeatureWeight()[i] << endl;
      }
      EXPECT_TRUE(fabs(learner.getFeatureWeight()[i] - expected[i]) < 0.001);
    }
     
  }

  void test_learnWithRegTerm() {
    Board_Move_Dataset_vector dataset;
    createTestDataset(dataset);

    LearningRateConstant regConst(0.8);
    ApprenticeshipLearning mylearner(light_extractor, lrcalc, regConst);
    mylearner.init();

    vector<Board_Move_Dataset *> wrapper; wrapper.push_back(&dataset);
    mylearner.learn(wrapper);
    double normalizer = 2 + 2*exp(7.0/6) + exp(-1.0/6);
    double expected[6] = {
      // 5/6, 1/3, -1/6, 0,0,0: after iter 1
      5.0/6 + 0.9*(1 - 2*exp(7.0/6)/normalizer) - 0.9*0.8 * 5.0 / 6,
      1.0/3 + 0.9*(1 - 2*exp(7.0/6)/normalizer) - 0.9*0.8 * 1.0 / 3,
      -1.0/6+ 0.9*(-exp(-1.0/6)/normalizer) + 0.9*0.8 * 1.0 / 6,
      + 0.9*(-1/normalizer), 0 , 0 // : after iter 2
    };

    for (size_t i=0; i<6; i++) {
      if (!(fabs(mylearner.getFeatureWeight()[i] - expected[i]) < 0.001)) {
        cerr << "dim " << i << endl;
        cerr << "expected: " << expected[i] << endl;
        cerr << "actual: " << mylearner.getFeatureWeight()[i] << endl;
      }
      EXPECT_TRUE(fabs(mylearner.getFeatureWeight()[i] - expected[i]) < 0.001);
    }
     
  }

  LightFeatureExtractor_ForTest light_extractor;
  ApprenticeshipLearning learner;
  LearningRateExponential lrcalc;

  AL_Test()
    : light_extractor()
    , learner(light_extractor, lrcalc)
    , lrcalc(0.9)
  {
  }
};

TEST_F(AL_Test, init) {
  test_init();
}

TEST_F(AL_Test, calcGradientOfFeatureVectors) {
  test_calcGradientOfFeatureVectors();
}

TEST_F(AL_Test, calcGraiendOfOneData) {
  test_calcGradientOfOneData();
}

TEST_F(AL_Test, learn) {
  test_learn();
}

TEST_F(AL_Test, learnWithRegularizationTerm) {
  test_learnWithRegTerm();
}
