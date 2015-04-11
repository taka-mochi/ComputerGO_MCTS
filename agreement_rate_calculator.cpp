#include "precomp.h"

#include "main_functions.h"

#include <stdio.h>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sstream>

#include <getopt.h>

#include "AI/PureMCAI.h"
#include "AI/NormalPlayout.h"
#include "AI/UCTPlayer.h"
#include "AI/UCTBoundCalculator.h"
#include "AI/UCTNode.h"
#include "AI/SoftmaxPolicyPlayout.h"

#include "AI/Parameters.h"

#include "ML/Board_Move_Dataset_record.h"

using namespace std;
using namespace Common;
using namespace Go;
using namespace ML;
using namespace AI;

static int START_SEQUENCE_THRESHOLD = 0;

void evalAgreemenRate(vector<Board_Move_Dataset *> dataset, unsigned int randomSeed, vector<int> &correctCount, vector<int> &positionCount);

void printHelp() {
  cout << "./agreement_calculator [PARAMETERS (below)]" << endl
       << "-h: Help" << endl
       << "--paramfile PARAMETER_FILE : a parameter file" << endl
       << "-i INPUT_FILES_LIST_FILE   : a list file for input file" << endl
       << "-r seed                    : a random seed" << endl
       << "-o output file             : a file to output result" << endl
       << "-t start move count threshold : a threshold of move count to calc agreement rate" << endl
       << endl;
}

void showInformation(std::ostream &ou, unsigned int randomSeed) {
  ou << "--- General Information ---" << endl;
  ou << "Random Seed: " << randomSeed << endl;
  if (AI::Parameters::getInstanceOfColor(BLACK).isResignAllowed()) {
    ou << "Black Resign: allowed" << endl;
  }
  
  Color c[] = {BLACK, WHITE};
  for (int i=0; i<2; i++) {
    ou << "--- UCT Search Information of " << (c == 0 ? "black" : "white" ) << "---" << endl;
    AI::Parameters &params = AI::Parameters::getInstanceOfColor(c[i]);
    ou << "C = " << params.getUCT_C() << endl;
    ou << "K = " << params.getRave_K() << endl;
    ou << "M = " << params.getRave_M() << endl;
    ou << "Expand Threshold = " << params.getThreshold_Expand() << endl;
    ou << "Playout Limit = " << params.getPlayoutLimit() << endl;
    ou << "Standard Features Weight File = " << params.getStandardFeatureWeightFile() << endl;
    ou << "Player Kind = " << AI::Parameters::PLAYER_KIND::toString(params.getPlayerKind()) << endl;

    ou << "--- End of Information ---" << endl;
    ou << endl;
  }
}

void get_input_files(const string &filelist_file, vector<string> &buf) {
  ifstream ifs(filelist_file.c_str());
  if (!ifs) {
    cerr << "Failed to read input file " << filelist_file << endl;
    printHelp();
    exit(0);
  }
  string line;
  buf.clear();
  while (!getline(ifs, line).eof()) {
    buf.push_back(line);
  }
  if (buf.empty()) {
    cerr << "No input file in " << filelist_file << endl;
    printHelp();
    exit(0);
  }
}

StandardFeatureExtractor featureExtractor;
std::vector<std::shared_ptr<FeatureWeights> > featureWeights;

