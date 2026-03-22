#ifndef ORDER_H
#define ORDER_H

#include <string>
#include "Side.h"

class Order {
public:
    std::string ClientOrderID;
    std::string Instrument;
    Side Side;
    double Price;
    int Quantity;
};

#endif
