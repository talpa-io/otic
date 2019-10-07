PHP_ARG_ENABLE(otic, whether to enable hello support, [ --enable-otic Enable hello support])
if test "$PHP_OTIC" == "yes"; then
    AC_DEFINE(HAVE_OTIC, 1, [Whether you have otic])
    PHP_NEW_EXTENSION(otic, otic_php.c, ../../../src/pack/pack.h, ../../../src/pack/pack.c, ../../../include/config/config.h,../../../src/core/core.c , ,../../../src/core/core.h, $ext_shared)
fi