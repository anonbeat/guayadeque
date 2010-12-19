// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2010 J.Rios
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
#include "mpris2.h"

#include "Commands.h"
#include "mpris.h"
#include "Utils.h"
#include "MainFrame.h"

// See http://www.mpris.org/2.0/spec/index.html
#define GUAYADEQUE_MPRIS2_SERVICE_NAME          "org.mpris.MediaPlayer2.guayadeque"
#define GUAYADEQUE_MPRIS2_OBJECT_PATH           "/org/mpris/MediaPlayer2"
#define GUAYADEQUE_PROPERTIES_INTERFACE         "org.freedesktop.DBus.Properties"

#define GUAYADEQUE_MPRIS2_INTERFACE_ROOT        "org.mpris.MediaPlayer2"
#define GUAYADEQUE_MPRIS2_INTERFACE_PLAYER      "org.mpris.MediaPlayer2.Player"
#define GUAYADEQUE_MPRIS2_INTERFACE_TRACKLIST   "org.mpris.MediaPlayer2.TrackList"

const char * guMPRIS2_INTROSPECTION_XML =
	"<node>"
	"  <interface name='org.mpris.MediaPlayer2'>"
	"    <method name='Raise'/>"
	"    <method name='Quit'/>"
	"    <property name='CanQuit' type='b' access='read'/>"
	"    <property name='CanRaise' type='b' access='read'/>"
	"    <property name='HasTrackList' type='b' access='read'/>"
	"    <property name='Identity' type='s' access='read'/>"
	"    <property name='DesktopEntry' type='s' access='read'/>"
	"    <property name='SupportedUriSchemes' type='as' access='read'/>"
	"    <property name='SupportedMimeTypes' type='as' access='read'/>"
	"  </interface>"
	"  <interface name='org.mpris.MediaPlayer2.Player'>"
	"    <method name='Next'/>"
	"    <method name='Previous'/>"
	"    <method name='Pause'/>"
	"    <method name='PlayPause'/>"
	"    <method name='Stop'/>"
	"    <method name='Play'/>"
	"    <method name='Seek'>"
	"      <arg direction='in' name='Offset' type='x'/>"
	"    </method>"
	"    <method name='SetPosition'>"
	"      <arg direction='in' name='TrackId' type='o'/>"
	"      <arg direction='in' name='Position' type='x'/>"
	"    </method>"
	"    <method name='OpenUri'>"
	"      <arg direction='in' name='Uri' type='s'/>"
	"    </method>"
	"    <signal name='Seeked'>"
	"      <arg name='Position' type='x'/>"
	"    </signal>"
	"    <property name='PlaybackStatus' type='s' access='read'/>"
	"    <property name='LoopStatus' type='s' access='readwrite'/>"
	"    <property name='Rate' type='d' access='readwrite'/>"
	"    <property name='Shuffle' type='b' access='readwrite'/>"
	"    <property name='Metadata' type='a{sv}' access='read'/>"
	"    <property name='Volume' type='d' access='readwrite'/>"
	"    <property name='Position' type='x' access='read'/>"
	"    <property name='MinimumRate' type='d' access='read'/>"
	"    <property name='MaximumRate' type='d' access='read'/>"
	"    <property name='CanGoNext' type='b' access='read'/>"
	"    <property name='CanGoPrevious' type='b' access='read'/>"
	"    <property name='CanPlay' type='b' access='read'/>"
	"    <property name='CanPause' type='b' access='read'/>"
	"    <property name='CanSeek' type='b' access='read'/>"
	"    <property name='CanControl' type='b' access='read'/>"
	"  </interface>"
	"  <interface name='org.mpris.MediaPlayer2.TrackList'>"
	"    <method name='GetTracksMetadata'>"
	"      <arg direction='in' name='TrackIds' type='ao'/>"
	"      <arg direction='out' name='Metadata' type='aa{sv}'/>"
	"    </method>"
	"    <method name='AddTrack'>"
	"      <arg direction='in' name='Uri' type='s'/>"
	"      <arg direction='in' name='AfterTrack' type='o'/>"
	"      <arg direction='in' name='SetAsCurrent' type='b'/>"
	"    </method>"
	"    <method name='RemoveTrack'>"
	"      <arg direction='in' name='TrackId' type='o'/>"
	"    </method>"
	"    <method name='GoTo'>"
	"      <arg direction='in' name='TrackId' type='o'/>"
	"    </method>"
	"    <signal name='TrackListReplaced'>"
	"      <arg name='Tracks' type='ao'/>"
	"      <arg name='CurrentTrack' type='o'/>"
	"    </signal>"
	"    <signal name='TrackAdded'>"
	"      <arg name='Metadata' type='a{sv}'/>"
	"      <arg name='AfterTrack' type='o'/>"
	"    </signal>"
	"    <signal name='TrackRemoved'>"
	"      <arg name='TrackId' type='o'/>"
	"    </signal>"
	"    <signal name='TrackMetadataChanged'>"
	"      <arg name='TrackId' type='o'/>"
	"      <arg name='Metadata' type='a{sv}'/>"
	"    </signal>"
	"    <property name='Tracks' type='ao' access='read'/>"
	"    <property name='CanEditTracks' type='b' access='read'/>"
	"  </interface>"
	"</node>";


// -------------------------------------------------------------------------------- //
guMPRIS2::guMPRIS2( guDBusServer * server, guPlayerPanel * playerpanel ) : guDBusClient( server )
{
    m_PlayerPanel = playerpanel;

    RegisterClient();

    RequestName( GUAYADEQUE_MPRIS2_SERVICE_NAME );
    RegisterObjectPath( GUAYADEQUE_MPRIS2_OBJECT_PATH );
}

