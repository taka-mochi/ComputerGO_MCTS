
#include "precomp.h"

#include <getopt.h>
#include <algorithm>
#include <fstream>

#include "ML/FeatureArray.h"
#include "ML/Board_Move_Dataset_record.h"
#include "ML/MM_Learning.h"
#include "ML/StandardFeatureExtractor.h"
#include "ML/ApprenticeshipLearning.h"
#include "utility/utility.h"
#include "ML/LearningRateInverseOfIteration.h"
#include "ML/LearningRateSqrtOfInverseOfIteration.h"
#include "ML/LearningRateConstant.h"

#ifdef USE_MPI
#include "mpi.h"
#endif // USE_MPI

using namespace std;
using namespace Common;
using namespace ML;

#ifdef USE_MPI
int my_rank;
#endif

double g_regularizationCoeff = 1;
string g_stepRateType("");
double g_stepRateValue = 0.1;

void iterative_learning(vector<Board_Move_Dataset *> &dataset, int position_count, StandardFeatureExtractor &featureExtractor, const string &output_file, int maxIteration);
void at_once_learning(vector<Board_Move_Dataset *> &dataset, int position_count, StandardFeatureExtractor &featureExtractor, const string &output_file, int maxIteration);
void stochastic_learning(vector<Board_Move_Dataset *> &dataset, int position_count, StandardFeatureExtractor &featureExtractor, const string &output_file, int maxIteration);

typedef void (*LearningInterface)(vector<Board_Move_Dataset *> &dataset, int position_count, StandardFeatureExtractor &featureExtractor, const string &output_file, int maxIteration);

void usage() {
  cout << "usage: input Training SGF Files from std input" << endl
       << "Parameters ('S' means only for stochastic):" << endl
       << "-h              : help (show this message)" << endl
       << "-t iterative/atonce/stochastic : set update type (MUST)" << endl
       << "-i INPUT_FILE_LIST_FILE : set a input file (MUST)" << endl
       << "-p PATTERN_FILE : set a pattern file (MUST)" << endl
       << "-o OUTPUT_FILE  : set an output file (MUST)" << endl
       << "-c MAX_ITERATION : set max iteration (default: 10)" << endl
       << "-r SEED         : set a random seed" << endl
       << "-s              : do shuffle training data" << endl
       << "-f              : use 100% filter" << endl
       << "-l VALUE        : 'S' coefficient of reguralization term (default:1)" << endl
       << "-a (const/inverse/sqrt)VALUE : 'S' type of learning rate and it's value. if value is 0.1, learning rate will be 0.1 when iter 1 finished." << endl
       << "--nohandicap    : Select records which do not include handicap games"<< endl
       << "--lowerdan DAN  : Exclude records which include weaker players than the DAN " << endl;
}

void get_input_files(const string &filelist_file, vector<string> &buf) {
  ifstream ifs(filelist_file.c_str());
  if (!ifs) {
    usage();
    exit(0);
  }
  string line;
  buf.clear();
  while (!getline(ifs, line).eof()) {
    buf.push_back(line);
  }
  if (buf.empty()) {
    usage();
    exit(0);
  }
}

void read_pattern_list(const string &pat_file, vector<PATTERN_3x3_HASH_VALUE> &values, bool useFilter) {
  ifstream ifs(pat_file.c_str());

  if (!ifs) {
    cerr << "Failed to load pattern file: " << pat_file << endl;
    exit(-1);
  }
  
  string line;
  do {
    getline(ifs, line);
  } while (line.find("hash,appear,executed") == string::npos);

  while (getline(ifs, line)) {
    vector<string> split_data;
    Common::split(line, split_data, ",");
    if (split_data.size() < 3) continue;
    string hash_str(split_data[0]);
    string appear_str(split_data[1]);
    string executed_str(split_data[2]);

    // hashvalues with executed_str == 0 should be ignored???
    // TODO: appear_str != executed_str is OK?
    if (executed_str != "0") {
      if (!useFilter || appear_str != executed_str) {
        PATTERN_3x3_HASH_VALUE value = static_cast<PATTERN_3x3_HASH_VALUE>(strtoul(hash_str.c_str(), NULL, 16));
        //cout << hex << value << endl;
        values.push_back(value);
      }
    }
  }
}

