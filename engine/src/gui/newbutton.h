/*
 * Vega Strike
 * Copyright (C) 2003 Mike Byron
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

#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "control.h"
#include "painttext.h"

//See cpp file for detailed descriptions of classes, functions, etc.

//The NewButton class supports the normal button control.  It can be
//pressed by the mouse, and, by default, send a command out when
//it is pressed.

class NewButton : public Control
{
public:
    //"Look" of the button.  Useful in drawing.
    enum
    {
        NORMAL_STATE = 0,                   //Normal state of a button.
        DOWN_STATE,                         //Pressed down.
        HIGHLIGHT_STATE,                    //Mouse is over button, but not pressed.
        DISABLED_STATE,                     //Pressing the button does nothing.
        FINAL_BUTTON_STATE = DISABLED_STATE //Last value *we've* defined.
    };

    //Set the button state.  If the state changes, it will redraw.
    //The button state is an "int" so that derived classes can add new
    //values.  Use pre-existing enum values if possible.
    virtual int drawingState(void);
    virtual void setDrawingState(int newState);

    //The command ID generated by a button when it is pressed.
    virtual EventCommandId command(void)
    {
        return m_commandId;
    }
    virtual void setCommand(EventCommandId id)
    {
        m_commandId = id;
    }

    //Label that appears on the button.
    virtual std::string label(void)
    {
        return m_label;
    }
    virtual void setLabel(std::string l)
    {
        m_label = l;
    }

    //Background color when mouse is over button.
    virtual GFXColor highlightColor(void)
    {
        return m_highlightColor;
    }
    virtual void setHighlightColor(const GFXColor &c)
    {
        m_highlightColor = c;
    }

    //Text color when mouse is over button.
    virtual GFXColor textHighlightColor(void)
    {
        return m_textHighlightColor;
    }
    virtual void setTextHighlightColor(const GFXColor &c)
    {
        m_textHighlightColor = c;
    }

    //Background color when button is pressed down.
    virtual GFXColor downColor(void)
    {
        return m_downColor;
    }
    virtual void setDownColor(const GFXColor &c)
    {
        m_downColor = c;
    }

    //Text color when button is pressed down.
    virtual GFXColor downTextColor(void)
    {
        return m_downTextColor;
    }
    virtual void setDownTextColor(const GFXColor &c)
    {
        m_downTextColor = c;
    }

    //Width of shadow lines in pixels.
    virtual float shadowWidth(void)
    {
        return m_shadowWidth;
    }
    virtual void setShadowWidth(float width)
    {
        m_shadowWidth = width;
    }

    //Variable-color border cycle time in seconds.  Substitutes for shadows.
    virtual float variableBorderCycleTime(void)
    {
        return m_variableBorderCycleTime;
    }
    virtual void setVariableBorderCycleTime(float cycleTime)
    {
        m_variableBorderCycleTime = cycleTime;
        m_cycleStepCount = (-1);
    }

    //Set the border color of the button.  This overrides the shadow color.
    virtual GFXColor borderColor(void)
    {
        return m_borderColor;
    }
    virtual void setBorderColor(const GFXColor &c)
    {
        m_borderColor = c;
    }

    //Border color at end of cycle.  Only used with variable border.
    virtual GFXColor endBorderColor(void)
    {
        return m_endBorderColor;
    }
    virtual void setEndBorderColor(const GFXColor &c)
    {
        m_endBorderColor = c;
    }

    //Draw the button.
    virtual void draw(void);

    //OVERRIDES
    virtual bool processMouseDown(const InputEvent &event);
    virtual bool processMouseUp(const InputEvent &event);

    //CONSTRUCTION
public:
    NewButton(void);
    virtual ~NewButton(void) {}

protected:
    //INTERNAL IMPLEMENTATION

    //This function is called when the button is pressed.
    //Override to change the behavior.
    virtual void sendButtonCommand(void);

    //Draw the cycled border.  Checks time to change colors, etc.
    virtual void drawCycleBorder(float lineWidth);

    //VARIABLES
protected:
    int m_drawingState;              //How the button looks.
    EventCommandId m_commandId;      //The command to send when pressed.
    std::string m_label;             //The text on this button.
    bool m_leftPressed;              //True = Mouse-down and no mouse-up yet.
    GFXColor m_highlightColor;       //Highlighted button color.
    GFXColor m_textHighlightColor;   //Text color when mouse is highlighted.
    GFXColor m_downColor;            //Background color when button is pressed down.
    GFXColor m_downTextColor;        //Text color when button is pressed down.
    float m_shadowWidth;             //Line width of shadows in pixels.
    float m_variableBorderCycleTime; //Variable border cycle time (in seconds).
    GFXColor m_borderColor;          //Color of border.
    GFXColor m_endBorderColor;       //End color of border if cycling.
    PaintText m_paintText;           //Object that displays label.

    //State for painting a cycling border.
    GFXColor m_currentCycleColor; //The current color of the cycling border.
    int m_currentCycle;           //The "step" in the cycle we are currently painting.
    int m_cycleStepCount;         //Number of steps from one color to the other (1/2 cycle).
    int m_cycleDirection;         //1 or -1 depending on which color we are heading toward.
    GFXColor m_cycleColorDelta;   //Change in each color for one cycle.
    double m_lastStepTime;        //Last time we changed steps.
};

#endif //__BUTTON_H__
