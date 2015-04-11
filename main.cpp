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

#include "ExecuteParameter.h"
#include "AI/Parameters.h"

#include "Go/GoBook.h"

using namespace std;
using namespace Common;
using namespace Go;

void printHelp() {
  cout << "./go_main [PARAMETERS (below)]" << endl
       << "-h: Help" << endl
       << "-o: Ouput File" << endl
       << "-r: Random Seed" << endl
       << "-b: board size" << endl
       << "-g: Game Count (if -g 1 is passed, the game will be done 2 times (both black and white))" << endl
    //<< "-m: playout count for mc of each hand" << endl
    // << "-u: playout count for UCT" << endl
       << "-l: move count threshold for draw" << endl
       << "--gtpmode: Run program as Gtp Talk Mode" << endl
       << "--noresign: Force computers not to resign" << endl
       << "--gtpparamfile: Set a parameter file for GTP Mode" << endl
       << "--blackparamfile: Set a parameter file for black player" << endl
       << "--whiteparamfile: Set a parameter file for white player" << endl
       << "--dontchangeturn: do not change BLACK and WHITE" << endl
       << endl;
}

void setDefaultParameter() {
  stringstream ss;
  //ss << "result" << time(NULL) << ".dat";
  ExecuteParameter::resultFile = "";

  ExecuteParameter::gameCountForEvaluation = 1000;
  ExecuteParameter::playoutCountForMC = 10000/81;
  ExecuteParameter::playoutCountForUCT = 10000;
  ExecuteParameter::boardSize = 9;
  ExecuteParameter::drawLimitCount = 2000;
  ExecuteParameter::gtpParamFile = "";
  ExecuteParameter::blackParamFile = "";
  ExecuteParameter::whiteParamFile = "";
  ExecuteParameter::dontChangeTurn = false;

  ExecuteParameter::randomSeed = time(NULL);
}

void parseArgs(int argc, char **argv) {
  // TODO:
  //  Player Selection
  //  Parameter setting (from file?)

  int c;

  setDefaultParameter();

#define OPTION_COUNT 6

  option *options = new option[OPTION_COUNT+1];
  options[0].name = "gtpmode";
  options[0].has_arg = no_argument;
  options[0].flag = NULL;
  options[0].val = 1;

  options[1].name = "noresign";
  options[1].has_arg = no_argument;
  options[1].flag = NULL;
  options[1].val = 2;

  options[2].name = "gtpparamfile";
  options[2].has_arg = required_argument;
  options[2].flag = NULL;
  options[2].val = 3;

  options[3].name = "blackparamfile";
  options[3].has_arg = required_argument;
  options[3].flag = NULL;
  options[3].val = 4;

  options[4].name = "whiteparamfile";
  options[4].has_arg = required_argument;
  options[4].flag = NULL;
  options[4].val = 5;

  options[5].name = "dontchangeturn";
  options[5].has_arg = no_argument;
  options[5].flag = NULL;
  options[5].val = 6;

  options[OPTION_COUNT].name = NULL;
  options[OPTION_COUNT].has_arg = 0;
  options[OPTION_COUNT].flag = NULL;
  options[OPTION_COUNT].val = 0;

  // -h: Help
  // -l: move count threshold for draw
  // -o: Ouput File
  // -r: Random Seed
  // -b: board size
  // -g: Game Count (if -g 1 is passed, the game will be done 2 times (both black and white))
  // //-m: playout count for mc of each hand
  // //-p: playout count for plain UCT
  int option_index = 0;
  while ((c = getopt_long(argc, argv, "hl:b:o:r:g:", options, &option_index)) != -1) {
    switch(c) {
    case 1:
      if (option_index == 0) {
        // gtpmode
        ExecuteParameter::isGtpMode = true;
      }
      break;
    case 2:
      if (option_index == 1) {
        // noresign
        AI::Parameters::getInstanceOfColor(BLACK).setResignDisabled();
        AI::Parameters::getInstanceOfColor(WHITE).setResignDisabled();
      }
      break;
    case 3:
      if (option_index == 2) {
        // gtp paramfile
        ExecuteParameter::gtpParamFile = optarg;
      }
      break;
    case 4:
      if (option_index == 3) {
        ExecuteParameter::blackParamFile = optarg;
      }
      break;
    case 5:
      if (option_index == 4) {
        ExecuteParameter::whiteParamFile = optarg;
      }
      break;
    case 6:
      if (option_index == 5) {
        ExecuteParameter::dontChangeTurn = true;
      }
      break;
    case 'h':
      printHelp();
      exit(0);
      break;
    case 'l':
      ExecuteParameter::drawLimitCount = atoi(optarg);
      break;
    case 'o':
      ExecuteParameter::resultFile = optarg;
      break;
    case 'r':
      ExecuteParameter::randomSeed = atoi(optarg);
      break;
    case 'b':
      ExecuteParameter::boardSize = atoi(optarg);
      break;
    case 'g':
      ExecuteParameter::gameCountForEvaluation = atoi(optarg);
      break;
      //case 'm':
      //ExecuteParameter::playoutCountForMC = atoi(optarg);
      //break;
      //case 'p':
      //ExecuteParameter::playoutCountForUCT = atoi(optarg);
      //break;
    default:
      cerr << "Unknown Command '" << static_cast<char>(c) << "'" << endl;
      exit(-1);
    }
  }

  delete [] options;

  // temporal arguments
  // ./evaluator [resultFile] [GameCount=1000] [PlayoutMC=10000/81] [PlayoutUCT=10000]
/*
  if (argc >= 2) {
  ExecuteParameter::resultFile = argv[1];
  } else {
  stringstream ss;
  ss << "result" << time(NULL) << ".dat";
  ExecuteParameter::resultFile = ss.str();
  std::ofstream ofs(ExecuteParameter::resultFile);
  }
  if (argc >= 3) {
  ExecuteParameter::gameCountForEvaluation = atoi(argv[2]);
  }
  if (argc >= 4) {
  ExecuteParameter::playoutCountForMC = atoi(argv[3]);
  }
  if (argc >= 5) {
  ExecuteParameter::playoutCountForPlainUCT = atoi(argv[4]);
  }
*/
}


