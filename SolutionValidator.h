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

class SudokuSolutionValidator
{
  private:
    std::vector<std::vector<int>* >* errorList;
    int** gameBoard;
    pthread_mutex_t lock;
    pthread_t threads[3];
    void formatErrors(std::vector<std::vector<int>* >* errors);
    void mergeVectors(std::vector<int>* vec1, std::vector<int>* vec2);
    void* checkRows(void* ph);
    void findRowError(int i);
    void* checkColumns(void* ph);
    void findColumnError(int j);
    void* checkBlocks(void* ph);
    void checkBoard();
    void findBlockError(int x, int y);
    std::vector<int>* identifyReplacementPair(std::vector<int>* error);
    std::vector<int>* identifyBlock(std::vector<int>* location);
    void makeReplacement(std::vector<int>* location, std::vector<int>* error);
    void toFix();
  public:
    SudokuSolutionValidator();
    ~SudokuSolutionValidator();
    void readFile(std::string fileName);
    void fixBoard();
};
