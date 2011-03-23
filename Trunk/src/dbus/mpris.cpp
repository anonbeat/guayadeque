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
#include "mpris.h"

#include "Commands.h"
#include "Utils.h"

const char * Introspection_XML_Data_Root =
"<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n"
"\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n"
"<node>\n"
"  <node name=\"Player\"/>\n"
"  <node name=\"TrackList\"/>\n"
"  <interface name=\"org.freedesktop.DBus.Introspectable\">\n"
"    <method name=\"Introspect\">\n"
"      <arg name=\"data\" direction=\"out\" type=\"s\"/>\n"
"    </method>\n"
"  </interface>\n"
"  <interface name=\"org.freedesktop.MediaPlayer\">\n"
"    <method name=\"Identity\">\n"
"      <arg type=\"s\" direction=\"out\" />\n"
"    </method>\n"
"    <method name=\"Quit\">\n"
"    </method>\n"
"    <method name=\"MprisVersion\">\n"
"      <arg type=\"(qq)\" direction=\"out\" />\n"
"    </method>\n"
"  </interface>\n"
"</node>\n"
;

const char * Introspection_XML_Data_Player =
"<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n"
"\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n"
"<node>"
"  <interface name=\"org.freedesktop.DBus.Introspectable\">\n"
"    <method name=\"Introspect\">\n"
"      <arg name=\"data\" direction=\"out\" type=\"s\"/>\n"
"    </method>\n"
"  </interface>\n"
"  <interface name=\"org.freedesktop.MediaPlayer\">\n"
"    <method name=\"Next\">\n"
"    </method>\n"
"    <method name=\"Prev\">\n"
"    </method>\n"
"    <method name=\"Pause\">\n"
"    </method>\n"
"    <method name=\"Stop\">\n"
"    </method>\n"
"    <method name=\"Play\">\n"
"    </method>\n"
"    <method name=\"Repeat\">\n"
"      <arg type=\"b\" direction=\"in\" />\n"
"    </method>\n"
"    <method name=\"GetStatus\">\n"
"      <arg type=\"(iiii)\" direction=\"out\"/>\n"
"    </method>\n"
"    <method name=\"GetMetadata\">\n"
"      <arg type=\"a{sv}\" direction=\"out\" />\n"
"    </method>\n"
"    <method name=\"GetCaps\">\n"
"      <arg type=\"i\" direction=\"out\" />\n"
"    </method>\n"
"    <method name=\"VolumeSet\">\n"
"      <arg type=\"i\" direction=\"in\" />\n"
"    </method>\n"
"    <method name=\"VolumeGet\">\n"
"      <arg type=\"i\" direction=\"out\" />\n"
"    </method>\n"
"    <method name=\"PositionSet\">\n"
"      <arg type=\"i\" direction=\"in\" />\n"
"    </method>\n"
"    <method name=\"PositionGet\">\n"
"      <arg type=\"i\" direction=\"out\" />\n"
"    </method>\n"
"    <signal name=\"TrackChange\">\n"
"      <arg type=\"a{sv}\"/>\n"
"    </signal>\n"
"    <signal name=\"StatusChange\">\n"
"      <arg type=\"(iiii)\"/>\n"
"    </signal>\n"
"    <signal name=\"CapsChange\">\n"
"      <arg type=\"i\"/>\n"
"    </signal>\n"
"  </interface>\n"
"</node>\n"
;

const char * Introspection_XML_Data_Tracklist =
"<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n"
"\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n"
"<node>"
"  <interface name=\"org.freedesktop.DBus.Introspectable\">\n"
"    <method name=\"Introspect\">\n"
"      <arg name=\"data\" direction=\"out\" type=\"s\"/>\n"
"    </method>\n"
"  </interface>\n"
"  <interface name=\"org.freedesktop.MediaPlayer\">\n"
"    <method name=\"GetMetadata\">\n"
"      <arg type=\"i\" direction=\"in\" />\n"
"      <arg type=\"a{sv}\" direction=\"out\" />\n"
"    </method>\n"
"    <method name=\"GetCurrentTrack\">\n"
"      <arg type=\"i\" direction=\"out\" />\n"
"    </method>\n"
"    <method name=\"GetLength\">\n"
"      <arg type=\"i\" direction=\"out\" />\n"
"    </method>\n"
"    <method name=\"AddTrack\">\n"
"      <arg type=\"s\" direction=\"in\" />\n"
"      <arg type=\"b\" direction=\"in\" />\n"
"      <arg type=\"i\" direction=\"out\" />\n"
"    </method>\n"
"    <method name=\"DelTrack\">\n"
"      <arg type=\"i\" direction=\"in\" />\n"
"    </method>\n"
"    <method name=\"SetLoop\">\n"
"      <arg type=\"b\" direction=\"in\" />\n"
"    </method>\n"
"    <method name=\"SetRandom\">\n"
"      <arg type=\"b\" direction=\"in\" />\n"
"    </method>\n"
"    <method name=\"AddTracks\">\n"
"      <arg type=\"as\" direction=\"in\" />\n"
"      <arg type=\"b\" direction=\"in\" />\n"
"      <arg type=\"i\" direction=\"out\" />\n"
"    </method>\n"
"    <signal name=\"TrackListChange\">\n"
"      <arg type=\"i\" />\n"
"    </signal>\n"
"  </interface>\n"
"</node>\n"
;

