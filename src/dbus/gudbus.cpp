// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2016 J.Rios anonbeat@gmail.com
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
//    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#include "gudbus.h"
#include "Utils.h"

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
DBusHandlerResult Handle_Messages( DBusConnection * conn, DBusMessage * msg, void * udata )
{
	guDBusServer * DBusObj = ( guDBusServer * ) udata;
	//guDBusMessage * Msg = new guDBusMessage( msg );
	guDBusMessage * Msg = new guDBusMessage( msg );
	guDBusMethodReturn * Reply = NULL;
	if( Msg->NeedReply() )
	{
	    Reply = new guDBusMethodReturn( msg );
	}
	DBusHandlerResult RetVal = DBusObj->HandleMessages( Msg, Reply );
	//printf( "*** End of handle_messages ***\n" );

	delete Msg;

	if( Reply )
        delete Reply;

 	return RetVal;
}

// -------------------------------------------------------------------------------- //
void Handle_Response( DBusPendingCall * PCall, void * udata )
{
	guDBusClient * DBusObj = ( guDBusClient * ) udata;
	DBusMessage * reply;
	reply = dbus_pending_call_steal_reply( PCall );
    guDBusMessage * Msg = new guDBusMessage( reply );

    DBusObj->HandleMessages( Msg, NULL );

    delete Msg;
	dbus_message_unref( reply );
	dbus_pending_call_unref( PCall );
}


// -------------------------------------------------------------------------------- //
guDBusServer::guDBusServer( const char * name, bool System )
{
    dbus_error_init( &m_DBusErr );
    m_DBusThread = NULL;

    m_DBusConn = dbus_bus_get( System ? DBUS_BUS_SYSTEM : DBUS_BUS_SESSION, &m_DBusErr );

    if( dbus_error_is_set( &m_DBusErr ) )
    {
        guLogError( wxT( "Get DBus object failed : %s" ), m_DBusErr.message );
        dbus_error_free( &m_DBusErr );
    }

    if( m_DBusConn )
    {
        if( name )
        {
            RequestName( name );
        }

        if( dbus_connection_add_filter( m_DBusConn, Handle_Messages, ( void * ) this, NULL ) )
        {
            m_DBusThread = new guDBusThread( this );
        }
    }
}

// -------------------------------------------------------------------------------- //
guDBusServer::~guDBusServer()
{
    if( m_Clients.Count() )
    {
        while( m_Clients.Count() )
        {
            guDBusClient * Client = m_Clients.Last();
            delete Client;
        }
    }

    if( m_DBusThread )
    {
        m_DBusThread->Pause();
        m_DBusThread->Delete();
    }

    if( m_DBusConn )
    {
        //dbus_connection_close( m_DBusConn );
        dbus_connection_unref( m_DBusConn );
    }
}

// -------------------------------------------------------------------------------- //
DBusConnection * guDBusServer::GetConnection()
{
    return m_DBusConn;
}

// -------------------------------------------------------------------------------- //
bool guDBusServer::HasOwner( const char * name )
{
    return dbus_bus_name_has_owner( m_DBusConn, name, NULL );
}

// -------------------------------------------------------------------------------- //
bool guDBusServer::RequestName( const char * name )
{
    wxASSERT( name );

    dbus_bus_request_name( m_DBusConn, name, DBUS_NAME_FLAG_REPLACE_EXISTING, &m_DBusErr );

    if( dbus_error_is_set( &m_DBusErr ) )
    {
        guLogError( wxT( "Request name '%s' failed : %s" ), name, m_DBusErr.message );
        dbus_error_free( &m_DBusErr );
        return false;
    }
    return true;
}

// -------------------------------------------------------------------------------- //
DBusHandlerResult guDBusServer::HandleMessages( guDBusMessage * msg, guDBusMessage * reply )
{
    wxASSERT( msg );

    DBusHandlerResult RetVal = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

    int index;
    int count = m_Clients.Count();
    for( index = 0; index < count; index++ )
    {
        m_ClientsMutex.Lock();
        if( m_Clients[ index ]->HandleMessages( msg, reply ) == DBUS_HANDLER_RESULT_HANDLED )
        {
            RetVal = DBUS_HANDLER_RESULT_HANDLED;
        }
        m_ClientsMutex.Unlock();

        if( RetVal == DBUS_HANDLER_RESULT_HANDLED )
            break;
    }

//    delete msg;
//
//    if( reply )
//        delete reply;

    return RetVal;
}