int main(int argc, char **argv) {
  int option_index = 0;
  int c;

#define OPTION_COUNT 1

  option *options = new option[OPTION_COUNT+1];

  options[0].name = "paramfile";
  options[0].has_arg = required_argument;
  options[0].flag = NULL;
  options[0].val = 1;

  options[OPTION_COUNT].name = NULL;
  options[OPTION_COUNT].has_arg = 0;
  options[OPTION_COUNT].flag = NULL;
  options[OPTION_COUNT].val = 0;

  unsigned int randomSeed = time(NULL);
  string outputFile = "";
  string inputFile = "";
  string paramfile = "";

  while ((c = getopt_long(argc, argv, "hi:r:o:t:", options, &option_index)) != -1) {
    switch(c) {
    case 1:
      paramfile = optarg;
      break;

    case 't':
      START_SEQUENCE_THRESHOLD = atoi(optarg);
      if (START_SEQUENCE_THRESHOLD < 0) START_SEQUENCE_THRESHOLD = 0;
      break;

    case 'i':
      inputFile = optarg;
      break;
      
    case 'r':
      randomSeed = atoi(optarg);
      break;
    case 'o':
      outputFile = optarg;
      break;
      
    case 'h':
      printHelp();
      exit(0);
    }
  }

  delete [] options;

  //Go::Block::initAllocator();

  if (paramfile == "") {
      //!AI::Parameters::getInstanceOfColor(BLACK).readParametersFromFile(paramfile)) {
      //!AI::Parameters::getInstanceOfColor(WHITE).readParametersFromFile(paramfile)) {
    cerr << "Failed to read a parameter file: " << paramfile << endl;
    printHelp();
    exit(-1);
  }
  //AI::Parameters::getInstanceOfColor(BLACK).setResignDisabled();
  //AI::Parameters::getInstanceOfColor(WHITE).setResignDisabled();

  if (inputFile == "") {
    cerr << "Specify input file" << endl;
    printHelp();
    exit(-1);
  }
  if (outputFile == "") {
    cerr << "Specify output file" << endl;
    printHelp();
    exit(-1);
  }

  {
    ofstream ofs(outputFile.c_str());
    if (!ofs) {
      cerr << "failed to attach output file: " << outputFile << endl;
      exit(-1);
    }
  }

  {
    // read patterns and weights
    vector<ML::PATTERN_3x3_HASH_VALUE> patterns;
    vector<shared_ptr<FeatureWeights> > weights;
    StandardFeatureExtractor::readPatternsAndWeightsFromFile(paramfile, patterns, weights);
    for (size_t i=0; i<patterns.size(); i++) {
      featureExtractor.registerPatternAsFeature(patterns[i]);
    }
    featureWeights = weights;
  }

  // show information
  //showInformation(cerr, randomSeed);
  cout << "RandomSeed," << randomSeed << endl;
  cerr << "input_file:" << inputFile << endl << 
    " output_file:" << outputFile << endl <<
    " paramfile: " << paramfile << endl <<
    " seed:" << randomSeed << endl <<
    " start calc threshold: " << START_SEQUENCE_THRESHOLD << endl;


  // read sgfs
  vector<string> inputSgfList;
  get_input_files(inputFile, inputSgfList);

  Common::StdlibRandom r(randomSeed);
  random_shuffle(inputSgfList.begin(), inputSgfList.end(), r);

  // read dataset
  vector<Board_Move_Dataset *> dataset;
  int all_position_count = 0;
  int eval_position_count = 0;
  for (size_t i=0; i<inputSgfList.size(); i++) {
    cerr << "reading SGF file: " << inputSgfList[i] << endl;
    Board_Move_Dataset_record *newset = new Board_Move_Dataset_record;
    if (!newset->readAndAddRecordFromSGF(inputSgfList[i])) {
      cerr << "failed to read " << inputSgfList[i] << endl;
    }
    if (!newset->empty()) {
      size_t positions = newset->getRecord(0)->getMoveSequence().size();
      all_position_count += positions;
      if (positions > START_SEQUENCE_THRESHOLD) {
        eval_position_count += (positions - START_SEQUENCE_THRESHOLD);
        dataset.push_back(newset);
      } else {
        delete newset;
      }
    } else {
      delete newset;
    }
  }
  
  cerr << "Evaluation position count/All position count = " << eval_position_count << "/" << all_position_count << endl;

  vector<int> correctCount, positionCount;
  evalAgreemenRate(dataset, randomSeed, correctCount, positionCount);

  ofstream ofs(outputFile);
  for (size_t i=0; i<correctCount.size(); i++) {
    stringstream ss;

    ss << static_cast<double>(correctCount[i])/positionCount[i] << "," << correctCount[i] << "," << positionCount[i];

    cout << ss.str() << endl;
    ofs << ss.str() << endl;

  }

  for (size_t i=0; i<dataset.size(); i++) {
    delete dataset[i];
  }

  return 0;
}

void evalAgreemenRate(vector<Board_Move_Dataset *> dataset, unsigned int randomSeed, vector<int> &correctCount, vector<int> &positionCount) {
  // init seed of random
  srand(randomSeed);

  correctCount.clear();
  positionCount.clear();

  SparseVector featureTable[MAX_BOARD_SIZE];
  SparseVector passFeature;

  int corrects = 0, positions = 0;

  vector<shared_ptr<FeatureWeights> >::iterator it, end = featureWeights.end();
  for (it = featureWeights.begin(); it!=end; it++) {
    shared_ptr<FeatureWeights> weights = *it;

    corrects = positions = 0;

    for (size_t i=0; i<dataset.size(); i++) {      

      Board_Move_Dataset &set = *dataset[i];
      set.seekToBegin();

      if (set.empty()) continue;

      Board_Move_Data init_data(set.get());

      //std::shared_ptr<AI::PlayerBase> players[2] = {
      //  AI::Parameters::getInstanceOfColor(BLACK).createPlayerAccordingToKind(init_data.state.get(), 1),
      //  AI::Parameters::getInstanceOfColor(WHITE).createPlayerAccordingToKind(init_data.state.get(), 1)
      //};

      do {
        Board_Move_Data data(set.get());

        Color turn = data.turn;

        // extract features
        PointSet possibleMoves;
        featureExtractor.extractFromStateForAllMoves(data.state.get(), turn, featureTable, passFeature, possibleMoves);
        double maxValue = -99999999999;
        Point maxMove = POINT_NULL;
        for (size_t i=0; i<possibleMoves.size(); i++) {
          Point p = possibleMoves[i];
          SparseVector &vec = featureTable[p];
          double sum = weights->dot(vec);
          if (sum > maxValue) {
            maxValue = sum;
            maxMove = p;
          }
        }
        double sum = weights->dot(passFeature);
        if (sum > maxValue) {
          maxValue = sum;
          maxMove = PASS;
        }

        //Point selected = players[turn == BLACK ? 0 : 1]->selectBestMove(turn);
        //data.state->printToErr();

        if (maxMove == data.move) {
          corrects++;
          //cerr << "correct " << endl;
        }
        positions++;

      } while (set.next());
      cerr << "end dataset " << i << endl;
    };

    correctCount.push_back(corrects);
    positionCount.push_back(positions);
  }
}

