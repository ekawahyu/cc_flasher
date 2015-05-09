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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <ccdebuginterface.h>
#include <log.h>
#include <time.h>


#define LOWBYTE(A) ( (A) & 0xff )
#define HIGHBYTE(A) ( ((A)>>8) & 0xff )

class CByteBuffer{
public:
	CByteBuffer( int iMaxSize ){
		iSize = iMaxSize;
		m_pBuff = new unsigned char[iMaxSize];
		iPos = 0;
	}
	void add( unsigned char ch ){
		m_pBuff[iPos++] = ch;
	}
	void add( unsigned char ch1, unsigned char ch2 ){
		m_pBuff[iPos++] = ch1;
		m_pBuff[iPos++] = ch2;
	}
	void add( unsigned char ch1, unsigned char ch2, unsigned char ch3 ){
		m_pBuff[iPos++] = ch1;
		m_pBuff[iPos++] = ch2;
		m_pBuff[iPos++] = ch3;
	}
	unsigned char *m_pBuff;
	int iPos;
	int iSize;
};


struct _INT_STR_PAIR_{
	int iInt;
	const  char *czStr;
};

_INT_STR_PAIR_ g_chipTypes[]={
		{0x01, "CC1110"},
		{0x85, "CC2430"},
		{0x89, "CC2431"},
		{0x81, "CC2510"},
		{0x91, "CC2511"},
		{0,0}
};

_INT_STR_PAIR_ g_statuses[]={
		{ CCDebugInterface::CHIP_ERASE_DONE, "CHIP_ERASE_DONE" },
		{ CCDebugInterface::PCON_IDLE, "PCON_IDLE" },
		{ CCDebugInterface::CPU_HALTED, "CPU_HALTED" },
		{ CCDebugInterface::POWER_MODE_0, "POWER_MODE_0" },
		{ CCDebugInterface::HALT_STATUS, "HALT_STATUS" },
		{ CCDebugInterface::DEBUG_LOCKED, "DEBUG_LOCKED" },
		{ CCDebugInterface::OSCILATOR_STABLE, "OSCILATOR_STABLE" },
		{ CCDebugInterface::STACK_OVERFLOW, "STACK_OVERFLOW" },
		{ 0,0}
};


CCDebugInterface::CCDebugInterface()
{
	
}

void CCDebugInterface::rawCommand( unsigned char cCommand, unsigned char cByte1,  unsigned char cByte2,  unsigned char cByte3, int iInBytes, unsigned char *pcInputBytes )
{
	
}
unsigned char CCDebugInterface::command( unsigned char cCommand, unsigned char cByte1,  unsigned char cByte2,  unsigned char cByte3 )
{
	int iInBytes= cCommand & 0x04 ? 1:0;
	unsigned char temp[10];
	TRACE2("command: %02X - %02X %02X %02X in=%d", cCommand, cByte1, cByte2, cByte3, iInBytes);
	rawCommand( cCommand, cByte1, cByte2, cByte3, iInBytes, temp );
	return temp[0];
}

unsigned short CCDebugInterface::getChipID()
{
	unsigned char buff[100];
	int iRead = 2;
	rawCommand( GET_CHIP_ID,0,0,0,iRead,buff);
	INFO( "Chip id: %02X = %s, version = %02X\n", buff[0], getChipType(buff[0]).c_str(),  buff[1]);
	return buff[0];
}


std::string CCDebugInterface::getChipType( unsigned char cID )
{
	for( int i=0; g_chipTypes[i].iInt; i++){
		if( g_chipTypes[i].iInt == cID )
			return g_chipTypes[i].czStr;
	}
	return "unknown";
}

int CCDebugInterface::getStatus()
{
	int iStatus = command( READ_STATUS );
	TRACE2("Status: %02X\n", iStatus);
	for( int i=0; g_statuses[i].czStr; i++){
		if( iStatus & g_statuses[i].iInt )
			TRACE2("\tStatus: %s\n", g_statuses[i].czStr);
	}
	return iStatus;
}

std::string CCDebugInterface::getStatusDesc()
{
	/*int iStatus = command( READ_STATUS );
	TRACE2("Status: %02X\n", iStatus);
	std::ostringstream out;
	out << std::setw(2) << std::hex <<  iStatus ;
	
	for( int i=0; g_statuses[i].czStr; i++){
		if( iStatus & g_statuses[i].iInt ){
			out << " " << g_statuses[i].czStr; 
		}
	}
	return out.str();*/
	return "";
}

