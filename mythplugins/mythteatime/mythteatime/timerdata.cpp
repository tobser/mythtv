#include "timerdata.h"

QString TimerData::toString(void)
{
    return QString ("TimerData[id=%1, text=%2,t=%3,ts=%4,dt=%5,kv=%6,jp=%7]").arg(Id)
        .arg(Message_Text). arg(Type).arg(Time_Span.toString()).arg(Date_Time.toString()).arg(Key_Events)
        .arg(Jump_Points);
}

bool TimerData::isExpired(void)
{
    QDateTime now = QDateTime::currentDateTime();
    return (now >= Date_Time);
}
