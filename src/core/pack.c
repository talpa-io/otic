#include <string.h>
#include <stdlib.h>
#include <zstd.h>
#include <core/base.h>
#include <otic.h>
#include "core/pack.h"
#include "core/portability.h"

#if OTIC_PACK_INLINE_ALL_STATIC
#define OTIC_PACK_INLINE inline
#else
#define OTIC_PACK_INLINE
#endif

OTIC_PACK_INLINE PURE
static uint32_t otic_hashFunction(const char* ptr)
{
    uint32_t hash_address = 0;
    while(*ptr)
        hash_address = PTR_M * hash_address + *ptr++;
    return hash_address % OTIC_PACK_CACHE_SIZE;
}

OTIC_PACK_INLINE
static otic_entry_t* otic_pack_entry_find(otic_pack_channel_t* pack, const char* o)
{
    otic_entry_t* ptr = pack->cache[otic_hashFunction(o)];
    while (ptr)
    {
        if (strcmp(ptr->name, o) == 0)
            return ptr;
        ptr = ptr->next;
    }
    return 0;
}

OTIC_PACK_INLINE
static otic_entry_t* otic_pack_entry_insert_routine(otic_pack_channel_t* channel, const otic_str_t* o, const otic_str_t* unit, uint16_t index)
{
    uint32_t hash_address = otic_hashFunction(o->ptr);
    otic_entry_t* ptr = channel->cache[hash_address];
    while (ptr)
    {
        if (ptr->index == index)
            return ptr;
        ptr = ptr->next;
    }
    ptr = malloc(sizeof(otic_entry_t));
    if (!ptr)
        return 0;
    ptr->next = 0;
    ptr->lastValue.val.sval.ptr = 0;
    ptr->lastValue.val.sval.size = 0;
    ptr->name = malloc(o->size + unit->size + 2);
    memcpy(ptr->name, o->ptr, o->size);
    ptr->name[o->size] = 0;
    memcpy(ptr->name + o->size + 1, unit->ptr, unit->size);
    ptr->name[o->size + unit->size + 1] = 0;
    ptr->index = index;
    ptr->next = channel->cache[hash_address];
    channel->cache[hash_address] = ptr;
    return ptr;
}

OTIC_PACK_INLINE
static otic_entry_t* otic_pack_entry_insert_bool(otic_pack_channel_t* channel, const otic_str_t* o, const otic_str_t* unit, uint16_t index, uint8_t value)
{
    otic_entry_t* ptr = otic_pack_entry_insert_routine(channel, o, unit, index);
    if (!ptr)
        return 0;
    ptr->lastValue.type = value ? OTIC_TYPE_TRUE : OTIC_TYPE_FALSE;
    return ptr;
}

OTIC_PACK_INLINE
static otic_entry_t* otic_pack_entry_insert_u(otic_pack_channel_t* channel, const otic_str_t* o, const otic_str_t* unit, uint16_t index, uint32_t value)
{
    otic_entry_t* ptr = otic_pack_entry_insert_routine(channel, o, unit, index);
    if (!ptr)
        return 0;
    ptr->lastValue.type = OTIC_TYPE_INT_POS;
    ptr->lastValue.val.lval = value;
    return ptr;
}

OTIC_PACK_INLINE
static otic_entry_t* otic_pack_entry_insert_un(otic_pack_channel_t* channel, const otic_str_t* o, const otic_str_t* unit, uint16_t index, uint32_t value)
{
    otic_entry_t* ptr = otic_pack_entry_insert_routine(channel, o, unit, index);
    if (!ptr)
        return 0;
    ptr->lastValue.type = OTIC_TYPE_INT_NEG;
    ptr->lastValue.val.lval = value;
    return ptr;
}

OTIC_PACK_INLINE
static otic_entry_t* otic_pack_entry_insert_d(otic_pack_channel_t* channel, const otic_str_t* o, const otic_str_t* unit, uint16_t index, double value)
{
    otic_entry_t* ptr = otic_pack_entry_insert_routine(channel, o, unit, index);
    if (!ptr)
        return 0;
    ptr->lastValue.type = OTIC_TYPE_DOUBLE;
    ptr->lastValue.val.dval = value;
    return ptr;
}

OTIC_PACK_INLINE
static otic_entry_t* otic_pack_entry_insert_s(otic_pack_channel_t* channel, const otic_str_t* o, const otic_str_t* unit, uint16_t index, const otic_str_t* value)
{
    otic_entry_t* ptr = otic_pack_entry_insert_routine(channel, o, unit, index);
    if (!ptr)
        return 0;
    ptr->lastValue.type = OTIC_TYPE_STRING;
    ptr->lastValue.val.sval.size = value->size;
    ptr->lastValue.val.sval.ptr = malloc(ptr->lastValue.val.sval.size);
    memcpy((char*)ptr->lastValue.val.sval.ptr, value->ptr, value->size);
    return ptr;
}

OTIC_PACK_INLINE
static otic_entry_t* otic_pack_entry_insert_n(otic_pack_channel_t* channel, const otic_str_t* o, const otic_str_t* unit, uint16_t index){
    otic_entry_t* ptr = otic_pack_entry_insert_routine(channel, o, unit, index);
    if (!ptr)
        return 0;
    ptr->lastValue.type = OTIC_TYPE_NULL;
    return ptr;
}