int CCDebugInterface::readXDATACMP( int iAddr, unsigned char *pBuff, unsigned char *pBuffCMP, int iCnt )
{
	TRACE( "readXDATA\n");
	command( DEBUG_INSTR3, 0x90, HIGHBYTE(iAddr), LOWBYTE( iAddr ) );//MOV DPTR, iAddr
	for( int i=0; i < iCnt; i++){
		pBuff[i] = command( DEBUG_INSTR1, 0xE0 );//MOVX A,@DPTR,
		if( pBuff[i] != pBuffCMP[i] )
		{
			LOG_ERROR("readXDATACMP compare LOG_ERROR %04X %02x %02x\n",iAddr, pBuffCMP[i], pBuff[i] )
			iAddr++;
			iAddr--;
		}
		//command( DEBUG_INSTR1, 0xA3 );//INC DPTR
		iAddr++;
		command( DEBUG_INSTR3, 0x90, HIGHBYTE(iAddr), LOWBYTE( iAddr ) );//MOV DPTR, iAddr
		//TRACE( "%02X ",pBuff[i] );
	}
	return iCnt;
	return 0;
}

int CCDebugInterface::readXDATA( int iAddr, unsigned char *pBuff, int iCnt )
{
	TRACE( "readXDATA\n");
	command( DEBUG_INSTR3, 0x90, HIGHBYTE(iAddr), LOWBYTE( iAddr ) );//MOV DPTR, iAddr
	for( int i=0; i < iCnt; i++){
		pBuff[i] = command( DEBUG_INSTR1, 0xE0 );//MOVX A,@DPTR,
		//command( DEBUG_INSTR1, 0xA3 );//INC DPTR
		iAddr++;
		command( DEBUG_INSTR3, 0x90, HIGHBYTE(iAddr), LOWBYTE( iAddr ) );//MOV DPTR, iAddr
		//TRACE( "%02X ",pBuff[i] );
	}
	return iCnt;
	return 0;
}

#ifndef NO_USB_LOGIC
//the logic in firmware way
#define BYTES_PER_WRITE 64
#define BYTES_PER_READ 64
int CCDebugInterface::writeXDATA( int iAddr, unsigned char *pBuff, int iCnt)
{
	int iBlinkCnt = 0;
	bool bToggle = true;
	command( DEBUG_INSTR3, 0x90, HIGHBYTE(iAddr), LOWBYTE( iAddr ) );//MOV DPTR, iAddr
	for( int i=0; i < iCnt; ){
		//writeXDATA32( pBuff+i );	
		int iWriteSize = (i + BYTES_PER_WRITE) < iCnt ? BYTES_PER_WRITE : iCnt - i;  
		writeXDATADirect( pBuff+i, iWriteSize );
		
		iBlinkCnt++;
		if( !(iBlinkCnt % LED_BLINK_RATE) ){
			led( bToggle);
			bToggle = !bToggle;
		}
		i += iWriteSize; 
	}
	return iCnt;
}
#else
//the old way
int CCDebugInterface::writeXDATA( int iAddr, unsigned char *pBuff, int iCnt)
{	
	bool bToggle = true;
	//command( DEBUG_INSTR3, 0x90, HIGHBYTE(iAddr), LOWBYTE( iAddr ) );//MOV DPTR, iAddr
	for( int i=0; i < iCnt; i++){
		command( DEBUG_INSTR3, 0x90, HIGHBYTE(iAddr), LOWBYTE( iAddr ) );//MOV DPTR, iAddr
		iAddr++;
		command( DEBUG_INSTR2, 0x74, pBuff[i] );//MOV A, pBuff[i]
		command( DEBUG_INSTR1, 0xF0 );//MOVX @DPTR,A
		//command( DEBUG_INSTR1, 0xA3 );//INC DPTR
		
		if( !(i % LED_BLINK_RATE) ){
			led( bToggle);
			bToggle = !bToggle;
		}
	}
	led( true );
	return iCnt;
}
#endif

