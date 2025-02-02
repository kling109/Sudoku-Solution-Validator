/*
Name: Trevor Kling
ID: 002270716
Email: kling109@mail.chapman.edu
Course: CPSC 380 Operating Systems
Last Date Modified: 26 March 2019
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

SudokuSolutionValidator::SudokuSolutionValidator()
{
  /*
  Default constuctor for a solution validator.  Initializes vectors for later use, and sets
  gameBoard to null to check if a file has been read later.
  */
  this->errorList = new vector<vector<int>*>();
  this->gameBoard = NULL;
}

SudokuSolutionValidator::~SudokuSolutionValidator()
{
  /*
  Default deconstructor for a solution validator.  Removes the allocated memory for the vectors, and
  deletes the gameboard if it has been allocated.  Note that iteration through the errorlist to remove
  nested vectors is unnecessary, as c++ std vectors are deleted when they fall out of scope.
  */
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

void SudokuSolutionValidator::readFile(string fileName)
{
  /*
  A file reader method for the SudokuSolutionValidator.  Takes an argument for a file name, then returns nothing.
  The file is used to initialize the gameBoard variable, which stores the value for each of the numbers in the
  sudoku grid.  To convert the characters read in by the getlne function to numbers, an ASCII conversion is used.
  Numbers start at 48 in ASCII code, so by converting the character to its code and subtracting 48 the number can be
  obtained.

  Keyword Arguments:
  fileName (string): The file to use when reading in a sudoku board.
  */
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

void* SudokuSolutionValidator::checkRows(void* ph)
{
  /*
  Iterates over all rows of the grid, checking them for errors.

  Keyword Arguments:
  ph (void*): a placeholder argument required for use in multithreading.
  */
  for (int i = 0; i < 9; ++i)
  {
    findRowError(i);
  }
}

void SudokuSolutionValidator::findRowError(int i)
{
  /*
  Checks a specific row for errors.  If a duplicate element is found in the row, it indicates that
  all 9 elements are not present in that row.  The program then pushes a record of the duplicate to the
  error list, informing the program of where an error has occurred in the grid.  A mutual exclusion lock
  ensures that, when a thread checks the list, all elements in the list are accounted for and one is not
  in the process of being added.

  Keyword Arguments:
  i (int): The row along which to search for errors.
  */
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
        errorList->push_back(error);
        pthread_mutex_unlock(&this->lock);
      }
    }
  }
}

void* SudokuSolutionValidator::checkColumns(void* ph)
{
  /*
  Iterates over all columnss of the grid, checking them for errors.

  Keyword Arguments:
  ph (void*): a placeholder argument required for use in multithreading.
  */
  for (int j = 0; j < 9; ++j)
  {
    findColumnError(j);
  }
}

void SudokuSolutionValidator::findColumnError(int j)
{
  /*
  Checks a specific column for errors.  If a duplicate element is found in the column, it indicates that
  all 9 elements are not present in that column.  The program then pushes a record of the duplicate to the
  error list, informing the program of where an error has occurred in the grid.  A mutual exclusion lock
  ensures that, when a thread checks the list, all elements in the list are accounted for and one is not
  in the process of being added.

  Keyword Arguments:
  j (int): The column along which to search for errors.
  */
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
        errorList->push_back(error);
        pthread_mutex_unlock(&this->lock);
      }
    }
  }
}

void* SudokuSolutionValidator::checkBlocks(void* ph)
{
  /*
  Iterates over all blocks in the grid, checking them for errors.

  Keyword Arguments:
  ph (void*): a placeholder argument required for use in multithreading.
  */
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

void SudokuSolutionValidator::findBlockError(int x, int y)
{
  /*
  Checks a specified 3x3 area of the grid for errors.  Each area is selected by inputting the
  x and y coordinate of the top left element.  The function then checks if every element in a
  3x3 range to the right and down from the given coordinates is unique.  If not, then the program
  adds an error to the error list.  A mutual exclusion lock ensures that, when a thread checks the
  list, all elements in the list are accounted for and one is not in the process of being added.

  Keyword Arguments:
  x (int): The x-coordinate of the upper left-hand element of the 3x3 block.
  y (int): The y-coordinate of the upper left-hand element of the 3x3 block.
  */
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
            errorList->push_back(error);
            pthread_mutex_unlock(&this->lock);
          }
        }
      }
    }
  }
}

