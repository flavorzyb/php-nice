/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2010 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Yanbin Zhu <zhuyanbin@shiwan.com>                            |
  | Date:   2010-11-25 18:10                                             |
  +----------------------------------------------------------------------+
*/

/* $Id: 2010-11-25 18:10 Yanbin Zhu <zhuyanbin@shiwan.com> $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_lnice.h"
#include <unistd.h>
#include <sys/sysinfo.h>

#define LNICE_CLOSE_FP(fp) { \
    if (fp) \
    { \
        fclose(fp); \
        fp = NULL; \
        LNICE_EXCEPTION(1); \
    } \
} \

#define LNICE_EXCEPTION(errno) { \
    lnice_exception = errno; \
} \

typedef struct lnice_cpu_info
{
    long unsigned id;
    long unsigned utime, ntime, stime, itime;
    long unsigned iowtime, irqtime, sirqtime;
} lnice_cpu_info;

typedef struct lnice_proc_info {
    pid_t pid;
    char state;
    long unsigned utime;
    long unsigned stime;
    long unsigned delta_utime;
    long unsigned delta_stime;
    long unsigned delta_time;
    long vss;
    long rss;
    long rlim;
    int cpu;
} lnice_proc_info;

static int cpu_num = 0;

static lnice_cpu_info *old_cpu = NULL, *new_cpu = NULL;

static lnice_proc_info proc, old_proc;

static pid_t lnice_proc_id;

static int lnice_exception = 0;

static int pageSize;

/* If you declare any globals in php_lnice.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(lnice)
*/

/* True global resources - no need for thread safety here */
static int le_lnice;

static int get_cpu_info(lnice_cpu_info *cpus)
{
    FILE *fp = NULL;
    int i;
    int num;
       // enough for a /proc/stat CPU line (not the intr line)
    char buf[1024];

   /* by opening this file once, we'll avoid the hit on minor page faults
      (sorry Linux, but you'll have to close it for us) */

    for (i = 0; i <= cpu_num; i++)
    {
        cpus[cpu_num].id = 0;
        cpus[cpu_num].utime = 0;
        cpus[cpu_num].ntime = 0;
        cpus[cpu_num].stime = 0;
        cpus[cpu_num].itime = 0;
        cpus[cpu_num].iowtime = 0;
        cpus[cpu_num].irqtime = 0;
        cpus[cpu_num].sirqtime = 0;
    }

    if (!(fp = fopen("/proc/stat", "r")))
    {
        LNICE_CLOSE_FP(fp);
        return -1;
    }

    rewind(fp);
    fflush(fp);

    // first value the last slot with the cpu summary line
    if (!fgets(buf, sizeof(buf), fp))
    {
        LNICE_CLOSE_FP(fp);
        return -1;
    }

    num = sscanf(buf, "cpu %lu %lu %lu %lu %lu %lu %lu",
                &cpus[cpu_num].utime,
                &cpus[cpu_num].ntime,
                &cpus[cpu_num].stime,
                &cpus[cpu_num].itime,
                &cpus[cpu_num].iowtime,
                &cpus[cpu_num].irqtime,
                &cpus[cpu_num].sirqtime);

    if (num < 4)
    {
        LNICE_CLOSE_FP(fp);
        return -1;
    }

    // and just in case we're 2.2.xx compiled without SMP support...
    if (cpu_num == 1)
    {
        cpus[1].id = 0;
        memcpy(cpus, &cpus[1], sizeof(lnice_cpu_info));
    }

    // now value each separate cpu's tics
    for (i = 0; 1 < cpu_num && i < cpu_num; i++)
    {
        if (!fgets(buf, sizeof(buf), fp))
        {
            LNICE_CLOSE_FP(fp);
            return -1;
        }

        num = sscanf(buf, "cpu%lu  %lu %lu %lu %lu %lu %lu %lu",
                    &cpus[i].id,
                    &cpus[i].utime,
                    &cpus[i].ntime,
                    &cpus[i].stime,
                    &cpus[i].itime,
                    &cpus[i].iowtime,
                    &cpus[i].irqtime,
                    &cpus[i].sirqtime);

        if (num < 4)
        {
            LNICE_CLOSE_FP(fp);
            return -1;
        }
    }

    fclose(fp);

    return 0;
}

static lnice_cpu_info get_cpu(lnice_cpu_info *cpus, int n)
{
    if ((n > cpu_num) || (n < 0))
    {
        n = cpu_num;
    }

    return cpus[n];
}

static int get_proc_info(lnice_proc_info *proc, pid_t pid)
{
    char filename[64];
    FILE *fp = NULL;
    char buf[1024], *p_char = NULL;
    int num = 0;

    proc->pid           = pid;
    proc->cpu           = 0;
    proc->delta_stime   = 0;
    proc->delta_utime   = 0;
    proc->delta_time    = 0;
    proc->utime         = 0;
    proc->stime         = 0;
    proc->cpu           = 0;
    proc->rss           = 0;
    proc->vss           = 0;
    proc->rlim          = 0;

    sprintf(filename, "/proc/%d/stat", pid);

    fp = fopen(filename, "r");
    if (!fp)
    {
        LNICE_CLOSE_FP(fp);
        return -1;
    }

    rewind(fp);
    fflush(fp);
    if (!fgets(buf, 1024, fp))
    {
        LNICE_CLOSE_FP(fp);
        return -1;
    }

    fclose(fp);

    /* Split at first '(' and last ')' to get process name. */
    p_char = strrchr(buf, ')');

    if (!p_char)
    {
        LNICE_EXCEPTION(1);
        return -1;
    }

    *p_char = '\0';

    /* Scan rest of string. */
    num = sscanf(p_char + 1, " %c %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d "
                 "%lu %lu %*d %*d %*d %*d %*d %*d %*d %lu %ld"
                 "%ld %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %d",
                 &proc->state, &proc->utime, &proc->stime, &proc->vss, &proc->rss, &proc->rlim, &proc->cpu);

    if (num < 4)
    {
        LNICE_EXCEPTION(1);
        return -1;
    }

    return 0;
}

