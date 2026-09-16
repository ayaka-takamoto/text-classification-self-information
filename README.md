# Information Quantity Classifier

This repository contains the source code for an information-quantity-based text classifier.

# Repository Structure

```text
repository/
├── README.md
├── data/
│   ├── sample_TRAIN.txt
│   └── sample_TEST.txt
├── result/
└── src/
    ├── file_name_list.txt
    ├── InfoQuantityClassifierPath.cpp
    ├── InfoQuantityClassifierPath.h
    ├── SuffixArrayMatcher.cpp
    └── SuffixArrayMatcher.h
```

# Requirements

* C++17 or later
* GCC / g++

# Compilation

Move to the src directory and compile the source files.

```
cd src
g++ -std=c++17 InfoQuantityClassifierPath.cpp SuffixArrayMatcher.cpp -o classifier
```

# Execution

Run the program from the src directory.
```
./classifier
```
The output files are written to the result directory.

# Input Data Format

Each line of the input data consists of a class label and text separated by a comma.

```
<class_id>,<text>
```

Example:
```
0,the spacecraft traveled through space toward a distant planet
1,the chef prepared a warm soup with fresh vegetables
```

Training and test files follow the naming convention:
```
<dataset_name>_TRAIN.txt
<dataset_name>_TEST.txt
```
The dataset names to be processed are specified in:
```
src/file_name_list.txt
```
Sample Data

The data directory contains artificial sample data for checking that the program runs correctly.

The sample data are not the datasets used in the experiments reported in the paper.