void showInformation(std::ostream &ou) {
  ou << "--- General Information ---" << endl;
  ou << "Random Seed: " << ExecuteParameter::randomSeed << endl;
  ou << "Change BLACK&WHITE: " << (ExecuteParameter::dontChangeTurn ? "no" : "yes") << endl;
  if (AI::Parameters::getInstanceOfColor(BLACK).isResignAllowed()) {
    ou << "Black Resign: allowed" << endl;
  } else {
    ou << "White Resign: disallowed" << endl;
  }
  if (AI::Parameters::getInstanceOfColor(WHITE).isResignAllowed()) {
    ou << "Black Resign: allowed" << endl;
  } else {
    ou << "White Resign: disallowed" << endl;
  }

  Color c[] = {BLACK, WHITE};
  for (int i=0; i<2; i++) {
    ou << "--- UCT Search Information of " << (i==0 ? "Black" : "White") << " ---" << endl;
    AI::Parameters &params = AI::Parameters::getInstanceOfColor(c[i]);
    ou << "C = " << params.getUCT_C() << endl;
    ou << "K = " << params.getRave_K() << endl;
    ou << "M = " << params.getRave_M() << endl;
    ou << "Expand Threshold = " << params.getThreshold_Expand() << endl;
    ou << "Playout Limit = " << params.getPlayoutLimit() << endl;
    ou << "Standard Features Weight File = " << params.getStandardFeatureWeightFile() << endl;
    ou << "Opening Book = " << (params.getUseOpeningBook() ? params.getOpeningBookFilename() : "none") << endl;
    ou << "Player Kind = " << AI::Parameters::PLAYER_KIND::toString(params.getPlayerKind()) << endl;
    ou << "Dynamic Alpha = " << params.getDynamicUpdatePolicyAlpha() << endl;
    ou << "Dynamic Lambda = " << params.getDynamicUpdatePolicyLambda() << endl;

    ou << "--- End of Information ---" << endl;
    ou << endl;
  }
}

