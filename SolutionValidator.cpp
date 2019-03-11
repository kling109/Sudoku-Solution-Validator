/*
Name: Trevor Kling
ID: 002270716
Email: kling109@mail.chapman.edu
Course: CPSC 380 Operating Systems
Last Date Modified: 11 March 2019
Project: Sudoku Solution Validator
*/

#include <string>
#include <vector>
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include "SolutionValidator.h"

using namespace std;

/*
Default constuctor for a solution validator.  Initializes vectors for later use, and sets
gameBoard to null to check if a file has been read later.
*/
SudokuSolutionValidator::SudokuSolutionValidator()
{
  this->solutionList = new vector<string>();
  this->errorList = new vector<vector<int>*>();
  this->gameBoard = NULL;
}

/*
Default deconstructor for a solution validator.  Removes the allocated memory for the vectors, and
deletes the gameboard if it has been allocated.  Note that iteration through the errorlist to remove
nested vectors is unnecessary, as c++ vectors are deleted when they fall out of scope.
*/
SudokuSolutionValidator::~SudokuSolutionValidator()
{
  delete this->solutionList;
  delete this->errorList;
  if (this->gameBoard != NULL)
  {
    for (int i = 0; i < sizeof(gameBoard)/sizeof(int); ++i)
    {
      delete this->gameBoard[i];
    }
    delete gameBoard;
  }
}

/*
A file reader method for the SudokuSolutionValidator.  Takes an argument for a file name, then returns nothing.
The file is used to initialize the gameBoard variable, which stores the value for each of the numbers in the
sudoku grid.  To convert the characters read in by the getlne function to numbers, an ASCII conversion is used.
Numbers start at 48 in ASCII code, so by converting the character to its code and subtracting 48 the number can be
obtained.
*/
void SudokuSolutionValidator::readFile(string fileName)
{
  if (fileName == "")
  {
    cout << "No file was given.";
  }
  else
  {
    ifstream boardReader (fileName);
    if (boardReader.is_open())
    {
      gameBoard = new int*[9];
      string line;
      int lineNum = 0;
      while (getline(boardReader, line))
      {
        gameBoard[lineNum] = new int[9];
        int index = 0;
        for (int i = 0; i < line.size(); ++i)
        {
          if (i%2 == 0)
          {
            gameBoard[lineNum][index] = (int)line[i] - 48;
            ++index;
          }
        }
        ++lineNum;
      }
    }
  }
}

/*
Checks if an element to be inserted into a list is unique.  If the element does not match any existing
list element, it is inserted and the function returns 0.  Otherwise, the element is not added and the
function returns 1.
*/
bool SudokuSolutionValidator::insertUniqueInt(int elem, vector<int>* list)
{
  bool unique = true;
  for (int k = 0; k < list->size(); ++k)
  {
    if (list->at(k) == elem)
    {
      unique = false;
      break;
    }
  }
  if (unique)
  {
    list->push_back(elem);
    return 0;
  }
  else
  {
    return 1;
  }
}

/*
Checks if a specific error record is unique before insertion.  If the identifier for the location of the
error on the grid is the same as an existing record, the error has already been recorded and is therefore
ignored.
*/
void SudokuSolutionValidator::insertUniqueRecord(vector<int>* record, vector<vector<int>* >* fullList)
{
  bool unique = true;
  for (int k = 0; k < fullList->size(); ++k)
  {
    if (fullList->at(k)->at(0) == record->at(0) && fullList->at(k)->at(1) == record->at(1))
    {
      unique = false;
      break;
    }
  }
  if (unique)
  {
    fullList->push_back(record);
  }
}

/*
Iterates over all rows of the grid, checking them for errors.
*/
void SudokuSolutionValidator::checkRows()
{
  for (int i = 0; i < 9; ++i)
  {
    findRowError(i);
  }
}

/*
Checks a specific row for errors.  If a duplicate element is found in the row, it indicates that
all 9 elements are not present in that row.  The program then pushes a record of the duplicate to the
error list, informing the program of where an error has occurred in the grid.
*/
void SudokuSolutionValidator::findRowError(int i)
{
  int* row = gameBoard[i];
  vector<int>* numsFound = new vector<int>();
  for (int j = 0; j < 9; ++j)
  {
    if (insertUniqueInt(row[j], numsFound) == 1)
    {
      vector<int>* error = new vector<int>();
      error->push_back(i);
      error->push_back(j);
      error->push_back(row[j]);
      insertUniqueRecord(error, errorList);
    }
  }
}
