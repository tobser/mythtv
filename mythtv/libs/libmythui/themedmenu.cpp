#include <qapplication.h>
#include <qimage.h>
#include <qdir.h>
#include <qdom.h>

#include <iostream>
#include <cmath>
using namespace std;

#include "themedmenu.h"
#include "mythcontext.h"
#include "util.h"
#include "mythmainwindow.h"
#include "mythfontproperties.h"
#include "mythimage.h"
#include "dialogbox.h"

struct TextAttributes
{
    QRect textRect;
    MythFontProperties font;
    int textflags;
};

struct ButtonIcon
{
    QString name;
    MythImage *icon;
    MythImage *activeicon;
    MythImage *watermark;
    QPoint offset;
};

struct ThemedButton
{
    QPoint pos;
    QRect  posRect;

    ButtonIcon *buttonicon;
    QPoint iconPos;
    QRect iconRect;

    QString text;
    QString altText;
    QStringList action;

    int row;
    int col;

    int status;
    bool visible;
};

struct MenuRow
{
    int numitems;
    bool visible;
    vector<ThemedButton *> buttons;
};

struct MenuState
{
    QString name;
    int row;
    int col;
};

static QPoint parsePoint(const QString &text)
{
    int x, y;
    QPoint retval;
    if (sscanf(text.data(), "%d,%d", &x, &y) == 2)
        retval = QPoint(x, y);
    return retval;
}

static QRect parseRect(const QString &text)
{
    int x, y, w, h;
    QRect retval;
    if (sscanf(text.data(), "%d,%d,%d,%d", &x, &y, &w, &h) == 4)
        retval = QRect(x, y, w, h);

    return retval;
}

static QString getFirstText(QDomElement &element)
{
    for (QDomNode dname = element.firstChild(); !dname.isNull();
         dname = dname.nextSibling())
    {
        QDomText t = dname.toText();
        if (!t.isNull())
            return t.data();
    }
    return "";
}

class ThemedMenuPrivate;

class ThemedMenuState
{
  public:
    ThemedMenuState(float lwmult, float lhmult,
                    int lscreenwidth, int lscreenheight);
   ~ThemedMenuState();

    void parseSettings(const QString &dir, const QString &menuname);

    void parseBackground(const QString &dir, QDomElement &element);
    void parseLogo(const QString &dir, QDomElement &element);
    void parseArrow(const QString &dir, QDomElement &element, bool up);
    void parseTitle(const QString &dir, QDomElement &element);
    void parseButtonDefinition(const QString &dir, QDomElement &element);
    void parseButton(const QString &dir, QDomElement &element);

    void parseText(TextAttributes &attributes, QDomElement &element);
    void parseOutline(TextAttributes &attributes, QDomElement &element);
    void parseShadow(TextAttributes &attributes, QDomElement &element);

    void Reset(void);
    void setDefaults(void);

    QRect menuRect() const;

    void setTitleIcon(const QString &menumode);
    ButtonIcon *getButtonIcon(const QString &type);

    void paintLogo(MythPainter *p, int alpha);
    void paintTitle(MythPainter *p, int alpha);
    void paintWatermark(MythPainter *p, int alpha, MythImage *watermark);

    void drawScrollArrows(MythPainter *p, int alpha, bool up, bool down);

    float wmult;
    float hmult;

    int screenwidth;
    int screenheight;

    QRect buttonArea;

    QRect logoRect;
    MythImage *logo;

    MythImage *buttonnormal;
    MythImage *buttonactive;

    QMap<QString, ButtonIcon> allButtonIcons;

    QString prefix;

    TextAttributes normalAttributes;
    TextAttributes activeAttributes;

    void (*callback)(void *, QString &);
    void *callbackdata;

    bool killable;

    bool spreadbuttons;

    QMap<QString, MythImage *> titleIcons;
    MythImage *curTitle;
    QString titleText;
    QPoint titlePos;
    QRect titleRect;
    bool drawTitle;

    MythImage *uparrow;
    QRect uparrowRect;
    MythImage *downarrow;
    QRect downarrowRect;

    QPoint watermarkPos;
    QRect watermarkRect;

    bool allowreorder;

    int visiblerowlimit;

    bool loaded;

    QString themeDir;

//    LCD *lcddev;
};

class ThemedMenuPrivate
{
  public:
    ThemedMenuPrivate(ThemedMenu *lparent, const char *cdir, 
                      float lwmult, float lhmult,
                      int lscreenwidth, int lscreenheight, 
                      ThemedMenuState *lstate);
   ~ThemedMenuPrivate();

    bool keyPressHandler(QKeyEvent *e);
    void ReloadTheme(void);

    void parseMenu(const QString &menuname, int row = -1, int col = -1);

    void parseThemeButton(QDomElement &element);

    void addButton(const QString &type, const QString &text,
                   const QString &alttext, const QStringList &action);
    void layoutButtons(void);
    void positionButtons(bool resetpos);
    bool makeRowVisible(int newrow, int oldrow, bool forcedraw = true);

    bool handleAction(const QString &action);
    bool findDepends(const QString &file);
    QString findMenuFile(const QString &menuname);

    void paintButton(unsigned int button, MythPainter *p, int alpha);
    void paintWatermark(MythPainter *p, int alpha);

    void Draw(MythPainter *p, int xoffset, int yoffset, int alphaMod);

    void clearToBackground(void);
    void drawInactiveButtons(void);
    void drawScrollArrows(MythPainter *p, int alpha);
    bool checkPinCode(const QString &timestamp_setting,
                      const QString &password_setting,
                      const QString &text);

    ThemedMenu *parent;

    ThemedMenuState *m_state;
    bool allocedstate;

    QString prefix;

    vector<ThemedButton> buttonList;
    ThemedButton *activebutton;
    int currentrow;
    int currentcolumn;

    vector<MenuRow> buttonRows;

    QString selection;
    bool foundtheme;

    vector<MenuState> menufiles;

    int exitModifier;

    bool ignorekeys;

    int maxrows;
    int visiblerows;
    int columns;

    bool wantpop;
};

/////////////////////////////////////////////////////////////////////////////

ThemedMenuState::ThemedMenuState(float lwmult, float lhmult, int lscreenwidth,
                                 int lscreenheight)
{
    wmult = lwmult;
    hmult = lhmult;
    screenwidth = lscreenwidth;
    screenheight = lscreenheight;

    allowreorder = true;

    logo = NULL;
    buttonnormal = NULL;
    buttonactive = NULL;
    uparrow = NULL;
    downarrow = NULL;

    loaded = false;

//    lcddev = gContext->GetLCDDevice();
}

ThemedMenuState::~ThemedMenuState()
{
    Reset();
}

