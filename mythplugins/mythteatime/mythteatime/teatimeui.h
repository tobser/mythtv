#ifndef MYTHTEATIME_UI_H
#define MYTHTEATIME_UI_H

// MythTV headers

#include <mythscreentype.h>
#include <mythuibutton.h>
#include <mythuispinbox.h>
#include <mythuitext.h>


#ifndef LOG_Tea
#define LOG_Tea(level, message) LOG(VB_GENERAL, level, QString("TT: %1").arg(message))
#endif

class TeaTimeData;
class TeaTime : public MythScreenType 
{
    Q_OBJECT
    
    public:
        TeaTime(MythScreenStack *parent, TeaTimeData* tt);
        bool Create(void);
        
    public slots:
        void OkClicked(void);
        void Setup(void);

    protected:
        void timerEvent(QTimerEvent *event);

    private:
        MythUIButton    *m_CancelButton;
        MythUIButton    *m_OkButton;
        MythUIButton    *m_Setup;
        MythUISpinBox   *m_TimeSpinbox;
        MythUIText      *m_InfoText;
        TeaTimeData     *m_TimeData;

};

#endif /* MYTHTEATIME_UI_H*/

