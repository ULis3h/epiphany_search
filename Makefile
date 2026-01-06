CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -I.
LDFLAGS = -lsqlite3

TARGET = epiphany_search
SRCS = epiphany/main.cc epiphany/database/sqlite_database.cc epiphany/server/http_server.cc epiphany/observability/metrics.cc
OBJS = $(SRCS:.cc=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) *.db