int main(int argc, char **argv) {

#ifdef USE_MPI
  MPI_Init(&argc, &argv);

  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

#endif // USE_MPI


  map<string, LearningInterface> nameToLearnFunc;
  nameToLearnFunc["iterative"] = iterative_learning;
  nameToLearnFunc["atonce"] = at_once_learning;
  nameToLearnFunc["stochastic"] = stochastic_learning;

  // required arguments
  // 1. SGF files as training data
  // 2. Pattern list to be added as features
  // 3. Output file
  string input_file;
  string pattern_file;
  string output_file;
  bool doShuffle = false;
  string type = "";
  unsigned int randomSeed = time(NULL);
  int iteration = 10;
  bool useFilter = false;
  double lambda_of_regterm = 1;
  string learningRateType("");
  double learningRateValue = 0;
  bool nohandicap = false;
  int lowerdan = -1;

#define OPTION_COUNT 2
  option *options = new option[OPTION_COUNT+1];
  options[0].name = "nohandicap";
  options[0].has_arg = no_argument;
  options[0].flag = NULL;
  options[0].val = 1;

  options[1].name = "lowerdan";
  options[1].has_arg = required_argument;
  options[1].flag = NULL;
  options[1].val = 2;

  options[OPTION_COUNT].name = NULL;
  options[OPTION_COUNT].has_arg = 0;
  options[OPTION_COUNT].flag = NULL;
  options[OPTION_COUNT].val = 0;

  // parameters
  char c;
  int option_index = 0;
  while ((c = getopt_long(argc, argv, "fht:i:p:o:r:sc:l:a:", options, &option_index)) != -1) {
    switch (c) {
    case 1:
      // nohandicap
      if (option_index == 0) {
        nohandicap = true;
      }
      break;
    case 2:
      if (option_index == 1) {
        lowerdan = atoi(optarg);
        if (lowerdan <= 0) lowerdan = -1;
      }
      break;
    case 'h':
      usage();
      exit(0);

    case 't':
    {
      string opt = optarg;
      if (nameToLearnFunc.find(opt) != nameToLearnFunc.end()) {
        type = opt;
      }
      break;
    }
    case 'c':
      iteration = atoi(optarg);
      if (iteration == 0) iteration = 10;
      break;

    case 'l':
      lambda_of_regterm = atof(optarg);
      break;

    case 'i':
      input_file = optarg;
      break;
      
    case 'o':
      output_file = optarg;
      break;
      
    case 'p':
      pattern_file = optarg;
      break;

    case 'r':
      randomSeed = atoi(optarg);
      break;

    case 'f':
      useFilter = true;
      break;

    case 's':
      doShuffle = true;
      break;

    case 'a':
    {
      string str = optarg;
      if (str.find("const") != string::npos) {
        learningRateType = "const";
        learningRateValue = atof(str.substr(5).c_str());
      } else if (str.find("inverse") != string::npos) {
        learningRateType = "inverse";
        learningRateValue = atof(str.substr(7).c_str());
      } else if (str.find("sqrt") != string::npos) {
        learningRateType = "sqrt";
        learningRateValue = atof(str.substr(4).c_str());
      }
      break;
    }
    }
  }

  delete [] options;

  if (input_file.empty() || pattern_file.empty() || output_file.empty()) {
    usage();
    //MPI_Finalize();
    exit(0);
  }

  if (type.empty()) {
    usage();
    exit(0);
  }

  if (type == "stochastic" && (learningRateType == ""  || learningRateValue == 0)) {
    cerr << "You have to specify learning rate type and value" << endl;
    usage();
    exit(0);
  }
  g_stepRateType = learningRateType;
  g_stepRateValue = learningRateValue;

  // if (lowerdan > 0) {
  //   std::cerr << "lower dan is not supported... sorry..." << std::endl;
  //   exit(-1);
  // }

  {
    ofstream ofs(output_file.c_str());
    if (!ofs) {
      cerr << "failed to attach output file: " << output_file << endl;
      //MPI_Finalize();
      exit(-1);
    }
  }

  cerr << "input_file:" << input_file << endl << 
    " pattern_file: " << pattern_file << endl <<
    " output_file:" << output_file << endl <<
    " seed:" << randomSeed << endl <<
    " shuffle:" << doShuffle << endl <<
    " maxIteration:" << iteration << endl <<
    " type:" << type << endl <<
    " filter:" << useFilter << endl <<
    " regularization coeff:" << lambda_of_regterm << endl <<
    " learning rate type: " << learningRateType << " value:" << learningRateValue << endl <<
    " no handicap: " << nohandicap << endl <<
    " lower dan: " << lowerdan << endl;

  //return 0;

  g_regularizationCoeff = lambda_of_regterm;

  // read pattern list
  vector<PATTERN_3x3_HASH_VALUE> pattern_list;
  read_pattern_list(pattern_file, pattern_list, useFilter);

  // Register patterns
  StandardFeatureExtractor featureExtractor;
  cout << "pattern size: " << pattern_list.size() << endl;
  for (size_t i=0; i<pattern_list.size(); i++) {
    featureExtractor.registerPatternAsFeature(pattern_list[i]);
  }

  // output feature index to pattern hash
#ifdef USE_MPI
  if (my_rank == 0)
#endif 
  {
    ofstream ofs(output_file.c_str(), ios::app);
  
    cout << "feature index to pattern hash" << endl;
    ofs << "feature index to pattern hash" << endl;
    for (size_t i=0; i<featureExtractor.getFeatureDimension(); i++) {
      PATTERN_3x3_HASH_VALUE value = featureExtractor.getPatternOfFeatureIndex(i);
      if (value != PatternExtractor_3x3::INVALID_HASH_VALUE) {
        cout << hex << value << ",";
        ofs << hex << value << ",";
      } else {
        cout << "-1,";
        ofs << "-1,";
      }
    }
    cout << dec << endl;
    ofs << dec << endl;
  }

  // get file names
  vector<string> input_files;
  get_input_files(input_file, input_files);

  // shuffle
  if (doShuffle) {
    Common::StdlibRandom r(randomSeed);
    random_shuffle(input_files.begin(), input_files.end(), r);
  }


  size_t begin_index_of_files = 0;
  size_t size_of_files = input_files.size();

#ifdef USE_MPI
  // calculate my area
  int total_process;
  MPI_Comm_size(MPI_COMM_WORLD, &total_process);
  int div = size_of_files/total_process;
  int rest = size_of_files%total_process;
  if (my_rank < rest) {
    begin_index_of_files = (div+1) * my_rank;
    size_of_files = div+1;
  } else {
    begin_index_of_files = size_of_files - (total_process - my_rank) * div;
    size_of_files = div;
  }
  cout << "my_rank = " << my_rank << " start,end = " << begin_index_of_files << "," << begin_index_of_files + size_of_files -1 << endl;
#endif

  // read dataset
  vector<Board_Move_Dataset *> dataset;
  int position_count = 0;
  for (size_t i=0; i<size_of_files; i++) {
    size_t index = i + begin_index_of_files;
#ifdef USE_MPI
    cerr << "rank " << my_rank << " ";
#endif
    cerr << "reading SGF file: " << input_files[index] << endl;
    Board_Move_Dataset_record *newset = new Board_Move_Dataset_record;
    if (!newset->readAndAddRecordFromSGF(input_files[index])) {
      cerr << "failed to read " << input_files[index] << endl;
    }
    bool inserted = false;
    if (!newset->empty()) {
      shared_ptr<Common::Record> record = newset->getRecord(0);
      if (nohandicap == false || (nohandicap && record->getHandicapCount() == 0)) {
        if (lowerdan > 0) {
          const string &br = record->getBlackRank();
          const string &wr = record->getWhiteRank();

          if (!br.empty() && !wr.empty() &&
              br.find("k") == string::npos && wr.find("k") == string::npos &&
              br.find("d") != string::npos && wr.find("d") != string::npos) {
            int bdan = atoi(br.substr(0, br.find("d")).c_str());
            int wdan = atoi(wr.substr(0, wr.find("d")).c_str());

            if (bdan > lowerdan && wdan > lowerdan) {
              cerr << "bdan = " << bdan << " wdan = " << wdan << " accepted." << endl;
              dataset.push_back(newset);
              inserted = true;
            } else {
              cerr << "bdan = " << bdan << " wdan = " << wdan << " rejected." << endl;
            }
          } else {
            cerr << "no ranks are specified, or include 'kyu'" << endl;
          }
          // 
        } else {
          dataset.push_back(newset);
          inserted = true;
        }
      } else {
        cerr << "handicap exists" << endl;
      }
    } else {
      cerr << "empty" << endl;
    }

    if (!inserted) {
      delete newset;
    } else {
      position_count += newset->getRecord(0)->getMoveSequence().size();
    }

  }
  cerr << "valid positions are " << position_count << endl;
#ifdef USE_MPI
  cerr << "Read " << size_of_files << " files in rank " << my_rank << endl;
#endif

  // Launch learner
  nameToLearnFunc[type](dataset, position_count, featureExtractor, output_file, iteration);

  for (size_t i=0; i<dataset.size(); i++) {
    delete dataset[i];
  }

#ifdef USE_MPI
  MPI_Finalize();
#endif

  return 0;
}