OTIC_PACK_INLINE
static otic_entry_t* otic_pack_entry_insert_b(otic_pack_channel_t* channel, const otic_str_t* o, const otic_str_t* unit, uint16_t index)
{
    otic_entry_t* ptr = otic_pack_entry_insert_routine(channel, o, unit, index);
    if (!ptr)
        return 0;
    ptr->lastValue.type = OTIC_TYPE_RAWBUFFER;
    return ptr;
}

OTIC_PACK_INLINE
static otic_entry_t* otic_pack_entry_insert_array(otic_pack_channel_t* channel, const otic_str_t* o, const otic_str_t* unit, uint16_t index, const oval_array_t* value)
{
    otic_entry_t* ptr = otic_pack_entry_insert_routine(channel, o, unit, index);
    if (!ptr)
        return 0;
    ptr->lastValue.type = OTIC_TYPE_ARRAY;
    if (value->size != 0) {
    }
    return ptr;
}

OTIC_PACK_INLINE
static void otic_pack_write_byte(otic_pack_channel_t* channel, uint8_t value)
{
    *channel->base.top++ = value;
}

OTIC_PACK_INLINE
static void write_long(otic_pack_channel_t* channel, uint64_t value)
{
    channel->base.top += leb128_encode_unsigned(value, channel->base.top);
}

OTIC_PACK_INLINE
static void write_double(otic_pack_channel_t* channel, double value)
{
    // TODO: Assert size of float and double a la prepocessor
    memcpy(channel->base.top, (char*)&value, sizeof(double));
    channel->base.top += sizeof(double);
}

OTIC_PACK_INLINE
static void write_float(otic_pack_channel_t* channel, float value)
{
    memcpy(channel->base.top, (char*)&value, sizeof(float));
    channel->base.top += sizeof(float);
}

OTIC_PACK_INLINE
static void write_string(otic_pack_channel_t* channel, const otic_str_t* value)
{
    otic_pack_write_byte(channel, value->size);
    memcpy(channel->base.top, value->ptr, value->size);
    channel->base.top += value->size;
}

OTIC_PACK_INLINE
static void write_object(otic_pack_channel_t* channel, oval_obj_t* value);

OTIC_PACK_INLINE
static void write_array(otic_pack_channel_t* channel, const oval_array_t* value)
{
    write_long(channel, value->size);
    for (uint32_t counter = 0; counter < value->size; ++counter)
    {
        oval_t* el = &value->elements[counter];
        switch (el->type)
        {
            case OTIC_TYPE_INT_POS:
                if (el->val.lval < SMALL_INT_LIMIT)
                    otic_pack_write_byte(channel, el->val.lval);
                else {
                    otic_pack_write_byte(channel, OTIC_TYPE_INT_POS);
                    write_long(channel, el->val.lval);
                }
                break;
            case OTIC_TYPE_INT_NEG:
                otic_pack_write_byte(channel, OTIC_TYPE_INT_NEG);
                write_long(channel, el->val.lval);
                break;
            case OTIC_TYPE_DOUBLE:
                otic_pack_write_byte(channel, OTIC_TYPE_DOUBLE);
                write_double(channel, el->val.dval);
                break;
            case OTIC_TYPE_FLOAT:
                otic_pack_write_byte(channel, OTIC_TYPE_FLOAT);
                write_float(channel, (float)el->val.dval);
                break;
            case OTIC_TYPE_STRING:
                otic_pack_write_byte(channel, OTIC_TYPE_STRING);
                write_string(channel, &el->val.sval);
                break;
            case OTIC_TYPE_ARRAY:
                otic_pack_write_byte(channel, OTIC_TYPE_ARRAY);
                write_array(channel, &el->val.aval);
                break;
            case OTIC_TYPE_OBJECT:
                otic_pack_write_byte(channel, OTIC_TYPE_OBJECT);
                write_object(channel, &el->val.oval);
                break;
            case OTIC_TYPE_TRUE:
                otic_pack_write_byte(channel, OTIC_TYPE_TRUE);
                break;
            case OTIC_TYPE_FALSE:
                otic_pack_write_byte(channel, OTIC_TYPE_FALSE);
                break;
        }
    }
}

OTIC_PACK_INLINE
static void write_object(otic_pack_channel_t* channel, oval_obj_t* value)
{
    write_long(channel, value->size);
}

OTIC_PACK_INLINE
static void write_buffer(otic_pack_channel_t* channel, uint8_t* buffer, size_t size)
{
    memcpy(channel->base.top, (char*)&size, sizeof(size));
    channel->base.top += sizeof(size);
    memcpy(channel->base.top, buffer, size);
    channel->base.top += size;
}

OTIC_PACK_INLINE
static void otic_pack_id_assign(otic_pack_channel_t* channel, const otic_str_t* sensorName, const otic_str_t* unit)
{
    otic_pack_write_byte(channel, OTIC_TYPE_NAME_ASSIGN);
    otic_pack_write_byte(channel, sensorName->size + unit->size + 1);
    memcpy(channel->base.top, sensorName->ptr, sensorName->size);
    channel->base.top += sensorName->size;
    otic_pack_write_byte(channel, 0);
    memcpy(channel->base.top, unit->ptr, unit->size);
    channel->base.top += unit->size;
#if OTIC_STATS
    ++channel->stats.cols_assigned;
#endif
}

OTIC_PACK_INLINE
static void otic_pack_flush_if_flushable(otic_pack_channel_t* channel)
{
    if (channel->base.top > channel->threshold)
        otic_pack_channel_flush(channel);
}

