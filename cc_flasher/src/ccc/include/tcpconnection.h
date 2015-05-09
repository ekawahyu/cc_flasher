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
#ifndef TCP_CLASSESTCPCONNECTION_H
#define TCP_CLASSESTCPCONNECTION_H

#include <string> 
#include "ccthread.h"
#define TCPCON_MAX_RECV_BUFF  2

using namespace std;

class TCPConnection:public CCThread{
public:
    TCPConnection(int Socket);
    ~TCPConnection();
    
    std::string ReadLine(int timeout = 36000000);
    int readByte(int timeout = 36000000);
    int Worker();
    /*virtual */bool WriteData(const unsigned char *data,int len);
    /*virtual */bool WriteLine(string line);
	
	void setName( std::string name ){
		m_sName = name;
	}
	std::string getName(){
		return m_sName;
	} 
protected:
    int m_Socket;
	std::string m_sName;
};


#endif