void SudokuSolutionValidator::checkBoard()
{
  /*
  Dispatcher method for the board verification.  The program distributes the rows, columns, and blocks to different threads to check.
  Once all have completed, they are merged back into the main thread.
  */
  pthread_create(&(threads[0]), NULL, (THREADFUNCPOINTER) &SudokuSolutionValidator::checkRows, this);
  pthread_create(&(threads[1]), NULL, (THREADFUNCPOINTER) &SudokuSolutionValidator::checkColumns, this);
  pthread_create(&(threads[2]), NULL, (THREADFUNCPOINTER) &SudokuSolutionValidator::checkBlocks, this);
  pthread_join(threads[0], NULL);
  pthread_join(threads[1], NULL);
  pthread_join(threads[2], NULL);
}

void SudokuSolutionValidator::formatErrors(vector<vector<int>* >* errors)
{
  /*
  A method to handle the formatting of errors.  Primarily, this means finding any errors which share row-column pairs and
  adding them to the same composite error.  Errors are removed from the list as they are merged into other vectors, requiring
  programmer-defined updating of the "j" loop variable when vectors merge in order to prevent vectors from being missed.

  Keyword Arguments:
  errors (vector<vector<int>*>*): A pointer to a list of errors produced by the "findError" methods.
  */
  for (int i = 0; i < errors->size(); ++i)
  {
    for (int j = i+1; j < errors->size(); ++j)
    {
      bool merged = false;
      for (int k = 0; k < (errors->at(i)->size()-1)/2; ++k)
      {
        for (int l = 0; l < (errors->at(j)->size()-1)/2; ++l)
        {
          if (errors->at(i)->at(2*k) == errors->at(j)->at(2*l) && errors->at(i)->at(2*k+1) == errors->at(j)->at(2*l+1))
          {
            mergeVectors(errors->at(i), errors->at(j));
            errors->erase(errors->begin() + j);
            merged = true;
            --j;
            break;
          }
        }
        if (merged)
        {
          break;
        }
      }
    }
  }
}

void SudokuSolutionValidator::mergeVectors(vector<int>* vec1, vector<int>* vec2)
{
  /*
  A helper method that merges two vectors, ensuring that all elements added to the first vector from the
  second vector are unique.  Non-unique elements are discarded.

  Keyword Arguments:
  vec1 (vector<int>*): The first vector to potentially be merged.
  vec2 (vector<int>*): The second vector to potentially be merged.
  */
  int backElem = vec1->back();
  vec1->pop_back();
  for (int i = 0; i < (vec2->size()-1)/2; ++i)
  {
    bool newElem = true;
    for (int j = 0; j < (vec1->size())/2; ++j)
    {
      if (vec2->at(2*i) == vec1->at(2*j) && vec2->at(2*i+1) == vec1->at(2*j+1))
      {
        newElem = false;
        break;
      }
    }
    if (newElem)
    {
      vec1->push_back(vec2->at(2*i));
      vec1->push_back(vec2->at(2*i+1));
    }
  }
  vec1->push_back(backElem);
}

vector<int>* SudokuSolutionValidator::identifyReplacementPair(vector<int>* error)
{
  /*
  A method which determines which row and column from a composite error to use.  The method uses a
  voting scheme, in which each occurance of a specific row or column value is a single vote for that
  option.  The most common rows/columns are then used as the location to be replaced.

  Keyword Arguments:
  error (vector<int>*): A sudoku board error to be fixed by the program.

  Returns:
  newLoc (vector<int>*): The location to be replaced to fix the given error.
  */
  vector<vector<int>*>* rowsSeen = new vector<vector<int>* >();
  for (int i = 0; i < (error->size()-1)/2; ++i)
  {
    int row = error->at(2*i);
    bool newR = true;
    for (int j = 0; j < rowsSeen->size(); ++j)
    {
      if (rowsSeen->at(j)->at(0) == row)
      {
        newR = false;
        ++rowsSeen->at(j)->at(1);
      }
    }
    if (newR)
    {
      vector<int>* newRow = new vector<int>();
      newRow->push_back(row);
      newRow->push_back(1);
      rowsSeen->push_back(newRow);
    }
  }
  int location = 0;
  int rowReplacement = rowsSeen->at(0)->at(0);
  for (int i = 0; i < rowsSeen->size(); ++i)
  {
    if (rowsSeen->at(i)->at(1) > rowsSeen->at(location)->at(1))
    {
      rowReplacement = rowsSeen->at(i)->at(0);
      location = i;
    }
  }
  vector<vector<int>*>* colsSeen = new vector<vector<int>* >();
  for (int i = 0; i < (error->size()-1)/2; ++i)
  {
    int col = error->at(2*i+1);
    bool newC = true;
    for (int j = 0; j < colsSeen->size(); ++j)
    {
      if (colsSeen->at(j)->at(0) == col)
      {
        newC = false;
        ++colsSeen->at(j)->at(1);
      }
    }
    if (newC)
    {
      vector<int>* newCol = new vector<int>();
      newCol->push_back(col);
      newCol->push_back(1);
      colsSeen->push_back(newCol);
    }
  }
  location = 0;
  int colReplacement = colsSeen->at(0)->at(0);
  for (int i = 0; i < colsSeen->size(); ++i)
  {
    if (colsSeen->at(i)->at(1) > colsSeen->at(location)->at(1))
    {
      colReplacement = colsSeen->at(i)->at(0);
      location = i;
    }
  }
  vector<int>* newLoc = new vector<int>();
  newLoc->push_back(rowReplacement);
  newLoc->push_back(colReplacement);
  return newLoc;
}

