#ifndef MYTH_TEATIME_DATA_H
#define MYTH_TEATIME_DATA_H

#include <mythmainwindow.h>

#include "teatimeui.h"

#include <QThread>
#include <QTimer>
#include <QString>
#include <QEvent>

/*
 * Holds the timer and opens the notification
 * after timer elapsed.
 */

class TeaTimeData :public QObject
{
    Q_OBJECT

    public:
        TeaTimeData(MythMainWindow *mainWin);
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
};


#endif /* MYTH_TEATIME_DATA_H*/

