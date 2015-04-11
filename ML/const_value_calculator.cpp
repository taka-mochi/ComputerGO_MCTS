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
#include "ML/LearningRateExponential.h"

using namespace std;
using namespace Common;
using namespace ML;

void usage() {
  cout << "usage: input Training SGF Files from std input" << endl
       << "Parameters:" << endl
       << "-h             : help (show this message)" << endl
       << "-t MM/stochastic : set feature weight file type (MUST)" << endl
       << "-i INPUT_FILE_LIST_FILE : set a input file (MUST)" << endl
       << "-w WEIGHT_FILE : set a weight file (MUST)" << endl
       << "-o OUTPUT_FILE : set an output file (MUST)" << endl;
}

void read_pattern_list_and_weight_list_from_weight_file(const string &weight_file, vector<PATTERN_3x3_HASH_VALUE> &values, vector<shared_ptr<FeatureWeights> > &weights, bool isWeightFileForMM) {
  ifstream ifs(weight_file.c_str());

  if (!ifs) {
    cerr << "Failed to load pattern file: " << weight_file << endl;
    exit(-1);
  }
  
  string line;
  do {
    getline(ifs, line);
  } while (line.find("feature index to pattern hash") == string::npos);

  getline(ifs, line);

  vector<string> split_data;
  Common::split(line, split_data, ",");

  size_t featureSize = split_data.size();
  
  for (size_t i=0; i<split_data.size(); i++) {
    if (split_data[i] != "-1") {
      PATTERN_3x3_HASH_VALUE value = static_cast<PATTERN_3x3_HASH_VALUE>(strtol(split_data[i].c_str(), NULL, 16));
      values.push_back(value);
    }
  }
  cerr << "pattern count: " << values.size() << endl;

  // read weights
  while (getline(ifs,line)) {
    if (line.size() == 0) continue;
    if (line[0] == '(') line = line.substr(1);
    if (line[line.size()-1] == ')') line = line.substr(0, line.size()-1);
    Common::split(line, split_data, ",");

    // stochastic の resultは、 ",)"みたいなのがある。あと、古いweight ファイルは stochastic でも exp とってる？
    assert(split_data.size() == featureSize);
    shared_ptr<FeatureWeights> weight(new FeatureWeights(featureSize));

    for (size_t i=0; i<split_data.size(); i++) {
      string value_str = split_data[i];
      if (value_str.size() == 0) {
        cerr << "fatal error in reading weights" << endl;
        exit(-1);
      }
      if (value_str[0] == '(') value_str = value_str.substr(1);
      if (value_str[value_str.size()-1] == ')') value_str = value_str.substr(0, value_str.size()-1);
      if (isWeightFileForMM) {
        (*weight)[i] = log(atof(value_str.c_str()));
      } else {
        (*weight)[i] = atof(value_str.c_str());
      }
    }
    weights.push_back(weight);
  }
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


int main(int argc, char **argv) {
  // required arguments
  // 1. SGF files as training data
  // 2. Pattern list to be added as features
  // 3. Output file
  string input_file;
  string weight_file;
  string output_file;
  string type = "";

  // parameters
  char c;
  while ((c = getopt(argc, argv, "hl:t:i:w:o:")) != -1) {
    switch (c) {
    case 'h':
      usage();
      exit(0);

    case 't':
    {
      string opt = optarg;
      if (opt == "stochastic" || opt == "MM") {
        type = opt;
      }
      break;
    }

    case 'i':
      input_file = optarg;
      break;
      
    case 'o':
      output_file = optarg;
      break;
      
    case 'w':
      weight_file = optarg;
      break;
    }
  }

  if (input_file.empty() || weight_file.empty() || output_file.empty()) {
    usage();
    //MPI_Finalize();
    exit(0);
  }

  if (type.empty()) {
    usage();
    exit(0);
  }

  {
    ofstream ofs(output_file.c_str());
    if (!ofs) {
      cerr << "failed to attach output file: " << output_file << endl;
      //MPI_Finalize();
      exit(-1);
    }
  }

  cerr << "input_file:" << input_file << endl << 
    " weight_file: " << weight_file << endl <<
    " output_file:" << output_file << endl <<
    " type:" << type << endl;

  // read pattern list
  vector<PATTERN_3x3_HASH_VALUE> pattern_list;
  vector<shared_ptr<FeatureWeights> > featureWeights;
  read_pattern_list_and_weight_list_from_weight_file(weight_file, pattern_list, featureWeights, type == "MM");

  // Register patterns
  StandardFeatureExtractor featureExtractor;
  cout << "pattern size: " << pattern_list.size() << endl;
  for (size_t i=0; i<pattern_list.size(); i++) {
    cerr << hex << pattern_list[i] << ",";
    featureExtractor.registerPatternAsFeature(pattern_list[i]);
  }
  cerr << dec << endl;

  // get file names
  vector<string> input_files;
  get_input_files(input_file, input_files);

  size_t size_of_files = input_files.size();
  // read dataset
  vector<Board_Move_Dataset *> dataset;
  int position_count = 0;
  for (size_t i=0; i<size_of_files; i++) {
    cerr << "reading SGF file: " << input_files[i] << endl;
    Board_Move_Dataset_record *newset = new Board_Move_Dataset_record;
    if (!newset->readAndAddRecordFromSGF(input_files[i])) {
      cerr << "failed to read " << input_files[i] << endl;
    }
    dataset.push_back(newset);
  }

  // calc
  LearningRateExponential lr(1);
  ApprenticeshipLearning learner(featureExtractor, lr);
  for (size_t i=0; i<featureWeights.size(); i++) {
    //cerr << featureWeights[i]->toString() << endl;
    learner.init(featureWeights[i].get());
    double value = learner.calculateLoglikelihoodValue(dataset);
    cerr << "LL value for weight " << i+1 << ": " << value << endl;
    ofstream ofs(output_file.c_str(), ios::app);
    if (ofs){
      ofs << value << endl;
    }
  }

  for (size_t i=0; i<dataset.size(); i++) {
    delete dataset[i];
  }

  return 0;
}
