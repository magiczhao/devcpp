target=gear
THIRD_PATH=../../local
srcs=${wildcard *.c}
objs=${patsubst %.c, %.o, $(srcs)}
CFLAGS=-I$(THIRD_PATH)/include -I../src -Wall -O2 -g
LDFLAGS=-L../src -ldev -L$(THIRD_PATH)/lib -ltcc -ldl -llog4c
all:$(target)

$(target):$(objs)
	gcc -o $@ $^ $(LDFLAGS)

%.o:%.c
	gcc -std=c99 -c $<  $(CFLAGS)

clean:
	rm $(target) -f
	rm $(objs) -fr
