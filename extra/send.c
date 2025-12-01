#include <stdio.h>
#include <stdbool.h>

#define UART_DATA   0x22000000
#define UART_STATUS 0x22000004
#define UART_TX_READY 0x2

void send_char(char c) {
	while ((*(int *)UART_STATUS & UART_TX_READY) == 0);
	*(int *)UART_DATA = c;
}

bool send_file(const char* filename) {
	FILE* f = fopen(filename, "r");
	if (f == NULL)
		return false;
	while (1) {
		char c = fgetc(f);
		if (c == EOF)
			break;
		send_char(c);
	}
	fclose(f);
	return true;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Usage: %s <filename>\n", argv[0]);
		return 1;
	}
	send_file(argv[1]);
	return 0;
}
