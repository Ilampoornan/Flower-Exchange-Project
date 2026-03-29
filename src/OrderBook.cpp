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
    oss << std::put_time(&tmLocal, "%Y%m%d-%H%M%S") << "." << std::setw(3)
        << std::setfill('0') << ms.count();
    return oss.str();
}

ExecutionReport OrderBook::BuildReport(
    const Order& order,
    const std::string& status,
    int quantity,
    double execPrice,
    const std::string& reason
) const {
    ExecutionReport report;
    report.OrderID = order.OrderID;
    report.ClientOrderID = order.ClientOrderID;
    report.Instrument = order.Instrument;
    report.Side = order.Side;
    report.Price = execPrice;
    report.Quantity = quantity;
    report.Status = status;
    report.Reason = reason;
    report.TransactionTime = BuildTimestamp();
    return report;
}

std::vector<ExecutionReport> OrderBook::ProcessOrder(const Order& incomingOrder) {
    std::vector<ExecutionReport> reports;
    Order incoming = incomingOrder;

    if (incoming.Side == Side::Buy) {
        // Keep matching against the best (cheapest) sell order as long as the buyer is willing to pay at least that price.
        while (incoming.Quantity > 0 && !sellOrders.Empty()) {
            Order& resting = sellOrders.Top();
            if (incoming.Price < resting.Price) {
                // The best available sell price is too high i.e no more matches possible.
                break;
            }

            // Trade the smaller of the two quantities.
            const int tradedQty = (incoming.Quantity < resting.Quantity)
                                      ? incoming.Quantity
                                      : resting.Quantity;

            const double execPrice = resting.Price;
            incoming.Quantity -= tradedQty;
            resting.Quantity -= tradedQty;

            // Fill = fully traded this side, PFill = still has quantity left after this match.
            reports.push_back(
                BuildReport(incoming, incoming.Quantity == 0 ? "Fill" : "PFill", tradedQty, execPrice));
            reports.push_back(
                BuildReport(resting, resting.Quantity == 0 ? "Fill" : "PFill", tradedQty, execPrice));

            if (resting.Quantity == 0) {
                sellOrders.PopTop();
            }
        }

        if (incoming.Quantity > 0) {
            if (incoming.Quantity == incoming.OriginalQuantity) {
                reports.push_back(BuildReport(incoming, "New", incoming.OriginalQuantity, incoming.Price));
            }
            buyOrders.InsertOrder(incoming);
        }
    } else {
        //  match against the best (highest) buy order.
        while (incoming.Quantity > 0 && !buyOrders.Empty()) {
            Order& resting = buyOrders.Top();
            if (incoming.Price > resting.Price) {
                // The best available buy price is too low i.e no more matches possible.
                break;
            }

            const int tradedQty = (incoming.Quantity < resting.Quantity)
                                      ? incoming.Quantity
                                      : resting.Quantity;
            const double execPrice = resting.Price;
            incoming.Quantity -= tradedQty;
            resting.Quantity -= tradedQty;

            reports.push_back(
                BuildReport(incoming, incoming.Quantity == 0 ? "Fill" : "PFill", tradedQty, execPrice));
            reports.push_back(
                BuildReport(resting, resting.Quantity == 0 ? "Fill" : "PFill", tradedQty, execPrice));

            if (resting.Quantity == 0) {
                buyOrders.PopTop();
            }
        }

        if (incoming.Quantity > 0) {
            if (incoming.Quantity == incoming.OriginalQuantity) {
                reports.push_back(BuildReport(incoming, "New", incoming.OriginalQuantity, incoming.Price));
            }
            sellOrders.InsertOrder(incoming);
        }
    }

    return reports;
}
