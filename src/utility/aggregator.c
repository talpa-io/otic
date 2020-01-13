#include <limits.h>
#include <float.h>
#include <string.h>
#include "utility/aggregator.h"
#include "core/base.h"


otic_aggregType_e otic_aggreg_getType(const otic_aggreg_t* aggreg)
{
    return aggreg->type;
}

void otic_aggreg_reset(otic_aggreg_t* aggreg)
{
    aggreg->error = OTIC_AGGREG_ERROR_NONE;
    otic_oval_setn(&aggreg->value);
    switch (otic_aggreg_getType(aggreg))
    {
        case OTIC_AGGREG_MIN:
            aggreg->value.val.dval = DBL_MAX;
            break;
        case OTIC_AGGREG_MAX:
            aggreg->value.val.dval = DBL_MIN;
            break;
        case OTIC_AGGREG_AVG:
            aggreg->value.val.dval = 0.0;
            aggreg->counter = 0;
            break;
        case OTIC_AGGREG_FIRST:
            aggreg->counter = 0;
            break;
        case OTIC_AGGREG_SUM:
            aggreg->value.val.dval = 0;
            break;
        case OTIC_AGGREG_COUNT:
            aggreg->value.val.lval = 0;
            break;
    }
}

void otic_aggreg_init(otic_aggreg_t* aggreg, otic_aggregType_e type)
{
    aggreg->type = type;
    aggreg->error = OTIC_AGGREG_ERROR_NONE;
    switch(type)
    {
        case OTIC_AGGREG_MIN:
           aggreg->value.type = OTIC_TYPE_DOUBLE;
           aggreg->insert = otic_aggreg_insert_min;
           aggreg->get = otic_aggreg_get_min;
           aggreg->value.val.dval = DBL_MAX;
           break;
        case OTIC_AGGREG_MAX:
           aggreg->value.type = OTIC_TYPE_DOUBLE;
           aggreg->insert = otic_aggreg_insert_max;
           aggreg->get = otic_aggreg_get_max;
           aggreg->value.val.dval = DBL_MIN;
           break;
        case OTIC_AGGREG_AVG:
           aggreg->value.type = OTIC_TYPE_NULL;
           aggreg->insert = otic_aggreg_insert_avg;
           aggreg->get = otic_aggreg_get_avg;
           aggreg->value.val.dval = 0;
           aggreg->counter = 0;
           break;
        case OTIC_AGGREG_FIRST:
           aggreg->counter = 0;
           aggreg->value.type = OTIC_TYPE_NULL;
           aggreg->insert = otic_aggreg_insert_first;
           aggreg->get = otic_aggreg_get_first;
           break;
        case OTIC_AGGREG_LAST:
           aggreg->insert = otic_aggreg_insert_last;
           aggreg->get = otic_aggreg_get_last;
           break;
        case OTIC_AGGREG_SUM:
           aggreg->value.type = OTIC_TYPE_NULL;
           aggreg->insert = otic_aggreg_insert_sum;
           aggreg->get = otic_aggreg_get_sum;
           aggreg->value.val.dval = 0;
           break;
        case OTIC_AGGREG_COUNT:
           aggreg->value.type = OTIC_TYPE_INT_POS;
           aggreg->insert = otic_aggreg_insert_count;
           aggreg->get = otic_aggreg_get_count;
           aggreg->value.val.lval= 0;
           break;
    }
}

void otic_aggreg_close(otic_aggreg_t* aggreg)
{
    aggreg->type = OTIC_AGGREG_NULL;
    otic_oval_setn(&aggreg->value);
    aggreg->counter = 0;
    aggreg->insert = 0;
    aggreg->get = 0;
}

void otic_aggreg_insert_min(otic_aggreg_t* aggreg, oval_t* val)
{
    switch(val->type)
    {
        case OTIC_TYPE_INT_POS:
            if (val->val.lval < aggreg->value.val.dval)
                aggreg->value.val.dval = val->val.lval;
            break;
        case OTIC_TYPE_DOUBLE:
            if (val->val.dval < aggreg->value.val.dval)
                aggreg->value.val.dval = val->val.dval;
            break;
        case OTIC_TYPE_INT_NEG:
            if (-val->val.lval < aggreg->value.val.dval)
                aggreg->value.val.dval = -val->val.lval;
            break;
   }
}