/* {{{ lnice_functions[]
 *
 * Every user visible function must have an entry in lnice_functions[].
 */
zend_function_entry lnice_functions[] = {
    PHP_FE(lnice_get_cpu_info,  NULL)
    {NULL, NULL, NULL}
};
/* }}} */

/* {{{ lnice_module_entry
 */
zend_module_entry lnice_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "lnice",
    lnice_functions,
    PHP_MINIT(lnice),
    PHP_MSHUTDOWN(lnice),
    PHP_RINIT(lnice),       /* Replace with NULL if there's nothing to do at request start */
    PHP_RSHUTDOWN(lnice),   /* Replace with NULL if there's nothing to do at request end */
    PHP_MINFO(lnice),
#if ZEND_MODULE_API_NO >= 20010901
    "0.1", /* Replace with version number for your extension */
#endif
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_LNICE
ZEND_GET_MODULE(lnice)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("lnice.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_lnice_globals, lnice_globals)
    STD_PHP_INI_ENTRY("lnice.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_lnice_globals, lnice_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_lnice_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_lnice_init_globals(zend_lnice_globals *lnice_globals)
{
    lnice_globals->global_value = 0;
    lnice_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(lnice)
{
    cpu_num = sysconf(_SC_NPROCESSORS_CONF);
    if (cpu_num < 1)
    {
        cpu_num = 1;
    }

    pageSize = getpagesize();

    old_cpu = (lnice_cpu_info *) malloc((1 + cpu_num) * sizeof(lnice_cpu_info));

    new_cpu = (lnice_cpu_info *) malloc((1 + cpu_num) * sizeof(lnice_cpu_info));


    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION魔兽世界活动
 */
PHP_MSHUTDOWN_FUNCTION(lnice)
{
    int i;
    for(i = 0; i <= cpu_num; i++)
    {
        free(old_cpu);
        free(new_cpu);

        old_cpu = NULL;
        new_cpu = NULL;
    }

    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(lnice)
{
    lnice_proc_id = getpid();
    get_cpu_info(old_cpu);
    get_proc_info(&old_proc, lnice_proc_id);
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(lnice)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(lnice)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "lnice support", "enabled");
    php_info_print_table_end();

    /* Remove comments if you have entries in php.ini
    DISPLAY_INI_ENTRIES();
    */
}
/* }}} */


/* Remove the following function when you have succesfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_lnice_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(lnice_get_cpu_info)
{
    long unsigned total_delta_time;
    lnice_cpu_info o_cpu, n_cpu;

    if (0 != lnice_exception)
    {
        RETURN_FALSE;
    }

    if ((0 != get_proc_info(&proc, lnice_proc_id)) ||
        (0 != get_cpu_info(new_cpu)) ||
        (0 != lnice_exception))
    {
        RETURN_FALSE;
    }

    o_cpu = get_cpu(old_cpu, proc.cpu);
    n_cpu = get_cpu(new_cpu, proc.cpu);

    total_delta_time = (n_cpu.utime + n_cpu.ntime + n_cpu.stime + n_cpu.itime
                        + n_cpu.iowtime + n_cpu.irqtime + n_cpu.sirqtime)
                     - (o_cpu.utime + o_cpu.ntime + o_cpu.stime + o_cpu.itime
                        + o_cpu.iowtime + o_cpu.irqtime + o_cpu.sirqtime);

    proc.delta_utime = proc.utime - old_proc.utime;
    proc.delta_stime = proc.stime - old_proc.stime;
    proc.delta_time  = proc.delta_utime + proc.delta_stime;

    array_init(return_value);

    add_assoc_long(return_value, "pid", lnice_proc_id);
    add_assoc_long(return_value, "cpu", proc.cpu);
    add_assoc_stringl(return_value, "state", &proc.state, 1, 0);

    add_assoc_double(return_value, "nice",   (total_delta_time == 0 ? 0 : proc.delta_time * 1.0 / total_delta_time));
    add_assoc_double(return_value, "u_nice", (total_delta_time == 0 ? 0 : proc.delta_utime * 1.0 / total_delta_time));
    add_assoc_double(return_value, "s_nice", (total_delta_time == 0 ? 0 : proc.delta_stime * 1.0 / total_delta_time));

    add_assoc_double(return_value, "idle",    (total_delta_time == 0 ? 0 : (n_cpu.itime - o_cpu.itime) * 1.0 / total_delta_time));
    add_assoc_double(return_value, "iowait",  (total_delta_time == 0 ? 0 : (n_cpu.iowtime - o_cpu.iowtime) * 1.0 / total_delta_time));
    add_assoc_double(return_value, "irq",     (total_delta_time == 0 ? 0 : (n_cpu.irqtime - o_cpu.irqtime) * 1.0 / total_delta_time));
    add_assoc_double(return_value, "softirq", (total_delta_time == 0 ? 0 : (n_cpu.sirqtime - o_cpu.sirqtime) * 1.0 / total_delta_time));

    add_assoc_long(return_value, "total_time", total_delta_time);

    add_assoc_long(return_value, "vss", proc.vss / 1024);
    add_assoc_long(return_value, "rss", proc.rss * pageSize / 1024);
    add_assoc_long(return_value, "rlim", proc.rlim / 1024);

    return ;
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
