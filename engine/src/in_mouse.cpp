/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 *
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "in_mouse.h"
#include "config_xml.h"
#include "gldrv/winsys.h"
#include "in_handler.h"
#include "in_joystick.h"
#include "options.h"
#include "vegastrike.h"
#include "vs_globals.h"
#include <deque>

using std::deque;
const int32_t NUM_BUTTONS = 15;

/** Gets the button number of the function used to draw the mouse*/
int32_t getMouseDrawFunc()
{
    return NUM_BUTTONS;
}
KBSTATE MouseState[NUM_BUTTONS + 1] = {RELEASE};
static MouseHandler mouseBindings[NUM_BUTTONS + 1];

int32_t mousex = 0;
int32_t mousey = 0;
void GetMouseXY(int32_t &mx, int32_t &my)
{
    mx = mousex;
    my = mousey;
}
int32_t getMouseButtonStatus()
{
    int32_t ret = 0;
    for (int32_t i = 0; i < NUM_BUTTONS; i++)
    {
        ret |= (MouseState[i] == PRESS || MouseState[i] == DOWN) ? (1 << i) : 0;
    }
    return ret;
}

struct MouseEvent
{
    enum EventType
    {
        CLICK,
        DRAG,
        MOTION
    } type;
    int32_t button;
    int32_t state;
    int32_t mod;
    int32_t x;
    int32_t y;
    MouseEvent(EventType type, int32_t button, int32_t state, int32_t mod, int32_t x, int32_t y)
        : type(type), button(button), state(state), mod(mod), x(x), y(y)
    {
    }
};

static deque<MouseEvent> eventQueue;
void mouseClickQueue(int32_t button, int32_t state, int32_t x, int32_t y)
{
    int32_t mod = 0;
    eventQueue.push_back(MouseEvent(MouseEvent::CLICK, button, state, mod, x, y));
}
int32_t delx = 0;
int32_t dely = 0;
void AddDelta(int32_t dx, int32_t dy)
{
    delx += dx;
    dely += dy;
}
int32_t warpallowage = 2;
void DealWithWarp(int32_t x, int32_t y)
{
    if (game_options.warp_mouse)
    {
        if (joystick[MOUSE_JOYSTICK]->player < _Universe->numPlayers())
        {
            if (x < game_options.warp_mouse_zone || y < game_options.warp_mouse_zone ||
                x > g_game.x_resolution - game_options.warp_mouse_zone ||
                y > g_game.y_resolution - game_options.warp_mouse_zone)
            {
                int32_t delx = -x + g_game.x_resolution / 2;
                int32_t dely = -y + g_game.y_resolution / 2;
                mousex += delx;
                mousey += dely;

                for (deque<MouseEvent>::iterator i = eventQueue.begin(); i != eventQueue.end(); i++)
                {
                    i->x += delx;
                    i->y += dely;
                }
                if (warpallowage-- >= 0)
                    winsys_warp_pointer(g_game.x_resolution / 2, g_game.y_resolution / 2);
            }
        }
    }
}

void mouseDragQueue(int32_t x, int32_t y)
{
    eventQueue.push_back(MouseEvent(MouseEvent::DRAG, -1, -1, -1, x, y));
    DealWithWarp(x, y);
}

void mouseMotionQueue(int32_t x, int32_t y)
{
    eventQueue.push_back(MouseEvent(MouseEvent::MOTION, -1, -1, -1, x, y));
    DealWithWarp(x, y);
}

int32_t lookupMouseButton(int32_t b)
{
    static int32_t adj = 0;
    if (b + adj < WS_LEFT_BUTTON)
    {
        adj = WS_LEFT_BUTTON - b;
    }
    b += adj;
    switch (b)
    {
    case WS_LEFT_BUTTON:
        return 0;

    case WS_RIGHT_BUTTON:
        return 2;

    case WS_MIDDLE_BUTTON:
        return 1;

    case WS_WHEEL_UP:
        return 3;

    case WS_WHEEL_DOWN:
        return 4;

    default:
        return ((b - WS_LEFT_BUTTON) >= NUM_BUTTONS) ? NUM_BUTTONS - 1 : b - WS_LEFT_BUTTON;
    }
    return 0;
}
void mouseClick0(int32_t button, int32_t state, int32_t mod, int32_t x, int32_t y)
{
    button = lookupMouseButton(button);
    if (button >= NUM_BUTTONS)
    {
        return;
    }
    AddDelta(x - mousex, y - mousey);
    mousex = x;
    mousey = y;
    mouseBindings[button](state == WS_MOUSE_DOWN ? PRESS : RELEASE, x, y, 0, 0, mod);
    MouseState[button] = (state == WS_MOUSE_DOWN) ? DOWN : UP;
}
void SetDelta(int32_t dx, int32_t dy)
{
    delx = dx;
    dely = dy;
}
void GetMouseDelta(int32_t &dx, int32_t &dy)
{
    dx = delx;
    dy = dely;
    delx = dely = 0;
}

void mouseDrag(int32_t x, int32_t y)
{
    for (int i = 0; i < NUM_BUTTONS + 1; i++)
    {
        mouseBindings[i](MouseState[i], x, y, x - mousex, y - mousey, 0);
    }
    AddDelta(x - mousex, y - mousey);
    mousex = x;
    mousey = y;
}

void mouseMotion(int32_t x, int32_t y)
{
    for (int32_t i = 0; i < NUM_BUTTONS + 1; i++)
    {
        mouseBindings[i](MouseState[i], x, y, x - mousex, y - mousey, 0);
    }
    AddDelta(x - mousex, y - mousey);
    mousex = x;
    mousey = y;
}

static void DefaultMouseHandler(KBSTATE, int32_t x, int32_t y, int32_t delx, int32_t dely, int32_t mod)
{
}

void UnbindMouse(int32_t key)
{
    mouseBindings[key] = DefaultMouseHandler;
}
void BindKey(int32_t key, MouseHandler handler)
{
    mouseBindings[key] = handler;
    handler(RESET, mousex, mousey, 0, 0, 0);
}
void RestoreMouse()
{
    winsys_set_mouse_func(mouseClickQueue);
    winsys_set_motion_func(mouseDragQueue);
    winsys_set_passive_motion_func(mouseMotionQueue);
}

void InitMouse()
{
    for (int32_t a = 0; a < NUM_BUTTONS + 1; a++)
    {
        UnbindMouse(a);
    }
    RestoreMouse();
}

void ProcessMouse()
{
    warpallowage = 2;
    while (eventQueue.size())
    {
        MouseEvent e = eventQueue.front();
        switch (e.type)
        {
        case MouseEvent::CLICK:
            mouseClick0(e.button, e.state, e.mod, e.x, e.y);
            break;
        case MouseEvent::DRAG:
            mouseDrag(e.x, e.y);
            break;
        case MouseEvent::MOTION:
            mouseMotion(e.x, e.y);
            break;
        }
        eventQueue.pop_front();
    }
}
