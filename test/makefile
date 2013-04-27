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
bloom_obj=${patsubst %.c, %.o, $(bloom_src)}

hashtable_target=thashtable
hashtable_src=test_hash_table.c
hashtable_obj=${patsubst %.c, %.o, $(hashtable_src)}

lock_target=tlock
lock_src=test_lock.c
lock_obj=${patsubst %.c, %.o, $(lock_src)}

dstring_target=tdstring
dstring_src=test_string.c
dstring_obj=${patsubst %.c, %.o, $(dstring_src)}

LD_FLAGS=-L$(THIRD_PATH)/lib -lcheck -L../src -ldev
CFLAGS=-I$(THIRD_PATH)/include -I../src -g -O2

all:$(list_target) $(fixed_target) $(bit_target) $(bloom_target) $(hashtable_target) $(lock_target) $(dstring_target)

$(dstring_target):$(dstring_obj)
	$(cc) -o $@ $^ $(LD_FLAGS)

$(lock_target):$(lock_obj)
	$(cc) -o $@ $^ $(LD_FLAGS)

$(bit_target):$(bit_obj)
	$(cc) -o $@ $^ $(LD_FLAGS)

$(hashtable_target):$(hashtable_obj)
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
	rm $(list_target) $(fixed_target) $(hashtable_target) $(bloom_target) $(bit_target) -fr
	rm *.o -fr
