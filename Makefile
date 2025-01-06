BIN_DIR=./bin
CFLAGS=-Wall -g3

SRC_C=$(wildcard ./*.c)
OBJ_C=$(patsubst %.c,$(BIN_DIR)/%.o,$(SRC_C))

$(BIN_DIR)/main: $(BIN_DIR) $(OBJ_C)
	gcc -o $@ $(OBJ_C)

$(BIN_DIR):
	mkdir -p $@

$(OBJ_C): $(BIN_DIR)/%.o: %.c
	gcc -c $(CFLAGS) $< -o $@
	
clean:
	@rm -rf $(BIN_DIR)