OTIC_PACK_INLINE
static uint8_t otic_ts_handler(otic_pack_channel_t* channel, double ts)
{
#if ENABLE_FULL_CHECKS
    if (ts < 0)
        return 0;
#endif
    uint64_t intTs = (uint64_t)(ts * OTIC_TS_MULTIPLICATOR);
    if (channel->base.timestampCurrent == intTs) {
        return 1;
    } else if (channel->base.timestampCurrent == TS_NULL) {
        channel->base.timestampStart = channel->base.timestampCurrent = intTs;
        otic_pack_write_byte(channel, OTIC_TYPE_SET_TIMESTAMP);
        write_long(channel, intTs);
        if (channel->timeInterval.time_start == TS_NULL)
            channel->timeInterval.time_start = ts;
#if OTIC_STATS
        ++channel->stats.time_sets;
#endif
    } else {
        if (intTs < channel->base.timestampCurrent) {
            otic_base_setError(&channel->base, OTIC_ERROR_INVALID_TIMESTAMP);
            return 0;
        }
        otic_pack_write_byte(channel, OTIC_TYPE_SHIFT_TIMESTAMP);
        write_long(channel, intTs - channel->base.timestampCurrent);
        channel->base.timestampCurrent = intTs;
        channel->timeInterval.time_end = ts;
#if OTIC_STATS
      ++channel->stats.time_shifts;
#endif
    }
    otic_pack_flush_if_flushable(channel);
    return 1;
}

OTIC_PACK_INLINE
static void otic_pack_setError(otic_pack_t* restrict oticPack, otic_error_e error)
{
    oticPack->error = error;
}

OTIC_PACK_INLINE
static otic_error_e otic_pack_getError(otic_pack_t* restrict oticPack)
{
    return oticPack->error;
}

OTIC_PACK_INLINE
static void otic_pack_setState(otic_pack_t* restrict oticPack, otic_state_e state)
{
    oticPack->state = state;
}

OTIC_PACK_INLINE
static otic_state_e otic_pack_getState(otic_pack_t* restrict oticPack)
{
    return oticPack->state;
}

OTIC_PACK_INLINE
static void otic_pack_releaser(oval_t* val)
{
    switch (val->type)
    {
        case OTIC_TYPE_STRING:
            free(val->val.sval.ptr);
            val->val.sval.ptr = 0;
            break;
        case OTIC_TYPE_ARRAY:
            return free(val->val.aval.elements);
        case OTIC_TYPE_OBJECT:
            return free(val->val.oval.elements);
    }
}

uint8_t otic_pack_channel_inject_bool(otic_pack_channel_t* channel, double timestamp, const char* sensorName, const char* unit, BOOL_UINT8 value)
{
    if (!otic_ts_handler(channel, timestamp))
        goto fail;
    otic_entry_t *entry = otic_pack_entry_find(channel, sensorName);
    otic_type_e type = value ? OTIC_TYPE_TRUE : OTIC_TYPE_FALSE;
    if (!entry){
        otic_str_t s = {(char*)sensorName, sensorName? strlen(sensorName) : 0}, u = {(char*)unit, unit? strlen(unit): 0};
        if (s.size + u.size + 1 > UINT8_MAX){
            otic_base_setError(&channel->base, OTIC_ERROR_BUFFER_OVERFLOW);
            goto fail;
        }
        if (!otic_pack_entry_insert_bool(channel, &s, &u, channel->totalEntries, value)){
            otic_base_setError(&channel->base, OTIC_ERROR_ENTRY_INSERTION_FAILURE);
            goto fail;
        }
        otic_pack_id_assign(channel, &s, &u);
        otic_pack_write_byte(channel, type);
        write_long(channel, channel->totalEntries++);
    } else{
        if (entry->lastValue.type == type){
            otic_pack_write_byte(channel, OTIC_TYPE_UNMODIFIED);
            write_long(channel, entry->index);
#if OTIC_STATS
            ++channel->stats.type_unmodified;
#endif
        } else {
            otic_pack_releaser(&entry->lastValue);
            entry->lastValue.type = type;
            otic_pack_write_byte(channel, type);
            write_long(channel, entry->index);
        }
    }
    ++channel->base.rowCounter;
#if OTIC_STATS
    ++channel->stats.type_bool;
#endif
    otic_pack_flush_if_flushable(channel);
    return 1;
fail:
    otic_base_setState(&channel->base, OTIC_STATE_CLOSED);
    return 0;
}


uint8_t otic_pack_channel_inject_i_neg(otic_pack_channel_t* channel, double timestamp, const char *sensorName, const char *unit, uint64_t value) {
    if (!otic_ts_handler(channel, timestamp))
        goto fail;
    otic_entry_t *entry = otic_pack_entry_find(channel, sensorName);
    if (!entry){
        otic_str_t s = {(char*)sensorName, sensorName? strlen(sensorName) : 0}, u = {(char*)unit, unit? strlen(unit): 0};
        if (s.size + u.size + 1 > UINT8_MAX){
            otic_base_setError(&channel->base, OTIC_ERROR_BUFFER_OVERFLOW);
            goto fail;
        }
        if (!otic_pack_entry_insert_un(channel, &s, &u, channel->totalEntries, value)){
            otic_base_setError(&channel->base, OTIC_ERROR_ENTRY_INSERTION_FAILURE);
            goto fail;
        }
        otic_pack_id_assign(channel, &s, &u);
        otic_pack_write_byte(channel, OTIC_TYPE_INT_NEG);
        write_long(channel, channel->totalEntries);
        write_long(channel, value);
        ++channel->totalEntries;
    } else{
        if (entry->lastValue.type == OTIC_TYPE_INT_NEG && entry->lastValue.val.lval == value){
            otic_pack_write_byte(channel, OTIC_TYPE_UNMODIFIED);
            write_long(channel, entry->index);
#if OTIC_STATS
            ++channel->stats.type_unmodified;
#endif
        } else {
            otic_pack_releaser(&entry->lastValue);
            entry->lastValue.val.lval = value;
            entry->lastValue.type = OTIC_TYPE_INT_NEG;
            otic_pack_write_byte(channel, OTIC_TYPE_INT_NEG);
            write_long(channel, entry->index);
            write_long(channel, value);
        }
    }
    ++channel->base.rowCounter;
#if OTIC_STATS
    ++channel->stats.type_integer;
#endif
    otic_pack_flush_if_flushable(channel);
    return 1u;
fail:
    otic_base_setState(&channel->base, OTIC_STATE_CLOSED);
    return 0;
}

