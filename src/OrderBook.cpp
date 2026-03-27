#include "OrderBook.h"

#include <chrono>
#include <iomanip>
#include <sstream>

OrderBook::OrderBook(const std::string& instrument)
    : buyOrders(true), sellOrders(false), instrument(instrument) {}

std::string OrderBook::BuildTimestamp() const {
    const auto now = std::chrono::system_clock::now();
    const auto nowTimeT = std::chrono::system_clock::to_time_t(now);

    std::tm tmLocal{};
#ifdef _WIN32
    localtime_s(&tmLocal, &nowTimeT);
#else
    localtime_r(&nowTimeT, &tmLocal);
#endif

    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch())
                    % 1000;

    std::ostringstream oss;
    oss << std::put_time(&tmLocal, "%Y%m%d-%H:%M:%S") << "." << std::setw(3)
        << std::setfill('0') << ms.count();
    return oss.str();
}

ExecutionReport OrderBook::BuildReport(
    const Order& order,
    const std::string& status,
    const std::string& reason
) const {
    ExecutionReport report;
    report.OrderID = order.OrderID;
    report.ClientOrderID = order.ClientOrderID;
    report.Instrument = order.Instrument;
    report.Side = order.Side;
    report.Price = order.Price;
    report.Quantity = order.OriginalQuantity;
    report.Status = status;
    report.Reason = reason;
    report.TransactionTime = BuildTimestamp();
    return report;
}

std::vector<ExecutionReport> OrderBook::ProcessOrder(const Order& incomingOrder) {
    std::vector<ExecutionReport> reports;
    Order incoming = incomingOrder;

    if (incoming.Side == Side::Buy) {
        while (incoming.Quantity > 0 && !sellOrders.Empty()) {
            Order& resting = sellOrders.Top();
            if (incoming.Price < resting.Price) {
                break;
            }

            const int tradedQty = (incoming.Quantity < resting.Quantity)
                                      ? incoming.Quantity
                                      : resting.Quantity;
            incoming.Quantity -= tradedQty;
            resting.Quantity -= tradedQty;

            reports.push_back(
                BuildReport(incoming, incoming.Quantity == 0 ? "Fill" : "PFill"));
            reports.push_back(
                BuildReport(resting, resting.Quantity == 0 ? "Fill" : "PFill"));

            if (resting.Quantity == 0) {
                sellOrders.PopTop();
            }
        }

        if (incoming.Quantity > 0) {
            if (incoming.Quantity == incoming.OriginalQuantity) {
                reports.push_back(BuildReport(incoming, "New"));
            } else {
                reports.push_back(BuildReport(incoming, "PFill"));
                reports.push_back(BuildReport(incoming, "New"));
            }
            buyOrders.InsertOrder(incoming);
        }
    } else {
        while (incoming.Quantity > 0 && !buyOrders.Empty()) {
            Order& resting = buyOrders.Top();
            if (incoming.Price > resting.Price) {
                break;
            }

            const int tradedQty = (incoming.Quantity < resting.Quantity)
                                      ? incoming.Quantity
                                      : resting.Quantity;
            incoming.Quantity -= tradedQty;
            resting.Quantity -= tradedQty;

            reports.push_back(
                BuildReport(incoming, incoming.Quantity == 0 ? "Fill" : "PFill"));
            reports.push_back(
                BuildReport(resting, resting.Quantity == 0 ? "Fill" : "PFill"));

            if (resting.Quantity == 0) {
                buyOrders.PopTop();
            }
        }

        if (incoming.Quantity > 0) {
            if (incoming.Quantity == incoming.OriginalQuantity) {
                reports.push_back(BuildReport(incoming, "New"));
            } else {
                reports.push_back(BuildReport(incoming, "PFill"));
                reports.push_back(BuildReport(incoming, "New"));
            }
            sellOrders.InsertOrder(incoming);
        }
    }

    return reports;
}