vector<int>* SudokuSolutionValidator::identifyBlock(vector<int>* location)
{
  /*
  A simple helper method to identify which block a given location falls in.  The cascading
  nature of a switch statement is used to quickly identify the correct cases, regardless of input.

  Keyword Arguments:
  location (vector<int>*): A position vector, containing an x and a y coordinate.

  Returns:
  block (vector<int>*): The upper left-hand location in the given element's block.
  */
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

void SudokuSolutionValidator::makeReplacement(vector<int>* location, vector<int>* error)
{
  /*
  Handles replacing a given location with a correct value.  The program finds which number(s) is missing from
  the given locations' block.  One of these numbers is then used to replace the error location, and a message is
  displayed to the user indicating how to change the grid.

  Keyword Arguments:
  location (vector<int>*): The sudoku board location to be replaced.  Consists of an x-y pair.
  error (vector<int>*): The specific error that requires a replacement at that location.
  */
  vector<int>* block = identifyBlock(location);
  vector<int>* nums = new vector<int>();
  for (int i = 1; i < 10; ++i)
  {
    bool unique = true;
    for (int j = 0; j < 9; ++j)
    {
      int x = block->at(0) + (j%3);
      int y = block->at(1) + (int)(j/3);
      if (gameBoard[x][y] == i)
      {
        unique = false;
      }
    }
    if (unique)
    {
      nums->push_back(i);
    }
  }
  cout << "Replace the " << error->back() << " at [" << location->at(0)+1 << ", " << location->at(1)+1 << "] with " << nums->at(0) << "." << endl;;
  gameBoard[location->at(0)][location->at(1)] = nums->at(0);
}

void SudokuSolutionValidator::toFix()
{
  /*
  Driver method for the program.  The method runs the required processes to fix the board, then checks the board for
  any new errors.  The program continues to run until the error list is empty after the board is checked.
  */
  while (this->errorList->size() != 0)
  {
    vector<int>* location = identifyReplacementPair(this->errorList->at(0));
    makeReplacement(location, this->errorList->at(0));
    this->errorList->clear();
    checkBoard();
    formatErrors(this->errorList);
  }
}

void SudokuSolutionValidator::fixBoard()
{
  /*
  Handles interfacing with the user and running the error fixing program.  The main focus of this loop
  is simply to inform the user of where errors were found in their input, and how to fix those errors.
  The program also displays a mesage when finsihed, or if the board given is already correct.
  */
  if (gameBoard == NULL)
  {
    cout << "No sudoku grid has been supplied." << endl;
    return;
  }
  checkBoard();
  if (errorList->size() != 0)
  {
    cout << "The following cells were found to be producing conflicts:" << endl;
    for (int i = 0; i < this->errorList->size(); ++i)
    {
      for (int j = 0; j < (this->errorList->at(i)->size()-1)/2; ++j)
      {
        cout << "[" << this->errorList->at(i)->at(2*j)+1 << ", " << this->errorList->at(i)->at(2*j + 1)+1 << "] ";
      }
      cout << " Element: " << this->errorList->at(i)->back() << endl;
    }
    cout << "To fix these:" << endl;;
    formatErrors(this->errorList);
    toFix();
    cout << "No further errors found in the supplied board." << endl;
  }
  else
  {
    cout << "The supplied board is correct." << endl;
  }
}
