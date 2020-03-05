#ifndef OTIC_TIME_SERIES_H
#define OTIC_TIME_SERIES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "base.h"

typedef enum {
    OTIC_TIMESERIE_NO_ERROR,
    OTIC_TIMESERIE_TIMESTAMP_NOT_IN_CHRONOLOGICAL_ORDER,
} timeSerie_error_e;

typedef struct {
    uint8_t fillEmpty;
    uint64_t sampleInterval;
    uint64_t currentFrameStart;
    uint64_t currentFrameEnd;
    uint64_t startTs;
    uint64_t endTs;
    timeSerie_error_e error;
} timeSerie_t;

uint8_t otic_timeserie_init(timeSerie_t *ts, double startTs, double endTs, uint8_t fillEmpty, double sampleIntervall);
uint8_t otic_timeserie_shift(timeSerie_t *ts, double timestamp);
uint8_t otic_timeserie_fillNull(timeSerie_t *ts, double timestamp);
void    otic_timeserie_increment(timeSerie_t *ts);
double  otic_timeserie_getStart(timeSerie_t *ts);
void    otic_timeserie_close(timeSerie_t *ts);

#ifdef __cplusplus
}
#endif


#endif // OTIC_TIME_SERIES_H