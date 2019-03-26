# Sudoku Solution Validator

Trevor Kling: CPSC 380 Assignment 1

## Introduction
This program implements a Sudoku puzzle validator.  The program takes an input of a formatted text file, and outputs a set of reccomendations to fix the given grid.   
The file should be formatted in a grid, with entries in each row separated by commas.  The program then will read in the file, evaluate the grid for errors, and output
a list of actions to take in order to fix any issues in the grid.

## Specification
Programmed in C++ with standard libraries.  Implemented with pthreads, compiled with g++.  Intended to be run on Linux based operating systems, tested on Ubuntu 18.04
subsystem for Windows.

## Known Issues
None.

## Compilation Instructions
Compile on the Linux command line with g++.  The flag "-lpthread" must be included to enable multithreading.

## Sources

https://thispointer.com/c-how-to-pass-class-member-function-to-pthread_create/
