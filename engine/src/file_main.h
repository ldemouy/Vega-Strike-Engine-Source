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
#ifndef _FILE_MAIN_H_
#define _FILE_MAIN_H_
#include <stdio.h>
#include <string.h>
//#include "gldrv/gfxlib.h"
#include "endianness.h"
#include "vsfilesystem.h"
using VSFileSystem::VSFile;
//using namespace VSFileSystem;
extern VSFile fpread;

/*File utility functions*/
inline void LoadFile(const char *filename)
{
    fpread.OpenReadOnly(filename);
}
inline void CloseFile()
{
    fpread.Close();
}

inline float readf(VSFileSystem::VSFile &f)
{
    union {
        float fval;
        unsigned int ival;
    } t;
    f.Read(&t.fval, sizeof t.fval);
    t.ival = le32_to_cpu(t.ival);
    return t.fval;
}
inline size_t readf(VSFileSystem::VSFile &f, float *b, int32_t n)
{
    size_t rode = f.Read(b, sizeof(*b) * n);
#ifndef NATURAL_ENDIANNESS
    for (size_t i = 0; i < n; i++)
    {
        ((uint32_t *)b)[i] = le32_to_cpu(((uint32_t *)b)[i]);
    }
#endif
    return (rode > 0) ? (rode / sizeof(*b)) : rode;
}
inline int16_t reads(VSFileSystem::VSFile &f)
{
    int16_t temp;
    f.Read(&temp, sizeof(int16_t));
    return le16_to_cpu(temp);
}
inline int32_t readi(VSFileSystem::VSFile &f)
{
    int32_t i;
    f.Read(&i, sizeof(int32_t));
    return le32_to_cpu(i);
}
inline size_t readi(VSFileSystem::VSFile &f, int32_t *b, int32_t n)
{
    size_t rode = f.Read(b, sizeof(*b) * n);
#ifndef NATURAL_ENDIANNESS
    for (size_t i = 0; i < n; i++)
    {
        b[i] = le32_to_cpu(b[i]);
    }
#endif
    return (rode > 0) ? (rode / sizeof(*b)) : rode;
}
inline uint8_t readc(VSFileSystem::VSFile &f)
{
    uint8_t temp;
    f.Read(&temp, sizeof(uint8_t));
    return temp;
}

/*Read simple data*/
inline void ReadInt(int32_t &integer)
{
    fpread.Read(&integer, sizeof(int32_t));
    integer = le32_to_cpu(integer);
}
inline void ReadFloat(float &num)
{
    fpread.Read(&num, sizeof(float));
    *((int32_t *)&num) = le32_to_cpu(*((int32_t *)&num));
}

inline void ReadString(char *string)
{
    int32_t length = strlen(string);

    ReadInt(length);
    fpread.Read(string, length);
    string[length] = '\0';
}

/*Read aggregated data*/
inline void ReadVector(float &x, float &y, float &z)
{
    ReadFloat(x);
    ReadFloat(y);
    ReadFloat(z);
}

inline void ReadVector(Vector &v)
{
    ReadVector(v.i, v.j, v.k);
}

inline void ReadGeneric(char *string, float &x, float &y, float &z)
{
    ReadString(string);
    ReadVector(x, y, z);
}

/*The goods*/
inline void ReadUnit(char *filename, int32_t &type, float &x, float &y, float &z)
{
    ReadGeneric(filename, x, y, z);
}

inline void ReadMesh(char *filename, float &x, float &y, float &z)
{
    ReadGeneric(filename, x, y, z);
}

inline void ReadWeapon(char *filename, float &x, float &y, float &z)
{
    ReadGeneric(filename, x, y, z);
}

inline void ReadRestriction(int32_t &isrestricted, float &start, float &end)
{
    ReadInt(isrestricted);
    ReadFloat(start);
    ReadFloat(end);
}

inline long GetPosition()
{
    return fpread.GetPosition();
}

inline void SetPosition(int64_t position)
{
    fpread.GoTo(position);
}

#endif
