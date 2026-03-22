#ifndef EXCHANGE_H
#define EXCHANGE_H

#include <map>
#include <string>
#include "OrderBook.h"

class Exchange {
public:
    std::map<std::string, OrderBook> orderBooks;

    void ReadFile();
};

#endif
