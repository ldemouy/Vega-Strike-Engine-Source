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
#include <list>
#include <lin_time.h>
#include "vegastrike.h"
#include "vs_globals.h"

#include "in_handler.h"
#include "in_joystick.h"
#include "config_xml.h"
#include "in_mouse.h"

#include "options.h"

//Used for storing the max and min values of the tree Joystick Axes - Okona
static int32_t maxx = 1;
static int32_t minx = -1;
static int32_t maxy = 1;
static int32_t miny = -1;
static int32_t maxz = 1;
static int32_t minz = -1;

JoyStick *joystick[MAX_JOYSTICKS]; //until I know where I place it
int32_t num_joysticks = 0;
void modifyDeadZone(JoyStick *j)
{
    for (int a = 0; a < j->nr_of_axes; a++)
    {
        if (fabs(j->joy_axis[a]) <= j->deadzone)
            j->joy_axis[a] = 0.0;
        else if (j->joy_axis[a] > 0)
            j->joy_axis[a] -= j->deadzone;
        else
            j->joy_axis[a] += j->deadzone;
        if (j->deadzone < .999)
            j->joy_axis[a] /= (1 - j->deadzone);
    }
}
void modifyExponent(JoyStick *j)
{
    if ((game_options.joystick_exponent != 1.0) && (game_options.joystick_exponent > 0))
    {
        for (int32_t a = 0; a < j->nr_of_axes; a++)
        {
            j->joy_axis[a] =
                ((j->joy_axis[a] < 0) ? -pow(-j->joy_axis[a], game_options.joystick_exponent) : pow(j->joy_axis[a], game_options.joystick_exponent));
        }
    }
}
static bool JoyStickToggle = true;
void JoyStickToggleDisable()
{
    JoyStickToggle = false;
}
void JoyStickToggleKey(const KBData &key, KBSTATE a)
{
    if (a == PRESS)
    {
        JoyStickToggle = !JoyStickToggle;
    }
}
void myGlutJoystickCallback(uint32_t buttonmask, int32_t x, int32_t y, int32_t z)
{
    for (size_t i = 0; i < MAX_AXES; i++)
    {
        joystick[0]->joy_axis[i] = 0.0;
    }
    joystick[0]->joy_buttons = 0;
    if (JoyStickToggle)
    {
        joystick[0]->joy_buttons = buttonmask;
        if (joystick[0]->nr_of_axes > 0)
        { //Set the max and min of each axis - Okona
            if (x < minx)
            {
                minx = x;
            }
        }
        if (x > maxx)
        {
            maxx = x;
        }
        //Calculate an autocalibrated value based on the max min values - Okona
        joystick[0]->joy_axis[0] = ((float)x - (((float)(maxx + minx)) / 2.0)) / (((float)(maxx - minx)) / 2.0);
        if (joystick[0]->nr_of_axes > 1)
        {
            if (y < miny)
            {
                miny = y;
            }
        }
        if (y > maxy)
        {
            maxy = y;
        }
        joystick[0]->joy_axis[1] = ((float)y - (((float)(maxy + miny)) / 2.0)) / (((float)(maxy - miny)) / 2.0);
        if (joystick[0]->nr_of_axes > 2)
        {
            if (z < minz)
            {
                minz = z;
            }
        }
        if (z > maxz)
        {
            maxz = z;
        }
        joystick[0]->joy_axis[2] = ((float)z - (((float)(maxz + minz)) / 2.0)) / (((float)(maxz - minz)) / 2.0);
        modifyDeadZone(joystick[0]);
        modifyExponent(joystick[0]);
    }
}

