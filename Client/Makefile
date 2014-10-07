# Declaration of variables
CC = g++
CC_FLAGS = -w $(INC)

# File names
EXEC = user
SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

# Main target
$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXEC)
	mv $(EXEC) $(BIN_DIR)

# To obtain object files
%.o: %.cpp
	$(CC) -c $(CC_FLAGS) $< -o $@

# To remove generated files
clean:
	rm -f $(BIN_DIR)/$(EXEC) $(OBJECTS)