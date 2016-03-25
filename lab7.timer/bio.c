//typedef unsigned int u32;
char *table = "0123456789ABCDEF";
int BASE = 10;

int gets(char s[ ])
{
	int i = 0;
	char c = '\0';

	while(1)
	{
		c = getc();
		putc(c);
		if(c == '\r')
		{
			s[i] = 0;
			putc('\n');
			break;
		}

		s[i++] = c;
	}
}

int rpu(u32 x)//print positive (non-zero) integer
{
	char c = 0;

	if(x)
	{
		c = table[x % BASE];
		rpu(x / BASE);
		putc(c);
	}

	return 0;
}



int printd(int n)
{
	BASE = 10;

	if(n == 0)
	{
		putc('0');
		return 0;
	}

	if(n < 0)
	{
		putc('-');
		n *= -1;
	}

	rpu((u32)n);
}

int printo(u32 n)
{
	BASE = 8;

	putc('0');

	if(n == 0){
		putc('0');
	}else{
		rpu(n);
	}

	return 0;

}

int printu(u32 n)
{
	BASE = 10;

	if(n == 0)
	{
		putc('0');
	}
	else
	{
		rpu('0');
	}


	return 0;
}

int printx(u32 n)
{
	BASE = 16;

	putc('0');
	putc('x');

	if(n == 0){
		putc('0');
	}else{
		rpu(n);
	}

	return 0;
}

int prints(char s[])
{
	while(*s)
	{
		putc(*s);
		if(*s == '\n')
			putc('\r');
		s++;
	}
}

// int printf(char *fmt, ...)
// {
// 	char *c = fmt;
// 	int *param = &fmt;
// 	param++;

// 	while(*c)
// 	{
// 		if(*c != '%')
// 		{
// 			putc(*c);
// 			if(*c == '\n')
// 				putc('\r');
// 		}
// 		else
// 		{
// 			c++;
// 			switch(*c)
// 			{
// 				case 'd':
// 				printd(*param);
// 				param++;
// 				break;

// 				case 's':
// 				prints(*param);
// 				param++;
// 				break;

// 				case 'c':
// 				putc(*param);
// 				param++;
// 				break;

// 				case 'o':
// 				printo(*param);
// 				param++;
// 				break;

// 				case 'x':
// 				printx(*param);
// 				param++;
// 				break;

// 				case 'u':
// 				printu(*param);
// 				param++;
// 				break;

// 				default:
// 				c--;
// 				putc(*c);
// 			}
// 		}

// 		c++;
// 	}
// }
/*
int strcmp(char *s1, char *s2)
{
	while(*s1 && *s2)
	{
		if(*s1 < *s2)
		{
			return -1;
		}
		else if(*s1 > *s2)
		{
			return 1;
		}
		else
		{
			s1++;
			s2++;
		}
	}

	if(!(*s1) && !(*s2))
	{
		return 0;
	}
	
	if(!(*s1)) return -1;

	if(!(*s2)) return 1;
}*/



