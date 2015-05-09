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

#include "ccmutex.h"
#include <iostream>

using namespace std;

int CCMutex::mutexCount = 0;
vector<SavedMutex> CCMutex::mutexList;
pthread_mutex_t CCMutex::listmutex = PTHREAD_MUTEX_INITIALIZER;

CCMutex::CCMutex(const char *mutexName,void *forObject)
{
    mutexCount ++;//inc mutexCount for every CCMutex object
    try{
        cout << "Ma ne me jebat, od kje";
        bool found=false;
        int idx=0;
        pthread_mutex_lock(&listmutex);
        if(mutexName){
            while(idx < mutexList.size()){
                if( mutexList[idx].mutexName == mutexName && mutexList[idx].objRef == forObject ){
                    found = true;
                    break;
                }
                idx++;
            }
        }
        if(found){
            memcpy(&mutex,&mutexList[idx].mutex,sizeof(pthread_mutex_t));
            //cout << "Mutex " << mutexName << " "<< mutex.__m_lock.__spinlock <<" found\n";
        }else{
            //cout << "Mutex " << mutexName << " "<< mutex.__m_lock.__spinlock<<" CREATED---------\n";
            SavedMutex smutex;
            pthread_mutex_init(&mutex,NULL);
            if(mutexName){
                memcpy(&smutex.mutex,&mutex,sizeof(pthread_mutex_t));
                smutex.mutexName = mutexName;
                smutex.objRef = forObject;
                mutexList.insert(mutexList.end(),smutex);
            }
        }
        pthread_mutex_unlock(&listmutex);
    }
    catch(...){
        pthread_mutex_unlock(&listmutex);  
    }
}
CCMutex::CCMutex()
{
    mutexCount ++;//inc mutexCount for every CCMutex object     
    pthread_mutex_init(&mutex,NULL);      
}

CCMutex::~CCMutex()
{
    mutexCount--;
    UnLock();
}

bool CCMutex::Lock()
{
    int  ret = pthread_mutex_lock(&mutex);
    /*if(ret)
        cout << "Mutex lock failed:"<<ret <<"\n";*/
    return ret == 0;
}

void CCMutex::UnLock()
{
    pthread_mutex_unlock(&mutex);
}