JoyStick::JoyStick()
{
    for (size_t j = 0; j < MAX_AXES; ++j)
    {
        axis_axis[j] = -1;
        axis_inverse[j] = false;
        joy_axis[j] = axis_axis[j] = 0;
    }
    joy_buttons = 0;
}
int JoystickPollingRate()
{
    return game_options.polling_rate;
}
void InitJoystick()
{

    for (int i = 0; i < NUMJBUTTONS; i++)
    {
        for (int j = 0; j < MAX_JOYSTICKS; j++)
        {
            UnbindJoyKey(j, i);
        }
    }
    for (int h = 0; h < MAX_HATSWITCHES; h++)
    {
        for (int v = 0; v < MAX_VALUES; v++)
        {
            UnbindHatswitchKey(h, v);
        }
    }
    for (int j = 0; j < MAX_JOYSTICKS; j++)
    {
        for (int h = 0; h < MAX_DIGITAL_HATSWITCHES; h++)
        {
            for (int v = 0; v < MAX_DIGITAL_VALUES; v++)
            {
                UnbindDigitalHatswitchKey(j, h, v);
            }
        }
    }

    num_joysticks = SDL_NumJoysticks();
    printf("%i joysticks were found.\n\n", num_joysticks);
    printf("The names of the joysticks are:\n");

    for (int32_t i = 0; i < MAX_JOYSTICKS; i++)
    {

        if (i < num_joysticks)
        {
            printf("    %s\n", SDL_JoystickName(i));
        }
        joystick[i] = new JoyStick(i); //SDL_Init is done in main.cpp
    }
}

void DeInitJoystick()
{
    for (int i = 0; i < MAX_JOYSTICKS; i++)
    {
        delete joystick[i];
    }
}
JoyStick::JoyStick(int32_t which) : mouse(which == MOUSE_JOYSTICK)
{
    for (int32_t j = 0; j < MAX_AXES; ++j)
    {
        axis_axis[j] = -1;
        axis_inverse[j] = false;
        joy_axis[j] = 0;
    }
    joy_buttons = 0;

    player = which; //by default bind players to whichever joystick it is
    debug_digital_hatswitch = game_options.debug_digital_hatswitch;
    if (which != MOUSE_JOYSTICK)
    {
        deadzone = game_options.deadband;
    }

    else
    {
        deadzone = game_options.mouse_deadband;
    };
    joy_available = 0;
    joy_x = joy_y = joy_z = 0;
    if (which == MOUSE_JOYSTICK)
    {
        InitMouse(which);
    }

    num_joysticks = SDL_NumJoysticks();
    if (which >= num_joysticks)
    {
        if (which != MOUSE_JOYSTICK)
        {
            joy_available = false;
        }
        return;
    }
    joy = SDL_JoystickOpen(which); //joystick nr should be configurable
    if (joy == nullptr)
    {
        printf("warning: no joystick nr %d\n", which);
        joy_available = false;
        return;
    }
    joy_available = true;
    nr_of_axes = SDL_JoystickNumAxes(joy);
    nr_of_buttons = SDL_JoystickNumButtons(joy);
    nr_of_hats = SDL_JoystickNumHats(joy);

    printf("axes: %d buttons: %d hats: %d\n", nr_of_axes, nr_of_buttons, nr_of_hats);
}
void JoyStick::InitMouse(int32_t which)
{
    player = 0; //default to first player
    joy_available = true;
    nr_of_axes = 2; //x and y for mouse
    nr_of_buttons = 15;
    nr_of_hats = 0;
}

