CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET   = flower_exchange
SRCS     = main.cpp Exchange.cpp OrderBook.cpp OrderBookSide.cpp Order.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET) *.o executions.csv output_csvs/executions.csv