#ifndef NO_USB_LOGIC
int CCDebugInterface::readCodeBank( int iAddr, int iBank, unsigned char *pBuff, int iCnt )
{
	bool bToggle = true;
	int iBlinkCnt = 0;
	command( DEBUG_INSTR3, 0x75, 0xC7, (iBank<<4)|0x01 );//MOV MEMCTR, iBank << 4, select code bank
	command( DEBUG_INSTR3, 0x90, HIBYTE( 0x8000|iAddr), LOWBYTE(0x8000|iAddr));//MOV DPTR, 0x8000|iAddr
	for( int i=0; i < iCnt; ){
		int iReadSize = (i + BYTES_PER_READ) < iCnt ? BYTES_PER_READ : iCnt - i; 
		
		readCodeDirect( iAddr + i, pBuff+i,iReadSize  );
		
		iBlinkCnt++;
		if( !(iBlinkCnt % LED_BLINK_RATE) ){
			led( bToggle);
			bToggle = !bToggle;
		}
		i += iReadSize;
	}
	return iCnt;
}
#else
int CCDebugInterface::readCodeBank( int iAddr, int iBank, unsigned char *pBuff, int iCnt )
{
	bool bToggle = true;
	
	command( DEBUG_INSTR3, 0x75, 0xC7, (iBank<<4)|0x01 );//MOV MEMCTR, iBank << 4, select code bank
	command( DEBUG_INSTR3, 0x90, HIBYTE( 0x8000|iAddr), LOWBYTE(0x8000|iAddr));//MOV DPTR, 0x8000|iAddr
	for( int i=0; i < iCnt; i++){
		command( DEBUG_INSTR1, 0xE4);//CLR A
		pBuff[i] = command( DEBUG_INSTR1, 0x93);//MOVC A, @A+DPTR
		command( DEBUG_INSTR1, 0xA3);//INC DPTR
		
		if( !(i % LED_BLINK_RATE) ){
			led( bToggle);
			bToggle = !bToggle;
		}
	}
	led( true );
	return iCnt;
}
#endif

int CCDebugInterface::readCode( int iAddr, unsigned char *pBuff, int iCnt )
{
	return readCodeBank( iAddr % 0x8000, iAddr / FLASH_BANK_SIZE, pBuff, iCnt );
}

typedef  unsigned char byte_t;

const byte_t internal_crc8_table[256] =
{
	0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65,
	157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220,
	35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98,
	190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255,
	70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7,
	219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154,
	101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36,
	248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,
	140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
	17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
	175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
	50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
	202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139,
	87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22,
	233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168,
	116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53
};

byte_t errorcheck_get_crc8( byte_t *byte, size_t size )
{
	size_t i = 0;
	byte_t crc = 0;

	for ( ; i < size; i++ )
	{
		crc = internal_crc8_table[ crc ^ byte[ i ] ];
	}

	return crc;
}

unsigned char CCDebugInterface::calcChecksum( unsigned char *pBuff, int iCnt )
{
	unsigned char cs = 0x10;
	
	for( int i=0; i < iCnt; i++ ){
		cs ^= pBuff[i];
		cs = (cs<<1)|((cs>>7) & 0x01);
	}
	return cs;
}

/**
 * Tole naj bi poklicalo program v flashu, ki zračuna checksum, nažalost zaradi nezmožnosti codemapinga(program za cs je v ramu)  ne dela 
 **/