uint8_t otic_pack_channel_inject_i(otic_pack_channel_t* channel, double timestamp, const char *sensorName, const char *sensorUnit, uint64_t value) {
    if (!otic_ts_handler(channel, timestamp))
        goto fail;
    otic_entry_t *entry = otic_pack_entry_find(channel, sensorName);
    if (!entry){
        otic_str_t s = {(char*)sensorName, sensorName? strlen(sensorName) : 0}, u = {(char*)sensorUnit, sensorUnit? strlen(sensorUnit): 0};
        if (s.size + u.size + 1 > UINT8_MAX){
            otic_base_setError(&channel->base, OTIC_ERROR_BUFFER_OVERFLOW);
            goto fail;
        }
        if (!otic_pack_entry_insert_u(channel, &s, &u, channel->totalEntries, value)) {
            otic_base_setError(&channel->base, OTIC_ERROR_ENTRY_INSERTION_FAILURE);
            goto fail;
        }
        otic_pack_id_assign(channel, &s, &u);
        if (value < SMALL_INT_LIMIT) {
            otic_pack_write_byte(channel, value);
            write_long(channel, channel->totalEntries);
        }
        else {
            otic_pack_write_byte(channel, OTIC_TYPE_INT_POS);
            write_long(channel, channel->totalEntries);
            write_long(channel, value);
        }
        ++channel->totalEntries;
    } else{
        if (entry->lastValue.type == OTIC_TYPE_INT_POS && entry->lastValue.val.lval == value){
            otic_pack_write_byte(channel, OTIC_TYPE_UNMODIFIED);
            write_long(channel, entry->index);
#if OTIC_STATS
            ++channel->stats.type_unmodified;
#endif
        } else if (value < SMALL_INT_LIMIT) {
            otic_pack_write_byte(channel, value);
            write_long(channel, entry->index);
            entry->lastValue.val.lval = value;
            entry->lastValue.type = OTIC_TYPE_INT_POS;
        } else {
            otic_pack_releaser(&entry->lastValue);
            entry->lastValue.val.lval = value;
            entry->lastValue.type = OTIC_TYPE_INT_POS;
            otic_pack_write_byte(channel, OTIC_TYPE_INT_POS);
            write_long(channel, entry->index);
            write_long(channel, value);
        }
    }
    ++channel->base.rowCounter;
#if OTIC_STATS
    ++channel->stats.type_integer;
#endif
    otic_pack_flush_if_flushable(channel);
    return 1u;
fail:
    otic_base_setState(&channel->base, OTIC_STATE_ON_ERROR);
    return 0;
}

uint8_t otic_pack_channel_inject_d(otic_pack_channel_t* channel, double timestamp, const char* sensorName, const char* unit, double value)
{
    if (!otic_ts_handler(channel, timestamp))
        goto fail;
    otic_entry_t* entry = otic_pack_entry_find(channel, sensorName);
    if (!entry) {
        otic_str_t s = {(char*)sensorName, sensorName? strlen(sensorName) : 0}, u = {(char*)unit, unit? strlen(unit): 0};
        if (s.size + u.size + 1 > UINT8_MAX){
            otic_base_setError(&channel->base, OTIC_ERROR_BUFFER_OVERFLOW);
            goto fail;
        }
        otic_pack_entry_insert_d(channel, &s, &u, channel->totalEntries, value);
        otic_pack_id_assign(channel, &s, &u);
        otic_pack_write_byte(channel, OTIC_TYPE_DOUBLE);
        write_long(channel, channel->totalEntries);
        write_double(channel, value);
        ++channel->totalEntries;
    } else {
        if (entry->lastValue.type == OTIC_TYPE_DOUBLE && entry->lastValue.val.dval == value) {
            otic_pack_write_byte(channel, OTIC_TYPE_UNMODIFIED);
            write_long(channel, entry->index);
#if OTIC_STATS
            ++channel->stats.type_unmodified;
#endif
        } else {
            otic_pack_releaser(&entry->lastValue);
            entry->lastValue.val.dval = value;
            entry->lastValue.type = OTIC_TYPE_DOUBLE;
            otic_pack_write_byte(channel, OTIC_TYPE_DOUBLE);
            write_long(channel, entry->index);
            write_double(channel, value);
        }
    }
    ++channel->base.rowCounter;
#if OTIC_STATS
    ++channel->stats.type_double;
#endif
    otic_pack_flush_if_flushable(channel);
    return 1u;
fail:
    otic_base_setState(&channel->base, OTIC_STATE_ON_ERROR);
    return 0;
}

