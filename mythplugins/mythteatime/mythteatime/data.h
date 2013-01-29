#ifndef MYTH_TEATIME_DATA_H
#define MYTH_TEATIME_DATA_H

#include <mythmainwindow.h>

#include "teatimeui.h"

#include <QThread>
#include <QTimer>
#include <QString>
#include <QEvent>
#include <QMap>

enum TimerType { Time_Span , Date_Time };
class TimerData {
    public:
        QString toString();

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


class TeaTimeData :public QObject
{
    Q_OBJECT

    public:
        TeaTimeData(MythMainWindow *mainWin);
        bool initialize(void);
        void startTimer(int seconds);
        bool hasActiveTimer()
        {
            return (m_TimerMilliSecs > 0);
        };
        int GetRemainingSeconds()
        {
            return QDateTime::currentDateTime().secsTo(m_TimeoutTime);
        };

    public slots:
        void done();

    private:
        void ResetTimerData();
        MythMainWindow *m_MainWindow;
        QTimer  * m_Timer;
        QDateTime m_TimeoutTime;
        int m_TimerMilliSecs;
        QMap<int, TimerData> m_Timers;
};



#endif /* MYTH_TEATIME_DATA_H*/

