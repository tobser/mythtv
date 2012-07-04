#ifndef MYTH_TEA_TIMER_SETTINGS_H
#define MYTH_TEA_TIMER_SETTINGS_H
#include <QObject>
#include <QString>
#include <QTimer>

#include <mythscreentype.h>
#include <mythuibutton.h>
#include <mythuitextedit.h>
#include <mythuicheckbox.h>

class TeaTimerSettings: public  MythScreenType
{
    Q_OBJECT

    public:
        TeaTimerSettings(MythScreenStack *parent);
        bool Create();

    public slots:
        void save();

    private:
        MythUIButton    *m_CancelButton;
        MythUIButton    *m_OkButton;
        MythUITextEdit  *m_NotifyText;
        MythUICheckBox  *m_PausePlayback;


};
#endif /* MYTH_TEA_TIMER_SETTINGS_H */
