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
 *        the author a 6pack of beer for every 1000� earned directly(including selling
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

#ifndef DEBUGSERVER_H_
#define DEBUGSERVER_H_
#include <ccdebuginterfaceusb.h>
#include <string>

class NoICEFrame;

#define CODECACHE_SIZE 0x40000

class CDebugServer{
public:
	CDebugServer( CCDebugInterface *dbg, int iPort );
	CCDebugInterface *m_dbg;
	int m_iPort;
	
	void run();
	void runInteractive();
	
	bool noICEHandler( NoICEFrame &inf, NoICEFrame &outf );
	
	
	int getPar( std::string cmd, int iPar );
	
	bool setPC( int iAddr, int iBank = 0 );
	int getPC();
	bool halt();
	bool resume();
	bool step();
	int getRegisters( unsigned char *registers );
	
	std::string setPCHandle( std::string cmd );
	std::string resumeHandle( std::string cmd );
	std::string haltHandle( std::string cmd) ;
	std::string stepHandle( std::string cmd) ;
	std::string getPCHandle( std::string cmd );
	std::string getRegistersHandle( std::string cmd );
	std::string getIDHandle( std::string cmd );
	std::string getSFRHandle( std::string cmd );
	std::string setSFRHandle( std::string cmd );
	std::string flashHandle( std::string cmd );
	
	std::string breakpointHandle( std::string cmd );
	std::string dumpHandle( std::string cmd );
	std::string resetHandle( std::string cmd );
	std::string statusHandle( std::string cmd );
	
	void initCodeCache();
	void getCode( int iAddr, int iCnt, unsigned char *pBuff);
	unsigned char codeCache[CODECACHE_SIZE];
	unsigned char codeCacheValid[CODECACHE_SIZE];//if data in codecache is valid
	
	int usedBP[4];
	FILE *fpLog;
};

#endif /*DEBUGSERVER_H_*/
