CC = gcc
SRC = src/main.c
OUT = xdb

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)