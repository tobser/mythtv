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
    m_Timer(NULL)
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
        TimerData* d = new TimerData;
        d->Id = query.value(0).toInt();
        d->Message_Text= query.value(1).toString();
        if (query.value(2).toString() == "time_span")
        {
            d->Type = Time_Span; 
        }
        else
        {
            d->Type = Date_Time; 
        }
        
       LOG_Tea( LOG_INFO, query.value(3).toString());
       d->Date_Time = QDateTime::fromString(query.value(3).toString());
        
       d->Time_Span =  QTime::fromString(query.value(4).toString(),
                                            "hh:mm:ss");
       d->Key_Events = query.value(5).toString();
       d->Jump_Points = query.value(6).toString();

       d->Pause_Playback = (query.value(7).toString() == "yes");
       d->Count_Down = query.value(8).toInt();
       m_Timers.insert(d->Id, d);
       LOG_Tea(LOG_INFO, d->toString());
    }

    startTimer();
    return true;
 }

void TeaTimeData::startTimer(void)
{
    stopTimer();

    m_Timer = new QTimer(this);
    if (!connect(m_Timer, SIGNAL(timeout()), this, SLOT(done())))
    {
        LOG_Tea(LOG_WARNING, "timout signal not connected.");
    }
    else
    {    
        m_Timer->start(1000);
    }
}


void TeaTimeData::done()
{
    LOG_Tea(LOG_INFO, "timout!");

    QMapIterator<int,TimerData*> it(m_Timers);
    while(it.hasNext())
    {
        it.next();
        TimerData *td = it.value();
        if (td->isExpired())
            LOG_Tea(LOG_INFO, QString("Done: ").append(td->toString()));
        else
            LOG_Tea(LOG_INFO, QString("not yet: ").append(td->toString()));

    }

    /*// popup
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

    stopTimer();
    */
}

void TeaTimeData::stopTimer()
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
}
