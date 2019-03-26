/*
Name: Trevor Kling
ID: 002270716
Email: kling109@mail.chapman.edu
Course: CPSC 380 Operating Systems
Last Date Modified: 26 March 2019
Project: Sudoku Solution Validator
*/

#include <iostream>
#include "SolutionValidator.h"

using namespace std;

/*
Main method of the program.  Primarily instantiates the solution testing method and passes arguments to the program.
*/
int main(int argc, char* argv[])
{
  SudokuSolutionValidator* tester = new SudokuSolutionValidator();
  if (argc > 1)
  {
      tester->readFile(argv[1]);
      tester->fixBoard();
  }
  else
  {
    cout << "Input a file for the validator by running the command './SolutionValidator [FileName]'" << endl;
  }
  return 0;
}
