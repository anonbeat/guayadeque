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
#include "gudbus.h"
#include "Utils.h"

// -------------------------------------------------------------------------------- //
DBusHandlerResult Handle_Messages( DBusConnection * conn, DBusMessage * msg, void * udata )
{
	guDBus * DBusObj = ( guDBus * ) udata;
	guDBusMessage * Msg = new guDBusMessage( msg );
	guDBusMethodReturn * Reply = NULL;
	if( Msg->NeedReply() )
	{
	    Reply = new guDBusMethodReturn( msg );
	}
	DBusHandlerResult RetVal = DBusObj->HandleMessages( Msg, Reply );
	//printf( "*** End of handle_messages ***\n" );
 	return RetVal;
}

// -------------------------------------------------------------------------------- //
guDBus::guDBus( const char * name, bool System )
{
    dbus_error_init( &m_DBusErr );

    m_DBusConn = dbus_bus_get( System ? DBUS_BUS_SYSTEM : DBUS_BUS_SESSION, &m_DBusErr );

    if( dbus_error_is_set( &m_DBusErr ) )
    {
        guLogError( wxT( "Get DBus object failed : %s" ), m_DBusErr.message );
        dbus_error_free( &m_DBusErr );
    }

    if( name )
    {
        RequestName( name );
    }

    if( m_DBusConn && dbus_connection_add_filter( m_DBusConn, Handle_Messages, ( void * ) this, NULL ) )
    {
        m_DBusThread = new guDBusThread( this );
    }
}

// -------------------------------------------------------------------------------- //
guDBus::~guDBus()
{
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
DBusConnection * guDBus::GetConnection()
{
    return m_DBusConn;
}

// -------------------------------------------------------------------------------- //
bool guDBus::RequestName( const char * name )
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
DBusHandlerResult guDBus::HandleMessages( guDBusMessage * msg, guDBusMessage * reply )
{
    wxASSERT( msg );

    dbus_message_unref( msg->GetMessage() );

    if( reply )
        dbus_message_unref( reply->GetMessage() );

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

// -------------------------------------------------------------------------------- //
bool guDBus::RegisterObjectPath( const char * objname )
{
    DBusObjectPathVTable VTable = { NULL, Handle_Messages, NULL, NULL, NULL, NULL };

    return dbus_connection_register_object_path( m_DBusConn, objname, &VTable, this );
}

// -------------------------------------------------------------------------------- //
bool guDBus::UnRegisterObjectPath( const char * objname )
{
    return dbus_connection_unregister_object_path( m_DBusConn, objname );
}

// -------------------------------------------------------------------------------- //
bool guDBus::AddMatch( const char * rule )
{
    DBusError error;

    dbus_error_init( &error );

    dbus_bus_add_match( m_DBusConn, rule, &error );

    if( dbus_error_is_set( &error ) )
    {
        printf( "Could not add the match %s : %s\n", rule, error.message );
        dbus_error_free( &error );
        return false;
    }
    return true;
}

// -------------------------------------------------------------------------------- //
bool guDBus::Send( guDBusMessage * msg )
{
    return dbus_connection_send( m_DBusConn, msg->GetMessage(), NULL );
}

// -------------------------------------------------------------------------------- //
void guDBus::Flush()
{
    dbus_connection_flush( m_DBusConn );
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
    wxASSERT( msg );
    m_DBusMsg = dbus_message_copy( msg->GetMessage() );
}

// -------------------------------------------------------------------------------- //
guDBusMessage::guDBusMessage( DBusMessage * msg )
{
    wxASSERT( msg );
    m_DBusMsg = dbus_message_copy( msg );
    //m_DBusMsg = msg;
}

// -------------------------------------------------------------------------------- //
guDBusMessage::~guDBusMessage()
{
    if( m_DBusMsg )
        dbus_message_unref( m_DBusMsg );
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
// guDBusReturnMessage
// -------------------------------------------------------------------------------- //
guDBusMethodReturn::guDBusMethodReturn( guDBusMessage * msg )  :
  guDBusMessage( dbus_message_new_method_return( msg->GetMessage() ) )
{
}

// -------------------------------------------------------------------------------- //
guDBusMethodReturn::guDBusMethodReturn( DBusMessage * msg )  :
  guDBusMessage( dbus_message_new_method_return( msg ) )
{
}

// -------------------------------------------------------------------------------- //
guDBusSignal::guDBusSignal( const char * path, const char * iface, const char * name )
{
    m_DBusMsg = dbus_message_new_signal( path, iface, name );
}

// -------------------------------------------------------------------------------- //
// guDBusThread
// -------------------------------------------------------------------------------- //
guDBusThread::guDBusThread( guDBus * dbusowner ) : wxThread( wxTHREAD_DETACHED )
{
    m_DBusOwner = dbusowner;

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY );
        Run();
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
        Sleep( DBUS_THREAD_IDLE_TIMEOUT );
    }
}

// -------------------------------------------------------------------------------- //
