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

int isdigit(char c) {
    return (c >= '0' && c <= '9');
}

int atoi(const char *s) {
    int res = 0;
    while (isdigit(*s)) {
        res = res * 10;
        res += (*s) - '0';
        s++;
    }
    return res;
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

int strlen(const void *s) {
    const char *p = (const char *)s;
    int n = 0;
    while (*p) {
        p++;
        n++;
    }
    return n;
}

void reverse(char *s) {
    int i, j;
    char c;

    for (i = 0, j = strlen(s) - 1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void itoa(int n, char s[]) {
    int i, sign = 0;
    if (n < 0) { /* record sign */
        s[0] = '-';
        itoa(-n, s + 1);
        return;
    }
    i = 0;
    do { /* generate digits in reverse order */

        s[i++] = n % 10 + '0'; /* get next digit */
    } while ((n /= 10) > 0);   /* delete it */
    if (sign == 1) {
        s[i++] = '-';
    }
    s[i] = '\0';
    reverse(s);
}


char *strncpy(char *d, const char *s, unsigned int l)
{
        char *s1 = d;
        const char *s2 = s;

        while (l) {
                l--;
                if ((*s1++ = *s2++) == '\0')
                        break;
        }

        /* This _is_ correct strncpy is supposed to zap */
        while (l-- != 0)
                *s1++ = '\0';
        return d;
}


int memset(unsigned int *dest, const unsigned int val, int n) {
    int k = n;
    while (k--) {
        *dest = val;
        dest++;
    }
    return n;
}

int memcpy(unsigned int *dest, const unsigned int *src, int n) {
    int k = n;
    while (k--) {
        *dest = *src;
        dest++;
        src++;
    }
    return n;
}

int memcpy_r(unsigned int *dest, const unsigned int *src, int n) {
    int k = n;
    while (k--) {
        *dest = *src;
        dest++;
        src++;
    }
    return n;
}

int memcmp(const void *mem1, const void *mem2, int len)
{
        const signed char *p1 = mem1, *p2 = mem2;

        if (!len)
                return 0;

        while (--len && *p1 == *p2) {
                p1++;
                p2++;
        }
        return *p1 - *p2;
}
     

char printnum_buffer[8];

void printhex(unsigned int n) {
	char * s = printnum_buffer + 7;
	char n_rem;
	*s = 0;
	s--;
	
	if(n == 0) {
		*s = '0';
		s--;
	}
	
	while(n) {
		n_rem = n & 0x0f;
		if(n_rem > 9) {
			*s = n_rem + 'A' - 10;
		} else {
			*s = n_rem + '0';
		}
		n = n >> 4;
		s--;
	}
	*s = 'x';
	s--;
	*s = '0';
	puts(s);
}


void printnum(int n) {
	char neg = 0;
	char * s = printnum_buffer + 7;
	char n_rem;
	*s = 0;
	if(n == 0) {
		s--;
		*s = '0';
	}
	if(n < 0) {
		neg = 1;
		n = -n;
	}
	while(n) {
		s--;
		n_rem = n % 10;
		*s = n_rem + '0';
		n = n / 10;
	}
	if(neg) {
		s--;
		*s = '-';
	}
	puts(s);
}




static void printchar(char **str, int c) {
    if (str) {
        **str = c;
        ++(*str);
    } else
        (void)putc(c);
}

#define PAD_RIGHT 1
#define PAD_ZERO 2

static int prints(char **out, const char *string, int width, int pad) {
    int pc = 0, padchar = ' ';

    if (width > 0) {
        int len = 0;
        const char *ptr;
        for (ptr = string; *ptr; ++ptr)
            ++len;
        if (len >= width)
            width = 0;
        else
            width -= len;
        if (pad & PAD_ZERO)
            padchar = '0';
    }
    if (!(pad & PAD_RIGHT)) {
        for (; width > 0; --width) {
            printchar(out, padchar);
            ++pc;
        }
    }
    for (; *string; ++string) {
        printchar(out, *string);
        ++pc;
    }
    for (; width > 0; --width) {
        printchar(out, padchar);
        ++pc;
    }

    return pc;
}

/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 12

static int printi(char **out, int i, int b, int sg, int width, int pad,
                  int letbase) {
    char *s;
    int t, neg = 0, pc = 0;
    unsigned int u = i;
    char print_buf[PRINT_BUF_LEN];

    if (i == 0) {
        print_buf[0] = '0';
        print_buf[1] = '\0';
        return prints(out, print_buf, width, pad);
    }

    if (sg && b == 10 && i < 0) {
        neg = 1;
        u = -i;
    }

    s = print_buf + PRINT_BUF_LEN - 1;
    *s = '\0';

    if (b == 16) {
        while (u) {
            t = u & 0xf;
            if (t >= 10)
                t += letbase - '0' - 10;
            *--s = t + '0';
            u = u >> 4;
        }
    } else {
        while (u) {
            t = u % b;
            if (t >= 10)
                t += letbase - '0' - 10;
            *--s = t + '0';
            u /= b;
        }
    }

    if (neg) {
        if (width && (pad & PAD_ZERO)) {
            printchar(out, '-');
            ++pc;
            --width;
        } else {
            *--s = '-';
        }
    }

    return pc + prints(out, s, width, pad);
}

static int printl(char **out, long i, int b, int sg, int width, int pad,
                  int letbase) {
    char *s;
    int t, neg = 0, pc = 0;
    unsigned long u = i;
    char print_buf[PRINT_BUF_LEN];

    if (i == 0) {
        print_buf[0] = '0';
        print_buf[1] = '\0';
        return prints(out, print_buf, width, pad);
    }

    if (sg && b == 10 && i < 0) {
        neg = 1;
        u = -i;
    }

    s = print_buf + PRINT_BUF_LEN - 1;
    *s = '\0';

    if (b == 16) {
        while (u) {
            t = u & 0xf;
            if (t >= 10)
                t += letbase - '0' - 10;
            *--s = t + '0';
            u = u >> 4;
        }
    } else {
        while (u) {
            t = u % b;
            if (t >= 10)
                t += letbase - '0' - 10;
            *--s = t + '0';
            u /= b;
        }
    }

    if (neg) {
        if (width && (pad & PAD_ZERO)) {
            printchar(out, '-');
            ++pc;
            --width;
        } else {
            *--s = '-';
        }
    }

    return pc + prints(out, s, width, pad);
}

static int print(char **out, int *varg) {
    char scr[2];
    int width, pad;
    int pc = 0;
    char *format = (char *)(*varg++);
    for (; *format != 0; ++format) {
        if (*format == '%') {
            ++format;
            width = pad = 0;
            if (*format == '\0')
                break;
            if (*format == '%')
                goto out;
            if (*format == '-') {
                ++format;
                pad = PAD_RIGHT;
            }
            while (*format == '0') {
                ++format;
                pad |= PAD_ZERO;
            }
            for (; *format >= '0' && *format <= '9'; ++format) {
                width *= 10;
                width += *format - '0';
            }
            if (*format == 's') {
                char *s = *((char **)varg++);
                pc += prints(out, s ? s : "(null)", width, pad);
                continue;
            }
            if (*format == 'd') {
                pc += printi(out, *varg++, 10, 1, width, pad, 'a');
                continue;
            }
            if (*format == 'x') {
                pc += printi(out, *varg++, 16, 0, width, pad, 'a');
                continue;
            }
            if (*format == 'X') {
                pc += printi(out, *varg++, 16, 0, width, pad, 'A');
                continue;
            }
            if (*format == 'u') {
                pc += printi(out, *varg++, 10, 0, width, pad, 'a');
                continue;
            }
            if (*format == 'l') {
                pc += printl(out, *varg++, 10, 1, width, pad, 'a');
                varg++;
                continue;
            }
            if (*format == 'c') {
                /* char are converted to int then pushed on the stack */
                scr[0] = *varg++;
                scr[1] = '\0';
                pc += prints(out, scr, width, pad);
                continue;
            }
        } else {
        out:
            printchar(out, *format);
            ++pc;
        }
    }
    if (out)
        **out = '\0';
    return pc;
}

/* assuming sizeof(void *) == sizeof(int) */

int printf(const char *format, ...) {
    int *varg = (int *)(&format);
    return print(0, varg);
}

int sprintf(char *out, const char *format, ...) {
    int *varg = (int *)(&format);
    return print(&out, varg);
}



int multiply16bit(int m, int n)
{
    unsigned char mLow = (m & 0x00FF);          // stores first 8-bits of m
    unsigned char mHigh = (m & 0xFF00) >> 8;    // stores last 8-bits of m
 
    unsigned char nLow = (n & 0x00FF);          // stores first 8-bits of n
    unsigned char nHigh = (n & 0xFF00) >> 8;    // stores last 8-bits of n
 
    unsigned short mLow_nLow = mLow * nLow;
    unsigned short mHigh_nLow = mHigh * nLow;
    unsigned short mLow_nHigh = mLow * nHigh;
    unsigned short mHigh_nHigh = mHigh * nHigh;
 
    // return 32-bit result (don't forget to shift mHigh_nLow and
    // mLow_nHigh by 1 byte and mHigh_nHigh by 2 bytes)
 
    return mLow_nLow + ((mHigh_nLow + mLow_nHigh) << 8) + (mHigh_nHigh << 16);
}

long __crt_MODU4(long a, long b) {
    return 1;
}

long __crt_DIVU4(long a, long b) {
    return 1;
}