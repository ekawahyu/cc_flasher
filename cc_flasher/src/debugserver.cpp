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

#include <debugserver.h>
#include <tcpconnection.h>
#include <tcplistener.h>
#include <log.h>
#include <CCFlasher.h>

extern CCFlasher *g_ccflasher;

CDebugServer *g_ds;

class MissingParameter
{
public:
	int m_iPar;
	MissingParameter(int iPar){
		m_iPar = iPar;
	}
	int getPar(){
		return m_iPar;
	}
};

#define NOICE_BUFFER_SIZE 10000

class NoICEFrame
{
public:
	NoICEFrame()
	{
		cnt = 0;
		memset( buffer, 0, NOICE_BUFFER_SIZE );
	};
	unsigned char buffer[NOICE_BUFFER_SIZE];
	unsigned int cnt;
	
	bool receiving(){
		return cnt > 0;
	}
	
	unsigned char *data(){
		return &buffer[2];
	}
	unsigned char len(){
		return buffer[1];
	}
	unsigned char function(){
		return buffer[0];
	}
	void setFunction( unsigned char func ){
		buffer[0] = func;
	}
	void addCS()
	{
		unsigned char sum = 0;
		cnt = buffer[1] + 2;
		for( int i=0; i < cnt;i++ ){
			sum += buffer[i];
		}
		sum = (~sum) + 1;
		buffer[cnt++] = sum;
	}
	
	void addByte( unsigned char byte ){
		buffer[ buffer[1] + 2 ] = byte;
		buffer[1]++;
		//printf("Add byte %d %02X\n", buffer[1]-1, byte);
	}
	
	bool checkCS()
	{
		unsigned char cs = 0;
		for( int i=0; i < cnt; i++ ){
			cs += buffer[i];
		}
		printf("checkCS. cs=%02X cnt=%d\n", cs, cnt);
		return cs == 0;
	}
	
	/**
	 * @return true if needs more bytes
	 */
	bool recvByte( int byt ){
		//printf("Add byte %d %02X\n", cnt, byt );
		if( byt < 0 ){
			cnt = 0;
			memset( buffer, 0, NOICE_BUFFER_SIZE );
			return true;//not a good one, need more
		}
		buffer[ cnt ] = byt;
		if( cnt == 0 && byt < 0x7F)
			return true;//need more
		cnt++;
		if( cnt > 2 ){
			if( cnt > (buffer[1]+2) ){
				if( checkCS() ){
					return false;//have enought
				}else{
					cnt = 0;
					memset( buffer, 0, NOICE_BUFFER_SIZE );
					return true;//get more
				}
			}
		}
		return true;
	}
	void dump(){
		std::cout << "NoICE Frame: ";
		for( int i=0; i < cnt; i++ ){
			printf("%02X ", buffer[i]);
		}
		std::cout << endl;
	}
	enum eFunctions{
		FN_GET_STATUS 	= 0xFF,
		FN_READ_MEM 	= 0xFE,
		FN_WRITE_MEM	= 0xFD,
		FN_READ_REGS	= 0xFC,
		FN_WRITE_REGS	= 0xFB,
		FN_RUN_TARGET	= 0xFA,
		FN_SET_BYTES	= 0xF9, 
		FN_IN			= 0xF8,
		FN_OUT 			= 0xF7,
		FN_RESET_TARGET = 0xF6,
		FN_STEP			= 0xF5,
		FN_STOP_TARGET	= 0xF4,
		FN_ERROR		= 0xF0
	};
};

class NoICETCPConnection: public TCPConnection
{
public:
	NoICETCPConnection( int socket );
	int Worker();
} ;