void at_once_learning(vector<Board_Move_Dataset *> &dataset, int position_count, StandardFeatureExtractor &featureExtractor, const string &output_file, int maxIteration) {
  MM_Learning learner(featureExtractor);
  for (int i=0; i<maxIteration; i++) {
    //dataset.seekToBegin();
    learner.learn_at_once(dataset);
    
#ifdef USE_MPI
    const FeatureValues &appears = learner.getFeatureAppearCountsInLastTraining();
    const FeatureWeights &cijofejs = learner.getCijofEjsInLastTraining();

    // send and wait
    int *reducedAppearsArray = new int[appears.size()*2];
    MPI_Allreduce(reinterpret_cast<void *>(const_cast<int *>(appears.raw())),
                  reinterpret_cast<void *>(reducedAppearsArray),
                  appears.size(), MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    cerr << "finished all reduce 1" << endl;
    double *reducedCijofEjsArray = new double[cijofejs.size()*2];
    MPI_Allreduce(reinterpret_cast<void *>(const_cast<double *>(cijofejs.raw())),
                  reinterpret_cast<void *>(reducedCijofEjsArray),
                  cijofejs.size(), MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    cerr << "finished all reduce 2" << endl;

    FeatureValues reducedAppears(appears.size()); reducedAppears.setFromArray(reducedAppearsArray, appears.size());
    FeatureWeights reducedCijofEjs(cijofejs.size());
    for(int j=0; j<appears.size(); j++) reducedCijofEjs[j] = reducedCijofEjsArray[j];
    delete [] reducedAppearsArray;
    delete [] reducedCijofEjsArray;
    
    learner.setWeightsByAppearCountsAndCijofEjs(reducedAppears, reducedCijofEjs);

#endif // USE_MPI

    string weight_str(learner.getFeatureWeight().toString());
    cout << "---- iter " << i+1 << " result: " << weight_str << endl;

#ifdef USE_MPI
    if (my_rank == 0) {
#endif
    ofstream ofs(output_file.c_str(), ios::app);
    ofs << weight_str << endl;
#ifdef USE_MPI
    }
#endif
  }

}

void iterative_learning(vector<Board_Move_Dataset *> &dataset, int position_count, StandardFeatureExtractor &featureExtractor, const string &output_file, int maxIteration) {
  MM_Learning learner(featureExtractor);
  for (int i=0; i<maxIteration; i++) {
    //dataset.seekToBegin();
    for (size_t j=0; j<featureExtractor.getFeatureDimension(); j++) {
      learner.learn_one_dimension(dataset, j);
    
#ifdef USE_MPI
      const FeatureValues &appears = learner.getFeatureAppearCountsInLastTraining();
      const FeatureWeights &cijofejs = learner.getCijofEjsInLastTraining();
      
      // send and wait
      int reducedAppear;
      MPI_Allreduce(reinterpret_cast<void *>(const_cast<int *>(appears.raw())+j),
                    reinterpret_cast<void *>(&reducedAppear),
                    1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
      cerr << "finished all reduce 1" << endl;
      double reducedCijofEj;
      MPI_Allreduce(reinterpret_cast<void *>(const_cast<double *>(cijofejs.raw())+j),
                    reinterpret_cast<void *>(&reducedCijofEj),
                    1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
      cerr << "finished all reduce 2" << endl;

      learner.setWeightByAppearCountAndCijofEj(j, reducedAppear, reducedCijofEj);
      //learner.setWeightsByAppearCountsAndCijofEjs(reducedAppears, reducedCijofEjs);
#endif // USE_MPI

      cout << "dim " << j << " weight = " << learner.getFeatureWeight()[j] << endl;
    }

    string weight_str(learner.getFeatureWeight().toString());
    cout << "---- iter " << i+1 << " result: " << weight_str << endl;

#ifdef USE_MPI
    if (my_rank == 0) {
#endif
    ofstream ofs(output_file.c_str(), ios::app);
    ofs << weight_str << endl;
#ifdef USE_MPI
    }
#endif
  }

}

void stochastic_learning(vector<Board_Move_Dataset *> &dataset, int position_count, StandardFeatureExtractor &featureExtractor, const string &output_file, int maxIteration) {
#ifdef USE_MPI
  cerr << "Stochastic Learning cannot run with MPI" << endl;
  cerr << "Program will finish" << endl;
  return;
#endif // USE_MPI

  std::auto_ptr<LearningRateCalculator> lr_calc;

  if (g_stepRateType == "const") {
    lr_calc.reset(new LearningRateConstant(g_stepRateValue));
  } else if (g_stepRateType == "inverse") {
    lr_calc.reset(new LearningRateInverseOfIteration(g_stepRateValue/(1-g_stepRateValue)*position_count,
                                                     g_stepRateValue/(1-g_stepRateValue)*position_count));
    cerr << "t0/pos = " << g_stepRateValue/(1-g_stepRateValue) << endl;
  } else if (g_stepRateType == "sqrt") {
    double sqrt_pos = sqrt(position_count);
    double div_value = sqrt(1 - g_stepRateValue * g_stepRateValue);
    double t0 = g_stepRateValue * sqrt_pos / div_value;
    cerr << "t0/sqrt(pos) = " << g_stepRateValue / div_value << endl;
    
    lr_calc.reset(new LearningRateSqrtOfInverseOfIteration(t0, t0*t0));
  }

  //LearningRateSqrtOfInverseOfIteration lr_calc(sqrt(position_count/3.0), position_count/3.0); // rate will be 0.5 at end of iter0
  //LearningRateSqrtOfInverseOfIteration lr_calc(sqrt(position_count/99.0), position_count/99.0); // rate will be 0.1 at end of iter0
  //LearningRateInverseOfIteration lr_calc(position_count, position_count); // rate will be 0.5 at end of iter0
  
  //LearningRateInverseOfIteration lr_calc(position_count/9, position_count/9); // rate will be 0.1 at end of iter0

  //LearningRateConstant lr_calc(0.9);

  LearningRateConstant regCalc(g_regularizationCoeff);
  //LearningRateConstant regCalc(0.5);

  std::auto_ptr<ApprenticeshipLearning> learner;

  if (g_regularizationCoeff == 0) {
    learner.reset(new ApprenticeshipLearning(featureExtractor, *lr_calc));
  } else {
    learner.reset(new ApprenticeshipLearning(featureExtractor, *lr_calc, regCalc));
  }

  for (int i=0; i<maxIteration; i++) {
    learner->learn(dataset);

    stringstream ss;
    //ss << "(";
    //for (size_t j=0; j<learner.getFeatureWeight().size(); j++) {
    //  ss << exp(learner.getFeatureWeight()[j]) << ",";
    //}
    //ss << ")";

    string weight_str(learner->getFeatureWeight().toString());
    cout << "---- iter " << i+1 << " result: " << weight_str << endl;

    ofstream ofs(output_file.c_str(), ios::app);
    ofs << weight_str << endl;
  }
}
