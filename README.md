# Flower Exchange Project

[Project Spec PDF](Session06_Project-Flower-Exchange.pdf)

---

## Folder structure

- **src/** - all the C++ source files, Exchange, OrderBook etc.
- **test_csvs/** - the 7 sample input files used for testing
- **output_csvs/** - expected outputs for each test case to compare
- **tests/** - the python test to compare
- **Makefile** - handles building the project

---

## Building

Just run make in the project root

```bash
make
```

This compiles everything and gives you the flower_exchange binary. To clean up build files run make clean.

---

## Running the app

```bash
./flower_exchange <input_csv> [output_csv]
```

The input file is required. The output path is optional.(default output_csvs/executions). for example:

```bash
# uses the default output path
./flower_exchange orders.csv

# custom output path
./flower_exchange test_csvs/Example\ 1.csv my_results.csv
```

---

## Running the tests

```bash
python3 tests/run_csv_tests.py
```

This compiles the project, runs all 7 test cases and prints pass or fail for each one.

### What the test cases cover

1. Single sell order with no match - New status
2. Multiple unmatched orders - all New
3. Two orders that fully match each other - Fill
4. Buy order larger than whats available - PFill
5. One sell order filling multiple buys - mix of PFill and Fill
6. Longer scenario with 4 orders and multiple matches
7. Invalid orders that should all be rejected - Reject

basically the difference between the 7 tests

---

## output_csvs and actual_test_runs

**output_csvs/Output 1-7.csv** are the expected output files the tests compare against. These dont include the Transaction Time column since that changes every single run and cant really be tested.

**output_csvs/actual_test_runs/** is where the test script writes the actual outputs each time you run the tests. These include Transaction Time so you can see real timestamps if you want to check what the program actually produced. This folder is not tracked by git so cant see

Note - when you run the app yourself the full output including Transaction Time goes into whatever output file you specified.
