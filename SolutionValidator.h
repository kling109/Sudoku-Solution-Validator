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

class SudokuSolutionValidator
{
  private:
    std::vector<std::string>* solutionList;
    std::vector<std::vector<int>* >* errorList;
    int** gameBoard;
    pthread_mutex_t lock;
    bool insertUniqueInt(int elem, std::vector<int>* list);
    void insertUniqueRecord(std::vector<int>* record, std::vector<std::vector<int>* >* fullList);
    void checkRows();
    void findRowError(int i);
    void checkColumns();
    void findColumnError(int j);
    void checkBlocks();
    void findBlockError(int i, int j);
    void toFix(std::vector<std::vector<int>* >* errors);
  public:
    SudokuSolutionValidator();
    ~SudokuSolutionValidator();
    void readFile(std::string fileName);
    void fixBoard();
};
