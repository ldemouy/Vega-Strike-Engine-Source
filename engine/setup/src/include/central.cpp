/***************************************************************************
 *                           central.cpp  -  description
 *                           ----------------------------
 *                           begin                : January 18, 2002
 *                           copyright            : (C) 2002 by David Ranger
 *                           email                : sabarok@start.com.au
 **************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   any later version.                                                    *
 *                                                                         *
 **************************************************************************/

#include "central.h"

struct category CATS;
struct group GROUPS;
struct global_settings CONFIG;

static char EMPTY_STR[] = "";

// Primary initialization function. Sets everything up and takes care of the program
void Start(int *argc, char ***argv)
{
    LoadMainConfig();
    InitGraphics(argc, argv);
    LoadConfig();
    ShowMain();
}

void SetGroup(char *group, char *setting)
{
    struct group *CURRENT;
    CURRENT = &GROUPS;
    do
    {
        if (CURRENT->name == nullptr)
        {
            continue;
        }
        if (strcmp(group, CURRENT->name) == 0)
        {
            CURRENT->setting = NewString(setting);
            return;
        }
    } while ((CURRENT = CURRENT->next) != nullptr);
}
void SetInfo(char *category, char *info)
{
    struct category *CURRENT;
    CURRENT = &CATS;
    do
    {
        if (CURRENT->name == nullptr)
        {
            continue;
        }
        if (strcmp(category, CURRENT->name) == 0)
        {
            CURRENT->info = NewString(info);
            return;
        }
    } while ((CURRENT = CURRENT->next) != nullptr);
}

char *GetInfo(char *category)
{
    struct category *CURRENT;
    CURRENT = &CATS;
    do
    {
        if (CURRENT->name == nullptr)
        {
            continue;
        }
        if (strcmp(category, CURRENT->name) == 0)
        {
            if (CURRENT->info)
            {
                return CURRENT->info;
            }
            else
            {
                return category;
            }
        }
    } while ((CURRENT = CURRENT->next) != nullptr);
    return category;
}

char *GetSetting(char *group)
{
    struct group *CUR;
    CUR = &GROUPS;
    do
    {
        if (CUR->name == nullptr)
        {
            continue;
        }
        if (strcmp(CUR->name, group) == 0)
        {
            return CUR->setting;
        }
    } while ((CUR = CUR->next) != nullptr);
    return EMPTY_STR;
}

struct category *GetCatStruct(char *name)
{
    struct category *CUR;
    CUR = &CATS;
    do
    {
        if (CUR->name == nullptr)
        {
            continue;
        }
        if (strcmp(CUR->name, name) == 0)
        {
            return CUR;
        }
    } while ((CUR = CUR->next) != nullptr);
    return 0;
}

struct group *GetGroupStruct(char *name)
{
    struct group *CUR;
    CUR = &GROUPS;
    do
    {
        if (CUR->name == NULL)
        {
            continue;
        }
        if (strcmp(CUR->name, name) == 0)
        {
            return CUR;
        }
    } while ((CUR = CUR->next) != nullptr);
    return 0;
}

struct category *GetNameFromInfo(char *info)
{
    struct category *CUR;
    CUR = &CATS;
    do
    {
        if (CUR->name == NULL)
        {
            continue;
        }
        if (CUR->info != NULL)
        {
            if (strcmp(CUR->info, info) == 0)
            {
                return CUR;
            }
        }
        else
        {
            if (strcmp(CUR->name, info) == 0)
            {
                return CUR;
            }
        }
    } while ((CUR = CUR->next) != nullptr);
    return 0;
}
