/***************************************************************************
 *   Copyright (C) 2005 by Peter Kuhar                                     *
 *   peter.kuhar@guest.arnes.si                                            *
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
 
#ifndef CCTHREAD_H
#define CCTHREAD_H

#ifdef _WIN32
    #include <windows.h>
#else
    #include <pthread.h>
#endif


enum CCThread_State{
    ThreadNull= 0,
    ThreadStarting,
    ThreadRunning,
    ThreadTerminating,
    ThreadStoped
};

class CCThread{
public:
            CCThread(int (*m_ExtFunct)(int param1, void * param2), int param1, void *param2);
            CCThread();
            ~CCThread();
    bool    Start();
    bool    Stop();
    bool    Join(bool stop= false);
    void    MainThreadProc();
    int     GetThreadResult();
    CCThread_State m_ThreadState;
protected:
    bool    mp_StopRequest;
private:
#ifdef _WIN32
    HANDLE  mp_Thread;
#else
    pthread_t mp_Thread;
#endif
    int     (*mp_ExtFunct)(int param1, void* param2); 
    int     mp_ReturnValue;
    int     mp_Param1;
    void    *mp_Param2;
    int     virtual Worker();
protected:
    void    Init();
};

#endif
