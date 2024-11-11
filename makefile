CC = gcc
CFLAGS = -lpthread

# Targets for building the main server
main: main.c echo.c http_message.c routines.c
	$(CC) -o main main.c echo.c http_message.c routines.c $(CFLAGS)

main.o: main.c echo.h http_message.h routines.h
	$(CC) -c main.c

echo.o: echo.c echo.h
	$(CC) -c echo.c

http_message.o: http_message.c http_message.h
	$(CC) -c http_message.c

routines.o: routines.c routines.h
	$(CC) -c routines.c


