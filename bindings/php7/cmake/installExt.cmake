execute_process(
        COMMAND php -v
        OUTPUT_VARIABLE PHP_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(REGEX MATCH "PHP [^.].[^.]" PHP_VERSION ${PHP_VERSION})
string(REPLACE "PHP " "" PHP_VERSION ${PHP_VERSION})
message(STATUS "Current PHP Version: ${PHP_VERSION}")
message(STATUS "Updated libotic_php.ini file")