uint8_t otic_pack_channel_inject_s(otic_pack_channel_t* channel, double timestamp, const char* sensorName, const char* unit, const char* value)
{
    if (!otic_ts_handler(channel, timestamp))
        goto fail;
    otic_entry_t* entry = otic_pack_entry_find(channel, sensorName);
    otic_str_t v = {(char*)value, value ? strlen(value) : 0};
    if (v.size > UINT8_MAX){
        otic_base_setError(&channel->base, OTIC_ERROR_BUFFER_OVERFLOW);
        goto fail;
    }
    if (!entry) {
        otic_str_t s = {(char*)sensorName, sensorName? strlen(sensorName) : 0}, u = {(char*)unit, unit? strlen(unit): 0};
        if (s.size + u.size + 1 > UINT8_MAX){
            otic_base_setError(&channel->base, OTIC_ERROR_BUFFER_OVERFLOW);
            goto fail;
        }
        otic_pack_entry_insert_s(channel, &s, &u, channel->totalEntries, &v);
        otic_pack_id_assign(channel, &s, &u);
        otic_pack_write_byte(channel, OTIC_TYPE_STRING);
        write_long(channel, channel->totalEntries);
        write_string(channel, &v);
        ++channel->totalEntries;
    } else {
        if (entry->lastValue.type == OTIC_TYPE_STRING && strcmp(entry->lastValue.val.sval.ptr, value) == 0) {
            otic_pack_write_byte(channel, OTIC_TYPE_UNMODIFIED);
            write_long(channel, entry->index);
#if OTIC_STATS
            ++channel->stats.type_unmodified;
#endif
        } else {
            otic_pack_releaser(&entry->lastValue);
            entry->lastValue.val.sval.ptr = malloc(v.size + 1);
            entry->lastValue.val.sval.ptr[v.size] = 0;
            entry->lastValue.val.sval.size = v.size;
            strncpy(entry->lastValue.val.sval.ptr, value, v.size);
            otic_pack_write_byte(channel, OTIC_TYPE_STRING);
            write_long(channel, entry->index);
            write_string(channel, &v);
            entry->lastValue.type = OTIC_TYPE_STRING;
        }
    }
    ++channel->base.rowCounter;
#if OTIC_STATS
    ++channel->stats.type_string;
#endif
    otic_pack_flush_if_flushable(channel);
    return 1u;
fail:
    otic_base_setState(&channel->base, OTIC_STATE_ON_ERROR);
    return 0;
}

uint8_t otic_pack_channel_inject_n(otic_pack_channel_t* channel, double timestamp, const char* sensorName, const char* unit)
{
    if (!otic_ts_handler(channel, timestamp))
        goto fail;
    otic_entry_t* entry = otic_pack_entry_find(channel, sensorName);
    if (!entry) {
        otic_str_t s = {(char*)sensorName, sensorName? strlen(sensorName) : 0}, u = {(char*)unit, unit? strlen(unit): 0};
        if (s.size + u.size + 1 > UINT8_MAX) {
            otic_base_setError(&channel->base, OTIC_ERROR_BUFFER_OVERFLOW);
            goto fail;
        }
        if (!otic_pack_entry_insert_n(channel, &s, &u, channel->totalEntries)) {
            otic_base_setError(&channel->base, OTIC_ERROR_ENTRY_INSERTION_FAILURE);
            goto fail;
        }
        otic_pack_entry_insert_n(channel, &s, &u, channel->totalEntries);
        otic_pack_id_assign(channel, &s, &u);
        otic_pack_write_byte(channel, OTIC_TYPE_NULL);
        write_long(channel, channel->totalEntries);
        ++channel->totalEntries;
    } else{
        if (entry->lastValue.type == OTIC_TYPE_NULL) {
            otic_pack_write_byte(channel, OTIC_TYPE_UNMODIFIED);
            write_long(channel, entry->index);
#if OTIC_STATS
            ++channel->stats.type_unmodified;
#endif
        } else {
            otic_pack_releaser(&entry->lastValue);
            entry->lastValue.type = OTIC_TYPE_NULL;
            otic_pack_write_byte(channel, OTIC_TYPE_NULL);
            write_long(channel, entry->index);
        }
    }
    ++channel->base.rowCounter;
#if OTIC_STATS
    ++channel->stats.type_null;
#endif
    otic_pack_flush_if_flushable(channel);
    return 1;
fail:
    otic_base_setState(&channel->base, OTIC_STATE_CLOSED);
    return 0;
}

uint8_t otic_pack_channel_inject_b(otic_pack_channel_t* channel, double timestamp, const char* sensorName, const char* unit, uint8_t* buffer, size_t size)
{
    if (!otic_ts_handler(channel, timestamp))
        goto fail;
    otic_entry_t* entry = otic_pack_entry_find(channel, sensorName);
    if (!entry){
        otic_str_t s = {(char*)sensorName, sensorName? strlen(sensorName) : 0}, u = {(char*)unit, unit? strlen(unit): 0};
        if (s.size + u.size + 1 > UINT8_MAX) {
            otic_base_setError(&channel->base, OTIC_ERROR_BUFFER_OVERFLOW);
            goto fail;
        }
        if (!otic_pack_entry_insert_b(channel, &s, &u, channel->totalEntries)){
            otic_base_setError(&channel->base, OTIC_ERROR_ENTRY_INSERTION_FAILURE);
            goto fail;
        }
        otic_pack_id_assign(channel, &s, &u);
        otic_pack_write_byte(channel, OTIC_TYPE_RAWBUFFER);
        write_long(channel, channel->totalEntries);
        ++channel->totalEntries;
    } else {
        otic_pack_releaser(&entry->lastValue);
        entry->lastValue.type = OTIC_TYPE_RAWBUFFER;
        otic_pack_write_byte(channel, OTIC_TYPE_RAWBUFFER);
        write_long(channel, entry->index);
    }
    write_buffer(channel, buffer, size);
    ++channel->base.rowCounter;
    otic_pack_flush_if_flushable(channel);
    return 1u;
fail:
    otic_base_setState(&channel->base, OTIC_STATE_ON_ERROR);
    return 0;
}

