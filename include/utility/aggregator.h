#include "base.h"

typedef enum
{
    OTIC_AGGREG_MIN,
    OTIC_AGGREG_MAX,
    OTIC_AGGREG_AVG,
    OTIC_AGGREG_FIRST,
    OTIC_AGGREG_LAST,
    OTIC_AGGREG_SUM,
    OTIC_AGGREG_COUNT
} otic_aggregType_e;

typedef enum
{
    OTIC_AGGREG_NO_ERROR,
} otic_aggregError_e;

typedef struct
{
    aggregatorError_e error;
    aggregatorType_e type;
    oval_t value;
    size_t counter;                     // if needed
    void(insert*)(struct otic_aggreg_t*, oval*);
    oval_t(get*)(struct otic_aggreg_t*);
} otic_aggreg_t;

void otic_aggreg_init(otic_aggreg_t* aggreg, otic_aggregType_e type);
void otic_aggreg_close(otic_aggreg_t* aggreg);
otic_aggregType_e otic_aggreg_getType(otic_aggreg_t* aggreg);
void otic_aggreg_reset(otic_aggreg_t* aggreg);
uint8_t otic_oval_isNumeric(oval_t* val);
void otic_oval_cpy(oval_t* dest, oval_t* source);

void otic_aggreg_insert_min(otic_aggreg_t* aggreg, oval_t* val);
void otic_aggreg_insert_max(otic_aggreg_t* aggreg, oval_t* val);
void otic_aggreg_insert_avg(otic_aggreg_t* aggreg, oval_t* val);
void otic_aggreg_insert_first(otic_aggreg_t* aggreg, oval_t* val);
void otic_aggreg_insert_last(otic_aggreg_t* aggreg, oval_t* val);
void otic_aggreg_insert_sum(otic_aggreg_t* aggreg, oval_t* val);
void otic_aggreg_insert_count(otic_aggreg_t* aggreg, oval_t* val);

oval_t otic_aggreg_get_min(otic_aggreg_t* aggreg);
oval_t otic_aggreg_get_max(otic_aggreg_t* aggreg);
oval_t otic_aggreg_get_avg(otic_aggreg_t* aggreg);
oval_t otic_aggreg_get_first(otic_aggreg_t* aggreg);
oval_t otic_aggreg_get_last(otic_aggreg_t* aggreg);
oval_t otic_aggreg_get_sum(otic_aggreg_t* aggreg);
oval_t otic_aggreg_get_count(otic_aggreg_t* aggreg);
