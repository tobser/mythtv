// C++ headers
#include <unistd.h>

// QT headers
#include <QApplication>
#include <QTimer>

// MythTV headers
#include <mythcontext.h>
#include <mythplugin.h>
#include <mythpluginapi.h>
#include <mythversion.h>
#include <mythmainwindow.h>
#include <dbutil.h>

// teatime headers
#include "teatimeui.h"
#include "data.h"

using namespace std;


static void openTimeSetter(void);

TeaTimeData* gTeaData;

static void setupKeys(void)
{
    REG_JUMPEX(QT_TRANSLATE_NOOP("MythControls", "Set a teatimer"),
            "Opens a dialog to setup a tea timer.", "" , openTimeSetter, false);

    LOG_Tea(LOG_INFO, "Registered JumpPoint");
}

static bool CreateTable(MSqlQuery query)
{
    bool success = query.exec(
            " CREATE TABLE IF NOT EXISTS `teatime`  ("
            "   `id` MEDIUMINT NOT NULL AUTO_INCREMENT,"
            "    PRIMARY KEY(`id`),"
            "   `message_text` text,  "
            "   `exec_date_time` timestamp NULL DEFAULT NULL "
            "        COMMENT 'time the timer actions will be executed',"
            "   `date_time` timestamp NULL DEFAULT NULL "
            "       COMMENT 'Either this or time_span have to contain valid data',"
            "   `time_span` time DEFAULT NULL "
            "       COMMENT 'Either this or date_time have to contain valid data',"
            "   `pause_playback` enum('no','yes') DEFAULT NULL,"
            "   `countdown_seconds` int(11) DEFAULT NULL"
            " ) ENGINE=MyISAM DEFAULT CHARSET=utf8"
            "    COMMENT 'stores timer data for the teatime plugin';"
            );

    if (!success)
    {
        LOG_Tea(LOG_WARNING, "Could not create initial teatime table");
        LOG_Tea(LOG_WARNING, query.lastError().text());
        return false;
    }

    success = query.exec(
            " CREATE TABLE IF NOT EXISTS `teatime_rundata`  ("
            " 	`timer_id` MEDIUMINT NOT NULL,   "
            " 	`run_order` TINYINT NOT NULL,"
            " 	`type` enum('jump_point','system_key_event') NOT NULL,"
            " 	`data` text NOT NULL"
            " ) ENGINE=MyISAM DEFAULT CHARSET=utf8 "
            "   COMMENT 'stores what shall be executed and in which order when the timer is expired'"
            " ;"
            );
    if (!success)
    {
        LOG_Tea(LOG_WARNING, "Could not create initial teatime_rundata table");
        LOG_Tea(LOG_WARNING, query.lastError().text());
        return false;
    }

    gCoreContext->SaveSetting("TeatimeDBSchemaVer", "1");

    query.exec(
            "INSERT INTO `mythconverg`.`teatime` "
            "   (`message_text`, `time_span`, `pause_playback`) "
            "   VALUES "
            "   ( 'Tea is ready!', '00:05:00', 'yes')"
            );

    return true;
}

static bool updateDb()
{
    LOG_Tea(LOG_INFO, "Checking DB state.");
    MSqlQuery query(MSqlQuery::InitCon());
    if (!DBUtil::TryLockSchema(query, 60))
    {
        LOG_Tea(LOG_WARNING, "Could not get db schema lock.");
        return false;
    }

    int dbVer =  gCoreContext->GetSetting("TeatimeDBSchemaVer", "0").toInt();

    bool success = true;
    switch(dbVer)
    {
        case 0:
            {
                if (!CreateTable(query))
                {
                    success = false;
                    break;
                }

            }
    }

    DBUtil::UnlockSchema(query);

    return success;
}



static void openTimeSetter(void)
{
    MythScreenStack *popupStack = GetMythMainWindow()->GetMainStack();
    if (!popupStack)
    {
        LOG_Tea(LOG_WARNING, "Could not get PopupStack.");
        return;
    }

    QString result = popupStack->GetLocation(true);
    if (result.contains("Playback"))
    {
            LOG_Tea(LOG_INFO, "Can not create Teatime UI while in playback.");
            return;
    }
    if (result.contains("teatime"))
        return;


    TeaTime* teatime = new TeaTime(popupStack, gTeaData);
    if (!teatime->Create())
    {
        LOG_Tea(LOG_WARNING, "Could not create Teatime UI.");
        delete teatime;
        teatime = NULL;
        return;
    }
    popupStack->AddScreen(teatime);
}

int mythplugin_init(const char *libversion)
{
    if (!gContext->TestPopupVersion("mythteatime",
        libversion, MYTH_BINARY_VERSION))
        return -1;

    if (!updateDb())
    {
        //ToDo: annoy user with error popup
    }


    setupKeys();

    MythMainWindow *mainWin = GetMythMainWindow();
    gTeaData = new TeaTimeData(mainWin);
    if (gTeaData->initialize())
    {
        LOG_Tea(LOG_INFO, "Teatime plugin started.");
        return 0;
    }
    else
    {
        LOG_Tea(LOG_WARNING, "Failed to init TeaTimeData.");
        return -1;
    }
}

int mythplugin_run(void)
{

    return 0;
}

int mythplugin_config(void)
{
    
    return 0;
}

