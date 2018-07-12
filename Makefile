cachesim: cachesim.c
	gcc -Wall -Werror -fsanitize=address -o cachesim cachesim.c -lm

clean:
	rm -rf cachesim 