// -------------------------------------------------------------------------------- //
guMPRIS::guMPRIS( guDBusServer * server, guPlayerPanel * playerpanel ) : guDBusClient( server )
{
    m_PlayerPanel = playerpanel;

    RegisterClient();

    RequestName( GUAYADEQUE_MPRIS_SERVICENAME );
}

// -------------------------------------------------------------------------------- //
guMPRIS::~guMPRIS()
{
}

// -------------------------------------------------------------------------------- //
void FillMetadataDetails( DBusMessageIter * Iter, const char * name, const char * value )
{
    DBusMessageIter dict_entry, variant;
    if( value )
    {
        dbus_message_iter_open_container( Iter, DBUS_TYPE_DICT_ENTRY, NULL, &dict_entry );

        dbus_message_iter_append_basic( &dict_entry, DBUS_TYPE_STRING, &name );
        dbus_message_iter_open_container( &dict_entry, DBUS_TYPE_VARIANT, DBUS_TYPE_STRING_AS_STRING, &variant );
        dbus_message_iter_append_basic( &variant, DBUS_TYPE_STRING, &value );
        dbus_message_iter_close_container( &dict_entry, &variant );

        dbus_message_iter_close_container( Iter, &dict_entry );
    }
}

// -------------------------------------------------------------------------------- //
void FillMetadataDetails( DBusMessageIter * Iter, const char * name, const int value )
{
    DBusMessageIter dict_entry, variant;
    if( value )
    {
        dbus_message_iter_open_container( Iter, DBUS_TYPE_DICT_ENTRY, NULL, &dict_entry );

        dbus_message_iter_append_basic( &dict_entry, DBUS_TYPE_STRING, &name );
        dbus_message_iter_open_container( &dict_entry, DBUS_TYPE_VARIANT, DBUS_TYPE_INT32_AS_STRING, &variant );
        dbus_message_iter_append_basic( &variant, DBUS_TYPE_INT32, &value );
        dbus_message_iter_close_container( &dict_entry, &variant );

        dbus_message_iter_close_container( Iter, &dict_entry );
    }
}

// -------------------------------------------------------------------------------- //
void FillMetadataArgs( guDBusMessage * reply, const guTrack * CurTrack )
{
    DBusMessageIter dict;
    DBusMessageIter args;

    wxASSERT( CurTrack );

    const char * metadata_names[] = {
        "location", "title", "artist", "album", "tracknumber",
        "time", "mtime", "genre", "rating", "year", "arturl", "bitrate"
    };

    dbus_message_iter_init_append( reply->GetMessage(), &args );

    dbus_message_iter_open_container( &args, DBUS_TYPE_ARRAY, "{sv}", &dict );

    FillMetadataDetails( &dict, metadata_names[ 0 ], ( const char * ) ( wxT( "file://" ) + CurTrack->m_FileName ).mb_str( wxConvUTF8 ) );
    FillMetadataDetails( &dict, metadata_names[ 1 ], ( const char * ) CurTrack->m_SongName.mb_str( wxConvUTF8 ) );
    FillMetadataDetails( &dict, metadata_names[ 2 ], ( const char * ) CurTrack->m_ArtistName.mb_str( wxConvUTF8 ) );
    FillMetadataDetails( &dict, metadata_names[ 3 ], ( const char * ) CurTrack->m_AlbumName.mb_str( wxConvUTF8 ) );
    if( CurTrack->m_Number )
        FillMetadataDetails( &dict, metadata_names[ 4 ], ( const int ) CurTrack->m_Number );
    FillMetadataDetails( &dict, metadata_names[ 5 ], ( const int ) CurTrack->m_Length );
    FillMetadataDetails( &dict, metadata_names[ 6 ], ( const int ) CurTrack->m_Length * 1000 );
    FillMetadataDetails( &dict, metadata_names[ 7 ], ( const char * ) CurTrack->m_GenreName.mb_str( wxConvUTF8 ) );

    if( CurTrack->m_Rating >= 0 )
        FillMetadataDetails( &dict, metadata_names[ 8 ], ( const int ) CurTrack->m_Rating );

    if( CurTrack->m_Year )
        FillMetadataDetails( &dict, metadata_names[ 9 ], ( const int ) CurTrack->m_Year );

    //if( !CurTrack->m_CoverPath.IsEmpty() )
    //    FillMetadataDetails( &dict, metadata_names[ 10 ], ( const char * ) ( wxT( "file://" ) + CurTrack->m_CoverPath ).mb_str( wxConvUTF8 ) );

    if( CurTrack->m_Rating )
        FillMetadataDetails( &dict, metadata_names[ 11 ], ( const int ) CurTrack->m_Bitrate );

    dbus_message_iter_close_container( &args, &dict );

}

