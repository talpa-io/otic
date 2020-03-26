message(STATUS "Current PHP Version: ${PHP_VERSION}")
file(WRITE "/etc/php/${PHP_VERSION}/mods-available/libotic_php.ini" "extension=libotic_php")
message(STATUS "Updated libotic_php.ini file")