void ThemedMenuState::Reset(void)
{
    if (logo)
        delete logo;
    if (buttonnormal)
        delete buttonnormal;
    if (buttonactive)
        delete buttonactive;
    if (uparrow)
        delete uparrow;
    if (downarrow)
        delete downarrow;

    logo = NULL;
    buttonnormal = NULL;
    buttonactive = NULL;
    uparrow = NULL;
    downarrow = NULL;

    QMap<QString, ButtonIcon>::Iterator it;
    for (it = allButtonIcons.begin(); it != allButtonIcons.end(); ++it)
    {
        if (it.data().icon)
            delete it.data().icon;
        if (it.data().activeicon)
            delete it.data().activeicon;
        if (it.data().watermark)
            delete it.data().watermark;
    }
    allButtonIcons.clear();

    QMap<QString, MythImage *>::Iterator jt;
    for (jt = titleIcons.begin(); jt != titleIcons.end(); ++jt)
    {
        delete jt.data();
    }
    titleIcons.clear();

    loaded = false;
}

void ThemedMenuState::setTitleIcon(const QString &menumode)
{
    if (titleIcons.contains(menumode))
    {
        drawTitle = true;
        curTitle = titleIcons[menumode];
        titleRect = QRect(titlePos.x(), titlePos.y(), curTitle->width(),
                          curTitle->height());
    }
    else
    {
        drawTitle = false;
    }
}

ButtonIcon *ThemedMenuState::getButtonIcon(const QString &type)
{
    if (allButtonIcons.find(type) != allButtonIcons.end())
        return &(allButtonIcons[type]);
    return NULL;
}

void ThemedMenuState::parseBackground(const QString& /* dir */, QDomElement& element)
{
    // bool tiledbackground = false;
    // QPixmap *bground = NULL;    
    QString path;

    bool hasarea = false;

    spreadbuttons = true;
    visiblerowlimit = 6;    // the old default

    for (QDomNode child = element.firstChild(); !child.isNull();
         child = child.nextSibling())
    {
        QDomElement info = child.toElement();
        if (!info.isNull())
        {
            if (info.tagName() == "image")
            {
            }
            else if (info.tagName() == "buttonarea")
            {
                QRect tmpArea = parseRect(getFirstText(info));

                tmpArea.moveTopLeft(QPoint((int)(tmpArea.x() * wmult), 
                                           (int)(tmpArea.y() * hmult)));
                tmpArea.setWidth((int)(tmpArea.width() * wmult));
                tmpArea.setHeight((int)(tmpArea.height() * hmult));

                buttonArea = tmpArea;
                hasarea = true;
            }
            else if (info.tagName() == "buttonspread")
            {
                QString val = getFirstText(info);
                if (val == "no")
                    spreadbuttons = false;
            }
            else if (info.tagName() == "visiblerowlimit")
            {
                visiblerowlimit = atoi(getFirstText(info).ascii());
            }
            else
            {
                cerr << "Unknown tag " << info.tagName() << " in background\n";
                return;
            }
        }
    }

    if (!hasarea)
    {
        cerr << "Missing buttonaread in background\n";
        return;
    }
}

void ThemedMenuState::parseShadow(TextAttributes &attributes, 
                                  QDomElement &element)
{
    attributes.font.hasShadow = true;

    bool hascolor = false;
    bool hasoffset = false;
    bool hasalpha = false;

    for (QDomNode child = element.firstChild(); !child.isNull();
         child = child.nextSibling())
    {
        QDomElement info = child.toElement();
        if (!info.isNull())
        {
            if (info.tagName() == "color")
            {
                attributes.font.shadowColor = QColor(getFirstText(info));
                hascolor = true;
            }
            else if (info.tagName() == "offset")
            {
                QPoint p = parsePoint(getFirstText(info));
                attributes.font.shadowOffset.setX((int)(p.x() * wmult));
                attributes.font.shadowOffset.setY((int)(p.y() * hmult));
                hasoffset = true;
            }
            else if (info.tagName() == "alpha")
            {
                attributes.font.shadowAlpha = atoi(getFirstText(info).ascii());
                hasalpha = true;
            }
            else
            {
                cerr << "Unknown tag " << info.tagName() << " in text/shadow\n";
                return;
            }
        }
    }

    if (!hascolor)
    {
        cerr << "Missing color tag in shadow\n";
        return;
    }

    if (!hasalpha)
    {
        cerr << "Missing alpha tag in shadow\n";
        return;
    }

    if (!hasoffset)
    {
        cerr << "Missing offset tag in shadow\n";
        return;
    }
}

void ThemedMenuState::parseOutline(TextAttributes &attributes, 
                                   QDomElement &element)
{
    attributes.font.hasOutline = true;

    bool hascolor = false;
    bool hassize = false;

    for (QDomNode child = element.firstChild(); !child.isNull();
         child = child.nextSibling())
    {
        QDomElement info = child.toElement();
        if (!info.isNull())
        {
            if (info.tagName() == "color")
            {
                attributes.font.outlineColor = QColor(getFirstText(info));
                hascolor = true;
            }
            else if (info.tagName() == "size")
            {
                int size = atoi(getFirstText(info).ascii());
                attributes.font.outlineSize = (int)(size * hmult);
                hassize = true;
            }
            else
            {
                cerr << "Unknown tag " << info.tagName() << " in text/shadow\n";
                return;
            }
        }
    }

    if (!hassize)
    {
        cerr << "Missing size in outline\n";
        return;
    }

    if (!hascolor)
    {
        cerr << "Missing color in outline\n";
        return;
    }
}

void ThemedMenuState::parseText(TextAttributes &attributes, 
                                QDomElement &element)
{
    bool hasarea = false;

    int weight = QFont::Normal;
    int fontsize = 14;
    QString fontname = "Arial";
    bool italic = false;

    for (QDomNode child = element.firstChild(); !child.isNull();
         child = child.nextSibling())
    {
        QDomElement info = child.toElement();
        if (!info.isNull())
        {
            if (info.tagName() == "area") 
            {
                hasarea = true;
                attributes.textRect = parseRect(getFirstText(info));
                attributes.textRect.moveTopLeft(QPoint(
                                       (int)(attributes.textRect.x() * wmult),
                                       (int)(attributes.textRect.y() * hmult)));
                attributes.textRect.setWidth((int)
                                       (attributes.textRect.width() * wmult));
                attributes.textRect.setHeight((int)
                                       (attributes.textRect.height() * hmult));
                attributes.textRect = QRect(attributes.textRect.x(), 
                                            attributes.textRect.y(),
                                            buttonnormal->width() - 
                                            attributes.textRect.width() - 
                                            attributes.textRect.x(), 
                                            buttonnormal->height() - 
                                            attributes.textRect.height() - 
                                            attributes.textRect.y());
            }
            else if (info.tagName() == "fontsize")
            {
                fontsize = atoi(getFirstText(info).ascii());
            }
            else if (info.tagName() == "fontname")
            { 
                fontname = getFirstText(info);
            }
            else if (info.tagName() == "bold")
            {
                if (getFirstText(info) == "yes")
                    weight = QFont::Bold;
            }
            else if (info.tagName() == "italics")
            {
                if (getFirstText(info) == "yes")
                    italic = true;
            }
            else if (info.tagName() == "color")
            {
                attributes.font.color = QColor(getFirstText(info));
            }
            else if (info.tagName() == "centered")
            {
                if (getFirstText(info) == "yes")
                {
                    if (gContext->GetLanguage() == "ja")
                    {
                        attributes.textflags = Qt::AlignVCenter | 
                                               Qt::AlignHCenter |
                                               Qt::WordBreak;
                    }
                    else
                    {
                        attributes.textflags = Qt::AlignTop | Qt::AlignHCenter |
                                               Qt::WordBreak;
                    }
                }
            } 
            else if (info.tagName() == "outline")
            {
                parseOutline(attributes, info);
            }
            else if (info.tagName() == "shadow")
            {
                parseShadow(attributes, info);
            }
            else
            {
                cerr << "Unknown tag " << info.tagName() << " in text\n";
                return;
            }
        }
    }

    attributes.font.face = QFont(fontname, (int)ceil(fontsize * hmult), weight, 
                                 italic);

    if (!hasarea)
    {
        cerr << "Missing 'area' tag in 'text' element of 'genericbutton'\n";
        return;
    }
}

