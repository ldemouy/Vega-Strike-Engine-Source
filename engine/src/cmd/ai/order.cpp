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
#include "order.h"
#include "cmd/collection.h"
#include "cmd/unit_generic.h"
#include "communication.h"
#include "config_xml.h"
#include "vegastrike.h"
#include "vs_globals.h"
using std::list;
using std::vector;
//#define ORDERDEBUG  // FIXME ?
void Order::Execute()
{
    static float airesptime = XMLSupport::parse_float(vs_config->getVariable("AI", "CommResponseTime", "3"));
    ProcessCommunicationMessages(airesptime, true);
    int32_t completed = 0;
    for (uint32_t i = 0; i < suborders.size(); i++)
    {
        if ((completed & ((suborders[i])->getType() & (ALLTYPES))) == 0)
        {
            (suborders[i])->Execute();
            completed |= (suborders[i])->getType();
            if ((suborders[i])->Done())
            {
                vector<Order *>::iterator ord = suborders.begin() + i;
                (*ord)->Destroy();
                suborders.erase(ord);
                i--;
            }
        }
    }
    if (suborders.size() == 0)
    {
        done = true;
    }
    else
    {
        done = false;
    }
}

Order *Order::queryType(uint32_t type)
{
    for (uint32_t i = 0; i < suborders.size(); i++)
    {
        if ((suborders[i]->type & type) == type)
        {
            return suborders[i];
        }
    }
    return nullptr;
}
Order *Order::queryAny(uint32_t type)
{
    for (uint32_t i = 0; i < suborders.size(); i++)
    {
        if ((suborders[i]->type & type) != 0)
        {
            return suborders[i];
        }
    }
    return nullptr;
}

void Order::eraseType(uint32_t type)
{
    for (uint32_t i = 0; i < suborders.size(); i++)
    {
        if ((suborders[i]->type & type) == type)
        {
            suborders[i]->Destroy();
            vector<Order *>::iterator j = suborders.begin() + i;
            suborders.erase(j);
            i--;
        }
    }
}

Order *Order::EnqueueOrder(Order *ord)
{
    if (ord == nullptr)
    {
        printf("NOT ENQEUEING nullptr ORDER\n");
        printf("this order: %s\n", getOrderDescription().c_str());
        return nullptr;
    }
    ord->SetParent(parent);
    suborders.push_back(ord);
    return this;
}
Order *Order::EnqueueOrderFirst(Order *ord)
{
    if (ord == nullptr)
    {
        printf("NOT ENQEUEING nullptr ORDER\n");
        printf("this order: %s\n", getOrderDescription().c_str());
        return nullptr;
    }
    ord->SetParent(parent);

    vector<Order *>::iterator first_elem = suborders.begin();
    suborders.insert(first_elem, ord);
    return this;
}
Order *Order::ReplaceOrder(Order *ord)
{
    for (vector<Order *>::iterator ordd = suborders.begin(); ordd != suborders.end();)
    {
        if ((ord->getType() & (*ordd)->getType() & (ALLTYPES)))
        {
            (*ordd)->Destroy();
            ordd = suborders.erase(ordd);
        }
        else
        {
            ordd++;
        }
    }
    suborders.push_back(ord);
    return this;
}

bool Order::AttachOrder(Unit *targets1)
{
    if (!(subtype & STARGET))
    {
        if (subtype & SSELF)
        {
            return AttachSelfOrder(targets1); // can use attach order to do shit
        }

        return false;
    }
    parent->Target(targets1);
    return true;
}

bool Order::AttachSelfOrder(Unit *targets1)
{
    if (!(subtype & SSELF))
    {
        return false;
    }
    group.SetUnit(targets1);
    return true;
}

bool Order::AttachOrder(QVector targetv)
{
    if (!(subtype & SLOCATION))
    {
        return false;
    }
    targetlocation = targetv;
    return true;
}

Order *Order::findOrder(Order *ord)
{
    if (ord == nullptr)
    {
        printf("FINDING EMPTY ORDER\n");
        printf("this order: %s\n", getOrderDescription().c_str());
        return nullptr;
    }
    for (uint32_t i = 0; i < suborders.size(); i++)
    {
        if (suborders[i] == ord)
        {
            return suborders[i];
        }
    }
    return nullptr;
}
Order::~Order()
{
    VSDESTRUCT1
}
void Order::Destructor()
{
    delete this;
}
void Order::Destroy()
{
    for (uint32_t i = 0; i < suborders.size(); i++)
    {
        if (suborders[i] == nullptr)
        {
            printf("ORDER: a null order\n");
            printf("this order: %s\n", getOrderDescription().c_str());
        }
        else
        {
            suborders[i]->Destroy();
        }
    }

    for (auto i = messagequeue.begin(); i != messagequeue.end(); i++)
    {
        delete (*i);
    }

    messagequeue.clear();
    suborders.clear();
    this->Destructor();
}
void Order::ClearMessages()
{

    for (uint32_t i = 0; i < suborders.size(); i++)
    {
        suborders[i]->ClearMessages();
    }

    for (auto i = messagequeue.begin(); i != messagequeue.end(); i++)
    {
        delete (*i);
    }

    messagequeue.clear();
}
void Order::eraseOrder(Order *ord)
{
    bool found = false;
    if (ord == nullptr)
    {
        printf("NOT ERASING A nullptr ORDER\n");
        printf("this order: %s\n", getOrderDescription().c_str());
        return;
    }
    for (uint32_t i = 0; i < suborders.size() && found == false; i++)
    {
        if (suborders[i] == ord)
        {
            suborders[i]->Destroy();
            vector<Order *>::iterator j = suborders.begin() + i;
            suborders.erase(j);
            found = true;
        }
    }
    if (!found)
    {
        printf("TOLD TO ERASE AN ORDER - NOT FOUND\n");
        printf("this order: %s\n", getOrderDescription().c_str());
    }
}

