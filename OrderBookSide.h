#ifndef ORDER_BOOK_SIDE_H
#define ORDER_BOOK_SIDE_H

#include <string>
#include <vector>
#include "Order.h"

class OrderBookSide {
public:
    std::vector<Order> orders;

    void InsertOrder(const Order& ord);
    void DeleteOrder(const std::string& orderID);
};

#endif
