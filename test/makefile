cc=gcc
THIRD_PATH=../../local
list_target=tlist_test
list_src=test_list.c
list_obj=${patsubst %.c, %.o, $(list_src)}

fixed_target=tfixed_test
fixed_src=test_fixed_alloc.c
fixed_obj=${patsubst %.c, %.o, $(fixed_src)}

bit_target=tbit_test
bit_src=test_bitop.c
bit_obj=${patsubst %.c, %.o, $(bit_src)}

bloom_target=tbloomfilter
bloom_src=test_bloomfilter.c
bloom_obj=${patsubst %c, %o, $(bloom_src)}

LD_FLAGS=-L$(THIRD_PATH)/lib -lcheck -L../src -ldev
CFLAGS=-I$(THIRD_PATH)/include -I../src

all:$(list_target) $(fixed_target) $(bit_target) $(bloom_target)

$(bit_target):$(bit_obj)
	$(cc) -o $@ $^ $(LD_FLAGS)

$(list_target):$(list_obj)
	$(cc) -o $@ $^ $(LD_FLAGS)

$(fixed_target):$(fixed_obj)
	$(cc) -o $@ $^ $(LD_FLAGS)

$(bloom_target):$(bloom_obj)
	$(cc) -o $@ $^ $(LD_FLAGS)

%.o:%.c
	$(cc) -c -std=c99 $< $(CFLAGS)

clean:
	rm $(list_target) $(fixed_target) -fr
	rm *.o -fr
