/*
 *  Warning: This is a beerware modification of the BSD licence
 * 
 *	Copyright (c) 2007, Peter Kuhar http://www.pkuhar.com
 *	
 *	All rights reserved.
 *	
 *	Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions are met:
 *	
 *	    * Redistributions of source code must retain the above copyright notice,
 *        this list of conditions and the following disclaimer.
 *	    * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *	    * Neither the name of the Peter Kuhar nor the names of its contributors 
 *        may be used to endorse or promote products derived from this software 
 *        without specific prior written permission.
 *	    * Whoever uses this software is morally obliged to buy the author a beer 
 *        they happen to meet.
 *      * Whoever uses this software for comercial purposes is morally obliged to buy 
 *        the author a 6pack of beer for every 1000€ earned directly(including selling
 *        a hardware that is supported by this software ) from this software.
 *	
 *	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *	A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *	PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *	LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *	NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
 
#include "ccthread.h"
#include <iostream>

using namespace std;

CCThread::CCThread()
{
    Init();
}
CCThread::CCThread(int (*m_ExtFunct)(int param1, void * param2), int param1, void *param2)
{
    Init();
    mp_ExtFunct = m_ExtFunct;
    mp_Param1 = param1;
    mp_Param2 = param2;
}
void CCThread::Init()
{
    mp_ExtFunct = NULL;
    mp_StopRequest = false;
    mp_ReturnValue = -1;
    m_ThreadState = ThreadNull;   
}
CCThread::~CCThread()
{
    Join(true);
}
#ifdef _WIN32
DWORD WINAPI CCThreadThreadHelper(void *arg){
    CCThread *ccThread= (CCThread*)arg;
    ccThread->MainThreadProc();
    return NULL;
};
#else
extern "C" void *CCThreadThreadHelper(void *arg){
    CCThread *ccThread= (CCThread*)arg;
    ccThread->MainThreadProc();
    return NULL;
};
#endif

bool CCThread::Start()
{
    m_ThreadState = ThreadStarting;
#ifdef _WIN32
    mp_Thread = CreateThread( NULL, 0, ::CCThreadThreadHelper, this, 0 ,0);
    int ret= 0;
    if(!mp_Thread)
   	ret=1; 
#else
	int ret = pthread_create( &mp_Thread, NULL,::CCThreadThreadHelper, this);
#endif
    //cout << "thread start " << ret << "\n";
    if(ret)
        return false;
    else
        return true;
}
bool CCThread::Join(bool stop)
{
    if(m_ThreadState != ThreadRunning && m_ThreadState != ThreadTerminating && m_ThreadState != ThreadStarting)
        return false;
    if(stop)
    		Stop();
#ifdef _WIN32
    if( WaitForSingleObject(mp_Thread,10000) == WAIT_TIMEOUT )
        TerminateThread(mp_Thread,0);
    m_ThreadState= ThreadStoped;	
    return true;
#else
    return pthread_join(mp_Thread, NULL) ? false : true; 
#endif
}
bool CCThread::Stop()
{
    m_ThreadState= ThreadTerminating;
    mp_StopRequest= true;
    return true;
}
void CCThread::MainThreadProc()
{
    try{
        //cout << "thread started\n";
        m_ThreadState= ThreadRunning;
        if(mp_ExtFunct){
            mp_ReturnValue= mp_ExtFunct(mp_Param1, mp_Param2);
        }else{
            mp_ReturnValue= Worker();
        }
        m_ThreadState= ThreadStoped;
    }
    catch(...)
    {
        m_ThreadState= ThreadStoped;
    }
    pthread_detach(mp_Thread);
    pthread_exit(NULL);
}
int CCThread::GetThreadResult()
{
    return mp_ReturnValue;
}

int CCThread::Worker(){
    return 0;
}

