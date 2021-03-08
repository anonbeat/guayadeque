// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2020 J.Rios anonbeat@gmail.com
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
//    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
//    Boston, MA 02110-1301 USA.
//
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#include "mpris2.h"

#include "EventCommandIds.h"
#include "mpris.h"
#include "Utils.h"
#include "MainFrame.h"

namespace Guayadeque {

// See http://www.mpris.org/2.0/spec/index.html
#define GUAYADEQUE_MPRIS2_SERVICE_NAME          "org.mpris.MediaPlayer2.guayadeque"
#define GUAYADEQUE_MPRIS2_OBJECT_PATH           "/org/mpris/MediaPlayer2"
#define GUAYADEQUE_MPRIS2_PLAYER_PATH           "/org/mpris/MediaPlayer2/Player"
#define GUAYADEQUE_PROPERTIES_INTERFACE         "org.freedesktop.DBus.Properties"

#define GUAYADEQUE_MPRIS2_INTERFACE_ROOT        "org.mpris.MediaPlayer2"
#define GUAYADEQUE_MPRIS2_INTERFACE_PLAYER      "org.mpris.MediaPlayer2.Player"
#define GUAYADEQUE_MPRIS2_INTERFACE_TRACKLIST   "org.mpris.MediaPlayer2.TrackList"
#define GUAYADEQUE_MPRIS2_INTERFACE_PLAYLISTS   "org.mpris.MediaPlayer2.Playlists"

const char * GUAYADEQUE_SUPPORTED_MIME_TYPES[] = {
   "application/ogg",
   "application/vnd.rn-realmedia",
   "application/x-3gp",
   "application/x-gst-av-dsf",
   "application/x-gst-av-iff",
   "application/x-ogg",
   "application/x-ogm-audio",
   "audio/aac",
   "audio/ape",
   "audio/midi",
   "audio/mp4",
   "audio/mpc",
   "audio/mpeg",
   "audio/mpegurl",
   "audio/ogg",
   "audio/vnd.rn-realaudio",
   "audio/vorbis",
   "audio/x-ac3",
   "audio/x-aiff",
   "audio/x-amr-wb-sh",
   "audio/x-au",
   "audio/x-flac",
   "audio/x-dts",
   "audio/x-m4a",
   "audio/x-matroska",
   "audio/x-mp3",
   "audio/x-mpeg",
   "audio/x-mpegurl",
   "audio/x-ms-wma",
   "audio/x-musepack",
   "audio/x-oggflac",
   "audio/x-pn-realaudio",
   "audio/x-scpls",
   "audio/x-speex",
   "audio/x-svx",
   "audio/x-voc",
   "audio/x-vorbis",
   "audio/x-vorbis+ogg",
   "audio/x-wav",
   "video/quicktime",
   "video/mpeg",
   "video/webm",
   "video/x-flv",
   "video/x-ms-asf",
   "video/x-msvideo",
   "x-content/audio-player",
   NULL };

const char * guMPRIS2_INTROSPECTION_XML =
    "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n"
    "\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n"
	"<node>\n"
	"  <interface name='org.mpris.MediaPlayer2'>\n"
	"    <method name='Raise'/>\n"
	"    <method name='Quit'/>\n"
	"    <property name='CanQuit' type='b' access='read'/>\n"
	"    <property name='Fullscreen' type='b' access='readwrite'/>\n"
	"    <property name='CanSetFullscreen' type='b' access='read'/>\n"
	"    <property name='CanRaise' type='b' access='read'/>\n"
	"    <property name='HasTrackList' type='b' access='read'/>\n"
	"    <property name='Identity' type='s' access='read'/>\n"
	"    <property name='DesktopEntry' type='s' access='read'/>\n"
	"    <property name='SupportedUriSchemes' type='as' access='read'/>\n"
	"    <property name='SupportedMimeTypes' type='as' access='read'/>\n"
	"  </interface>\n"
	"  <interface name='org.mpris.MediaPlayer2.Player'>\n"
	"    <method name='Next'/>\n"
	"    <method name='NextAlbum'/>\n" // Non-standard
	"    <method name='Previous'/>\n"
	"    <method name='PreviousAlbum'/>\n" // Non-standard
	"    <method name='Pause'/>\n"
	"    <method name='PlayPause'/>\n"
	"    <method name='Stop'/>\n"
	"    <method name='Play'/>\n"
	"    <method name='Seek'>\n"
	"      <arg direction='in' name='Offset' type='x'/>\n"
	"    </method>\n"
	"    <method name='SetPosition'>\n"
	"      <arg direction='in' name='TrackId' type='o'/>\n"
	"      <arg direction='in' name='Position' type='x'/>\n"
	"    </method>\n"
	"    <method name='OpenUri'>\n"
	"      <arg direction='in' name='Uri' type='s'/>\n"
	"    </method>\n"
	"    <signal name='Seeked'>\n"
	"      <arg name='Position' type='x'/>\n"
	"    </signal>\n"
	"    <property name='PlaybackStatus' type='s' access='read'/>\n"
	"    <property name='LoopStatus' type='s' access='readwrite'/>\n"
	"    <property name='Rate' type='d' access='readwrite'/>\n"
	"    <property name='Shuffle' type='b' access='readwrite'/>\n"
	"    <property name='Metadata' type='a{sv}' access='read'/>\n"
	"    <property name='Volume' type='d' access='readwrite'/>\n"
	"    <property name='Position' type='x' access='read'/>\n"
	"    <property name='MinimumRate' type='d' access='read'/>\n"
	"    <property name='MaximumRate' type='d' access='read'/>\n"
	"    <property name='CanGoNext' type='b' access='read'/>\n"
	"    <property name='CanGoPrevious' type='b' access='read'/>\n"
	"    <property name='CanPlay' type='b' access='read'/>\n"
	"    <property name='CanPause' type='b' access='read'/>\n"
	"    <property name='CanSeek' type='b' access='read'/>\n"
	"    <property name='CanControl' type='b' access='read'/>\n"
	"  </interface>\n"
	"  <interface name='org.mpris.MediaPlayer2.TrackList'>\n"
	"    <method name='GetTracksMetadata'>\n"
	"      <arg direction='in' name='TrackIds' type='ao'/>\n"
	"      <arg direction='out' name='Metadata' type='aa{sv}'/>\n"
	"    </method>\n"
	"    <method name='AddTrack'>\n"
	"      <arg direction='in' name='Uri' type='s'/>\n"
	"      <arg direction='in' name='AfterTrack' type='o'/>\n"
	"      <arg direction='in' name='SetAsCurrent' type='b'/>\n"
	"    </method>\n"
	"    <method name='RemoveTrack'>\n"
	"      <arg direction='in' name='TrackId' type='o'/>\n"
	"    </method>\n"
	"    <method name='GoTo'>\n"
	"      <arg direction='in' name='TrackId' type='o'/>\n"
	"    </method>\n"
	"    <signal name='TrackListReplaced'>\n"
	"      <arg name='Tracks' type='ao'/>\n"
	"      <arg name='CurrentTrack' type='o'/>\n"
	"    </signal>\n"
	"    <signal name='TrackAdded'>\n"
	"      <arg name='Metadata' type='a{sv}'/>\n"
	"      <arg name='AfterTrack' type='o'/>\n"
	"    </signal>\n"
	"    <signal name='TrackRemoved'>\n"
	"      <arg name='TrackId' type='o'/>\n"
	"    </signal>\n"
	"    <signal name='TrackMetadataChanged'>\n"
	"      <arg name='TrackId' type='o'/>\n"
	"      <arg name='Metadata' type='a{sv}'/>\n"
	"    </signal>\n"
	"    <property name='Tracks' type='ao' access='read'/>\n"
	"    <property name='CanEditTracks' type='b' access='read'/>\n"
	"  </interface>\n"
	"  <interface name='org.mpris.MediaPlayer2.Playlists'>\n"
	"    <method name='ActivatePlaylist'>\n"
	"      <arg direction='in' name='PlaylistId' type='o'/>\n"
	"    </method>\n"
	"    <method name='GetPlaylists'>\n"
	"      <arg direction='in' name='Index' type='u'/>\n"
	"      <arg direction='in' name='MaxCount' type='u'/>\n"
	"      <arg direction='in' name='Order' type='s'/>\n"
	"      <arg direction='in' name='ReverseOrder' type='b'/>\n"
	"      <arg direction='out' name='Playlists' type='a(oss)'/>\n"
	"    </method>\n"
	"    <signal name='PlaylistChanged'>\n"
	"      <arg name='Playlist' type='b(oss)'/>\n"
	"    </signal>\n"
	"    <property name='PlaylistCount' type='u' access='read'/>\n"
	"    <property name='Orderings' type='as' access='read'/>\n"
	"    <property name='ActivePlaylist' type='b(oss)' access='read'/>\n"
	"  </interface>\n"
	"</node>\n";

guMPRIS2 * guMPRIS2::m_MPRIS2 = NULL;

// -------------------------------------------------------------------------------- //
guMPRIS2::guMPRIS2( guDBusServer * server, guPlayerPanel * playerpanel, guDbLibrary * db ) : guDBusClient( server )
{
    m_PlayerPanel = playerpanel;
    m_Db = db;

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
static void FillMetadataDetails( DBusMessageIter * Iter, const char * name, const char ** value )
{
    const char **Str;
    DBusMessageIter dict_entry, variant, array;
    if( value )
    {
        dbus_message_iter_open_container( Iter, DBUS_TYPE_DICT_ENTRY, NULL, &dict_entry );

        dbus_message_iter_append_basic( &dict_entry, DBUS_TYPE_STRING, &name );

        dbus_message_iter_open_container( &dict_entry, DBUS_TYPE_VARIANT, "as", &variant );

        dbus_message_iter_open_container( &variant, DBUS_TYPE_ARRAY, "s", &array );

        for( Str = value; * Str; Str++ )
            dbus_message_iter_append_basic( &array, DBUS_TYPE_STRING, Str );

        dbus_message_iter_close_container( &variant, &array );

        dbus_message_iter_close_container( &dict_entry, &variant );

        dbus_message_iter_close_container( Iter, &dict_entry );
    }
}

// -------------------------------------------------------------------------------- //
static void FillMetadataDetails( DBusMessageIter * Iter, const char * name, const int value )
{
    DBusMessageIter dict_entry, variant;
    if( name )
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
    if( name )
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
    if( name )
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
static void FillMetadataDetails( DBusMessageIter * Iter, const char * name, const dbus_bool_t value )
{
    DBusMessageIter dict_entry, variant;
    if( name )
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
static void FillMetadataDetails( DBusMessageIter * Iter, const char * name, const dbus_bool_t plstate,
                            const char * plpath, const char * plname, const char * plicon )
{
    DBusMessageIter dict_entry, vstruct, ostruct, plstruct;
    if( name )
    {
        guLogMessage( wxT( "playlist active %i" ), plstate );

        dbus_message_iter_open_container( Iter, DBUS_TYPE_DICT_ENTRY, NULL, &dict_entry );

        dbus_message_iter_append_basic( &dict_entry, DBUS_TYPE_STRING, &name );

        dbus_message_iter_open_container( &dict_entry, DBUS_TYPE_VARIANT, "(b(oss))", &vstruct );

        dbus_message_iter_open_container( &vstruct, DBUS_TYPE_STRUCT, NULL, &ostruct );

        dbus_message_iter_append_basic( &ostruct, DBUS_TYPE_BOOLEAN, &plstate );

        dbus_message_iter_open_container( &ostruct, DBUS_TYPE_STRUCT, NULL, &plstruct );

        dbus_message_iter_append_basic( &plstruct, DBUS_TYPE_OBJECT_PATH, &plpath );
        dbus_message_iter_append_basic( &plstruct, DBUS_TYPE_STRING, &plname );
        dbus_message_iter_append_basic( &plstruct, DBUS_TYPE_STRING, &plicon );

        dbus_message_iter_close_container( &ostruct, &plstruct );

        dbus_message_iter_close_container( &vstruct, &ostruct );

        dbus_message_iter_close_container( &dict_entry, &vstruct );

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

// -------------------------------------------------------------------------------- //
static void FillMetadataIter( DBusMessageIter * iter, const guCurrentTrack * curtrack, const int trackid )
{
    DBusMessageIter dict;

    dbus_message_iter_open_container( iter, DBUS_TYPE_ARRAY, "{sv}", &dict );

    FillMetadataDetails( &dict, "mpris:trackid", ( const char * ) wxString::Format( wxT( "/org/mpris/MediaPlayer2/Track/%u" ), trackid ).mb_str( wxConvUTF8 ) );

    if( curtrack && curtrack->m_Loaded )
    {
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
        FillMetadataDetails( &dict, "mpris:length", ( const gint64 ) gint64( curtrack->m_Length * 1000 ) );
        FillMetadataAsList( &dict, "xesan:genre", ( const char * ) curtrack->m_GenreName.mb_str( wxConvUTF8 ) );
        if( curtrack->m_Rating >= 0 )
            FillMetadataDetails( &dict, "xesam:userRating", ( const double ) double( curtrack->m_Rating * 0.2 ) );

        if( curtrack->m_Year )
        {
            FillMetadataDetails( &dict, "xesam:contentCreated", ( const char * ) wxString::Format( wxT( "%4d-%02d-%02dT%02d:%02d:%02dZ" ), curtrack->m_Year, 1, 1, 0, 0, 0 ).mb_str( wxConvUTF8 ) );
        }
        FillMetadataDetails( &dict, "xesam:useCount", ( const int ) curtrack->m_PlayCount );

        if( curtrack->m_CoverType == GU_SONGCOVER_ID3TAG )
        {
            wxString TempFile = wxFileName::GetTempDir() + wxT( "/" ) + guTEMPORARY_COVER_FILENAME + wxT( "1.png" );
            if( !wxFileExists( TempFile ) )
            {
                TempFile.RemoveLast( 5 );
                TempFile.Append( wxT( "2.png" ) );
            }
            if( wxFileExists( TempFile ) )
                FillMetadataDetails( &dict, "mpris:artUrl", ( const char * ) ( wxT( "file://" ) + TempFile ).mb_str( wxConvUTF8 ) );
        }
        else if( !curtrack->m_CoverPath.IsEmpty() )
        {
            FillMetadataDetails( &dict, "mpris:artUrl", ( const char * ) ( wxT( "file://" ) + curtrack->m_CoverPath ).mb_str( wxConvUTF8 ) );
        }

        if( curtrack->m_Bitrate )
            FillMetadataDetails( &dict, "xesam:audioBitrate", ( const int ) curtrack->m_Bitrate * 1000 );
    }

    dbus_message_iter_close_container( iter, &dict );
}

// -------------------------------------------------------------------------------- //
static void FillMetadataDetails( DBusMessageIter * Iter, const char * name, const guCurrentTrack * curtrack, const int trackid )
{
    DBusMessageIter dict_entry, variant;
    if( name && strlen( name ) )
    {
        dbus_message_iter_open_container( Iter, DBUS_TYPE_DICT_ENTRY, NULL, &dict_entry );

        dbus_message_iter_append_basic( &dict_entry, DBUS_TYPE_STRING, &name );

        dbus_message_iter_open_container( &dict_entry, DBUS_TYPE_VARIANT, "a{sv}", &variant );

        FillMetadataIter( &variant, curtrack, trackid );

        dbus_message_iter_close_container( &dict_entry, &variant );

        dbus_message_iter_close_container( Iter, &dict_entry );
    }
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
static bool GetVariant( DBusMessage * msg, const int type, void * data )
{
	DBusMessageIter iter, variant;

	if( !dbus_message_iter_init( msg, &iter ) )
	{
        guLogMessage( wxT( "GetVariant called without arguments" ) );
        return false;
	}
	do {
	    if( dbus_message_iter_get_arg_type( &iter ) == DBUS_TYPE_VARIANT )
	    {
	        guLogMessage( wxT( "Found the Variant variable") );
            dbus_message_iter_recurse( &iter, &variant );
            if( dbus_message_iter_get_arg_type( &variant ) == type )
            {
                dbus_message_iter_get_basic( &variant, data );
                return true;
            }
	    }
	} while( dbus_message_iter_next( &iter ) );

	return false;
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
        int PlayLoop = m_PlayerPanel->GetPlayMode();
        if( PlayLoop == guPLAYER_PLAYMODE_REPEAT_TRACK )
            LoopStatus = "Track";
        else if( PlayLoop == guPLAYER_PLAYMODE_REPEAT_PLAYLIST )
            LoopStatus = "Playlist";
        else //if( PlayLoop == guPLAYER_PLAYMODE_NONE )
            LoopStatus = "None";

        FillMetadataDetails( &dict, "LoopStatus", LoopStatus );

        int Caps = m_PlayerPanel->GetCaps();

        dbus_bool_t CanGoNext = ( Caps & MPRIS_CAPS_CAN_GO_NEXT ) > 0;
        FillMetadataDetails( &dict, "CanGoNext", CanGoNext );

        dbus_bool_t CanGoPrev = ( Caps & MPRIS_CAPS_CAN_GO_PREV ) > 0;
        FillMetadataDetails( &dict, "CanGoPrevious", CanGoPrev );

        dbus_bool_t CanPlay = ( Caps & MPRIS_CAPS_CAN_PLAY ) > 0;
        FillMetadataDetails( &dict, "CanPlay", CanPlay );

        dbus_bool_t CanPause = ( Caps & MPRIS_CAPS_CAN_PAUSE ) > 0;
        FillMetadataDetails( &dict, "CanPause", CanPause );

        dbus_bool_t CanSeek = ( Caps & MPRIS_CAPS_CAN_SEEK ) > 0;
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
        int PlayLoop = m_PlayerPanel->GetPlayMode();
        if( PlayLoop == guPLAYER_PLAYMODE_REPEAT_TRACK )
            LoopStatus = "Track";
        else if( PlayLoop == guPLAYER_PLAYMODE_REPEAT_PLAYLIST )
            LoopStatus = "Playlist";
        else //if( PlayLoop == guPLAYER_PLAYMODE_NONE )
            LoopStatus = "None";

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

        dbus_bool_t CanGoNext = ( Caps & MPRIS_CAPS_CAN_GO_NEXT ) > 0;
        FillMetadataDetails( &dict, "CanGoNext", CanGoNext );

        dbus_bool_t CanGoPrev = ( Caps & MPRIS_CAPS_CAN_GO_PREV ) > 0;
        FillMetadataDetails( &dict, "CanGoPrevious", CanGoPrev );

        dbus_bool_t CanPlay = ( Caps & MPRIS_CAPS_CAN_PLAY ) > 0;
        FillMetadataDetails( &dict, "CanPlay", CanPlay );

        dbus_bool_t CanPause = ( Caps & MPRIS_CAPS_CAN_PAUSE ) > 0;
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
void guMPRIS2::OnPlayerSeeked( const unsigned int newpos )
{
    guDBusSignal * signal = new guDBusSignal( GUAYADEQUE_MPRIS2_OBJECT_PATH, GUAYADEQUE_MPRIS2_INTERFACE_PLAYER, "Seeked" );
    if( signal )
    {
        DBusMessageIter args;

        dbus_message_iter_init_append( signal->GetMessage(), &args );

        gint64 Position = newpos * 1000;
        dbus_message_iter_append_basic( &args, DBUS_TYPE_INT64, &Position );

        Send( signal );
        Flush();
        delete signal;
    }
    else
    {
        guLogError( wxT( "Could not create Seeked signal object" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guMPRIS2::OnTrackListChange( void )
{
}

// -------------------------------------------------------------------------------- //
void guMPRIS2::OnFullscreenChanged( void )
{
    guDBusSignal * signal = new guDBusSignal( GUAYADEQUE_MPRIS2_OBJECT_PATH, GUAYADEQUE_PROPERTIES_INTERFACE, "PropertiesChanged" );
    if( signal )
    {
        DBusMessageIter dict;
        DBusMessageIter args;

        dbus_message_iter_init_append( signal->GetMessage(), &args );

        const char * Interface = GUAYADEQUE_MPRIS2_INTERFACE_ROOT;
        dbus_message_iter_append_basic( &args, DBUS_TYPE_STRING, &Interface );

        dbus_message_iter_open_container( &args, DBUS_TYPE_ARRAY, "{sv}", &dict );

        guMainFrame * MainFrame = guMainFrame::GetMainFrame();
        dbus_bool_t FullScreen = MainFrame->IsFullScreen();
        FillMetadataDetails( &dict, "Fullscreen", FullScreen );

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
void inline IterAppendBasic( DBusMessageIter * iter, const int type, const char * value )
{
    dbus_message_iter_append_basic( iter, type, &value );
}

// -------------------------------------------------------------------------------- //
void AppendPlaylist( DBusMessageIter * iter, guListItem &playlist )
{
    DBusMessageIter plstruct;
    const char * plicon = "";

    dbus_message_iter_open_container( iter, DBUS_TYPE_STRUCT, NULL, &plstruct );

    IterAppendBasic( &plstruct, DBUS_TYPE_OBJECT_PATH,
        wxString::Format( wxT( "/org/mpris/MediaPlayer2/guayadeque/Playlist/%i" ), playlist.m_Id ).mb_str( wxConvUTF8 ) );
    IterAppendBasic( &plstruct, DBUS_TYPE_STRING, playlist.m_Name.mb_str( wxConvUTF8 ) );
    IterAppendBasic( &plstruct, DBUS_TYPE_STRING, plicon );

    dbus_message_iter_close_container( iter, &plstruct );

}

// -------------------------------------------------------------------------------- //
bool guMPRIS2::GetPlaylists( DBusMessage * msg, const dbus_int32_t start, const dbus_int32_t maxcount,
                     const char * order, const dbus_bool_t reverseorder )
{
    DBusMessageIter args, arrentry;
    guListItems Playlists;
    m_Db->GetPlayLists( Playlists );

    dbus_message_iter_init_append( msg, &args );

    dbus_message_iter_open_container( &args, DBUS_TYPE_ARRAY, "(oss)", &arrentry );

    int Index;
    int Count = wxMin( start + maxcount, ( int ) Playlists.Count() );

    for( Index = start; Index < Count; Index++ )
    {
        AppendPlaylist( &arrentry, Playlists[ Index ] );
    }

    dbus_message_iter_close_container( &args, &arrentry );

    return true;
}

// -------------------------------------------------------------------------------- //
DBusHandlerResult guMPRIS2::HandleMessages( guDBusMessage * msg, guDBusMessage * reply )
{
    //
    DBusHandlerResult RetVal = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    const char *    Interface = msg->GetInterface();
    const char *    Member = msg->GetMember();
    int             Type = msg->GetType();
    const char *    Path = msg->GetPath();
//    int             Serial = msg->GetSerial();
//    int             RSerial = msg->GetReplySerial();

    // Show the details of the msg
//    const char *    Dest = msg->GetDestination();
//    guLogMessage( wxT( "==MPRIS2========================" ) );
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

        if( !strcmp( Interface, "org.freedesktop.DBus.Introspectable" ) )
        {
            if( !strcmp( Member, "Introspect" ) )
            {
                const char *    Dest = msg->GetDestination();
                if( Dest && !strcmp( Dest, GUAYADEQUE_MPRIS2_SERVICE_NAME ) )
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
        }
        else if( !strcmp( Interface, GUAYADEQUE_PROPERTIES_INTERFACE ) )
        {
            if( !strcmp( Path, "/org/mpris/MediaPlayer2" ) )
            {
                DBusError error;
                dbus_error_init( &error );

                if( !strcmp( Member, "GetAll" ) )
                {
                    const char *    QueryIface;

                    dbus_message_get_args( msg->GetMessage(), &error,
                          DBUS_TYPE_STRING, &QueryIface,
                          DBUS_TYPE_INVALID );

//                    guLogMessage( wxT( "QIface : %s" ), wxString::FromAscii( QueryIface ).c_str() );

                    if( dbus_error_is_set( &error ) )
                    {
                        guLogMessage( wxT( "Could not read the parameter : %s" ), wxString( error.message, wxConvUTF8 ).c_str() );
                        dbus_error_free( &error );
                    }
                    else
                    {
                        DBusMessageIter args;
                        DBusMessageIter dict;

                        if( !strcmp( QueryIface, "org.mpris.MediaPlayer2" ) )
                        {
                            dbus_bool_t ReplyVal = true;
                            guMainFrame * MainFrame = guMainFrame::GetMainFrame();
                            dbus_bool_t FullScreen = MainFrame->IsFullScreen();

                            dbus_message_iter_init_append( reply->GetMessage(), &args );

                            dbus_message_iter_open_container( &args, DBUS_TYPE_ARRAY, "{sv}", &dict );

                            FillMetadataDetails( &dict, "CanQuit", ReplyVal );
                            FillMetadataDetails( &dict, "Fullscreen", FullScreen );
                            FillMetadataDetails( &dict, "CanSetFullscreen", ReplyVal );
                            FillMetadataDetails( &dict, "CanRaise", ReplyVal );
                            FillMetadataDetails( &dict, "HasTrackList", ReplyVal );
                            const char * AppName = "Guayadeque Music Player";
                            FillMetadataDetails( &dict, "Identity", AppName );
                            const char * DesktopPath = "guayadeque";
                            FillMetadataDetails( &dict, "DesktopEntry", DesktopPath );
                            const char * SupportedUriSchemes[] = { "file", "http", "https", "smb", "sftp", "cdda", NULL };
                            FillMetadataDetails( &dict, "SupportedUriSchemes", SupportedUriSchemes );
                            FillMetadataDetails( &dict, "SupportedMimeTypes", GUAYADEQUE_SUPPORTED_MIME_TYPES );

                            dbus_message_iter_close_container( &args, &dict );

                            Send( reply );
                            Flush();
                            RetVal = DBUS_HANDLER_RESULT_HANDLED;
                        }
                        else if( !strcmp( QueryIface, "org.mpris.MediaPlayer2.Player" ) )
                        {
                            dbus_message_iter_init_append( reply->GetMessage(), &args );

                            dbus_message_iter_open_container( &args, DBUS_TYPE_ARRAY, "{sv}", &dict );

                            ////////////////////////////////////////////////////////////////////////
                            const char * PlaybackStatus;
                            guMediaState State = m_PlayerPanel->GetState();
                            if( State == guMEDIASTATE_STOPPED )
                                PlaybackStatus = "Stopped";
                            else if( State == guMEDIASTATE_PAUSED )
                                PlaybackStatus = "Paused";
                            else //if( State == guMEDIASTATE_PLAYING )
                                PlaybackStatus = "Playing";

                            FillMetadataDetails( &dict, "PlaybackStatus", PlaybackStatus );

                            ////////////////////////////////////////////////////////////////////////
                            const char * LoopStatus;
                            int PlayLoop = m_PlayerPanel->GetPlayMode();
                            if( PlayLoop == guPLAYER_PLAYMODE_REPEAT_TRACK )
                                LoopStatus = "Track";
                            else if( PlayLoop == guPLAYER_PLAYMODE_REPEAT_PLAYLIST )
                                LoopStatus = "Playlist";
                            else //if( PlayLoop == guPLAYER_PLAYMODE_NONE )
                                LoopStatus = "None";

                            FillMetadataDetails( &dict, "LoopStatus", LoopStatus );

                            ////////////////////////////////////////////////////////////////////////
                            double Rate = 1.0;
                            FillMetadataDetails( &dict, "Rate", Rate );


                            ////////////////////////////////////////////////////////////////////////
                            dbus_bool_t Shuffle = false;
                            FillMetadataDetails( &dict, "Shuffle", Shuffle );


                            ////////////////////////////////////////////////////////////////////////
                            const guCurrentTrack * CurTrack = m_PlayerPanel->GetCurrentTrack();
                            FillMetadataDetails( &dict, "Metadata", CurTrack, m_PlayerPanel->GetCurrentItem() );


                            ////////////////////////////////////////////////////////////////////////
                            double CurVolume = m_PlayerPanel->GetVolume() / 100;
                            FillMetadataDetails( &dict, "Volume", CurVolume );


                            ////////////////////////////////////////////////////////////////////////
                            gint64 CurPosition = m_PlayerPanel->GetPosition() * 1000;
                            FillMetadataDetails( &dict, "Position", CurPosition );


                            ////////////////////////////////////////////////////////////////////////
                            FillMetadataDetails( &dict, "MinimumRate", Rate );


                            ////////////////////////////////////////////////////////////////////////
                            FillMetadataDetails( &dict, "MaximumRate", Rate );


                            ////////////////////////////////////////////////////////////////////////
                            dbus_bool_t CanGoNext = ( m_PlayerPanel->GetCaps() & MPRIS_CAPS_CAN_GO_NEXT ) > 0;
                            FillMetadataDetails( &dict, "CanGoNext", CanGoNext );


                            ////////////////////////////////////////////////////////////////////////
                            dbus_bool_t CanGoPrev = ( m_PlayerPanel->GetCaps() & MPRIS_CAPS_CAN_GO_PREV ) > 0;
                            FillMetadataDetails( &dict, "CanGoPrevious", CanGoPrev );


                            ////////////////////////////////////////////////////////////////////////
                            dbus_bool_t CanPlay = ( m_PlayerPanel->GetCaps() & MPRIS_CAPS_CAN_PLAY ) > 0;
                            FillMetadataDetails( &dict, "CanPlay", CanPlay );


                            ////////////////////////////////////////////////////////////////////////
                            dbus_bool_t CanPause = ( m_PlayerPanel->GetCaps() & MPRIS_CAPS_CAN_PAUSE ) > 0;
                            FillMetadataDetails( &dict, "CanPause", CanPause );


                            ////////////////////////////////////////////////////////////////////////
                            dbus_bool_t CanSeek = ( m_PlayerPanel->GetCaps() & MPRIS_CAPS_CAN_SEEK ) > 0;
                            FillMetadataDetails( &dict, "CanSeek", CanSeek );


                            ////////////////////////////////////////////////////////////////////////
                            dbus_bool_t CanControl = true;
                            FillMetadataDetails( &dict, "CanControl", CanControl );

                            dbus_message_iter_close_container( &args, &dict );

                            Send( reply );
                            Flush();
                            RetVal = DBUS_HANDLER_RESULT_HANDLED;
                        }
                        else if( !strcmp( QueryIface, "org.mpris.MediaPlayer2.Tracklist" ) )
                        {
                        }
                        else if( !strcmp( QueryIface, "org.mpris.MediaPlayer2.Playlists" ) )
                        {
                            dbus_message_iter_init_append( reply->GetMessage(), &args );

                            dbus_message_iter_open_container( &args, DBUS_TYPE_ARRAY, "{sv}", &dict );

                            int PlaylistCount = m_Db->GetPlayListsCount();
                            FillMetadataDetails( &dict, "PlaylistCount", PlaylistCount );

                            const char * PlaylistOrders[] = {
                               "Default",
                               NULL };
                            FillMetadataDetails( &dict, "Orderings", PlaylistOrders );

                            dbus_bool_t plstate = false;
                            const char * plpath = "/org/mpris/MediaPlayer2/guayadeque/Playlist/0";
                            const char * plname = "";
                            const char * plicon = "";
                            FillMetadataDetails( &dict, "ActivePlaylist", plstate, plpath, plname, plicon );

                            dbus_message_iter_close_container( &args, &dict );

                            Send( reply );
                            Flush();
                            RetVal = DBUS_HANDLER_RESULT_HANDLED;
                        }
                    }
                }
                else
                {
                    const char *    QueryIface;
                    const char *    QueryProperty;

                    dbus_message_get_args( msg->GetMessage(), &error,
                          DBUS_TYPE_STRING, &QueryIface,
                          DBUS_TYPE_STRING, &QueryProperty,
                          DBUS_TYPE_INVALID );

                    //guLogMessage( wxT( "QIface : %s" ), wxString::FromAscii( QueryIface ).c_str() );
                    //guLogMessage( wxT( "QProp. : %s" ), wxString::FromAscii( QueryProperty ).c_str() );

                    if( dbus_error_is_set( &error ) )
                    {
                        guLogMessage( wxT( "Could not read the '" GUAYADEQUE_PROPERTIES_INTERFACE "' parameter : %s" ), wxString( error.message, wxConvUTF8 ).c_str() );
                        dbus_error_free( &error );
                    }
                    else
                    {
                        if( !strcmp( Member, "Get" ) )
                        {
                            if( !strcmp( QueryIface, "org.mpris.MediaPlayer2" ) )
                            {
                                if( !strcmp( QueryProperty, "CanQuit" ) )
                                {
                                    dbus_bool_t ReplyVal = true;
                                    if( AddVariant( reply->GetMessage(), DBUS_TYPE_BOOLEAN, &ReplyVal ) )
                                    {
                                        Send( reply );
                                        Flush();
                                        RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                    }
                                }
                                else if( !strcmp( QueryProperty, "Fullscreen" ) )
                                {
                                    guMainFrame * MainFrame = guMainFrame::GetMainFrame();
                                    dbus_bool_t FullScreen = MainFrame->IsFullScreen();
                                    if( AddVariant( reply->GetMessage(), DBUS_TYPE_BOOLEAN, &FullScreen ) )
                                    {
                                        Send( reply );
                                        Flush();
                                        RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                    }
                                }
                                else if( !strcmp( QueryProperty, "CanSetFullscreen" ) )
                                {
                                    dbus_bool_t ReplyVal = true;
                                    if( AddVariant( reply->GetMessage(), DBUS_TYPE_BOOLEAN, &ReplyVal ) )
                                    {
                                        Send( reply );
                                        Flush();
                                        RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                    }
                                }
                                else if( !strcmp( QueryProperty, "CanRaise" ) )
                                {
                                    dbus_bool_t ReplyVal = true;
                                    if( AddVariant( reply->GetMessage(), DBUS_TYPE_BOOLEAN, &ReplyVal ) )
                                    {
                                        Send( reply );
                                        Flush();
                                        RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                    }
                                }
                                else if( !strcmp( QueryProperty, "HasTrackList" ) )
                                {
                                    dbus_bool_t ReplyVal = true;
                                    if( AddVariant( reply->GetMessage(), DBUS_TYPE_BOOLEAN, &ReplyVal ) )
                                    {
                                        Send( reply );
                                        Flush();
                                        RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                    }
                                }
                                else if( !strcmp( QueryProperty, "Identity" ) )
                                {
                                    const char * AppName = "Guayadeque Music Player";
                                    if( AddVariant( reply->GetMessage(), DBUS_TYPE_STRING, &AppName ) )
                                    {
                                        Send( reply );
                                        Flush();
                                        RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                    }
                                }
                                else if( !strcmp( QueryProperty, "DesktopEntry" ) )
                                {
                                    const char * DesktopPath = "guayadeque";
                                    if( AddVariant( reply->GetMessage(), DBUS_TYPE_STRING, &DesktopPath ) )
                                    {
                                        Send( reply );
                                        Flush();
                                        RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                    }
                                }
                                else if( !strcmp( QueryProperty, "SupportedUriSchemes" ) )
                                {
                                    const char * SupportedUriSchemes[] = { "file", "http", "smb", "sftp", "cdda", NULL };
                                    if( AddVariant( reply->GetMessage(), DBUS_TYPE_ARRAY, &SupportedUriSchemes ) )
                                    {
                                        Send( reply );
                                        Flush();
                                        RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                    }
                                }
                                else if( !strcmp( QueryProperty, "SupportedMimeTypes" ) )
                                {
                                    if( AddVariant( reply->GetMessage(), DBUS_TYPE_ARRAY, &GUAYADEQUE_SUPPORTED_MIME_TYPES ) )
                                    {
                                        Send( reply );
                                        Flush();
                                        RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                    }
                                }
                            }
                            else if( !strcmp( QueryIface, "org.mpris.MediaPlayer2.Player" ) )
                            {
                                //guLogMessage( wxT( "Query: '%s'" ), QueryProperty );
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
                                    if( PlayLoop == guPLAYER_PLAYMODE_NONE )
                                        LoopStatus = "None";
                                    else if( PlayLoop == guPLAYER_PLAYMODE_REPEAT_TRACK )
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
                                    dbus_bool_t Shuffle = false;
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
                                    gint64 CurPosition = m_PlayerPanel->GetPosition() * 1000;
                                    if( AddVariant( reply->GetMessage(), DBUS_TYPE_INT64, &CurPosition ) )
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
                                    dbus_bool_t CanGoNext = ( m_PlayerPanel->GetCaps() & MPRIS_CAPS_CAN_GO_NEXT ) > 0;
                                    if( AddVariant( reply->GetMessage(), DBUS_TYPE_BOOLEAN, &CanGoNext ) )
                                    {
                                        Send( reply );
                                        Flush();
                                        RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                    }
                                }
                                else if( !strcmp( QueryProperty, "CanGoPreviouss" ) )
                                {
                                    dbus_bool_t CanGoPrev = ( m_PlayerPanel->GetCaps() & MPRIS_CAPS_CAN_GO_PREV ) > 0;
                                    if( AddVariant( reply->GetMessage(), DBUS_TYPE_BOOLEAN, &CanGoPrev ) )
                                    {
                                        Send( reply );
                                        Flush();
                                        RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                    }
                                }
                                else if( !strcmp( QueryProperty, "CanPlay" ) )
                                {
                                    dbus_bool_t CanPlay = ( m_PlayerPanel->GetCaps() & MPRIS_CAPS_CAN_PLAY ) > 0;
                                    if( AddVariant( reply->GetMessage(), DBUS_TYPE_BOOLEAN, &CanPlay ) )
                                    {
                                        Send( reply );
                                        Flush();
                                        RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                    }
                                }
                                else if( !strcmp( QueryProperty, "CanPause" ) )
                                {
                                    dbus_bool_t CanPause = ( m_PlayerPanel->GetCaps() & MPRIS_CAPS_CAN_PAUSE ) > 0;
                                    if( AddVariant( reply->GetMessage(), DBUS_TYPE_BOOLEAN, &CanPause ) )
                                    {
                                        Send( reply );
                                        Flush();
                                        RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                    }
                                }
                                else if( !strcmp( QueryProperty, "CanSeek" ) )
                                {
                                    dbus_bool_t CanSeek = ( m_PlayerPanel->GetCaps() & MPRIS_CAPS_CAN_SEEK ) > 0;
                                    if( AddVariant( reply->GetMessage(), DBUS_TYPE_BOOLEAN, &CanSeek ) )
                                    {
                                        Send( reply );
                                        Flush();
                                        RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                    }
                                }
                                else if( !strcmp( QueryProperty, "CanControl" ) )
                                {
                                    dbus_bool_t CanControl = true;
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
                                    dbus_bool_t CanEdit = true;
                                    if( AddVariant( reply->GetMessage(), DBUS_TYPE_BOOLEAN, &CanEdit ) )
                                    {
                                        Send( reply );
                                        Flush();
                                        RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                    }
                                }
                            }
                            else if( !strcmp( QueryIface, "org.mpris.MediaPlayer2.Playlists" ) )
                            {
                                if( !strcmp( QueryProperty, "PlaylistCount" ) )
                                {
                                }
                                else if( !strcmp( QueryProperty, "Orderings" ) )
                                {
                                }
                                else if( !strcmp( QueryProperty, "ActivePlaylist" ) )
                                {
                                }
                            }
                        }
                        else if( !strcmp( Member, "Set" ) )
                        {
                            if( !strcmp( QueryIface, "org.mpris.MediaPlayer2" ) )
                            {
                                if( !strcmp( QueryProperty, "Fullscreen" ) )
                                {
                                    dbus_bool_t FullScreen;
                                    if( GetVariant( msg->GetMessage(), DBUS_TYPE_BOOLEAN, &FullScreen ) )
                                    {
                                        guMainFrame * MainFrame = guMainFrame::GetMainFrame();

                                        wxCommandEvent event( wxEVT_MENU, ID_MENU_VIEW_FULLSCREEN );
                                        event.SetInt( FullScreen );
                                        wxPostEvent( MainFrame, event );

                                        Send( reply );
                                        Flush();
                                        RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                    }
                                }
                            }
                            else if( !strcmp( QueryIface, "org.mpris.MediaPlayer2.Player" ) )
                            {
                                if( !strcmp( QueryProperty, "LoopStatus" ) )
                                {
                                    const char * LoopStatus;
                                    if( GetVariant( msg->GetMessage(), DBUS_TYPE_STRING, &LoopStatus ) )
                                    {
                                        int PlayLoop;
                                        if( !strcmp( LoopStatus, "None" ) )
                                        {
                                            PlayLoop = guPLAYER_PLAYMODE_NONE;
                                        }
                                        else if( !strcmp( LoopStatus, "Track" ) )
                                        {
                                            PlayLoop = guPLAYER_PLAYMODE_REPEAT_TRACK;
                                        }
                                        else //if( !strcmp( LoopStatus, "Playlist" ) )
                                        {
                                            PlayLoop = guPLAYER_PLAYMODE_REPEAT_PLAYLIST;
                                        }

                                        wxCommandEvent event( wxEVT_MENU, ID_PLAYERPANEL_SETLOOP );
                                        event.SetInt( PlayLoop );
                                        wxPostEvent( m_PlayerPanel, event );

                                        Send( reply );
                                        Flush();
                                        RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                    }
                                }
                                else if( !strcmp( QueryProperty, "Rate" ) )
                                {
                                    // We are not going to support rate
                                    Send( reply );
                                    Flush();
                                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                }
                                else if( !strcmp( QueryProperty, "Shuffle" ) )
                                {
                                    wxCommandEvent event( wxEVT_MENU, ID_PLAYERPANEL_SETRANDOM );
                                    wxPostEvent( m_PlayerPanel, event );
                                    Send( reply );
                                    Flush();
                                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                }
                                else if( !strcmp( QueryProperty, "Volume" ) )
                                {
                                    double Volume;
                                    if( GetVariant( msg->GetMessage(), DBUS_TYPE_DOUBLE, &Volume ) )
                                    {
                                        wxCommandEvent event( wxEVT_MENU, ID_PLAYERPANEL_SETVOLUME );
                                        event.SetInt( ( int ) ( Volume * 100 ) );
                                        wxPostEvent( m_PlayerPanel, event );
                                        Send( reply );
                                        Flush();
                                        RetVal = DBUS_HANDLER_RESULT_HANDLED;
                                    }
                                }
                            }

                        }
                    }
                }
            }
        }
        else if( !strcmp( Interface, GUAYADEQUE_MPRIS2_INTERFACE_ROOT ) )
        {
            if( !strcmp( Path, GUAYADEQUE_MPRIS2_OBJECT_PATH ) )
            {
                if( !strcmp( Member, "Raise" ) )
                {
                    wxCommandEvent RaiseCmd( wxEVT_MENU, ID_MAINFRAME_WINDOW_RAISE );
                    wxPostEvent( guMainFrame::GetMainFrame(), RaiseCmd );

                    Send( reply );
                    Flush();
                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                }
                else if( !strcmp( Member, "Quit" ) )
                {
                    wxCommandEvent QuitCmd( wxEVT_MENU, ID_MENU_QUIT );
                    wxPostEvent( guMainFrame::GetMainFrame(), QuitCmd );
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
                    wxCommandEvent event( wxEVT_MENU, ID_PLAYERPANEL_NEXTTRACK );
                    wxPostEvent( m_PlayerPanel, event );
                    Send( reply );
                    Flush();
                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                }
                else if( !strcmp( Member, "Previous" ) )
                {
                    wxCommandEvent event( wxEVT_MENU, ID_PLAYERPANEL_PREVTRACK );
                    wxPostEvent( m_PlayerPanel, event );
                    Send( reply );
                    Flush();
                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                }
                else if( !strcmp( Member, "NextAlbum" ) )
                {
                    wxCommandEvent event( wxEVT_MENU, ID_PLAYERPANEL_NEXTALBUM );
                    wxPostEvent( m_PlayerPanel, event );
                    Send( reply );
                    Flush();
                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                }
                else if( !strcmp( Member, "PreviousAlbum" ) )
                {
                    wxCommandEvent event( wxEVT_MENU, ID_PLAYERPANEL_PREVALBUM );
                    wxPostEvent( m_PlayerPanel, event );
                    Send( reply );
                    Flush();
                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                }
                else if( !strcmp( Member, "Pause" ) )
                {
                    if( m_PlayerPanel->GetState() == guMEDIASTATE_PLAYING )
                    {
                        wxCommandEvent event( wxEVT_MENU, ID_PLAYERPANEL_PLAY );
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
                        wxCommandEvent event( wxEVT_MENU, ID_PLAYERPANEL_PLAY );
                        wxPostEvent( m_PlayerPanel, event );
                    //}
                    Send( reply );
                    Flush();
                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                }
                else if( !strcmp( Member, "Stop" ) )
                {
                    wxCommandEvent event( wxEVT_MENU, ID_PLAYERPANEL_STOP );
                    wxPostEvent( m_PlayerPanel, event );
                    Send( reply );
                    Flush();
                    RetVal = DBUS_HANDLER_RESULT_HANDLED;
                }
                else if( !strcmp( Member, "Play" ) )
                {
                    if( m_PlayerPanel->GetState() != guMEDIASTATE_PLAYING )
                    {
                        wxCommandEvent event( wxEVT_MENU, ID_PLAYERPANEL_PLAY );
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
                            DBUS_TYPE_OBJECT_PATH, &TrackId,
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
                    DBusError error;
                    dbus_error_init( &error );

                    const char * Uri;
                    dbus_message_get_args( msg->GetMessage(), &error,
                            DBUS_TYPE_STRING, &Uri,
                            DBUS_TYPE_INVALID );

                    if( dbus_error_is_set( &error ) )
                    {
                        guLogMessage( wxT( "Could not read the OpenUri parameter : %s" ), wxString( error.message, wxConvUTF8 ).c_str() );
                        dbus_error_free( &error );
                    }
                    else
                    {
                        wxArrayString Streams;
                        Streams.Add( wxString( Uri, wxConvUTF8 ) );

                        m_PlayerPanel->SetPlayList( Streams );
                    }
                }
            }
        }
        else if( !strcmp( Interface, GUAYADEQUE_MPRIS2_INTERFACE_TRACKLIST ) )
        {
            if( !strcmp( Path, GUAYADEQUE_MPRIS2_OBJECT_PATH ) )
            {
                if( !strcmp( Member, "GetTracksMetadata" ) )
                {
                }
                else if( !strcmp( Member, "AddTrack" ) )
                {
                }
                else if( !strcmp( Member, "RemoveTrack" ) )
                {
                }
                else if( !strcmp( Member, "GoTo" ) )
                {
                }
            }
        }
        else if( !strcmp( Interface, GUAYADEQUE_MPRIS2_INTERFACE_PLAYLISTS ) )
        {
            if( !strcmp( Path, GUAYADEQUE_MPRIS2_OBJECT_PATH ) )
            {
                if( !strcmp( Member, "ActivatePlaylist" ) )
                {
                    DBusError error;
                    dbus_error_init( &error );

                    const char * PlaylistPath;
                    dbus_message_get_args( msg->GetMessage(), &error,
                            DBUS_TYPE_OBJECT_PATH, &PlaylistPath,
                            DBUS_TYPE_INVALID );

                    if( dbus_error_is_set( &error ) )
                    {
                        guLogMessage( wxT( "Could not read the ActivatePlaylist parameter : %s" ), wxString( error.message, wxConvUTF8 ).c_str() );
                        dbus_error_free( &error );
                    }
                    else
                    {
                        wxString PlaylistName = wxString( PlaylistPath, wxConvUTF8 ).AfterLast( wxT( '/' ) );
                        long PlaylistId;
                        if( PlaylistName.ToLong( &PlaylistId ) )
                        {
                            // Send Event to the PlayerPanel to load the playlist
                            wxCommandEvent CmdEvent( wxEVT_MENU, ID_MAINFRAME_LOAD_PLAYLIST );
                            CmdEvent.SetInt( PlaylistId );
                            wxPostEvent( guMainFrame::GetMainFrame(), CmdEvent );

                            Send( reply );
                            Flush();
                            RetVal = DBUS_HANDLER_RESULT_HANDLED;
                        }
                    }
                }
                else if( !strcmp( Member, "GetPlaylists" ) )
                {
                    DBusError error;
                    dbus_error_init( &error );

                    dbus_uint32_t    Index;
                    dbus_uint32_t    MaxCount;
                    const char *    Order;
                    dbus_bool_t     ReverseOrder;

                    dbus_message_get_args( msg->GetMessage(), &error,
                            DBUS_TYPE_UINT32, &Index,
                            DBUS_TYPE_UINT32, &MaxCount,
                            DBUS_TYPE_STRING, &Order,
                            DBUS_TYPE_BOOLEAN, &ReverseOrder,
                            DBUS_TYPE_INVALID );

                    if( dbus_error_is_set( &error ) )
                    {
                        guLogMessage( wxT( "Could not read the GetPlaylists parameter : %s" ), wxString( error.message, wxConvUTF8 ).c_str() );
                        dbus_error_free( &error );
                    }
                    else
                    {
                        if( GetPlaylists( reply->GetMessage(), Index, MaxCount, Order, ReverseOrder ) )
                        {
                            Send( reply );
                            Flush();
                            RetVal = DBUS_HANDLER_RESULT_HANDLED;
                        }
                    }
                }
            }
        }
    }

    return RetVal;
}

#define INDICATORS_SOUND_RETRY_COUNT    10
#define INDICATORS_SOUND_WAIT_TIME      100

// -------------------------------------------------------------------------------- //
int guMPRIS2::Indicators_Sound_BlacklistMediaPlayer( const dbus_bool_t blacklist )
{
    int RetryCnt = 0;
    guDBusMethodCall * Msg = new guDBusMethodCall( "com.canonical.indicators.sound",
                                               "/com/canonical/indicators/sound/service",
                                               "com.canonical.indicators.sound",
                                               "BlacklistMediaPlayer" );

    const char * desktopname = "guayadeque";
    int RetVal = wxNOT_FOUND;

    dbus_message_append_args( Msg->GetMessage(), DBUS_TYPE_STRING, &desktopname, DBUS_TYPE_BOOLEAN, &blacklist, DBUS_TYPE_INVALID );

    guDBusMessage * Reply = NULL;
    do {
        Reply = SendWithReplyAndBlock( Msg );
        if( Reply )
        {
            DBusError error;
            dbus_error_init( &error );

            dbus_bool_t Changed = false;

            dbus_message_get_args( Reply->GetMessage(), &error, DBUS_TYPE_BOOLEAN, &Changed, DBUS_TYPE_INVALID );

            if( dbus_error_is_set( &error ) )
            {
                guLogMessage( wxT( "Indicator Sound parameter error : %s" ), wxString( error.message, wxConvUTF8 ).c_str() );
                dbus_error_free( &error );
            }
            else
            {
                RetVal = Changed;
            }

            delete Reply;
        }
        else
        {
            if( RetryCnt++ > INDICATORS_SOUND_RETRY_COUNT )
                break;

            wxMilliSleep( INDICATORS_SOUND_WAIT_TIME );
        }

    } while( !Reply );

    delete Msg;

    guLogMessage( wxT( "Indicators_Sound_BlacklistMediaPlayer( %i ) => %i" ), blacklist, RetVal );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
int guMPRIS2::Indicators_Sound_IsBlackListed( void )
{
    int RetryCnt = 0;
    int RetVal = wxNOT_FOUND;

    guDBusMethodCall * Msg = new guDBusMethodCall( "com.canonical.indicators.sound",
                                               "/com/canonical/indicators/sound/service",
                                               "com.canonical.indicators.sound",
                                               "IsBlacklisted" );

    const char * desktopname = "guayadeque";

    dbus_message_append_args( Msg->GetMessage(), DBUS_TYPE_STRING, &desktopname, DBUS_TYPE_INVALID );

    guDBusMessage * Reply = NULL;
    do {
        Reply = SendWithReplyAndBlock( Msg );
        if( Reply )
        {
            DBusError error;
            dbus_error_init( &error );

            dbus_bool_t Blacklisted = false;

            dbus_message_get_args( Reply->GetMessage(), &error, DBUS_TYPE_BOOLEAN, &Blacklisted, DBUS_TYPE_INVALID );

            if( dbus_error_is_set( &error ) )
            {
                guLogMessage( wxT( "Indicator Sound parameter error : %s" ), wxString( error.message, wxConvUTF8 ).c_str() );
                dbus_error_free( &error );
            }
            else
            {
                RetVal = Blacklisted;
            }

            delete Reply;
        }
        else
        {
            if( RetryCnt++ > INDICATORS_SOUND_RETRY_COUNT )
                break;

            wxMilliSleep( INDICATORS_SOUND_WAIT_TIME );
        }

    } while( !Reply );

    delete Msg;

    guLogMessage( wxT( "Indicators_Sound_IsBlacklisted() => %i" ), RetVal );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
bool guMPRIS2::Indicators_Sound_Available( void )
{
    bool RetVal = HasOwner( "com.canonical.indicators.sound" );
    guLogMessage( wxT( "Indicators_Sound_Available() => %i" ), RetVal );
    return RetVal;
}

}

// -------------------------------------------------------------------------------- //
