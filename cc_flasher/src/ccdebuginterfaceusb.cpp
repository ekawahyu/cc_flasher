#include <ccdebuginterfaceusb.h>
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

#include <stdio.h>

#include <log.h>

#define USB_DIR_IN  0x80
#define DEBUG_COMMAND                4
#define VENDOR_LED	1
#define VENDOR_RESET 2
#define VENDOR_ENTER_DEBUG_MODE 3
#define VENDOR_DEBUG 4
#define VENDOR_DEBUG_CLOCK_DELAY 5
#define VENDOR_XDATA32 6
#define VENDOR_READCODE 7

CCDebugInterfaceUSB::CCDebugInterfaceUSB( unsigned int iVendorID, unsigned int iProductID  )
{
	m_phUSB = 0;
	usb_init();
	struct usb_device *pDevice = findDevice( iVendorID, iProductID );
	if( pDevice )
		m_phUSB = openDevice( pDevice );
}

bool CCDebugInterfaceUSB::isInterfaceFound()
{
	return m_phUSB != NULL ? true:false;
}

void CCDebugInterfaceUSB::rawCommand( unsigned char cCommand, unsigned char cByte1,  unsigned char cByte2,  unsigned char cByte3, int iInBytes, unsigned char *pcInputBytes )
{
	if( !m_phUSB )
		return;

	volatile unsigned int usPar1 = cCommand | (cByte1<<8);
	volatile unsigned int usPar2 = cByte2 | (cByte3<<8);
	
    int ret = usb_control_msg(
              m_phUSB,
              USB_TYPE_VENDOR | USB_DIR_IN , VENDOR_DEBUG,
              usPar1 , usPar2,
              (char*)pcInputBytes, iInBytes,
              /*timeout*/50 );
	TRACE2("send_command ret %d \n", ret);
	TRACE2("Received %d bytes ", ret, pcInputBytes[0], pcInputBytes[1], pcInputBytes[2], pcInputBytes[3]);
	for( int i=0; i < iInBytes; i++)
		TRACE2("%02X ", pcInputBytes[i]);
	TRACE2("\n");
	if( ret < 0 ){
		LOG_ERROR( "usb_control_msg LOG_ERROR %d\n", ret);
		throw CUsbException( ret );
	}
}

void CCDebugInterfaceUSB::reset( bool bOn)
{
	if( !m_phUSB )
			return;
		
	unsigned char str[ 36 ];

    int ret = usb_control_msg(
              m_phUSB,
              USB_TYPE_VENDOR | USB_DIR_IN , VENDOR_RESET,
              bOn ? 1:0 , bOn ? 1:0,
              (char*)str, 0,
              /*timeout*/50 );
	TRACE2("send_command ret %d \n", ret);
	TRACE2("Received %d bytes %02X %02X %02X %02X\n", ret, str[0], str[1], str[2], str[3]);
	if( ret < 0 ){
		LOG_ERROR( "usb_control_msg LOG_ERROR %d\n", ret);
		throw CUsbException( ret );
	}
}

void CCDebugInterfaceUSB::enterDebugMode()
{
	if( !m_phUSB )
			return;
		
	unsigned char str[ 36 ];

    int ret = usb_control_msg(
              m_phUSB,
              USB_TYPE_VENDOR | USB_DIR_IN , VENDOR_ENTER_DEBUG_MODE,
              0,0,
              (char*)str, 0,
              /*timeout*/50 );
	TRACE2("send_command ret %d \n", ret);
	TRACE2("Received %d bytes %02X %02X %02X %02X\n", ret, str[0], str[1], str[2], str[3]);	
	
	if( ret < 0 ){
		LOG_ERROR( "usb_control_msg LOG_ERROR %d\n", ret);
		throw CUsbException( ret );
	}
}

struct usb_device *CCDebugInterfaceUSB::findDevice( unsigned int iVendorID, unsigned int iProductID )
{
	int ret = 0;
    struct usb_bus *bus;
    struct usb_device *dev;
    
    INFO( "searching for cc_flasher\n" );
    
    ret = usb_find_busses();
    INFO( "usb_find_busses: %d\n", ret );
    ret = usb_find_devices();
    INFO( "usb_find_devices: %d\n", ret );

    for ( bus = usb_busses; bus; bus = bus->next )
    {
        for ( dev = bus->devices; dev; dev = dev->next )
        {
            TRACE( "%s/%s     %04X/%04X\n", bus->dirname, dev->filename,
                        dev->descriptor.idVendor, dev->descriptor.idProduct );
            if ( dev->descriptor.idVendor != iVendorID || dev->descriptor.idProduct != iProductID )
            {
                continue;
            }
            else
            {
                INFO( "Found it\n" );
                return dev;
            }
        }
    }
    LOG_ERROR("Not found");
    return 0;
}

CCDebugInterfaceUSB::~CCDebugInterfaceUSB()
{
	if( m_phUSB ){
		usb_release_interface( m_phUSB, 0 );
		usb_close( m_phUSB );
	}
}

