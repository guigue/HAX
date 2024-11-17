/**
 * Copyright (c) 2017 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>

#define LOG_VERSION "0.1.0"

typedef void (*log_LockFn)(void *udata, int lock);

enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

#define log_trace(...) log_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  log_log(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  log_log(LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

void log_set_udata(void *udata);
void log_set_lock(log_LockFn fn);
void log_set_fp(FILE *fp);
void log_set_level(int level);
void log_set_quiet(int enable);

void log_log(int level, const char *file, int line, const char *fmt, ...);

static void lock(void);
static void unlock(void);



// Preliminar logging definitions
// ------------------------------

static struct
{
        void *udata;
        log_LockFn lock;
        FILE *fp;
        int level;
        int quiet;
}L;

static const char *level_names[] =
{
        "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

#ifdef LOG_USE_COLOR
static const char *level_colors[] =
{
        "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};
#endif



        // SISTEMA DE LOGGING
        // ------------------
        //  Setting the logging file name by year and concatenate with the DIRECTORY_LOG path
        //  Log file name format: HATS_Control_<YEAR>.log
        // ---------------------------------------------------------------------------------
/*
        time_t current_time;
        struct tm *time_info;
        char year[5];
        int size;
        size=strlen(DIRECTORY_LOG)+5+17;   //Size of "HATS_Control_" + ".log" = 17 caracteres
        char Nome_Arquivo_Log[size];

        current_time = time(NULL);
        time_info = localtime(&current_time);
        strftime(year,5,"%Y",time_info);
        sprintf(Nome_Arquivo_Log,"%sHATS_Control_%s.log",DIRECTORY_LOG,year);


        FILE *fp;

        if ((fp=fopen(Nome_Arquivo_Log, "a"))==NULL)
        {
                printf("Can´t open/create the log file! Check directory permissions.\n\n");
                exit(1);
        }

        log_set_fp(fp);
        log_set_quiet(1);
        //-----------------------------------------------------------------------------------

*/


/***
*     L O G G I N G    F U N C T I O N S
*****************************************************************/


static void lock(void)
{
        if (L.lock)
        {
                L.lock(L.udata, 1);
        }
}

static void unlock(void)
{
        if (L.lock)
        {
                L.lock(L.udata, 0);
        }
}

void log_set_udata(void *udata)
{
        L.udata = udata;
}

void log_set_lock(log_LockFn fn)
{
        L.lock = fn;
}

void log_set_fp(FILE *fp)
{
        L.fp = fp;
}

void log_set_level(int level)
{
        L.level = level;
}

void log_set_quiet(int enable)
{
        L.quiet = enable ? 1 : 0;
}
void log_log(int level, const char *file, int line, const char *fmt, ...)
{
        if (level < L.level)
        {
                return;
        }

        /* Acquire lock */
        lock();

        /* Get current time */
        time_t t = time(NULL);
        struct tm *lt = localtime(&t);

        /* Log to stderr */
        if (!L.quiet)
        {
                va_list args;
                char buf[16];
                buf[strftime(buf, sizeof(buf), "%H:%M:%S", lt)] = '\0';
                #ifdef LOG_USE_COLOR
                        fprintf(
                                stderr, "%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ",
                                buf, level_colors[level], level_names[level], file, line);
                #else
                        fprintf(stderr, "%s %-5s %s:%d: ", buf, level_names[level], file, line);
                #endif
                        va_start(args, fmt);
                        vfprintf(stderr, fmt, args);
                        va_end(args);
                        fprintf(stderr, "\n");
                        fflush(stderr);
        }
        /* Log to file */
        if (L.fp)
        {
                va_list args;
                char buf[160];
                buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt)] = '\0';
                fprintf(L.fp, "%s %-5s %s:%d: ", buf, level_names[level], file, line);
                va_start(args, fmt);
                vfprintf(L.fp, fmt, args);
                va_end(args);
                fprintf(L.fp, "\n");
                fflush(L.fp);
        }

        /* Release lock */
        unlock();
}



#endif

