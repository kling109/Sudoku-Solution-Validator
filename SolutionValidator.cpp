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

void SudokuSolutionValidator::checkBoard()
{
  pthread_create(&(threads[0]), NULL, (THREADFUNCPOINTER) &SudokuSolutionValidator::checkRows, this);
  pthread_create(&(threads[1]), NULL, (THREADFUNCPOINTER) &SudokuSolutionValidator::checkColumns, this);
  pthread_create(&(threads[2]), NULL, (THREADFUNCPOINTER) &SudokuSolutionValidator::checkBlocks, this);
  pthread_join(threads[0], NULL);
  pthread_join(threads[1], NULL);
  pthread_join(threads[2], NULL);
}

vector<int>* SudokuSolutionValidator::identifyReplacementPair(vector<int>* error)
{
  int rowChoice = -1;
  int colChoice = -1;
  for (int i = 0; i < error->size()-1; i = i+2)
  {
    for (int j = 0; j < error->size()-1; j = j+2)
    {
      if (rowChoice == -1 && i != j && error->at(i) == error->at(j))
      {
        rowChoice = error->at(i);
      }
      if (colChoice == -1 && i != j && error->at(i+1) == error->at(j+1))
      {
        colChoice = error->at(i+1);
      }
    }
    if (rowChoice != -1 && colChoice != -1)
    {
      break;
    }
  }
  vector<int>* pair = new vector<int>();
  pair->push_back(rowChoice);
  pair->push_back(colChoice);
  return pair;
}

vector<int>* SudokuSolutionValidator::identifyBlock(vector<int>* location)
{
  vector<int>* block = new vector<int>();
  switch(location->at(0))
  {
    case 0:
    case 1:
    case 2: block->push_back(0);
            break;
    case 3:
    case 4:
    case 5: block->push_back(3);
            break;
    case 6:
    case 7:
    case 8: block->push_back(6);
            break;
    default: break;
  }
  switch(location->at(1))
  {
    case 0:
    case 1:
    case 2: block->push_back(0);
            break;
    case 3:
    case 4:
    case 5: block->push_back(3);
            break;
    case 6:
    case 7:
    case 8: block->push_back(6);
            break;
    default: break;
  }
  return block;
}

void SudokuSolutionValidator::makeReplacement(vector<int>* location)
{
  vector<int>* rowNums = new vector<int>();
  vector<int>* colNums = new vector<int>();
  vector<int>* blockNums = new vector<int>();
  for (int i = 1; i < 10; ++i)
  {
    rowNums->push_back(i);
    colNums->push_back(i);
    blockNums->push_back(i);
  }
  vector<int>* block = identifyBlock(location);
  for (int i = 0; i < 9; ++i)
  {
    for (int j = 0; j < rowNums->size(); ++j)
    {
      if (gameBoard[location->at(0)][i] == rowNums->at(j))
      {
        rowNums->erase(rowNums->begin() + j);
        break;
      }
    }
    for (int j = 0; j < colNums->size(); ++j)
    {
      if (gameBoard[i][location->at(1)] == colNums->at(j))
      {
        colNums->erase(colNums->begin() + j);
        break;
      }
    }
    int x = block->at(0) + (i%3);
    int y = block->at(1) + (int)(i/3);
    for (int j = 0; j < blockNums->size(); ++j)
    {
      if (gameBoard[x][y] == blockNums->at(j))
      {
        blockNums->erase(blockNums->begin() + j);
        break;
      }
    }
  }
  int num = -1;
  if (rowNums->size() != 0)
  {
    num = rowNums->at(0);
  }
  else if (colNums->size() != 0)
  {
    num = colNums->at(0);
  }
  else if (blockNums->size() != 0)
  {
    num = blockNums->at(0);
  }
  if (num != -1)
  {
    cout << "Change the contents of cell [" << location->at(0)+1 << ", " << location->at(1)+1 << "] to " << num << endl;
    gameBoard[location->at(0)][location->at(1)] = num;
  }
}

/*
Can find which cell to change; still needs a helper method to find which
element to replace with.
*/
void SudokuSolutionValidator::toFix(vector<vector<int>* >* errors)
{
  if (errors->size() == 0)
  {
    cout << "There are no errors remaining in the board." << endl;
    return;
  }
  vector<int>* replacement = identifyReplacementPair(errors->at(0));
  makeReplacement(replacement);
  errors->erase(errors->begin());
  checkBoard();
  toFix(errorList);
}

void SudokuSolutionValidator::fixBoard()
{
  if (gameBoard == NULL)
  {
    cout << "No sudoku grid has been supplied." << endl;
    return;
  }
  checkBoard();
  toFix(errorList);
}
