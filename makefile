debug:
	gcc ./source/main.c -Wall -Wextra -fsanitize=address -g -o ./../interpret.elf

release:
	gcc ./source/main.c -Wall -Wextra -o ./../interpret.elf