NoICETCPConnection::NoICETCPConnection( int socket ) : TCPConnection( socket )
{}
int NoICETCPConnection::Worker()
{
    try
    {
    	int iLastFunction = 0;
    	
        while ( 1 )
        {
        	NoICEFrame frame;
        	
        	int ch = 0;
        	do{
        		//block 0s if in run state, otherwise 60s
        		ch = readByte( iLastFunction == NoICEFrame::FN_RUN_TARGET ? 0: 60 );
        		
        		//if we are not just receiving a frame from NOICE, we can check for status
        		if( ch == -1 && frame.receiving() == false ){
        			if( iLastFunction == NoICEFrame::FN_RUN_TARGET ){//only if previous command was target running	
	        			int iStatus = g_ds->m_dbg->getStatus();
	        			//return registers if cpu is halted
	        			if( iStatus & CCDebugInterface::CPU_HALTED ){
	        				NoICEFrame cmd;
	        				NoICEFrame tmpOut;
	        				cmd.setFunction(NoICEFrame::FN_READ_REGS );
	        				cmd.addCS();
	        				tmpOut.setFunction(NoICEFrame::FN_RUN_TARGET );
	        				if(  iStatus & CCDebugInterface::CPU_HALTED  && 
	        					iStatus & CCDebugInterface::HALT_STATUS ){
	        					
	        					//after the hw breakpoint, the pc will be one byte over the breakpoint
	        					//so -- it here. I hope this hack does not come bank to hunt me
	        					int iPC = g_ds->m_dbg->getPC();
	        					g_ds->m_dbg->setPC( iPC - 1 );
	        				}
	        				
	        				
	        				if( g_ds->noICEHandler( cmd, tmpOut ) ){
	        					tmpOut.addCS();
	        					//tmpOut.dump();
					        	WriteData( tmpOut.buffer, tmpOut.cnt );
				        	}	
	        				iLastFunction = 0;
        				}
        			}
        		}	
        	}while( frame.recvByte( ch ) );
        	
        	iLastFunction = frame.function();
        	//frame.dump();
        	NoICEFrame outframe;
        	outframe.setFunction( frame.function() );
        	if( g_ds->noICEHandler( frame, outframe ) ){
        		outframe.addCS();
	        	//outframe.dump();
	        	WriteData( outframe.buffer, outframe.cnt );
        	}	   
        }
    }
    catch ( ... )
    {
        close( m_Socket );
        //cout <<"Connection closed\n";
    }
    return 0;
}


class InteractiveTCPConnection: public TCPConnection
{
public:
	InteractiveTCPConnection( int socket );
	int Worker();
} ;



InteractiveTCPConnection::InteractiveTCPConnection( int socket ) : TCPConnection( socket )
{}
int InteractiveTCPConnection::Worker()
{
    string str;
    try
    {
        while ( 1 )
        {
        	try{
	            str = ReadLine();
	            int p1 = 0, p2 = 0;
	            //condense whitespace
	            while( p2 < str.length() ){
	            	if( str[p2] == ' ' && str[p2+1] == ' ' ){
	            		p2++;
	            		str[p1] = str[p2];
	            	}else{
	            		p1++;
	            		p2++;
	            		str[p1] = str[p2];
	            	}
	            }
	            //trim
	            while( str[ str.length() -1 ] == ' ' ){
	            	str = str.substr( 0,  str.length() -1 );
	            }
	            
	            if ( str == "exit" || str == "q" || str == "quit" )
	            {
	                WriteLine( "bye\n" );
	                close( m_Socket );
	                return 0;
	            }
				else if ( !str.find("halt") || str == "h" )
	            {
	                WriteLine( g_ds->haltHandle( str ) );
	            }
				else if ( !str.find("resume") || str == "c" )
	            {
	                WriteLine( g_ds->resumeHandle( str ) );
	            }
				else if ( !str.find("setpc") )
	            {
	                WriteLine( g_ds->setPCHandle( str ) );
	            }
				else if ( !str.find("getpc") )
	            {
	                WriteLine( g_ds->getPCHandle( str ) );
	            }
				else if ( !str.find("regs") )
	            {
	                WriteLine( g_ds->getRegistersHandle( str ) );
	            }
				else if ( !str.find("id") )
				{
				    WriteLine( g_ds->getIDHandle( str ) );
				}
				else if ( !str.find("setsfr") )
				{
				    WriteLine( g_ds->setSFRHandle( str ) );
				}
				else if ( !str.find("sfr") )
				{
				    WriteLine( g_ds->getSFRHandle( str ) );
				}
				else if ( !str.find("step") || str == "s" )
				{
				    WriteLine( g_ds->stepHandle( str ) );
				}
				else if ( !str.find("flash") )
				{
				    WriteLine( g_ds->flashHandle( str ) );
				}
				else if ( !str.find("bp") )
				{
				    WriteLine( g_ds->breakpointHandle( str ) );
				}
				else if ( !str.find("dumpx") )
				{
				    WriteLine( g_ds->dumpHandle( str ) );
				}
				else if ( !str.find("reset") )
				{
				    WriteLine( g_ds->resetHandle( str ) );
				}
				else if ( !str.find("status") )
				{
				    WriteLine( g_ds->statusHandle( str ) );
				}
				else if ( !str.find("clearcodecache") )
				{
					g_ds->initCodeCache();
				    WriteLine( "OK" );
				}
				else{
	            	WriteLine( "ERR what did you mean bjatch");
	            }
        	}
        	catch( MissingParameter mp ){
        		char temp[100];
        		sprintf(temp, "ERR Missing parameter %d ", mp.getPar());
        		WriteLine( temp );
        	}
        }
    }
    catch ( ... )
    {
        close( m_Socket );
        //cout <<"Connection closed\n";
    }
    return 0;
}

