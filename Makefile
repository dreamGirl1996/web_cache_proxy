CCFLAGS=--std=gnu++14 -pedantic -Wall -Werror -ggdb3 -pthread

hwk2_proxy: main.cpp  # node.h define.h test.h numint.h mcint.h
	g++ -o $@ $(CCFLAGS) $?

# clean: ./testcases/*~ 
clean:
	rm -f hwk2_proxy  *~ 