// -------------------------------------------------------------------------------- //
guMPRIS2::~guMPRIS2()
{
}

// -------------------------------------------------------------------------------- //
static void FillMetadataDetails( DBusMessageIter * Iter, const char * name, const char * value )
{
    DBusMessageIter dict_entry, variant;
    if( value && strlen( value ) )
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
static void FillMetadataDetails( DBusMessageIter * Iter, const char * name, const int value )
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
static void FillMetadataDetails( DBusMessageIter * Iter, const char * name, const gint64 value )
{
    DBusMessageIter dict_entry, variant;
    if( value )
    {
        dbus_message_iter_open_container( Iter, DBUS_TYPE_DICT_ENTRY, NULL, &dict_entry );

        dbus_message_iter_append_basic( &dict_entry, DBUS_TYPE_STRING, &name );
        dbus_message_iter_open_container( &dict_entry, DBUS_TYPE_VARIANT, DBUS_TYPE_INT64_AS_STRING, &variant );
        dbus_message_iter_append_basic( &variant, DBUS_TYPE_INT64, &value );
        dbus_message_iter_close_container( &dict_entry, &variant );

        dbus_message_iter_close_container( Iter, &dict_entry );
    }
}

// -------------------------------------------------------------------------------- //
static void FillMetadataDetails( DBusMessageIter * Iter, const char * name, const double value )
{
    DBusMessageIter dict_entry, variant;
    if( value )
    {
        dbus_message_iter_open_container( Iter, DBUS_TYPE_DICT_ENTRY, NULL, &dict_entry );

        dbus_message_iter_append_basic( &dict_entry, DBUS_TYPE_STRING, &name );
        dbus_message_iter_open_container( &dict_entry, DBUS_TYPE_VARIANT, DBUS_TYPE_DOUBLE_AS_STRING, &variant );
        dbus_message_iter_append_basic( &variant, DBUS_TYPE_DOUBLE, &value );
        dbus_message_iter_close_container( &dict_entry, &variant );

        dbus_message_iter_close_container( Iter, &dict_entry );
    }
}

// -------------------------------------------------------------------------------- //
static void FillMetadataDetails( DBusMessageIter * Iter, const char * name, const bool value )
{
    DBusMessageIter dict_entry, variant;
    if( value )
    {
        dbus_message_iter_open_container( Iter, DBUS_TYPE_DICT_ENTRY, NULL, &dict_entry );

        dbus_message_iter_append_basic( &dict_entry, DBUS_TYPE_STRING, &name );
        dbus_message_iter_open_container( &dict_entry, DBUS_TYPE_VARIANT, DBUS_TYPE_BOOLEAN_AS_STRING, &variant );
        dbus_message_iter_append_basic( &variant, DBUS_TYPE_BOOLEAN, &value );
        dbus_message_iter_close_container( &dict_entry, &variant );

        dbus_message_iter_close_container( Iter, &dict_entry );
    }
}

// -------------------------------------------------------------------------------- //
static void FillMetadataAsList( DBusMessageIter * Iter, const char * name, const char * value )
{
    DBusMessageIter dict_entry, variant, array;
    if( value && strlen( value ) )
    {
        dbus_message_iter_open_container( Iter, DBUS_TYPE_DICT_ENTRY, NULL, &dict_entry );

        dbus_message_iter_append_basic( &dict_entry, DBUS_TYPE_STRING, &name );

        dbus_message_iter_open_container( &dict_entry, DBUS_TYPE_VARIANT, "as", &variant );

        dbus_message_iter_open_container( &variant, DBUS_TYPE_ARRAY, "s", &array );

        dbus_message_iter_append_basic( &array, DBUS_TYPE_STRING, &value );

        dbus_message_iter_close_container( &variant, &array );

        dbus_message_iter_close_container( &dict_entry, &variant );

        dbus_message_iter_close_container( Iter, &dict_entry );
    }
}

//// -------------------------------------------------------------------------------- //
//void FillMetadataArgs( guDBusMessage * reply, const guTrack * CurTrack )
//{
//    DBusMessageIter dict;
//    DBusMessageIter args;
//
//    wxASSERT( CurTrack );
//
//    const char * metadata_names[] = {
//        "location", "title", "artist", "album", "tracknumber",
//        "time", "mtime", "genre", "rating", "year", "arturl", "bitrate"
//    };
//
//    dbus_message_iter_init_append( reply->GetMessage(), &args );
//
//    dbus_message_iter_open_container( &args, DBUS_TYPE_ARRAY, "{sv}", &dict );
//
//    FillMetadataDetails( &dict, metadata_names[ 0 ], ( const char * ) ( wxT( "file://" ) + CurTrack->m_FileName ).mb_str( wxConvUTF8 ) );
//    FillMetadataDetails( &dict, metadata_names[ 1 ], ( const char * ) CurTrack->m_SongName.mb_str( wxConvUTF8 ) );
//    FillMetadataDetails( &dict, metadata_names[ 2 ], ( const char * ) CurTrack->m_ArtistName.mb_str( wxConvUTF8 ) );
//    FillMetadataDetails( &dict, metadata_names[ 3 ], ( const char * ) CurTrack->m_AlbumName.mb_str( wxConvUTF8 ) );
//    if( CurTrack->m_Number )
//        FillMetadataDetails( &dict, metadata_names[ 4 ], ( const int ) CurTrack->m_Number );
//    FillMetadataDetails( &dict, metadata_names[ 5 ], ( const int ) CurTrack->m_Length );
//    FillMetadataDetails( &dict, metadata_names[ 6 ], ( const int ) CurTrack->m_Length * 1000 );
//    FillMetadataDetails( &dict, metadata_names[ 7 ], ( const char * ) CurTrack->m_GenreName.mb_str( wxConvUTF8 ) );
//
//    if( CurTrack->m_Rating >= 0 )
//        FillMetadataDetails( &dict, metadata_names[ 8 ], ( const int ) CurTrack->m_Rating );
//
//    if( CurTrack->m_Year )
//        FillMetadataDetails( &dict, metadata_names[ 9 ], ( const int ) CurTrack->m_Year );
//
////    if( !CurTrack->m_CoverPath.IsEmpty() )
////        FillMetadataDetails( &dict, metadata_names[ 10 ], ( const char * ) ( wxT( "file://" ) + CurTrack->m_CoverPath ).mb_str( wxConvUTF8 ) );
//
//    if( CurTrack->m_Rating )
//        FillMetadataDetails( &dict, metadata_names[ 11 ], ( const int ) CurTrack->m_Bitrate );
//
//    dbus_message_iter_close_container( &args, &dict );
//
//}

// -------------------------------------------------------------------------------- //
static void FillMetadataIter( DBusMessageIter * iter, const guCurrentTrack * curtrack, const int trackid )
{
    DBusMessageIter dict;

    dbus_message_iter_open_container( iter, DBUS_TYPE_ARRAY, "{sv}", &dict );

    if( curtrack->m_Loaded )
    {
        FillMetadataDetails( &dict, "mpris:trackid", ( const char * ) wxString::Format( wxT( "/org/mpris/MediaPlayer2/Track/%u" ), trackid ).mb_str( wxConvUTF8 ) );
        wxString LocationUrl = wxT( "file://" ) + curtrack->m_FileName;
        LocationUrl.Replace( wxT( " "), wxT( "%20" ) );
        FillMetadataDetails( &dict, "xesam:url", ( const char * ) LocationUrl.mb_str( wxConvUTF8 ) );
        FillMetadataDetails( &dict, "xesam:title", ( const char * ) curtrack->m_SongName.mb_str( wxConvUTF8 ) );
        FillMetadataAsList( &dict, "xesam:artist", ( const char * ) curtrack->m_ArtistName.mb_str( wxConvUTF8 ) );
        FillMetadataAsList( &dict, "xesam:albumArtist", ( const char * ) curtrack->m_AlbumArtist.mb_str( wxConvUTF8 ) );
        FillMetadataAsList( &dict, "xesam:comment", ( const char * ) curtrack->m_Comments.mb_str( wxConvUTF8 ) );
        FillMetadataAsList( &dict, "xesam:composer", ( const char * ) curtrack->m_Composer.mb_str( wxConvUTF8 ) );
        FillMetadataDetails( &dict, "xesam:album", ( const char * ) curtrack->m_AlbumName.mb_str( wxConvUTF8 ) );
        if( curtrack->m_Number )
            FillMetadataDetails( &dict, "xesam:trackNumber", ( const int ) curtrack->m_Number );
        FillMetadataDetails( &dict, "mpris:length", ( const gint64 ) gint64( curtrack->m_Length * 1000000 ) );
        FillMetadataAsList( &dict, "xesan:genre", ( const char * ) curtrack->m_GenreName.mb_str( wxConvUTF8 ) );
        if( curtrack->m_Rating >= 0 )
            FillMetadataDetails( &dict, "xesam:userRating", ( const double ) double( curtrack->m_Rating * 0.2 ) );

    //    if( curtrack->m_Year )
    //        FillMetadataDetails( &dict, "year", ( const int ) curtrack->m_Year );
        FillMetadataDetails( &dict, "xesam:useCount", ( const int ) curtrack->m_PlayCount );

        if( !curtrack->m_CoverPath.IsEmpty() )
            FillMetadataDetails( &dict, "mpris:artUrl", ( const char * ) ( wxT( "file://" ) + curtrack->m_CoverPath ).mb_str( wxConvUTF8 ) );

        if( curtrack->m_Bitrate )
            FillMetadataDetails( &dict, "xesam:audioBitrate", ( const int ) curtrack->m_Bitrate * 1000 );
    }

    dbus_message_iter_close_container( iter, &dict );
}

// -------------------------------------------------------------------------------- //
static void FillMetadataArgs( guDBusMessage * reply, const guCurrentTrack * curtrack, const int trackid )
{
    DBusMessageIter args, variant;

    dbus_message_iter_init_append( reply->GetMessage(), &args );

    dbus_message_iter_open_container( &args, DBUS_TYPE_VARIANT, "a{sv}", &variant );

    FillMetadataIter( &variant, curtrack, trackid );

    dbus_message_iter_close_container( &args, &variant );
}

// -------------------------------------------------------------------------------- //
static bool AddVariant( DBusMessage * msg, const int type, const void * data )
{
	DBusMessageIter iter, sub;
	char Types[ 2 ];

	Types[ 0 ] = ( char ) type;
	Types[ 1 ] = DBUS_TYPE_INVALID;

	dbus_message_iter_init_append( msg, &iter );

	return dbus_message_iter_open_container( &iter, DBUS_TYPE_VARIANT, Types, &sub ) &&
           dbus_message_iter_append_basic( &sub, type, data ) &&
           dbus_message_iter_close_container( &iter, &sub );
}

// -------------------------------------------------------------------------------- //
void guMPRIS2::OnPlayerTrackChange( void )
{
    guDBusSignal * signal = new guDBusSignal( GUAYADEQUE_MPRIS2_OBJECT_PATH, GUAYADEQUE_PROPERTIES_INTERFACE, "PropertiesChanged" );
    if( signal )
    {
        DBusMessageIter args;
        DBusMessageIter dict;

        dbus_message_iter_init_append( signal->GetMessage(), &args );

        const char * Interface = GUAYADEQUE_MPRIS2_INTERFACE_PLAYER;
        dbus_message_iter_append_basic( &args, DBUS_TYPE_STRING, &Interface );

        dbus_message_iter_open_container( &args, DBUS_TYPE_ARRAY, "{sv}", &dict );

        DBusMessageIter dict_entry, variant;
        dbus_message_iter_open_container( &dict, DBUS_TYPE_DICT_ENTRY, NULL, &dict_entry );

        const char * Metadata = "Metadata";
        dbus_message_iter_append_basic( &dict_entry, DBUS_TYPE_STRING, &Metadata );

        dbus_message_iter_open_container( &dict_entry, DBUS_TYPE_VARIANT, "a{sv}", &variant );

        const guCurrentTrack * CurTrack = m_PlayerPanel->GetCurrentTrack();
        const int TrackId = m_PlayerPanel->GetCurrentItem();
        FillMetadataIter( &variant, CurTrack, TrackId );

        dbus_message_iter_close_container( &dict_entry, &variant );

        dbus_message_iter_close_container( &dict, &dict_entry );

        const char * PlaybackStatus;
        guMediaState State = m_PlayerPanel->GetState();
        if( State == guMEDIASTATE_STOPPED )
            PlaybackStatus = "Stopped";
        else if( State == guMEDIASTATE_PAUSED )
            PlaybackStatus = "Paused";
        else //if( State == guMEDIASTATE_PLAYING )
            PlaybackStatus = "Playing";

        FillMetadataDetails( &dict, "PlaybackStatus", PlaybackStatus );

        const char * LoopStatus;
        int PlayLoop = m_PlayerPanel->GetPlayLoop();
        if( PlayLoop == guPLAYER_PLAYLOOP_NONE )
            LoopStatus = "None";
        else if( PlayLoop == guPLAYER_PLAYLOOP_TRACK )
            LoopStatus = "Track";
        else //if( PlayLoop == guPLAYER_PLAYLOOP_PLAYLIST )
            LoopStatus = "Playlist";

        FillMetadataDetails( &dict, "LoopStatus", LoopStatus );

        int Caps = m_PlayerPanel->GetCaps();

        bool CanGoNext = Caps & MPRIS_CAPS_CAN_GO_NEXT;
        FillMetadataDetails( &dict, "CanGoNext", CanGoNext );

        bool CanGoPrev = Caps & MPRIS_CAPS_CAN_GO_PREV;
        FillMetadataDetails( &dict, "CanGoPrevious", ( bool ) CanGoPrev );

        bool CanPlay = Caps & MPRIS_CAPS_CAN_PLAY;
        FillMetadataDetails( &dict, "CanPlay", CanPlay );

        bool CanPause = Caps & MPRIS_CAPS_CAN_PAUSE;
        FillMetadataDetails( &dict, "CanPause", CanPause );

        bool CanSeek = Caps & MPRIS_CAPS_CAN_SEEK;
        FillMetadataDetails( &dict, "CanSeek", CanSeek );


        dbus_message_iter_close_container( &args, &dict );

        dbus_message_iter_open_container( &args, DBUS_TYPE_ARRAY, "s", &dict );
        dbus_message_iter_close_container( &args, &dict );

        //const char * Signature = dbus_message_get_signature( signal->GetMessage() );
        //guLogMessage( wxT( "Signal signature: '%s'" ), wxString( Signature, wxConvUTF8 ).c_str() );

        Send( signal );
        Flush();
        delete signal;
    }
    else
    {
        guLogError( wxT( "Could not create EmitPropertyChangedSignal object" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guMPRIS2::OnPlayerStatusChange( void )
{
    guDBusSignal * signal = new guDBusSignal( GUAYADEQUE_MPRIS2_OBJECT_PATH, GUAYADEQUE_PROPERTIES_INTERFACE, "PropertiesChanged" );
    if( signal )
    {
        DBusMessageIter dict;
        DBusMessageIter args;

        dbus_message_iter_init_append( signal->GetMessage(), &args );

        const char * Interface = GUAYADEQUE_MPRIS2_INTERFACE_PLAYER;
        dbus_message_iter_append_basic( &args, DBUS_TYPE_STRING, &Interface );

        dbus_message_iter_open_container( &args, DBUS_TYPE_ARRAY, "{sv}", &dict );

        const char * PlaybackStatus;
        guMediaState State = m_PlayerPanel->GetState();
        if( State == guMEDIASTATE_STOPPED )
            PlaybackStatus = "Stopped";
        else if( State == guMEDIASTATE_PAUSED )
            PlaybackStatus = "Paused";
        else //if( State == guMEDIASTATE_PLAYING )
            PlaybackStatus = "Playing";

        FillMetadataDetails( &dict, "PlaybackStatus", PlaybackStatus );

        const char * LoopStatus;
        int PlayLoop = m_PlayerPanel->GetPlayLoop();
        if( PlayLoop == guPLAYER_PLAYLOOP_NONE )
            LoopStatus = "None";
        else if( PlayLoop == guPLAYER_PLAYLOOP_TRACK )
            LoopStatus = "Track";
        else //if( PlayLoop == guPLAYER_PLAYLOOP_PLAYLIST )
            LoopStatus = "Playlist";

        FillMetadataDetails( &dict, "LoopStatus", LoopStatus );

        dbus_message_iter_close_container( &args, &dict );

        dbus_message_iter_open_container( &args, DBUS_TYPE_ARRAY, "s", &dict );
        dbus_message_iter_close_container( &args, &dict );

        Send( signal );
        Flush();
        delete signal;
    }
    else
    {
        guLogError( wxT( "Could not create EmitPropertyChangedSignal object" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guMPRIS2::OnPlayerCapsChange( void )
{
    guDBusSignal * signal = new guDBusSignal( GUAYADEQUE_MPRIS2_OBJECT_PATH, GUAYADEQUE_PROPERTIES_INTERFACE, "PropertiesChanged" );
    if( signal )
    {
        DBusMessageIter dict;
        DBusMessageIter args;

        dbus_message_iter_init_append( signal->GetMessage(), &args );

        const char * Interface = GUAYADEQUE_MPRIS2_INTERFACE_PLAYER;
        dbus_message_iter_append_basic( &args, DBUS_TYPE_STRING, &Interface );

        dbus_message_iter_open_container( &args, DBUS_TYPE_ARRAY, "{sv}", &dict );

        int Caps = m_PlayerPanel->GetCaps();

        bool CanGoNext = Caps & MPRIS_CAPS_CAN_GO_NEXT;
        FillMetadataDetails( &dict, "CanGoNext", CanGoNext );

        bool CanGoPrev = Caps & MPRIS_CAPS_CAN_GO_PREV;
        FillMetadataDetails( &dict, "CanGoPrevious", CanGoPrev );

        bool CanPlay = Caps & MPRIS_CAPS_CAN_PLAY;
        FillMetadataDetails( &dict, "CanPlay", CanPlay );

        bool CanPause = Caps & MPRIS_CAPS_CAN_PAUSE;
        FillMetadataDetails( &dict, "CanPause", CanPause );


        dbus_message_iter_close_container( &args, &dict );

        dbus_message_iter_open_container( &args, DBUS_TYPE_ARRAY, "s", &dict );
        dbus_message_iter_close_container( &args, &dict );

        Send( signal );
        Flush();
        delete signal;
    }
    else
    {
        guLogError( wxT( "Could not create EmitPropertyChangedSignal object" ) );
    }
}


// -------------------------------------------------------------------------------- //
void guMPRIS2::OnPlayerVolumeChange( void )
{
    guDBusSignal * signal = new guDBusSignal( GUAYADEQUE_MPRIS2_OBJECT_PATH, GUAYADEQUE_PROPERTIES_INTERFACE, "PropertiesChanged" );
    if( signal )
    {
        DBusMessageIter dict;
        DBusMessageIter args;

        dbus_message_iter_init_append( signal->GetMessage(), &args );

        const char * Interface = GUAYADEQUE_MPRIS2_INTERFACE_PLAYER;
        dbus_message_iter_append_basic( &args, DBUS_TYPE_STRING, &Interface );

        dbus_message_iter_open_container( &args, DBUS_TYPE_ARRAY, "{sv}", &dict );

        double CurVolume = m_PlayerPanel->GetVolume() / 100;
        FillMetadataDetails( &dict, "Volume", CurVolume );

        dbus_message_iter_close_container( &args, &dict );

        dbus_message_iter_open_container( &args, DBUS_TYPE_ARRAY, "s", &dict );
        dbus_message_iter_close_container( &args, &dict );

        Send( signal );
        Flush();
        delete signal;
    }
    else
    {
        guLogError( wxT( "Could not create EmitPropertyChangedSignal object" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guMPRIS2::OnTrackListChange( void )
{
}

// -------------------------------------------------------------------------------- //
DBusHandlerResult guMPRIS2::HandleMessages( guDBusMessage * msg, guDBusMessage * reply )
{
    wxASSERT( msg );
    //
    DBusHandlerResult RetVal = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    const char *    Interface = msg->GetInterface();
    const char *    Member = msg->GetMember();
    int             Type = msg->GetType();
    const char *    Path = msg->GetPath();
    int             Serial = msg->GetSerial();
    int             RSerial = msg->GetReplySerial();

    // Show the details of the msg
    guLogMessage( wxT( "==MPRIS2========================" ) );
    guLogMessage( wxT( "Type   : %i" ), Type );
    guLogMessage( wxT( "Iface  : %s" ), wxString::FromAscii( Interface ).c_str() );
    guLogMessage( wxT( "Path   : %s" ), wxString::FromAscii( Path ).c_str() );
    guLogMessage( wxT( "OPath  : %s" ), wxString::FromAscii( msg->GetObjectPath() ).c_str() );
    guLogMessage( wxT( "Member : %s" ), wxString::FromAscii( Member ).c_str() );
    guLogMessage( wxT( "Serial : %i" ), Serial );
    guLogMessage( wxT( "RSerial: %i" ), RSerial );

    // If its a method call
    if( Type == DBUS_MESSAGE_TYPE_METHOD_CALL )
    {
        // Some buggie applications send incomplete message calls
        if( !Interface || !Member || !Path )
            return RetVal;

        if( !strcmp( Interface, "org.freedesktop.DBus.Introspectable" ) )
        {
            if( !strcmp( Member, "Introspect" ) )
            {
                DBusMessageIter args;
                dbus_message_iter_init_append( reply->GetMessage(), &args );

                if( !dbus_message_iter_append_basic( &args, DBUS_TYPE_STRING, &guMPRIS2_INTROSPECTION_XML ) )
                {
                    guLogError( wxT( "Failed to attach the Introspection info" ) );
                }
                Send( reply );
                Flush();
                RetVal = DBUS_HANDLER_RESULT_HANDLED;
            }
        }
        else if( !strcmp( Interface, GUAYADEQUE_PROPERTIES_INTERFACE ) )
        {
            DBusError error;
            dbus_error_init( &error );

            const char *    QueryIface;
            const char *    QueryProperty;

            dbus_message_get_args( msg->GetMessage(), &error,
                  DBUS_TYPE_STRING, &QueryIface,
                  DBUS_TYPE_STRING, &QueryProperty,
                  DBUS_TYPE_INVALID );

            guLogMessage( wxT( "Asking for '%s' -> '%s' parameter" ), wxString( QueryIface, wxConvUTF8 ).c_str(), wxString( QueryProperty, wxConvUTF8 ).c_str() );

            if( !strcmp( Path, "/org/mpris/MediaPlayer2" ) )
            {
                if( !strcmp( Member, "Get" ) )
                {
                    if( dbus_error_is_set( &error ) )
                    {
                        guLogMessage( wxT( "Could not read the '"GUAYADEQUE_PROPERTIES_INTERFACE "' parameter : %s" ), wxString( error.message, wxConvUTF8 ).c_str() );
                        dbus_error_free( &error );
                    }
                    else
                    {
                        if( !strcmp( QueryIface, "org.mpris.MediaPlayer2" ) )
                        {
                            if( !strcmp( QueryProperty, "CanQuit" ) )
                            {
                                bool ReplyVal = true;
                                if( AddVariant( reply->GetMessage(), DBUS_TYPE_BOOLEAN, &ReplyVal ) )
                                {
                                    Send( reply );
                                    Flush();
                                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                }
                            }
                            else if( !strcmp( QueryProperty, "CanRaise" ) )
                            {
                                bool ReplyVal = true;
                                if( AddVariant( reply->GetMessage(), DBUS_TYPE_BOOLEAN, &ReplyVal ) )
                                {
                                    Send( reply );
                                    Flush();
                                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                }
                            }
                            else if( !strcmp( QueryProperty, "HasTrackList" ) )
                            {
                                bool ReplyVal = true;
                                if( AddVariant( reply->GetMessage(), DBUS_TYPE_BOOLEAN, &ReplyVal ) )
                                {
                                    Send( reply );
                                    Flush();
                                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                }
                            }
                            else if( !strcmp( QueryProperty, "Identity" ) )
                            {
                                const char * AppName = "Guayadeque " ID_GUAYADEQUE_VERSION;
                                if( AddVariant( reply->GetMessage(), DBUS_TYPE_STRING, &AppName ) )
                                {
                                    Send( reply );
                                    Flush();
                                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                }
                            }
                            else if( !strcmp( QueryProperty, "DesktopEntry" ) )
                            {
                                const char * DesktopPath = "/usr/share/applications/guayadeque.desktop";
                                if( wxFileExists( wxString( DesktopPath, wxConvUTF8 ) ) )
                                {
                                    if( AddVariant( reply->GetMessage(), DBUS_TYPE_STRING, &DesktopPath ) )
                                    {
                                        Send( reply );
                                        Flush();
                                        RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                    }
                                }
                                else
                                {
                                    const char * DesktopPath = "/usr/local/share/applications/guayadeque.desktop";
                                    if( wxFileExists( wxString( DesktopPath, wxConvUTF8 ) ) )
                                    {
                                        if( AddVariant( reply->GetMessage(), DBUS_TYPE_STRING, &DesktopPath ) )
                                        {
                                            Send( reply );
                                            Flush();
                                            RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                        }
                                    }
                                }
                            }
                            else if( !strcmp( QueryProperty, "SupportedUriSchemes" ) )
                            {
                                const char * SupportedUriSchemes[] = { "file", "http", "smb", "sftp", NULL };
                                if( AddVariant( reply->GetMessage(), DBUS_TYPE_ARRAY, &SupportedUriSchemes ) )
                                {
                                    Send( reply );
                                    Flush();
                                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                }
                            }
                            else if( !strcmp( QueryProperty, "SupportedMimeTypes" ) )
                            {
                                const char * SupportedMimeTypes[] = {
                                   "application/ogg",
                                   "application/x-ogg",
                                   "application/x-ogm-audio",
                                   "audio/aac",
                                   "audio/ape",
                                   "audio/mp4",
                                   "audio/mpc",
                                   "audio/mpeg",
                                   "audio/mpegurl",
                                   "audio/ogg",
                                   "audio/vnd.rn-realaudio",
                                   "audio/vorbis",
                                   "audio/x-flac",
                                   "audio/x-mp3",
                                   "audio/x-mpeg",
                                   "audio/x-mpegurl",
                                   "audio/x-ms-wma",
                                   "audio/x-musepack",
                                   "audio/x-oggflac",
                                   "audio/x-pn-realaudio",
                                   "audio/x-scpls",
                                   "audio/x-speex",
                                   "audio/x-vorbis",
                                   "audio/x-vorbis+ogg",
                                   "audio/x-wav",
                                   "video/x-ms-asf",
                                   "x-content/audio-player",
                                   NULL };
                                if( AddVariant( reply->GetMessage(), DBUS_TYPE_ARRAY, &SupportedMimeTypes ) )
                                {
                                    Send( reply );
                                    Flush();
                                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                }
                            }
                        }
                        else if( !strcmp( QueryIface, "org.mpris.MediaPlayer2.Player" ) )
                        {
                            if( !strcmp( QueryProperty, "PlaybackStatus" ) )
                            {
                                const char * PlaybackStatus;
                                guMediaState State = m_PlayerPanel->GetState();
                                if( State == guMEDIASTATE_STOPPED )
                                    PlaybackStatus = "Stopped";
                                else if( State == guMEDIASTATE_PAUSED )
                                    PlaybackStatus = "Paused";
                                else //if( State == guMEDIASTATE_PLAYING )
                                    PlaybackStatus = "Playing";

                                if( AddVariant( reply->GetMessage(), DBUS_TYPE_STRING, &PlaybackStatus ) )
                                {
                                    Send( reply );
                                    Flush();
                                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                }
                            }
                            else if( !strcmp( QueryProperty, "LoopStatus" ) )
                            {
                                const char * LoopStatus;
                                int PlayLoop = m_PlayerPanel->GetPlayLoop();
                                if( PlayLoop == guPLAYER_PLAYLOOP_NONE )
                                    LoopStatus = "None";
                                else if( PlayLoop == guPLAYER_PLAYLOOP_TRACK )
                                    LoopStatus = "Track";
                                else //if( PlayLoop == guPLAYER_PLAYLOOP_PLAYLIST )
                                    LoopStatus = "Playlist";

                                if( AddVariant( reply->GetMessage(), DBUS_TYPE_STRING, &LoopStatus ) )
                                {
                                    Send( reply );
                                    Flush();
                                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                }
                            }
                            else if( !strcmp( QueryProperty, "Rate" ) )
                            {
                                double Rate = 1.0;
                                if( AddVariant( reply->GetMessage(), DBUS_TYPE_DOUBLE, &Rate ) )
                                {
                                    Send( reply );
                                    Flush();
                                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                }
                            }
                            else if( !strcmp( QueryProperty, "Shuffle" ) )
                            {
                                bool Shuffle = false;
                                if( AddVariant( reply->GetMessage(), DBUS_TYPE_BOOLEAN, &Shuffle ) )
                                {
                                    Send( reply );
                                    Flush();
                                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                }
                            }
                            else if( !strcmp( QueryProperty, "Metadata" ) )
                            {
                                const guCurrentTrack * CurTrack = m_PlayerPanel->GetCurrentTrack();

                                FillMetadataArgs( reply, CurTrack, m_PlayerPanel->GetCurrentItem() );

                                Send( reply );
                                Flush();
                                RetVal = DBUS_HANDLER_RESULT_HANDLED;
                            }
                            else if( !strcmp( QueryProperty, "Volume" ) )
                            {
                                double CurVolume = m_PlayerPanel->GetVolume() / 100;
                                if( AddVariant( reply->GetMessage(), DBUS_TYPE_DOUBLE, &CurVolume ) )
                                {
                                    Send( reply );
                                    Flush();
                                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                }
                            }
                            else if( !strcmp( QueryProperty, "Position" ) )
                            {
                                double CurPosition = m_PlayerPanel->GetPosition() * 1000;
                                if( AddVariant( reply->GetMessage(), DBUS_TYPE_DOUBLE, &CurPosition ) )
                                {
                                    Send( reply );
                                    Flush();
                                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                }
                            }
                            else if( !strcmp( QueryProperty, "MinimumRate" ) )
                            {
                                double Rate = 1.0;
                                if( AddVariant( reply->GetMessage(), DBUS_TYPE_DOUBLE, &Rate ) )
                                {
                                    Send( reply );
                                    Flush();
                                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                }
                            }
                            else if( !strcmp( QueryProperty, "MaximumRate" ) )
                            {
                                double Rate = 1.0;
                                if( AddVariant( reply->GetMessage(), DBUS_TYPE_DOUBLE, &Rate ) )
                                {
                                    Send( reply );
                                    Flush();
                                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                }
                            }
                            else if( !strcmp( QueryProperty, "CanGoNext" ) )
                            {
                                bool CanGoNext = m_PlayerPanel->GetCaps() & MPRIS_CAPS_CAN_GO_NEXT;
                                if( AddVariant( reply->GetMessage(), DBUS_TYPE_BOOLEAN, &CanGoNext ) )
                                {
                                    Send( reply );
                                    Flush();
                                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                }
                            }
                            else if( !strcmp( QueryProperty, "CanGoPrevious" ) )
                            {
                                bool CanGoPrev = m_PlayerPanel->GetCaps() & MPRIS_CAPS_CAN_GO_PREV;
                                if( AddVariant( reply->GetMessage(), DBUS_TYPE_BOOLEAN, &CanGoPrev ) )
                                {
                                    Send( reply );
                                    Flush();
                                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                }
                            }
                            else if( !strcmp( QueryProperty, "CanPlay" ) )
                            {
                                bool CanPlay = m_PlayerPanel->GetCaps() & MPRIS_CAPS_CAN_PLAY;
                                if( AddVariant( reply->GetMessage(), DBUS_TYPE_BOOLEAN, &CanPlay ) )
                                {
                                    Send( reply );
                                    Flush();
                                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                }
                            }
                            else if( !strcmp( QueryProperty, "CanPause" ) )
                            {
                                bool CanPause = m_PlayerPanel->GetCaps() & MPRIS_CAPS_CAN_PAUSE;
                                if( AddVariant( reply->GetMessage(), DBUS_TYPE_BOOLEAN, &CanPause ) )
                                {
                                    Send( reply );
                                    Flush();
                                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                }
                            }
                            else if( !strcmp( QueryProperty, "CanSeek" ) )
                            {
                                bool CanSeek = m_PlayerPanel->GetCaps() & MPRIS_CAPS_CAN_SEEK;
                                if( AddVariant( reply->GetMessage(), DBUS_TYPE_BOOLEAN, &CanSeek ) )
                                {
                                    Send( reply );
                                    Flush();
                                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                }
                            }
                            else if( !strcmp( QueryProperty, "CanControl" ) )
                            {
                                bool CanControl = true;
                                if( AddVariant( reply->GetMessage(), DBUS_TYPE_BOOLEAN, &CanControl ) )
                                {
                                    Send( reply );
                                    Flush();
                                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                }
                            }
                        }
                        else if( !strcmp( QueryIface, "org.mpris.MediaPlayer2.TrackList" ) )
                        {
                            if( !strcmp( QueryProperty, "Tracks" ) )
                            {
                            }
                            else if( !strcmp( QueryProperty, "CanEditTracks" ) )
                            {
                            }
                        }
                    }
                }
                else if( !strcmp( Member, "Set" ) )
                {

                }
            }
        }
        else if( !strcmp( Interface, GUAYADEQUE_MPRIS2_INTERFACE_ROOT ) )
        {
            if( !strcmp( Path, GUAYADEQUE_MPRIS2_OBJECT_PATH ) )
            {
                if( !strcmp( Member, "Raise" ) )
                {
                    guMainFrame * MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
                    if( !MainFrame->IsShown() )
                    {
                        MainFrame->Show( true );
                        if( MainFrame->IsIconized() )
                            MainFrame->Iconize( false );
                    }
                    Send( reply );
                    Flush();
                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                }
                else if( !strcmp( Member, "Quit" ) )
                {
                    guMainFrame * MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
                    wxCommandEvent QuitCmd( wxEVT_COMMAND_MENU_SELECTED, ID_MENU_QUIT );
                    wxPostEvent( MainFrame, QuitCmd );
                    Send( reply );
                    Flush();
                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                }
            }

        }
        else if( !strcmp( Interface, GUAYADEQUE_MPRIS2_INTERFACE_PLAYER ) )
        {
            if( !strcmp( Path, GUAYADEQUE_MPRIS2_OBJECT_PATH ) )
            {
                if( !strcmp( Member, "Next" ) )
                {
                    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_NEXTTRACK );
                    wxPostEvent( m_PlayerPanel, event );
                    Send( reply );
                    Flush();
                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                }
                else if( !strcmp( Member, "Previous" ) )
                {
                    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_PREVTRACK );
                    wxPostEvent( m_PlayerPanel, event );
                    Send( reply );
                    Flush();
                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                }
                else if( !strcmp( Member, "Pause" ) )
                {
                    if( m_PlayerPanel->GetState() == guMEDIASTATE_PLAYING )
                    {
                        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_PLAY );
                        wxPostEvent( m_PlayerPanel, event );
                    }
                    Send( reply );
                    Flush();
                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                }
                else if( !strcmp( Member, "PlayPause" ) )
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
                    if( m_PlayerPanel->GetState() != guMEDIASTATE_PLAYING )
                    {
                        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_PLAY );
                        wxPostEvent( m_PlayerPanel, event );
                    }
                    Send( reply );
                    Flush();
                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                }
                else if( !strcmp( Member, "Seek" ) )
                {
                    DBusError error;
                    dbus_error_init( &error );

                    dbus_int64_t Position;
                    dbus_message_get_args( msg->GetMessage(), &error, DBUS_TYPE_INT64, &Position, DBUS_TYPE_INVALID );

                    if( dbus_error_is_set( &error ) )
                    {
                        guLogMessage( wxT( "Could not read the Position parameter : %s" ), wxString( error.message, wxConvUTF8 ).c_str() );
                        dbus_error_free( &error );
                    }
                    else
                    {
                        m_PlayerPanel->SetPosition( Position / 1000 );

                        Send( reply );
                        Flush();
                        RetVal = DBUS_HANDLER_RESULT_HANDLED;
                    }
                }
                else if( !strcmp( Member, "SetPosition" ) )
                {
                    DBusError error;
                    dbus_error_init( &error );

                    const char * TrackId;
                    dbus_int64_t Position;
                    dbus_message_get_args( msg->GetMessage(), &error,
                            DBUS_TYPE_STRING, &TrackId,
                            DBUS_TYPE_INT64, &Position,
                            DBUS_TYPE_INVALID );

                    if( dbus_error_is_set( &error ) )
                    {
                        guLogMessage( wxT( "Could not read the Position parameter : %s" ), wxString( error.message, wxConvUTF8 ).c_str() );
                        dbus_error_free( &error );
                    }
                    else
                    {
                        if( wxString::Format( wxT( "/org/mpris/MediaPlayer2/Track/%u" ), m_PlayerPanel->GetCurrentItem() ) == wxString( TrackId, wxConvUTF8 ) )
                        {
                            m_PlayerPanel->SetPosition( Position / 1000 );

                            Send( reply );
                            Flush();
                            RetVal = DBUS_HANDLER_RESULT_HANDLED;
                        }
                    }
                }
                else if( !strcmp( Member, "OpenUri" ) )
                {
                }
            }
        }
    }

    return RetVal;
}

// -------------------------------------------------------------------------------- //
