#include "Exchange.h"

#include <chrono>
#include <set>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>

const std::set<std::string> Exchange::validInstruments = {
    "Rose", "Lavender", "Lotus", "Tulip", "Orchid"
};

bool Exchange::IsHeader(const std::string& line) {
    return line.find("ClientOrderID") != std::string::npos ||
           line.find("Client Order ID") != std::string::npos;
}

std::vector<std::string> Exchange::SplitCsvLine(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream ss(line);
    std::string token;
    while (std::getline(ss, token, ',')) {
        fields.push_back(token);
    }
    return fields;
}

bool Exchange::TryParseSide(const std::string& value, Side& side) {
    if (value == "1") {
        side = Side::Buy;
        return true;
    }
    if (value == "2") {
        side = Side::Sell;
        return true;
    }
    return false;
}

std::string Exchange::SideToCsv(Side side) {
    return side == Side::Buy ? "1" : "2";
}

bool Exchange::ValidateOrder(const Order& order, std::string& reason) const {
    if (order.ClientOrderID.empty()) {
        reason = "Invalid ClientOrderID";
        return false;
    }
    if (validInstruments.find(order.Instrument) == validInstruments.end()) {
        reason = "Invalid instrument";
        return false;
    }
    if (order.Price <= 0.0) {
        reason = "Invalid price";
        return false;
    }
    if (order.Quantity % 10 != 0 || order.Quantity < 10 || order.Quantity > 1000) {
        reason = "Invalid quantity";
        return false;
    }
    reason.clear();
    return true;
}

std::string Exchange::GenerateExchangeOrderId() {
    std::ostringstream oss;
    oss << "EX" << nextExchangeOrderId++;
    return oss.str();
}

std::string Exchange::BuildTimestamp() const {
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

ExecutionReport Exchange::BuildRejectReport(const Order& order, const std::string& reason) const {
    ExecutionReport report;
    report.OrderID = order.OrderID;
    report.ClientOrderID = order.ClientOrderID;
    report.Instrument = order.Instrument;
    report.Side = order.Side;
    report.Price = order.Price;
    report.Quantity = order.OriginalQuantity;
    report.Status = "Reject";
    report.Reason = reason;
    report.TransactionTime = BuildTimestamp();
    return report;
}

std::vector<ExecutionReport> Exchange::RouteOrder(const Order& order) {
    auto it = orderBooks.find(order.Instrument);
    if (it == orderBooks.end()) {
        it = orderBooks.emplace(order.Instrument, OrderBook(order.Instrument)).first;
    }
    return it->second.ProcessOrder(order);
}

std::vector<ExecutionReport> Exchange::ProcessOrdersFromFile(const std::string& inputFilePath) {
    std::vector<ExecutionReport> reports;

    std::ifstream inputFile(inputFilePath);
    if (!inputFile.is_open()) {
        return reports;
    }

    auto normalizeHeader = [](const std::string& text) {
        std::string out;
        out.reserve(text.size());
        for (char ch : text) {
            if (ch == ' ' || ch == '_') {
                continue;
            }
            out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
        }
        return out;
    };

    std::map<std::string, int> columnIndex;
    std::string line;
    while (std::getline(inputFile, line)) {
        if (line.empty()) {
            continue;
        }

        if (IsHeader(line)) {
            columnIndex.clear();
            const std::vector<std::string> headerFields = SplitCsvLine(line);
            for (int i = 0; i < static_cast<int>(headerFields.size()); ++i) {
                columnIndex[normalizeHeader(headerFields[i])] = i;
            }
            continue;
        }

        const std::vector<std::string> fields = SplitCsvLine(line);
        if (fields.size() != 5) {
            Order malformed;
            malformed.OrderID = GenerateExchangeOrderId();
            malformed.ClientOrderID = fields.empty() ? "" : fields[0];
            malformed.Instrument = fields.size() > 1 ? fields[1] : "";
            malformed.Side = Side::Buy;
            malformed.Price = 0.0;
            malformed.Quantity = 0;
            malformed.OriginalQuantity = 0;
            malformed.Sequence = nextOrderSequence++;
            reports.push_back(BuildRejectReport(malformed, "Invalid CSV format"));
            continue;
        }

        Order order;
        order.OrderID = GenerateExchangeOrderId();

        auto getColumn = [&](const std::string& normalizedName, int fallbackIndex) {
            const auto it = columnIndex.find(normalizedName);
            const int index =
                (it != columnIndex.end() && it->second >= 0 && it->second < static_cast<int>(fields.size()))
                    ? it->second
                    : fallbackIndex;
            return fields[index];
        };

        order.ClientOrderID = getColumn("clientorderid", 0);
        order.Instrument = getColumn("instrument", 1);

        Side parsedSide = Side::Buy;
        if (!TryParseSide(getColumn("side", 2), parsedSide)) {
            order.Side = Side::Buy;
            order.Price = 0.0;
            order.Quantity = 0;
            order.OriginalQuantity = 0;
            order.Sequence = nextOrderSequence++;
            reports.push_back(BuildRejectReport(order, "Invalid side"));
            continue;
        }
        order.Side = parsedSide;

        try {
            order.Price = std::stod(getColumn("price", 3));
            order.Quantity = std::stoi(getColumn("quantity", 4));
        } catch (...) {
            order.Price = 0.0;
            order.Quantity = 0;
            order.OriginalQuantity = 0;
            order.Sequence = nextOrderSequence++;
            reports.push_back(BuildRejectReport(order, "Invalid numeric value"));
            continue;
        }

        order.OriginalQuantity = order.Quantity;
        order.Sequence = nextOrderSequence++;

        std::string reason;
        if (!ValidateOrder(order, reason)) {
            reports.push_back(BuildRejectReport(order, reason));
            continue;
        }

        const std::vector<ExecutionReport> orderReports = RouteOrder(order);
        reports.insert(reports.end(), orderReports.begin(), orderReports.end());
    }

    return reports;
}

void Exchange::WriteReportsToFile(
    const std::vector<ExecutionReport>& reports,
    const std::string& outputFilePath
) const {
    std::ofstream outputFile(outputFilePath);
    outputFile << "OrderID,ClientOrderID,Instrument,Side,Price,Quantity,Status,Reason,TransactionTime\n";

    for (const ExecutionReport& report : reports) {
        outputFile << report.OrderID << ","
                   << report.ClientOrderID << ","
                   << report.Instrument << ","
                   << SideToCsv(report.Side) << ","
                   << std::fixed << std::setprecision(1) << report.Price << ","
                   << report.Quantity << ","
                   << report.Status << ","
                   << report.Reason << ","
                   << report.TransactionTime << "\n";
    }
}
