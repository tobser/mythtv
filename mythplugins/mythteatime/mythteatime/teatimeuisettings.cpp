#include <mythlogging.h>
#include <mythcontext.h>

#include "teatimeui.h"
#include "teatimeuisettings.h"

TeaTimerSettings::TeaTimerSettings (MythScreenStack *parent)
    :MythScreenType(parent, "teatimesettings"),
    m_CancelButton(NULL),
    m_OkButton(NULL),
    m_NotifyText(NULL),
    m_PausePlayback(NULL)
{
}
bool TeaTimerSettings::Create()
{
   LOG_Tea(LOG_INFO, "creating teatimersettings."); 
    bool foundtheme = false;

    // Load the theme for this screen
    foundtheme = LoadWindowFromXML("teatime-ui.xml", "teatimesettings", this);
    
    if (!foundtheme)
    {
        LOG_Tea(LOG_WARNING, "window teatime in teatime-ui.xml is missing."); 
        return  false;
    }
    UIUtilW::Assign(this, m_CancelButton, "cancel");

    bool err = false;
    UIUtilE::Assign(this, m_OkButton, "ok", &err);
    UIUtilE::Assign(this, m_NotifyText, "notification_message", &err);
    UIUtilE::Assign(this, m_PausePlayback, "pause_playback", &err);
    if (err)
    {
        LOG_Tea(LOG_WARNING, "Theme is missing required elements."); 
        return  false;
    }

    QString notifyText =  gCoreContext->GetSetting("Teatime_NotificationText", 
                                                    tr("Tea is ready!"));
    m_NotifyText->SetText(notifyText);
    bool pause =  gCoreContext->GetSetting("Teatime_PausePlayback", 
                                                    "1") == "1";
    m_PausePlayback->SetCheckState(pause);

    
    if (m_CancelButton)
        connect(m_CancelButton, SIGNAL(Clicked()), SLOT(Close()));

    connect(m_OkButton,     SIGNAL(Clicked()), SLOT(save()));

    BuildFocusList();

    return true;

}
void TeaTimerSettings::save()
{

    gCoreContext->SaveSetting("Teatime_NotificationText", m_NotifyText->GetText());
    gCoreContext->SaveSetting("Teatime_PausePlayback", m_PausePlayback->GetBooleanCheckState()? "1":"0");

    LOG_Tea(LOG_INFO, "Saved teatime settings.");
    Close();
}
