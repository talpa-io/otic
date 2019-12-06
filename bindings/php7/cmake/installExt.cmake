message(STATUS "Greeting from the Extension Installer")

execute_process(
        COMMAND php -r "phpinfo();"
        OUTPUT_VARIABLE PHP_INI_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(REGEX MATCH "Loaded Configuration File => [^ ]*php.ini" PHP_INI_PATH ${PHP_INI_PATH})
string(REPLACE "Loaded Configuration File => " "" PHP_INI_PATH ${PHP_INI_PATH})

message(STATUS "PHP Ini Path: ${PHP_INI_PATH}")

set(EXTENSION_PARAMETERS ";extension=libotic_php.so")

FILE(READ ${PHP_INI_PATH} PHP_INI_CONTENT)
string(REGEX MATCH "[Inter[^]]*]" EXT_RES ${PHP_INI_CONTENT})


#if (NOT ${EXT_RES})
#    message(STATUS "NOT Found!")
#else()
#    message(STATUS "Found: ${EXT_RES}")
#endif()