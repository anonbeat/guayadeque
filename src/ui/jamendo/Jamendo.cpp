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
#include "Jamendo.h"

#include "TagInfo.h"
#include "MainFrame.h"
#include "SelCoverFile.h"
#include "StatusBar.h"

#include <wx/wfstream.h>
#include <wx/mstream.h>
#include <wx/zstream.h>
#include <wx/xml/xml.h>

#include <id3v1genres.h>

//
//    Data not in the dump but easily fetchable with the IDs
//
//    (Please replace {TRACKID} and {ALBUMID} in these urls by the numeric IDs found in the dump)
//
//        * MP3s for streaming : http://api.jamendo.com/get2/stream/track/redirect/?id={TRACKID}&streamencoding=mp31
//        * OGGs for streaming : http://api.jamendo.com/get2/stream/track/redirect/?id={TRACKID}&streamencoding=ogg2
//
//        * .torrent file for download (MP3 archive) : http://api.jamendo.com/get2/bittorrent/file/plain/?album_id={ALBUMID}&type=archive&class=mp32
//        * .torrent file for download (OGG archive) : http://api.jamendo.com/get2/bittorrent/file/plain/?album_id={ALBUMID}&type=archive&class=ogg3
//
//        * Album Covers are available here: http://api.jamendo.com/get2/image/album/redirect/?id={ALBUMID}&imagesize={100-600}
//
//
// To download directly in zip format
// http://www.jamendo.com/en/download/album/57557
// >> http://download25.jamendo.com/request/album/61518/mp32/fabc1123
// << Jamendo_HttpDownloadCallback('ready','fb267e3f92');
// >> http://download25.jamendo.com/download/album/61518/mp32/"+data+"/"+encodeURIComponent("noblemo - PIANO -- Jamendo - MP3 VBR 192k - 2010.02.16 [www.jamendo.com].zip")

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
// guJamendoLibrary
// -------------------------------------------------------------------------------- //
guJamendoLibrary::guJamendoLibrary( const wxString &dbname ) :
    guDbLibrary( dbname )
{
}

// -------------------------------------------------------------------------------- //
guJamendoLibrary::~guJamendoLibrary()
{
}

// -------------------------------------------------------------------------------- //
void guJamendoLibrary::UpdateArtistsLabels( const guArrayListItems &labelsets )
{
  guListItems   LaItems;

  // The ArtistLabels string is the same for all songs so done out of the loop
  GetLabels( &LaItems, true );

  int           ArIndex;
  int           ArCount = labelsets.Count();
  for( ArIndex = 0; ArIndex < ArCount; ArIndex++ )
  {
    wxArrayInt ArLabels = labelsets[ ArIndex ].GetData();

    //guLogMessage( wxT( "Artist Labels : '%s'" ), ArtistLabelStr.c_str() );
    // Update the Database
    wxArrayInt Artists;
    Artists.Add( labelsets[ ArIndex ].GetId() );
    SetArtistsLabels( Artists, ArLabels );

  }
}

// -------------------------------------------------------------------------------- //
void guJamendoLibrary::UpdateSongsLabels( const guArrayListItems &labelsets )
{
  guListItems   LaItems;

  // The ArtistLabels string is the same for all songs so done out of the loop
  GetLabels( &LaItems, true );

  int           ItemIndex;
  int           ItemCount = labelsets.Count();
  for( ItemIndex = 0; ItemIndex < ItemCount; ItemIndex++ )
  {
    wxArrayInt ItemLabels = labelsets[ ItemIndex ].GetData();

    //guLogMessage( wxT( "Artist Labels : '%s'" ), ArtistLabelStr.c_str() );
    // Update the Database
    wxArrayInt ItemIds;
    ItemIds.Add( labelsets[ ItemIndex ].GetId() );
    SetSongsLabels( ItemIds, ItemLabels );

  }
}

// -------------------------------------------------------------------------------- //
void guJamendoLibrary::UpdateAlbumsLabels( const guArrayListItems &labelsets )
{
  guListItems   LaItems;

  // The ArtistLabels string is the same for all songs so done out of the loop
  GetLabels( &LaItems, true );

  int           ItemIndex;
  int           ItemCount = labelsets.Count();
  for( ItemIndex = 0; ItemIndex < ItemCount; ItemIndex++ )
  {
    wxArrayInt ItemLabels = labelsets[ ItemIndex ].GetData();

    //guLogMessage( wxT( "Artist Labels : '%s'" ), ArtistLabelStr.c_str() );
    // Update the Database
    wxArrayInt ItemIds;
    ItemIds.Add( labelsets[ ItemIndex ].GetId() );
    SetAlbumsLabels( ItemIds, ItemLabels );
  }
}

// -------------------------------------------------------------------------------- //
void guJamendoLibrary::CreateNewSong( guTrack * track )
{
    wxString query;
    wxSQLite3ResultSet dbRes;

    track->m_Path = track->m_GenreName + wxT( "/" ) +
                    track->m_ArtistName + wxT( "/" ) +
                    track->m_AlbumName + wxT( "/" );
    track->m_PathId = GetPathId( track->m_Path );

    query = wxString::Format( wxT( "SELECT song_id FROM songs WHERE song_id = %i LIMIT 1;" ), track->m_SongId );
    dbRes = ExecuteQuery( query );

    if( !dbRes.NextRow() )
    {
        wxString query = wxString::Format( wxT( "INSERT INTO songs( " \
                    "song_id, song_playcount, song_addedtime, " \
                    "song_name, song_genreid, song_genre, song_artistid, song_artist, " \
                    "song_albumid, song_album, song_pathid, song_path, song_filename, song_format, song_number, song_year, " \
                    "song_coverid, song_disk, song_length, song_offset, song_bitrate, song_rating, " \
                    "song_filesize ) VALUES( %u, 0, %lu, '%s', %u, '%s', %u, '%s', %u, '%s', " \
                    "%u, '%s', '%s', 'mp3,ogg', %u, %u, %u, '%s', %u, 0, 0, -1, 0 )" ),
                    track->m_SongId,
                    wxDateTime::GetTimeNow(),
                    escape_query_str( track->m_SongName ).c_str(),
                    track->m_GenreId,
                    escape_query_str( track->m_GenreName ).c_str(),
                    track->m_ArtistId,
                    escape_query_str( track->m_ArtistName ).c_str(),
                    track->m_AlbumId,
                    escape_query_str( track->m_AlbumName ).c_str(),
                    track->m_PathId,
                    escape_query_str( track->m_Path ).c_str(),
                    escape_query_str( track->m_FileName ).c_str(),
                    track->m_Number,
                    track->m_Year,
                    track->m_CoverId,
                    escape_query_str( track->m_Disk ).c_str(),
                    track->m_Length );

        //guLogMessage( wxT( "%s" ), query.c_str() );
        ExecuteUpdate( query );

    }

//    guLogMessage( wxT( "%s/%s/%s/%i - %s" ),
//                track->m_GenreName.c_str(),
//                track->m_ArtistName.c_str(),
//                track->m_AlbumName.c_str(),
//                track->m_Number,
//                track->m_SongName.c_str() );

}




