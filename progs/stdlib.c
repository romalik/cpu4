#include "stdlib.h"

void putc(char c) {
  *(char *)(0x4000) = c;
}

void puts(char *s) {
  while(*s) {
    putc(*s);
    s++;
  }
}

char getc() {
   return *(char *)0x4000;
}

int strcmp(const char *s1, const char *s2)
{
	const unsigned char *c1 = (const unsigned char *)s1;
	const unsigned char *c2 = (const unsigned char *)s2;
	unsigned char ch;
	int d = 0;

	while (1) {
		d = (int)(ch = *c1++) - (int)*c2++;
		if (d || !ch)
			break;
	}

	return d;
}

char *strcpy(char *dst, const char *src)
{
	char *q = dst;
	const char *p = src;
	char ch;

	do {
		*q++ = ch = *p++;
	} while (ch);

	return dst;
}

char printnum_buffer[8];
void printnum(int n) {
	char neg = 0;
	char * s = printnum_buffer + 7;
	char n_rem;
	*s = 0;
	s--;
	if(n == 0) {
		*s = '0';
		s--;
	}
	if(n < 0) {
		neg = 1;
		n = -n;
	}
	while(n) {
		n_rem = n % 10;
		*s = n_rem + '0';
		n = n / 10;
	}
	if(neg) {
		*s = '-';
		s++;
	}
	puts(s);
}