struct usb_dev_handle *CCDebugInterfaceUSB::openDevice( struct usb_device *dev )
{
	TRACE( "openning device \n");
    int ret;
    usb_dev_handle *udev;
    udev = usb_open( dev );
    usb_set_configuration(udev, 1);
	
    if ( !udev ){
        LOG_ERROR( "usb_open LOG_ERROR\n");
    	return 0;
    }
    ret = usb_claim_interface( udev, 0 );
    if ( ret )
    {
    	LOG_ERROR( "Unable to claim interface %d\n", ret);
    	usb_reset( udev );
        usb_close( udev );
        return 0;
    }
    TRACE("device openned\n");
    return udev;
}

void CCDebugInterfaceUSB::init()
{
	
}

void CCDebugInterfaceUSB::led( bool bOn)
{
	if( !m_phUSB )
			return;
		
	unsigned char str[ 36 ];

    int ret = usb_control_msg(
              m_phUSB,
              USB_TYPE_VENDOR | USB_DIR_IN , VENDOR_LED,
              bOn ? 1:0 , bOn ? 1:0,
              (char*)str, 0,
              /*timeout*/50 );
	TRACE2("send_command ret %d \n", ret);
	TRACE2("Received %d bytes %02X %02X %02X %02X\n", ret, str[0], str[1], str[2], str[3]);	
	
	if( ret < 0 ){
		LOG_ERROR( "usb_control_msg LOG_ERROR %d\n", ret);
		throw CUsbException( ret );
	}
}

void CCDebugInterfaceUSB::setDebugClockDelay( unsigned char chDelay )
{
	if( !m_phUSB )
			return;
		
	unsigned char str[ 36 ];

    int ret = usb_control_msg(
              m_phUSB,
              USB_TYPE_VENDOR | USB_DIR_IN , VENDOR_DEBUG_CLOCK_DELAY,
              chDelay , chDelay,
              (char*)str, 0,
              /*timeout*/50 );
	TRACE2("send_command ret %d \n", ret);
	TRACE2("Received %d bytes %02X %02X %02X %02X\n", ret, str[0], str[1], str[2], str[3]);	
	
	if( ret < 0 ){
		LOG_ERROR( "usb_control_msg LOG_ERROR %d\n", ret);
		throw CUsbException( ret );
	}
}

#define USB_DIR_OUT 0

/**
 * for usbtiny gcc version of interface
 */
void CCDebugInterfaceUSB::writeXDATADirect( unsigned char *pBuff, int iCnt )
{
	if( !m_phUSB )
				return;
	unsigned char str[ 36 ];			
	
	volatile unsigned int usPar1 = pBuff[0] | (pBuff[1]<<8);
	volatile unsigned int usPar2 = pBuff[2] | (pBuff[3]<<8);
		
    int ret = usb_control_msg(
              m_phUSB,
              USB_TYPE_VENDOR | USB_DIR_OUT , VENDOR_XDATA32,
              0 , 0,
              (char*)pBuff, iCnt,
              /*timeout*/50 );
	TRACE2("send_command ret %d \n", ret);
	TRACE2("Received %d bytes %02X %02X %02X %02X\n", ret, str[0], str[1], str[2], str[3]);	
	
	if( ret < 0 ){
		LOG_ERROR( "usb_control_msg LOG_ERROR %d\n", ret);
		throw CUsbException( ret );
	}
}
/**
 * for igor asm version of interface
 */
void CCDebugInterfaceUSB::writeXDATA32( unsigned char *pBuff )
{
	if( !m_phUSB )
				return;
	unsigned char str[ 36 ];			
	
	volatile unsigned int usPar1 = pBuff[0] | (pBuff[1]<<8);
	volatile unsigned int usPar2 = pBuff[2] | (pBuff[3]<<8);
		
    int ret = usb_control_msg(
              m_phUSB,
              USB_TYPE_VENDOR | USB_DIR_IN , VENDOR_XDATA32,
              usPar1 , usPar2,
              (char*)str, 0,
              /*timeout*/50 );
	TRACE2("send_command ret %d \n", ret);
	TRACE2("Received %d bytes %02X %02X %02X %02X\n", ret, str[0], str[1], str[2], str[3]);	
	
	if( ret < 0 ){
		LOG_ERROR( "usb_control_msg LOG_ERROR %d\n", ret);
		throw CUsbException( ret );
	}
}

void CCDebugInterfaceUSB::readCodeDirect( int iAddr, unsigned char *pBuff, int iCnt)
{
	if( !m_phUSB )
					return;
				
		
    int ret = usb_control_msg(
              m_phUSB,
              USB_TYPE_VENDOR | USB_DIR_IN , VENDOR_READCODE,
              iAddr , iAddr,
              (char*)pBuff, iCnt,
              /*timeout*/50 );
	TRACE2("send_command ret %d \n", ret);
	///TRACE2("Received %d bytes %02X %02X %02X %02X\n", ret, str[0], str[1], str[2], str[3]);	
	
	if( ret < 0 ){
		LOG_ERROR( "usb_control_msg LOG_ERROR %d\n", ret);
		throw CUsbException( ret );
	}
}