uint8_t otic_pack_channel_inject_array(otic_pack_channel_t* channel, double timestamp, const char* sensorName, const char* unit, const oval_array_t* v)
{
    if (!otic_ts_handler(channel, timestamp))
        goto fail;
    otic_entry_t* entry = otic_pack_entry_find(channel, sensorName);
    if (!entry) {
        otic_str_t s = {(char*)sensorName, sensorName? strlen(sensorName) : 0}, u = {(char*)unit, unit? strlen(unit): 0};
        if (s.size + u.size + 1 > UINT8_MAX){
            otic_base_setError(&channel->base, OTIC_ERROR_BUFFER_OVERFLOW);
            goto fail;
        }
        otic_pack_entry_insert_array(channel, &s, &u, channel->totalEntries, v);
        otic_pack_id_assign(channel, &s, &u);
        otic_pack_write_byte(channel, OTIC_TYPE_ARRAY);
        write_long(channel, channel->totalEntries);
        write_array(channel, v);
        ++channel->totalEntries;
    } else {
        if (entry->lastValue.type == OTIC_TYPE_ARRAY && oval_array_cmp(&entry->lastValue.val.aval, v)) {
            otic_pack_write_byte(channel, OTIC_TYPE_UNMODIFIED);
            write_long(channel, entry->index);
#if OTIC_STATS
            ++channel->stats.type_unmodified;
#endif
        } else {
            otic_pack_releaser(&entry->lastValue);
            entry->lastValue.type = OTIC_TYPE_ARRAY;
            entry->lastValue.val.aval.size = v->size;
            otic_pack_write_byte(channel, OTIC_TYPE_ARRAY);
            write_long(channel, entry->index);
            write_array(channel, v);
        }
    }
    ++channel->base.rowCounter;
#if OTIC_STATS
    ++channel->stats.type_array;
#endif
    return 1;
fail:
    otic_base_setState(&channel->base, OTIC_STATE_ON_ERROR);
    return 0;
}

uint8_t otic_pack_channel_init(otic_pack_channel_t* channel, uint8_t id, channel_type_e channelType, otic_pack_t* parent, uint32_t bucketSize)
{
    channel->ztd_out = malloc(bucketSize);
    if (!channel->ztd_out) {
        otic_pack_setError(channel->info.parent, OTIC_ERROR_ALLOCATION_FAILURE);
        goto fail;
    }
    if (!otic_base_init(&channel->base, bucketSize))
        goto fail;
    channel->cCtx = ZSTD_createCCtx();
    if (!channel->cCtx){
        otic_base_setError(&channel->base, OTIC_ERROR_ZSTD);
        goto fail;
    }
    size_t counter;
    for (counter = 0; counter < OTIC_PACK_CACHE_SIZE; counter++)
        channel->cache[counter] = 0;
    channel->totalEntries = 0;
    channel->threshold = &channel->base.cache[bucketSize - OTIC_PACK_CACHE_TOP_LIMIT];
    channel->info.channelId = id;
    channel->info.channelType = channelType;
    channel->info.parent = parent;
    channel->timeInterval.time_start = TS_NULL;
#if OTIC_STATS
    memset(&channel->stats, 0, sizeof(channel->stats));
#endif
    return 1;
fail:
    free(channel->ztd_out);
    otic_base_close(&channel->base);
    otic_base_setState(&channel->base, OTIC_STATE_ON_ERROR);
    return 0;
}

uint8_t otic_pack_channel_close(otic_pack_channel_t* channel)
{
    otic_pack_write_byte(channel, OTIC_TYPE_EOF);
    write_long(channel, channel->base.rowCounter);
    otic_pack_channel_flush(channel);
    ZSTD_freeCCtx(channel->cCtx);
    otic_entry_t* temp = 0, *next = 0;
    for (size_t counter = 0; counter < OTIC_PACK_CACHE_SIZE; counter++)
    {
        next = channel->cache[counter];
        while (next)
        {
            free(next->name);
            otic_pack_releaser(&next->lastValue);
            temp = next->next;
            free(next);
           next = temp;
        }
    }
    otic_pack_channel_t* current = channel->info.parent->channels;
    otic_pack_channel_t* before = current;
    while (current)
    {
        if (current == channel) {
            if (current == channel->info.parent->channels)
                channel->info.parent->channels = current->previous;
            else
                before->previous = current->previous;
            otic_base_close(&channel->base);
            free(channel->ztd_out);
            free(channel);
            return 1;
        }
        before = current;
        current = current->previous;
    }
    return 0;
}