CDebugServer::CDebugServer( CCDebugInterface *dbg, int iPort )
{
	m_dbg = dbg;
	m_iPort = iPort;
	g_ds = this;
	initCodeCache();
	fpLog = 0;
	
	if( getenv("LOGFILE") ){
		fpLog = fopen( getenv("LOGFILE"),"w");
	}
	m_dbg->command( CCDebugInterface::WR_CONFIG , CCDebugInterface::TIMER_SUSPEND );
		
};
void CDebugServer::run()
{
	INFO("Running server\n")
	TCPListener<InteractiveTCPConnection> conn( m_iPort );
	conn.Start();
	TCPListener<NoICETCPConnection> noice( m_iPort +1 );
	noice.Start();
	while(1){
		sleep(1000);
	}
}

void CDebugServer::runInteractive()
{
	
}



bool CDebugServer::setPC( int iAddr, int iBank  )
{
	return true;
}

int CDebugServer::getPC()
{
	return m_dbg->getPC();
}

bool CDebugServer::halt()
{
	return true;
}

bool CDebugServer::resume()
{
	return true;
}

bool CDebugServer::step()
{
	return true;
}

int CDebugServer::getRegisters( unsigned char *registers )
{
	return 0;
}


std::string CDebugServer::setPCHandle( std::string cmd )
{
	int iAddr = getPar( cmd, 0 );
	
	int iStatus = m_dbg->getStatus();
	if( !( iStatus & CCDebugInterface::CPU_HALTED ) ){
		m_dbg->halt();
	}
	
	if( iAddr >= 0x8000 )	
		m_dbg->setSFR( CCDebugInterface::FMAP, ( iAddr >> 16 ) & 0x03);
	m_dbg->setPC( iAddr && 0xFFFF );
	
	if( !( iStatus & CCDebugInterface::CPU_HALTED ) ){
		m_dbg->resume();
	}
	
	return "OK";
}

std::string CDebugServer::resumeHandle( std::string cmd )
{
	m_dbg->resume();
	return "OK";
}

std::string CDebugServer::haltHandle( std::string cmd) 
{
	m_dbg->halt();
	return getPCHandle( cmd );
}

std::string CDebugServer::stepHandle( std::string cmd) 
{
	m_dbg->step();
	return getPCHandle( cmd );
}

std::string CDebugServer::getPCHandle( std::string cmd )
{
	char temp[100];
	int iStatus = m_dbg->getStatus();
	int iACC = m_dbg->getSFR(CCDebugInterface::ACC);
	int iBank = m_dbg->getSFR(CCDebugInterface::FMAP) & 0x3;
	int iFMAP = iBank;
	int iAddr = m_dbg->getPC();
	if( iAddr < 0x8000 )
		iBank = 0;
	sprintf(temp, "PC %d:%04X ACC=%02X", iBank, iAddr, iACC );
	
	m_dbg->setSFR(CCDebugInterface::ACC, iACC );
	
	if( fpLog ){
		fprintf( fpLog, "%s FMAP=%02X\n",temp,iFMAP);
	}
	return temp;
}

std::string CDebugServer::getRegistersHandle( std::string cmd )
{
	return "OK";
}

std::string CDebugServer::getIDHandle( std::string cmd )
{
	return m_dbg->getChipType( m_dbg->getChipID() );
}

std::string CDebugServer::getSFRHandle( std::string cmd )
{	
	char temp[100];
	bool bHalted = m_dbg->getStatus() & CCDebugInterface::CPU_HALTED;
	if( !bHalted )
		m_dbg->halt();
	int iACC = m_dbg->getSFR(0xE0 );
	
	sprintf(temp, "SFR %02X = %02X", getPar( cmd, 0), m_dbg->getSFR(getPar( cmd, 0 )) );
	m_dbg->setSFR(0xE0,iACC );
	
	if( !bHalted )
		m_dbg->resume();
	
	return temp;
}
std::string CDebugServer::setSFRHandle( std::string cmd )
{
	bool bHalted = m_dbg->getStatus() & CCDebugInterface::CPU_HALTED;
	if( !bHalted )
		m_dbg->halt();
		
	m_dbg->setSFR( getPar( cmd, 0), getPar( cmd, 1 ));
	
	if( !bHalted )
		m_dbg->resume();
	
	return "OK";
}

