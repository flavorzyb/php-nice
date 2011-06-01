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

#ifndef PHP_LNICE_H
#define PHP_LNICE_H

extern zend_module_entry lnice_module_entry;
#define phpext_lnice_ptr &lnice_module_entry

#ifdef PHP_WIN32
#define PHP_LNICE_API __declspec(dllexport)
#else
#define PHP_LNICE_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(lnice);
PHP_MSHUTDOWN_FUNCTION(lnice);
PHP_RINIT_FUNCTION(lnice);
PHP_RSHUTDOWN_FUNCTION(lnice);
PHP_MINFO_FUNCTION(lnice);

PHP_FUNCTION(lnice_get_cpu_info);

/*
    Declare any global variables you may need between the BEGIN
    and END macros here:

ZEND_BEGIN_MODULE_GLOBALS(lnice)
    long  global_value;
    char *global_string;
ZEND_END_MODULE_GLOBALS(lnice)
*/

/* In every utility function you add that needs to use variables
   in php_lnice_globals, call TSRMLS_FETCH(); after declaring other
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as LNICE_G(variable).  You are
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define LNICE_G(v) TSRMG(lnice_globals_id, zend_lnice_globals *, v)
#else
#define LNICE_G(v) (lnice_globals.v)
#endif

#endif  /* PHP_LNICE_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
