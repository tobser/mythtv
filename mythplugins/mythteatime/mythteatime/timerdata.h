#ifndef MYTH_TIMER_DATA_H
#define MYTH_TIMER_DATA_H

#include <QString>
#include <QVariant>
#include <QTime>

class TeaAction{
    public:
        QString Action_Type;
        QString Action_Data;
};


class TimerData {
    public:
        TimerData(int id);
        bool init(void);
        bool isActive(void);
        void execute(void);
        QString toString(void);

        int Id;
        QString Message_Text;
        QDateTime Exec_Date_Time;
        QDateTime Date_Time;
        QTime   Time_Span;
        QList<TeaAction> Exec_Actions;
        bool Pause_Playback;
        int Count_Down;

};

#endif