bool JoyStick::isAvailable()
{
    return joy_available;
}
struct mouseData
{
    int32_t dx;
    int32_t dy;
    float time;
    mouseData()
    {
        dx = dy = 0;
        time = 0;
    }
    mouseData(int32_t ddx, int32_t ddy, float ttime)
    {
        dx = ddx;
        dy = ddy;
        time = ttime;
    }
};
extern void GetMouseXY(int32_t &mousex, int32_t &mousey);
void JoyStick::GetMouse(float &x, float &y, float &z, int &buttons)
{
    int32_t def_mouse_sens = 1;
    int32_t _dx, _dy;
    float fdx, fdy;
    int32_t _mx, _my;
    GetMouseXY(_mx, _my);
    GetMouseDelta(_dx, _dy);
    if (0 && (_dx || _dy))
    {
        printf("x:%d y:%d\n", _dx, _dy);
    }
    if (!game_options.warp_mouse)
    {
        fdx = (float)(_dx = _mx - g_game.x_resolution / 2);
        def_mouse_sens = 25;
        fdy = (float)(_dy = _my - g_game.y_resolution / 2);
    }
    else
    {
        static std::list<mouseData> md;
        float ttime = getNewTime();
        float lasttime = ttime - game_options.mouse_blur;
        int32_t avg = (_dx || _dy) ? 1 : 0;
        float valx = _dx;
        float valy = _dy;
        for (std::list<mouseData>::iterator i = md.begin(); i != md.end();)
        {
            if ((*i).time >= lasttime)
            {
                bool found = false;
                int32_t ldx = (*i).dx;
                int32_t ldy = (*i).dy;
                if ((ldx >= 0) * _dx * ldx == (_dx >= 0) * _dx * ldx)
                {
                    //make sure same sign or zero
                    valx += (*i).dx;
                    found = true;
                }
                if ((ldy >= 0) * _dy * ldy == (_dy >= 0) * _dy * ldy)
                {
                    //make sure same sign or zero
                    valy += (*i).dy;
                    found = true;
                }
                if (found)
                {
                    avg++;
                }
                ++i;
            }
            else if ((i = md.erase(i)) == md.end())
            {
                break;
            }
        }
        if (_dx || _dy)
        {
            md.push_back(mouseData(_dx, _dy, ttime));
        }
        if (avg)
        {
            _dx = float_to_int(valx / avg);
            _dy = float_to_int(valy / avg);
        }
        fdx = float(valx) / game_options.mouse_blur;
        fdy = float(valy) / game_options.mouse_blur;
    }
    joy_axis[0] = fdx / (g_game.x_resolution * def_mouse_sens / game_options.mouse_sensitivity);
    joy_axis[1] = fdy / (g_game.y_resolution * def_mouse_sens / game_options.mouse_sensitivity);
    if (!game_options.warp_mouse)
    {
        modifyDeadZone(this);
    }
    joy_axis[0] *= game_options.mouse_exponent;
    joy_axis[1] *= game_options.mouse_exponent;
    x = joy_axis[0];
    y = joy_axis[1];

    joy_axis[2] = z = 0;
    buttons = getMouseButtonStatus();
}
void JoyStick::GetJoyStick(float &x, float &y, float &z, int32_t &buttons)
{
    //int status;
    if (joy_available == false)
    {
        for (int32_t a = 0; a < MAX_AXES; a++)
        {
            joy_axis[a] = 0;
        }
        x = y = z = 0;
        joy_buttons = buttons = 0;
        return;
    }
    else if (mouse)
    {
        GetMouse(x, y, z, buttons);
        return;
    }

    int numaxes = SDL_JoystickNumAxes(joy) < MAX_AXES ? SDL_JoystickNumAxes(joy) : MAX_AXES;
    vector<Sint16> axi(numaxes);
    for (int32_t a = 0; a < numaxes; a++)
    {
        axi[a] = SDL_JoystickGetAxis(joy, a);
    }
    joy_buttons = 0;
    nr_of_buttons = SDL_JoystickNumButtons(joy);
    for (int i = 0; i < nr_of_buttons; i++)
    {
        if (SDL_JoystickGetButton(joy, i) == 1)
        {
            joy_buttons |= (1 << i);
        }
    }
    for (int h = 0; h < nr_of_hats; h++)
    {
        digital_hat[h] = SDL_JoystickGetHat(joy, h);
    }
    for (int32_t a = 0; a < numaxes; a++)
    {
        joy_axis[a] = ((float)axi[a] / 32768.0);
    }
    modifyDeadZone(this);
    modifyExponent(this);

    x = joy_axis[0];
    y = joy_axis[1];
    z = joy_axis[2];
    buttons = joy_buttons;
}

int JoyStick::NumButtons()
{
    return nr_of_buttons;
}