uint8_t otic_pack_channel_flush(otic_pack_channel_t* channel)
{
#if OTIC_PACK_NO_COMPRESSION
    otic_payload_t payload = {.dataLen = channel->base.top - channel->base.cache, .channelId = channel->info.channelId};
    if (!channel->info.parent->flusher((uint8_t*)&payload, sizeof(payload), channel->info.parent->data)){
        otic_base_setError(&channel->base, OTIC_ERROR_FLUSH_FAILED);
        goto fail;
    }
    if (!channel->info.parent->flusher(channel->base.cache, channel->base.top - channel->base.cache, channel->info.parent->data)){
        otic_base_setError(&channel->base, OTIC_ERROR_FLUSH_FAILED);
        goto fail;
    }
#else

    size_t ret = ZSTD_compressCCtx(
            channel->cCtx,
            channel->ztd_out,
            channel->base.cacheSize,
            channel->base.cache,
            channel->base.top - channel->base.cache,
            OTIC_ZSTD_COMPRESSION_LEVEL
            );

    if (ZSTD_isError(ret)) {
        otic_base_setError(&channel->base, OTIC_ERROR_ZSTD);
        goto fail;
    }
    if (ret > UINT32_MAX) {
        otic_pack_setError(channel->info.parent, OTIC_ERROR_BUFFER_OVERFLOW);
        goto fail;
    }
    otic_meta_data_size_t meta = {.meta = {.channelId = channel->info.channelId, .metaType = OTIC_META_TYPE_DATA}, .size = ret};
    if (!channel->info.parent->flusher((uint8_t*)&meta, sizeof(otic_meta_data_size_t), channel->info.parent->data)) {
        otic_base_setError(&channel->base, OTIC_ERROR_FLUSH_FAILED);
        goto fail;
    }
    if (!channel->info.parent->flusher(channel->ztd_out, ret, channel->info.parent->data)) {
        otic_base_setError(&channel->base, OTIC_ERROR_FLUSH_FAILED);
        goto fail;
    }
#endif
    channel->base.top = channel->base.cache;
#if OTIC_STATS
    ++channel->stats.blocksWritten;
#endif
    return 1;
fail:
    otic_base_setState(&channel->base, OTIC_STATE_ON_ERROR);
    otic_pack_setState(channel->info.parent, OTIC_STATE_ON_ERROR);
    return 0;
}

uint8_t otic_pack_channel_resizeBucket(otic_pack_channel_t* channel, uint32_t bucketSize)
{
    if (otic_base_getState(&channel->base) != OTIC_STATE_OPENED) {
        otic_pack_setError(channel->info.parent, OTIC_ERROR_AT_INVALID_STATE);
        goto fail;
    }
    if (bucketSize == 0)
        bucketSize = OTIC_BASE_CACHE_SIZE;
    if (!((channel->base.top - channel->base.cache < bucketSize) && bucketSize > 1024)) {
        otic_pack_setError(channel->info.parent, OTIC_ERROR_ALLOCATION_FAILURE);
        goto fail;
    }
    uint8_t* ptr = realloc(channel->base.cache, bucketSize);
    if (!ptr) {
        otic_pack_setError(channel->info.parent, OTIC_ERROR_ALLOCATION_FAILURE);
        return 0;
    }
    channel->base.cacheSize = bucketSize;
    channel->base.cache = ptr;
    channel->threshold = channel->base.cache + bucketSize - OTIC_PACK_CACHE_TOP_LIMIT;
    channel->info.parent->flusher((uint8_t*)&(otic_meta_data_size_t){.meta = {OTIC_META_TYPE_CHUNK_SIZE, channel->info.channelId}, bucketSize},
                                  sizeof(otic_meta_data_size_t), channel->info.parent->data);
    return 1;
fail:
    otic_pack_setState(channel->info.parent, OTIC_STATE_ON_ERROR);
    return 0;
}

uint8_t otic_pack_init(otic_pack_t* oticPack, uint8_t features, uint8_t(*flusher)(uint8_t*, size_t, void*), void* data)
{
    if (!flusher) {
        otic_pack_setError(oticPack, OTIC_ERROR_INVALID_POINTER);
        goto fail;
    }
    oticPack->flusher = flusher;
    oticPack->data = data;
    oticPack->channels = 0;
    flusher((uint8_t*)&(otic_header_t){.magic = OTIC_MAGIC, .features=features, .version = OTIC_VERSION_MAJOR}, sizeof(otic_header_t), data);
    otic_pack_setState(oticPack, OTIC_STATE_OPENED);
    otic_pack_setError(oticPack, OTIC_ERROR_NONE);
    return 1;
fail:
    otic_pack_setState(oticPack, OTIC_STATE_ON_ERROR);
    return 0;
}

/**
 * @param oticPack
 * @param channelType
 * @param id
 * @param features
 * @warning The result of this function needs to be used! \a __attribute__((warn_unused_result)) is specified to warn the
 * developer if this shouldn't be the case
 * @attention The null-channel (id = 0) is the file itself, and is therefore reserved for file-wide metadata.
 * @return A Pointer to the new channel in case of successful allocation, else a nullptr in case of failure
 * Reasons for failure could be an invalid or already occupied id, allocation/reallocation failure or an invalid
 * pointer to \a oticPack
 */
