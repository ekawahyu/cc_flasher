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

#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <string.h>
#include <getopt.h>
#include <math.h>
#include <errno.h>

#include "CCFlasher.h"
#include <log.h>

#ifndef min
#define min(A,B) ((A) < (B) ? (A) : (B))
#endif 

CCFlasher *g_ccflasher;

CCFlasher::CCFlasher():
	m_iChipID(0)
{
	m_iOperation = GET_CHIP_ID;
	m_bMassErase = false;
	m_pDbgInt = NULL;
	m_iDebugClockDelay = 1;
	m_pBuff = NULL;
	m_bResume = true;
	m_bRestoreMac = true;
	m_iVerify = 1;
	
	g_ccflasher = this;
}

CCFlasher::~CCFlasher()
{
	if( m_pDbgInt )
		delete m_pDbgInt;
	if( m_pBuff )
		delete m_pBuff;
}

void CCFlasher::startDebugServer( CCDebugInterface *dbg )
{
	m_pDbgInt->led( true ) ;
	m_pDbgInt->reset(true);
	m_pDbgInt->enterDebugMode();
	m_pDbgInt->setDebugClockDelay(m_iDebugClockDelay);
	m_pDbgInt->getChipID();
	m_pDbgInt->command( CCDebugInterface::DEBUG_INSTR1,0x00 );// EXEC NOP, to resfresh debug lock bit in status
#ifdef CC_DEBUGER	
	CDebugServer ds( dbg, m_iServerPort );
	ds.run();
#endif //CC_DEBUGER
}

int CCFlasher::run( char *argv[], int argc )
{
	if( !parseOptions( argv, argc ) )
		return 1;
	
	bool bRet = true;;
	try{
		m_pDbgInt = new CCDebugInterfaceUSB( VENDOR_ID, PRODUCT_ID );
		if(!((CCDebugInterfaceUSB*)m_pDbgInt)->isInterfaceFound()){
			return 1;
		}
		bool bRet;
		switch( m_iOperation ){
		case WRITE:
			bRet = write( m_sFilename.c_str() );
			break;
		case TEST:
			bRet =test();
			break;
		case DEBUG_SERVER:
			startDebugServer(m_pDbgInt);
			break;
		case RESET:
			m_pDbgInt->reset(true);
			m_pDbgInt->reset(false);
			break;
		case GET_CHIP_ID:
		default:
			{	
				m_pDbgInt->led( true ) ;
				m_pDbgInt->reset(true);
				m_pDbgInt->enterDebugMode();
				m_pDbgInt->setDebugClockDelay(m_iDebugClockDelay);
				
				int iID = m_pDbgInt->getChipID();
				printf("[INFO] Chip id = %02X %s\n", iID, m_pDbgInt->getChipType( iID ).c_str() );
				
				m_pDbgInt->led( false ) ;
				return 0;
			}
			break;
		}
	}catch( CUsbException e){
		fprintf( stderr, "[ERROR] CUsbException\n");
		return 1;
	}
	return bRet ? 0 : 1;
}
bool CCFlasher::loadFile( const char *czFilename )
{
	if( !czFilename )
		czFilename = m_sFilename.c_str();
	if( m_pBuff )
		delete m_pBuff;
	m_pBuff = new unsigned char[ FLASH_SIZE ];
	
	FILE *fp = fopen( czFilename, "rb");
	if( !fp ){
		fprintf( stderr, "[ERROR] File open ERROR. File %s, ERROR %s ", czFilename, strerror( errno ) );
		return false;
	}

	memset( m_pBuff, 0xFF, FLASH_SIZE );
	int iRead = fread( m_pBuff, 1, FLASH_SIZE, fp );
	if( ferror( fp ) ){
		fprintf( stderr, "[ERROR] File read ERROR. File %s, ERROR %s ", czFilename, strerror( errno ) );
		fclose( fp );
		return false;
	}
	fclose( fp );
	return true;
}

void CCFlasher::usage()
{
	printf("cc_flasher:\n"
			"-f | --filename <input binary file>      (use objcopy -I ihex -O binary to convert to bin)\n"
			"-w | --write          Write flash\n"
			"-m |  --masserase     Do mass erase before programming\n"
			"-v |  --verify 0|1|2  Verify flash after writing 0=no, 1=verify first page, 2=verify all pages\n"
			"-x |  --reset         Reset target\n"
			"-i |  --chipid        Get chip id (this is the default if called without parameters\n"
			"-a |  --mac xx:xx:xx:xx:xx:xx:xx:xx New mac address ( this will be written to the end of the flash )\n"
			"-s |  --server <port> Start debug server\n"
			"           on port:   Command line interface\n"
			"         on port+1:   NoICE serial protocol, use some kind of virtual serial port that connects\n"
			"                      to tcp for connection with NoICE, this may be socat on linux or \n"
			"                      \"HW Virtual Serial Port\" on windows. Install native mfcXX.dll to run NoICE\n"
			"                      under wine on linux\n" );
}