int CDebugServer::getPar( std::string cmd, int iPar )
{
	char p0[1000];
	int p[20];
	int iCnt = sscanf( cmd.c_str(), "%s %x %x %x %x %x %x %x %x",p0, &p[0],&p[1],&p[2],&p[3],&p[4],&p[5],&p[6],&p[7] );
	if( iCnt < (iPar + 2) )
		throw MissingParameter(iPar);
	return p[iPar];
}

std::string CDebugServer::breakpointHandle( std::string cmd )
{
	int iNum = getPar(cmd, 0);
	int iEn = getPar(cmd, 1);
	int iAddr = getPar(cmd, 2);
	if( iEn )
		m_dbg->setHwBreakpoint( iNum,  iAddr >>16, iAddr & 0xFFFF );
	else
		m_dbg->rmHwBreakpoint( iNum );
	return "OK";
}

std::string CDebugServer::flashHandle( std::string cmd )
{
	char temp1[1000];
	char czFile[1000];
	char *pFile = 0;
	if( sscanf( cmd.c_str(), "%s %s", temp1, czFile ) == 2 ){
		pFile = czFile;
		g_ccflasher->m_sFilename = czFile;
	}
	
	if( g_ccflasher->write( pFile ) )
		return "OK";
	else 
		return "ERR";
}

std::string CDebugServer::dumpHandle( std::string cmd )
{
	int iAddr = getPar( cmd, 0 );
	int iCnt = getPar( cmd, 1 );
	unsigned char buff[0x20000];
	
	int iACC = m_dbg->getSFR( 0xE0 );
	int iDPL = m_dbg->getSFR( 0x82 );
	int iDPH = m_dbg->getSFR( 0x83 );
	
	m_dbg->readXDATA( iAddr, buff, iCnt );
			
	m_dbg->setSFR( 0x82, iDPL );
	m_dbg->setSFR( 0x83, iDPH );
	m_dbg->setSFR( 0xE0, iACC );
				

	
	
	std::string str = "";
	char tmp[1000];
	sprintf(tmp,"%04X - %04X %d bytes\n", iAddr, iAddr + iCnt, iCnt );
	str += tmp;
	str += "-----------------------------------------------------\n";
	str += "     00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F";
	for( int i=0; i < iCnt ; i++ ){
		if( (i % 16) == 0){
			sprintf( tmp, "\n%04X:", iAddr + i );
			str += tmp;
		}
		if( (i % 16) == 8 ){
			str += " ";
		}
		sprintf( tmp, "%02X ", buff[ i ]);
		str += tmp;
	}
	str += "\nOK";
	return str;
}

std::string CDebugServer::resetHandle( std::string cmd )
{
	m_dbg->reset(true);
	usleep(200000);
	m_dbg->reset(false);
	m_dbg->enterDebugMode();
	m_dbg->command(CCDebugInterface::DEBUG_INSTR1,0x00 );//nop, to get debug status (otherwise status is not valid)
	return "OK";
}

std::string CDebugServer::statusHandle( std::string cmd )
{
	return m_dbg->getStatusDesc();
}


