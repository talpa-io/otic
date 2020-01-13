//
// Created by talpaadmin on 06.01.20.
//


#include <php.h>
#include <zend_interfaces.h>
#include "aggregator.h"

#define OTIC_AGGREG_CLASS(className, methodName) PHP_METHOD(className, methodName)

#define OTIC_AGGREG_CLASS_addValue(aggregType) PHP_METHOD(aggregType##Aggregator, addValue)
#define OTIC_AGGREG_CLASS_reset(aggregType) PHP_METHOD(aggregType##Aggregator, reset)
#define OTIC_AGGREG_CLASS_getAggregated(aggregType) PHP_METHOD(aggregType##Aggregator, getAggregated)


zend_object_handlers otic_aggreg_object_handlers;
zend_class_entry* aggregator_ce;

PHP_METHOD(Aggregator, __construct)
{
}

PHP_METHOD(Aggregator, reset)
{
}

PHP_METHOD(Aggregator, addValue)
{
}

PHP_METHOD(Aggregator, getAggregated)
{
}

PHP_METHOD(Aggregator, __destruct)
{
}

ZEND_BEGIN_ARG_INFO_EX(ADD_VAL_INFO, 0, 0, 1)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()


const zend_function_entry aggregator_methods[] = {
        PHP_ABSTRACT_ME(Aggregator, __construct, NULL)
        PHP_ABSTRACT_ME(Aggregator, __destruct, NULL)
        PHP_ME(Aggregator, reset, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT | ZEND_ACC_INTERFACE)
        PHP_ME(Aggregator, addValue, ADD_VAL_INFO, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT | ZEND_ACC_INTERFACE)
        PHP_ME(Aggregator, getAggregated, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT | ZEND_ACC_INTERFACE)
};


OTIC_AGGREG_CLASS_addValue(Avg)
{

}

OTIC_AGGREG_CLASS(AvgAggregator, getAggregated)
{

}

#undef OTIC_AGGREG_CLASS
#undef OTIC_AGGREG_CLASS_addValue
#undef OTIC_AGGREG_CLASS_reset
#undef OTIC_AGGREG_CLASS_getAggregated