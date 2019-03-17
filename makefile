CC = g++
CFLAGS = -Wall -Wextra -DDEBUG -g -std=c++11
TARGET = predictors
MAIN = read_input

VALGRIND = valgrind --leak-check=yes

all: $(TARGET)

$(TARGET):
	$(CC) $(CFLAGS) -o $(TARGET) $(MAIN).cpp $(LDFLAGS)

clean:
	rm -f $(TARGET) *~

run:
	./$(TARGET)
