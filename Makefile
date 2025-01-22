# Zmienne
CC = gcc
CFLAGS = -Iinclude -Wall -Wextra -O2 -pthread
LDFLAGS = -pthread
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
TARGET = hive_simulation

# Pliki źródłowe i obiekty
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Reguła główna
all: $(TARGET)

# Budowanie programu
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@

# Kompilacja plików obiektowych
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Czyszczenie
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# Phony targets (dla poprawności działania)
.PHONY: all clean
