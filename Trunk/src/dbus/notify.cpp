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
#include "notify.h"

#include "Utils.h"

// -------------------------------------------------------------------------------- //
guDBusNotify::guDBusNotify( guDBusServer * server ) : guDBusClient( server )
{
    m_MsgId = 0;

    RegisterClient();
}

// -------------------------------------------------------------------------------- //
guDBusNotify::~guDBusNotify()
{
    if( m_MsgId )
    {
        //guLogMessage( wxT( "Sending Close Message... %i" ), m_MsgId );
//        guDBusMethodCall * Msg = new guDBusMethodCall( "org.freedesktop.Notifications",
//                                               "/org/freedesktop/Notifications",
//                                               "org.freedesktop.Notifications",
//                                               "CloseNotification" );
//        dbus_message_append_args( Msg->GetMessage(), DBUS_TYPE_UINT32, &m_MsgId, DBUS_TYPE_INVALID );
//        Msg->SetNoReply( true );
//        Send( Msg );
//
//        delete Msg;
    }
}

// -------------------------------------------------------------------------------- //
// Its done this way to avoid the warning of temporary address
void inline Append_String( DBusMessageIter * iter, const char * str )
{
    dbus_message_iter_append_basic( iter, DBUS_TYPE_STRING, &str );
}

// -------------------------------------------------------------------------------- //
void guDBusNotify::Notify( const wxString &icon, const wxString &summary,
                            const wxString &body, wxImage * image, bool newnotify )
{
    guDBusMethodCall * Msg = new guDBusMethodCall( "org.freedesktop.Notifications",
                                               "/org/freedesktop/Notifications",
                                               "org.freedesktop.Notifications",
                                               "Notify");

    const char * msg_app = "guayadeque-music-player";
    int  msg_timeout = -1;           // Default timeout
    int  msg_newid = 0;

    DBusMessageIter iter;
    DBusMessageIter array;
    DBusMessageIter dict;
    DBusMessageIter value;
    DBusMessageIter variant;
    DBusMessageIter data;

    dbus_message_iter_init_append( Msg->GetMessage(), &iter );
    dbus_message_iter_append_basic( &iter, DBUS_TYPE_STRING, &msg_app );
    dbus_message_iter_append_basic( &iter, DBUS_TYPE_UINT32, newnotify ? &msg_newid : &m_MsgId );
    //Append_String( &iter, icon.char_str() );
    //Append_String( &iter, summary.char_str() );
    //Append_String( &iter, body.char_str() );
    Append_String( &iter, icon.mb_str( wxConvUTF8 ) );
    Append_String( &iter, summary.mb_str( wxConvUTF8 ) );
    Append_String( &iter, body.mb_str( wxConvUTF8 ) );
    dbus_message_iter_open_container( &iter, DBUS_TYPE_ARRAY, DBUS_TYPE_STRING_AS_STRING, &array );
    dbus_message_iter_close_container( &iter, &array );
    dbus_message_iter_open_container( &iter, DBUS_TYPE_ARRAY, "{sv}", &array );

    if( image )
    {
        int Width;
        int Height;
        int Size;
        int BitsPerSample;
        int Channels;
        int BufferSize;
        dbus_bool_t HasAlpha;
        char * ImageData;

        const char * icon_data = "icon_data";

        HasAlpha        = image->HasAlpha();
        Width           = image->GetWidth();
        Height          = image->GetHeight();
        Size            = Width * 4;
        BitsPerSample   = 8;
        Channels        = 4;
        BufferSize      = Width * Height * 4;
        //guLogMessage( wxT( "Has Alpha: %i (%i-%i)" ), HasAlpha, image->GetWidth(), image->GetHeight() );

        ImageData = ( char * ) malloc( BufferSize );
        unsigned char * adata = NULL;
        if( HasAlpha )
            adata = image->GetAlpha();
        unsigned char * pdata = image->GetData();
        int Index = 0;
        int DataCnt = 0;
        int AlphaCnt = 0;
        while( Index < BufferSize )
        {
            ImageData[ Index++ ] = pdata[ DataCnt++ ];
            ImageData[ Index++ ] = pdata[ DataCnt++ ];
            ImageData[ Index++ ] = pdata[ DataCnt++ ];
            ImageData[ Index++ ] = HasAlpha ? adata[ AlphaCnt++ ] : 255;
        }

        HasAlpha = true;
        dbus_message_iter_open_container( &array, DBUS_TYPE_DICT_ENTRY, 0, &dict );

        dbus_message_iter_append_basic( &dict, DBUS_TYPE_STRING, &icon_data );
        dbus_message_iter_open_container( &dict, DBUS_TYPE_VARIANT, "(iiibiiay)", &variant );
        dbus_message_iter_open_container( &variant, DBUS_TYPE_STRUCT, NULL, &value );
        dbus_message_iter_append_basic( &value, DBUS_TYPE_INT32, &Width );
        dbus_message_iter_append_basic( &value, DBUS_TYPE_INT32, &Height );
        dbus_message_iter_append_basic( &value, DBUS_TYPE_INT32, &Size );
        dbus_message_iter_append_basic( &value, DBUS_TYPE_BOOLEAN, &HasAlpha );
        dbus_message_iter_append_basic( &value, DBUS_TYPE_INT32, &BitsPerSample );
        dbus_message_iter_append_basic( &value, DBUS_TYPE_INT32, &Channels );
        dbus_message_iter_open_container( &value, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE_AS_STRING, &data );
        dbus_message_iter_append_fixed_array( &data, DBUS_TYPE_BYTE, &ImageData, BufferSize );
        dbus_message_iter_close_container( &value, &data );
        dbus_message_iter_close_container( &variant,&value );
        dbus_message_iter_close_container( &dict, &variant );
        dbus_message_iter_close_container( &array, &dict );

        free( ImageData );
    }

    dbus_message_iter_close_container( &iter, &array );
    dbus_message_iter_append_basic( &iter, DBUS_TYPE_INT32, &msg_timeout );

    guDBusMessage * Reply = SendWithReplyAndBlock( Msg );
    if( Reply )
    {
        dbus_message_get_args( Reply->GetMessage(), NULL, DBUS_TYPE_UINT32, &m_MsgId, DBUS_TYPE_INVALID );
        delete Reply;
    }

    delete Msg;
}

