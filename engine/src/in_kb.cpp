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

#include "in_kb.h"
#include "gldrv/winsys.h"
#include "in_handler.h"
#include "in_kb_data.h"
#include "vegastrike.h"
#include "vs_globals.h"
#include <list>
#include <queue>

static void DefaultKBHandler(const KBData &, KBSTATE newState) // FIXME ?
{
    // do nothing
}
struct HandlerCall
{
    KBHandler function;
    KBData data;
    HandlerCall()
    {
        function = DefaultKBHandler;
    }
};
static HandlerCall keyBindings[LAST_MODIFIER][WSK_LAST];
static unsigned int playerBindings[LAST_MODIFIER][WSK_LAST];
KBSTATE keyState[LAST_MODIFIER][WSK_LAST];

static void kbGetInput(int32_t key, int32_t modifiers, bool release, int32_t x, int32_t y)
{
    /// FIXME If key is out of array index range, do nothing. This is a quick hack, the underlying cause of invalid
    /// parameters ever being given should probably be fixed instead
    if (key < 0 || key >= WSK_LAST)
    {
        return;
    }
    int i = _Universe->CurrentCockpit();
    _Universe->SetActiveCockpit(playerBindings[modifiers][key]);
    if ((keyState[modifiers][key] == RESET || keyState[modifiers][key] == UP) && !release)
    {
        keyBindings[modifiers][key].function(keyBindings[modifiers][key].data, PRESS);
    }
    if ((keyState[modifiers][key] == DOWN || keyState[modifiers][key] == RESET) && release)
    {
        keyBindings[modifiers][key].function(keyBindings[modifiers][key].data, RELEASE);
    }
    keyState[modifiers][key] = release ? UP : DOWN;
    _Universe->SetActiveCockpit(i);
}

static bool kbHasBinding(int32_t key, int32_t modifiers)
{
    static HandlerCall defaultHandler;
    return keyBindings[modifiers][key].function != defaultHandler.function;
}

static const char _lomap[] = "0123456789-=\';/.,`\\";
static const char _himap[] = ")!@#$%^&*(_+\":?><~|";

int shiftup(int32_t ch)
{
    if (ch == (ch & 0xFF))
    {
        const char *c = strchr(_lomap, ch);
        if (c)
        {
            return _himap[c - _lomap];
        }

        else
        {
            return toupper(ch);
        }
    }
    else
    {
        return ch;
    }
}

int shiftdown(int32_t ch)
{
    if (ch == (ch & 0xFF))
    {
        const char *c = strchr(_himap, ch);
        if (c)
        {
            return _lomap[c - _himap];
        }

        else
        {
            return tolower(ch);
        }
    }
    else
    {
        return ch;
    }
}

static uint32_t _activeModifiers = 0;

void setActiveModifiers(uint32_t mask)
{
    _activeModifiers = mask;
}

void setActiveModifiersSDL(SDLMod mask)
{
    setActiveModifiers(((mask & (KMOD_LSHIFT | KMOD_RSHIFT)) ? KB_MOD_SHIFT : 0) |
                       ((mask & (KMOD_LCTRL | KMOD_RCTRL)) ? KB_MOD_CTRL : 0) |
                       ((mask & (KMOD_LALT | KMOD_RALT)) ? KB_MOD_ALT : 0));
}