MUST_CHECK
otic_pack_channel_t* otic_pack_defineChannel(otic_pack_t* oticPack, channel_type_e channelType, uint8_t id, otic_feature_e features, uint32_t bufferSize) {
    if (otic_pack_getState(oticPack) != OTIC_STATE_OPENED) {
        otic_pack_setError(oticPack, OTIC_ERROR_AT_INVALID_STATE);
        goto fail;
    }
    if (!id) {
        otic_pack_setError(oticPack, OTIC_ERROR_INVALID_ARGUMENT);
        goto fail;
    }
    otic_pack_channel_t * current = oticPack->channels;
    while (current)
    {
        if (current->info.channelId == id) {
            otic_pack_setError(oticPack, OTIC_ERROR_INVALID_ARGUMENT);
            goto fail;
        }
        current = current->previous;
    }
    otic_pack_channel_t* temp = malloc(sizeof(otic_pack_channel_t));
    if (!temp) {
        otic_pack_setError(oticPack, OTIC_ERROR_ALLOCATION_FAILURE);
        goto fail;
    }
    if (!bufferSize)
        bufferSize = OTIC_BASE_CACHE_SIZE;
    if (!otic_pack_channel_init(temp, id, channelType, oticPack, bufferSize)) {
        free(temp);
        goto fail;
    }
    temp->previous = oticPack->channels;
    oticPack->channels = temp;
    // Very ugly but cheap!
    uint8_t buffer[14] =
            {OTIC_META_TYPE_CHANNEL_DEFINE, id,
             OTIC_META_TYPE_CHANNEL_TYPE, id, channelType,
             OTIC_META_TYPE_COMPRESSION_METHOD, id, OTIC_FEATURE_COMPRESSION_ZSTD,
             OTIC_META_TYPE_CHUNK_SIZE, id};
    memcpy(buffer + 10, (uint8_t*)&bufferSize, sizeof(uint32_t));
    oticPack->flusher(buffer, sizeof(buffer), oticPack->data);
    return oticPack->channels;
fail:
    otic_pack_setState(oticPack, OTIC_STATE_ON_ERROR);
    return 0;
}

uint8_t otic_pack_closeChannel(otic_pack_t* oticPack, uint8_t id)
{
    otic_pack_channel_t* previous = oticPack->channels;
    otic_pack_channel_t* before;
    while (previous)
    {
        before = previous;
        if (previous->info.channelId == id) {
            if (previous == oticPack->channels) {
                oticPack->channels = previous->previous;
            } else {
                before->previous = previous->previous;
            }
            free(previous);
        }
        previous = previous->previous;
    }
    otic_pack_setError(oticPack, OTIC_ERROR_INVALID_ARGUMENT);
    otic_pack_setState(oticPack, OTIC_STATE_ON_ERROR);
    return 0;
}

uint8_t otic_pack_getTotalAmountOfChannel(otic_pack_t* oticPack)
{
    uint8_t counter = 0;
    for (otic_pack_channel_t* current = oticPack->channels; current != 0; current = current->previous, ++counter);
    return counter;
}

uint8_t otic_pack_flush(otic_pack_t* oticPack)
{
    otic_payload_t payload;
#if OTIC_PACK_NO_COMPRESSION
    for (counter = 0; counter < oticPack->totalChannels; counter++)
    {
        payload.channelId = oticPack->channels[counter]->info.channelId;
        payload.dataLen = oticPack->channels[counter]->base.top - oticPack->channels[counter]->base.cache;
        if (!oticPack->flusher((uint8_t*)&payload, sizeof(otic_payload_t), oticPack->data)){
            otic_base_setError(&oticPack->channels[counter]->base, OTIC_ERROR_FLUSH_FAILED);
            goto fail;
        }
        if (!oticPack->flusher(oticPack->channels[counter]->base.cache, oticPack->channels[counter]->base.top - oticPack->channels[counter]->base.cache, oticPack->data)){
            otic_base_setError(&oticPack->channels[counter]->base, OTIC_ERROR_FLUSH_FAILED);
            goto fail;
        }
        oticPack->channels[counter]->base.top = oticPack->channels[counter]->base.cache;
    }
#else
    size_t ret;
    otic_pack_channel_t* current = oticPack->channels;
    while (current)
    {
        ret = ZSTD_compressCCtx(
                current->cCtx, current->ztd_out, current->base.cacheSize,
                current->base.cache,
                current->base.top - current->base.cache, OTIC_ZSTD_COMPRESSION_LEVEL
                );
        if (ZSTD_isError(ret)){
            otic_base_setError(&current->base, OTIC_ERROR_ZSTD);
            goto fail;
        }
        payload.channelId = current->info.channelId;
        payload.dataLen = ret;
        if (!oticPack->flusher((uint8_t*)&payload, sizeof(otic_payload_t), oticPack->data)){
            otic_base_setError(&current->base, OTIC_ERROR_FLUSH_FAILED);
            goto fail;
        }
        if (!oticPack->flusher(current->ztd_out, ret, oticPack->data)){
            otic_base_setError(&current->base, OTIC_ERROR_FLUSH_FAILED);
            goto fail;
        }
        current->base.top = current->base.cache;
#if OTIC_STATS
        ++current->stats.blocksWritten;
#endif
        current = current->previous;
    }
#endif
    return 1;
fail:
    otic_pack_setState(oticPack, OTIC_STATE_ON_ERROR);
    return 0;
}

void otic_pack_clearErrorFlag(otic_pack_t* oticPack)
{
    oticPack->error = OTIC_ERROR_NONE;
    oticPack->state = OTIC_STATE_OPENED;
}

uint8_t otic_pack_close(otic_pack_t* oticPack) {
    otic_pack_channel_t* current = oticPack->channels;
    while (current) {
        otic_pack_channel_t* temp = current->previous;
        if (!otic_pack_channel_close(current))
            goto fail;
        current = temp;
    }
    otic_pack_setState(oticPack, OTIC_STATE_CLOSED);
    return 1;
fail:
    otic_pack_setState(oticPack, OTIC_STATE_ON_ERROR);
    return 0;
}
