CC = gcc
CFLAGS = -W -Wall -Wno-unused-parameter -Who-unused-variable -std=c11 -pedantic
all: main produced consumer
main: main.c
	$(CC) $(CFLAGS) main.c -o main
produced: produced.c
	$(CC) $(CFLAGS) produced.c -o produced
consumer: consumer.c
	$(CC) $(CFLAGS) consumer.c -o consumer
clean:
	rm main produced consumer