unsigned char CCDebugInterface::getPageChecksum( int iPageAddress )
{
	int iAddr = iPageAddress % 0x8000;
	int iBank = iAddr / FLASH_BANK_SIZE;
	
	//command( DEBUG_INSTR3, 0x75, 0xC7, (iBank<<4)|0x01 );//MOV MEMCTR, iBank << 4, select code bank
	//command( DEBUG_INSTR3, 0x90, HIBYTE( 0x8000|iAddr), LOWBYTE(0x8000|iAddr));//MOV DPTR, 0x8000|iAddr
		
	CByteBuffer routine(1000);
	

	routine.add( 0x90, HIBYTE( 0x8000|iAddr), LOWBYTE(0x8000|iAddr) );//MOV DPTR, 0x8000|iAddr
	
	routine.add( 0x79, 0x10 );//                 4 		mov		R1,#0
	                              
	routine.add( 0x7E, 0x00 );//                6 		mov 	R6,#0
	routine.add( 0x7F, 0x08 );//                7 		mov		R7,#8
	  
	routine.add( 0xE4  );//                  9 		clr 	a
	routine.add( 0x93 );//                  10 		MOVC 	A,@A+DPTR
	routine.add( 0xA3 );//	INC DPTR
	routine.add( 0x69  );//                 11 		xrl 	a,R1
	routine.add( 0x23  );//                 12 		rl		A
	routine.add( 0xF9  );//                 13 		mov 	R1,A
 		
	routine.add( 0xDE, 0xF8 );//               15 		DJNZ	R6, loop
	routine.add( 0xDF, 0xF6 );//               16 		DJNZ	R7, loop
		
	routine.add( 0xA5  );//                 18 		nop;  change to 0xA5 for break
/*
	routine.add( 0x74,  0x00 );  //  5 		main: mov 	a,#0
	routine.add( 0xF5,  0xF0 );  //  5 		main: mov 	b,a
	routine.add( 0x7E, 0x00 );  //     7 		loop: mov 	R6,#0
	routine.add( 0x7F, 0x08 );  //     8 		mov		R7,#8
	routine.add( 0xE4 );  //        9 		clr 	a
	routine.add( 0x93 );  //       10 		MOVC A,@A+DPTR
	
	routine.add( 0x75, 0xE0, 0x00 );//                11 		mov acc,#0
	//routine.add( 0xF5, 0xE0 );//                11 		mov acc,a
	//routine.add( 0x00 );
	
	routine.add( 0x12, 0xF0, 0x17 );  // 11 		lcall crc8
		
	routine.add( 0xDE, 0xF6 );  //    13 		DJNZ	R6, 0xloop
	routine.add( 0xDF, 0xF4 );  //    14 		DJNZ	R7, 0xloop
		
	routine.add( 0xAE, 0xF0 );  //    16 		mov	R6,b
	routine.add( 0xA5 );  //       17 		nop;  change to 0xA5 for break


	routine.add( 0xC0, 0xE0 );  //    20 		push	acc			; Save Acc     crc8: 
	routine.add( 0xC0, 0x00 );  //    21 		push	0			; Save R0
	routine.add( 0xC0, 0xE0 );  //    22 		push	acc			; Save Value
	routine.add( 0x78, 0x08 );  //    23 		mov	r0,#8			; Number Bits In Byte

	routine.add( 0x65, 0xF0 );  //    25 lp1:		xrl	a,b			; Calculate CRC
	routine.add( 0x13 );  //       26 		rrc	a			; Move To Carry
	routine.add( 0xE5, 0xF0 );  //    27 		mov	a,b			; Get The Last CRC Value
	routine.add( 0x50, 0x02 );  //    28 		jnc	lp2			; Skip If Data == 0
	routine.add( 0x64, 0x18 );  //    29 		xrl	a,#0x18			; Update The CRC Value

	routine.add( 0x13 );  //       31 lp2:		rrc	a			; Position The New CRC
	routine.add( 0xF5, 0xF0 );  //    32 		mov	b,a			; Store The New CRC
	routine.add( 0xD0, 0xE0 );  //    33 		pop	acc			; Get The Remaining Bits
	routine.add( 0x03 );  //       34 		rr	a			; Position The Next Bit
	routine.add( 0xC0, 0xE0 );  //    35 		push	acc			; Save The Remaining Bits
	routine.add( 0xD8, 0xED );  //    36 		djnz	r0,lp1			; Repeat For 8 Bits

	routine.add( 0xD0, 0xE0 );  //    38 		pop	acc			; Clean Up Stack
	routine.add( 0xD0, 0x00 );  //    39 		pop	0			; Recover R0
	routine.add( 0xD0, 0xE0 );  //    40 		pop	acc			; Recover Acc
	routine.add( 0x22  );   //       41 		ret				; Return To Caller*/
	
	writeXDATA( 0xF000 , routine.m_pBuff, routine.iPos );
	command( DEBUG_INSTR3, 0x75, 0xC7, ((iBank<<4)) | 0x10 | 0x01);//0x40 unified mapping, 0x10 bank1, 0x01 allways 1
	setPC( 0xF000 );
	resume();
	
	int iStatus;
	do{
		iStatus = getStatus();
	}while( !( iStatus & CPU_HALTED ) );
	
	unsigned char cs = command( DEBUG_INSTR1, 0xE9);//MOV A, R6
	return cs;
}

/**
 * check if page is empty( all FF FF FF FF )
 */
bool CCDebugInterface::isPageEmpty( unsigned char *pBuff)
{
	for( int i=0; i < FLASH_PAGE_SIZE; i++ ){
		if( *pBuff++ != 0xFF )
			return false;
	}
	return true;
}
void CCDebugInterface::startTiming()
{
	m_tStart = time(0);
}
int CCDebugInterface::getElapsedSeconds()
{
	return time(0) - m_tStart;
}

