SRC_DIR = .
TARGET = GIS
CC=g++ -std=c++11

default:
	$(CC) -o $(TARGET) $(SRC_DIR)/$(TARGET).cc

clean:
	rm -r $(TARGET)

run:
	./$(TARGET)

all: $(TARGET) clean run

.PHONY: default clean