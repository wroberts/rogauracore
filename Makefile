TARGET   = rogaura
OBJS     = rogaura.o
FLAGS    = -g -O0 -Wall
INCLUDES =
LIBS     =
CC       = gcc

all       : $(TARGET)

clean     :
	rm -f $(TARGET) $(OBJS)

$(TARGET) : $(OBJS)
	$(CC) $(FLAGS) -o $@ $^ $(LIBS)

%.o       : %.c
	$(CC) $(FLAGS) $(INCLUDES) -o $@ -c $<