// -------------------------------------------------------------------------------- //
DBusHandlerResult guDBusNotify::HandleMessages( guDBusMessage * msg, guDBusMessage * reply )
{
    wxASSERT( msg );

    DBusHandlerResult RetVal = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
//    const char *    Interface = msg->GetInterface();
    const char *    Member = msg->GetMember();
    int             Type = msg->GetType();
//    const char *    Path = msg->GetPath();
//    int             Serial = msg->GetSerial();
//    int             RSerial = msg->GetReplySerial();

//    // Show the details of the msg
//    guLogMessage( wxT( "==DBusNotify========================" ) );
//    guLogMessage( wxT( "Type   : %i" ), Type );
//    guLogMessage( wxT( "Iface  : %s" ), wxString::FromAscii( Interface ).c_str() );
//    guLogMessage( wxT( "Path   : %s" ), wxString::FromAscii( Path ).c_str() );
//    guLogMessage( wxT( "OPath  : %s" ), wxString::FromAscii( msg->GetObjectPath() ).c_str() );
//    guLogMessage( wxT( "Member : %s" ), wxString::FromAscii( Member ).c_str() );
//    guLogMessage( wxT( "Serial : %i" ), Serial );
//    guLogMessage( wxT( "RSerial: %i" ), RSerial );

//    if( Type == DBUS_MESSAGE_TYPE_METHOD_RETURN )
//    {
//        dbus_message_get_args( msg->GetMessage(), NULL, DBUS_TYPE_UINT32, &m_MsgId, DBUS_TYPE_INVALID );
//        //guLogMessage( wxT( "The Id is: %i" ), m_MsgId );
//        RetVal = DBUS_HANDLER_RESULT_HANDLED;
//    }
//    else if( Type == DBUS_MESSAGE_TYPE_SIGNAL )
    if( Type == DBUS_MESSAGE_TYPE_SIGNAL )
    {
        if( !strcmp( Member, "NotificationClosed" ) )
        {
            int Id;
            int Reason;
            if( ( dbus_message_has_signature( msg->GetMessage(), "u" ) &&
                  dbus_message_get_args( msg->GetMessage(), NULL, DBUS_TYPE_UINT32, &Id, DBUS_TYPE_INVALID ) ) ||
                ( dbus_message_has_signature( msg->GetMessage(), "uu" ) &&
                  dbus_message_get_args( msg->GetMessage(), NULL, DBUS_TYPE_UINT32, &Id, DBUS_TYPE_UINT32, &Reason, DBUS_TYPE_INVALID ) ) )
            {
                if( Id == m_MsgId )
                {
                    m_MsgId = 0;
                }
            }
            RetVal = DBUS_HANDLER_RESULT_HANDLED;
        }
    }

    return RetVal;
}

// -------------------------------------------------------------------------------- //