int CCFlasher::parseOptions( char *argv[], int argc )
{
	int c;
    int digit_optind = 0;
    while (1) {
        int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
            {"filename", 1, 0, 'f'},
            {"write", 0, 0, 'w'},
            {"read", 0, 0, 'r'},
            {"verify", 1, 0, 'v'},
            {"erase", 0, 0, 'e'},
            {"masserase",0,0,'m'},
            {"chipid",0,0,'i'},
            {"clock", 0,0,'c'},		
            {"resume",0,0,'u'},
            {"reset",0,0,'x'},
            {"test",0,0,'t'},
            {"mac",1,0,'a'},
            {"server",1,0,'s'},
            {"help",0,0,'h'},
            {0, 0, 0, 0}
        };
        c = getopt_long (argc, argv, "f:wrv:emicutha:s:x",
                 long_options, &option_index);
        if (c == -1)
            break;
        switch (c) {
        case 'h':
    		usage();
    		return 0;//stop processing
        case 'f':
        	m_sFilename = optarg;
        	break;
        case 'w':
        	m_iOperation = WRITE;
        	break;
        case 'r':
            m_iOperation = READ;
            break;
        case 'e':
            m_iOperation = ERASE;
            break;
        case 'i':
        	m_iOperation = GET_CHIP_ID;
        	break;
        case 'x':
            m_iOperation = RESET;
            break;
        case 'm':
        	m_bMassErase = true;
        	break;
        case 't':
        	m_iOperation = TEST;
            break;
        case 's':
            m_iOperation = DEBUG_SERVER;
            m_iServerPort = atoi( optarg );
            break;
        case 'c':
        	m_iDebugClockDelay = min( atoi( optarg ),1 );
        	break;
        case 'a':
            m_sMac = optarg;
            break;
        case 'v':
        	m_iVerify = atoi( optarg );
        	break;
        default:
            printf ("option %s", long_options[option_index].name);
            if (optarg)
                printf (" with arg %s", optarg);
            printf ("\n");
            break;
        }
    }
    if (optind < argc) {
        printf ("non-option ARGV-elements: ");
        while (optind < argc)
            printf ("%s ", argv[optind++]);
        printf ("\n");
    }
    return 1;//cont
}

bool CCFlasher::write( const char *czFilename )
{
	if( !loadFile( czFilename ) )
		return false;
	
	
	
	m_pDbgInt->led( true ) ;
	m_pDbgInt->reset(true);
	m_pDbgInt->enterDebugMode();
	m_pDbgInt->setDebugClockDelay(m_iDebugClockDelay);
	m_pDbgInt->getChipID();
	
	
	unsigned char pOldMAC[8];
	m_pDbgInt->readCodeBank(FLASH_SIZE -8, FLASH_SIZE/FLASH_BANK_SIZE -1, pOldMAC, 8 );
	printf("[INFO] Old MAC: ");
	for( int i=0; i < 8;i++) printf("%02X ", pOldMAC[i]);
	printf("\n");
	
	if( m_bRestoreMac ){
		memcpy( m_pBuff+  (FLASH_SIZE -8), pOldMAC , 8 );
		printf("[INFO] Restoring MAC\n");
	}
	if( m_sMac.length() ){
		unsigned char mac[8];
		if( sscanf( m_sMac.c_str(),"%X:%X:%X:%X:%X:%X:%X:%X"
				, &mac[0]
				, &mac[1]
			  	, &mac[2]
			  	, &mac[3]
			  	, &mac[4]
			  	, &mac[5]
			  	, &mac[6]
			  	, &mac[7]
		) == 8 ){
			
			printf("[INFO] Using new mac %s\n",  m_sMac.c_str());
			memcpy( m_pBuff+  (FLASH_SIZE -8), mac , 8 );
		}
	}
	
	
	if( m_bMassErase ){
		printf("[INFO] Will do mass Erase\n");
	}
	int iRet = m_pDbgInt->programFlash( m_pBuff, FLASH_SIZE, m_bMassErase, m_iVerify );
	if( iRet ){
		m_pDbgInt->led( false ) ;
	}else{
		m_pDbgInt->led( true ) ;//indicate LOG_ERROR
		fprintf(stderr,"[ERROR] Flash failed\n");
	}
	if( m_bResume ){
		m_pDbgInt->reset(true);
		m_pDbgInt->reset(false);
	}
	return iRet;
}

bool CCFlasher::test( )
{
	m_pDbgInt->led( true ) ;
	m_pDbgInt->reset(true);
	m_pDbgInt->enterDebugMode();
	m_pDbgInt->setDebugClockDelay(m_iDebugClockDelay);
	
	int iRet = m_pDbgInt->testChip( );
	
	m_pDbgInt->led( false ) ;
	
	m_pDbgInt->reset(true);
	m_pDbgInt->reset(false);
	
	return iRet;
}


