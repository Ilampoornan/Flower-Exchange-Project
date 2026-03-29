#include "OrderBookSide.h"

OrderBookSide::OrderBookSide(bool isBuySide)
    : isBuySide(isBuySide) {}

void OrderBookSide::InsertOrder(const Order& ord) {
    /*
      Best Buy- highest price first 
      Best Sell- lowest price first 
    When two orders share the same price the one that arrived earlier gets priority 
    */
    auto goesBefore = [this](const Order& lhs, const Order& rhs) {
        if (lhs.Price != rhs.Price) {
            if (isBuySide) {
                return lhs.Price > rhs.Price;
            }
            return lhs.Price < rhs.Price;
        }
        return lhs.Sequence < rhs.Sequence;
    };

    auto it = orders.begin();
    for (; it != orders.end(); ++it) {
        if (goesBefore(ord, *it)) {
            break;
        }
    }
    orders.insert(it, ord);
}

void OrderBookSide::DeleteOrder(const std::string& orderID) {
    for (auto it = orders.begin(); it != orders.end(); ++it) {
        if (it->OrderID == orderID) {
            orders.erase(it);
            break;
        }
    }
}

bool OrderBookSide::Empty() const {
    return orders.empty();
}

Order& OrderBookSide::Top() {
    return orders.front();
}

const Order& OrderBookSide::Top() const {
    return orders.front();
}

void OrderBookSide::PopTop() {
    if (!orders.empty()) {
        orders.erase(orders.begin());
    }
}
