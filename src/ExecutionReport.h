#ifndef EXECUTION_REPORT_H
#define EXECUTION_REPORT_H

#include <string>
#include "Side.h"

class ExecutionReport {
public:
    std::string OrderID;
    std::string ClientOrderID;
    std::string Instrument;
    Side Side;
    double Price;
    int Quantity;
    std::string Status;
    std::string Reason;
    std::string TransactionTime;
};

#endif