/**
 * @param iVerify =0 dont verify =1 verify first page > 1 verify all
 */
int CCDebugInterface::programFlash( unsigned char *pBuf, int iSize, bool bMassErase, int iVerify )
{
	INFO("Writing flash\n");
	led( true ) ;
	int iID = getChipID();
	
	clockInit();
	
	int iUsedPageCnt = 0;
	for( int i=0; i < (FLASH_SIZE/FLASH_PAGE_SIZE); i++){
		if( !isPageEmpty( &pBuf[i*FLASH_PAGE_SIZE] ) ){
			iUsedPageCnt++;
		}
	}
	INFO("Num pages used: %d\n", iUsedPageCnt);
	if( bMassErase )
		massEraseFlash();
	
	
	startTiming();
	
	bool bFailed = false;
	int iErrorCnt = 0;
	int iFirstFailedAddress = -1;
	int iPageAddress = 0;
	int iProgrammedCount = 0;
	for( int iPage = 0; iPage < (FLASH_SIZE/FLASH_PAGE_SIZE); iPage++ ){
		iPageAddress = iPage * FLASH_PAGE_SIZE;
		if( isPageEmpty( &pBuf[iPageAddress] ) ){
			TRACE("Flash page empty %d. Skiping\n", iPage);
			continue;
		}
		//unsigned char chCalcCS = calcChecksum( &pBuf[iPageAddress], FLASH_PAGE_SIZE );
		
		INFO("Writing flash page %d %ds\n", iPage, getElapsedSeconds() );
		writeFlashPage( iPageAddress, &pBuf[iPageAddress] , !bMassErase );
		unsigned char pReadTemp[ FLASH_PAGE_SIZE ];
		
		
		//unsigned char chCSFromChip = getPageChecksum( iPageAddress );
		
		if( iVerify )
		{
			INFO("Verifing flash page %d %ds\n", iPage,  getElapsedSeconds() );
			readFlashPage( iPageAddress, pReadTemp);
			for( int i=0; i < FLASH_PAGE_SIZE; i++ ){
				if( pReadTemp[ i ] != pBuf[ iPageAddress + i ] ){
					TRACE("Flash compare LOG_ERROR at %05X %02X != %02X\n", iPageAddress + i, pReadTemp[ i ] , pBuf[ iPageAddress + i ]  );
					iErrorCnt++;
					if( iFirstFailedAddress < 0 )
						iFirstFailedAddress = iPageAddress+i;
				}
			}
			//only first page?
			if( iVerify == 1)
				iVerify = 0;
		}
		
		if( iErrorCnt ){
			LOG_ERROR("LOG_ERROR writing flash. Verification failed at %05X\n", iFirstFailedAddress);
			return 0;
		}
		iProgrammedCount++;
		printf("[PROGRESS] %3d%% (%d/%d)\n", iProgrammedCount*100/iUsedPageCnt, iProgrammedCount, iUsedPageCnt);
	}
	INFO("Flash writing complete in %ds\n", getElapsedSeconds() );
	printf("[COMPLETE] Flash complete\n");
	
	//dumpMemory( temp2, FLASH_PAGE_SIZE );
	led( false ) ;
	return 1;
}

int CCDebugInterface::readFlash( unsigned char *pBuf, int iSize )
{
	led( true ) ;
	int iID = getChipID();
		
	clockInit();
	unsigned char temp2[FLASH_PAGE_SIZE];
	readFlashPage( 0x0000, temp2 );
	dumpMemory( temp2, FLASH_PAGE_SIZE );
	led( false ) ;
}

