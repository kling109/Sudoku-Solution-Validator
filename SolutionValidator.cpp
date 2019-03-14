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

typedef void* (*THREADFUNCPOINTER)(void *);

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
      bool noMatch = true;
      for (int j = 0; j < (int)((fullList->at(k)->size()-3)/2); ++j)
      {
        if (fullList->at(k)->at(2*j) == record->at(2) && fullList->at(k)->at(2*j+1) == record->at(3))
        {
          noMatch = false;
        }
      }
      if (noMatch)
      {
        int item = fullList->at(k)->back();
        fullList->at(k)->pop_back();
        fullList->at(k)->push_back(record->at(2));
        fullList->at(k)->push_back(record->at(3));
        fullList->at(k)->push_back(item);
      }
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
void* SudokuSolutionValidator::checkRows(void* ph)
{
  for (int i = 0; i < 9; ++i)
  {
    findRowError(i);
  }
}

/*
Checks a specific row for errors.  If a duplicate element is found in the row, it indicates that
all 9 elements are not present in that row.  The program then pushes a record of the duplicate to the
error list, informing the program of where an error has occurred in the grid.  A mutual exclusion lock
ensures that, when a thread checks the list, all elements in the list are accounted for and one is not
in the process of being added.
*/
void SudokuSolutionValidator::findRowError(int i)
{
  if (i > 8)
  {
    cout << "Improper bounds received." << endl;
    return;
  }
  for (int j = 0; j < 9; ++j)
  {
    for (int k = j+1; k < 9; ++k)
    {
      if (gameBoard[i][j] == gameBoard[i][k])
      {
        vector<int>* error = new vector<int>();
        error->push_back(i);
        error->push_back(j);
        error->push_back(i);
        error->push_back(k);
        error->push_back(gameBoard[i][j]);
        pthread_mutex_lock(&this->lock);
        insertUniqueRecord(error, errorList);
        pthread_mutex_unlock(&this->lock);
      }
    }
  }
}

/*
Iterates over all columnss of the grid, checking them for errors.
*/
void* SudokuSolutionValidator::checkColumns(void* ph)
{
  for (int j = 0; j < 9; ++j)
  {
    findColumnError(j);
  }
}

/*
Checks a specific column for errors.  If a duplicate element is found in the column, it indicates that
all 9 elements are not present in that column.  The program then pushes a record of the duplicate to the
error list, informing the program of where an error has occurred in the grid.  A mutual exclusion lock
ensures that, when a thread checks the list, all elements in the list are accounted for and one is not
in the process of being added.
*/
void SudokuSolutionValidator::findColumnError(int j)
{
  if (j > 8)
  {
    cout << "Improper bounds received." << endl;
    return;
  }
  for (int i = 0; i < 9; ++i)
  {
    for (int k = i+1; k < 9; ++k)
    {
      if (gameBoard[i][j] == gameBoard[k][j])
      {
        vector<int>* error = new vector<int>();
        error->push_back(i);
        error->push_back(j);
        error->push_back(k);
        error->push_back(j);
        error->push_back(gameBoard[i][j]);
        pthread_mutex_lock(&this->lock);
        insertUniqueRecord(error, errorList);
        pthread_mutex_unlock(&this->lock);
      }
    }
  }
}

/*
Iterates over all blocks in the grid, checking them for errors.
*/
void* SudokuSolutionValidator::checkBlocks(void* ph)
{
  for (int i = 0; i < 3; ++i)
  {
    for (int j = 0; j < 3; ++j)
    {
      int xcoord = 3*i;
      int ycoord = 3*j;
      findBlockError(xcoord, ycoord);
    }
  }
}

/*
Checks a specified 3x3 area of the grid for errors.  Each area is selected by inputting the
x and y coordinate of the top left element.  The function then checks if every element in a
3x3 range to the right and down from the given coordinates is unique.  If not, then the program
adds an error to the error list.  A mutual exclusion lock ensures that, when a thread checks the
list, all elements in the list are accounted for and one is not in the process of being added.
*/
void SudokuSolutionValidator::findBlockError(int x, int y)
{
  if (x+2 > 8 || y+2 > 8)
  {
    cout << "Improper bounds received." << endl;
    return;
  }
  for (int i = 0; i < 3; ++i)
  {
    for (int j = 0; j < 3; ++j)
    {
      for (int k = i; k < 3; ++k)
      {
        int startLocation = 0;
        if (k == i)
        {
          startLocation = j+1;
        }
        for (int l = startLocation; l < 3; ++l)
        {
          if (gameBoard[x+i][y+j] == gameBoard[x+k][y+l])
          {
            vector<int>* error = new vector<int>();
            error->push_back(x+i);
            error->push_back(y+j);
            error->push_back(x+k);
            error->push_back(y+l);
            error->push_back(gameBoard[x+i][y+j]);
            pthread_mutex_lock(&this->lock);
            insertUniqueRecord(error, errorList);
            pthread_mutex_unlock(&this->lock);
          }
        }
      }
    }
  }
}

void SudokuSolutionValidator::fixBoard()
{
  if (gameBoard == NULL)
  {
    cout << "No sudoku grid has been supplied." << endl;
    return;
  }
  pthread_create(&(threads[0]), NULL, (THREADFUNCPOINTER) &SudokuSolutionValidator::checkRows, this);
  pthread_create(&(threads[1]), NULL, (THREADFUNCPOINTER) &SudokuSolutionValidator::checkColumns, this);
  pthread_create(&(threads[2]), NULL, (THREADFUNCPOINTER) &SudokuSolutionValidator::checkBlocks, this);
  pthread_join(threads[0], NULL);
  pthread_join(threads[1], NULL);
  pthread_join(threads[2], NULL);
  if (errorList->size() == 0)
  {
    cout << "No errors found in board" << endl;
  }
  else
  {
    for (int i = 0; i < errorList->size(); ++i)
    {
      for (int j = 0; j < (int)((errorList->at(i)->size()-1)/2); ++j)
      {
        cout << "[" << errorList->at(i)->at(2*j)+1 << "," << errorList->at(i)->at(2*j + 1)+1 << "], ";
      }
      cout << errorList->at(i)->at(errorList->at(i)->size()-1) << endl;
    }
  }
}