// -------------------------------------------------------------------------------- //
void FillMetadataArgs( guDBusMessage * reply, const guCurrentTrack * CurTrack )
{
    DBusMessageIter dict;
    DBusMessageIter args;

    wxASSERT( CurTrack );

    const char * metadata_names[] = {
        "location", "title", "artist", "album", "tracknumber",
        "time", "mtime", "genre", "rating", "year", "arturl", "bitrate"
    };

    dbus_message_iter_init_append( reply->GetMessage(), &args );

    dbus_message_iter_open_container( &args, DBUS_TYPE_ARRAY, "{sv}", &dict );

    FillMetadataDetails( &dict, metadata_names[ 0 ], ( const char * ) ( wxT( "file://" ) + CurTrack->m_FileName ).mb_str( wxConvUTF8 ) );
    FillMetadataDetails( &dict, metadata_names[ 1 ], ( const char * ) CurTrack->m_SongName.mb_str( wxConvUTF8 ) );
    FillMetadataDetails( &dict, metadata_names[ 2 ], ( const char * ) CurTrack->m_ArtistName.mb_str( wxConvUTF8 ) );
    FillMetadataDetails( &dict, metadata_names[ 3 ], ( const char * ) CurTrack->m_AlbumName.mb_str( wxConvUTF8 ) );
    if( CurTrack->m_Number )
        FillMetadataDetails( &dict, metadata_names[ 4 ], ( const int ) CurTrack->m_Number );
    FillMetadataDetails( &dict, metadata_names[ 5 ], ( const int ) CurTrack->m_Length );
    FillMetadataDetails( &dict, metadata_names[ 6 ], ( const int ) CurTrack->m_Length * 1000 );
    FillMetadataDetails( &dict, metadata_names[ 7 ], ( const char * ) CurTrack->m_GenreName.mb_str( wxConvUTF8 ) );
    if( CurTrack->m_Rating >= 0 )
        FillMetadataDetails( &dict, metadata_names[ 8 ], ( const int ) CurTrack->m_Rating );

    if( CurTrack->m_Year )
        FillMetadataDetails( &dict, metadata_names[ 9 ], ( const int ) CurTrack->m_Year );

    if( !CurTrack->m_CoverPath.IsEmpty() )
        FillMetadataDetails( &dict, metadata_names[ 10 ], ( const char * ) ( wxT( "file://" ) + CurTrack->m_CoverPath ).mb_str( wxConvUTF8 ) );

    if( CurTrack->m_Rating )
        FillMetadataDetails( &dict, metadata_names[ 11 ], ( const int ) CurTrack->m_Bitrate );

    dbus_message_iter_close_container( &args, &dict );

}

