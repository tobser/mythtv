#ifndef MYTH_DATA_H
#define MYTH_DATA_H

#include <mythmainwindow.h>

#include "teatimeui.h"
#include "timerdata.h"

#include <QThread>
#include <QTimer>
#include <QString>
#include <QEvent>
#include <QMap>



class TeaTimeData :public QObject
{
    Q_OBJECT

    public:
        TeaTimeData(MythMainWindow *mainWin);
        bool initialize(void);

    public slots:
        void done();

    private:
        void stopTimer(void);
        void startTimer(void);
        MythMainWindow *m_MainWindow;
        QTimer  * m_Timer;
        QMap<int, TimerData *> m_Timers;
};



#endif /* MYTH_DATA_H*/