uint32_t getActiveModifiers()
{
    return _activeModifiers;
}
uint32_t pullActiveModifiers()
{

    setActiveModifiersSDL(SDL_GetModState());
    return getActiveModifiers();
}
uint32_t getModifier(const char *mod_name)
{
    if (mod_name[0] == '\0')
    {
        return 0;
    }
    uint32_t rv = 0;
    if (strstr(mod_name, "shift") || strstr(mod_name, "uppercase") || strstr(mod_name, "caps"))
    {
        rv |= KB_MOD_SHIFT;
    }
    if (strstr(mod_name, "ctrl") || strstr(mod_name, "cntrl") || strstr(mod_name, "control"))
    {
        rv |= KB_MOD_CTRL;
    }
    if (strstr(mod_name, "alt") || strstr(mod_name, "alternate"))
    {
        rv |= KB_MOD_ALT;
    }
    return rv;
}
int32_t getModifier(bool alton, bool cntrlon, bool shifton)
{
    return cntrlon ? KB_MOD_CTRL : (alton ? KB_MOD_ALT : (shifton ? KB_MOD_SHIFT : 0));
}
void glut_keyboard_cb(uint32_t ch, uint32_t mod, bool release, int32_t x, int32_t y)
{
    bool shifton = false;
    int32_t alton = false;
    int32_t ctrlon = false;

    uint32_t modmask = KB_MOD_MASK;
    if ((WSK_MOD_LSHIFT == (mod & WSK_MOD_LSHIFT)) || (WSK_MOD_RSHIFT == (mod & WSK_MOD_RSHIFT)))
    {
        // This is ugly, but we have to support legacy config files...
        //...maybe add config option to disable this soooo ugly thing...
        if (!kbHasBinding(ch, KB_MOD_SHIFT))
        {
            ch = shiftup(ch);
            modmask &= ~KB_MOD_SHIFT;
        }
        shifton = true;
    }
    if ((WSK_MOD_LALT == (mod & WSK_MOD_LALT)) || (WSK_MOD_RALT == (mod & WSK_MOD_RALT)))
    {
        alton = true;
    }
    if ((WSK_MOD_LCTRL == (mod & WSK_MOD_LCTRL)) || (WSK_MOD_RCTRL == (mod & WSK_MOD_RCTRL)))
    {
        ctrlon = true;
    }
    // Polling state
    setActiveModifiers((shifton ? KB_MOD_SHIFT : 0) | (alton ? KB_MOD_ALT : 0) | (ctrlon ? KB_MOD_CTRL : 0));

    int32_t curmod = getModifier(alton, ctrlon, shifton) & modmask;
    kbGetInput(ch, curmod, release, x, y);
    if (release)
    {
        for (int i = 0; i < LAST_MODIFIER; ++i)
        {
            if (i != curmod)
            {
                if (keyState[i][shiftdown(ch)] == DOWN)
                {
                    kbGetInput(shiftdown(ch), i, release, x, y);
                }
                if (keyState[i][shiftup(ch)] == DOWN)
                {
                    kbGetInput(shiftup(ch), i, release, x, y);
                }
            }
            else
            {
                if (shifton)
                {
                    if (((uint32_t)shiftdown(ch)) != ch && keyState[i][shiftdown(ch)] == DOWN)
                    {
                        kbGetInput(shiftdown(ch), i, release, x, y);
                    }
                }
                else if (((uint32_t)shiftup(ch)) != ch && keyState[i][shiftup(ch)] == DOWN)
                {
                    kbGetInput(shiftup(ch), i, release, x, y);
                }
            }
        }
    }
}

void RestoreKB()
{
    for (int32_t i = 0; i < LAST_MODIFIER; ++i)
        for (int32_t a = 0; a < KEYMAP_SIZE; a++)
            if (keyState[i][a] == DOWN)
            {
                keyBindings[i][a].function(keyBindings[i][a].data, RELEASE);
                keyState[i][a] = UP;
            }
    winsys_set_keyboard_func(glut_keyboard_cb);
}

void InitKB()
{
    for (int32_t i = 0; i < LAST_MODIFIER; ++i)
        for (int32_t a = 0; a < KEYMAP_SIZE; a++)
        {
            keyState[i][a] = UP;
            UnbindKey(a, i);
        }
    RestoreKB();
}

void ProcessKB(uint32_t player)
{
    for (int32_t mod = 0; mod < LAST_MODIFIER; mod++)
    {
        for (int32_t a = 0; a < KEYMAP_SIZE; a++)
        {
            if (playerBindings[mod][a] == player)
            {
                keyBindings[mod][a].function(keyBindings[mod][a].data, keyState[mod][a]);
            }
        }
    }
}

void BindKey(int32_t key, uint32_t mod, uint32_t player, KBHandler handler, const KBData &data)
{
    keyBindings[mod][key].function = handler;
    keyBindings[mod][key].data = data;
    playerBindings[mod][key] = player;
    handler(std::string(), RESET); // key is not used in handler
}

void UnbindKey(int32_t key, uint32_t mod)
{
    keyBindings[mod][key] = HandlerCall();
}
