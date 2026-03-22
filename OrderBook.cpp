#include "OrderBook.h"

void OrderBook::ProcessOrder(const Order& ord) {
    if (ord.Side == Side::Buy) {
        buyOrders.InsertOrder(ord);
    } else {
        sellOrders.InsertOrder(ord);
    }
}

bool OrderBook::isMatchingOrder(const Order& ord) {
    (void)ord;
    return false;
}