void ThemedMenuState::parseButtonDefinition(const QString &dir, 
                                            QDomElement &element)
{
    bool hasnormal = false;
    bool hasactive = false;
    bool hasactivetext = false;

    QString setting;

    QImage *tmp;

    for (QDomNode child = element.firstChild(); !child.isNull();
         child = child.nextSibling())
    {
        QDomElement info = child.toElement();
        if (!info.isNull())
        {
            if (info.tagName() == "normal")
            {
                setting = dir + getFirstText(info);
                tmp = gContext->LoadScaleImage(setting);
                if (tmp)
                {
                    buttonnormal = MythImage::FromQImage(tmp);
                    hasnormal = true;
                }
            }
            else if (info.tagName() == "active")
            {
                setting = dir + getFirstText(info);
                tmp = gContext->LoadScaleImage(setting);
                if (tmp)
                {
                    buttonactive = MythImage::FromQImage(tmp);
                    hasactive = true;
                }
            }
            else if (info.tagName() == "text")
            {
                if (!hasnormal)
                {
                    cerr << "The 'normal' tag needs to come before the "
                         << "'normaltext' tag\n";
                    return;
                }
                parseText(normalAttributes, info);
            }
            else if (info.tagName() == "activetext")
            {
                if (!hasactive)
                {
                    cerr << "The 'active' tag needs to come before the "
                         << "'activetext' tag\n";
                    return;
                }
                parseText(activeAttributes, info);
                hasactivetext = true;
            }
            else if (info.tagName() == "watermarkposition")
            {
                watermarkPos = parsePoint(getFirstText(info));
            }
            else
            {
                cerr << "Unknown tag " << info.tagName() 
                     << " in genericbutton\n";
                return;
            }
        }
    }

    if (!hasnormal)
    {
        cerr << "No normal button image defined\n";
        return;
    }

    if (!hasactive)
    {
        cerr << "No active button image defined\n";
        return;
    }

    if (!hasactivetext)
    {
        activeAttributes = normalAttributes;
    }

    watermarkPos.setX((int)(watermarkPos.x() * wmult));
    watermarkPos.setY((int)(watermarkPos.y() * hmult));

    watermarkRect = QRect(watermarkPos, QSize(0, 0));
}

void ThemedMenuState::parseLogo(const QString &dir, QDomElement &element)
{
    bool hasimage = false;
    bool hasposition = false;

    QPoint logopos;

    for (QDomNode child = element.firstChild(); !child.isNull();
         child = child.nextSibling())
    {
        QDomElement info = child.toElement();
        if (!info.isNull())
        {
            if (info.tagName() == "image")
            {
                QString logopath = dir + getFirstText(info);
                QImage *tmp = gContext->LoadScaleImage(logopath); 
                if (tmp)
                {
                    logo = MythImage::FromQImage(tmp);
                    hasimage = true;
                }
            }
            else if (info.tagName() == "position")
            {
                logopos = parsePoint(getFirstText(info));
                hasposition = true;
            }
            else
            {
                cerr << "Unknown tag " << info.tagName() << " in logo\n";
                return;
            }
        }
    }

    if (!hasimage)
    {
        cerr << "Missing image tag in logo\n";
        return;
    }

    if (!hasposition)
    {
        cerr << "Missing position tag in logo\n";
        return;
    }

    logopos.setX((int)(logopos.x() * wmult));
    logopos.setY((int)(logopos.y() * hmult));
    logoRect = QRect(logopos.x(), logopos.y(), logo->width(),
                     logo->height());
}

void ThemedMenuState::parseTitle(const QString &dir, QDomElement &element)
{
    bool hasimage = false;
    bool hasposition = false;

    for (QDomNode child = element.firstChild(); !child.isNull();
         child = child.nextSibling())
    {
        QDomElement info = child.toElement();
        if (!info.isNull())
        {
            if (info.tagName() == "image")
            {
                QString titlepath = dir + getFirstText(info);
                QImage *tmppix = gContext->LoadScaleImage(titlepath);

                if (!tmppix)
                    continue;

                QString name = info.attribute("mode", "");
                if (name != "")
                {
                    titleIcons[name] = MythImage::FromQImage(tmppix);
                }
                else
                {
                    cerr << "Missing mode in titles/image\n";
                    return;
                }

                hasimage = true;
            }
            else if (info.tagName() == "position")
            {
                titlePos = parsePoint(getFirstText(info));
                hasposition = true;
            }
            else
            {
                cerr << "Unknown tag " << info.tagName() << " in logo\n";
                return;
            }
        }
    }

    if (!hasimage)
    {
        cerr << "Missing image tag in titles\n";
        return;
    }

    if (!hasposition)
    {
        cerr << "Missing position tag in titles\n";
        return;
    }

    titlePos.setX((int)(titlePos.x() * wmult));
    titlePos.setY((int)(titlePos.y() * hmult));
}

void ThemedMenuState::parseArrow(const QString &dir, QDomElement &element, 
                                   bool up)
{
    QRect arrowrect;
    QPoint arrowpos;
    QImage *pix = NULL;    

    bool hasimage = false;
    bool hasposition = false;

    for (QDomNode child = element.firstChild(); !child.isNull();
         child = child.nextSibling())
    {
        QDomElement info = child.toElement();
        if (!info.isNull())
        {
            if (info.tagName() == "image")
            {
                QString arrowpath = dir + getFirstText(info);
                pix = gContext->LoadScaleImage(arrowpath);
                if (pix)
                    hasimage = true;
            }
            else if (info.tagName() == "position")
            {
                arrowpos = parsePoint(getFirstText(info));
                hasposition = true;
            }
            else
            {
                cerr << "Unknown tag " << info.tagName() << " in arrow\n";
                return;
            }
        }
    }

    if (!hasimage)
    {
        cerr << "Missing image tag in arrow\n";
        return;
    }

    if (!hasposition)
    {
        cerr << "Missing position tag in arrow\n";
        return;
    }

    arrowpos.setX((int)(arrowpos.x() * wmult));
    arrowpos.setY((int)(arrowpos.y() * hmult));
    arrowrect = QRect(arrowpos.x(), arrowpos.y(), pix->width(),
                      pix->height());

    if (up)
    {
        uparrow = MythImage::FromQImage(pix);
        uparrowRect = arrowrect;
    }
    else
    {
        downarrow = MythImage::FromQImage(pix);
        downarrowRect = arrowrect;
    }
}

