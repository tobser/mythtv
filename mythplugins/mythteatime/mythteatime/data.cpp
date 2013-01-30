#include "data.h"
#include "timerdata.h"
#include "teatimeui.h"

#include <mythlogging.h>
#include <dbutil.h>


TeaTimeData::TeaTimeData(MythMainWindow *mainWin)
    :m_MainWindow(mainWin), 
    m_Timer(NULL)
{
}
bool TeaTimeData::initialize(void)
 {
    LOG_Tea(LOG_INFO, "Loading timers from DB");
    MSqlQuery query(MSqlQuery::InitCon());
    bool success = query.exec("SELECT id from `teatime`");
    if (!success)
    {
        LOG_Tea(LOG_WARNING, "Could not read timers from DB.");
        return false;
    }

    while(query.next())
    {
        int tId = query.value(0).toInt();
        TimerData* d = new TimerData(tId);
        d->init();
        m_Timers.insert(d->Id, d);
        LOG_Tea(LOG_INFO, d->toString());
        if (d->isActive())
            m_ActiveTimers << d;
    }

    if (!m_ActiveTimers.isEmpty())
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
        LOG_Tea(LOG_INFO, "Timer started.");
    }
}


void TeaTimeData::done()
{
    LOG_Tea(LOG_INFO, "timout!");

    QMutableListIterator<TimerData*> it(m_ActiveTimers);
    while(it.hasNext())
    {
        TimerData *val = it.next();
        if (val->isActive())
        {
            LOG_Tea(LOG_INFO, QString("not yet: ").append(val->toString()));
        }
        else
        {
            LOG_Tea(LOG_INFO, QString("Done: ").append(val->toString()));
            val->execute();
            it.remove();
        }
    }

    if (m_ActiveTimers.isEmpty())
    {
        stopTimer();
    }
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
        LOG_Tea(LOG_INFO, "Timer stopped.");
    }
}
