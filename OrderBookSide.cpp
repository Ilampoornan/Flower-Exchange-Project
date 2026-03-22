#include "OrderBookSide.h"

void OrderBookSide::InsertOrder(const Order& ord) {
    orders.push_back(ord);
}

void OrderBookSide::DeleteOrder(const std::string& orderID) {
    for (auto it = orders.begin(); it != orders.end(); ++it) {
        if (it->ClientOrderID == orderID) {
            orders.erase(it);
            break;
        }
    }
}
