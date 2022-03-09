
#ifndef _INFILE_DEBUG_H
#define _INFILE_DEBUG_H

#ifdef _cplusplus
exetern "C" {
#endif

#define ESC_START	"\033[0;31m"	// red
#define ESC_RED		"\033[0;31m"
#define ESC_BLUE	"\033[0;34m"
#define ESC_END		"\033[0m"

//---------------------------------------------
//-- In-File Debug
//--	You should define the following macros in each file which includes this header file.
//--		#define LOG_INFO	0
//--		#define LOG_DBG		0
//--		#define LOG_VERBOSE	0
//--		#define LOG_FUNC	0
//---------------------------------------------

#define NULL_FUNCTION				do {} while(0)
#define error_printf(format, ...)   fprintf(stderr, ESC_START "[ERROR] %s()@%s,#%d: " format ESC_END, __FUNCTION__, __FILE__, __LINE__,  ##__VA_ARGS__)
#define error_puts(format)			fputs(ESC_START format ESC_END, stderr)

#if LOG_INFO==1
#define info_printf(format, ...)		fprintf(stderr, format, ##__VA_ARGS__)
#define info_puts(s)					fputs(s, stderr)
#else
#define info_printf(format, ...)		NULL_FUNCTION
info_puts(s)							NULL_FUNCTION
#endif

#if LOG_WARN==1
#define warn_printf(format, ...)		fprintf(stderr, ESC_BLUE "[WARN]@%s,#%d: " format ESC_END, __FILE__, __LINE__,  ##__VA_ARGS__)
#define warn_puts(s)					fputs(ESC_BLUE s ESC_END, stderr)
#else
#define warn_printf(format, ...)		NULL_FUNCTION
#define warn_puts(s)					NULL_FUNCTION
#endif

#if LOG_DBG==1
#define debug_printf(format, ...)		fprintf(stderr, format, ##__VA_ARGS__)
#define debug_puts(s)					fputs(s, stderr)
#else
#define debug_printf(format, ...)		NULL_FUNCTION
#define debug_puts(s)					NULL_FUNCTION
#endif

#if LOG_VERBOSE==1
#define verbose_printf(format, ...)	fprintf(stderr, format, ##__VA_ARGS__)
#define verbose_puts(s)					fputs(s, stderr)
#else
#define verbose_printf(format, ...)	NULL_FUNCTION
#define verbose_puts(s)				NULL_FUNCTION
#endif

#if LOG_FUNC==1
#define FUNC_ENTRY(format, ...)     fprintf(stderr, "Entering --> %s("format")\n", __FUNCTION__, ##__VA_ARGS__)
#define FUNC_EXIT()                 fprintf(stderr, "Exiting <-- %s()", __FUNCTION__)
#else
#define FUNC_ENTRY(format, ...)     NULL_FUNCTION
#define FUNC_EXIT()                 NULL_FUNCTION
#endif



#ifdef _cplusplus
}
#endif

#endif	// _INFILE_DEBUG_H
