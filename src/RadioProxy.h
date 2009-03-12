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
#ifndef RADIOPROXY_H
#define RADIOPROXY_H

#include "PlayerPanel.h"

#include <wx/socket.h>
#include <wx/thread.h>
#include <wx/uri.h>

class guRadioProxyClient;

// -------------------------------------------------------------------------------- //
class guRadioProxyServer : public wxThread
{
  protected :
    guPlayerPanel *         PlayerPanel;    // Owner of this thread
    wxSocketServer *        SocketServer;   // Accepts conenctions from the client.
    wxURI                   RemoteServer;
    guRadioProxyClient *    RadioProxyClient;

  public :
    guRadioProxyServer( guPlayerPanel * playerpanel );
    ~guRadioProxyServer();
    void SetRemoteAddr( const wxString &address );
    bool ClientRunning( void );
    void StopClient( void );

    virtual ExitCode Entry();

    friend class guRadioProxyClient;
};

// -------------------------------------------------------------------------------- //
class guRadioProxyClient : public wxThread
{
  protected :
    guPlayerPanel *         PlayerPanel;
    guRadioProxyServer *    RadioProxyServer;
    wxSocketBase *          SocketPlayer;
    wxSocketClient *        SocketClient;   // Connects to the server
    wxURI                   RemoteServer;

    wxString            ReadLine( wxSocketBase * socket );
    wxArrayString       ReadHeaders( wxSocketBase * socket );
    int                 WriteHeaders( wxSocketBase * socket, const wxArrayString &headers );
    wxString            GetHeader( const wxArrayString &headers, const wxString &header );
    void                SetHeader( const wxArrayString &headers, const wxString &header, const wxString &value );
    int                 ReadMetadata( wxSocketBase * socket, char * buffer );
    int                 WriteMetadata( wxSocketBase * socket, char * buffer );

    int                 ReadLen( wxSocketBase * socket, char * buffer, int count );
    int                 WriteLen( wxSocketBase * socket, char * buffer, int count );

  public :
    guRadioProxyClient( guRadioProxyServer * radioproxyserver, guPlayerPanel * playerpanel, wxSocketBase * socketplayer, const wxURI &remoteserver );
    ~guRadioProxyClient();

    virtual ExitCode Entry();

};

#endif
// -------------------------------------------------------------------------------- //