void otic_aggreg_insert_max(otic_aggreg_t* aggreg, oval_t* val)
{
    switch(val->type)
    {
        case OTIC_TYPE_INT_POS:
            if (val->val.lval > aggreg->value.val.dval)
                aggreg->value.val.dval = val->val.lval;
            break;
        case OTIC_TYPE_INT_NEG:
            if (-val->val.lval > aggreg->value.val.dval)
                aggreg->value.val.dval = -val->val.lval;
            break;
        case OTIC_TYPE_DOUBLE:
            if (val->val.dval > aggreg->value.val.dval)
                aggreg->value.val.dval = val->val.dval;
            break;
    }
}

void otic_aggreg_insert_avg(otic_aggreg_t* aggreg, oval_t* val)
{
    switch(val->type)
    {
        case OTIC_TYPE_INT_POS:
           aggreg->value.val.dval += val->val.lval;
           ++aggreg->counter;
           break;
        case OTIC_TYPE_INT_NEG:
           aggreg->value.val.dval += -val->val.lval;
           ++aggreg->counter;
           break;
        case OTIC_TYPE_DOUBLE:
           aggreg->value.val.dval += val->val.dval;
           ++aggreg->counter;
          break;
    }
}

void otic_aggreg_insert_first(otic_aggreg_t* aggreg, oval_t* val)
{
    if (aggreg->counter == 1)
        return;
    aggreg->counter = 1;
    otic_oval_cpy(&aggreg->value, val);
}

void otic_aggreg_insert_last(otic_aggreg_t* aggreg, oval_t* val)
{
    otic_oval_cpy(&aggreg->value, val);
}

void otic_aggreg_insert_sum(otic_aggreg_t* aggreg, oval_t* val)
{
    switch(val->type)
    {
        case OTIC_TYPE_INT_POS:
           aggreg->value.val.dval += val->val.lval;
           aggreg->value.type = OTIC_TYPE_DOUBLE;
           break;
        case OTIC_TYPE_INT_NEG:
           aggreg->value.val.dval += -val->val.lval;
           aggreg->value.type = OTIC_TYPE_DOUBLE;
           break;
        case OTIC_TYPE_DOUBLE:
           aggreg->value.val.dval += val->val.dval;
           aggreg->value.type = OTIC_TYPE_DOUBLE;
           break;
    }
}

// TODO: Compare performance of returning/moving complete oval_t or returning const oval_t*, with static temp values
void otic_aggreg_insert_count(otic_aggreg_t* aggreg, oval_t* val)
{
    ++aggreg->value.val.lval;
}

oval_t otic_aggreg_get_min(otic_aggreg_t* aggreg)
{
    return (oval_t){.type = (aggreg->value.val.dval == DBL_MAX ? OTIC_TYPE_NULL : OTIC_TYPE_DOUBLE), .val.dval = aggreg->value.val.dval};
}

oval_t otic_aggreg_get_max(otic_aggreg_t* aggreg)
{
    return (oval_t){.type = aggreg->value.val.dval == DBL_MIN ? OTIC_TYPE_NULL : OTIC_TYPE_DOUBLE, .val.dval = aggreg->value.val.dval};
}

oval_t otic_aggreg_get_avg(otic_aggreg_t* aggreg)
{
    return aggreg->counter == 0 ? (oval_t){.type = OTIC_TYPE_NULL} : (oval_t){.val.dval = aggreg->value.val.dval / aggreg->counter, .type = OTIC_TYPE_DOUBLE};
}

oval_t otic_aggreg_get_first(otic_aggreg_t* aggreg)
{
    return aggreg->value;
}

oval_t otic_aggreg_get_last(otic_aggreg_t* aggreg)
{
    return aggreg->value;
}

oval_t otic_aggreg_get_sum(otic_aggreg_t* aggreg)
{
    return (oval_t){.type = aggreg->value.type, .val.dval = aggreg->value.val.dval};
}

oval_t otic_aggreg_get_count(otic_aggreg_t* aggreg)
{
    return aggreg->value;
}
