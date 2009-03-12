// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2009 J.Rios
//	anonbeat@gmail.com
//
//    This Program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2, or (at your option)
//    any later version.
//
//    This Program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; see the file LICENSE.  If not, write to
//    the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#include "RadioProxy.h"

#include "Commands.h"
#include "Utils.h"

#include <wx/strconv.h>
#include <wx/datetime.h>

#define GURADIOPROXY_SLEEPTIME          100
#define GURADIOPROXY_TIMEOUT            50
#define GURADIOPROXY_METADATABUFSIZE    4096

// -------------------------------------------------------------------------------- //
// guRadioProxyServer
// -------------------------------------------------------------------------------- //
guRadioProxyServer::guRadioProxyServer( guPlayerPanel * playerpanel )
{
    PlayerPanel         = playerpanel;
    RadioProxyClient    = NULL;
    SocketServer        = NULL;

    wxIPV4address addr;
    addr.Hostname( wxT( "127.0.0.1" ) );
    addr.Service( 11752 ); // TODO : Add option to configure this port
    SocketServer = new wxSocketServer( addr, wxSOCKET_NOWAIT );
    if( SocketServer )
    {
        if( !SocketServer->IsOk() )
        {
            guLogError( wxT( "Could not open local port for radio proxy server" ) );
        }
        if( Create() == wxTHREAD_NO_ERROR )
        {
            Run();
        }
    }
    else
    {
        guLogError( wxT( "Could not create the RadioProxy Server Object" ) );
    }
}

