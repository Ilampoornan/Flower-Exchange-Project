#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H

#include <string>
#include <vector>

#include "ExecutionReport.h"
#include "Order.h"
#include "OrderBookSide.h"

class OrderBook {
public:
    explicit OrderBook(const std::string& instrument = "");

    OrderBookSide buyOrders;
    OrderBookSide sellOrders;

    std::vector<ExecutionReport> ProcessOrder(const Order& incomingOrder);

private:
    std::string instrument;

    std::string BuildTimestamp() const;
    ExecutionReport BuildReport(
        const Order& order,
        const std::string& status,
        const std::string& reason = ""
    ) const;
};

#endif
