#ifndef ORDER_H
#define ORDER_H

#include <string>
#include "Side.h"

class Order {
public:
    std::string OrderID;
    std::string ClientOrderID;
    std::string Instrument;
    Side Side;
    double Price;
    int Quantity;
    int OriginalQuantity;
    long long Sequence;

    Order()
        : Side(Side::Buy), Price(0.0), Quantity(0), OriginalQuantity(0), Sequence(0) {}
};

#endif