// -------------------------------------------------------------------------------- //
guRadioProxyServer::~guRadioProxyServer()
{
    if( RadioProxyClient )
    {
        RadioProxyClient->Pause();
        RadioProxyClient->Delete();
    }

    if( !TestDestroy() )
    {
        //guLogMessage( wxT( "Clear the PlayerPanel RadioProxy var" ) );
//        PlayerPanel->ClearRadioProxy();
    }

    if( SocketServer )
    {
        SocketServer->Close();
        SocketServer->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
bool guRadioProxyServer::ClientRunning( void )
{
    return RadioProxyClient != NULL;
}

// -------------------------------------------------------------------------------- //
void guRadioProxyServer::StopClient( void )
{
    if( RadioProxyClient )
    {
        RadioProxyClient->Pause();
        RadioProxyClient->Delete();
        RadioProxyClient = NULL;
    }
}

// -------------------------------------------------------------------------------- //
void guRadioProxyServer::SetRemoteAddr( const wxString &address )
{
    StopClient();
    RemoteServer.Create( address );
}

// -------------------------------------------------------------------------------- //
guRadioProxyServer::ExitCode guRadioProxyServer::Entry()
{
    if( SocketServer )
    {
        SocketServer->SetTimeout( 10 );
        SocketServer->SetFlags( wxSOCKET_NOWAIT );
        // Waits for a new connection
        while( !TestDestroy() )
        {
            //guLogMessage( wxT( "Waiting for incomming connections..." ) );
            if( SocketServer->WaitForAccept( 0 ) )
            {
                wxSocketBase * SocketPlayer = SocketServer->Accept();
                if( SocketPlayer )
                {
                    //guLogMessage( wxT( "Player connected to the server" ) );
                    SocketPlayer->SetTimeout( 30 );
                    SocketPlayer->SetFlags( wxSOCKET_NOWAIT );
                    RadioProxyClient = new guRadioProxyClient( this, PlayerPanel,
                                                            SocketPlayer, RemoteServer );
                    if( !RadioProxyClient )
                        guLogError( wxT( "Could not create the Proxy Client Object" ) );
                }
            }
            if( SocketServer->Error() && ( SocketServer->LastError() != wxSOCKET_WOULDBLOCK ) )
                break;
            Sleep( GURADIOPROXY_SLEEPTIME );
        }
    }
    return 0;
}


// -------------------------------------------------------------------------------- //
// guRadioProxyClient
// -------------------------------------------------------------------------------- //
guRadioProxyClient::guRadioProxyClient( guRadioProxyServer * radioproxyserver,
        guPlayerPanel * playerpanel, wxSocketBase * socketplayer, const wxURI &remoteaddress )
{
    RadioProxyServer    = radioproxyserver;
    PlayerPanel         = playerpanel;
    SocketPlayer        = socketplayer;
    SocketClient        = NULL;
    RemoteServer        = remoteaddress;

    if( Create() == wxTHREAD_NO_ERROR )
    {
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guRadioProxyClient::~guRadioProxyClient()
{
    if( !TestDestroy() )
    {
        RadioProxyServer->RadioProxyClient = NULL;
    }

    if( SocketPlayer )
        SocketPlayer->Destroy();
    if( SocketClient )
        SocketClient->Destroy();
}

// -------------------------------------------------------------------------------- //
wxString guRadioProxyClient::ReadLine( wxSocketBase * socket )
{
    wxString RetVal = wxEmptyString;
    char Buffer[ 16384 ];

    while( !TestDestroy() )
    {
        if( socket->WaitForRead( 0 ) )
        {
            socket->Peek( Buffer, sizeof( Buffer ) - 1 );
            int ReadCnt = socket->LastCount();

            if( !ReadCnt && socket->Error() )
            {
                if( socket->LastError() != wxSOCKET_WOULDBLOCK )
                {
                    //guLogError( wxT( "ReadLine %i" ), socket->LastError() );
                    return wxEmptyString;
                }
            }

            if( ReadCnt )
            {
                //guLogMessage( wxT( "Buffer: %s\n" ), Buffer );
                Buffer[ ReadCnt ] = 0;
                char * EndPos = strstr( Buffer, "\r\n" );
                if( EndPos )
                {
                    * EndPos = 0;
                    RetVal += wxString( Buffer, wxConvLibc );
                    socket->Read( Buffer, ( EndPos + 2 ) - Buffer );
                    break;
                }
                else
                {
                    RetVal += wxString( Buffer, wxConvLibc );
                    socket->Read( Buffer, ReadCnt );
                }
            }
        }
        Sleep( GURADIOPROXY_SLEEPTIME );
    }
    //guLogMessage( wxT( "Line: '%s'" ), RetVal.c_str() );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
wxArrayString guRadioProxyClient::ReadHeaders( wxSocketBase * socket )
{
    wxArrayString RetVal;
    wxString CurLine;
    do {
        CurLine = ReadLine( socket );
//        guLogMessage( wxT( "Header: %s" ), CurLine.c_str() );
        RetVal.Add( CurLine );
        if( socket->Error() && ( socket->LastError() != wxSOCKET_WOULDBLOCK ) )
        {
            break;
        }
    } while( !CurLine.IsEmpty() && !TestDestroy() );
    //guLogMessage( wxT( "Finished ReadHeaders..." ) );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
int guRadioProxyClient::WriteHeaders( wxSocketBase * socket, const wxArrayString &headers )
{
    wxString AllHeaders = wxEmptyString;
    int index;
    int count = headers.Count();
    for( index = 0; index < count; index++ )
    {
        AllHeaders += headers[ index ];
        AllHeaders += wxT( "\r\n" );
    }
    socket->Write( AllHeaders.char_str(), AllHeaders.Length() );
//    guLogMessage( wxT( "WriteHeaders:\n%s" ), AllHeaders.c_str() );
    return socket->LastCount();
}

// -------------------------------------------------------------------------------- //
wxString guRadioProxyClient::GetHeader( const wxArrayString &headers, const wxString &header )
{
    wxString RetVal = wxEmptyString;
    int index;
    int count = headers.Count();
    for( index = 0; index < count; index++ )
    {
        if( headers[ index ].StartsWith( header, &RetVal ) )
        {
            break;
        }
    }
    return RetVal.Trim();
}

// -------------------------------------------------------------------------------- //
void guRadioProxyClient::SetHeader( const wxArrayString &headers, const wxString &header, const wxString &value )
{
    int index;
    int count = headers.Count();
    for( index = 0; index < count; index++ )
    {
        if( headers[ index ].StartsWith( header ) )
        {
            headers[ index ] = header + wxT( " " ) + value;
            break;
        }
    }
}


//                                         0553 7472              .Str
//	0x0170:  6561 6d54 6974 6c65 3d27 5072 696e 6365  eamTitle='Prince
//	0x0180:  202d 204b 6973 7320 2831 3938 3629 273b  .-.Kiss.(1986)';
//	0x0190:  5374 7265 616d 5572 6c3d 2768 7474 703a  StreamUrl='http:
//	0x01a0:  2f2f 7777 772e 3937 376d 7573 6963 2e63  //www.977music.c
//	0x01b0:  6f6d 273b 0000 0000 0000 0000 00
//

// -------------------------------------------------------------------------------- //
int guRadioProxyClient::ReadMetadata( wxSocketBase * socket, char * buffer )
{
    int ReadCnt;
    wxDateTime EndTime = wxDateTime::UNow() + wxTimeSpan::Seconds( GURADIOPROXY_TIMEOUT );
    while( !TestDestroy() )
    {
        if( socket->WaitForRead( 0 ) )
        {
            socket->Peek( buffer, GURADIOPROXY_METADATABUFSIZE );
            ReadCnt = socket->LastCount();

            if( !ReadCnt && socket->Error() )
            {
                if( socket->LastError() != wxSOCKET_WOULDBLOCK )
                {
                    break;
                }
            }

            if( ReadCnt )
            {
                int DataSize = 1 + ( buffer[ 0 ] * 16 );
                if( DataSize > 1 )
                {
//                    printf( "Buffer[ 0 ] = %i %i\n", buffer[ 0 ], DataSize );
//                    printf( "Metadata: %s\n", buffer + 1 );
                    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_UPDATERADIOTRACK );
                    //event.SetEventObject( ( wxObject * ) this );
                    char * evtbf = ( char * ) malloc( DataSize );
                    memcpy( evtbf, buffer + 1, DataSize - 1 );
                    event.SetClientData( evtbf );
                    wxPostEvent( PlayerPanel, event );
//                   guLogMessage( wxT( "Sent RadioUpdateTrack event" ) );

                }
                return ReadLen( socket, buffer, DataSize );
            }
        }
        // Check if timedout
        if( wxDateTime::UNow() > EndTime )
        {
            guLogError( wxT( "RadioProxy::ReadMetadata timeout" ) );
            return -1;
        }
        Sleep( GURADIOPROXY_SLEEPTIME );
    }
    return -1;
}

// -------------------------------------------------------------------------------- //
int guRadioProxyClient::WriteMetadata( wxSocketBase * socket, char * buffer )
{
    return WriteLen( socket, buffer, 1 + ( buffer[ 0 ] * 16 ) );
}

//// -------------------------------------------------------------------------------- //
//void ShowHeaders( const wxArrayString &Headers )
//{
//    int index;
//    int count = Headers.Count();
//    for( index = 0; index < count; index++ )
//        guLogMessage( wxT( "Header: %s" ), Headers[ index ].c_str() );
//}


// -------------------------------------------------------------------------------- //
int guRadioProxyClient::ReadLen( wxSocketBase * socket, char * buffer, int count )
{
    int ReadPos = 0;
    int ReadCnt;
    wxDateTime EndTime = wxDateTime::UNow() + wxTimeSpan::Seconds( GURADIOPROXY_TIMEOUT );
    while( !TestDestroy() )
    {
        if( socket->WaitForRead( 0 ) )
        {
            socket->Read( buffer + ReadPos, count - ReadPos );
            ReadCnt = socket->LastCount();
            if( !ReadCnt && socket->Error() )
            {
                if( socket->LastError() != wxSOCKET_WOULDBLOCK )
                {
                    return -1;
                }
            }
            if( ReadCnt )
            {
                ReadPos += ReadCnt;
                if( ReadPos == count )
                    break;
            }
        }
        // Check if timedout
        if( wxDateTime::UNow() > EndTime )
        {
            guLogError( wxT( "RadioProxy::ReadLen timeout" ) );
            return -1;
        }
        Sleep( GURADIOPROXY_SLEEPTIME );
    }
    return ReadPos;
}

// -------------------------------------------------------------------------------- //
int guRadioProxyClient::WriteLen( wxSocketBase * socket, char * buffer, int count )
{
    int WritePos = 0;
    int WriteCnt;
    wxDateTime EndTime = wxDateTime::UNow() + wxTimeSpan::Seconds( GURADIOPROXY_TIMEOUT );
    while( !TestDestroy() )
    {
        if( socket->WaitForWrite( 0 ) )
        {
            socket->Write( buffer + WritePos, count - WritePos );
            WriteCnt = socket->LastCount();
            if( !WriteCnt && socket->Error() )
            {
                if( socket->LastError() != wxSOCKET_WOULDBLOCK )
                {
                    return -1;
                }
            }
            if( WriteCnt )
            {
                WritePos += WriteCnt;
                if( WritePos == count )
                    break;
            }
        }
        // Check if timedout
        if( wxDateTime::UNow() > EndTime )
        {
            guLogError( wxT( "RadioProxy::WriteLen timeout" ) );
            return -1;
        }
        Sleep( GURADIOPROXY_SLEEPTIME );
        //guLogMessage( wxT( "=" ) );
    }
    return WritePos;
}

// -------------------------------------------------------------------------------- //
guRadioProxyClient::ExitCode guRadioProxyClient::Entry()
{
    wxIPV4address   addr;
    char *          Buffer;
    char *          MetaData;
    wxArrayString   ReqHeaders;
    wxArrayString   ResHeaders;
    long            BlockSize;
    int             Result;

    SocketClient = new wxSocketClient();
    if( SocketClient )
    {
        //guLogMessage( wxT( "Created the SocketClient object" ) );

        SocketClient->SetTimeout( 30 );
        SocketClient->SetFlags( wxSOCKET_NOWAIT );

        addr.Hostname( RemoteServer.GetServer() );
        addr.Service( RemoteServer.GetPort() );
        SocketClient->Connect( addr, wxSOCKET_NOWAIT );
        if( SocketClient->WaitOnConnect() && SocketClient->IsConnected() )
        {
            //guLogMessage( wxT( "Connected to the remove server" ) );

            //guLogMessage( wxT( "Reading headers from the player" ) );
            ReqHeaders = ReadHeaders( SocketPlayer );
            if( ReqHeaders.Count() )
            {
                //guLogMessage( wxT( "Read request from player" ) );

                SetHeader( ReqHeaders, wxT( "Host:" ), RemoteServer.GetServer() );

                WriteHeaders( SocketClient, ReqHeaders );

                ResHeaders = ReadHeaders( SocketClient );

                if( ResHeaders.Count() )
                {
                    WriteHeaders( SocketPlayer, ResHeaders );

                    GetHeader( ResHeaders, wxT( "icy-metaint:" ) ).ToLong( &BlockSize );
                    //guLogMessage( wxT( "BlockSize : %i" ), BlockSize ),

                    Buffer = ( char * ) malloc( BlockSize );
                    MetaData = ( char * ) malloc( GURADIOPROXY_METADATABUFSIZE );
                    //
                    while( !TestDestroy() )
                    {
                        // Read Data from the server
                        Result = ReadLen( SocketClient, Buffer, BlockSize );
                        if( TestDestroy() || Result < 0 || ( SocketClient->Error() && ( SocketClient->LastError() != wxSOCKET_WOULDBLOCK ) ) )
                        {
                            //guLogMessage( wxT( "RadioProxy:ReadData %i" ), SocketClient->LastError() );
                            break;
                        }

                        // Write the data to the Player
                        Result = WriteLen( SocketPlayer, Buffer, BlockSize );
                        if( TestDestroy() || Result < 0 || ( SocketClient->Error() && ( SocketClient->LastError() != wxSOCKET_WOULDBLOCK ) ) )
                        {
                            //guLogMessage( wxT( "RadioProxy:WriteData %i" ), SocketPlayer->LastError() );
                            break;
                        }

                        Result = ReadMetadata( SocketClient, MetaData );
                        if( TestDestroy() || Result < 0 || ( SocketClient->Error() && ( SocketClient->LastError() != wxSOCKET_WOULDBLOCK ) ) )
                        {
                            //guLogMessage( wxT( "RadioProxy:ReadMetaData %i" ), SocketClient->LastError() );
                            break;
                        }

                        Result = WriteMetadata( SocketPlayer, MetaData );
                        if( TestDestroy() || Result < 0 || ( SocketClient->Error() && ( SocketClient->LastError() != wxSOCKET_WOULDBLOCK ) ) )
                        {
                            //guLogMessage( wxT( "RadioProxy:WriteMetaData %i" ), SocketPlayer->LastError() );
                            break;
                        }
                        Sleep( GURADIOPROXY_SLEEPTIME );
                    }
                    //
                    free( MetaData );
                    free( Buffer );
                }
            }
            else
            {
                guLogError( wxT( "Could not read request from the player" ) );
            }
        }
        else
        {
            guLogError( wxT( "Could not connect to the remote server" ) );
        }
    }
    else
    {
        guLogMessage( wxT( "Could not create the Socket Client" ) );
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
