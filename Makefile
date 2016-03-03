EXEC           = srt-player
SOURCES        = $(wildcard *.cpp)
CFLAGS         = -Wall
LDLIBS        = -lncurses # -levent
OBJECTS        = $(SOURCES:.cpp=.o)
CC             = g++

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDLIBS)
	
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
	
clean:
	rm -rf *.o $(EXEC)