void ThemedMenuState::parseButton(const QString &dir, QDomElement &element)
{
    bool hasname = false;
    bool hasoffset = false;
    bool hasicon = false;

    QString name = "";
    QImage *image = NULL;
    QImage *activeimage = NULL;
    QImage *watermark = NULL;
    QPoint offset;

    name = element.attribute("name", "");
    if (name != "")
        hasname = true;

    for (QDomNode child = element.firstChild(); !child.isNull();
         child = child.nextSibling())
    {    
        QDomElement info = child.toElement();
        if (!info.isNull())
        {
            if (info.tagName() == "image")
            {
                QString imagepath = dir + getFirstText(info); 
                image = gContext->LoadScaleImage(imagepath);
                if (image)
                    hasicon = true;
            }
            else if (info.tagName() == "activeimage")
            {
                QString imagepath = dir + getFirstText(info);
                activeimage = gContext->LoadScaleImage(imagepath);
            }
            else if (info.tagName() == "offset")
            {
                offset = parsePoint(getFirstText(info));
                offset.setX((int)(offset.x() * wmult));
                offset.setY((int)(offset.y() * hmult));
                hasoffset = true;
            }
            else if (info.tagName() == "watermarkimage")
            {
                QString imagepath = dir + getFirstText(info);
                watermark = gContext->LoadScaleImage(imagepath);
            }    
            else
            {
                cerr << "Unknown tag " << info.tagName() << " in buttondef\n";
                return;
            }
        }
    }

    if (!hasname)
    {
        cerr << "Missing name in button\n";
        return;
    }

    if (!hasoffset)
    {
        cerr << "Missing offset in buttondef " << name << endl;
        return;
    }

    if (!hasicon) 
    {
        cerr << "Missing image in buttondef " << name << endl;
        return;
    }

    ButtonIcon newbutton;

    newbutton.name = name;
    newbutton.icon = MythImage::FromQImage(image);
    newbutton.offset = offset;
    newbutton.activeicon = MythImage::FromQImage(activeimage);

    if (watermark)
    {
        if (watermark->width() > watermarkRect.width())
            watermarkRect.setWidth(watermark->width());

        if (watermark->height() > watermarkRect.height())
            watermarkRect.setHeight(watermark->height());
    }

    newbutton.watermark = MythImage::FromQImage(watermark);

    allButtonIcons[name] = newbutton;
}

void ThemedMenuState::setDefaults(void)
{
    logo = NULL;
    buttonnormal = buttonactive = NULL;

    normalAttributes.textflags = Qt::AlignTop | Qt::AlignLeft | Qt::WordBreak;
    activeAttributes.textflags = Qt::AlignTop | Qt::AlignLeft | Qt::WordBreak;

    titleIcons.clear();
    titleText = "";
    curTitle = NULL;
    drawTitle = false;
    uparrow = NULL;
    downarrow = NULL;
    watermarkPos = QPoint(0, 0);
    watermarkRect = QRect(0, 0, 0, 0);
}

void ThemedMenuState::parseSettings(const QString &dir, const QString &menuname)
{
    QString filename = dir + menuname;

    QDomDocument doc;
    QFile f(filename);

    if (!f.open(IO_ReadOnly))
    {
        cerr << "ThemedMenuState::parseSettings(): Can't open: " 
             << filename << endl;
        return;
    }

    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;

    if (!doc.setContent(&f, false, &errorMsg, &errorLine, &errorColumn))
    {
        cerr << "Error parsing: " << filename << endl;
        cerr << "at line: " << errorLine << "  column: " << errorColumn << endl;
        cerr << errorMsg << endl;
        f.close();
        return;
    }

    f.close();

    bool setbackground = false;
    bool setbuttondef = false;

    setDefaults();

    QDomElement docElem = doc.documentElement();
    QDomNode n = docElem.firstChild();
    while (!n.isNull())
    {
        QDomElement e = n.toElement();
        if (!e.isNull())
        {
            if (e.tagName() == "background")
            {
                parseBackground(dir, e);
                setbackground = true;
            }
            else if (e.tagName() == "genericbutton")
            {
                parseButtonDefinition(dir, e);
                setbuttondef = true;
            }
            else if (e.tagName() == "logo")
            {
                parseLogo(dir, e);
            }
            else if (e.tagName() == "buttondef")
            {
                parseButton(dir, e);
            }
            else if (e.tagName() == "titles")
            {
                parseTitle(dir, e);
            }
            else if (e.tagName() == "uparrow")
            {
                parseArrow(dir, e, true);
            }
            else if (e.tagName() == "downarrow")
            {
                parseArrow(dir, e, false);
            }
            else
            {
                cerr << "Unknown element " << e.tagName() << endl;
                return;
            }
        }
        n = n.nextSibling();
    }

    if (!setbackground)
    {
        cerr << "Missing background element\n";
        return;
    }

    if (!setbuttondef)
    {
        cerr << "Missing genericbutton definition\n";
        return;
    }

    loaded = true;
}

QRect ThemedMenuState::menuRect() const
{
    QRect r(0, 0, screenwidth, screenheight);
    return r;
}

void ThemedMenuState::drawScrollArrows(MythPainter *p, int alpha, bool up,
                                       bool down)
{
    if (!uparrow || !downarrow)
        return;

    if (!up && !down)
        return;

    if (up)
        p->DrawImage(uparrowRect.topLeft(), uparrow, alpha);
    if (down)
        p->DrawImage(downarrowRect.topLeft(), downarrow, alpha);
}

void ThemedMenuState::paintLogo(MythPainter *p, int alpha)
{
    if (logo)
        p->DrawImage(logoRect.topLeft(), logo, alpha);
}

void ThemedMenuState::paintTitle(MythPainter *p, int alpha)
{
    if (curTitle)
        p->DrawImage(titleRect.topLeft(), curTitle, alpha);
}

void ThemedMenuState::paintWatermark(MythPainter *p, int alpha, 
                                     MythImage *watermark)
{
    if (watermark)
    {
        p->DrawImage(watermarkPos, watermark, alpha);
    }
}

/////////////////////////////////////////////////////////////////////////////

ThemedMenuPrivate::ThemedMenuPrivate(ThemedMenu *lparent, const char *cdir,
                                     float lwmult, float lhmult, 
                                     int lscreenwidth, int lscreenheight, 
                                     ThemedMenuState *lstate)
{
    if (!lstate)
    {
        m_state = new ThemedMenuState(lwmult, lhmult, lscreenwidth,
                                      lscreenheight);
        allocedstate = true;
    }
    else
    {
        m_state = lstate;
        allocedstate = false;
    }

    parent = lparent;
    ignorekeys = false;
    wantpop = false;

    m_state->themeDir = cdir;
}

ThemedMenuPrivate::~ThemedMenuPrivate()
{
    if (allocedstate)
        delete m_state;
}

