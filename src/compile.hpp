#ifndef COMPILE_HPP
#define COMPILE_HPP

#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <fstream>

// Function to compile a C++ source file
bool compile(const std::string &filename);

// Function to run an executable
void run(const std::string &filename);

// Function to run in another window
void run_in_another_window(const std::string &filename);

// Function to debug a file
void debug(const std::string &filename);

#endif // COMPILE_HPP
