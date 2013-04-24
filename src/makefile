target=libdev.a
srcs=${wildcard *.c}
objs=${patsubst %.c, %.o, $(srcs)}
all:$(target)

$(target):$(objs)
	ar -rcu $@ $^

%.o:%.c
	gcc -std=c99 -c $<  -Wall -g -O2

clean:
	rm $(target) -f
	rm $(objs) -fr