void ThemedMenuPrivate::parseThemeButton(QDomElement &element)
{
    QString type = "";
    QString text = "";
    QStringList action;
    QString alttext = "";

    bool addit = true;

    for (QDomNode child = element.firstChild(); !child.isNull();
         child = child.nextSibling())
    {
        QDomElement info = child.toElement();
        if (!info.isNull())
        {
            if (info.tagName() == "type")
            {
                type = getFirstText(info);
            }
            else if (info.tagName() == "text")
            {
                if ((text.isNull() || text.isEmpty()) && 
                    info.attribute("lang","") == "")
                {
                    text = getFirstText(info);
                }
                else if (info.attribute("lang","").lower() == 
                         gContext->GetLanguage())
                {
                    text = getFirstText(info);
                }
            }
            else if (info.tagName() == "alttext")
            {
                if ((alttext.isNull() || alttext.isEmpty()) &&
                    info.attribute("lang","") == "")
                {
                    alttext = getFirstText(info);
                }
                else if (info.attribute("lang","").lower() ==
                         gContext->GetLanguage())
                {
                    alttext = getFirstText(info);
                }
            }
            else if (info.tagName() == "action")
            {
                action += getFirstText(info);
            }
            else if (info.tagName() == "depends")
            {
                addit = findDepends(getFirstText(info));
            }
            else if (info.tagName() == "dependssetting")
            {
                addit = gContext->GetNumSetting(getFirstText(info));
            }
            else if (info.tagName() == "dependjumppoint")
            {
                addit = GetMythMainWindow()->DestinationExists(getFirstText(info));
            }
            else
            {
                cerr << "Unknown tag " << info.tagName() << " in button\n";
                return;
            }
        }
    }

    if (text == "")
    {
        cerr << "Missing 'text' in button\n";
        return;
    }
   
    if (action.empty())
    {
        cerr << "Missing 'action' in button\n";
        return;
    }

    if (addit)
        addButton(type, text, alttext, action);
}

void ThemedMenuPrivate::parseMenu(const QString &menuname, int row, int col)
{
    QString filename = findMenuFile(menuname);

    QDomDocument doc;
    QFile f(filename);

    if (!f.open(IO_ReadOnly))
    {
        
        cerr << "Couldn't read menu file " << menuname << endl;
        if(menuname == "mainmenu.xml" )
        {
            exit(0);
        }
        else
        {
            parent->GetScreenStack()->PopScreen();
#if 0
            MythPopupBox::showOkPopup(gContext->GetMainWindow(), QObject::tr("No Menu File"),
                                      QObject::tr(QString("Myth could not locate the menu file %1.\n\n"
                                      "We will now return to the main menu.").arg(menuname)));
#endif
            return;
        }
        
        
    }

    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;

    if (!doc.setContent(&f, false, &errorMsg, &errorLine, &errorColumn))
    {
        cerr << "Error parsing: " << filename << endl;
        cerr << "at line: " << errorLine << "  column: " << errorColumn << endl;
        cerr << errorMsg << endl;
        f.close();

        if (menuname == "mainmenu.xml" )
        {
            exit(0);
        }

        parent->GetScreenStack()->PopScreen();
#if 0
        MythPopupBox::showOkPopup(gContext->GetMainWindow(), 
                                  QObject::tr("Bad Menu File"),
                                  QObject::tr(QString("The menu file %1 is "
                                              "incomplete.\n\nWe will now "
                                              "return to the main menu.")
                                              .arg(menuname)));
#endif
        return;
    }

    f.close();

    buttonList.clear();
    buttonRows.clear();

    QDomElement docElem = doc.documentElement();

    QString menumode = docElem.attribute("name", "");

    QDomNode n = docElem.firstChild();
    while (!n.isNull())
    {
        QDomElement e = n.toElement();
        if (!e.isNull())
        {
            if (e.tagName() == "button")
            {
                parseThemeButton(e);
            }
            else
            {
                cerr << "Unknown element " << e.tagName() << endl;
                exit(1);
            }
        }
        n = n.nextSibling();
    }

    if (buttonList.size() == 0)
    {
        cerr << "No buttons for menu " << menuname << endl;
        exit(1);
    }

    layoutButtons();
    positionButtons(true);

    if (row != -1 && col != -1)
    {
        int oldrow = currentrow;

        if (row < (int)buttonRows.size() && col < buttonRows[row].numitems)
        {
            currentrow = row;
            currentcolumn = col;
        }

        while (!buttonRows[currentrow].visible)
        {
            makeRowVisible(oldrow + 1, oldrow, false);
            oldrow = oldrow + 1;
        }

        activebutton = buttonRows[currentrow].buttons[currentcolumn];
    }

    MenuState state;
    state.name = menuname;
    state.row = currentrow;
    state.col = currentcolumn;
    menufiles.push_back(state);

    m_state->setTitleIcon(menumode);

    drawInactiveButtons();

#if 0
    if (lcddev)
    {
        titleText = "MYTH-";
        titleText += menumode;
        QPtrList<LCDMenuItem> menuItems;
        menuItems.setAutoDelete(true);
        bool selected;

        for (int r = 0; r < (int)buttonRows.size(); r++)
        {
            if (r == currentrow)
                selected = true;
            else
                selected = false;

            if (currentcolumn < buttonRows[r].numitems)
                menuItems.append(new LCDMenuItem(selected, NOTCHECKABLE,
                                 buttonRows[r].buttons[currentcolumn]->text));
        }

        if (!menuItems.isEmpty())
            lcddev->switchToMenu(&menuItems, titleText);
    }
#endif

    selection = "";
    parent->SetRedraw();
#if 0
    parent->update(menuRect());
#endif
}

void ThemedMenuPrivate::addButton(const QString &type, const QString &text, 
                                  const QString &alttext, 
                                  const QStringList &action)
{
    ThemedButton newbutton;

    newbutton.buttonicon = m_state->getButtonIcon(type);
    newbutton.text = text;
    newbutton.altText = alttext;
    newbutton.action = action;
    newbutton.status = -1;
    newbutton.visible = false;

    buttonList.push_back(newbutton);
}

void ThemedMenuPrivate::layoutButtons(void)
{
    int numbuttons = buttonList.size();
  
    columns = m_state->buttonArea.width() / m_state->buttonnormal->width();
    maxrows = m_state->buttonArea.height() / m_state->buttonnormal->height();

    if (maxrows < 2)
    {
        cerr << "Must have room for at least 2 rows of buttons\n";
        exit(1);
    }
    
    if (columns < 1)
    {
        cerr << "Must have room for at least 1 column of buttons\n";
        exit(1);
    }

    // keep the rows balanced
    if (numbuttons <= 4)
    {
        if (columns > 2)
            columns = 2;
    }
    else
    {
        if (columns > 3)
            columns = 3;
    }    

    // limit it to 6 items displayed at one time
    if (columns * maxrows > m_state->visiblerowlimit)
    {
        maxrows = m_state->visiblerowlimit / columns;
    }
                             
    vector<ThemedButton>::iterator iter = buttonList.begin();

    int rows = numbuttons / columns;
    rows++;

    visiblerows = 0;

    for (int i = 0; i < rows; i++)
    {
        MenuRow newrow;
        newrow.numitems = 0;

        for (int j = 0; j < columns && iter != buttonList.end(); 
             j++, iter++)
        {
            if (columns == 3 && j == 1 && m_state->allowreorder)
                newrow.buttons.insert(newrow.buttons.begin(), &(*iter));
            else
                newrow.buttons.push_back(&(*iter));
            newrow.numitems++;
        }

        if (i < maxrows && newrow.numitems > 0)
        {
            newrow.visible = true;
            visiblerows++;
        }
        else
            newrow.visible = false;
 
        if (newrow.numitems > 0)
            buttonRows.push_back(newrow);
    }            
}

