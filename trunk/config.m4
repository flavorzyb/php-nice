
dnl $Id$
dnl author: Yanbin Zhu <haker-haker@163.com>
dnl version: 1.0.0
dnl config.m4 for extension lnice


PHP_ARG_WITH(lnice, for lnice support,
[  --with-lnice             Include lnice support])

PHP_ARG_ENABLE(lnice, whether to enable lnice support,
[  --enable-lnice           Enable lnice support])

if test "$PHP_LNICE" != "no"; then
  PHP_NEW_EXTENSION(lnice, lnice.c, $ext_shared)
fi
