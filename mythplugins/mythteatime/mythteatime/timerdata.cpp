#include "teatimeui.h" // LOG_Tea
#include "timerdata.h"
#include <dbutil.h>
#include <mythscreentype.h>
#include <mythcontext.h>
#include <mythmainwindow.h>
#include <mythevent.h>

#include <QCoreApplication>

TimerData::TimerData(int id)
    :Id(id)
{

}

QString TimerData::toString(void)
{
    return QString ("TimerData[id=%1,t=\"%2\",e=%3,ts=%4,dt=%5,a=\"%6\"]")
        .arg(Id)
        .arg(Message_Text)
        .arg(Exec_Date_Time.toString())
        .arg(Time_Span.toString())
        .arg(Date_Time.toString())
        .arg(Exec_Actions.count());
}

bool TimerData::isActive(void)
{
    QDateTime now = QDateTime::currentDateTime();
    return (now <= Exec_Date_Time);
}

bool TimerData::init(void)
{
    MSqlQuery query(MSqlQuery::InitCon());
    QString q = QString ("SELECT "
              " message_text,"      // 0
              " exec_date_time, "   // 1 
              " date_time, "        // 2
              " time_span, "        // 3
              " pause_playback, "   // 4
              " countdown_seconds " // 5
            "from `teatime` WHERE id = '%1'").arg(Id);
    bool success = query.exec(q);
    if (!success)
    {
        QString msg = QString("Read timer data for timer %1 failed. %2")
                        .arg(Id).arg(query.lastError().text());
        LOG_Tea(LOG_WARNING, msg);
        return false;
    }

    if(!query.next())
    {
        return false;
    }

    Message_Text= query.value(0).toString();

    Exec_Date_Time = query.value(1).toDateTime();
    Date_Time = QDateTime::fromString(query.value(2).toString());
    Time_Span = QTime::fromString(query.value(3).toString(), "hh:mm:ss");
    Pause_Playback = (query.value(4).toString() == "yes");
    Count_Down = query.value(5).toInt();

    QString rundData = QString("SELECT type,data FROM teatime_rundata "
                     "WHERE timer_id = %1 ORDER BY run_order ASC")
                    .arg(Id);

    success = query.exec(rundData);
    if (!success)
    {
        QString msg = QString("Read rundata for timer %1 failed. %2")
                        .arg(Id).arg(query.lastError().text());
        LOG_Tea(LOG_WARNING, msg);
        return false;
    }

    while(query.next())
    {
        TeaAction a;
        a.Action_Type = query.value(0).toString();
        a.Action_Data = query.value(1).toString();
        Exec_Actions << a;
    }
    LOG_Tea(LOG_INFO, QString("%1 active: %2").arg(Id).arg(isActive()));

    return true;
}


void TimerData::execute(void)
{
    MythMainWindow *mainWin = GetMythMainWindow();
    if (!mainWin)
    {
        LOG_Tea(LOG_WARNING, "Could not get main window.");
        return;
    }

    QStringList sl ;
    if (Pause_Playback)
        sl << "pauseplayback";

    LOG_Tea(LOG_INFO, QString("Popup: %1").arg(Message_Text));
    MythEvent* me = new MythEvent(MythEvent::MythUserMessage, Message_Text, sl);
    QCoreApplication::instance()->postEvent(mainWin, me);

/*
    QString sys_event = QString("LOCAL_SYSTEM_EVENT EventCmdKey01");
    MythEvent* me2 = new MythEvent(MythEvent::MythEventMessage,sys_event);
    QCoreApplication::instance()->postEvent(m_MainWindow, me2);
    LOG_Tea(LOG_INFO, QString("Positing ").append(sys_event));
    */

}