/*NOICE*/
bool  CDebugServer::noICEHandler( NoICEFrame &inf, NoICEFrame &outf )
{
	switch( inf.function() ){
	case NoICEFrame::FN_GET_STATUS:
		outf.addByte(4);//processor type 8051
		outf.addByte(255);//buffer size
		outf.addByte(0x70);
		outf.addByte(0);//Low bound of target mapped memory (0 if not mapped). Least significant byte first.
		outf.addByte(0x80);
		//outf.addByte(0);//Low bound of target mapped memory (0 if not mapped). Least significant byte first.
		//outf.addByte(0);
		outf.addByte(0xFF);//High bound of target mapped memory (0 if not mapped). Least significant byte first.
		outf.addByte(0xFF);	
		outf.addByte(1);//Length of target's breakpoint instruction.
		outf.addByte(0xA9);//Length of target's breakpoint instruction.
		outf.addByte('c');
		outf.addByte('c');
		outf.addByte('2');
		outf.addByte('4');
		outf.addByte('3');
		outf.addByte('0');
		outf.addByte(0xa9);
		
		return true;
		break;
	case NoICEFrame::FN_RESET_TARGET:
		INFO("NoICE: FN_RESET_TARGET\n")
		m_dbg->enterDebugMode();
		m_dbg->halt();
		m_dbg->setPC(0);
		initCodeCache();
		break;
	case NoICEFrame::FN_STOP_TARGET:
		INFO("NoICE: FN_STOP_TARGET\n")
		m_dbg->halt();
		break;
	
	case NoICEFrame::FN_RUN_TARGET:
		INFO("NoICE: FN_RUN_TARGET\n")
		m_dbg->resume();
		//goto label_readregs;
		return false;
	case NoICEFrame::FN_SET_BYTES://breakpoints
		{
			INFO("NoICE: FN_SET_BYTES\n");
			for( int i = 0; i < inf.len();i+=4){
				int iBank = inf.data()[i+0] & 0x03;
				int iAddr = inf.data()[i+1];
				iAddr += (inf.data()[i+2]) << 8;
				if( (inf.data()[i+3]) == 0xA9 ){
					printf("NoICE set BP at %02X%04X\n ", iBank, iAddr);
					int j;
					for( j=0; j<4; j++ ){
						if( !usedBP[j])
							break;
					}
					if( j == 4){
						INFO("NoICE: out of breakpoints\n");
						return true;
					}
					m_dbg->setHwBreakpoint( j, iBank, iAddr );
					usedBP[j] = 1;
					unsigned char origByte;
					getCode( iAddr | (iBank<<16), 1, &origByte );
					outf.addByte( j );//return the original byte
				}else/* if( (inf.data()[i+3]) & 0x32 )*/{//remove bp
					int iBPnum = (inf.data()[i+3]) & 3;
					int iBank = inf.data()[i+0] & 0x03;
					int iAddr = inf.data()[i+1];
					usedBP[iBPnum] = 0;
					printf("NoICE remove BP  %02X %02X %04X\n ", iBPnum, iBank, iAddr );
					m_dbg->rmHwBreakpoint( iBPnum );
					outf.addByte(0xA9);
				}
			}
		}
		break;
		
	case NoICEFrame::FN_STEP:
			INFO("NoICE: FN_STEP\n")
			m_dbg->step();;
label_readregs:	
	case NoICEFrame::FN_READ_REGS:
	{
		INFO("NoICE: FN_READ_REGS\n");
		bool bHalted = m_dbg->getStatus() & CCDebugInterface::CPU_HALTED;
		if( !bHalted )
			m_dbg->halt();
		
		int iACC = m_dbg->getSFR( 0xE0 );
		
		int iAddr = m_dbg->getPC();
		int iPage = (m_dbg->getSFR( 0x9F ) & 0x03);
		iPage |= 0xC0;
		
		if( iAddr < 0x8000)
			iPage = 0x00;//below 0x8000 allways first bank
		
		int iPSW = m_dbg->getSFR( 0xd0 );
		
		outf.addByte( 0 );//state
		outf.addByte( iPage );//page = FMAP ( for code pages )
		outf.addByte( iPSW  );//PWS
		outf.addByte( m_dbg->getSFR( 0x82 ) );//DPTR L
		outf.addByte( m_dbg->getSFR( 0x83 ) );//DPTR H
		outf.addByte( m_dbg->getSFR( 0xA0 ) );//IE
		outf.addByte( iACC );//ACC
		outf.addByte( m_dbg->getSFR( 0xF0 ) );//B
		
		for( int i=0; i < 8; i++ ){
			outf.addByte( m_dbg->getSFR( i + ((iPSW>>3)&3 )*8 ));//R
		}
		
		outf.addByte( iAddr & 0xFF );//PCL
		outf.addByte( (iAddr >> 8) & 0xFF );//PCH
		outf.addByte( m_dbg->getSFR( 0x81 ) );//B
		
		m_dbg->setSFR( 0xE0 ,iACC );// restore acc as it's modified by getSFRs
		
		if( !bHalted )
			m_dbg->resume();
				
		return true;
	}
	case NoICEFrame::FN_READ_MEM:
	{
		
		int iPage = inf.data()[0];
		int iAddr = inf.data()[1];
		iAddr += (inf.data()[2]) << 8;
		int iCnt = inf.data()[3];
		INFO("NoICE: FN_READ_MEM page=0x%X,addr=0x%04X,cnt=0x%X\n",iPage, iAddr,iCnt);
		
		if(  (iPage & 0xF0) == 0xC0 || iAddr < 0x8000){//we have code page
			TRACE("CODE read: %02X %04X %02X\n", iPage, iAddr,iCnt );
			unsigned char temp[1000];
			//restore registers fucked up by readCode

			getCode(iAddr | ((iPage&0x03)<<16),iCnt,temp);
					
			for( int i=0; i < iCnt ; i++){
				outf.addByte( temp[i] );
			}
		}else if(  (iPage & 0xFF) == 0x0D ){//we have DATA page
			TRACE("DATA read: %02X %04X %02X\n", iPage, iAddr,iCnt );
			unsigned char temp[1000];
			//restore registers fucked up by readCode
			int iACC = m_dbg->getSFR( 0xE0 );
			int iDPL = m_dbg->getSFR( 0x82 );
			int iDPH = m_dbg->getSFR( 0x83 );
			
			m_dbg->readXDATA( iAddr, temp, iCnt );
					
			m_dbg->setSFR( 0x82, iDPL );
			m_dbg->setSFR( 0x83, iDPH );
			m_dbg->setSFR( 0xE0, iACC );
			
			for( int i=0; i < iCnt ; i++){
				outf.addByte( temp[i] );
			}
		}
		else{
			LOG_ERROR("NoICE: FN_READ_MEM unknown page page=0x%X,addr=0x%04X,cnt=0x%X\n",iPage, iAddr,iCnt);	
		}
		return true;
	}
	default:
		outf.setFunction( NoICEFrame::FN_ERROR );
	}	              	     
	return true;
}

