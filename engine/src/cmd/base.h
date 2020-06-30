#ifndef __BASE_H__
#define __BASE_H__
#include <vector>
#include <string>
#include "basecomputer.h"
#include "gfx/hud.h"
#include "gfx/sprite.h"
#include <stdio.h>
#include "gui/glut_support.h"

#include "audio/Types.h"
#include "audio/Source.h"
#include <memory>

#define BASE_EXTENSION ".py"

void RunPython(const char *filnam);

class BaseInterface
{
    int32_t curlinkindex;
    int32_t lastmouseindex; //Last link index to be under the mouse
    MousePointerStyle mousePointerStyle;
    bool enabledj;
    bool terminate_scheduled;
    bool midloop;
    TextPlane curtext;

public:
    class Room
    {
    public:
        class Link
        {
        public:
            enum
            {
                ClickEvent = (1 << 0),
                DownEvent = (1 << 1),
                UpEvent = (1 << 2),
                MoveEvent = (1 << 3),
                EnterEvent = (1 << 4),
                LeaveEvent = (1 << 5)
            };

            std::string pythonfile;
            float x, y, wid, hei, alpha;
            std::string text;
            const std::string index;
            uint32_t eventMask;
            int32_t clickbtn;

            virtual void Click(::BaseInterface *base, float x, float y, int32_t button, int32_t state);
            virtual void MouseMove(::BaseInterface *base, float x, float y, int32_t buttonmask);
            virtual void MouseEnter(::BaseInterface *base, float x, float y, int32_t buttonmask);
            virtual void MouseLeave(::BaseInterface *base, float x, float y, int32_t buttonmask);

            void setEventMask(uint32_t mask)
            {
                eventMask = mask;
            }

            explicit Link(const std::string &ind, const std::string &pfile) : pythonfile(pfile), alpha(1.0f), index(ind), eventMask(ClickEvent), clickbtn(-1) {}
            virtual ~Link() {}

            virtual void Relink(const std::string &pfile);
        };
        class Goto : public Link
        {
        public:
            int32_t index;
            virtual void Click(::BaseInterface *base, float x, float y, int32_t button, int32_t state);
            virtual ~Goto() {}
            explicit Goto(const std::string &ind, const std::string &pythonfile) : Link(ind, pythonfile) {}
        };
        class Comp : public Link
        {
        public:
            vector<BaseComputer::DisplayMode> modes;
            virtual void Click(::BaseInterface *base, float x, float y, int32_t button, int32_t state);
            virtual ~Comp() {}
            explicit Comp(const std::string &ind, const std::string &pythonfile) : Link(ind, pythonfile) {}
        };
        class Launch : public Link
        {
        public:
            virtual void Click(::BaseInterface *base, float x, float y, int32_t button, int32_t state);
            virtual ~Launch() {}
            explicit Launch(const std::string &ind, const std::string &pythonfile) : Link(ind, pythonfile) {}
        };
        class Eject : public Link
        {
        public:
            virtual void Click(::BaseInterface *base, float x, float y, int32_t button, int32_t state);
            virtual ~Eject() {}
            explicit Eject(const std::string &ind, const std::string &pythonfile) : Link(ind, pythonfile) {}
        };
        class Talk : public Link
        {
        public:
            //At the moment, the BaseInterface::Room::Talk class is unused... but I may find a use for it later...
            std::vector<std::string> say;
            std::vector<std::string> soundfiles;
            int32_t index;
            int32_t curroom;
            virtual void Click(::BaseInterface *base, float x, float y, int32_t button, int32_t state);
            explicit Talk(const std::string &ind, const std::string &pythonfile);
            virtual ~Talk() {}
        };
        class Python : public Link
        {
        public:
            Python(const std::string &ind, const std::string &pythonfile) : Link(ind, pythonfile) {}
            virtual ~Python() {}
        };
        class BaseObj
        {
        public:
            const std::string index;
            virtual void Draw(::BaseInterface *base);

            virtual ~BaseObj()
            {
            }
            explicit BaseObj(const std::string &ind) : index(ind) {}
        };
        class BasePython : public BaseObj
        {
        public:
            std::string pythonfile;
            float timeleft;
            float maxtime;
            virtual void Draw(::BaseInterface *base);

            virtual ~BasePython()
            {
            }
            BasePython(const std::string &ind, const std::string &python, float time) : BaseObj(ind), pythonfile(python), timeleft(0), maxtime(time) {}
            virtual void Relink(const std::string &python);
        };
        class BaseText : public BaseObj
        {
        public:
            TextPlane text;
            virtual void Draw(::BaseInterface *base);

