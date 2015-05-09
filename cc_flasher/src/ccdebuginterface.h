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

#ifndef CCDEBUFINTERFACE_H_
#define CCDEBUFINTERFACE_H_

#include <time.h>

//NO_USB_LOGIC disables advenced functions is firmware, they are emulated in software
//like writeXDATA32 and readcodedirect
//#define NO_USB_LOGIC 1

#define LOBYTE(w) ((unsigned char)(w))
#define HIBYTE(w) ((unsigned char)(((unsigned short)(w) >> 8) & 0xFF))

#define FLASH_BANK_SIZE 0x8000 //32k
#define FLASH_PAGE_SIZE 0x0800 //2k
#define FLASH_WORD_SIZE 4
#define FLASH_SIZE 		0x20000 //128k

#define TEST_SIZE FLASH_PAGE_SIZE

#define LED_BLINK_RATE 16


#include <string>

class CCDebugInterface{
public:
	CCDebugInterface();
	
	/**
	 * Sends raw command to cc chip
	 */
	virtual void rawCommand( unsigned char cCommand, unsigned char cByte1,  unsigned char cByte2,  unsigned char cByte3, int iInBytes, unsigned char *pcInputBytes );
	unsigned char command( unsigned char cCommand, unsigned char cByte1 = 0,  unsigned char cByte2 = 0,  unsigned char cByte3 = 0 );
	

	virtual void led( bool bOn ){};
	virtual void reset( bool bOn ){};
	virtual void enterDebugMode(){};
	virtual void setDebugClockDelay( unsigned char chDelay ){};
	virtual void writeXDATA32( unsigned char *pBuff ){};
	virtual void writeXDATADirect( unsigned char *pBuff, int iCnt ){};
	virtual void readCodeDirect( int iAddr, unsigned char *pBuff, int iCnt){};
	
	std::string getChipType( unsigned char cType );
	void dumpMemory( unsigned char *pBuff, int iCnt, int iStartAddress = 0 );
	
	int getStatus();
	std::string getStatusDesc();
	unsigned short getChipID();
	int readXDATA( int iAddr, unsigned char *Buff, int iCnt );
	int readXDATACMP( int iAddr, unsigned char *pBuff, unsigned char *pBuffCMP, int iCnt );
	int readCode( int iAddr, unsigned char *Buff, int iCnt );
	int readCodeBank( int iAddr, int iBank, unsigned char *Buff, int iCnt );
	
	int writeXDATA( int iAddr, unsigned char *Buff, int iCnt );
	
	int programFlash( unsigned char *pBuf, int iSize , bool bMassErase, int iVerify = 1 );
	int readFlash( unsigned char *pBuf, int iSize );
	
	bool writeFlashPage( int iPageAddress, unsigned char *pBuff, bool bErasePage );
	bool readFlashPage( int iPageAddress, unsigned char *pBuff );
	unsigned char calcChecksum( unsigned char *pBuff, int iCnt );
	unsigned char getPageChecksum( int iPageAddress );
	bool isPageEmpty( unsigned char *pBuff);
	bool clockInit();
	bool massEraseFlash();
	bool setPC( int iAddr );
	int getPC();
	bool resume();
	bool halt();
	bool step();
	bool testChip();
	int getSFR( int iSFR );
	bool setSFR( int iSFR, int iVal );
	int getIRAM( int iAddr );
	void setIRAM( int iAddr );
	
	bool setHwBreakpoint( int iNum, int iBank, int iAddr );
	bool rmHwBreakpoint( int iNum );
	
	void startTiming();
	int getElapsedSeconds();
	time_t m_tStart;
	
	enum eCommands{
		CHIP_ERASE	= 0x14,
		WR_CONFIG	= 0x1D,
		READ_STATUS	= 0x34,
		GET_CHIP_ID = 0x68,
		HALT		= 0x44,
		RESUME		= 0x4C,
		DEBUG_INSTR1= 0x55,
		DEBUG_INSTR2= 0x56,
		DEBUG_INSTR3= 0x57,
		GET_PC		= 0x28,
		SET_HW_BRKPNT= 0x3F,
		STEP_INSTR	= 0x5C,
		
	};
	
	enum eConfig{
		TIMERS_OFF			= 0x08,
		DMA_PAUSE			= 0x04,
		TIMER_SUSPEND		= 0x02,
		SEL_FLASH_INFO_PAGE	= 0x01
	};
	
	enum eStatus{
		CHIP_ERASE_DONE		= 0x80,
		PCON_IDLE			= 0x40,
		CPU_HALTED			= 0x20,
		POWER_MODE_0		= 0x10,
		HALT_STATUS			= 0x08,
		DEBUG_LOCKED		= 0x04,
		OSCILATOR_STABLE	= 0x02,
		STACK_OVERFLOW		= 0x01
	};
	
	enum eSFR{
			FMAP = 0x9F,
			ACC	 = 0xE0
	};
	
};

#endif /*CCDEBUFINTERFACE_H_*/