bool CCDebugInterface::testChip()
{
	int iID = getChipID();	
	clockInit();
	//massEraseFlash();
	
	startTiming();
	

	
	unsigned char temp[ TEST_SIZE ];
	memset( temp, 0xff, TEST_SIZE);
	for( int i=0; i < TEST_SIZE; i++ ){
		temp[i]  = rand()/(RAND_MAX/255);
	}
	strcpy( (char*)temp, "Pero je car 1234567890abcdefghijklmnoprsABCDEFGHIJH");
	printf("Write ram start %ds", getElapsedSeconds());
	writeXDATA( 0xF000, temp, TEST_SIZE );
	printf("Write ram end  %ds", getElapsedSeconds());
	unsigned char temp2[TEST_SIZE];
	printf("Read ram start %ds", getElapsedSeconds());
	readXDATACMP( 0xF000, temp2,temp, TEST_SIZE);
	printf("Read ram END   %ds", getElapsedSeconds());
	for( int i=0; i < TEST_SIZE; i++){
		if( temp[i] != temp2[i] ){
			LOG_ERROR("LOG_ERROR at %04X %02X != %02X\n", i, temp2[i], temp[i] );
		}
	}
	
	
	/*INFO("READ2\n");
	
	readXDATACMP( 0xF000, temp2,temp, TEST_SIZE);
		
	for( int i=0; i < TEST_SIZE; i++){
		if( temp[i] != temp2[i] ){
			LOG_ERROR("LOG_ERROR at %04X %02X != %02X\n", i, temp2[i], temp[i] );
		}
	}*/
		
	return true;
}


bool CCDebugInterface::writeFlashPage( int iPageAddress, unsigned char *pBuff , bool bErasePage )
{
	int iBank = iPageAddress / FLASH_BANK_SIZE;
	int iPage = iPageAddress / FLASH_PAGE_SIZE;
	
	CByteBuffer routine(1000);
			
	//0x75, 0xAD, ((address >> 8) / FLASH_WORD_SIZE) & 0x7E, // MOV FADDRH, #imm;
	routine.add( 0x75, 0xAD, ((iPageAddress >> 8)/FLASH_WORD_SIZE ) & 0x7E);
	
	//routine.add( 0x75, 0x90, 0x03); //MOV P1,3
	routine.add( 0x75, 0xFE, 0x01); //MOV P1DIR,1 //LED ON
	
	//		0x75, 0xAC, 0x00, // MOV FADDRL, #00;
	routine.add( 0x75, 0xAC, 0x00);
	//*****ERASE BEGIN******
	if( bErasePage ){
		//		0x75, 0xAE, 0x01, // MOV FLC, #01H; // ERASE
		routine.add( 0x75, 0xAE, 0x01 );
				// ; Wait for flash erase to complete
		//		0xE5, 0xAE, // eraseWaitLoop: MOV A, FLC;
		routine.add( 0xE5, 0xAE );
		//		0x20, 0xE7, 0xFB, // JB ACC_BUSY, eraseWaitLoop;
		routine.add( 0x20, 0xE7, 0xFB );
	}
	//*****ERASE END********
	
			// ; Initialize the data pointer
	//		0x90, 0xF0, 0x00, // MOV DPTR, #0F000H;
	routine.add( 0x90, 0xF0, 0x00 );
	// ; Outer loops
	//		0x7F, HIBYTE(WORDS_PER_FLASH_PAGE), // MOV R7, #imm;
	routine.add( 0x7F, HIBYTE( FLASH_PAGE_SIZE / FLASH_WORD_SIZE ) );
	//		0x7E, LOBYTE(WORDS_PER_FLASH_PAGE), // MOV R6, #imm;
	routine.add( 0x7E, LOWBYTE( FLASH_PAGE_SIZE / FLASH_WORD_SIZE ) );
	//		0x75, 0xAE, 0x02, // MOV FLC, #02H; // WRITE
	routine.add( 0x75, 0xAE, 0x02 );
			// ; Inner loops
	//		0x7D, FLASH_WORD_SIZE, // writeLoop: MOV R5, #imm;
	routine.add( 0x7D, FLASH_WORD_SIZE );
	//		0xE0, // writeWordLoop: MOVX A, @DPTR;
	routine.add( 0xE0 );
	//		0xA3, // INC DPTR;
	routine.add( 0xA3 );
	//		0xF5, 0xAF, // MOV FWDATA, A;
	routine.add( 0xF5, 0xAF );
	//		0xDD, 0xFA, // DJNZ R5, writeWordLoop;
	routine.add( 0xDD, 0xFA );
			// ; Wait for completion
	//		0xE5, 0xAE, // writeWaitLoop: MOV A, FLC;
	routine.add( 0xE5, 0xAE );
	//		0x20, 0xE6, 0xFB, // JB ACC_SWBSY, writeWaitLoop;
	routine.add( 0x20, 0xE6, 0xFB );
	//		0xDE, 0xF1, // DJNZ R6, writeLoop;
	routine.add( 0xDE, 0xF1 );
	//		0xDF, 0xEF, // DJNZ R7, writeLoop;
	routine.add( 0xDF, 0xEF );
	
	routine.add( 0x75, 0xFE, 0x00); //MOV P1DIR,1 //LED OFF
	
			// ; Done, fake a breakpoint
	//		0xA5
	routine.add( 0xA5 );
	
	writeXDATA( 0xF000, pBuff, FLASH_PAGE_SIZE );
	writeXDATA( 0xF000 + FLASH_PAGE_SIZE, routine.m_pBuff, routine.iPos );
	command( DEBUG_INSTR3, 0x75, 0xC7, 0x40 | 0x10 | 0x01);//0x40 unified mapping, 0x10 bank1, 0x01 allways 1
	setPC( 0xF000 + FLASH_PAGE_SIZE );
	resume();
	int iStatus;
	do{
		iStatus = getStatus();
	}while( !( iStatus & CPU_HALTED ) );
	
	return 0;
}
bool CCDebugInterface::readFlashPage( int iPageAddress, unsigned char *pBuff )
{
	readCode( iPageAddress, pBuff, FLASH_PAGE_SIZE);
	return true;
}
bool CCDebugInterface::clockInit()
{
	int iCnt = 5;
	command( DEBUG_INSTR3, 0x75, 0xC6, 0x00 );//MOV CLKCON, 0x00
	int sleepReg;
	do{
		sleepReg = command( DEBUG_INSTR2, 0xE5, 0xBE );
	}while( !( sleepReg & 0x40 ) && iCnt-- );
	
	return iCnt > 0 ;
}

