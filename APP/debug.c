#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>




/* LOG_TRACE(log level, format, args ) */
void LOG_TRACE(int lvl, char *fmt, ... )
{
	va_list  list;
	char *s, c;
	int i;
	
	if( (lvl==LOG_INFO) || (lvl==LOG_ERR))
	{
		va_start( list, fmt );

		while(*fmt)
		{
			if ( *fmt != '%' )
				putc( *fmt, stdout );
			else
			{
				switch ( *++fmt )
				{
					case 's':
						/* set r as the next char in list (string) */
						s = va_arg( list, char * );
						printf("%s", s);
						break;

					case 'd':
						i = va_arg( list, int );
						printf("%d", i);
						break;

					case 'c':
						c = va_arg( list, int);
						printf("%c",c);
						break;

					default:
						putc( *fmt, stdout );
						break;
				}
			}
			++fmt;
		}
		va_end( list );
	}
	fflush( stdout );
}



