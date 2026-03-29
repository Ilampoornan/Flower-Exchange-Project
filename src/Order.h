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
    int OriginalQuantity; // so that even if quanity is updated the original is kept
    long Sequence; // to see which order arrived first. (rather than including this in the data structure level)

    Order()
        : Side(Side::Buy), Price(0.0), Quantity(0), OriginalQuantity(0), Sequence(0) {}
};

#endif
