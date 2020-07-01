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

/*
 *  Joystick support written by Alexander Rawass <alexannika@users.sourceforge.net>
 */
#include "in_kb_data.h"
#ifndef _JOYSTICK_H_
#define _JOYSTICK_H_

#include <SDL/SDL.h>

#include "in_kb.h"
#include "vegastrike.h"

class JoyStick;

extern void ProcessJoystick(int32_t whichjoystick);
extern void InitJoystick();
extern void DeInitJoystick();

const int32_t MAX_JOYSTICKS = 16;
const int32_t MOUSE_JOYSTICK = MAX_JOYSTICKS - 1;
const int32_t MAX_BUTTONS = 48;
const int32_t MAX_DIGITAL_HATSWITCHES = 4;
const int32_t MAX_DIGITAL_VALUES = 9;
const int32_t MAX_AXES = 32;
const int32_t NUMJBUTTONS = 32;

enum
{
    VS_HAT_CENTERED = 0,
    VS_HAT_LEFT,
    VS_HAT_RIGHT,
    VS_HAT_DOWN,
    VS_HAT_UP,
    VS_HAT_RIGHTUP,
    VS_HAT_RIGHTDOWN,
    VS_HAT_LEFTUP,
    VS_HAT_LEFTDOWN
};

extern JoyStick *joystick[MAX_JOYSTICKS];

class JoyStick
{
    bool mouse;
    void InitMouse(int32_t i);
    void GetMouse(float &x, float &y, float &z, int32_t &buttons);

  public:
    // initializes the joystick
    JoyStick(int);
    // engine calls GetJoyStick to get coordinates and buttons
    void GetJoyStick(float &x, float &y, float &z, int32_t &buttons);
    bool isAvailable(void);
    bool is_around(float axe, float hswitch);
    int32_t NumButtons();

    SDL_Joystick *joy;

    int32_t nr_of_axes, nr_of_buttons, nr_of_hats;
    int32_t hat_margin;
    size_t player;

    bool axis_inverse[MAX_AXES];
    int32_t axis_axis[MAX_AXES];
    float joy_axis[MAX_AXES];
    JoyStick();

    uint8_t digital_hat[MAX_DIGITAL_HATSWITCHES];

    bool debug_digital_hatswitch;

    int32_t joy_buttons;
    bool joy_available;
    float joy_xmin, joy_xmax, joy_ymin, joy_ymax, joy_zmin, joy_zmax;
    float joy_x, joy_y, joy_z;
    float deadzone;
};

extern JoyStick *joystick[MAX_JOYSTICKS];
typedef void (*JoyHandler)(KBSTATE, float x, float y, int32_t mod);
void BindJoyKey(int32_t key, int32_t joystick, KBHandler handler, const KBData &data);
void UnbindJoyKey(int32_t joystick, int32_t key);

void UnbindHatswitchKey(int32_t hatswitch, int32_t val_index);
void BindHatswitchKey(int32_t hatswitch, int32_t val_index, KBHandler handler, const KBData &data);

void BindDigitalHatswitchKey(int32_t joystick, int32_t hatswitch, int32_t dir_index, KBHandler handler,
                             const KBData &data);
void UnbindDigitalHatswitchKey(int32_t joystick, int32_t hatswitch, int32_t dir_index);

#endif //_JOYSTICK_H_
