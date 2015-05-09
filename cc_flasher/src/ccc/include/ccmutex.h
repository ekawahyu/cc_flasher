/***************************************************************************
 *   Copyright (C) 2005 by Peter Kuhar                                     *
 *   peter.kuhar{guest.arnes.si                                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef CCMUTEX_H
#define CCMUTEX_H

#include <vector>
#include <string>
#include <pthread.h>

using namespace std;

struct SavedMutex{
    string mutexName;
    pthread_mutex_t mutex;
    void *objRef;
};
class CCMutex{
public:
    CCMutex(const char *mutexName, void *forObject);
    CCMutex();
    ~CCMutex();
    bool Lock();
    void UnLock();

public:
    static int mutexCount;
    static vector<SavedMutex> mutexList;
    pthread_mutex_t mutex;
    static pthread_mutex_t listmutex;
};

#endif
