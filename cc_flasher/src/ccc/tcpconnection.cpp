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

#include "tcpconnection.h"
#include "ccmutex.h"

#include <iostream>
#include <string>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <utils.h>

using namespace std;

TCPConnection::TCPConnection(int socket)
{
    m_Socket = socket;
	
	m_sName = "TCPConnection";
}


TCPConnection::~TCPConnection()
{
    if(m_ThreadState == ThreadRunning && m_Socket )
        close(m_Socket);
    m_Socket=0;
}

string TCPConnection::ReadLine(int timeout)
{
    int rectPtr=0,i;
    char recvBuffer[TCPCON_MAX_RECV_BUFF];
    int n;
    int offset;
    string outStr="";
    bool foundLF = false;
    
    fd_set rfds;
    struct timeval tv;
    int retval;

    /* Watch m_Socket to see when it has input. */
    FD_ZERO(&rfds);
    FD_SET(m_Socket, &rfds);
    /* Wait up to five seconds. */
    while(1){
        tv.tv_sec = timeout;
        tv.tv_usec = 0;
        FD_ZERO(&rfds);
        FD_SET(m_Socket, &rfds);
        retval = select(m_Socket + 1, &rfds, NULL, NULL, &tv);
      
        if (retval == -1){
            throw 4;
        }else if (retval > 0){
            n = recv(m_Socket,recvBuffer, TCPCON_MAX_RECV_BUFF-1 , 0); 
            if (n<0) {
                //perror(" cannot receive data ");
                outStr.clear();
                throw 3;
            } else if (n==0) {
                //printf(" connection closed by client\n");
                close(m_Socket);
				m_Socket = -1;
                outStr.clear();
                throw 2;
            }
            recvBuffer[n] = 0;
            for(i = 0 ; i < n ; i++){
                if(recvBuffer[i] == '\n'){
                    foundLF = true;
                    recvBuffer[i] = 0; //terminate it
                    /*if( i >= 1 && recvBuffer[i-1]=='\r')
                        recvBuffer[i-1] = 0;  */  
                }
                
                    
            }
            outStr += recvBuffer;
            if( foundLF ){
				if( outStr.length() > 0 && outStr[ outStr.length()-1 ] == '\r' ){
					outStr.erase( outStr.length()-1, 1 );
				}
                return outStr;
            }
        }else{
           // cout << "No data within five seconds.\n";
        }
    }
    outStr.clear();
    throw 1;
}

int TCPConnection::readByte(int timeout)
{
    int rectPtr=0,i;
    unsigned char recvBuffer[TCPCON_MAX_RECV_BUFF];
    int n;
    int offset;
    
    bool foundLF = false;
    
    fd_set rfds;
    struct timeval tv;
    int retval;

    /* Watch m_Socket to see when it has input. */
    FD_ZERO(&rfds);
    FD_SET(m_Socket, &rfds);
    /* Wait up to five seconds. */
    while(1){
        tv.tv_sec = timeout;
        tv.tv_usec = 0;
        FD_ZERO(&rfds);
        FD_SET(m_Socket, &rfds);
        retval = select(m_Socket + 1, &rfds, NULL, NULL, &tv);
      
        if (retval == -1){
            throw 4;
        }else if (retval > 0){
            n = recv(m_Socket,recvBuffer, 1 , 0); 
            if (n<0) {
                //perror(" cannot receive data ");
                
                throw 3;
            } else if (n==0) {
                //printf(" connection closed by client\n");
                close(m_Socket);
				m_Socket = -1;
                
                throw 2;
            }
            return recvBuffer[0];
 
        }else{
            //cout << "No data within five seconds.\n";
            return -1;//timeout
        }
    }
    throw 1;
}

int TCPConnection::Worker()
{
    string ret;
    try{
        while(! mp_StopRequest ){
            ret = ReadLine();
        }
        return 0;
    }
    catch(int err)
    {
        close(m_Socket);
        m_Socket = -1;
        return 1;
    }
}

bool TCPConnection::WriteData(const unsigned char *data,int len)
{
    if(m_Socket == -1)
        return false;
        
    if( send(m_Socket, data , len , 0) != len) {
        return false;
    }
    return true;   
}

bool TCPConnection::WriteLine(string line)
{
    line+="\n";
    return WriteData((unsigned char*)line.c_str(),line.length());
}
