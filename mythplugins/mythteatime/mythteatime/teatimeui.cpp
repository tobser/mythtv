// POSIX headers
#include <unistd.h>

// MythTV headers
#include <mythuibutton.h>
#include <mythuitext.h>
#include <mythmainwindow.h>
#include <mythcontext.h>
#include <mythdirs.h>
#include <mythdialogbox.h>
#include <mythuiutils.h>

// MythTeaTime headers
#include "teatimeui.h"
#include "teatimeuisettings.h"
#include "data.h"

TeaTime::TeaTime(MythScreenStack *parent, TeaTimeData* tt):
    MythScreenType(parent, "teatime"),
    m_CancelButton(NULL),
    m_OkButton(NULL),
    m_Setup(NULL),
    m_TimeSpinbox(NULL),
    m_InfoText(NULL),
    m_TimeData(tt)
{
}

bool TeaTime::Create(void)
{

    bool foundtheme = false;

    // Load the theme for this screen
    foundtheme = LoadWindowFromXML("teatime-ui.xml", "teatime", this);
    
    if (!foundtheme)
    {
        LOG_Tea(LOG_WARNING, "window teatime in teatime-ui.xml is missing."); 
        return  false;

    }

    UIUtilW::Assign(this, m_CancelButton, "cancel");
    UIUtilW::Assign(this, m_InfoText, "infotext");
    m_InfoText->SetText(tr("Please select a timeout value."));

    bool err = false;
    UIUtilE::Assign(this, m_OkButton, "ok", &err);
    UIUtilE::Assign(this, m_Setup, "setup", &err);
    UIUtilE::Assign(this, m_TimeSpinbox, "time_span", &err);

    if (err)
    {
        LOG_Tea(LOG_WARNING, "Theme is missing required elements."); 
        return  false;
    }

    connect(m_CancelButton, SIGNAL(Clicked()), SLOT(Close()));
    connect(m_OkButton,     SIGNAL(Clicked()), SLOT(OkClicked()));
    connect(m_Setup,        SIGNAL(Clicked()), SLOT(Setup()));

    int minutes = gCoreContext->GetSetting("Teatime_Minutes", "5").toInt();
    m_TimeSpinbox->SetRange(0, 600, 1, 5);
    m_TimeSpinbox->SetValue(minutes);
    m_TimeSpinbox->SetVisible(true);

    BuildFocusList();

    return true;
}

void TeaTime::OkClicked()
{
    int minutes = m_TimeSpinbox->GetValue().toInt();
    gCoreContext->SaveSetting("Teatime_Minutes", QString("%1").arg( minutes ));
    Close();
}
void TeaTime::Setup()
{
    MythScreenStack *stack = GetScreenStack();
    if (stack == NULL)
    {
        LOG_Tea(LOG_WARNING, "Could not get screenstack."); 
        return;
    }
    TeaTimerSettings *settings = new TeaTimerSettings(stack);
    if (!settings->Create())
    {
        LOG_Tea(LOG_WARNING, "Could not load teatime settings window."); 
        return;
    }
    stack->AddScreen(settings);
}

void TeaTime::timerEvent(QTimerEvent *event)
{
}
