#line 1 "../test/mini-printf.c"

#line 43 "../test/mini-printf.c"

#line 1 "../include/x86/linux//stdarg.h"






typedef char *__va_list;

static float __va_arg_tmp;
typedef __va_list va_list;


#line 14 "../include/x86/linux//stdarg.h"

#line 19 "../include/x86/linux//stdarg.h"

#line 22 "../include/x86/linux//stdarg.h"



typedef void *__gnuc_va_list;

#line 46 "../test/mini-printf.c"
#line 1 "../test/mini-printf.h"

#line 29 "../test/mini-printf.h"





#line 1 "../include/x86/linux//stdarg.h"












#line 14 "../include/x86/linux//stdarg.h"

#line 19 "../include/x86/linux//stdarg.h"

#line 22 "../include/x86/linux//stdarg.h"





#line 35 "../test/mini-printf.h"

int mini_vsnprintf(char* buffer, unsigned int buffer_len, char *fmt, va_list va);
int mini_snprintf(char* buffer, unsigned int buffer_len, char *fmt, ...);





#line 47 "../test/mini-printf.c"

static unsigned int
mini_strlen(const char *s)
{
	unsigned int len = 0;
	while (s[len] != '\0') len++;
	return len;
}

static unsigned int
mini_itoa(int value, unsigned int radix, unsigned int uppercase, unsigned int unsig,
	 char *buffer, unsigned int zero_pad)
{
	char	*pbuffer = buffer;
	int	negative = 0;
	unsigned int	i, len;


	if (radix > 16)
		return 0;

	if (value < 0 && !unsig) {
		negative = 1;
		value = -value;
	}


	do {
		int digit = value % radix;
		*(pbuffer++) = (digit < 10 ? '0' + digit : (uppercase ? 'A' : 'a') + digit - 10);
		value /= radix;
	} while (value > 0);

	for (i = (pbuffer - buffer); i < zero_pad; i++)
		*(pbuffer++) = '0';

	if (negative)
		*(pbuffer++) = '-';

	*(pbuffer) = '\0';


#line 90 "../test/mini-printf.c"
	len = (pbuffer - buffer);
	for (i = 0; i < len / 2; i++) {
		char j = buffer[i];
		buffer[i] = buffer[len-i-1];
		buffer[len-i-1] = j;
	}

	return len;
}

int
mini_vsnprintf(char *buffer, unsigned int buffer_len, char *fmt, va_list va)
{
	char *pbuffer = buffer;
	char bf[24];
	char ch;



























	while ((ch=*(fmt++))) {
		if ((unsigned int)((pbuffer - buffer) + 1) >= buffer_len)
			break;
		if (ch!='%')
			_putc(ch);
		else {
			char zero_pad = 0;
			char *ptr;
			unsigned int len;

			ch=*(fmt++);


			if (ch=='0') {
				ch=*(fmt++);
				if (ch == '\0')
					goto end;
				if (ch >= '0' && ch <= '9')
					zero_pad = ch - '0';
				ch=*(fmt++);
			}

			switch (ch) {
				case 0:
					goto end;

				case 'u':
				case 'd':
					len = mini_itoa(( __typecode(unsigned int)==1 && sizeof(unsigned int)==4 ? (__va_arg_tmp = *(double *)(&(va += ((sizeof(double)+ 3U)&~ 3U))[-(int)((sizeof(double)+ 3U)&~ 3U)]), *(unsigned int *)&__va_arg_tmp) : *(unsigned int *)(&(va += ((sizeof(unsigned int)+ 3U)&~ 3U))[-(int)((sizeof(unsigned int)+ 3U)&~ 3U)])), 10, 0, (ch=='u'), bf, zero_pad);
					_puts(bf, len);
					break;

				case 'x':
				case 'X':
					len = mini_itoa(( __typecode(unsigned int)==1 && sizeof(unsigned int)==4 ? (__va_arg_tmp = *(double *)(&(va += ((sizeof(double)+ 3U)&~ 3U))[-(int)((sizeof(double)+ 3U)&~ 3U)]), *(unsigned int *)&__va_arg_tmp) : *(unsigned int *)(&(va += ((sizeof(unsigned int)+ 3U)&~ 3U))[-(int)((sizeof(unsigned int)+ 3U)&~ 3U)])), 16, (ch=='X'), 1, bf, zero_pad);
					_puts(bf, len);
					break;

				case 'c' :
					_putc((char)(( __typecode(int)==1 && sizeof(int)==4 ? (__va_arg_tmp = *(double *)(&(va += ((sizeof(double)+ 3U)&~ 3U))[-(int)((sizeof(double)+ 3U)&~ 3U)]), *(int *)&__va_arg_tmp) : *(int *)(&(va += ((sizeof(int)+ 3U)&~ 3U))[-(int)((sizeof(int)+ 3U)&~ 3U)]))));
					break;

				case 's' :
					ptr =( __typecode(char*)==1 && sizeof(char*)==4 ? (__va_arg_tmp = *(double *)(&(va += ((sizeof(double)+ 3U)&~ 3U))[-(int)((sizeof(double)+ 3U)&~ 3U)]), *(char* *)&__va_arg_tmp) : *(char* *)(&(va += ((sizeof(char*)+ 3U)&~ 3U))[-(int)((sizeof(char*)+ 3U)&~ 3U)]));
					_puts(ptr, mini_strlen(ptr));
					break;

				default:
					_putc(ch);
					break;
			}
		}
	}
end:
	return pbuffer - buffer;
}


int
mini_snprintf(char* buffer, unsigned int buffer_len, char *fmt, ...)
{
	int ret;
	va_list va;
((void)((va) = (sizeof(fmt)<4 ? (char *)((int *)&(fmt)+1) : (char *)(&(fmt)+1))));
	ret = mini_vsnprintf(buffer, buffer_len, fmt, va);
((void)0);

	return ret;
}
