#include <filesystem>
#include <iostream>
#include <string>

#include "Exchange.h"

int main(int argc, char* argv[]) {
    const std::string inputFilePath = argc > 1 ? argv[1] : "orders.csv";
    const std::string outputFilePath =
        argc > 2 ? argv[2] : "output_csvs/executions.csv";

    const std::filesystem::path outputPath(outputFilePath);
    const std::filesystem::path outputDir = outputPath.parent_path();
    if (!outputDir.empty()) {
        std::error_code ec;
        std::filesystem::create_directories(outputDir, ec);
        if (ec) {
            std::cerr << "Failed to create output directory: " << outputDir
                      << " (" << ec.message() << ")" << std::endl;
            return 1;
        }
    }

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
