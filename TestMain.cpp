/*
Name: Trevor Kling
ID: 002270716
Email: kling109@mail.chapman.edu
Course: CPSC 380 Operating Systems
Last Date Modified: 11 March 2019
Project: Sudoku Solution Validator
*/

#include "SolutionValidator.h"

using namespace std;

int main()
{
  SudokuSolutionValidator* tester = new SudokuSolutionValidator();
  tester->readFile("test1.txt");
  tester->fixBoard();
  return 0;
}
