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
#ifndef TCP_CLASSESTCPLISTENER_H
#define TCP_CLASSESTCPLISTENER_H

//only compile once (becose the code is in this include file)
#pragma once

#include <string>
#include <list>
#include <iostream>

#include "tcpconnection.h"
#include "ccthread.h"
#include "ccmutex.h"
#include "ccconlist.h"

using namespace std;

template<class TCPCONNECTION>
class TCPListener:public CCThread{
public:
    TCPListener(int port);
    ~TCPListener();
    bool IsListening();
    bool Stop();
    int GC();
    int Dispatch(unsigned char *data, int len);
	int DispatchLine(std::string line);
protected:
    int Worker();
private:
    int mv_Port;
    int mv_ConnCount;
    int mv_ServerSocketHandle;
    CCConList connections;
    CCMutex *m_ConListMutex;
};

//*****************************************************************************************************
//implementation
//

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h> /* close */
#include <pthread.h>

using namespace std;


template<class TCPCONNECTION>
TCPListener<TCPCONNECTION>::TCPListener(int port)
{
    mv_Port= port;
    m_ConListMutex= new CCMutex();
}

template<class TCPCONNECTION>
TCPListener<TCPCONNECTION>::~TCPListener()
{
    delete m_ConListMutex;
}
template<class TCPCONNECTION>
int TCPListener<TCPCONNECTION>::Worker()
{
    int newSd;
    int i;
    socklen_t cliLen;
    sockaddr_in cliAddr, servAddr;
   
    /* create socket */
    mv_ServerSocketHandle = socket( AF_INET, SOCK_STREAM, 0 );

    if ( mv_ServerSocketHandle < 0 )
    {
        perror( "cannot open socket " );
        return -1; 
    }
    /* bind server port */
    servAddr.sin_family = AF_INET;                  //it's inet protocol 
    servAddr.sin_addr.s_addr = htonl( INADDR_ANY ); //listen on all network interfaces
    servAddr.sin_port = htons( mv_Port);            //on defined TCP port  
    
    if ( bind( mv_ServerSocketHandle, ( struct sockaddr * ) & servAddr, sizeof( servAddr ) ) < 0 ) //bind to interface/port
    {
        perror( "cannot bind socket " );
        return -1;
    }
    listen( mv_ServerSocketHandle, 5 ); //start listening for connection. Up to 5 clients can be waiting in quoue for accept to accept in
    while ( ! mp_StopRequest )
    {
        cliLen = sizeof( cliAddr );
        newSd = accept( mv_ServerSocketHandle, ( struct sockaddr * ) & cliAddr, &cliLen ); //acept client connection(this is blocking call). the connected client address will be in cliAddr
        if ( newSd == -1 )//something went wrong
        {
            usleep(100000);//we dont want this loop to use all of the CPU
            continue;
        }
        /*printf( "Connection received from %s:TCP%d\n",
                inet_ntoa( cliAddr.sin_addr ),
                ntohs( cliAddr.sin_port ) );*/
        TCPCONNECTION *newcon = new TCPCONNECTION(newSd);
        
        GC();
        
        if(!newcon)
            cout << "WHAT DA FAKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK!\n";
        if(newcon){
            m_ConListMutex->Lock();
            connections.AddItem(newcon);
            m_ConListMutex->UnLock();
            newcon->Start();
        }
    }
    close(mv_ServerSocketHandle);  
    return 0; 
}
template<class TCPCONNECTION>
bool TCPListener<TCPCONNECTION>::Stop()
{
    close(mv_ServerSocketHandle);//close connection
    return CCThread::Stop();
}


template<class TCPCONNECTION>
int TCPListener<TCPCONNECTION>::GC()
{
    int cnt=0;
    m_ConListMutex->Lock();
    TCPCONNECTION  *conn;
    try{
        bool found;
        found = connections.GetFirstItem((void **)&conn);
        while(found){
            if(conn->m_ThreadState == ThreadStoped){
                delete conn;
                connections.DeleteCurrentItem();//after delete pointer moves to the next item
                found = connections.GetCurrentItem((void **)&conn);
            }else{
                found = connections.GetNextItem((void **)&conn);
            }
        }
    }
    catch(...)
    {
    }
    m_ConListMutex->UnLock();
    return cnt;
}
template<class TCPCONNECTION>
int TCPListener<TCPCONNECTION>::Dispatch(unsigned char *data, int len)
{
    int cnt=0;
    TCPCONNECTION  *conn;
    m_ConListMutex->Lock();
    try{
        bool found;
        found = connections.GetFirstItem((void **)&conn);
        while(found){
            conn->WriteData(data,len);
            found = connections.GetNextItem((void **)&conn);
        }
    }
    catch(...)
    {
    }
    m_ConListMutex->UnLock();
    return cnt;
}

template<class TCPCONNECTION>
int TCPListener<TCPCONNECTION>::DispatchLine(std::string line)
{
    return Dispatch((unsigned char*)(line+"\n").c_str(), line.length()+1);
}

#endif