// -------------------------------------------------------------------------------- //
// guJamendoPanel
// -------------------------------------------------------------------------------- //
guJamendoPanel::guJamendoPanel( wxWindow * parent, guMediaViewer * mediaviewer ) :
    guLibPanel( parent, mediaviewer )
{
    Bind( wxEVT_MENU, &guJamendoPanel::OnDownloadAlbum, this, ID_JAMENDO_DOWNLOAD_TORRENT_ALBUM );
    Bind( wxEVT_MENU, &guJamendoPanel::OnDownloadTrackAlbum, this, ID_JAMENDO_DOWNLOAD_TORRENT_TRACK_ALBUM );
    Bind( wxEVT_MENU, &guJamendoPanel::OnDownloadAlbum, this, ID_JAMENDO_DOWNLOAD_DIRECT_ALBUM );
    Bind( wxEVT_MENU, &guJamendoPanel::OnDownloadTrackAlbum, this, ID_JAMENDO_DOWNLOAD_DIRECT_TRACK_ALBUM );
}

// -------------------------------------------------------------------------------- //
guJamendoPanel::~guJamendoPanel()
{
    Unbind( wxEVT_MENU, &guJamendoPanel::OnDownloadAlbum, this, ID_JAMENDO_DOWNLOAD_TORRENT_ALBUM );
    Unbind( wxEVT_MENU, &guJamendoPanel::OnDownloadTrackAlbum, this, ID_JAMENDO_DOWNLOAD_TORRENT_TRACK_ALBUM );
    Unbind( wxEVT_MENU, &guJamendoPanel::OnDownloadAlbum, this, ID_JAMENDO_DOWNLOAD_DIRECT_ALBUM );
    Unbind( wxEVT_MENU, &guJamendoPanel::OnDownloadTrackAlbum, this, ID_JAMENDO_DOWNLOAD_DIRECT_TRACK_ALBUM );
}

// -------------------------------------------------------------------------------- //
void guJamendoPanel::OnDownloadAlbum( wxCommandEvent &event )
{
    guLogMessage( wxT( "OnDownloadAlbum" ) );

    wxArrayInt Albums = m_AlbumListCtrl->GetSelectedItems();

    ( ( guMediaViewerJamendo * ) m_MediaViewer )->DownloadAlbums( Albums, ( event.GetId() == ID_JAMENDO_DOWNLOAD_TORRENT_ALBUM ) );
}

// -------------------------------------------------------------------------------- //
void guJamendoPanel::OnDownloadTrackAlbum( wxCommandEvent &event )
{
    guLogMessage( wxT( "OnDownloadTrackAlbum" ) );

    guTrackArray Tracks;
    m_SongListCtrl->GetSelectedSongs( &Tracks );

    wxArrayInt Albums;
    int Index;
    int Count = Tracks.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( Albums.Index( Tracks[ Index ].m_AlbumId ) == wxNOT_FOUND )
            Albums.Add( Tracks[ Index ].m_AlbumId );
    }

    ( ( guMediaViewerJamendo * ) m_MediaViewer )->DownloadAlbums( Albums, ( event.GetId() == ID_JAMENDO_DOWNLOAD_TORRENT_TRACK_ALBUM ) );
}




