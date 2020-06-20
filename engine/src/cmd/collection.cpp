#include "collection.h"

#include <list>
#include <vector>
#ifndef LIST_TESTING
#include "unit_util.h"
#include "unit_generic.h"

using std::list;
using std::vector;
//UnitIterator  BEGIN:

UnitCollection::UnitIterator &UnitCollection::UnitIterator::operator=(const UnitCollection::UnitIterator &orig)
{
    if (col != orig.col)
    {
        if (col)
            col->unreg(this);
        col = orig.col;
        if (col)
            col->reg(this);
    }
    it = orig.it;
    return *this;
}

UnitCollection::UnitIterator::UnitIterator(const UnitIterator &orig)
{
    col = orig.col;
    it = orig.it;
    if (col)
        col->reg(this);
}

UnitCollection::UnitIterator::UnitIterator(UnitCollection *orig)
{
    col = orig;
    it = col->units.begin();
    col->reg(this);
    while (it != col->units.end())
    {
        if ((*it) == nullptr)
            ++it;
        else
        {
            if ((*it)->Killed())
                col->erase(it);
            else
                break;
        }
    }
}

UnitCollection::UnitIterator::~UnitIterator()
{
    if (col)
        col->unreg(this);
}

void UnitCollection::UnitIterator::remove()
{
    if (col && it != col->units.end())
        col->erase(it);
}

void UnitCollection::UnitIterator::moveBefore(UnitCollection &otherlist)
{
    if (col && it != col->units.end())
    {
        otherlist.prepend(*it);
        col->erase(it);
    }
}

void UnitCollection::UnitIterator::preinsert(Unit *unit)
{
    if (col && unit)
        col->insert(it, unit);
}

void UnitCollection::UnitIterator::postinsert(Unit *unit)
{
    list<Unit *>::iterator tmp = it;
    if (col && unit && it != col->units.end())
    {
        ++tmp;
        col->insert(tmp, unit);
    }
}

void UnitCollection::UnitIterator::advance()
{
    if (!col || it == col->units.end())
        return;
    ++it;
    while (it != col->units.end())
    {
        if ((*it) == nullptr)
            ++it;
        else
        {
            if ((*it)->Killed())
                col->erase(it);
            else
                break;
        }
    }
}

Unit *UnitCollection::UnitIterator::next()
{
    advance();
    return *it;
}

//UnitIterator END:

//ConstIterator Begin:

UnitCollection::ConstIterator &UnitCollection::ConstIterator::operator=(const UnitCollection::ConstIterator &orig)
{
    col = orig.col;
    it = orig.it;
    return *this;
}

UnitCollection::ConstIterator::ConstIterator(const ConstIterator &orig)
{
    col = orig.col;
    it = orig.it;
}

UnitCollection::ConstIterator::ConstIterator(const UnitCollection *orig)
{
    col = orig;
    for (it = orig->units.begin(); it != col->units.end(); ++it)
        if ((*it) && !(*it)->Killed())
            break;
}

UnitCollection::ConstIterator::~ConstIterator()
{
}

Unit *UnitCollection::ConstIterator::next()
{
    advance();
    if (col && it != col->units.end())
        return *it;
    return nullptr;
}

inline void UnitCollection::ConstIterator::advance()
{
    if (!col || it == col->units.end())
        return;
    ++it;
    while (it != col->units.end())
    {
        if ((*it) == nullptr)
            ++it;
        else
        {
            if ((*it)->Killed())
                ++it;
            else
                break;
        }
    }
}

const UnitCollection::ConstIterator &UnitCollection::ConstIterator::operator++()
{
    advance();
    return *this;
}

const UnitCollection::ConstIterator UnitCollection::ConstIterator::operator++(int)
{
    UnitCollection::ConstIterator tmp(*this);
    advance();
    return tmp;
}

//ConstIterator  END:

//UnitCollection  BEGIN:

UnitCollection::UnitCollection()
{
    activeIters.reserve(20);
}

UnitCollection::UnitCollection(const UnitCollection &unit_collection)
{
    list<Unit *>::const_iterator in = unit_collection.units.begin();
    while (in != unit_collection.units.end())
    {
        append(*in);
        ++in;
    }
}

void UnitCollection::insert_unique(Unit *unit)
{
    if (unit)
    {
        for (list<Unit *>::iterator it = units.begin(); it != units.end(); ++it)
        {
            if (*it == unit)
            {
                return;
            }
        }
        unit->Ref();
        units.push_front(unit);
    }
}