bool CCDebugInterface::massEraseFlash()
{
	int iCnt = 100;
	command(DEBUG_INSTR1, 0x00 );//nop
	command( CHIP_ERASE );
	int iStatus;
	do{
		iStatus = getStatus();
	}while( !(iStatus & CHIP_ERASE_DONE) && iCnt--);
	
	return iCnt > 0;
}
bool CCDebugInterface::setPC( int iAddr )
{
	command( DEBUG_INSTR3, 0x02, HIGHBYTE(iAddr), LOWBYTE(iAddr) );//LJMP iAddr
	return true;
}
bool CCDebugInterface::resume()
{
	command( RESUME );
	return true;
}
bool CCDebugInterface::halt()
{
	command( HALT );
	return true;
}
int CCDebugInterface::getPC()
{
	unsigned char temp[2];
	rawCommand( GET_PC, 0, 0, 0, 2, temp );
	
	return temp[1]  + (temp[0]<<8);
}
bool CCDebugInterface::step()
{
	command( STEP_INSTR );
	return true;
}

int CCDebugInterface::getSFR( int iSFR )
{
	return command( DEBUG_INSTR2, 0xE5, iSFR );
}

bool CCDebugInterface::setSFR( int iSFR, int iVal )
{
	command( DEBUG_INSTR3, 0x75, iSFR, iVal );//MOV CLKCON, 0x00
	return true;
}


	
bool CCDebugInterface::setHwBreakpoint( int iNum, int iBank, int iAddr )
{
	int iEn = 1;
	int iFlags = ((iEn & 1)<<2) | ((iNum&0x03)<< 3) | ((iBank) & 0x03);
	unsigned char read[10];
	TRACE( "SET_HW_BRKPNT: %02X %02X %04X\n", iNum, iBank, iAddr)
	command( SET_HW_BRKPNT, iFlags, (iAddr >> 8) & 0xff, iAddr & 0xFF );
	return true;
}

bool CCDebugInterface::rmHwBreakpoint( int iNum )
{
	int iEn = 0;
	int iAddr = 0;
	int iFlags = ((iEn & 1)<<2) | ((iNum&0x03)<< 3) | ((iAddr >> 16) & 0x03);
	command( SET_HW_BRKPNT, iFlags, (iAddr >> 8) & 0xff, iAddr & 0xFF );
	return true;
}

void CCDebugInterface::dumpMemory( unsigned char *pBuff, int iCnt, int iStartAddress  )
{
	for( int i = 0; i < iCnt; i++ ){
		if( (i % 16) == 0 ){
			printf("\n %05X: ", i + iStartAddress );
		}
		if( ( i % 8 ) == 0 ){
			printf(" - ");
		}
		printf("%02X ", pBuff[i] );
	}
	printf("\n");
}

