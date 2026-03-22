#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H

#include "Order.h"
#include "OrderBookSide.h"

class OrderBook {
public:
    OrderBookSide buyOrders;
    OrderBookSide sellOrders;

    void ProcessOrder(const Order& ord);
    bool isMatchingOrder(const Order& ord);
};

#endif
