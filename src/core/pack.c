#include <string.h>
#include <stdlib.h>
#include <zstd.h>
#include <core/base.h>
#include "core/pack.h"

#if OTIC_PACK_INLINE_ALL_STATIC
#define OTIC_PACK_INLINE inline
#else
#define OTIC_PACK_INLINE
#endif


OTIC_PACK_INLINE
__attribute_pure__ static uint32_t otic_hashFunction(const char* ptr)
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
        if (strncmp(ptr->name, o, strlen(o)) == 0)
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
    ptr->name = malloc(o->size + unit->size + 1);
    memcpy(ptr->name, o->ptr, o->size);
    ptr->name[o->size] = ':';
    memcpy(ptr->name + o->size + 1, unit->ptr, unit->size);
    ptr->name[o->size + unit->size] = 0;
    ptr->index = index;
    ptr->next = channel->cache[hash_address];
    channel->cache[hash_address] = ptr;
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
static otic_entry_t* otic_pack_entry_insert_a(otic_pack_channel_t* channel, const otic_str_t* o, const otic_str_t* unit, uint16_t index, const oval_array_t* value)
{
    otic_entry_t* ptr = otic_pack_entry_insert_routine(channel, o, unit, index);
    if (!ptr)
        return 0;
    ptr->lastValue.type = OTIC_TYPE_ARRAY;
    if (value->size != 0) {
        ptr->aHolder = malloc(value->size * sizeof(oval_t));
        memcpy(ptr->aHolder, value->elements, value->size * sizeof(oval_t));
    }
    ptr->lastValue.val.aval.elements = ptr->aHolder;
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
                if (el->val.lval < 0xC9)
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
    otic_pack_write_byte(channel, ':');
    memcpy(channel->base.top, unit->ptr, unit->size);
    channel->base.top += unit->size;
}

OTIC_PACK_INLINE
static uint8_t otic_ts_handler(otic_pack_channel_t* channel, double ts)
{
    uint64_t intTs = (uint64_t)(ts * OTIC_TS_MULTIPLICATOR);
    if (channel->base.timestamp_current == intTs) {
        return 1;
    } else if (channel->base.timestamp_current == 0) {
        channel->base.timestamp_start = channel->base.timestamp_current = intTs;
        otic_pack_write_byte(channel, OTIC_TYPE_SET_TIMESTAMP);
        write_long(channel, intTs);
        if (channel->timeInterval.time_start == 0)
            channel->timeInterval.time_start = ts;
    } else {
        if (intTs < channel->base.timestamp_current) {
            otic_base_setError(&channel->base, OTIC_ERROR_INVALID_TIMESTAMP);
            return 0;
        }
        otic_pack_write_byte(channel, OTIC_TYPE_SHIFT_TIMESTAMP);
        write_long(channel, intTs - channel->base.timestamp_current);
        channel->base.timestamp_current = intTs;
        channel->timeInterval.time_end = ts;
    }
    return 1;
}