// -------------------------------------------------------------------------------- //
DBusHandlerResult guMPRIS::HandleMessages( guDBusMessage * msg, guDBusMessage * reply )
{
    //
    DBusHandlerResult RetVal = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    const char *    Interface = msg->GetInterface();
    const char *    Member = msg->GetMember();
    const char *    Dest = msg->GetDestination();
    int             Type = msg->GetType();
    const char *    Path = msg->GetPath();
//    int             Serial = msg->GetSerial();
//    int             RSerial = msg->GetReplySerial();

//    // Show the details of the msg
//    guLogMessage( wxT( "==MPRIS========================" ) );
//    guLogMessage( wxT( "Type   : %i" ), Type );
//    guLogMessage( wxT( "Iface  : %s" ), wxString::FromAscii( Interface ).c_str() );
//    guLogMessage( wxT( "Dest   : %s" ), wxString::FromAscii( Dest ).c_str() );
//    guLogMessage( wxT( "Path   : %s" ), wxString::FromAscii( Path ).c_str() );
//    guLogMessage( wxT( "OPath  : %s" ), wxString::FromAscii( msg->GetObjectPath() ).c_str() );
//    guLogMessage( wxT( "Member : %s" ), wxString::FromAscii( Member ).c_str() );
//    guLogMessage( wxT( "Serial : %i" ), Serial );
//    guLogMessage( wxT( "RSerial: %i" ), RSerial );

    // If its a method call
    if( Type == DBUS_MESSAGE_TYPE_METHOD_CALL )
    {
        // Some buggie applications send incomplete message calls
        if( !Interface || !Member || !Path )
            return RetVal;

        // INTROSPECT
        if( !strcmp( Interface, "org.freedesktop.DBus.Introspectable" ) &&
            !strcmp( Member, "Introspect" ) )
        {
            if( Dest && !strcmp( Dest, GUAYADEQUE_MPRIS_SERVICENAME ) )
            {
                if( !strcmp( Path, "/" ) )
                {
                    DBusMessageIter args;
                    dbus_message_iter_init_append( reply->GetMessage(), &args );

                    if( !dbus_message_iter_append_basic( &args, DBUS_TYPE_STRING, &Introspection_XML_Data_Root ) )
                    {
                        guLogError( wxT( "Failed to attach the Introspection info" ) );
                    }
                    Send( reply );
                    Flush();
                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                }
                else if( !strcmp( Path, "/Player" ) )
                {
                    DBusMessageIter args;
                    dbus_message_iter_init_append( reply->GetMessage(), &args );

                    if( !dbus_message_iter_append_basic( &args, DBUS_TYPE_STRING, &Introspection_XML_Data_Player ) )
                    {
                        guLogError( wxT( "Failed to attach the Introspection info" ) );
                    }
                    Send( reply );
                    Flush();
                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                }
                else if( !strcmp( Path, "/TrackList" ) )
                {
                    DBusMessageIter args;
                    dbus_message_iter_init_append( reply->GetMessage(), &args );

                    if( !dbus_message_iter_append_basic( &args, DBUS_TYPE_STRING, &Introspection_XML_Data_Tracklist ) )
                    {
                        guLogError( wxT( "Failed to attach the Introspection info" ) );
                    }
                    Send( reply );
                    Flush();
                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                }
            }
        }


        if( !strcmp( Interface, "org.freedesktop.MediaPlayer" ) )
        {
            //
            // ROOT
            //
            if( !strcmp( Path, "/" ) )
            {
                if( !strcmp( Member, "Identity" ) )
                {
                    DBusMessageIter args;
                    dbus_message_iter_init_append( reply->GetMessage(), &args );
                    const char * VersionStr = "Guayadeque " ID_GUAYADEQUE_VERSION;
                    if( !dbus_message_iter_append_basic( &args, DBUS_TYPE_STRING, &VersionStr ) )
                    {
                        guLogError( wxT( "Failed to attach the root identity" ) );
                    }
                    Send( reply );
                    Flush();
                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                }
                else if( !strcmp( Member, "Quit" ) )
                {
                    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_MENU_QUIT );
                    wxPostEvent( m_PlayerPanel, event );
                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                }
                else if( !strcmp( Member, "MprisVersion" ) )
                {
                    dbus_uint16_t ver_major = GUAYADEQUE_MPRIS_VERSION_MAJOR;
                    dbus_uint16_t ver_minor = GUAYADEQUE_MPRIS_VERSION_MINOR;

                    DBusMessageIter verstruct;
                    DBusMessageIter args;
                    dbus_message_iter_init_append( reply->GetMessage(), &args );

                    dbus_message_iter_open_container( &args, DBUS_TYPE_STRUCT, NULL, &verstruct );

                    dbus_message_iter_append_basic( &verstruct, DBUS_TYPE_UINT16, &ver_major );
                    dbus_message_iter_append_basic( &verstruct, DBUS_TYPE_UINT16, &ver_minor );

                    dbus_message_iter_close_container( &args, &verstruct );
                    Send( reply );
                    Flush();
                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                }
            }

            //
            // PLAYER
            //
            else if( !strcmp( Path, "/Player" ) )
            {
                if( !strcmp( Member, "Next" ) )
                {
                    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_NEXTTRACK );
                    wxPostEvent( m_PlayerPanel, event );
                    Send( reply );
                    Flush();
                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                }
                else if( !strcmp( Member, "Prev" ) )
                {
                    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_PREVTRACK );
                    wxPostEvent( m_PlayerPanel, event );
                    Send( reply );
                    Flush();
                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                }
                else if( !strcmp( Member, "Pause" ) )
                {
                    //if( m_PlayerPanel->GetState() == guMediaState_PLAYING )
                    //{
                        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_PLAY );
                        wxPostEvent( m_PlayerPanel, event );
                    //}
                    Send( reply );
                    Flush();
                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                }
                else if( !strcmp( Member, "Stop" ) )
                {
                    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_STOP );
                    wxPostEvent( m_PlayerPanel, event );
                    Send( reply );
                    Flush();
                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                }
                else if( !strcmp( Member, "Play" ) )
                {
                    if( m_PlayerPanel->GetState() == guMEDIASTATE_PAUSED )
                    {
                        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_PLAY );
                        wxPostEvent( m_PlayerPanel, event );
                    }
                    else
                    {
                        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_STOP );
                        wxPostEvent( m_PlayerPanel, event );
                        event.SetId( ID_PLAYERPANEL_PLAY );
                        wxPostEvent( m_PlayerPanel, event );
                    }
                    Send( reply );
                    Flush();
                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                }
                else if( !strcmp( Member, "Repeat" ) )
                {
                    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_SETREPEAT );
                    wxPostEvent( m_PlayerPanel, event );

                    Send( reply );
                    Flush();
                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                }
                else if( !strcmp( Member, "GetStatus" ) )
                {
                    int PlayStatus;
                    int Dummy = 0;
                    guMediaState State = m_PlayerPanel->GetState();
                    if( State == guMEDIASTATE_STOPPED )
                        PlayStatus = 2;
                    else if( State == guMEDIASTATE_PAUSED )
                        PlayStatus = 1;
                    else if( State == guMEDIASTATE_PLAYING )
                        PlayStatus = 0;
                    int PlaySingle = ( m_PlayerPanel->GetPlayLoop() == 2 );
                    int PlayLoop = ( m_PlayerPanel->GetPlayLoop() == 1 );

                    DBusMessageIter status;
                    DBusMessageIter args;
                    dbus_message_iter_init_append( reply->GetMessage(), &args );

                    dbus_message_iter_open_container( &args, DBUS_TYPE_STRUCT, NULL, &status );
                    dbus_message_iter_append_basic( &status, DBUS_TYPE_INT32, &PlayStatus );
                    dbus_message_iter_append_basic( &status, DBUS_TYPE_INT32, &Dummy );
                    dbus_message_iter_append_basic( &status, DBUS_TYPE_INT32, &PlaySingle );
                    dbus_message_iter_append_basic( &status, DBUS_TYPE_INT32, &PlayLoop );
                    dbus_message_iter_close_container( &args, &status );
                    Send( reply );
                    Flush();
                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                }
                else if( !strcmp( Member, "GetMetadata" ) )
                {
                    const guCurrentTrack * CurTrack = m_PlayerPanel->GetCurrentTrack();
                    if( CurTrack )
                    {
                        FillMetadataArgs( reply, CurTrack );
                    }
                    Send( reply );
                    Flush();
                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                }
                else if( !strcmp( Member, "GetCaps" ) )
                {
                    DBusMessageIter args;
                    dbus_message_iter_init_append( reply->GetMessage(), &args );
                    int Caps = m_PlayerPanel->GetCaps();
                    if( !dbus_message_iter_append_basic( &args, DBUS_TYPE_INT32, &Caps ) )
                    {
                        guLogError( wxT( "Failed to attach the player Caps" ) );
                    }
                    Send( reply );
                    Flush();
                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                }
                else if( !strcmp( Member, "VolumeSet" ) )
                {
                    DBusError error;
                    dbus_error_init( &error );

                    dbus_int32_t Volume;

                    dbus_message_get_args( msg->GetMessage(), &error, DBUS_TYPE_INT32, &Volume, DBUS_TYPE_INVALID );

                    if( dbus_error_is_set( &error ) )
                    {
                        printf( "Could not read the Volume parameter : %s\n", error.message );
                        dbus_error_free( &error );
                        RetVal =  DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
                    }
                    else
                    {
                        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_SETVOLUME );
                        event.SetInt( Volume );
                        wxPostEvent( m_PlayerPanel, event );

                        Send( reply );
                        Flush();
                        RetVal = DBUS_HANDLER_RESULT_HANDLED;
                    }
                }
                else if( !strcmp( Member, "VolumeGet" ) )
                {
                    DBusMessageIter args;
                    dbus_message_iter_init_append( reply->GetMessage(), &args );

                    int Volume = m_PlayerPanel->GetVolume();

                    if( !dbus_message_iter_append_basic( &args, DBUS_TYPE_INT32, &Volume ) )
                    {
                        guLogError( wxT( "Failed to attach the Player volume" ) );
                    }
                    Send( reply );
                    Flush();
                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                }
                else if( !strcmp( Member, "PositionSet" ) )
                {
                    DBusError error;
                    dbus_error_init( &error );

                    dbus_int32_t Position;

                    dbus_message_get_args( msg->GetMessage(), &error, DBUS_TYPE_INT32, &Position, DBUS_TYPE_INVALID );

                    if( dbus_error_is_set( &error ) )
                    {
                        printf( "Could not read the Position parameter : %s\n", error.message );
                        dbus_error_free( &error );
                        RetVal =  DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
                    }
                    else
                    {
                        m_PlayerPanel->SetPosition( Position );

                        Send( reply );
                        Flush();
                        RetVal = DBUS_HANDLER_RESULT_HANDLED;
                    }
                }
                else if( !strcmp( Member, "PositionGet" ) )
                {
                    DBusMessageIter args;
                    dbus_message_iter_init_append( reply->GetMessage(), &args );

                    int Position = m_PlayerPanel->GetPosition();

                    if( !dbus_message_iter_append_basic( &args, DBUS_TYPE_INT32, &Position ) )
                    {
                        guLogError( wxT( "Failed to attach the Player position" ) );
                    }
                    Send( reply );
                    Flush();
                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                }
            }

            //
            // TRACKLIST
            //
            else if( !strcmp( Path, "/TrackList" ) )
            {
                if( !strcmp( Member, "GetMetadata" ) )
                {
                    DBusError error;
                    dbus_error_init( &error );

                    dbus_int32_t TrackNum;

                    dbus_message_get_args( msg->GetMessage(), &error, DBUS_TYPE_INT32, &TrackNum, DBUS_TYPE_INVALID );

                    if( dbus_error_is_set( &error ) )
                    {
                        printf( "Could not get the GetMetadata parameter : %s\n", error.message );
                        dbus_error_free( &error );
                        RetVal =  DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
                    }
                    else
                    {
                        const guTrack * CurTrack = m_PlayerPanel->GetTrack( TrackNum );
                        if( CurTrack )
                        {
                            FillMetadataArgs( reply, CurTrack );
                        }
                        Send( reply );
                        Flush();
                        RetVal = DBUS_HANDLER_RESULT_HANDLED;
                    }
                }
                else if( !strcmp( Member, "GetCurrentTrack" ) )
                {
                    DBusMessageIter args;
                    dbus_message_iter_init_append( reply->GetMessage(), &args );

                    int CurTrack = m_PlayerPanel->GetCurrentItem();

                    if( !dbus_message_iter_append_basic( &args, DBUS_TYPE_INT32, &CurTrack ) )
                    {
                        guLogError( wxT( "Failed to attach the Player position" ) );
                    }
                    Send( reply );
                    Flush();
                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                }
                else if( !strcmp( Member, "GetLength" ) )
                {
                    DBusMessageIter args;
                    dbus_message_iter_init_append( reply->GetMessage(), &args );

                    int TrackCnt = m_PlayerPanel->GetItemCount();

                    if( !dbus_message_iter_append_basic( &args, DBUS_TYPE_INT32, &TrackCnt ) )
                    {
                        guLogError( wxT( "Failed to attach the Player position" ) );
                    }
                    Send( reply );
                    Flush();
                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                }
                else if( !strcmp( Member, "AddTrack" ) )
                {
                    DBusError error;
                    dbus_error_init( &error );

                    const char *    TrackPath;
                    dbus_bool_t     PlayTrack;

                    dbus_message_get_args( msg->GetMessage(), &error,
                          DBUS_TYPE_STRING, &TrackPath,
                          DBUS_TYPE_BOOLEAN, &PlayTrack,
                          DBUS_TYPE_INVALID );

                    //guLogMessage( wxT( "MPRIS: AddTrack\n%s\n%i" ), wxString( TrackPath, wxConvUTF8 ).c_str(), PlayTrack );

                    if( dbus_error_is_set( &error ) )
                    {
                        printf( "Could not read the AddTrack parameters : %s\n", error.message );
                        dbus_error_free( &error );
                        RetVal =  DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
                    }
                    else
                    {
                        DBusMessageIter args;
                        dbus_message_iter_init_append( reply->GetMessage(), &args );

                        wxArrayString * TrackList = new wxArrayString();
                        TrackList->Add( wxString( TrackPath, wxConvUTF8 ) );

                        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_ADDTRACKS );
                        event.SetInt( PlayTrack );
                        event.SetClientData( TrackList );
                        wxPostEvent( m_PlayerPanel, event );

                        int TrackAdded = 1;
                        if( !dbus_message_iter_append_basic( &args, DBUS_TYPE_INT32, &TrackAdded ) )
                        {
                            guLogError( wxT( "Failed to attach the AddTrack return code" ) );
                        }

                        Send( reply );
                        Flush();
                        RetVal = DBUS_HANDLER_RESULT_HANDLED;
                    }
                }
                else if( !strcmp( Member, "AddTracks" ) )
                {
                    DBusError error;
                    dbus_error_init( &error );

                    char **         Tracks;
                    dbus_int32_t    TracksCount;
                    dbus_bool_t     PlayTrack;

                    dbus_message_get_args( msg->GetMessage(), &error,
                          DBUS_TYPE_ARRAY, DBUS_TYPE_STRING, &Tracks, &TracksCount,
                          DBUS_TYPE_BOOLEAN, &PlayTrack,
                          DBUS_TYPE_INVALID );
//
//                    //guLogMessage( wxT( "MPRIS: AddTrack\n%s\n%i" ), wxString( TrackPath, wxConvUTF8 ).c_str(), PlayTrack );
//
                    if( dbus_error_is_set( &error ) )
                    {
                        guLogMessage( wxT( "Could not read the AddTracks parameters : '%s'" ), wxString( error.message, wxConvUTF8 ).c_str() );
                        dbus_error_free( &error );
                        RetVal =  DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
                    }
                    else
                    {
                        DBusMessageIter args;
                        dbus_message_iter_init_append( reply->GetMessage(), &args );

                        wxArrayString * TrackList = new wxArrayString();
                        int Index;
                        for( Index = 0; Index < TracksCount; Index++ )
                        {
                            TrackList->Add( wxString( Tracks[ Index ], wxConvUTF8 ) );
                        }

                        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_ADDTRACKS );
                        event.SetInt( PlayTrack );
                        event.SetClientData( TrackList );
                        wxPostEvent( m_PlayerPanel, event );

                        int TrackAdded = 1;
                        if( !dbus_message_iter_append_basic( &args, DBUS_TYPE_INT32, &TrackAdded ) )
                        {
                            guLogError( wxT( "Failed to attach the AddTrack return code" ) );
                        }

                        Send( reply );
                        Flush();
                        RetVal = DBUS_HANDLER_RESULT_HANDLED;
                    }
                }
                else if( !strcmp( Member, "DelTrack" ) )
                {
                    DBusError error;
                    dbus_error_init( &error );

                    dbus_int32_t TrackNum;

                    dbus_message_get_args( msg->GetMessage(), &error, DBUS_TYPE_INT32, &TrackNum, DBUS_TYPE_INVALID );

                    if( dbus_error_is_set( &error ) )
                    {
                        printf( "Could not get DelTrack parameter : %s\n", error.message );
                        dbus_error_free( &error );
                        RetVal =  DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
                    }
                    else
                    {
                        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_REMOVETRACK );
                        event.SetInt( TrackNum );
                        wxPostEvent( m_PlayerPanel, event );

                        //Send( reply );
                        Flush();
                        RetVal = DBUS_HANDLER_RESULT_HANDLED;
                    }
                }
                else if( !strcmp( Member, "SetLoop" ) )
                {
                    DBusError error;
                    dbus_error_init( &error );

                    dbus_bool_t     PlayLoop;

                    dbus_message_get_args( msg->GetMessage(), &error,
                          DBUS_TYPE_BOOLEAN, &PlayLoop,
                          DBUS_TYPE_INVALID );

                    if( dbus_error_is_set( &error ) )
                    {
                        printf( "Could not read the AddTrack parameters : %s\n", error.message );
                        dbus_error_free( &error );
                        RetVal =  DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
                    }
                    else
                    {
                        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_SETLOOP );
                        event.SetInt( PlayLoop );
                        wxPostEvent( m_PlayerPanel, event );

                        Flush();
                        RetVal = DBUS_HANDLER_RESULT_HANDLED;
                    }
                }
                else if( !strcmp( Member, "SetRandom" ) )
                {
                    DBusError error;
                    dbus_error_init( &error );

                    dbus_bool_t     Random;

                    dbus_message_get_args( msg->GetMessage(), &error,
                          DBUS_TYPE_BOOLEAN, &Random,
                          DBUS_TYPE_INVALID );

                    if( dbus_error_is_set( &error ) )
                    {
                        printf( "Could not read the AddTrack parameters : %s\n", error.message );
                        dbus_error_free( &error );
                        RetVal =  DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
                    }
                    else
                    {
                        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_SETRANDOM );
                        wxPostEvent( m_PlayerPanel, event );

                        Flush();
                        RetVal = DBUS_HANDLER_RESULT_HANDLED;
                    }
                }
            }
        }
    }

    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guMPRIS::OnPlayerTrackChange()
{
    guDBusSignal * signal = new guDBusSignal( "/Player", "org.freedesktop.MediaPlayer", "TrackChange" );
    if( signal )
    {
        const guCurrentTrack * CurTrack = m_PlayerPanel->GetCurrentTrack();
        FillMetadataArgs( signal, CurTrack );
        Send( signal );
        Flush();
        delete signal;
    }
    else
    {
        guLogError( wxT( "Could not create CapsChange signal object" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guMPRIS::OnPlayerStatusChange()
{
    guDBusSignal * signal = new guDBusSignal( "/Player", "org.freedesktop.MediaPlayer", "StatusChange" );
    if( signal )
    {
        int PlayStatus;
        int Dummy = 0;
        guMediaState State = m_PlayerPanel->GetState();
        if( State == guMEDIASTATE_STOPPED )
            PlayStatus = 2;
        else if( State == guMEDIASTATE_PAUSED )
            PlayStatus = 1;
        else if( State == guMEDIASTATE_PLAYING )
            PlayStatus = 0;
        int PlaySingle = ( m_PlayerPanel->GetPlayLoop() == 2 );
        int PlayLoop = ( m_PlayerPanel->GetPlayLoop() == 1 );

        guLogMessage( wxT( "StatusChanged( %i, %i, %i, %i )" ), PlayStatus, Dummy, Dummy, PlayLoop );

        DBusMessageIter status;
        DBusMessageIter args;
        dbus_message_iter_init_append( signal->GetMessage(), &args );

        dbus_message_iter_open_container( &args, DBUS_TYPE_STRUCT, NULL, &status );
        dbus_message_iter_append_basic( &status, DBUS_TYPE_INT32, &PlayStatus );
        dbus_message_iter_append_basic( &status, DBUS_TYPE_INT32, &Dummy );
        dbus_message_iter_append_basic( &status, DBUS_TYPE_INT32, &PlaySingle );
        dbus_message_iter_append_basic( &status, DBUS_TYPE_INT32, &PlayLoop );
        dbus_message_iter_close_container( &args, &status );

        Send( signal );
        Flush();
        delete signal;
    }
    else
    {
        guLogError( wxT( "Could not create CapsChange signal object" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guMPRIS::OnPlayerCapsChange()
{
    guDBusSignal * signal = new guDBusSignal( "/Player", "org.freedesktop.MediaPlayer", "CapsChange" );
    if( signal )
    {
        DBusMessageIter args;
        dbus_message_iter_init_append( signal->GetMessage(), &args );
        int Caps = m_PlayerPanel->GetCaps();
        if( !dbus_message_iter_append_basic( &args, DBUS_TYPE_INT32, &Caps ) )
        {
            guLogError( wxT( "Failed to attach the player Caps" ) );
        }
        Send( signal );
        Flush();
        delete signal;
    }
    else
    {
        guLogError( wxT( "Could not create CapsChange signal object" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guMPRIS::OnTrackListChange()
{
    guDBusSignal * signal = new guDBusSignal( "/TrackList", "org.freedesktop.MediaPlayer", "TrackListChange" );
    if( signal )
    {
        DBusMessageIter args;
        dbus_message_iter_init_append( signal->GetMessage(), &args );

        int TrackCnt = m_PlayerPanel->GetItemCount();

        if( !dbus_message_iter_append_basic( &args, DBUS_TYPE_INT32, &TrackCnt ) )
        {
            guLogError( wxT( "Failed to attach the TrackCount" ) );
        }
        Send( signal );
        Flush();
        delete signal;
    }
    else
    {
        guLogError( wxT( "Could not create TrackListChange signal object" ) );
    }
}

// -------------------------------------------------------------------------------- //
