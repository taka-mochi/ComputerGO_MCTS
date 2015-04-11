#include "precomp.h"

#include "gtest/gtest.h"
#include <stdexcept>
#include "ExecuteParameter.h"
#include "AI/Parameters.h"

#include <ctime>
#include <execinfo.h>

using namespace std;
using namespace Common;
using namespace Go;
using namespace ML;
using namespace AI;


TEST(ParametersTest, ExecuteParameter) {
  // nothing to test
}

TEST(AI_ParametersTest, PlayerKind) {
  EXPECT_EQ(AI::Parameters::PLAYER_KIND::PURE_MC, AI::Parameters::PLAYER_KIND::fromString("Pure MC"));
  EXPECT_EQ(AI::Parameters::PLAYER_KIND::UCT_PLAIN, AI::Parameters::PLAYER_KIND::fromString("Plain UCT"));
  EXPECT_EQ(AI::Parameters::PLAYER_KIND::UCT_PLAIN_RAVE, AI::Parameters::PLAYER_KIND::fromString("Plain UCT Rave"));
  EXPECT_EQ(AI::Parameters::PLAYER_KIND::UCT_SOFTMAX_STANDARD_FEATURES, AI::Parameters::PLAYER_KIND::fromString("Softmax UCT Standard Features"));
  EXPECT_EQ(AI::Parameters::PLAYER_KIND::UCT_SOFTMAX_STANDARD_FEATURES_RAVE, AI::Parameters::PLAYER_KIND::fromString("Softmax UCT Rave Standard Features"));
  EXPECT_EQ(AI::Parameters::PLAYER_KIND::PURE_MC, AI::Parameters::PLAYER_KIND::fromString("pure mc"));
  EXPECT_EQ(AI::Parameters::PLAYER_KIND::UCT_PLAIN, AI::Parameters::PLAYER_KIND::fromString("plain uct"));
  EXPECT_EQ(AI::Parameters::PLAYER_KIND::UCT_PLAIN_RAVE, AI::Parameters::PLAYER_KIND::fromString("plain uct rave"));
  EXPECT_EQ(AI::Parameters::PLAYER_KIND::UCT_SOFTMAX_STANDARD_FEATURES, AI::Parameters::PLAYER_KIND::fromString("softmax uct standard features"));
  EXPECT_EQ(AI::Parameters::PLAYER_KIND::UCT_SOFTMAX_STANDARD_FEATURES_RAVE, AI::Parameters::PLAYER_KIND::fromString("softmax uct rave standard features"));

  EXPECT_EQ(AI::Parameters::PLAYER_KIND::INVALID_PLAYER_KIND, AI::Parameters::PLAYER_KIND::fromString("PureMC"));
  EXPECT_EQ(AI::Parameters::PLAYER_KIND::INVALID_PLAYER_KIND, AI::Parameters::PLAYER_KIND::fromString("Plain UC"));
  EXPECT_EQ(AI::Parameters::PLAYER_KIND::INVALID_PLAYER_KIND, AI::Parameters::PLAYER_KIND::fromString("Plain UCT Raves"));
  EXPECT_EQ(AI::Parameters::PLAYER_KIND::INVALID_PLAYER_KIND, AI::Parameters::PLAYER_KIND::fromString("-Softmax UCT Standard Features"));
  EXPECT_EQ(AI::Parameters::PLAYER_KIND::INVALID_PLAYER_KIND, AI::Parameters::PLAYER_KIND::fromString("UCT Softmax Rave Standard Features"));

  EXPECT_EQ(string("Pure MC"), AI::Parameters::PLAYER_KIND::toString(AI::Parameters::PLAYER_KIND::PURE_MC));
  EXPECT_EQ(string("Plain UCT"), AI::Parameters::PLAYER_KIND::toString(AI::Parameters::PLAYER_KIND::UCT_PLAIN));
  EXPECT_EQ(string("Plain UCT Rave"), AI::Parameters::PLAYER_KIND::toString(AI::Parameters::PLAYER_KIND::UCT_PLAIN_RAVE));
  EXPECT_EQ(string("Softmax UCT Standard Features"), AI::Parameters::PLAYER_KIND::toString(AI::Parameters::PLAYER_KIND::UCT_SOFTMAX_STANDARD_FEATURES));
  EXPECT_EQ(string("Softmax UCT Rave Standard Features"), AI::Parameters::PLAYER_KIND::toString(AI::Parameters::PLAYER_KIND::UCT_SOFTMAX_STANDARD_FEATURES_RAVE));
}

TEST(AI_ParametersTest, parameterSettings) {
  AI::Parameters params;

  EXPECT_TRUE(params.isResignAllowed());
  params.setResignDisabled();
  EXPECT_FALSE(params.isResignAllowed());

  params.setPlayoutLimit(200);
  EXPECT_EQ(200, params.getPlayoutLimit());

  params.setUCT_C(0.31);
  EXPECT_EQ(0.31, params.getUCT_C());

  params.setRave_K(5.43);
  EXPECT_EQ(5.43, params.getRave_K());

  params.setRave_M(123.4);
  EXPECT_EQ(123.4, params.getRave_M());

  params.setThreshold_Expand(49);
  EXPECT_EQ(49, params.getThreshold_Expand());

  EXPECT_EQ(string(""), params.getStandardFeatureWeightFile());
  EXPECT_EQ(AI::Parameters::PLAYER_KIND::INVALID_PLAYER_KIND, params.getPlayerKind());
}

TEST(AI_ParametersTest, readFromFile) {
  AI::Parameters params;

  EXPECT_TRUE(params.readParametersFromFile("parameter_files/parameters_for_gtest.txt"));

  EXPECT_EQ(0.023, params.getUCT_C());
  EXPECT_EQ(18, params.getRave_K());
  EXPECT_EQ(500, params.getRave_M());
  EXPECT_EQ(25, params.getThreshold_Expand());
  EXPECT_EQ(2674, params.getPlayoutLimit());
  EXPECT_FALSE(params.isResignAllowed());
  EXPECT_EQ(string("save_result/stochastic_results/for_all_data/stochastic_result_alldata_inv0.1_r10_iter30"), params.getStandardFeatureWeightFile());
  EXPECT_EQ(AI::Parameters::PLAYER_KIND::UCT_PLAIN_RAVE, params.getPlayerKind());

  EXPECT_FALSE(params.readParametersFromFile("parameter_files/nulllll_fileeeeeee.txt"));
}