void UnitCollection::prepend(Unit *unit)
{
    if (unit)
    {
        unit->Ref();
        units.push_front(unit);
    }
}

void UnitCollection::prepend(UnitIterator *it)
{
    Unit *tmp = nullptr;
    if (!it)
        return;
    list<Unit *>::iterator tmpI = units.begin();
    while ((tmp = **it))
    {
        tmp->Ref();
        units.insert(tmpI, tmp);
        ++tmpI;
        it->advance();
    }
}

void UnitCollection::append(Unit *un)
{
    if (un)
    {
        un->Ref();
        units.push_back(un);
    }
}

void UnitCollection::append(UnitIterator *it)
{
    if (!it)
        return;
    Unit *tmp = nullptr;
    while ((tmp = **it))
    {
        tmp->Ref();
        units.push_back(tmp);
        it->advance();
    }
}

void UnitCollection::insert(list<Unit *>::iterator &temp, Unit *unit)
{
    if (unit)
    {
        unit->Ref();
        temp = units.insert(temp, unit);
    }
    temp = units.end();
}

void UnitCollection::clear()
{
    if (!activeIters.empty())
    {
        fprintf(stderr, "WARNING! Attempting to clear a collection with active iterators!\n");
        return;
    }

    for (list<Unit *>::iterator it = units.begin(); it != units.end(); ++it)
    {
        (*it)->UnRef();
        (*it) = nullptr;
    }
    units.clear();
}

void UnitCollection::destr()
{
    for (list<Unit *>::iterator it = units.begin(); it != units.end(); ++it)
        if (*it)
        {
            (*it)->UnRef();
            (*it) = nullptr;
        }
    for (auto t = activeIters.begin(); t != activeIters.end(); ++t)
    {
        (*t)->col = nullptr;
    }
}

bool UnitCollection::contains(const Unit *unit) const
{
    if (units.empty() || !unit)
        return false;
    for (list<Unit *>::const_iterator it = units.begin(); it != units.end(); ++it)
        if ((*it) == unit && !(*it)->Killed())
            return true;
    return false;
}

inline void UnitCollection::erase(list<Unit *>::iterator &it2)
{
    if (!(*it2))
    {
        ++it2;
        return;
    }
    //If we have more than 4 iterators, just push node onto vector.
    if (activeIters.size() > 3)
    {
        removedIters.push_back(it2);
        (*it2)->UnRef();
        (*it2) = nullptr;
        ++it2;
        return;
    }
    //If we have between 2 and 4 iterators, see if any are actually
    //on the node we want to remove, if so, just push onto vector.
    //Purpose : This special case is to reduce the size of the list in the
    //situation where removedIters isn't being processed.
    if (activeIters.size() > 1)
        for (vector<UnitCollection::UnitIterator *>::size_type i = 0; i < activeIters.size(); ++i)
            if (activeIters[i]->it == it2)
            {
                removedIters.push_back(it2);
                (*it2)->UnRef();
                (*it2) = nullptr;
                ++it2;
                return;
            }
    //If we have 1 iterator, or none of the iterators are currently on the
    //requested node to be removed, then remove it right away.
    (*it2)->UnRef();
    (*it2) = nullptr;
    it2 = units.erase(it2);
}

bool UnitCollection::remove(const Unit *unit)
{
    if (units.empty() || !unit)
        return false;
    for (list<Unit *>::iterator it = units.begin(); it != units.end(); ++it)
    {
        if ((*it) == unit)
        {
            erase(it);
            return (true);
        }
    }
    return (false);
}

const UnitCollection &UnitCollection::operator=(const UnitCollection &uc)
{
    destr();
    list<Unit *>::const_iterator in = uc.units.begin();
    while (in != uc.units.end())
    {
        append(*in);
        ++in;
    }
    return *this;
}

inline void UnitCollection::reg(UnitCollection::UnitIterator *iter)
{
    activeIters.push_back(iter);
}

inline void UnitCollection::unreg(UnitCollection::UnitIterator *iter)
{
    for (auto t = activeIters.begin(); t != activeIters.end(); ++t)
    {
        if ((*t) == iter)
        {
            activeIters.erase(t);
            break;
        }
    }
    if (activeIters.empty() || (activeIters.size() == 1 && (activeIters[0]->it == units.end() || (*(activeIters[0]->it)))))
    {
        while (!removedIters.empty())
        {
            units.erase(removedIters.back());
            removedIters.pop_back();
        }
    }
}

//UnitCollection END:

#endif //USE_STL_COLLECTION
