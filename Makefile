TARGET   = rogauracore
OBJS     = rogauracore.o
FLAGS    = -g -O0 -Wall
INCLUDES =
LIBS     = -lusb-1.0
CC       = gcc

all       : $(TARGET)

clean     :
	rm -f $(TARGET) $(OBJS)

$(TARGET) : $(OBJS)
	$(CC) $(FLAGS) -o $@ $^ $(LIBS)

%.o       : %.c
	$(CC) $(FLAGS) $(INCLUDES) -o $@ -c $<