            virtual ~BaseText()
            {
            }
            BaseText(const std::string &texts,
                     float posx,
                     float posy,
                     float wid,
                     float hei,
                     float charsizemult,
                     GFXColor backcol,
                     GFXColor forecol,
                     const std::string &ind) : BaseObj(ind), text(forecol, backcol)
            {
                text.SetPos(posx, posy);
                text.SetSize(wid, hei);
                float cx = 0, cy = 0;
                text.GetCharSize(cx, cy);
                cx *= charsizemult;
                cy *= charsizemult;
                text.SetCharSize(cx, cy);
                text.SetText(texts);
            }
            void SetText(const std::string &newtext)
            {
                text.SetText(newtext);
            }
            void SetPos(float posx, float posy)
            {
                text.SetPos(posx, posy);
            }
            void SetSize(float wid, float hei)
            {
                text.SetSize(wid, hei);
            }
        };
        class BaseShip : public BaseObj
        {
        public:
            virtual void Draw(::BaseInterface *base);
            Matrix mat;
            virtual ~BaseShip() {}

            explicit BaseShip(const std::string &ind) : BaseObj(ind)
            {
            }
            BaseShip(float r0,
                     float r1,
                     float r2,
                     float r3,
                     float r4,
                     float r5,
                     float r6,
                     float r7,
                     float r8,
                     QVector pos,
                     const std::string &ind) : BaseObj(ind), mat(r0, r1, r2, r3, r4, r5, r6, r7, r8, QVector(pos.i / 2, pos.j / 2, pos.k)) {}
        };
        class BaseVSSprite : public BaseObj
        {
        public:
            virtual void Draw(::BaseInterface *base);
            VSSprite spr;
            std::shared_ptr<Audio::Source> soundsource;
            std::string soundscene;

            virtual ~BaseVSSprite();
            BaseVSSprite(const std::string &spritefile, const std::string &ind);
            void SetSprite(const std::string &spritefile);
            void SetPos(float posx, float posy)
            {
                spr.SetPosition(posx, posy);
            }
            void SetSize(float wid, float hei)
            {
                spr.SetSize(wid, hei);
            }
            void SetTime(float t)
            {
                spr.SetTime(t);
            }
            bool isPlaying() const;

        protected:
            BaseVSSprite(const std::string &ind, const VSSprite &sprite) : BaseObj(ind), spr(sprite) {}
        };

        class BaseVSMovie : public BaseVSSprite
        {
            std::string callback;
            bool playing;
            bool hidePointer;
            double hidePointerTime;

        public:
            virtual ~BaseVSMovie() {}
            BaseVSMovie(const std::string &moviefile, const std::string &ind);
            void SetMovie(const std::string &moviefile);

            float GetTime() const;
            void SetTime(float t);

            bool GetHidePointer() const { return hidePointer; };
            void SetHidePointer(bool hide);

            const std::string &getCallback() const { return callback; }
            void setCallback(const std::string &callback) { this->callback = callback; }

            virtual void Draw(::BaseInterface *base);
        };

        class BaseTalk : public BaseObj
        {
        public:
            static bool hastalked;
            virtual void Draw(::BaseInterface *base);
            //Talk * caller;
            uint32_t curchar;
            float curtime;
            virtual ~BaseTalk() {}
            std::string message;
            BaseTalk(const std::string &msg, const std::string &ind, bool only_one_talk);
        };
        std::string soundfile;
        std::string deftext;
        std::vector<Link *> links;
        std::vector<BaseObj *> objs;

        void Draw(::BaseInterface *base);
        void Click(::BaseInterface *base, float x, float y, int button, int state);
        int32_t MouseOver(::BaseInterface *base, float x, float y);
        Room();
        ~Room();
    };
    friend class Room;
    friend class Room::BaseTalk;
    int32_t curroom;
    std::vector<Room *> rooms;
    TextPlane othtext;
    static BaseInterface *CurrentBase;
    bool CallComp;
    UnitContainer caller;
    UnitContainer baseun;

    std::string python_kbhandler;

    void Terminate();
    void GotoLink(int32_t linknum);
    void InitCallbacks();
    void CallCommonLinks(const std::string &name, const std::string &value);
    void Load(const char *filename, const char *time_of_day, const char *faction);
    static void ClickWin(int32_t x, int32_t y, int32_t button, int32_t state);
    void Click(int32_t x, int32_t y, int32_t button, int32_t state);
    void Key(uint32_t ch, uint32_t mod, bool release, int32_t x, int32_t y);
    static void PassiveMouseOverWin(int32_t x, int32_t y);
    static void ActiveMouseOverWin(int32_t x, int32_t y);
    static void ProcessKeyboardBuffer();
    void MouseOver(int32_t x, int32_t y);
    BaseInterface(const char *basefile, Unit *base, Unit *un);
    ~BaseInterface();
    void Draw();

    void setDJEnabled(bool enabled);
    bool isDJEnabled() const { return enabledj; }
};

#endif