// -------------------------------------------------------------------------------- //
// guJamendoDownloadThread
// -------------------------------------------------------------------------------- //
guJamendoDownloadThread::guJamendoDownloadThread( guMediaViewerJamendo * mediaviewer )
{
    m_MediaViewer = mediaviewer;
    m_Db = ( guJamendoLibrary * ) mediaviewer->GetDb();

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guJamendoDownloadThread::~guJamendoDownloadThread()
{
    if( !TestDestroy() )
    {
        m_MediaViewer->EndDownloadThread();
    }
}

// -------------------------------------------------------------------------------- //
void guJamendoDownloadThread::AddAlbum( const int albumid, const bool iscover )
{
    if( iscover )
    {
        m_CoversMutex.Lock();
        m_Covers.Add( albumid );
        m_CoversMutex.Unlock();
    }
    else
    {
        m_AlbumsMutex.Lock();
        m_Albums.Add( albumid );
        m_AlbumsMutex.Unlock();
    }
}

// -------------------------------------------------------------------------------- //
void guJamendoDownloadThread::AddAlbums( const wxArrayInt &albumids, const bool iscover )
{
    int Index;
    int Count = albumids.Count();

    if( iscover )
    {
        m_CoversMutex.Lock();
        for( Index = 0; Index < Count; Index++ )
        {
            m_Covers.Add( albumids[ Index ] );
        }
        m_CoversMutex.Unlock();
    }
    else
    {
        m_AlbumsMutex.Lock();
        for( Index = 0; Index < Count; Index++ )
        {
            m_Albums.Add( albumids[ Index ] );
        }
        m_AlbumsMutex.Unlock();
    }
}

// -------------------------------------------------------------------------------- //
guJamendoDownloadThread::ExitCode guJamendoDownloadThread::Entry()
{
    int Count;
    int LoopCount = 0;
    guConfig * Config = ( guConfig * ) guConfig::Get();
    int AudioFormat = Config->ReadNum( CONFIG_KEY_JAMENDO_AUDIOFORMAT, 1, CONFIG_PATH_JAMENDO );
    wxString TorrentCmd = Config->ReadStr( CONFIG_KEY_JAMENDO_TORRENT_COMMAND, wxEmptyString, CONFIG_PATH_JAMENDO );
    while( !TestDestroy() )
    {
        m_CoversMutex.Lock();
        Count = m_Covers.Count();
        m_CoversMutex.Unlock();

        size_t CurTime = wxGetLocalTimeMillis().GetLo();
        if( Count )
        {
            LoopCount = 0;
            wxString CoverFile = guPATH_JAMENDO_COVERS;
            CoverFile += wxString::Format( wxT( "%u.jpg" ), m_Covers[ 0 ] );

            if( !wxFileExists( CoverFile ) )
            {
                if( !wxDirExists( wxPathOnly( CoverFile ) + wxT( "/" ) ) )
                {
                    wxMkdir( wxPathOnly( CoverFile ) + wxT( "/" ), 0770 );
                }
                wxString CoverUrl = wxString::Format( guJAMENDO_COVER_DOWNLOAD_URL, m_Covers[ 0 ], 300 );
                DownloadImage( CoverUrl, CoverFile, 300 );
            }

            if( wxFileExists( CoverFile ) )
            {
                int CoverId = m_Db->AddCoverFile( CoverFile );

                wxString query = wxString::Format( wxT( "UPDATE songs SET song_coverid = %u WHERE song_albumid = %u" ),
                                    CoverId, m_Covers[ 0 ] );

                m_Db->ExecuteUpdate( query );

                // Notify the panel that the cover is downloaded
                wxCommandEvent event( wxEVT_MENU, ID_JAMENDO_COVER_DOWNLAODED );
                event.SetInt( m_Covers[ 0 ] );
                wxPostEvent( m_MediaViewer, event );
            }
            else
            {
                guLogMessage( wxT( "Could not get the jamendo cover art %s" ), CoverFile.c_str() );
            }

            m_CoversMutex.Lock();
            m_Covers.RemoveAt( 0 );
            m_CoversMutex.Unlock();
        }
        else
        {
            LoopCount++;
            if( LoopCount > 8 )
            {
                break;
            }
        }

        if( TestDestroy() )
            break;

        size_t Elapsed = wxGetLocalTimeMillis().GetLo() - CurTime;
        if( !( Elapsed > 1000 ) )
        {
            Sleep( 1000 - Elapsed );
        }

        //
        // Album Torrents
        //
        m_AlbumsMutex.Lock();
        Count = m_Albums.Count();
        m_AlbumsMutex.Unlock();

        CurTime = wxGetLocalTimeMillis().GetLo();
        if( Count )
        {
            LoopCount = 0;

            wxString Url = wxString::Format( guJAMENDO_TORRENT_DOWNLOAD_URL, m_Albums[ 0 ] );
            Url += AudioFormat ? guJAMENDO_DOWNLOAD_FORMAT_OGG : guJAMENDO_DOWNLOAD_FORMAT_MP3;

            //guLogMessage( wxT( "Getting %s" ), Url.c_str() );

            wxString TorrentUrl = GetUrlContent( Url );

            //guLogMessage( wxT( "Downloading '%s'" ), TorrentUrl.c_str() );
            if( !TorrentUrl.IsEmpty() )
            {
                wxString TmpFileName = wxFileName::CreateTempFileName( wxString::Format( wxT( "%u" ), m_Albums[ 0 ] ) );
                TmpFileName += wxT( ".torrent" );
                if( DownloadFile( TorrentUrl, TmpFileName ) )
                {
                    guExecute( TorrentCmd + wxT( " " ) + TmpFileName );
                }
            }

            m_AlbumsMutex.Lock();
            m_Albums.RemoveAt( 0 );
            m_AlbumsMutex.Unlock();
        }
        else
        {
            LoopCount++;
            if( LoopCount > 8 )
            {
                break;
            }
        }

        if( TestDestroy() )
            break;

        Elapsed = wxGetLocalTimeMillis().GetLo() - CurTime;
        if( !( Elapsed > 1000 ) )
        {
            Sleep( 1000 - Elapsed );
        }
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
// guJamendoUpdateThread
// -------------------------------------------------------------------------------- //
guJamendoUpdateThread::guJamendoUpdateThread( guMediaViewerJamendo * mediaviewer, int action, int gaugeid )
{
    m_MediaViewer = mediaviewer;
    m_Db = ( guJamendoLibrary * ) mediaviewer->GetDb();
    m_MainFrame = ( guMainFrame * ) guMainFrame::GetMainFrame();
    m_Action = action;
    m_GaugeId = gaugeid;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    m_AllowedGenres = Config->ReadANum( CONFIG_KEY_JAMENDO_GENRES_GENRE, 0, CONFIG_PATH_JAMENDO_GENRES );

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 10 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guJamendoUpdateThread::~guJamendoUpdateThread()
{
    //
    wxCommandEvent event( wxEVT_MENU, ID_STATUSBAR_GAUGE_REMOVE );
    event.SetInt( m_GaugeId );
    wxPostEvent( m_MainFrame, event );

    if( !TestDestroy() )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        if( Config )
        {
            wxDateTime Now = wxDateTime::Now();
            Config->WriteNum( CONFIG_KEY_JAMENDO_LAST_UPDATE, Now.GetTicks(), CONFIG_PATH_JAMENDO );
            Config->Flush();
        }

        event.SetId( ID_JAMENDO_UPDATE_FINISHED );
        event.SetEventObject( ( wxObject * ) this );
        wxPostEvent( m_MediaViewer, event );
    }
}

/*
<JamendoData epoch="1282819258" documentation="http://developer.jamendo.com/en/wiki/DatabaseDumps" type="artistalbumtrack">
<Artists>
    <artist>
        <id>338334</id>
        <name>0/20</name>
        <url>http://www.jamendo.com/artist/0_20</url>
        <mbgid></mbgid>
        <location>
            <country>ECU</country>
            <state></state>
            <city>QUITO</city>
            <latitude>-0.229498</latitude>
            <longitude>-78.5243</longitude>
        </location>
        <Albums>
            <album>
                <id>19831</id>
                <name>!Ya Estoy Harto de Mis Profesores¡¡¡¡¡¡¡(DEMO VERSION)</name>
                <url>http://www.jamendo.com/album/19831</url>
                <releasedate>2008-02-29T15:29:52+01:00</releasedate>
                <filename>0 20 - !Ya Estoy Harto de Mis Profesores DEMO VERSION</filename>
                <id3genre>43</id3genre>
                <mbgid></mbgid>
                <license_artwork>http://creativecommons.org/licenses/by-sa/3.0/</license_artwork>
                <Tracks>
                    <track><id>138701</id>
                        <name>Odio Estudiar</name>
                        <duration>40</duration>
                        <numalbum>1</numalbum>
                        <filename>01 - 0 20 - Odio Estudiar</filename>
                        <mbgid></mbgid>
                        <id3genre>43</id3genre>
                        <license>http://creativecommons.org/licenses/by-sa/3.0/</license>
                    </track>
                    <track>
                        <id>138702</id>
                        <name>Turro Colegio</name>
                        <duration>145</duration>
                        <numalbum>2</numalbum>
                        <filename>02 - 0 20 - Turro Colegio</filename>
                        <mbgid></mbgid>
                        <id3genre>40</id3genre>
                        <license>http://creativecommons.org/licenses/by-sa/3.0/</license>
                    </track>
                </Tracks>
            </album>
        </Albums>
    </artist>
...
</Artists>
*/

// -------------------------------------------------------------------------------- //
void ReadJamendoXmlTrack( wxXmlNode * xmlnode, guJamendoUpdateThread * thread, guTrack * track )
{
    long Id;
    while( xmlnode && !thread->TestDestroy() )
    {
        wxString ItemName = xmlnode->GetName();
        if( ItemName == wxT( "id" ) )
        {
            xmlnode->GetNodeContent().ToLong( &Id );
            track->m_SongId = Id;
        }
        else if( ItemName == wxT( "name" ) )
        {
            track->m_SongName = xmlnode->GetNodeContent().Trim( true ).Trim( false );
        }
        else if( ItemName == wxT( "duration" ) )
        {
            xmlnode->GetNodeContent().ToLong( &Id );
            track->m_Length = Id * 1000;
        }
        else if( ItemName == wxT( "numalbum" ) )
        {
            xmlnode->GetNodeContent().ToLong( &Id );
            track->m_Number = Id;
        }
        else if( ItemName == wxT( "filename" ) )
        {
            track->m_FileName = xmlnode->GetNodeContent();
        }
        else if( ItemName == wxT( "id3genre" ) )
        {
            xmlnode->GetNodeContent().ToLong( &Id );
            track->m_GenreName = TStringTowxString( TagLib::ID3v1::genre( Id ) );
            track->m_GenreId = Id + 1;  // Avoid use Id 0 ('Blues') as for us its 'All Genres'
        }
        xmlnode = xmlnode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void ReadJamendoXmlTracks( wxXmlNode * xmlnode, guJamendoUpdateThread * thread, guTrack * track, guJamendoLibrary * db, wxArrayInt &genres )
{
    while( xmlnode && !thread->TestDestroy() )
    {
        if( xmlnode->GetName() == wxT( "track" ) )
        {
            ReadJamendoXmlTrack( xmlnode->GetChildren(), thread, track );

            if( genres.Index( track->m_GenreId - 1 ) != wxNOT_FOUND )
            {
                db->CreateNewSong( track );
            }
        }
        xmlnode = xmlnode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void ReadJamendoXmlAlbum( wxXmlNode * xmlnode, guJamendoUpdateThread * thread, guTrack * track, guJamendoLibrary * db, wxArrayInt &genres )
{
    long Id;
    while( xmlnode && !thread->TestDestroy() )
    {
        wxString ItemName = xmlnode->GetName();
        if( ItemName == wxT( "id" ) )
        {
            xmlnode->GetNodeContent().ToLong( &Id );
            track->m_AlbumId = Id;
        }
        else if( ItemName == wxT( "name" ) )
        {
            track->m_AlbumName = xmlnode->GetNodeContent().Trim( true ).Trim( false );
        }
        else if( ItemName == wxT( "releasedate" ) )
        {
            wxString ReleaseDate = xmlnode->GetNodeContent();
            if( !ReleaseDate.IsEmpty() )
            {
                ReleaseDate.Mid( 0, 4 ).ToLong( &Id );
                track->m_Year = Id;
            }
        }
        else if( ItemName == wxT( "id3genre" ) )
        {
            xmlnode->GetNodeContent().ToLong( &Id );
            track->m_GenreName = TStringTowxString( TagLib::ID3v1::genre( Id ) );
            track->m_GenreId = Id + 1;  // Avoid use Id 0 ('Blues') as for us its 'All Genres'
        }
        else if( ItemName == wxT( "Tracks" ) )
        {
            ReadJamendoXmlTracks( xmlnode->GetChildren(), thread, track, db, genres );
        }
        xmlnode = xmlnode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void ReadJamendoXmlAlbums( wxXmlNode * xmlnode, guJamendoUpdateThread * thread, guTrack * track, guJamendoLibrary * db, wxArrayInt &genres )
{
    while( xmlnode && !thread->TestDestroy() )
    {
        if( xmlnode->GetName() == wxT( "album" ) )
        {
            track->m_CoverId = 0;
            track->m_GenreName = _( "Unknown" );
            track->m_GenreId = 10000;
            ReadJamendoXmlAlbum( xmlnode->GetChildren(), thread, track, db, genres );
        }
        xmlnode = xmlnode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void ReadJamendoXmlArtist( wxXmlNode * xmlnode, guJamendoUpdateThread * thread, guTrack * track, guJamendoLibrary * db, wxArrayInt &genres )
{
    while( xmlnode && !thread->TestDestroy() )
    {
        wxString ItemName = xmlnode->GetName();
        if( ItemName == wxT( "id" ) )
        {
            long Id;
            xmlnode->GetNodeContent().ToLong( &Id );
            track->m_ArtistId = Id;
        }
        else if( ItemName == wxT( "name" ) )
        {
            track->m_ArtistName = xmlnode->GetNodeContent().Trim( true ).Trim( false );
            guLogMessage( wxT( "Artist: '%s'" ), track->m_ArtistName.c_str() );
        }
        else if( ItemName == wxT( "Albums" ) )
        {
            ReadJamendoXmlAlbums( xmlnode->GetChildren(), thread, track, db, genres );
        }
        xmlnode = xmlnode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
bool guJamendoUpdateThread::UpgradeDatabase( void )
{
    if( DownloadFile( guJAMENDO_DATABASE_DUMP_URL, guPATH_JAMENDO wxT( "dbdump_artistalbumtrack.xml.gz" ) ) )
    {
        wxFileInputStream Ins( guPATH_JAMENDO wxT( "dbdump_artistalbumtrack.xml.gz" ) );
        if( Ins.IsOk() )
        {
            wxZlibInputStream ZIn( Ins );
            if( ZIn.IsOk() )
            {
                wxFileOutputStream ZOuts( guPATH_JAMENDO wxT( "dbdump_artistalbumtrack.xml" ) );
                if( ZOuts.IsOk() )
                {
                    ZIn.Read( ZOuts );
                    return true;
                }
            }
        }
        else
        {
            guLogError( wxT( "Could not open the Jamendo local database copy" ) );
        }
    }
    return false;
}

// -------------------------------------------------------------------------------- //
guJamendoUpdateThread::ExitCode guJamendoUpdateThread::Entry()
{
    wxString query;

    wxCommandEvent evtup( wxEVT_MENU, ID_STATUSBAR_GAUGE_UPDATE );
    evtup.SetInt( m_GaugeId );

    wxCommandEvent evtmax( wxEVT_MENU, ID_STATUSBAR_GAUGE_SETMAX );
    evtmax.SetInt( m_GaugeId );

    if( m_Action == guJAMENDO_ACTION_UPDATE &&
        !wxFileExists( guPATH_JAMENDO wxT( "dbdump_artistalbumtrack.xml" ) ) )
    {
        m_Action = guJAMENDO_ACTION_UPGRADE;
    }

    guLogMessage( wxT( "Starting the Jamendo Update process..." ) );
    if( !TestDestroy() && ( m_Action == guJAMENDO_ACTION_UPDATE || UpgradeDatabase() ) )
    {
        wxFile XmlFile( guPATH_JAMENDO wxT( "dbdump_artistalbumtrack.xml" ), wxFile::read );
        if( XmlFile.IsOpened() )
        {
            guListItems CurrentGenres;
            m_Db->GetGenres( &CurrentGenres, true );

            wxArrayInt GenresToDel;
            int Index;
            int Count = CurrentGenres.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                if( m_AllowedGenres.Index( CurrentGenres[ Index ].m_Id - 1 ) == wxNOT_FOUND )
                    GenresToDel.Add( CurrentGenres[ Index ].m_Id );
            }

//            query = wxT( "BEGIN TRANSACTION" );
//            m_Db->ExecuteUpdate( query );

            if( GenresToDel.Count() )
            {
                guLogMessage( wxT( "Deleting old jamendo genres" ) );
                query = wxT( "DELETE FROM songs WHERE " ) + ArrayToFilter( GenresToDel, wxT( "song_genreid" ) );
                m_Db->ExecuteUpdate( query );
            }

            evtmax.SetExtraLong( XmlFile.Length() );
            wxPostEvent( guMainFrame::GetMainFrame(), evtmax );

            wxFileOffset CurPos;

            if( m_AllowedGenres.Count() )
            {
                wxString ArtistChunk = guGetNextXMLChunk( XmlFile, CurPos, "<artist>", "</artist>" );
                int LastTime = wxGetLocalTime() + 2;
                while( !TestDestroy() && !ArtistChunk.IsEmpty() )
                {
                    wxStringInputStream Ins( ArtistChunk );
                    wxXmlDocument XmlDoc( Ins );
                    if( XmlDoc.IsOk() )
                    {
                        wxXmlNode * XmlNode = XmlDoc.GetRoot();

                        if( XmlNode && XmlNode->GetName() == wxT( "artist" ) )
                        {
                            ReadJamendoXmlArtist( XmlNode->GetChildren(), this, &m_CurrentTrack, m_Db, m_AllowedGenres );
                        }
                    }
                    else
                    {
                        guLogMessage( wxT( "Error in artist chunk:\n%s" ), ArtistChunk.c_str() );
                    }

                    ArtistChunk = guGetNextXMLChunk( XmlFile, CurPos, "<artist>", "</artist>" );

                    if( wxGetLocalTime() > LastTime )
                    {
                        LastTime = wxGetLocalTime() + 2;
//                        query = wxT( "END TRANSACTION" );
//                        m_Db->ExecuteUpdate( query );

                        evtup.SetExtraLong( CurPos );
                        wxPostEvent( guMainFrame::GetMainFrame(), evtup );

//                        query = wxT( "BEGIN TRANSACTION" );
//                        m_Db->ExecuteUpdate( query );
                    }
                }
            }

//            query = wxT( "END TRANSACTION" );
//            m_Db->ExecuteUpdate( query );

            XmlFile.Close();
        }
        else
        {
            guLogMessage( wxT( "Could not open the file %s" ), wxString( guPATH_JAMENDO wxT( "dbdump_artistalbumtrack.xml" ) ).c_str() );
        }
    }

    return 0;
}




// -------------------------------------------------------------------------------- //
// guJamendoAlbumBrowser
// -------------------------------------------------------------------------------- //
guJamendoAlbumBrowser::guJamendoAlbumBrowser( wxWindow * parent, guMediaViewer * mediaviewer ) :
    guAlbumBrowser( parent, mediaviewer )
{
    Bind( wxEVT_MENU, &guJamendoAlbumBrowser::OnDownloadAlbum, this, ID_JAMENDO_DOWNLOAD_TORRENT_ALBUM );
    Bind( wxEVT_MENU, &guJamendoAlbumBrowser::OnDownloadAlbum, this, ID_JAMENDO_DOWNLOAD_DIRECT_ALBUM );
}

// -------------------------------------------------------------------------------- //
guJamendoAlbumBrowser::~guJamendoAlbumBrowser()
{
    Unbind( wxEVT_MENU, &guJamendoAlbumBrowser::OnDownloadAlbum, this, ID_JAMENDO_DOWNLOAD_TORRENT_ALBUM );
    Unbind( wxEVT_MENU, &guJamendoAlbumBrowser::OnDownloadAlbum, this, ID_JAMENDO_DOWNLOAD_DIRECT_ALBUM );
}

// -------------------------------------------------------------------------------- //
void guJamendoAlbumBrowser::OnDownloadAlbum( wxCommandEvent &event )
{
    guLogMessage( wxT( "OnDownloadAlbum" ) );
    if( m_LastAlbumBrowserItem )
    {
        wxArrayInt Albums;
        Albums.Add( m_LastAlbumBrowserItem->m_AlbumId );
        ( ( guMediaViewerJamendo * ) m_MediaViewer )->DownloadAlbums( Albums, ( event.GetId() == ID_JAMENDO_DOWNLOAD_TORRENT_ALBUM ) );
    }
}




// -------------------------------------------------------------------------------- //
// guJamendoTreePanel
// -------------------------------------------------------------------------------- //
guJamendoTreePanel::guJamendoTreePanel( wxWindow * parent, guMediaViewer * mediaviewer ) :
    guTreeViewPanel( parent, mediaviewer )
{
    Bind( wxEVT_MENU, &guJamendoTreePanel::OnDownloadAlbum, this, ID_JAMENDO_DOWNLOAD_TORRENT_ALBUM );
    Bind( wxEVT_MENU, &guJamendoTreePanel::OnDownloadAlbum, this, ID_JAMENDO_DOWNLOAD_DIRECT_ALBUM );
    Bind( wxEVT_MENU, &guJamendoTreePanel::OnDownloadTrackAlbum, this, ID_JAMENDO_DOWNLOAD_DIRECT_TRACK_ALBUM );
    Bind( wxEVT_MENU, &guJamendoTreePanel::OnDownloadTrackAlbum, this, ID_JAMENDO_DOWNLOAD_TORRENT_TRACK_ALBUM );
}

// -------------------------------------------------------------------------------- //
guJamendoTreePanel::~guJamendoTreePanel()
{
    Unbind( wxEVT_MENU, &guJamendoTreePanel::OnDownloadAlbum, this, ID_JAMENDO_DOWNLOAD_TORRENT_ALBUM );
    Unbind( wxEVT_MENU, &guJamendoTreePanel::OnDownloadAlbum, this, ID_JAMENDO_DOWNLOAD_DIRECT_ALBUM );
    Unbind( wxEVT_MENU, &guJamendoTreePanel::OnDownloadTrackAlbum, this, ID_JAMENDO_DOWNLOAD_DIRECT_TRACK_ALBUM );
    Unbind( wxEVT_MENU, &guJamendoTreePanel::OnDownloadTrackAlbum, this, ID_JAMENDO_DOWNLOAD_TORRENT_TRACK_ALBUM );
}

// -------------------------------------------------------------------------------- //
void guJamendoTreePanel::OnDownloadAlbum( wxCommandEvent &event )
{
    guLogMessage( wxT( "OnDownloadAlbum" ) );
    const wxTreeItemId &CurItemId = m_TreeViewCtrl->GetSelection();
    guTreeViewData * TreeViewData = ( guTreeViewData * ) m_TreeViewCtrl->GetItemData( CurItemId );
    int ItemType = TreeViewData->GetType();

    if( ItemType == guLIBRARY_ELEMENT_ALBUMS )
    {
        wxArrayInt Albums;
        Albums.Add( TreeViewData->m_Id );
        ( ( guMediaViewerJamendo * ) m_MediaViewer )->DownloadAlbums( Albums, ( event.GetId() == ID_JAMENDO_DOWNLOAD_TORRENT_ALBUM ) );
    }
}

// -------------------------------------------------------------------------------- //
void guJamendoTreePanel::OnDownloadTrackAlbum( wxCommandEvent &event )
{
    guLogMessage( wxT( "OnDownloadTrackAlbum" ) );

    guTrackArray Tracks;
    m_TVTracksListBox->GetSelectedSongs( &Tracks );

    wxArrayInt Albums;
    int Index;
    int Count = Tracks.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( Albums.Index( Tracks[ Index ].m_AlbumId ) == wxNOT_FOUND )
            Albums.Add( Tracks[ Index ].m_AlbumId );
    }

    ( ( guMediaViewerJamendo * ) m_MediaViewer )->DownloadAlbums( Albums, ( event.GetId() == ID_JAMENDO_DOWNLOAD_TORRENT_TRACK_ALBUM ) );
}




// -------------------------------------------------------------------------------- //
// guJamendoPlayListPanel
// -------------------------------------------------------------------------------- //
guJamendoPlayListPanel::guJamendoPlayListPanel( wxWindow * parent, guMediaViewer * mediaviewer ) :
    guPlayListPanel( parent, mediaviewer )
{
    Bind( wxEVT_MENU, &guJamendoPlayListPanel::OnDownloadTrackAlbum, this, ID_JAMENDO_DOWNLOAD_DIRECT_TRACK_ALBUM );
    Bind( wxEVT_MENU, &guJamendoPlayListPanel::OnDownloadTrackAlbum, this, ID_JAMENDO_DOWNLOAD_TORRENT_TRACK_ALBUM );
}

// -------------------------------------------------------------------------------- //
guJamendoPlayListPanel::~guJamendoPlayListPanel()
{
    Unbind( wxEVT_MENU, &guJamendoPlayListPanel::OnDownloadTrackAlbum, this, ID_JAMENDO_DOWNLOAD_DIRECT_TRACK_ALBUM );
    Unbind( wxEVT_MENU, &guJamendoPlayListPanel::OnDownloadTrackAlbum, this, ID_JAMENDO_DOWNLOAD_TORRENT_TRACK_ALBUM );
}

// -------------------------------------------------------------------------------- //
void guJamendoPlayListPanel::OnDownloadTrackAlbum( wxCommandEvent &event )
{
    guLogMessage( wxT( "OnDownloadTrackAlbum" ) );

    guTrackArray Tracks;
    m_PLTracksListBox->GetSelectedSongs( &Tracks );

    wxArrayInt Albums;
    int Index;
    int Count = Tracks.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( Albums.Index( Tracks[ Index ].m_AlbumId ) == wxNOT_FOUND )
            Albums.Add( Tracks[ Index ].m_AlbumId );
    }

    ( ( guMediaViewerJamendo * ) m_MediaViewer )->DownloadAlbums( Albums, ( event.GetId() == ID_JAMENDO_DOWNLOAD_TORRENT_TRACK_ALBUM ) );
}




// -------------------------------------------------------------------------------- //
// guMediaVIewerJamendo
// -------------------------------------------------------------------------------- //
guMediaViewerJamendo::guMediaViewerJamendo( wxWindow * parent, guMediaCollection &mediacollection,
        const int basecmd, guMainFrame * mainframe, const int mode, guPlayerPanel * playerpanel ) :
    guMediaViewer( parent, mediacollection, basecmd, mainframe, mode, playerpanel )
{
    m_DownloadThread = NULL;
    m_ContextMenuFlags = ( guCONTEXTMENU_DOWNLOAD_COVERS | guCONTEXTMENU_LINKS );

    Bind( wxEVT_MENU, &guMediaViewerJamendo::OnCoverDownloaded, this, ID_JAMENDO_COVER_DOWNLAODED );
    Bind( wxEVT_MENU, &guMediaViewerJamendo::OnUpdateFinished, this, ID_JAMENDO_UPDATE_FINISHED );

    InitMediaViewer( mode );
}

// -------------------------------------------------------------------------------- //
guMediaViewerJamendo::~guMediaViewerJamendo()
{
    Unbind( wxEVT_MENU, &guMediaViewerJamendo::OnCoverDownloaded, this, ID_JAMENDO_COVER_DOWNLAODED );
    Unbind( wxEVT_MENU, &guMediaViewerJamendo::OnUpdateFinished, this, ID_JAMENDO_UPDATE_FINISHED );
}

// -------------------------------------------------------------------------------- //
void guMediaViewerJamendo::LoadMediaDb( void )
{
    guLogMessage( wxT( "LoadMediaDb... JAMENDO..." ) );
    m_Db = new guJamendoLibrary( guPATH_COLLECTIONS + m_MediaCollection->m_UniqueId + wxT( "/guayadeque.db" ) );
    m_Db->SetMediaViewer( this );
}

// -------------------------------------------------------------------------------- //
void guMediaViewerJamendo::OnConfigUpdated( wxCommandEvent &event )
{
    guMediaViewer::OnConfigUpdated( event );

    if( event.GetInt() & guPREFERENCE_PAGE_FLAG_JAMENDO )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        if( Config->ReadBool( CONFIG_KEY_JAMENDO_NEED_UPGRADE, false, CONFIG_PATH_JAMENDO ) )
        {
            UpdateLibrary();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewerJamendo::UpdateLibrary( void )
{
    int GaugeId;
    GaugeId = m_MainFrame->AddGauge( m_MediaCollection->m_Name );

    if( m_UpdateThread )
    {
        m_UpdateThread->Pause();
        m_UpdateThread->Delete();
    }

    m_UpdateThread = new guJamendoUpdateThread( this, guJAMENDO_ACTION_UPDATE, GaugeId );
}

// -------------------------------------------------------------------------------- //
void guMediaViewerJamendo::UpgradeLibrary( void )
{
    int GaugeId;
    GaugeId = m_MainFrame->AddGauge( m_MediaCollection->m_Name );

    if( m_UpdateThread )
    {
        m_UpdateThread->Pause();
        m_UpdateThread->Delete();
    }

    m_UpdateThread = new guJamendoUpdateThread( this, guJAMENDO_ACTION_UPGRADE, GaugeId );
}

// -------------------------------------------------------------------------------- //
void guMediaViewerJamendo::NormalizeTracks( guTrackArray * tracks, const bool isdrag )
{
    int Index;
    int Count;
    if( tracks && ( Count = tracks->Count() ) )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        int AudioFormat = Config->ReadNum( CONFIG_KEY_JAMENDO_AUDIOFORMAT, 1, CONFIG_PATH_JAMENDO );
        for( Index = 0; Index < Count; Index++ )
        {
            guTrack * Track = &( * tracks )[ Index ];
            Track->m_FileName = wxString::Format( guJAMENDO_FILE_STREAM_URL, Track->m_SongId );
            Track->m_FileName += AudioFormat ? guJAMENDO_STREAM_FORMAT_OGG : guJAMENDO_STREAM_FORMAT_MP3;
            Track->m_Type = guTRACK_TYPE_JAMENDO;
            if( isdrag )
                Track->m_FileName.Replace( wxT( "http://" ), wxT( "/" ) );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewerJamendo::AddDownload( const int albumid, const bool iscover )
{
    //guLogMessage( wxT( "Error... %i" ), 1000 / 0 );
    wxMutexLocker Lock( m_DownloadThreadMutex );

    if( !m_DownloadThread )
    {
        m_DownloadThread = new guJamendoDownloadThread( this );

        if( !m_DownloadThread )
        {
            guLogMessage( wxT( "Could not create the jamendo download thread" ) );
            return;
        }
    }

    m_DownloadThread->AddAlbum( albumid, iscover );
}

// -------------------------------------------------------------------------------- //
void guMediaViewerJamendo::AddDownloads( const wxArrayInt &albumids, const bool iscover )
{
    wxMutexLocker Lock( m_DownloadThreadMutex );

    if( !m_DownloadThread )
    {
        m_DownloadThread = new guJamendoDownloadThread( this );

        if( !m_DownloadThread )
        {
            guLogMessage( wxT( "Could not create the jamendo download thread" ) );
            return;
        }
    }

    m_DownloadThread->AddAlbums( albumids, iscover );
}

// -------------------------------------------------------------------------------- //
void guMediaViewerJamendo::OnCoverDownloaded( wxCommandEvent &event )
{
    AlbumCoverChanged( event.GetInt() );
}

// -------------------------------------------------------------------------------- //
void guMediaViewerJamendo::EndUpdateThread( void )
{
    m_UpdateThread = NULL;
}

// -------------------------------------------------------------------------------- //
void guMediaViewerJamendo::EndDownloadThread( void )
{
    wxMutexLocker Lock( m_DownloadThreadMutex );
    m_DownloadThread = NULL;
}

// -------------------------------------------------------------------------------- //
void guMediaViewerJamendo::OnUpdateFinished( wxCommandEvent &event )
{
    EndUpdateThread();

    LibraryUpdated();
}

// -------------------------------------------------------------------------------- //
void guMediaViewerJamendo::DownloadAlbumCover( const int albumid )
{
    AddDownload( albumid, true );
}

// -------------------------------------------------------------------------------- //
void guMediaViewerJamendo::CreateContextMenu( wxMenu * menu, const int windowid )
{
    wxMenu * Menu = new wxMenu();
    wxMenuItem * MenuItem;

    int BaseCommand = GetBaseCommand();

    if( ( windowid == guLIBRARY_ELEMENT_ALBUMS ) )
    {
        MenuItem = new wxMenuItem( menu, ID_JAMENDO_DOWNLOAD_DIRECT_ALBUM, _( "Download Albums" ), _( "Download the current selected album" ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( menu, ID_JAMENDO_DOWNLOAD_TORRENT_ALBUM, _( "Download Albums torrents" ), _( "Download the current selected album" ) );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();
    }
    else if( ( windowid == guLIBRARY_ELEMENT_TRACKS ) )
    {
        MenuItem = new wxMenuItem( menu, ID_JAMENDO_DOWNLOAD_DIRECT_TRACK_ALBUM, _( "Download Albums" ), _( "Download the current selected album" ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( menu, ID_JAMENDO_DOWNLOAD_TORRENT_TRACK_ALBUM, _( "Download Albums torrents" ), _( "Download the current selected album" ) );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();
    }

    MenuItem = new wxMenuItem( Menu, BaseCommand + guCOLLECTION_ACTION_UPDATE_LIBRARY, _( "Update" ), _( "Update the collection library" ), wxITEM_NORMAL );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, BaseCommand + guCOLLECTION_ACTION_RESCAN_LIBRARY, _( "Rescan" ), _( "Rescan the collection library" ), wxITEM_NORMAL );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, BaseCommand + guCOLLECTION_ACTION_SEARCH_COVERS, _( "Search Covers" ), _( "Search the collection missing covers" ), wxITEM_NORMAL );
    Menu->Append( MenuItem );

    Menu->AppendSeparator();

    MenuItem = new wxMenuItem( Menu, BaseCommand + guCOLLECTION_ACTION_VIEW_PROPERTIES, _( "Properties" ), _( "Show collection properties" ), wxITEM_NORMAL );
    Menu->Append( MenuItem );

    menu->AppendSeparator();
    menu->AppendSubMenu( Menu, wxT( "Jamendo" ) );
}

// -------------------------------------------------------------------------------- //
bool guMediaViewerJamendo::CreateLibraryView( void )
{
    guLogMessage( wxT( "CreateLibraryView... Jamendo...") );
    m_LibPanel = new guJamendoPanel( this, this );
    m_LibPanel->SetBaseCommand( m_BaseCommand + 1 );
    return true;
}

// -------------------------------------------------------------------------------- //
bool guMediaViewerJamendo::CreateAlbumBrowserView( void )
{
    m_AlbumBrowser = new guJamendoAlbumBrowser( this, this );
    return true;
}

// -------------------------------------------------------------------------------- //
bool guMediaViewerJamendo::CreateTreeView( void )
{
    m_TreeViewPanel = new guJamendoTreePanel( this, this );
    return true;
}

// -------------------------------------------------------------------------------- //
bool guMediaViewerJamendo::CreatePlayListView( void )
{
    m_PlayListPanel = new guJamendoPlayListPanel( this, this );
    return true;
}

// -------------------------------------------------------------------------------- //
wxImage * guMediaViewerJamendo::GetAlbumCover( const int albumid, int &coverid,
                wxString &coverpath, const wxString &artistname, const wxString &albumname )
{
    wxImage * CoverImage = guMediaViewer::GetAlbumCover( albumid, coverid, coverpath, artistname, albumname );
    if( CoverImage )
        return CoverImage;

    wxString CoverFile = guPATH_JAMENDO_COVERS + GetCoverName( albumid ) + wxT( ".jpg" );
    if( wxFileExists( CoverFile ) )
    {
        wxImage * CoverImage = new wxImage( CoverFile, wxBITMAP_TYPE_JPEG );
        if( CoverImage )
        {
            if( CoverImage->IsOk() )
            {
                coverpath = CoverFile;

                SetAlbumCover( albumid, guPATH_JAMENDO_COVERS, coverpath );

                coverid = m_Db->GetAlbumCoverId( albumid );

                return CoverImage;
            }
            delete CoverImage;
        }
    }

    AddDownload( albumid );

    return NULL;
}

// -------------------------------------------------------------------------------- //
wxString guMediaViewerJamendo::GetCoverName( const int albumid )
{
    return wxString::Format( wxT( "%u" ), albumid );
}

// -------------------------------------------------------------------------------- //
void guMediaViewerJamendo::SelectAlbumCover( const int albumid )
{
    guSelCoverFile * SelCoverFile = new guSelCoverFile( this, this, albumid );
    if( SelCoverFile )
    {
        if( SelCoverFile->ShowModal() == wxID_OK )
        {
            wxString CoverFile = SelCoverFile->GetSelFile();
            if( !CoverFile.IsEmpty() )
            {
                if( SetAlbumCover( albumid, guPATH_JAMENDO_COVERS, CoverFile ) )
                {
                    if( SelCoverFile->EmbedToFiles() )
                    {
                        EmbedAlbumCover( albumid );
                    }
                }
            }
        }
        SelCoverFile->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewerJamendo::DownloadAlbums( const wxArrayInt &albums, const bool istorrent )
{
    int Index;
    int Count;
    if( ( Count = albums.Count() ) )
    {
        if( istorrent )
        {
            guConfig * Config = ( guConfig * ) guConfig::Get();
            wxString TorrentCmd = Config->ReadStr( CONFIG_KEY_JAMENDO_TORRENT_COMMAND, wxEmptyString, CONFIG_PATH_JAMENDO );
            if( TorrentCmd.IsEmpty() )
            {
                //OnEditSetup( event );
                return;
            }

            AddDownloads( albums, false );
        }
        else
        {
            for( Index = 0; Index < Count; Index++ )
            {
                guWebExecute( wxString::Format( guJAMENDO_DOWNLOAD_DIRECT, albums[ Index ] ) );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
bool guMediaViewerJamendo::FindMissingCover( const int albumid, const wxString &artistname, const wxString &albumname, const wxString &albumpath )
{
    AddDownload( albumid, true );
    return true;
}

// -------------------------------------------------------------------------------- //
void guMediaViewerJamendo::EditProperties( void )
{
    wxCommandEvent CmdEvent( wxEVT_MENU, ID_MENU_PREFERENCES );
    CmdEvent.SetInt( guPREFERENCE_PAGE_JAMENDO );
    wxPostEvent( m_MainFrame, CmdEvent );
}

}

// -------------------------------------------------------------------------------- //
