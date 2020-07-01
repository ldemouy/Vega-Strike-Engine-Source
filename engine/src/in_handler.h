/*
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn & Alan Shieh
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

#ifndef INHANDLER_H
#define INHANDLER_H
#include "in_kb.h"
//#include "cmd_unit.h"
class Unit;

typedef void (*INDISPATCH)(int32_t x, int32_t y);

class InputListener
{
  public:
    int32_t *mousex, *mousey;
    KBSTATE (*keystate)[KEYMAP_SIZE];

    Unit *parent;

    InputListener(Unit *parent)
    {
        this->parent = parent;
        // int a;
        // mousex = mousey = 0;
        // ZeroMemory(keystate, sizeof(keystate));
    }

    virtual void MoveMouse(int32_t x, int32_t y)
    {
        // mousex = x;
        // mousey = y;
    }
    virtual void MoveMouse(int32_t *x, int32_t *y, int32_t num)
    {
    }
    virtual void KeyDown(int32_t key)
    {
        // keystate[key] = 1;
    }
    virtual void KeyDown(int32_t *key, int32_t num)
    {
    }
    virtual void KeyUp(int32_t key)
    {
        // keystate[key] = 0;
    }
    virtual void KeyUp(int32_t *key, int32_t num)
    {
    }

    virtual void Activate()
    {
    }
    virtual void Deactivate()
    {
    }

    // InHandler &Dispatch(); // Drives the input dispatcher, for switching input states
    // InHandler &Revert(); // return to the previous state
};

// const InputListener End;
#endif
