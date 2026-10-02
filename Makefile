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
	@echo "Automated tests are not implemented yet."

clean:
	rm -f $(TARGET)
