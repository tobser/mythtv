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

// teatime headers
#include "teatimeui.h"
#include "teatimedata.h"

using namespace std;


static void openTimeSetter(void);

TeaTimeData* gTeaData;

static void setupKeys(void)
{
    REG_JUMPEX(QT_TRANSLATE_NOOP("MythControls", "Set a teatimer"),
            "Opens a dialog to setup a tea timer.", "" , openTimeSetter, false);

    LOG_Tea(LOG_INFO, "Registered JumpPoint");
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

    setupKeys();

    MythMainWindow *mainWin = GetMythMainWindow();
    gTeaData = new TeaTimeData(mainWin);
    LOG_Tea(LOG_INFO, "Teatime plugin started.");
    return 0;
}

int mythplugin_run(void)
{

    return 0;
}

int mythplugin_config(void)
{
    
    return 0;
}

