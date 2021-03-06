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

#ifndef CCFLASHER_H_
#define CCFLASHER_H_

#include <ccdebuginterfaceusb.h>
#include <string>

#ifdef CC_DEBUGER
	#include <debugserver.h>
#endif //CC_DEBUGER

#define VENDOR_ID  0x03eb
#define  PRODUCT_ID 0x0001

class CCFlasher
{
public:
	CCFlasher();
	virtual ~CCFlasher();
	int run( char *argv[], int argc );
	int parseOptions( char *argv[], int argc );
	bool write( const char *czFilename );
	bool test();
	bool loadFile( const char *czFilename );
	void startDebugServer( CCDebugInterface *dbg );
	void usage();
	CCDebugInterface *m_pDbgInt;
public:
	//options
	std::string m_sFilename;
	bool m_bMassErase;
	enum operation{
		WRITE,
		READ,
		VERIFY,
		ERASE,
		RESET,
		GET_CHIP_ID,
		TEST,
		DEBUG_SERVER
	};
	operation m_iOperation;
	int m_iDebugClockDelay;
	bool m_bResume;
	bool m_bRestoreMac;
	int m_iVerify;
	int m_iServerPort;
	std::string m_sMac;//MAC ADDRESS
public:
	int m_iChipID;
	unsigned char *m_pBuff;
};

#endif /*CCFLASHER_H_*/
