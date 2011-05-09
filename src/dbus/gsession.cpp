// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2011 J.Rios
//	anonbeat@gmail.com
//
//    This Program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 3, or (at your option)
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
#include "gsession.h"

#include "Utils.h"

// -------------------------------------------------------------------------------- //
guGSession::guGSession( guDBusServer * server ) : guDBusClient( server )
{
    RegisterClient();

    m_ObjectPath = wxEmptyString;
    m_Status = guGSESSION_STATUS_REGISTER_CLIENT;

    // Support for the MultimediaKeys
    guDBusMethodCall * Msg = new guDBusMethodCall( "org.gnome.SessionManager",
            "/org/gnome/SessionManager",
			"org.gnome.SessionManager",
			"RegisterClient" );

    const char AppId[] = "guayadeque-music-player";
    const char * pAppId = AppId;

    wxGetEnv( wxT( "DESKTOP_AUTOSTART_ID" ), &m_ObjectPath );
    char AutoStartId[ 1000 ];
    strcpy( AutoStartId, m_ObjectPath.char_str() );
    const char * pAutoStartId = AutoStartId;

    if( !m_ObjectPath.IsEmpty() )
        guLogMessage( wxT( "$DESKTOP_AUTOSTART_ID=%s" ), m_ObjectPath.c_str() );

    dbus_message_append_args( Msg->GetMessage(),
                              DBUS_TYPE_STRING, &pAppId,
                              DBUS_TYPE_STRING, &pAutoStartId,
                              DBUS_TYPE_INVALID );

    SendWithReply( Msg );

    delete Msg;

}

// -------------------------------------------------------------------------------- //
guGSession::~guGSession()
{
    guDBusMethodCall * Msg = new guDBusMethodCall( "org.gnome.SessionManager",
			"/org/gnome/SessionManager",
			"org.gnome.SessionManager",
			"UnregisterClient" );

    const char * pClient = strdup( m_ObjectPath.mb_str() );

    dbus_message_append_args( Msg->GetMessage(),
        DBUS_TYPE_OBJECT_PATH, &pClient,
        DBUS_TYPE_INVALID );

    SendWithReply( Msg );

    delete pClient;
    delete Msg;
    //guLogMessage( wxT( "Unregistering GSession '%s'" ), m_ObjectPath.c_str() );
}

// -------------------------------------------------------------------------------- //
DBusHandlerResult guGSession::HandleMessages( guDBusMessage * msg, guDBusMessage * reply )
{
    wxASSERT( msg );

    DBusHandlerResult RetVal = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
//    const char *    Interface = msg->GetInterface();
    const char *    Member = msg->GetMember();
    int             Type = msg->GetType();
    const char *    Path = msg->GetPath();
//    int             Serial = msg->GetSerial();
//    int             RSerial = msg->GetReplySerial();

//    // Show the details of the msg
//    guLogMessage( wxT( "==GSession========================" ) );
//    guLogMessage( wxT( "Type   : %i" ), Type );
//    guLogMessage( wxT( "Iface  : %s" ), wxString::FromAscii( Interface ).c_str() );
//    guLogMessage( wxT( "Path   : %s" ), wxString::FromAscii( Path ).c_str() );
//    guLogMessage( wxT( "OPath  : %s" ), wxString::FromAscii( msg->GetObjectPath() ).c_str() );
//    guLogMessage( wxT( "Member : %s" ), wxString::FromAscii( Member ).c_str() );
//    guLogMessage( wxT( "Serial : %i" ), Serial );
//    guLogMessage( wxT( "RSerial: %i" ), RSerial );

    if( Type == DBUS_MESSAGE_TYPE_METHOD_RETURN )
    {
        if( m_Status == guGSESSION_STATUS_REGISTER_CLIENT )
        {
            m_ObjectPath = wxString::FromAscii( msg->GetObjectPath() );
            if( m_ObjectPath.IsEmpty() )
            {
                guLogError( wxT( "Could not get the GSession Object Path" ) );
                m_Status = guGSESSION_STATUS_ERROR;
            }
            else
            {
                //guLogMessage( wxT( "Got the GSession Client Id : '%s'" ), m_ObjectPath.c_str() );
//                AddMatch( wxString::Format( wxT( "type='signal',"
//	                      "interface='org.gnome.SessionManager.ClientPrivate',"
//	                      "path='%s'" ), m_ObjectPath.c_str() ).char_str() );
                m_Status = guGSESSION_STATUS_INITIALIZED;
            }
            RetVal = DBUS_HANDLER_RESULT_HANDLED;
        }
    }
    else if( Type == DBUS_MESSAGE_TYPE_SIGNAL )
    {
        if( !strcmp( Path, m_ObjectPath.ToAscii() ) )
        {
            //guLogMessage( wxT( "Received GSession Client Signal" ) );
            if( !strcmp( Member, "Stop" ) )
            {
                RetVal = DBUS_HANDLER_RESULT_HANDLED;
            }
            else if( !strcmp( Member, "QueryEndSession" ) )
            {
                m_Status = guGSESSION_STATUS_QUERY_END_SESSION;

                guDBusMethodCall * MsgCall = new guDBusMethodCall( "org.gnome.SessionManager",
                    m_ObjectPath.ToAscii(),
                    NULL,
                    "EndSessionResponse" );

                bool IsOk = true;

                const char * Reason = "No reason given";

                dbus_message_append_args( MsgCall->GetMessage(),
                    DBUS_TYPE_BOOLEAN, &IsOk,
                    DBUS_TYPE_STRING, &Reason,
                    DBUS_TYPE_INVALID );

                SendWithReply( MsgCall );

                delete MsgCall;
                RetVal = DBUS_HANDLER_RESULT_HANDLED;
            }
            else if( !strcmp( Member, "EndSession" ) )
            {
                m_Status = guGSESSION_STATUS_END_SESSION;

                guDBusMethodCall * MsgCall = new guDBusMethodCall( "org.gnome.SessionManager",
                    m_ObjectPath.ToAscii(),
                    NULL,
                    "EndSessionResponse" );

                bool IsOk = true;

                const char * Reason = "";

                dbus_message_append_args( MsgCall->GetMessage(),
                    DBUS_TYPE_BOOLEAN, &IsOk,
                    DBUS_TYPE_STRING, &Reason,
                    DBUS_TYPE_INVALID );

                SendWithReply( MsgCall );

                delete MsgCall;
                RetVal = DBUS_HANDLER_RESULT_HANDLED;

                // Closes the App
                wxTheApp->GetTopWindow()->Close( true );
            }
            else if( !strcmp( Member, "CancelEndSession" ) )
            {
                RetVal = DBUS_HANDLER_RESULT_HANDLED;
            }
        }
    }

    return RetVal;
}

// -------------------------------------------------------------------------------- //
