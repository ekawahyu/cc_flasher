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

#include <utils.h>

#include <string>

#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include <sys/types.h>
#include <sys/time.h>


using namespace std;

string getPreciseTimeStamp( time_t tTime /*= 0*/ )
{
        char czTimestamp[30];
        struct timeval ttime;
        struct timezone tzone;
        gettimeofday( &ttime, &tzone );

        // ce ni podan, vzamemo trenutnega
        struct tm *tdate = localtime( tTime ? &tTime : &ttime.tv_sec );


        sprintf( czTimestamp, "%04d-%02d-%02d-%02d.%02d.%02d.%06d", 1900 + tdate->tm_year,

                       tdate->tm_mon + 1,//januar = 0

                       tdate->tm_mday,

                       tdate->tm_hour,

                       tdate->tm_min,

                       tdate->tm_sec,

                       tTime ? 0 : ttime.tv_usec );
        return czTimestamp; 
}

string getDateString( time_t tTime /*= 0*/ )
{
        char czDate[30];
        struct timeval ttime;
        struct timezone tzone;
        gettimeofday( &ttime, &tzone );

        // ce ni podan, vzamemo trenutnega
        struct tm *tdate = localtime( tTime ? &tTime : &ttime.tv_sec );


        sprintf( czDate, "%04d-%02d-%02d", 1900 + tdate->tm_year,

                       tdate->tm_mon + 1,//januar = 0

                       tdate->tm_mday );
        return czDate; 
}

int utils_loglevel = 0;
void setLogLevel( int iLevel ){
	utils_loglevel = iLevel;
}

void logit( const char *czApp, const char *czFunction, int iLevel, const char *czString, ... )
{
	static FILE *fp=NULL;

	char czTemp[1000];
	
	
	if( iLevel > utils_loglevel )
		return;//dot log this level		

	va_list ap;
	va_start(ap,czString);
	vsprintf( czTemp , czString, ap);
	va_end(ap);
	
	printf( "%s %s - %s - %s(): %s\n", logLevels[iLevel], getPreciseTimeStamp().c_str(), czApp,czFunction, czTemp );

	if(!fp)
	{
		std::string sFilename = "/var/log/hae";
		sFilename += getDateString();
		sFilename += ".log";
		fp = fopen( sFilename.c_str(),"a+");
		if( !fp )
			perror("Opening log file:");
	}
	if(fp)
	{
		
		fprintf(fp,"%s %s - %s - %s(): %s\n",  logLevels[iLevel], getPreciseTimeStamp().c_str(), czApp,czFunction, czTemp );
		fflush(fp);	
	}
}
void logit2( const char *czFilename,  const char *czString, ... )
{
	FILE *fp=NULL;

	char czTemp[1000];
		
	va_list ap;
	va_start(ap,czString);
	vsprintf( czTemp , czString, ap);
	va_end(ap);
	
	printf( "%s: %s\n", getPreciseTimeStamp().c_str(), czTemp );

	if(!fp)
	{
		fp = fopen(czFilename,"a+");
		if( !fp )
			perror( czFilename );
	}
	if(fp)
	{
		fprintf(fp,"%s: %s\n", getPreciseTimeStamp().c_str(), czTemp );
		fflush(fp);	
		fclose( fp );
	}
	
}