OTIC_PACK_INLINE
static void otic_pack_flush_if_flushable(otic_pack_channel_t* channel)
{
    if (channel->base.top > channel->threshold)
        otic_pack_channel_flush(channel);
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

uint8_t otic_pack_channel_inject_i_neg(otic_pack_channel_t* channel, double timestamp, const char *sensorName, const char *unit, uint64_t value) {
    if (!otic_ts_handler(channel, timestamp))
        goto fail;
    otic_entry_t *entry = otic_pack_entry_find(channel, sensorName);
    if (!entry){
        otic_str_t s = {(char*)sensorName, sensorName? strlen(sensorName) : 0}, u = {(char*)unit, unit? strlen(unit): 0};
        if (s.size + u.size > UINT8_MAX){
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
        } else {
            if (entry->lastValue.val.sval.size != 0) {
                free((void *)entry->lastValue.val.sval.ptr);
                entry->lastValue.val.sval.ptr = 0;
                entry->lastValue.val.sval.size = 0;
            }
            entry->lastValue.val.lval = value;
            entry->lastValue.type = OTIC_TYPE_INT_NEG;
            otic_pack_write_byte(channel, OTIC_TYPE_INT_NEG);
            write_long(channel, entry->index);
            write_long(channel, value);
        }
    }
    ++channel->base.rowCounter;
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
        if (s.size + u.size > UINT8_MAX){
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
        } else if (value < 0xC9) {
            otic_pack_write_byte(channel, value);
            write_long(channel, entry->index);
            entry->lastValue.val.lval = value;
            entry->lastValue.type = OTIC_TYPE_INT_POS;
        } else {
            if (entry->lastValue.val.sval.size != 0) {
                free((void *)entry->lastValue.val.sval.ptr);
                entry->lastValue.val.sval.ptr = 0;
                entry->lastValue.val.sval.size = 0;
            }
            entry->lastValue.val.lval = value;
            entry->lastValue.type = OTIC_TYPE_INT_POS;
            otic_pack_write_byte(channel, OTIC_TYPE_INT_POS);
            write_long(channel, entry->index);
            write_long(channel, value);
        }
    }
    ++channel->base.rowCounter;
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
        if (s.size + u.size > UINT8_MAX){
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
        } else {
            if (entry->lastValue.val.sval.size != 0) {
                free((void *)entry->lastValue.val.sval.ptr);
                entry->lastValue.val.sval.ptr = 0;
                entry->lastValue.val.sval.size = 0;
            }
            entry->lastValue.val.dval = value;
            entry->lastValue.type = OTIC_TYPE_DOUBLE;
            otic_pack_write_byte(channel, OTIC_TYPE_DOUBLE);
            write_long(channel, entry->index);
            write_double(channel, value);
        }
    }
    ++channel->base.rowCounter;
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
        if (s.size + u.size > UINT8_MAX){
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
        } else {
            if (entry->lastValue.type != OTIC_TYPE_STRING)
                entry->lastValue.val.sval.ptr = 0;
            if (entry->lastValue.val.sval.size < v.size) {
                entry->lastValue.val.sval.ptr = realloc((char*)entry->lastValue.val.sval.ptr, v.size + 1);
                entry->lastValue.val.sval.size = v.size;
            }
            strncpy(entry->lastValue.val.sval.ptr, value, v.size);
            otic_pack_write_byte(channel, OTIC_TYPE_STRING);
            write_long(channel, entry->index);
            write_string(channel, &v);
            entry->lastValue.type = OTIC_TYPE_STRING;
        }
    }
    ++channel->base.rowCounter;
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
        if (s.size + u.size > UINT8_MAX) {
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
        } else {
            if (entry->lastValue.val.sval.size != 0) {
                free((void *)entry->lastValue.val.sval.ptr);
                entry->lastValue.val.sval.ptr = 0;
                entry->lastValue.val.sval.size = 0;
            }
            entry->lastValue.type = OTIC_TYPE_NULL;
            otic_pack_write_byte(channel, OTIC_TYPE_NULL);
            write_long(channel, entry->index);
        }
    }
    ++channel->base.rowCounter;
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
        if (s.size + u.size > UINT8_MAX) {
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
        if (entry->lastValue.val.sval.size != 0) {
            free((void *)entry->lastValue.val.sval.ptr);
            entry->lastValue.val.sval.ptr = 0;
            entry->lastValue.val.sval.size = 0;
        }
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

uint8_t otic_pack_channel_inject_a(otic_pack_channel_t* channel, double timestamp, const char* sensorName, const char* unit, oval_array_t* v)
{
    if (!otic_ts_handler(channel, timestamp))
        goto fail;
    otic_entry_t* entry = otic_pack_entry_find(channel, sensorName);
    if (!entry) {
        otic_str_t s = {(char*)sensorName, sensorName? strlen(sensorName) : 0}, u = {(char*)unit, unit? strlen(unit): 0};
        if (s.size + u.size > UINT8_MAX){
            otic_base_setError(&channel->base, OTIC_ERROR_BUFFER_OVERFLOW);
            goto fail;
        }
        otic_pack_entry_insert_a(channel, &s, &u, channel->totalEntries, v);
        otic_pack_id_assign(channel, &s, &u);
        otic_pack_write_byte(channel, OTIC_TYPE_ARRAY);
        write_long(channel, channel->totalEntries);
        write_array(channel, v);
        ++channel->totalEntries;
    } else {
        if (entry->lastValue.type == OTIC_TYPE_ARRAY && oval_array_cmp(&entry->lastValue.val.aval, v)) {
            otic_pack_write_byte(channel, OTIC_TYPE_UNMODIFIED);
            write_long(channel, entry->index);
        } else {
            entry->aHolder = realloc(entry->aHolder, v->size * sizeof(oval_t));
            memcpy(entry->aHolder, v->elements, v->size * sizeof(oval_t));
            entry->lastValue.type = OTIC_TYPE_ARRAY;
            entry->lastValue.val.aval.size = v->size;
            entry->lastValue.val.aval.elements = entry->aHolder;
            otic_pack_write_byte(channel, OTIC_TYPE_ARRAY);
            write_long(channel, entry->index);
            write_array(channel, v);
        }
    }
    return 1;
fail:
    otic_base_setState(&channel->base, OTIC_STATE_ON_ERROR);
    return 0;
}

uint8_t otic_pack_channel_init(otic_pack_channel_t* channel, uint8_t id, channel_type_e channelType, otic_pack_t* parent)
{
    otic_base_init(&channel->base);
    channel->cCtx = ZSTD_createCCtx();
    if (!channel->cCtx){
        otic_base_setError(&channel->base, OTIC_ERROR_ZSTD);
        goto fail;
    }
    size_t counter;
    for (counter = 0; counter < OTIC_PACK_CACHE_SIZE; counter++)
        channel->cache[counter] = 0;
    channel->totalEntries = 0;
    channel->threshold = &channel->base.cache[OTIC_BASE_CACHE_SIZE - OTIC_PACK_CACHE_TOP_LIMIT];
    channel->info.channelId = id;
    channel->info.channelType = channelType;
    channel->info.parent = parent;
    channel->timeInterval.time_start = 0;
    return 1;
fail:
    otic_base_setState(&channel->base, OTIC_STATE_ON_ERROR);
    return 0;
}

uint8_t otic_pack_channel_close(otic_pack_channel_t* channel)
{
    otic_pack_write_byte(channel, OTIC_TYPE_EOF);
    write_long(channel, channel->base.rowCounter);
    otic_pack_channel_flush(channel);
    ZSTD_freeCCtx(channel->cCtx);
    otic_entry_t* temp = 0;
    otic_entry_t* next = 0;
    for (size_t counter = 0; counter < OTIC_PACK_CACHE_SIZE; counter++)
    {
        next = channel->cache[counter];
        while (next)
        {
            free(next->name);
            if (next->lastValue.val.sval.size != 0)
                free((char *)next->lastValue.val.sval.ptr);
            temp = next->next;
            free(next);
           next = temp;
        }
    }
    for (uint8_t sCounter = 0; sCounter < channel->info.parent->totalChannels; sCounter++)
        if (channel->info.parent->channels[sCounter]->info.channelId == channel->info.channelId)
        {
            channel->info.parent->channels[sCounter] = channel->info.parent->channels[--channel->info.parent->totalChannels];
            channel->info.parent->channels[channel->info.parent->totalChannels] = channel;
            free(channel->info.parent->channels[channel->info.parent->totalChannels]);
            otic_base_setState(&channel->base, OTIC_STATE_CLOSED);
            return 1;
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
            ZSTD_OUT_SIZE,
            channel->base.cache,
            channel->base.top - channel->base.cache,
            OTIC_ZSTD_COMPRESSION_LEVEL
            );
    if (ZSTD_isError(ret)) {
        otic_base_setError(&channel->base, OTIC_ERROR_ZSTD);
        goto fail;
    }
    otic_payload_t payload = {.dataLen = ret, .channelId = channel->info.channelId};
    uint8_t val = OTIC_TYPE_DATA;
    channel->info.parent->flusher(&val, 1, channel->info.parent->data);
    if (!channel->info.parent->flusher((uint8_t*)&payload, sizeof(otic_payload_t), channel->info.parent->data)) {
        otic_base_setError(&channel->base, OTIC_ERROR_FLUSH_FAILED);
        goto fail;
    }
    if (!channel->info.parent->flusher(channel->ztd_out, ret, channel->info.parent->data)) {
        otic_base_setError(&channel->base, OTIC_ERROR_FLUSH_FAILED);
        goto fail;
    }
#endif
    channel->base.timestamp_start = 0;
    channel->base.top = channel->base.cache;
    return 1;
fail:
    otic_base_setState(&channel->base, OTIC_STATE_ON_ERROR);
    otic_pack_setState(channel->info.parent, OTIC_STATE_ON_ERROR);
    return 0;
}

uint8_t otic_pack_init(otic_pack_t* oticPack, uint8_t(*flusher)(uint8_t*, size_t, void*), void* data)
{
    if (!flusher) {
        otic_pack_setError(oticPack, OTIC_ERROR_INVALID_POINTER);
        goto fail;
    }
    oticPack->flusher = flusher;
    oticPack->data = data;
    oticPack->channels = 0;
    oticPack->totalChannels = 0;
    flusher((uint8_t*)&(otic_header_t){.magic = "OC\x07\xFF", .features=0x00, .version = OTIC_VERSION_MAJOR}, sizeof(otic_header_t), data);
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
 * @attention 0 is the reserved ID for end of file.
 * @return A Pointer to the new channel in case of successful allocation, else a nullptr in case of failure
 * Reasons for failure could be an invalid or already occupied id, allocation/reallocation failure or an invalid
 * pointer to \a oticPack
 */
otic_pack_channel_t* otic_pack_defineChannel(otic_pack_t* oticPack, channel_type_e channelType, uint8_t id, otic_feature_e features)
{
    if (otic_pack_getState(oticPack) != OTIC_STATE_OPENED) {
        otic_pack_setError(oticPack, OTIC_ERROR_AT_INVALID_STATE);
        goto fail;
    }
    uint8_t counter;
    for (counter = 0; counter < oticPack->totalChannels; counter++)
        if (oticPack->channels[counter]->info.channelId == id) {
            otic_pack_setError(oticPack, OTIC_ERROR_INVALID_ARGUMENT);
            goto fail;
        }
    otic_pack_channel_t** ptr = realloc(oticPack->channels, sizeof(otic_pack_channel_t*) * (oticPack->totalChannels + 1));
    if (!ptr) {
        otic_pack_setError(oticPack, OTIC_ERROR_ALLOCATION_FAILURE);
        goto fail;
    }
    oticPack->channels = ptr;
    // TODO: Try to use malloc instead!
    oticPack->channels[oticPack->totalChannels] = calloc(1, sizeof(otic_pack_channel_t));
//    oticPack->channels[oticPack->totalChannels] = malloc(sizeof(otic_pack_channel_t));
    if (!oticPack->channels[oticPack->totalChannels]){
        otic_pack_setError(oticPack, OTIC_ERROR_ALLOCATION_FAILURE);
        goto fail;
    }
    otic_meta_data_t oticMetaData[3] = {
            {.metaType = OTIC_META_TYPE_CHANNEL_DEFINE, .metaArg = id},
            {.metaType = OTIC_META_TYPE_CHANNEL_TYPE, .metaArg = channelType},
            {.metaType = OTIC_META_TYPE_COMPRESSION_METHOD, .metaArg = OTIC_FEATURE_COMPRESSION_ZSTD}
    };
    otic_meta_head_t oticMetaHead = {.otic_type = OTIC_TYPE_METADATA, .metaDataSize = sizeof(oticMetaData)/ sizeof(otic_meta_data_t)};
    oticPack->flusher((uint8_t*)&oticMetaHead, sizeof(otic_meta_head_t), oticPack->data);
    oticPack->flusher((uint8_t*)&oticMetaData, sizeof(oticMetaData), oticPack->data);
    otic_pack_channel_init(oticPack->channels[oticPack->totalChannels], id, channelType, oticPack);
    return oticPack->channels[oticPack->totalChannels++];
fail:
    otic_pack_setState(oticPack, OTIC_STATE_ON_ERROR);
    return 0;
}

uint8_t otic_pack_closeChannel(otic_pack_t* oticPack, uint8_t id)
{
    uint8_t counter;
    for (counter = 0; counter < oticPack->totalChannels; counter++)
    {
        if (oticPack->channels[counter]->info.channelId == id)
        {
            otic_pack_channel_close(oticPack->channels[counter]);
            otic_pack_channel_t* temp = oticPack->channels[counter];
            oticPack->channels[counter] = oticPack->channels[oticPack->totalChannels];
            free(temp);
            oticPack = realloc(oticPack, sizeof(otic_pack_channel_t*) * oticPack->totalChannels);
            return 1;
        }
    }
    otic_pack_setError(oticPack, OTIC_ERROR_INVALID_ARGUMENT);
    otic_pack_setState(oticPack, OTIC_STATE_ON_ERROR);
    return 0;
}

uint8_t otic_pack_flush(otic_pack_t* oticPack)
{
    uint8_t counter;
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
    for (counter = 0; counter < oticPack->totalChannels; counter++)
    {
        ret = ZSTD_compressCCtx(
                oticPack->channels[counter]->cCtx, oticPack->channels[counter]->ztd_out, ZSTD_OUT_SIZE,
                oticPack->channels[counter]->base.cache,
                oticPack->channels[counter]->base.top - oticPack->channels[counter]->base.cache, OTIC_ZSTD_COMPRESSION_LEVEL
                );
        if (ZSTD_isError(ret)){
            otic_base_setError(&oticPack->channels[counter]->base, OTIC_ERROR_ZSTD);
            goto fail;
        }
        payload.channelId = oticPack->channels[counter]->info.channelId;
        payload.dataLen = ret;
        if (!oticPack->flusher((uint8_t*)&payload, sizeof(otic_payload_t), oticPack->data)){
            otic_base_setError(&oticPack->channels[counter]->base, OTIC_ERROR_FLUSH_FAILED);
            goto fail;
        }
        if (!oticPack->flusher(oticPack->channels[counter]->ztd_out, ret, oticPack->data)){
            otic_base_setError(&oticPack->channels[counter]->base, OTIC_ERROR_FLUSH_FAILED);
            goto fail;
        }
        oticPack->channels[counter]->base.top = oticPack->channels[counter]->base.cache;
    }
#endif
    return 1;
fail:
    otic_pack_setState(oticPack, OTIC_STATE_ON_ERROR);
    return 0;
}

void otic_pack_close(otic_pack_t* oticPack) {
    while (oticPack->totalChannels) {
        otic_pack_channel_close(oticPack->channels[oticPack->totalChannels - 1]);
    }
    free(oticPack->channels);
    otic_pack_setState(oticPack, OTIC_STATE_CLOSED);
}
