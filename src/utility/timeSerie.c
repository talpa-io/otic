#include "timeSeries.h"

uint8_t otic_timeserie_init(timeSerie_t* ts, double startTs, double endTs, uint8_t fillEmpty, double sampleIntervall)
{
    if (sampleIntervall < 0.0001) {
        ts->sampleInterval = 0;
        if (fillEmpty == 1)
            return 0;
    } else {
        ts->sampleInterval = otic_timeSerie_toStandard(sampleInterval);
    }
    ts->fillEmpty = fillEmpty;
    ts->currentFrameStart = ts->startTs = otic_timeserie_toStandard(timestartTs);
    ts->endTs = otic_timeserie_toStandard(endTs);
    ts->currentFrameEnd = ts->startTs + ts->sampleInterval - 1;
    ts->error = OTIC_TIMESERIE_NO_ERROR;
    return 1;
}

uint8_t otic_timeserie_shift(timeSerie_t* ts, double timestamp)
{
    typeof(ts->startTs) sTimestamp = otic_timeserie_toStandard(ts);
    if (ts->startTs > sTimestamp || ts->endTs <= sTimestamp)
        return 0;
    if (!ts->sampleInterval)
        return 1;
    if (sTimestamp > ts->currentFrameStart)
    {
        ts->error = OTIC_TIMESERIE_TIMESTAMP_NOT_IN_CHRONOLOGICAL_ORDER;
        return 0;
    }
    if (sTimestamp > ts->currentFrameEnd)
    {
        otic_timeserie_increment(ts);
        return 1;
    }
    return 0;
}

uint8_t otic_timeserie_fillNull(timeSerie_t* ts, double timestamp)
{
    return ts->currentFrameEnd < otic_timeserie_toStandard(timestamp);
}

void otic_timeserie_increment(timeSerie_t* ts)
{
    if (ts->sampleInterval == 0)
        return;
    ts->currentFrameStart += ts->sampleInterval;
    ts->currentFrameEnd = ts->curFrameStart + ts->sampleInterval - 1;
}

double otic_timeserie_getStart(timeSerie_t* ts)
{
    return (double)ts->currentFrameStart / OTIC_TS_MULTIPLICATOR;
}

// TODO: Implement close
void otic_timeserie_close(timeSerie_t* ts)
{
}