// -------------------------------------------------------------------------------- //
bool guDBusServer::RegisterObjectPath( const char * objname )
{
    DBusObjectPathVTable VTable = { NULL, Handle_Messages, NULL, NULL, NULL, NULL };

    return dbus_connection_register_object_path( m_DBusConn, objname, &VTable, this );
}

// -------------------------------------------------------------------------------- //
bool guDBusServer::UnRegisterObjectPath( const char * objname )
{
    return dbus_connection_unregister_object_path( m_DBusConn, objname );
}

// -------------------------------------------------------------------------------- //
bool guDBusServer::AddMatch( const char * rule )
{
    DBusError error;

    dbus_error_init( &error );

    dbus_bus_add_match( m_DBusConn, rule, &error );

    if( dbus_error_is_set( &error ) )
    {
        guLogError( wxT( "Could not add the match %s : %s" ),
            wxString::FromAscii( rule ).c_str(),
            wxString::FromAscii( error.message ).c_str() );
        dbus_error_free( &error );
        return false;
    }
    return true;
}

// -------------------------------------------------------------------------------- //
bool guDBusServer::Send( guDBusMessage * msg )
{
    return msg && dbus_connection_send( m_DBusConn, msg->GetMessage(), NULL );
}

// -------------------------------------------------------------------------------- //
bool guDBusServer::SendWithReply( guDBusMessage * msg, guDBusClient * client, int timeout )
{
    DBusPendingCall * PCall = NULL;
    bool RetVal = dbus_connection_send_with_reply( m_DBusConn, msg->GetMessage(), &PCall, timeout );
	dbus_pending_call_set_notify( PCall, Handle_Response, (void *) client, NULL );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
guDBusMessage * guDBusServer::SendWithReplyAndBlock( guDBusMessage * msg, int timeout )
{
    DBusError error;
    dbus_error_init( &error );
    guDBusMessage * Reply;

    DBusMessage * Result = dbus_connection_send_with_reply_and_block( m_DBusConn, msg->GetMessage(), timeout, &error );
    Reply =  Result ? new guDBusMessage( Result ) : NULL;
//    if( Result )
//        dbus_message_unref( Result );
    return Reply;
}

// -------------------------------------------------------------------------------- //
void guDBusServer::Flush()
{
    dbus_connection_flush( m_DBusConn );
}

// -------------------------------------------------------------------------------- //
bool guDBusServer::RegisterClient( guDBusClient * client )
{
    wxMutexLocker Locker( m_ClientsMutex );
    if( m_Clients.Index( client ) == wxNOT_FOUND )
    {
        m_Clients.Add( client );
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guDBusServer::UnRegisterClient( guDBusClient * client )
{
    wxMutexLocker Locker( m_ClientsMutex );
    if( m_Clients.Index( client ) != wxNOT_FOUND )
    {
        m_Clients.Remove( client );
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
void guDBusServer::Run()
{
    m_DBusThread->Run();
}

// -------------------------------------------------------------------------------- //
// guDBusClient
// -------------------------------------------------------------------------------- //
guDBusClient::guDBusClient( guDBusServer * server )
{
    wxASSERT( server );
    m_DBusServer = server;
}

// -------------------------------------------------------------------------------- //
guDBusClient::~guDBusClient()
{
    UnRegisterClient();
}

// -------------------------------------------------------------------------------- //
bool guDBusClient::HasOwner( const char * name )
{
    return m_DBusServer->HasOwner( name );
}

// -------------------------------------------------------------------------------- //
bool guDBusClient::RegisterClient( void )
{
    return m_DBusServer->RegisterClient( this );
}

// -------------------------------------------------------------------------------- //
bool guDBusClient::UnRegisterClient( void )
{
    return m_DBusServer->UnRegisterClient( this );
}

// -------------------------------------------------------------------------------- //
DBusHandlerResult guDBusClient::HandleMessages( guDBusMessage * msg, guDBusMessage * reply )
{
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

// -------------------------------------------------------------------------------- //
bool guDBusClient::RequestName( const char * name )
{
    return m_DBusServer->RequestName( name );
}

// -------------------------------------------------------------------------------- //
DBusConnection * guDBusClient::GetConnection()
{
    return m_DBusServer->GetConnection();
}

// -------------------------------------------------------------------------------- //
bool guDBusClient::RegisterObjectPath( const char * objname )
{
    return m_DBusServer->RegisterObjectPath( objname );
}

// -------------------------------------------------------------------------------- //
bool guDBusClient::UnRegisterObjectPath( const char * objname )
{
    return m_DBusServer->UnRegisterObjectPath( objname );
}

// -------------------------------------------------------------------------------- //
bool guDBusClient::AddMatch( const char * rule )
{
    return m_DBusServer->AddMatch( rule );
}

// -------------------------------------------------------------------------------- //
bool guDBusClient::Send( guDBusMessage * msg )
{
    return m_DBusServer->Send( msg );
}

// -------------------------------------------------------------------------------- //
bool guDBusClient::SendWithReply( guDBusMessage * msg, int timeout )
{
    return m_DBusServer->SendWithReply( msg, this, timeout );
}

// -------------------------------------------------------------------------------- //
guDBusMessage * guDBusClient::SendWithReplyAndBlock( guDBusMessage * msg, int timeout )
{
    return m_DBusServer->SendWithReplyAndBlock( msg, timeout );
}

// -------------------------------------------------------------------------------- //
void guDBusClient::Flush()
{
    m_DBusServer->Flush();
}

// -------------------------------------------------------------------------------- //
// guDBusMessage
// -------------------------------------------------------------------------------- //
guDBusMessage::guDBusMessage( int type )
{
    // types can be :
    //      DBUS_MESSAGE_TYPE_INVALID
    //      DBUS_MESSAGE_TYPE_METHOD_CALL
    //      DBUS_MESSAGE_TYPE_METHOD_RETURN
    //      DBUS_MESSAGE_TYPE_ERROR
    //      DBUS_MESSAGE_TYPE_SIGNAL
    m_DBusMsg = dbus_message_new( type );
}

// -------------------------------------------------------------------------------- //
guDBusMessage::guDBusMessage( guDBusMessage * msg )
{
    //m_DBusMsg = dbus_message_copy( msg->GetMessage() );
    m_DBusMsg = msg->GetMessage();
    dbus_message_ref( m_DBusMsg );
}

// -------------------------------------------------------------------------------- //
guDBusMessage::guDBusMessage( DBusMessage * msg )
{
    //m_DBusMsg = dbus_message_copy( msg );
    m_DBusMsg = msg;
    dbus_message_ref( msg );
}

// -------------------------------------------------------------------------------- //
guDBusMessage::~guDBusMessage()
{
    if( m_DBusMsg )
    {
        dbus_message_unref( m_DBusMsg );
    }
}

// -------------------------------------------------------------------------------- //
DBusMessage * guDBusMessage::GetMessage()
{
    return m_DBusMsg;
}

// -------------------------------------------------------------------------------- //
const char * guDBusMessage::GetErrorName()
{
    return dbus_message_get_error_name( m_DBusMsg );
}

// -------------------------------------------------------------------------------- //
const char * guDBusMessage::GetInterface()
{
    return dbus_message_get_interface( m_DBusMsg );
}

// -------------------------------------------------------------------------------- //
const char * guDBusMessage::GetMember()
{
    return dbus_message_get_member( m_DBusMsg );
}

// -------------------------------------------------------------------------------- //
bool guDBusMessage::NeedReply()
{
    return !dbus_message_get_no_reply( m_DBusMsg );
}

// -------------------------------------------------------------------------------- //
void guDBusMessage::SetNoReply( const bool needreply )
{
    dbus_message_set_no_reply( m_DBusMsg, needreply );
}

// -------------------------------------------------------------------------------- //
const char * guDBusMessage::GetPath()
{
    return dbus_message_get_path( m_DBusMsg );
}

// -------------------------------------------------------------------------------- //
unsigned int guDBusMessage::GetReplySerial()
{
    return dbus_message_get_reply_serial( m_DBusMsg );
}

// -------------------------------------------------------------------------------- //
const char * guDBusMessage::GetSender()
{
    return dbus_message_get_sender( m_DBusMsg );
}

// -------------------------------------------------------------------------------- //
const char * guDBusMessage::GetDestination()
{
    return dbus_message_get_destination( m_DBusMsg );
}

// -------------------------------------------------------------------------------- //
unsigned int guDBusMessage::GetSerial()
{
    return dbus_message_get_serial( m_DBusMsg );
}

// -------------------------------------------------------------------------------- //
int guDBusMessage::GetType()
{
    return dbus_message_get_type( m_DBusMsg );
}

// -------------------------------------------------------------------------------- //
bool guDBusMessage::HasDestination( const char * dest )
{
    return dbus_message_has_destination( m_DBusMsg, dest );
}

// -------------------------------------------------------------------------------- //
bool guDBusMessage::HasInterface( const char * iface )
{
    return dbus_message_has_interface( m_DBusMsg, iface );
}

// -------------------------------------------------------------------------------- //
bool guDBusMessage::HasMember( const char * member )
{
    return dbus_message_has_member( m_DBusMsg, member );
}

// -------------------------------------------------------------------------------- //
bool guDBusMessage::HasPath( const char * path )
{
    return dbus_message_has_path( m_DBusMsg, path );
}

// -------------------------------------------------------------------------------- //
bool guDBusMessage::HasSender( const char * sender )
{
    return dbus_message_has_sender( m_DBusMsg, sender );
}

// -------------------------------------------------------------------------------- //
bool guDBusMessage::IsError( const char * errname )
{
    return dbus_message_is_error( m_DBusMsg, errname );
}

// -------------------------------------------------------------------------------- //
bool guDBusMessage::IsMethodCall( const char * iface, const char * method )
{
    return dbus_message_is_method_call( m_DBusMsg, iface, method );
}

// -------------------------------------------------------------------------------- //
bool guDBusMessage::IsSignal( const char * iface, const char * signal_name )
{
    return dbus_message_is_signal( m_DBusMsg, iface, signal_name );
}

// -------------------------------------------------------------------------------- //
const char * guDBusMessage::GetObjectPath()
{
    const char * RetVal;
    if( !dbus_message_get_args( m_DBusMsg, NULL, DBUS_TYPE_OBJECT_PATH, &RetVal, DBUS_TYPE_INVALID ) )
        return NULL;
    return RetVal;
}

// -------------------------------------------------------------------------------- //
// guDBusMethodCall
// -------------------------------------------------------------------------------- //
guDBusMethodCall::guDBusMethodCall( const char * dest, const char * path, const char *iface, const char * method )
{
    m_DBusMsg = dbus_message_new_method_call( dest, path, iface, method );
}

// -------------------------------------------------------------------------------- //
// guDBusMethodReturn
// -------------------------------------------------------------------------------- //
guDBusMethodReturn::guDBusMethodReturn( guDBusMessage * msg )
{
    m_DBusMsg = dbus_message_new_method_return( msg->GetMessage() );
}

// -------------------------------------------------------------------------------- //
guDBusMethodReturn::guDBusMethodReturn( DBusMessage * msg )
{
    m_DBusMsg = dbus_message_new_method_return( msg );
}

// -------------------------------------------------------------------------------- //
guDBusSignal::guDBusSignal( const char * path, const char * iface, const char * name )
{
    m_DBusMsg = dbus_message_new_signal( path, iface, name );
}

// -------------------------------------------------------------------------------- //
// guDBusThread
// -------------------------------------------------------------------------------- //
guDBusThread::guDBusThread( guDBusServer * dbusowner ) : wxThread( wxTHREAD_DETACHED )
{
    m_DBusOwner = dbusowner;

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY );
        //Run();
    }
}

// -------------------------------------------------------------------------------- //
guDBusThread::~guDBusThread()
{
}

// -------------------------------------------------------------------------------- //
guDBusThread::ExitCode guDBusThread::Entry()
{
    while( !TestDestroy() )
    {
        dbus_connection_read_write_dispatch( m_DBusOwner->GetConnection(), 0 );
        Sleep( guDBUS_THREAD_IDLE_TIMEOUT );
    }
    return 0;
}

}

// -------------------------------------------------------------------------------- //
