#ifndef EXCHANGE_H
#define EXCHANGE_H

#include <map>
#include <string>
#include <vector>

#include "ExecutionReport.h"
#include "Order.h"
#include "OrderBook.h"

class Exchange {
public:
    std::map<std::string, OrderBook> orderBooks;

    std::vector<ExecutionReport> ProcessOrdersFromFile(const std::string& inputFilePath);
    void WriteReportsToFile(
        const std::vector<ExecutionReport>& reports,
        const std::string& outputFilePath
    ) const;

private:
    long long nextOrderSequence = 1;
    long long nextExchangeOrderId = 1;

    static bool IsHeader(const std::string& line);
    static std::vector<std::string> SplitCsvLine(const std::string& line);
    static bool TryParseSide(const std::string& value, Side& side);
    static std::string SideToCsv(Side side);

    bool ValidateOrder(const Order& order, std::string& reason) const;
    std::string GenerateExchangeOrderId();
    std::string BuildTimestamp() const;
    ExecutionReport BuildRejectReport(const Order& order, const std::string& reason) const;

    std::vector<ExecutionReport> RouteOrder(const Order& order);
};

#endif
