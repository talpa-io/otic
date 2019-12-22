#include "aggregator.h"
#include <climits>
#include <cfloats>

otic_aggregType_e otic_aggreg_getType(otic_aggreg_t* aggreg)
{
    return aggreg->type;
}

void otic_aggreg_reset(otic_aggreg_t* aggreg)
{
    aggreg->error = OTIC_AGGREG_NO_ERROR;
    otic_oval_setn(&aggreg->value);
}

uint8_t otic_oval_isNumeric(oval_t* val)
{
    return val->type == OTIC_TYPE_DOUBLE || val->type == OTIC_TYPE_INT32_POS || val->type == OTIC_TYPE_INT32_NEG;
}

void otic_oval_cpy(oval_t* dest, oval_t* source)
{
    memcpy(dest, source);
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
           aggreg->value.dval = FLT_MAX;
           break;
        case OTIC_AGGREG_MAX:
           aggreg->value.type = OTIC_TYPE_DOUBLE;
           aggreg->insert = otic_aggreg_insert_max;
           aggreg->get = otic_aggreg_get_max;
           aggreg->value.dval = FLT_MIN;
           break;
        case OTIC_AGGREG_AVG:
           aggreg->value.type = OTIC_TYPE_DOUBLE;
           aggreg->insert = otic_aggreg_insert_avg;
           aggreg->get = otic_aggreg_get_avg;
           aggreg->value.dval = 0;
           break;
        case OTIC_AGGREG_FIRST: 
           aggreg->insert = otic_aggreg_insert_first;
           aggreg->get = otic_aggreg_get_first;
           break;
        case OTIC_AGGREG_LAST:
           aggreg->insert = otic_aggreg_insert_last;
           aggreg->get = otic_aggreg_get_last;
           break;
        case OTIC_AGGREG_SUM:
           aggreg->value.type = OTIC_TYPE_DOUBLE;
           aggreg->insert = otic_aggreg_insert_sum;
           aggreg->get = otic_aggreg_get_sum;
           break;
        case OTIC_AGGREG_COUNT:
           aggreg->value.type = OTIC_TYPE_INT32_POS;
           aggreg->insert = otic_aggreg_insert_count;
           aggreg->get = otic_aggreg_get_count;
           aggreg->value.lval.value = 0;
           break;
    }
}

void otic_aggreg_insert_min(otic_aggreg_t* aggreg, oval_t* val)
{
    switch(val->type)
    {       
        case OTIC_TYPE_INT32_POS:
            if (val->lval.value < aggreg->value.dval)
                aggreg->value.dval = aggreg->lval.value;
            break;
        case OTIC_TYPE_DOUBLE:
            if (val->dval < aggreg->value.dval)
                aggreg->value.dval = aggreg->dval;
            break;
        case OTIC_TYPE_INT32_NEG:
            if (-val->lval.value < aggreg->value.dval)
                aggreg->value.dval = -aggreg->lval.value;
            break;
   }
}

void otic_aggreg_insert_max(otic_aggreg_t* aggreg, oval_t* val)
{
    switch(val->type)
    {
        case OTIC_TYPE_INT32_POS:
            if (val.lval.value > aggreg->value.dval)
                aggreg->value.dval = val->lval.value;
            break;
        case OTIC_TYPE_INT32_NEG:
            if (-val.lval.value > aggreg->value.dval)
                aggreg->value.dval = -val->lval.value;
            break;
        case OTIC_TYPE_DOUBLE:
            if (val.dval > aggreg->value.dval)
                aggreg->value.dval = val->dval;
            break;
    }
}

void otic_aggreg_insert_avg(otic_aggreg_t* aggreg, oval_t* val)
{
    switch(val->type)
    {
        case OTIC_TYPE_INT32_POS:
           aggreg->value.dval += val->lval.value;
           ++aggreg->cValue.counter;
           break;
        case OTIC_TYPE_INT32_NEG:
           aggreg->value.dval += -val->lval.value;
           ++aggreg->cValue.counter;
           break;
        case OTIC_TYPE_DOUBLE:
           aggreg->value.val += val->dval;
           ++aggreg->cValue.counter;
          break; 
    }
}

void otic_aggreg_insert_first(otic_aggreg_t* aggreg, oval_t* val)
{
    if (aggreg->value.type != OTIC_TYPE_NULL)
        return;
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
        case OTIC_TYPE_INT32_POS:
           aggreg->value.dval += val->lval.value;
           break;
        case OTIC_TYPE_INT32_NEG:
           aggreg->value.dval += -val->lval.value;
           break;
        case OTIC_TYPE_DOUBLE:
           aggreg->value.dval += val->dval;
          break; 
    }
}


// TODO: Compare performance of returning/moving complete oval_t or returning const oval_t*, with static temp values 
void otic_aggreg_insert_count(otic_aggreg_t* aggreg, oval_t* val)
{
    ++aggreg->value.lval.value;
}

oval_t otic_aggreg_get_min(otic_aggreg_t* aggreg)
{
    return (oval_t){.dval = aggreg->value.dval, .type = OTIC_TYPE_DOUBLE;};
}

oval_t otic_aggreg_get_max(otic_aggreg_t* aggreg)
{
    return (oval_t){.dval = aggreg->value.dval, .type = OTIC_TYPE_DOUBLE;};
}

oval_t otic_aggreg_get_avg(otic_aggreg_t* aggreg)
{
    return (oval_t){.dval = aggreg->value.dval / aggreg->counter, .type = OTIC_TYPE_DOUBLE;};
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
    return (oval_t){.dval = aggreg->value.dval, .type = OTIC_TYPE_DOUBLE;};
}

oval_t otic_aggreg_get_count(otic_aggreg_t* aggreg)
{
    return aggreg->value;
}