void ThemedMenuPrivate::positionButtons(bool resetpos)
{
    QRect buttonArea = m_state->buttonArea;
    int buttonHeight = m_state->buttonnormal->height();
    int buttonWidth = m_state->buttonnormal->width();

    int rows = visiblerows;
    int yspacing = (buttonArea.height() - buttonHeight * rows) /
                   (rows + 1);
    int ystart = 0;
    
    if (!m_state->spreadbuttons)
    {
        yspacing = 0;
        ystart = (buttonArea.height() - buttonHeight * rows) / 2;
    }

    int row = 1;

    vector<MenuRow>::iterator menuiter = buttonRows.begin();
    for (; menuiter != buttonRows.end(); menuiter++)
    {
        if (!(*menuiter).visible)
        {
            vector<ThemedButton *>::iterator biter;
            biter = (*menuiter).buttons.begin();
            for (; biter != (*menuiter).buttons.end(); biter++)
            {
                ThemedButton *tbutton = (*biter);
                tbutton->visible = false;
            }
            continue;
        }

        int ypos = yspacing * row + (buttonHeight * (row - 1));
        ypos += buttonArea.y() + ystart;

        int xspacing = (buttonArea.width() - buttonWidth *
                       (*menuiter).numitems) / ((*menuiter).numitems + 1);
        int col = 1;
        vector<ThemedButton *>::iterator biter = (*menuiter).buttons.begin();
        for (; biter != (*menuiter).buttons.end(); biter++)
        {
            int xpos = xspacing * col + (buttonWidth * (col - 1));
            xpos += buttonArea.x();

            ThemedButton *tbutton = (*biter);

            tbutton->visible = true;
            tbutton->row = row;
            tbutton->col = col;
            tbutton->pos = QPoint(xpos, ypos);
            tbutton->posRect = QRect(tbutton->pos.x(), tbutton->pos.y(),
                                     buttonWidth, buttonHeight);

            if (tbutton->buttonicon)
            {
                tbutton->iconPos = tbutton->buttonicon->offset + tbutton->pos;
                tbutton->iconRect = QRect(tbutton->iconPos.x(),
                                          tbutton->iconPos.y(),
                                          tbutton->buttonicon->icon->width(),
                                          tbutton->buttonicon->icon->height());
            }
            
            col++;
        }

        row++;
    }

    if (resetpos)
    {
        activebutton = &(*(buttonList.begin()));
        currentrow = activebutton->row - 1;
        currentcolumn = activebutton->col - 1;
    }
}

bool ThemedMenuPrivate::makeRowVisible(int newrow, int oldrow, bool forcedraw)
{
    if (buttonRows[newrow].visible)
        return true;

    if (newrow > oldrow)
    {
        int row;
        for (row = newrow; row >= 0; row--)
        {
            if (row > newrow - visiblerows)
                buttonRows[row].visible = true;
            else
                buttonRows[row].visible = false;
        }
    }
    else
    {
        int row;
        for (row = newrow; row < (int)buttonRows.size(); row++)
        {
            if (row < newrow + visiblerows)
                buttonRows[row].visible = true;
            else
                buttonRows[row].visible = false;
        }
    }

    positionButtons(false);

    if (forcedraw)
        clearToBackground();

    return true;
}

void ThemedMenuPrivate::clearToBackground(void)
{
#if 0
    drawInactiveButtons();
#endif
}

void ThemedMenuPrivate::drawInactiveButtons(void)
{
#if 0
    QPainter tmp(&bground);

    paintLogo(&tmp);
    paintTitle(&tmp);

    ThemedButton *store = activebutton;
    activebutton = NULL;
    
    for (unsigned int i = 0; i < buttonList.size(); i++)
    {
        paintButton(i, &tmp, true, true);
    }

    drawScrollArrows(&tmp);

    activebutton = store;

    tmp.end();

    parent->setPaletteBackgroundPixmap(bground);

    parent->erase(buttonArea);
    parent->erase(uparrowRect);
    parent->erase(downarrowRect);
    parent->erase(logoRect);
    if (drawTitle)
        parent->erase(titleRect);
#endif
}

void ThemedMenuPrivate::drawScrollArrows(MythPainter *p, int alpha)
{
    bool needup = false;
    bool needdown = false;

    if (!buttonRows.front().visible)
        needup = true;
    if (!buttonRows.back().visible)
        needdown = true;

    if (!needup && !needdown)
        return;

    m_state->drawScrollArrows(p, alpha, needup, needdown);
}

void ThemedMenuPrivate::paintWatermark(MythPainter *p, int alpha)
{
    if (activebutton && activebutton->buttonicon &&
        activebutton->buttonicon->watermark)
    {
        m_state->paintWatermark(p, alpha,  activebutton->buttonicon->watermark);
    }
}

void ThemedMenuPrivate::paintButton(unsigned int button, MythPainter *p, 
                                    int alpha)
{
    TextAttributes attributes;
    ThemedButton *tbutton = &(buttonList[button]);

    if (!tbutton->visible)
        return;

    QRect cr;
    if (tbutton->buttonicon)
        cr = tbutton->posRect.unite(tbutton->iconRect);
    else
        cr = tbutton->posRect;

#if 0
    if (!erased)
    {
        if (tbutton->status == 1 && activebutton == tbutton)
            return;
        if (tbutton->status == 0 && activebutton != tbutton)
            return;
    }
#endif

#if 0
    QRect newRect(0, 0, tbutton->posRect.width(), tbutton->posRect.height());
    newRect.moveBy(tbutton->posRect.x() - cr.x(), 
                   tbutton->posRect.y() - cr.y());
#endif
    QRect newRect = tbutton->posRect;

    MythImage *buttonback = NULL;
    if (tbutton == activebutton)
    {
        buttonback = m_state->buttonactive;
        tbutton->status = 1;
        attributes = m_state->activeAttributes;
    }
    else
    {
        tbutton->status = 0;
#if 0
        if (!drawinactive)
        {
            parent->erase(cr);
            return;
        }
#endif
        buttonback = m_state->buttonnormal;
        attributes = m_state->normalAttributes;
    }

    p->DrawImage(newRect.topLeft(), buttonback, alpha);

    QRect buttonTextRect = attributes.textRect;
    buttonTextRect.moveBy(newRect.x(), newRect.y());

    QString message = tbutton->text;

    QRect testBoundRect = buttonTextRect;
    testBoundRect.addCoords(0, 0, 0, 40);
#if 0
    QRect testBound = tmp.boundingRect(testBoundRect, attributes.textflags, 
                                       message);

    if (testBound.height() > buttonTextRect.height() && tbutton->altText != "")
        message = buttonList[button].altText;
#endif

    p->DrawText(buttonTextRect, message, attributes.textflags, 
                attributes.font, alpha);

    if (buttonList[button].buttonicon)
    {
        MythImage *blendImage = tbutton->buttonicon->icon;

        if (tbutton == activebutton && tbutton->buttonicon->activeicon)
            blendImage = tbutton->buttonicon->activeicon;

        p->DrawImage(tbutton->iconRect.topLeft(), blendImage, alpha);
    }
}

