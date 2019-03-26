#!/usr/bin/env bash
#Test methods for a Sudoku Solution validator
rm testMethodsFile.txt
g++ *.cpp -pthread -o SolutionValidator
for i in {1..6}
do
echo "Test Case $i" |& tee -a testMethodsFile.txt
./SolutionValidator "test$i.txt" |& tee -a testMethodsFile.txt
echo $'\n' |& tee -a testMethodsFile.txt
done