Order *Order::findOrderList()
{
    Order *found_order = nullptr;
    for (uint32_t i = 0; i < suborders.size() && found_order == nullptr; i++)
    {
        found_order = suborders[i]->findOrderList();
    }
    return found_order;
}

string Order::createFullOrderDescription(int level)
{
    string tabs;
    for (int32_t i = 0; i < level; i++)
    {
        tabs = tabs + "   ";
    }
    string desc = tabs + "+" + getOrderDescription() + "\n";
    for (uint32_t j = 0; j < suborders.size(); j++)
    {
        desc = desc + suborders[j]->createFullOrderDescription(level + 1);
    }
    return desc;
}

using std::list;
using std::vector;
void Order::AdjustRelationTo(Unit *un, float factor)
{
    // virtual stub function
}

void Order::Communicate(const CommunicationMessage &c)
{
    int completed = 0;
    unsigned int i = 0;
    CommunicationMessage *newC = new CommunicationMessage(c);
    for (i = 0; i < suborders.size(); i++)
        if ((completed & ((suborders[i])->getType() & (MOVEMENT | FACING | WEAPON))) == 0)
        {
            (suborders[i])->Communicate(*newC);
            completed |= (suborders[i])->getType();
        }
    Unit *un;
    bool already_communicated = false;
    for (list<CommunicationMessage *>::iterator ii = messagequeue.begin(); ii != messagequeue.end(); ii++)
    {
        un = (*ii)->sender.GetUnit();
        bool thisissender = (un == newC->sender.GetUnit());
        if (un == nullptr || thisissender)
        {
            delete (*ii);
            if (thisissender)
                already_communicated = true;
            if ((ii = messagequeue.erase(ii)) == messagequeue.end())
                break;
        }
    }
    if ((un = newC->sender.GetUnit()))
    {
        if (un != parent)
        {
            static bool talk_more_helps =
                XMLSupport::parse_bool(vs_config->getVariable("AI", "talking_faster_helps", "true"));
            static float talk_factor =
                XMLSupport::parse_float(vs_config->getVariable("AI", "talk_relation_factor", ".5"));
            if (talk_more_helps || !already_communicated)
                AdjustRelationTo(un, newC->getDeltaRelation() * talk_factor);
            messagequeue.push_back(newC);
        }
    }
}

void Order::ProcessCommMessage(CommunicationMessage &c)
{
}
void Order::ProcessCommunicationMessages(float AICommresponseTime, bool RemoveMessageProcessed)
{
    float time = AICommresponseTime / SIMULATION_ATOM;
    if (time <= .001)
        time += .001;
    if (!messagequeue.empty())
    {
        bool cleared = false;
        if (messagequeue.back()->curstate == messagequeue.back()->fsm->GetRequestLandNode())
        {
            cleared = true;
            RemoveMessageProcessed = true;
            Unit *un = messagequeue.back()->sender.GetUnit();
            if (un)
            {
                CommunicationMessage c(parent, un, nullptr, 0);
                if (parent->getRelation(un) >= 0 ||
                    (parent->getFlightgroup() && parent->getFlightgroup()->name == "Base"))
                {
                    parent->RequestClearance(un);
                    c.SetCurrentState(c.fsm->GetAbleToDockNode(), nullptr, 0);
                }
                else
                {
                    c.SetCurrentState(c.fsm->GetUnAbleToDockNode(), nullptr, 0);
                }
                Order *o = un->getAIState();
                if (o)
                    o->Communicate(c);
            }
        }
        if (cleared || (((float)rand()) / RAND_MAX) < (1 / time))
        {
            FSM::Node *n;
            if ((n = messagequeue.back()->getCurrentState()))
                ProcessCommMessage(*messagequeue.back());
            if (RemoveMessageProcessed)
            {
                delete messagequeue.back();
                messagequeue.pop_back();
            }
            else
            {
                messagequeue.push_front(messagequeue.back());
                messagequeue.pop_back();
            }
        }
    }
}

namespace Orders
{

void ExecuteFor::Execute()
{
    if (child)
    {
        child->SetParent(parent);
        type = child->getType();
    }
    if (time > maxtime)
    {
        done = true;
        return;
    }
    time += SIMULATION_ATOM;
    if (child)
    {
        child->Execute();
    }
}

Join::Join(Unit *parent, Order *first, Order *second)
    : Order(first->getType() | second->getType(), first->getSubType()), first(first), second(second)
{
    assert((first->getType() & second->getType()) == 0);
    assert(first->getSubType() == second->getSubType());

    SetParent(parent);
    EnqueueOrder(first);
    EnqueueOrder(second);
}

void Join::Execute()
{
    // Execute both sub-orders
    Order::Execute();
    // Wait for both sub-orders to have finished
    if (first->Done() && second->Done())
    {
        done = true;
    }
}

Sequence::Sequence(Unit *parent, Order *order, uint32_t excludeTypes)
    : Order(order->getType() | excludeTypes, order->getSubType()), order(order)
{
    SetParent(parent);
    EnqueueOrder(order);
}

void Sequence::Execute()
{
    Order::Execute();
    if (order->Done())
    {
        done = true;
    }
}

} // namespace Orders