int main(int argc, char **argv)
{
  parseArgs(argc, argv);

  //srand(ExecuteParameter::randomSeed);
  MTRandom::getInstance().init(ExecuteParameter::randomSeed);

  Go::Block::initAllocator();

  if (ExecuteParameter::isGtpMode) {
    // gtp mode
    cerr << "RandomSeed," << ExecuteParameter::randomSeed << endl;
    Gtp::gtp_main();
    return 0;
  }

  if (ExecuteParameter::blackParamFile == "" ||
      !AI::Parameters::getInstanceOfColor(BLACK).readParametersFromFile(ExecuteParameter::blackParamFile)) {
    cerr << "Failed to read a parameter file for black!! Program will finish." << endl;
    printHelp();
    exit(-1);
  }
  if (ExecuteParameter::whiteParamFile == "" ||
      !AI::Parameters::getInstanceOfColor(WHITE).readParametersFromFile(ExecuteParameter::whiteParamFile)) {
    cerr << "Failed to read a parameter file for white!! Program will finish." << endl;
    printHelp();
    exit(-1);
  }

  cout << "RandomSeed," << ExecuteParameter::randomSeed << endl;
  showInformation(cerr);

  // TODO: implement ExecuteParameter::boardSize

  int turn = Common::BLACK;  
  bool before_pass = false;
  int count = 0;

  for (int gameCount=0; gameCount<ExecuteParameter::gameCountForEvaluation; gameCount++) {
    for (int player_init = 0; player_init <= 1; player_init++) {
      if (player_init == 1 && ExecuteParameter::dontChangeTurn) {
        break;
      }

      Board board(ExecuteParameter::boardSize);

      AI::Parameters *parameters[2] = {
        &AI::Parameters::getInstanceOfColor(BLACK),
        &AI::Parameters::getInstanceOfColor(WHITE)
      };
      shared_ptr<AI::PlayerBase> players[2] = {shared_ptr<AI::PlayerBase>(), shared_ptr<AI::PlayerBase>()};

      shared_ptr<Go::GoBook> books[2] = {shared_ptr<Go::GoBook>(), shared_ptr<Go::GoBook>()};

      for (int i=0; i<2; i++) {
        AI::Parameters &params = *parameters[i];
        //params.setResignDisabled();

        players[i] = params.createPlayerAccordingToKind(&board, 1);
        if (players[i] == NULL) {
          cerr << "Unsupported Player Kind for " << (i==0 ? "black" : "white") << endl;
          printHelp();
          exit(-1);
        }

        if (params.getUseOpeningBook()) {
          shared_ptr<Go::GoBook> book(new Go::GoBook());
          if (book->readFromFile(params.getOpeningBookFilename())) {
            books[i] = book;
          }
        }
      }

      board.clear();
      turn = Common::BLACK;
      before_pass = false;
      int player_index = player_init;

      count = 0;

      bool isResign = false;
      int resignLoserIndex = -1;

      //cerr << "UCTNode capacity = " << AI::UCTNode::getNewNodeCapacity() << endl;

      while (true) {
        Common::Point pos = POINT_NULL;

        // check board
        if (books[player_index]) {
          vector<Point> moves = books[player_index]->getMovesInBook(board);
          if (moves.size()>0) {
            pos = moves[MTRandom::getInstance()()%moves.size()];
            cerr << "Use the opening book"<< endl;
          }
        }

        if (pos == POINT_NULL) {
          pos = players[player_index]->selectBestMove(turn);
        }

        if (players[player_index]->shouldResign()) {
          isResign = true;
          resignLoserIndex = player_index;
          break;
        }

        if (pos == POINT_NULL) {
          // NO GAME
          cerr << "This game is NO GAME" << endl;
        } else {
          int err = board.put(pos, turn);
          if (err != 0) {
            cerr << "!!Invalid Move!!" << endl;
            exit(-1);
          }
        }
        if (pos == POINT_NULL || count > ExecuteParameter::drawLimitCount) {
          cerr << "More than " << count << " plays!! This game is draw and program will exit." << endl;
          cout << "Draw" << endl;
          if (!ExecuteParameter::resultFile.empty()) {
            std::ofstream ofs(ExecuteParameter::resultFile.c_str(), std::ios::out | std::ios::app);
            ofs << "Draw" << endl;
          }
          exit(-1);
        }
        cerr << "Move Count = " << count << endl;
        board.printToErr();
        
        if (pos == Common::PASS &&
            before_pass) {
          break;
        }
        
        turn = Board::flipColor(turn);
        player_index = 1-player_index;

        count++;
        before_pass = (pos == Common::PASS);

      }


      std::stringstream resultCoutStr;
      //std::ofstream ofs(ExecuteParameter::resultFile, std::ios::out | std::ios::app);

      double b, w;
      board.countScoreDetail(b, w);
      std::string black_name = players[player_init]->getAIName();
      std::string white_name = players[1-player_init]->getAIName();
      double win = board.countScore(Common::BLACK);

      if (!isResign) {
        if (win > 0) {
          //ofs << black_name << ",Black," << b << "," << white_name << ",White," << w << endl;
          resultCoutStr << black_name << ",Black," << b << "," << white_name << ",White," << w << endl;
          cerr << "Black wins: ";
        } else {
          //ofs << white_name << ",White," << w << "," << black_name << ",Black," << b << endl;
          resultCoutStr << white_name << ",White," << w << "," << black_name << ",Black," << b << endl;
          cerr << "White wins: ";
        }
      } else {
        resultCoutStr << players[1-resignLoserIndex]->getAIName() << "," <<
          (1-resignLoserIndex == player_init ? "Black" : "White") << ",resign," <<
          players[resignLoserIndex]->getAIName() << "," <<
          (resignLoserIndex == player_init ? "Black" : "White") << "," << endl;
        cerr << (1-resignLoserIndex == player_init ? "Black" : "White") << " wins: ";
      }

      cerr << "black_score(" << b << "), white_score(" << w << ")" << endl;
      int b_cap = board.getCapturedCount(Common::BLACK);
      int w_cap = board.getCapturedCount(Common::WHITE);
      
      cerr << "black_captured(" << b_cap << "), white_captured(" << w_cap << ")" << endl;

      cout << resultCoutStr.str();
      if (!ExecuteParameter::resultFile.empty()) {
        std::ofstream ofs(ExecuteParameter::resultFile.c_str(), std::ios::out | std::ios::app);
        ofs << resultCoutStr.str();
      }
    }
  }

  return 0;
}
