#include <iostream>
#include <string>

#include "Exchange.h"

int main(int argc, char* argv[]) {
    const std::string inputFilePath = argc > 1 ? argv[1] : "orders.csv";
    const std::string outputFilePath = argc > 2 ? argv[2] : "executions.csv";

    Exchange exchange;
    const std::vector<ExecutionReport> reports = exchange.ProcessOrdersFromFile(inputFilePath);

    if (reports.empty()) {
        std::cerr << "No execution reports generated. Check input file path and contents." << std::endl;
        return 1;
    }

    exchange.WriteReportsToFile(reports, outputFilePath);

    std::cout << "Processed input: " << inputFilePath << std::endl;
    std::cout << "Generated output: " << outputFilePath << std::endl;
    std::cout << "Execution report count: " << reports.size() << std::endl;

    return 0;
}
