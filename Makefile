.PHONY: install test

WARN_FLAGS := -Wall -Wextra -Wpedantic -Wuninitialized -Wcast-qual -Wdisabled-optimization -Winit-self -Wlogical-op -Wmissing-include-dirs -Wredundant-decls -Wshadow -Wswitch-default -Wundef -Wstrict-prototypes -Wpointer-to-int-cast -Wint-to-pointer-cast -Wconversion -Wduplicated-cond -Wduplicated-branches -Wformat=2 -Wshift-overflow=2 -Wint-in-bool-context -Wlong-long -Wvector-operation-performance -Wvla -Wdisabled-optimization -Wredundant-decls -Wmissing-parameter-type -Wold-style-declaration -Wlogical-not-parentheses -Waddress -Wmemset-transposed-args -Wmemset-elt-size -Wsizeof-pointer-memaccess -Wwrite-strings -Wcast-align=strict -Wbad-function-cast -Wtrampolines -Werror=implicit-function-declaration

export GCC_COLORS = warning=01;33

tools.o: tools.c tools.h
	gcc -fPIC -std=gnu2x $(WARN_FLAGS) -O2 -c tools.c

install: tools.h tools.o stack.h map.h arena.h prng_seeds.h
	sudo cp tools.h /usr/local/include/tools.h
	sudo cp tools.c /usr/local/include/tools.c
	sudo cp tools.o /usr/local/include/tools.o
	sudo cp stack.h /usr/local/include/stack.h
	sudo cp map.h /usr/local/include/map.h
	sudo cp arena.h /usr/local/include/arena.h
	sudo cp prng_seeds.h /usr/local/include/prng_seeds.h

test: tools.o test.c
	gcc -std=gnu2x $(WARN_FLAGS) -O2 -o test tools.o test.c
	./test