void ThemedMenuPrivate::ReloadTheme(void)
{
    buttonList.clear();
    buttonRows.clear();

    parent->ReloadExitKey();
 
    m_state->Reset();
 
#if 0
    gContext->GetScreenSettings(screenwidth, wmult, screenheight, hmult);
    parent->setFixedSize(QSize(screenwidth, screenheight));

    parent->setFont(gContext->GetMediumFont());
    parent->setCursor(QCursor(Qt::BlankCursor));

    gContext->ThemeWidget(parent);
#endif

    QString themedir = gContext->GetThemeDir();
    m_state->parseSettings(themedir, "theme.xml");

    QString file = menufiles.back().name;
    int row = menufiles.back().row;
    int col = menufiles.back().col;
    menufiles.pop_back();

    parseMenu(file, row, col);
}

bool ThemedMenuPrivate::keyPressHandler(QKeyEvent *e)
{
    ThemedButton *lastbutton = activebutton;
    int oldrow = currentrow;
    bool handled = false;

    QStringList actions;
    GetMythMainWindow()->TranslateKeyPress("menu", e, actions);

    for (unsigned int i = 0; i < actions.size() && !handled; i++)
    {
        QString action = actions[i];
        handled = true;

        if (columns == 1)
        {
            if (action == "LEFT")
                action = "ESCAPE";
            else if (action == "RIGHT")
                action = "SELECT";
        }

        if (action == "UP")
        { 
            if (currentrow > 0)
                currentrow--;
            else if (columns == 1)
                currentrow = buttonRows.size() - 1;

            if (currentcolumn >= buttonRows[currentrow].numitems)
                currentcolumn = buttonRows[currentrow].numitems - 1;
        }
        else if (action == "PAGEUP")
        {
            currentrow = max(currentrow - m_state->visiblerowlimit, 0);

            if (currentcolumn >= buttonRows[currentrow].numitems)
                currentcolumn = buttonRows[currentrow].numitems - 1;
        }
        else if (action == "LEFT")
        {
            if (currentcolumn > 0)
                currentcolumn--;
        }
        else if (action == "DOWN")
        {
            if (currentrow < (int)buttonRows.size() - 1)
                currentrow++;
            else if (columns == 1)
                currentrow = 0;

            if (currentcolumn >= buttonRows[currentrow].numitems)
                currentcolumn = buttonRows[currentrow].numitems - 1;
        }
        else if (action == "PAGEDOWN")
        {
            currentrow = min(currentrow + m_state->visiblerowlimit,
                             (int)buttonRows.size() - 1);

            if (currentcolumn >= buttonRows[currentrow].numitems)
                currentcolumn = buttonRows[currentrow].numitems - 1;
        }
        else if (action == "RIGHT")
        {
            if (currentcolumn < buttonRows[currentrow].numitems - 1)
                currentcolumn++;
        }
        else if (action == "SELECT")
        {
#if 0
            if (lcddev)
                lcddev->stopAll();
#endif

            lastbutton = activebutton;
            activebutton = NULL;
            parent->SetRedraw();
#if 0
            parent->repaint(lastbutton->posRect);
#endif

            QStringList::Iterator it = lastbutton->action.begin();
            for (; it != lastbutton->action.end(); it++)
            {
                if (handleAction(*it))
                    break;
            }

            lastbutton = NULL;
        }
        else if (action == "ESCAPE")
        {
            QString action = "UPMENU";
            if (!allocedstate)
                handleAction(action);
            else if (m_state->killable || e->state() == exitModifier)
            {
                wantpop = true;
            }
            lastbutton = NULL;
        }
        else
            handled = false;
    }

    if (!handled)
        return false;

    if (!buttonRows[currentrow].visible)
    {
        makeRowVisible(currentrow, oldrow);
        lastbutton = NULL;
    }

    activebutton = buttonRows[currentrow].buttons[currentcolumn];
#if 0
    if (lcddev)
    {
        // Build a list of the menu items
        QPtrList<LCDMenuItem> menuItems;
        menuItems.setAutoDelete(true);
        bool selected;
        for (int r = 0; r < (int)buttonRows.size(); r++)
        {
            if (r == currentrow)
                selected = true;
            else
                selected = false;

            if (currentcolumn < buttonRows[r].numitems)
                menuItems.append(new LCDMenuItem(selected, NOTCHECKABLE,
                                 buttonRows[r].buttons[currentcolumn]->text));
        }

        if (!menuItems.isEmpty())
            lcddev->switchToMenu(&menuItems, titleText);
    }
#endif

    parent->SetRedraw();
#if 0
    parent->update(watermarkRect);
    if (lastbutton)
        parent->update(lastbutton->posRect);
    parent->update(activebutton->posRect);
#endif

    return true;
} 

QString ThemedMenuPrivate::findMenuFile(const QString &menuname)
{
    QString testdir = QDir::homeDirPath() + "/.mythtv/" + menuname;
    QFile file(testdir);
    if (file.exists())
        return testdir;

    testdir = gContext->GetMenuThemeDir() + "/" + menuname;
    file.setName(testdir);
    if (file.exists())
        return testdir;

        
    testdir = gContext->GetThemeDir() + "/" + menuname;
    file.setName(testdir);
    if (file.exists())
        return testdir;
        
    testdir = prefix + "/share/mythtv/" + menuname;
    file.setName(testdir);
    if (file.exists())
        return testdir;
        
    testdir = "../mythfrontend/" + menuname;
    file.setName(testdir);
    if (file.exists())
        return testdir;
        
    return "";
}

