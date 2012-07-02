#include "teatimedata.h"
#include "teatimeui.h"

#include <mythlogging.h>
#include <mythevent.h>
#include <mythcontext.h>
#include <mythevent.h>

#include <QCoreApplication>

TeaTimeData::TeaTimeData(MythMainWindow *mainWin)
    :m_MainWindow(mainWin), 
    m_Timer(NULL),
    m_TimerMilliSecs(0)
{
}

void TeaTimeData::startTimer(int seconds)
{
    ResetTimerData();
    if (seconds == 0)
        return;

    m_TimeoutTime = QDateTime::currentDateTime().addSecs(seconds);
    m_TimerMilliSecs = seconds * 1000; 
    

    m_Timer = new QTimer(this);
    m_Timer->setSingleShot(true);
    if (!connect(m_Timer, SIGNAL(timeout()), this, SLOT(done())))
    {
        LOG_Tea(LOG_WARNING, "timout signal not connected.");
    }
    else
    {    
        m_Timer->start(m_TimerMilliSecs);
    }
}


void TeaTimeData::done()
{
    LOG_Tea(LOG_INFO, "timout!");

    // popup
    QString notifyText =  gCoreContext->GetSetting("Teatime_NotificationText", 
                                                    tr("Tea is ready!"));
    QStringList sl ;
    sl << "pauseplayback"; // this only works with a patched mythtv
    MythEvent* me = new MythEvent(MythEvent::MythUserMessage, notifyText, sl);
    QCoreApplication::instance()->postEvent(m_MainWindow, me);

    ResetTimerData();
}

void TeaTimeData::ResetTimerData()
{
    if (m_Timer != NULL)
    {
        if (m_Timer->isActive())
        {
            m_Timer->stop();
        }
        delete m_Timer;        
        m_Timer = NULL;
    }

    m_TimerMilliSecs = 0;
}
