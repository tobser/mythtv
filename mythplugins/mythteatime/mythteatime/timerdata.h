#ifndef MYTH_TIMER_DATA_H
#define MYTH_TIMER_DATA_H

#include <QString>
#include <QTime>


enum TimerType { Time_Span , Date_Time };
class TimerData {
    public:
        bool isExpired(void);
        QString toString(void);

        int Id;
        QString Message_Text;
        TimerType Type;
        QDateTime Date_Time;
        QTime   Time_Span;
        QString Key_Events;
        QString Jump_Points;
        bool Pause_Playback;
        int Count_Down;

};

#endif