bool ThemedMenuPrivate::handleAction(const QString &action)
{
    if (action.left(5) == "EXEC ")
    {
        QString rest = action.right(action.length() - 5);
        myth_system(rest);

        return false;
    }
    else if (action.left(7) == "EXECTV ")
    {
        QString rest = action.right(action.length() - 7).stripWhiteSpace();
        QStringList strlist = QString("LOCK_TUNER");
        gContext->SendReceiveStringList(strlist);
        int cardid = strlist[0].toInt();

        if (cardid >= 0)
        {
            rest = rest.sprintf(rest,
                                (const char*)strlist[1],
                                (const char*)strlist[2],
                                (const char*)strlist[3]);

            myth_system(rest);

            strlist = QString("FREE_TUNER %1").arg(cardid);
            gContext->SendReceiveStringList(strlist);
            QString ret = strlist[0];
        }
        else
        {
            if (cardid == -2)
                cerr << "themedmenu.o: Tuner already locked" << endl;
           
#if 0 
            DialogBox *error_dialog = new DialogBox(gContext->GetMainWindow(),
                    "\n\nAll tuners are currently in use. If you want to watch "
                    "TV, you can cancel one of the in-progress recordings from "
                    "the delete menu");
            error_dialog->AddButton("OK");
            error_dialog->exec();
            delete error_dialog;
#endif
        }
    }
    else if (action.left(5) == "MENU ")
    {
        QString rest = action.right(action.length() - 5);

        menufiles.back().row = currentrow;
        menufiles.back().col = currentcolumn;

        if (rest == "main_settings.xml" && 
            gContext->GetNumSetting("SetupPinCodeRequired", 0) &&
            !checkPinCode("SetupPinCodeTime", "SetupPinCode", "Setup Pin:"))
        {
            return true;
        }

        MythScreenStack *stack = parent->GetScreenStack();

        ThemedMenu *newmenu = new ThemedMenu(m_state->themeDir.local8Bit(),
                                             rest, stack, "menu", 
                                             m_state->allowreorder, m_state);
        stack->AddScreen(newmenu);
    }
    else if (action.left(6) == "UPMENU")
    {
        wantpop = true;
    }
    else if (action.left(12) == "CONFIGPLUGIN")
    {
#if 0
        QString rest = action.right(action.length() - 13);
        MythPluginManager *pmanager = gContext->getPluginManager();
        if (pmanager)
            pmanager->config_plugin(rest.stripWhiteSpace());
#endif
    }
    else if (action.left(6) == "PLUGIN")
    {
#if 0
        QString rest = action.right(action.length() - 7);
        MythPluginManager *pmanager = gContext->getPluginManager();
        if (pmanager)
            pmanager->run_plugin(rest.stripWhiteSpace());
#endif
    }
    else if (action.left(8) == "SHUTDOWN")
    {
        if (allocedstate)
        {
            wantpop = true;
        }
    }
    else if (action.left(5) == "JUMP ")
    {
        QString rest = action.right(action.length() - 5);
        GetMythMainWindow()->JumpTo(rest);
    }
    else
    {
        selection = action;
        if (m_state->callback != NULL)
        {
            m_state->callback(m_state->callbackdata, selection);
        }
    }

    return true;   
}

bool ThemedMenuPrivate::findDepends(const QString &file)
{
    QString filename = findMenuFile(file);
    if (filename != "")
        return true;

    QString newname = QString(PREFIX) + "/lib/mythtv/plugins/lib" + file +
                      ".so";

    QFile checkFile(newname);
    if (checkFile.exists())
        return true;

    return false;
}

bool ThemedMenuPrivate::checkPinCode(const QString &timestamp_setting, 
                              const QString &password_setting,
                              const QString& /* text */)
{
    QDateTime curr_time = QDateTime::currentDateTime();
    QString last_time_stamp = gContext->GetSetting(timestamp_setting);
    QString password = gContext->GetSetting(password_setting);

    if (password.length() < 1)
        return true;

    if (last_time_stamp.length() < 1)
    {
        cerr << "themedmenu.o: Could not read password/pin time stamp. "
             << "This is only an issue if it happens repeatedly. " << endl;
    }
    else
    {
        QDateTime last_time = QDateTime::fromString(last_time_stamp, 
                                                    Qt::TextDate);
        if (last_time.secsTo(curr_time) < 120)
        {
            last_time_stamp = curr_time.toString(Qt::TextDate);
            gContext->SetSetting(timestamp_setting, last_time_stamp);
            gContext->SaveSetting(timestamp_setting, last_time_stamp);
            return true;
        }
    }

    if (password.length() > 0)
    {
        bool ok = false;
#if 0
        MythPasswordDialog *pwd = new MythPasswordDialog(text, &ok, password,
                                                     gContext->GetMainWindow());
        pwd->exec();
        delete pwd;
#endif
        if (ok)
        {
            last_time_stamp = curr_time.toString(Qt::TextDate);
            gContext->SetSetting(timestamp_setting, last_time_stamp);
            gContext->SaveSetting(timestamp_setting, last_time_stamp);
            return true;
        }
    }
    else
    {
        return true;
    }

    return false;
}

void ThemedMenuPrivate::Draw(MythPainter *p, 
                             int /* xoffset */, 
                             int /* yoffset */, 
                             int alphaMod)
{
#if 0
    d->paintLogo(p, alphaMod);
#endif

    m_state->paintTitle(p, alphaMod);

    paintWatermark(p, alphaMod);

    for (unsigned int i = 0; i < buttonList.size(); i++)
    {
        paintButton(i, p, alphaMod);
    }

    drawScrollArrows(p, alphaMod);
}

////////////////////////////////////////////////////////////////////////////

ThemedMenu::ThemedMenu(const char *cdir, const char *menufile,
                       MythScreenStack *parent, const char *name,
                       bool allowreorder, ThemedMenuState *state)
          : MythScreenType(parent, name)
{
    float wmult = 1, hmult = 1;
    int screenwidth = 800;
    int screenheight = 600;

    d = new ThemedMenuPrivate(this, cdir, wmult, hmult, 
                              screenwidth, screenheight,
                              state);
    d->m_state->allowreorder = allowreorder;

    Init(cdir, menufile);
}

void ThemedMenu::Init(const char *cdir, const char *menufile)
{
    QString dir = QString(cdir) + "/";
    QString filename = dir + "theme.xml";

    d->foundtheme = true;
    QFile filetest(filename);
    if (!filetest.exists())
    {
        d->foundtheme = false;
        return;
    }

    d->prefix = d->m_state->prefix = gContext->GetInstallPrefix();

    d->m_state->callback = NULL;

    ReloadExitKey();

    if (!d->m_state->loaded)
        d->m_state->parseSettings(dir, "theme.xml");

    d->parseMenu(menufile);
}

ThemedMenu::~ThemedMenu(void)
{
    if (d)
        delete d;
}

bool ThemedMenu::foundTheme(void)
{
    return d->foundtheme;
}

void ThemedMenu::setCallback(void (*lcallback)(void *, QString &), void *data)
{
    d->m_state->callback = lcallback;
    d->m_state->callbackdata = data;
}

void ThemedMenu::setKillable(void)
{
    d->m_state->killable = true;
}

QString ThemedMenu::getSelection(void)
{
    return d->selection;
}

void ThemedMenu::ReloadExitKey(void)
{
    int allowsd = gContext->GetNumSetting("AllowQuitShutdown");
    d->m_state->killable = (allowsd == 4);

    if (allowsd == 1)
        d->exitModifier = Qt::ControlButton;
    else if (allowsd == 2)
        d->exitModifier = Qt::MetaButton;
    else if (allowsd == 3)
        d->exitModifier = Qt::AltButton;
    else
        d->exitModifier = -1;
}

void ThemedMenu::Draw(MythPainter *p, int xoffset, int yoffset, int alphaMod)
{
#if 0
    d->paintLogo(p, alphaMod);
#endif

    int alpha = CalcAlpha(alphaMod);

    d->Draw(p, xoffset, yoffset, alpha);

    MythScreenType::Draw(p, xoffset, yoffset, alphaMod);
}

void ThemedMenu::ReloadTheme(void)
{
    d->ReloadTheme();
}

bool ThemedMenu::keyPressEvent(QKeyEvent *e)
{
    if (d->ignorekeys)
        return false;

    d->ignorekeys = true;

    bool ret = true;
    if (!d->keyPressHandler(e))
        ret = MythScreenType::keyPressEvent(e);

    d->ignorekeys = false;


    if (d->wantpop)
        m_ScreenStack->PopScreen();

    return ret;
}