void CDebugServer::initCodeCache()
{
	int i;
	for( i=0; i < sizeof( codeCache)/sizeof(unsigned char); i++ ){
		codeCache[i] = 0xFF;
		codeCacheValid[i] = 0;
 	}
	for( i=0; i < 4; i++ ){
		usedBP[i] = 0;
	}
}


void CDebugServer::getCode( int iAddr, int iCnt, unsigned char *pBuff)
{
	iAddr = iAddr & 0x3FFFF;
	int iFInvalid = -1;
	int iLInvalid = iAddr;
	//get the maximum valid range that we have
	for( int i=0; i < iCnt; i++ ){
		if( codeCacheValid[i + iAddr] == 0 && iFInvalid == -1 ){
			iFInvalid = i + iAddr;
		}
		if( codeCacheValid[i + iAddr] == 0 ){
			iLInvalid = i + iAddr;
		}
	}
	INFO("Getting code %06X %X %d %d\n", iAddr, iCnt, iFInvalid, iLInvalid );
	if( iFInvalid != -1 ){//do we need to read some more code?
		int iReadBegin = iFInvalid;
		int iReadEnd   = iLInvalid;
		
		//get more code than requested ( it's faster )
		if( (iReadBegin / FLASH_BANK_SIZE) == ((iReadBegin + 64)/FLASH_BANK_SIZE) && (iReadBegin + 64) > iReadEnd ){
			iReadEnd =  iReadBegin + 64;
		}
		
		//restore registers fucked up by readCode
		int iACC = m_dbg->getSFR( 0xE0 );
		int iPSW = m_dbg->getSFR( 0xd0 );
		int iBackMEMCTRL = m_dbg->getSFR( 0xC7 );
		int iBackDPTRL = m_dbg->getSFR( 0x82 );
		int iBackDPTRH = m_dbg->getSFR( 0x83 );
		
		bool bHalted = m_dbg->getStatus() & CCDebugInterface::CPU_HALTED;
		if( !bHalted )
			m_dbg->halt();
		
		//banked address to address in flash
		int iFlashAddress = (iReadBegin & 0x7FFF) + 0x8000 * (iReadBegin >> 16);
		
		m_dbg->readCode( iFlashAddress, &codeCache[iReadBegin], iReadEnd - iReadBegin + 1 );
		
		if( !bHalted )
			m_dbg->resume();
		
		m_dbg->setSFR( 0xC7, iBackMEMCTRL );
		m_dbg->setSFR( 0x82, iBackDPTRL );
		m_dbg->setSFR( 0x83, iBackDPTRH );
		m_dbg->setSFR( 0xd0, iPSW );
		m_dbg->setSFR( 0xe0, iACC );
		
		memset( &codeCacheValid[ iReadBegin], 1, iReadEnd - iReadBegin + 1 );
	}
	memcpy( pBuff, &codeCache[iAddr], iCnt );
}
