.PHONY: install test

WARN_FLAGS := -Wcast-align -Wcast-qual -Wdisabled-optimization -Winit-self -Wlogical-op -Wmissing-include-dirs -Wredundant-decls -Wshadow -Wsign-conversion -Wstrict-overflow=5 -Wswitch-default -Wundef -Wall -Wstrict-prototypes -Wextra -Wpedantic -Wuninitialized -Wpointer-to-int-cast -Wint-to-pointer-cast -Wconversion -Wduplicated-cond -Wformat=2 -Wshift-overflow=2 -Wold-style-definition -Wint-in-bool-context -Wlong-long -Wvector-operation-performance -Wvla -Wdisabled-optimization -Wredundant-decls -Wmissing-parameter-type -Wold-style-declaration -Wlogical-not-parentheses -Waddress -Wmemset-transposed-args -Wmemset-elt-size -Wsizeof-pointer-memaccess -Wwrite-strings -Wcast-align=strict -Wbad-function-cast -Wundef -Wtrampolines -Werror=implicit-function-declaration

install: tools.h stack.h map.h arena.h prng_seeds.h
	sudo cp tools.h /usr/local/include/tools.h
	sudo cp stack.h /usr/local/include/stack.h
	sudo cp map.h /usr/local/include/map.h
	sudo cp arena.h /usr/local/include/arena.h
	sudo cp prng_seeds.h /usr/local/include/prng_seeds.h

test: test.c
	gcc -std=gnu2x $(WARN_FLAGS) -fsanitize=undefined -O2 -o test test.c
	./test
