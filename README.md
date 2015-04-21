ComputerGO_AI_MCTS
====

## Overview

The computer GO framework and the AI using MCTS (Monte Carlo Tree Search) 

## Description
Source code that the author uses for master thesis.

* A framework that runs computer GO between AI &amp; humans, or AI &amp; AI.
* Computer players using Monte-Carlo Tree Search (MCTS). These AI can battle with other computer players via GTP commands.
* Machine learning frameworks using pat GO records to enforce the computer players.

## Requirements
You can run main computer GO framework only on Linux OS because some API (especially pthread) depends on Linux API.

## Usage
* Build: run "make" in root directory (read Makefile for other make options).
* Run: read go_main_release_dynamic_runner.h or go_main_release_vs_gnugo_runner.h. These scripts run a computer player vs another computer player.

## Directories

Source code

* Root dir: Entiry points and some common functions/classes. 
* AI: Computer players 
* Go: The main framework of computer GO.
* Gtp: GTP command wrapper and a main entry point to run the framework as GTP mode. 
* ML: A machine learning framework. Source code in this directory are not incorporated with the main GO framework. You can run it as standalone.
* Record: Classes to read past records. These classes are mainly used by ML.
* utility: Utility classes, such as random, vector.
* tests: Tests run by Google Test.

Other files

* book: Books (in Japanese, "Jouseki") for Fuego ([http://fuego.sourceforge.net/])
* for_fuego_parameter_files: Parameter files to run fuego (author used these files to check performance author's AI v.s. Fuego)
* parameter_files: Parameter files to run author's AI.
* save_results: Result files of battle between AI, machine learning and so on... 

## License
Except for external libraries and resources, this software is released under the MIT License, see LICENSE.txt.

## Author
omochi (@omochi64): [https://github.com/omochi64]
