CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

TARGET = vfs-emulator
SRC = $(wildcard src/*.cpp)

.PHONY: all run test clean

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run: all
	./$(TARGET)

test:
	./scripts/test_touch.sh
	./scripts/test_startup.sh

clean:
	rm -f $(TARGET)
