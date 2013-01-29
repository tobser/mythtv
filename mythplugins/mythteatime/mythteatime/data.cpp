#include "data.h"
#include "teatimeui.h"

#include <mythlogging.h>
#include <mythevent.h>
#include <mythcontext.h>
#include <mythevent.h>
#include <dbutil.h>

#include <QCoreApplication>

TeaTimeData::TeaTimeData(MythMainWindow *mainWin)
    :m_MainWindow(mainWin), 
    m_Timer(NULL),
    m_TimerMilliSecs(0)
{
}
 bool TeaTimeData::initialize(void)
 {
    LOG_Tea(LOG_INFO, "Loading timers from DB");
    MSqlQuery query(MSqlQuery::InitCon());
    bool success = query.exec("SELECT * from `teatime`");
    if (!success)
    {
        LOG_Tea(LOG_WARNING, "Could not read timers from DB.");
        return false;
    }

    while(query.next())
    {
        TimerData d;
        d.Id = query.value(0).toInt();
        d.Message_Text= query.value(1).toString();
        if (query.value(2).toString() == "time_span")
            d.Type == Time_Span; 
        else
            d.Type == Date_Time; 

      //  d.Date_Time = QDateTime(query.value(3).toString());
      //  d.Time_Span = QTime(query.value(4).toInt());
       d.Key_Events = query.value(5).toString();
       d.Jump_Points = query.value(6).toString();

       d.Pause_Playback = (query.value(7).toString() == "yes");
       d.Count_Down = query.value(8).toInt();
       LOG_Tea(LOG_INFO, d.toString());
    }
     // read all db entries and populate map
    return true;
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
    bool pause =  gCoreContext->GetSetting("Teatime_PausePlayback", 
                                                    "1") == "1";
 
    QStringList sl ;
    if (pause)
        sl << "pauseplayback"; 

    MythEvent* me = new MythEvent(MythEvent::MythUserMessage, notifyText, sl);
    QCoreApplication::instance()->postEvent(m_MainWindow, me);

    QString sys_event = QString("LOCAL_SYSTEM_EVENT EventCmdKey01");
    MythEvent* me2 = new MythEvent(MythEvent::MythEventMessage,sys_event);
    QCoreApplication::instance()->postEvent(m_MainWindow, me2);
    LOG_Tea(LOG_INFO, QString("Positing ").append(sys_event));

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
QString TimerData::toString(void)
{
    return QString ("TimerData[id=%1, text=%2, type=%3]").arg(Id)
        .arg(Message_Text);
}
