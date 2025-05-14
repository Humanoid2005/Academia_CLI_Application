CC = gcc
CFLAGS = -Wall -Wextra -g
BIN_DIR = bin
OBJ_DIR = $(BIN_DIR)/obj
SRC_DIR = .
CLASSES_DIR = classes
LDFLAGS = -pthread -lrt

# Define source files
SERVER_SRC = server.c
SERVER_HELPER_SRC = $(CLASSES_DIR)/server_helper.c
DATA_ACCESS_SRC = $(CLASSES_DIR)/data_access.c
CLIENT_SRC = client.c

# Define object files
SERVER_OBJ = $(OBJ_DIR)/server.o
SERVER_HELPER_OBJ = $(OBJ_DIR)/server_helper.o
DATA_ACCESS_OBJ = $(OBJ_DIR)/data_access.o
CLIENT_OBJ = $(OBJ_DIR)/client.o

# Define executables
SERVER_EXEC = $(BIN_DIR)/server
CLIENT_EXEC = $(BIN_DIR)/client

# Default target
all: dirs $(SERVER_EXEC) $(CLIENT_EXEC)

# Create necessary directories
dirs:
	mkdir -p $(BIN_DIR)
	mkdir -p $(OBJ_DIR)

# Compile server
$(SERVER_EXEC): $(SERVER_OBJ) $(SERVER_HELPER_OBJ) $(DATA_ACCESS_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile client
$(CLIENT_EXEC): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile server.c
$(SERVER_OBJ): $(SERVER_SRC) $(CLASSES_DIR)/structures.h $(CLASSES_DIR)/data_access.h $(CLASSES_DIR)/server_helper.h
	$(CC) $(CFLAGS) -I$(CLASSES_DIR) -c $< -o $@

# Compile client.c
$(CLIENT_OBJ): $(CLIENT_SRC) $(CLASSES_DIR)/structures.h
	$(CC) $(CFLAGS) -I$(CLASSES_DIR) -c $< -o $@

# Compile server_helper.c
$(SERVER_HELPER_OBJ): $(SERVER_HELPER_SRC) $(CLASSES_DIR)/structures.h $(CLASSES_DIR)/data_access.h
	$(CC) $(CFLAGS) -I$(CLASSES_DIR) -c $< -o $@

# Compile data_access.c
$(DATA_ACCESS_OBJ): $(DATA_ACCESS_SRC) $(CLASSES_DIR)/structures.h
	$(CC) $(CFLAGS) -I$(CLASSES_DIR) -c $< -o $@

# Run targets
run-server: $(SERVER_EXEC)
	./$(SERVER_EXEC)

run-client: $(CLIENT_EXEC)
	./$(CLIENT_EXEC)

# Clean up
clean:
	rm -rf $(BIN_DIR)

.PHONY: all dirs run-server run-client clean