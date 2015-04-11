# ComputerGO_AI_MCTS
The computer GO framework and the AI using MCTS (Monte Carlo Tree Search) 

- Description
Source code that the author uses for master thesis.

* A framework that runs computer GO between AI & humans, or AI & AI.
* Computer players using Monte-Carlo Tree Search (MCTS).
** These AI can battle with other computer players via GTP commands.
* Machine learning frameworks to enforce the computer players.
** Using the past GO records.

- Notice
You can run main computer GO framework only on Linux OS because some API (especially pthread) depends on Linux API.

- Directories

-- Source code
* AI
** Computer players 
* Go
** The main framework of computer GO.
* Gtp
** GTP command wrapper and a main entry point to run the framework as GTP mode. 
* ML
** A machine learning framework. Source code in this directory are not incorporated with the main GO framework. You can run it as standalone.
* Record
** Classes to read past records. These classes are mainly used by ML.
* utility
** 
* tests

 