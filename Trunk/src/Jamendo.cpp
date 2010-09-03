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

    query = wxString::Format( wxT( "SELECT song_id FROM songs WHERE song_id = %i LIMIT 1" ), track->m_SongId );
    dbRes = ExecuteQuery( query );

    if( dbRes.NextRow() )
    {
      query = wxString::Format( wxT( "UPDATE songs SET song_name = '%s', "
                                 "song_genreid = %u, song_genre = '%s', "
                                 "song_artistid = %u, song_artist = '%s', "
                                 "song_albumid = %u, song_album = '%s', "
                                 "song_pathid = %u, song_path = '%s', "
                                 "song_filename = '%s', "
                                 "song_number = %u, song_year = %u, "
                                 "song_length = %u "
                                 "WHERE song_id = %u;" ),
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
                    track->m_Length,
                    track->m_SongId );

        ExecuteUpdate( query );
    }
    else
    {
        wxString query = wxString::Format( wxT( "INSERT INTO songs( "
                    "song_id, song_playcount, song_addedtime, "
                    "song_name, song_genreid, song_genre, song_artistid, song_artist, "
                    "song_albumid, song_album, song_pathid, song_path, song_filename, song_format, song_number, song_year, "
                    "song_coverid, song_disk, song_length, song_offset, song_bitrate, song_rating, "
                    "song_filesize ) VALUES( %u, 0, %u, '%s', %u, '%s', %u, '%s', %u, '%s', "
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
guJamendoPanel::guJamendoPanel( wxWindow * parent, guJamendoLibrary * db, guPlayerPanel * playerpanel, const wxString &prefix ) :
    guLibPanel( parent, db, playerpanel, prefix )
{
    SetBaseCommand( ID_MENU_VIEW_JAMENDO );

    m_ContextMenuFlags = ( guLIBRARY_CONTEXTMENU_DOWNLOAD_COVERS | guLIBRARY_CONTEXTMENU_LINKS );

    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->RegisterObject( this ); // Get notified when configuration changes

    Connect( ID_JAMENDO_EDIT_GENRES, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guJamendoPanel::OnEditSetup ), NULL, this );
    Connect( ID_JAMENDO_SETUP, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guJamendoPanel::OnEditSetup ), NULL, this );
    Connect( ID_JAMENDO_UPDATE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guJamendoPanel::OnUpdate ), NULL, this );
    Connect( ID_JAMENDO_UPGRADE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guJamendoPanel::OnUpgrade ), NULL, this );

    Connect( ID_JAMENDO_COVER_DOWNLAODED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guJamendoPanel::OnCoverDownloaded ), NULL, this );

    Connect( ID_CONFIG_UPDATED, guConfigUpdatedEvent, wxCommandEventHandler( guJamendoPanel::OnConfigUpdated ), NULL, this );
}

// -------------------------------------------------------------------------------- //
guJamendoPanel::~guJamendoPanel()
{
}

// -------------------------------------------------------------------------------- //
void guJamendoPanel::NormalizeTracks( guTrackArray * tracks )
{
    int Index;
    int Count;
    if( tracks && ( Count = tracks->Count() ) )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        int AudioFormat = Config->ReadNum( wxT( "AudioFormat" ), 1, wxT( "Jamendo" ) );
        for( Index = 0; Index < Count; Index++ )
        {
            guTrack * Track = &( * tracks )[ Index ];
            Track->m_FileName = wxString::Format( guJAMENDO_FILE_STREAM_URL, Track->m_SongId );
            Track->m_FileName += AudioFormat ? guJAMENDO_FILE_FORMAT_OGG : guJAMENDO_FILE_FORMAT_MP3;
            Track->m_Type = guTRACK_TYPE_JAMENDO;
        }
    }
}

// -------------------------------------------------------------------------------- //
void guJamendoPanel::CreateContextMenu( wxMenu * menu )
{
    wxMenu *     SubMenu;
    SubMenu = new wxMenu();

    wxMenuItem * MenuItem = new wxMenuItem( menu, ID_JAMENDO_UPDATE, _( "Update" ), _( "Download the latest Jamendo database" ) );
    SubMenu->Append( MenuItem );

    MenuItem = new wxMenuItem( menu, ID_JAMENDO_EDIT_GENRES, _( "Genres" ), _( "Selects the enabled Jamendo genres" ) );
    SubMenu->Append( MenuItem );

    MenuItem = new wxMenuItem( menu, ID_JAMENDO_SETUP, _( "Options" ), _( "Configure the Jamendo options" ) );
    SubMenu->Append( MenuItem );

    menu->AppendSeparator();
    menu->AppendSubMenu( SubMenu, _( "Jamendo" ), _( "Global Jamendo options" ) );
}

// -------------------------------------------------------------------------------- //
void guJamendoPanel::OnEditSetup( wxCommandEvent &event )
{
    wxCommandEvent CmdEvent( wxEVT_COMMAND_MENU_SELECTED, ID_MENU_PREFERENCES );
    CmdEvent.SetInt( guPREFERENCE_PAGE_JAMENDO );
    wxPostEvent( wxTheApp->GetTopWindow(), CmdEvent );
}

// -------------------------------------------------------------------------------- //
void guJamendoPanel::OnUpdate( wxCommandEvent &event )
{
    guMainFrame * MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
    guStatusBar * StatusBar = ( guStatusBar * ) MainFrame->GetStatusBar();
    int GaugeId = StatusBar->AddGauge( _( "Jamendo" ) );
    guJamendoUpdateThread * JamendoUpdateThread = new guJamendoUpdateThread( ( guJamendoLibrary * ) m_Db,
                                guJAMENDO_ACTION_UPDATE, GaugeId );
    if( !JamendoUpdateThread )
    {
        guLogError( wxT( "Could not create the Jamendo update thread" ) );
        StatusBar->RemoveGauge( GaugeId );
    }
}

// -------------------------------------------------------------------------------- //
void guJamendoPanel::OnUpgrade( wxCommandEvent &event )
{
    guMainFrame * MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
    guStatusBar * StatusBar = ( guStatusBar * ) MainFrame->GetStatusBar();
    int GaugeId = StatusBar->AddGauge( _( "Jamendo" ) );
    guJamendoUpdateThread * JamendoUpdateThread = new guJamendoUpdateThread( ( guJamendoLibrary * ) m_Db,
                                guJAMENDO_ACTION_UPGRADE, GaugeId );
    if( !JamendoUpdateThread )
    {
        guLogError( wxT( "Could not create the Jamendo upgrade thread" ) );
        StatusBar->RemoveGauge( GaugeId );
    }
}

// -------------------------------------------------------------------------------- //
void guJamendoPanel::OnConfigUpdated( wxCommandEvent &event )
{
    if( event.GetInt() & guPREFERENCE_PAGE_FLAG_JAMENDO )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        bool DoUpgrade = Config->ReadBool( wxT( "NeedUpgrade" ), false, wxT( "Jamendo" ) );

        if( DoUpgrade )
        {
            OnUpgrade( event );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guJamendoPanel::DownloadCover( const int albumid )
{
    guJamendoDownloadCoverThread * DownloadCoverThread = new guJamendoDownloadCoverThread( this );
    if( !DownloadCoverThread )
    {
        guLogMessage( wxT( "Could not create the jamendo download cover thread" ) );
        return;
    }
    DownloadCoverThread->AddAlbum( albumid );
}

// -------------------------------------------------------------------------------- //
wxImage * guJamendoPanel::GetAlbumCover( const int albumid, wxString &coverpath )
{
    wxString CoverFile = wxGetHomeDir() + wxT( "/.guayadeque/Jamendo/Covers/" );
    CoverFile += wxString::Format( wxT( "%u.jpg" ), albumid );
    if( wxFileExists( CoverFile ) )
    {
        wxImage * CoverImage = new wxImage( CoverFile, wxBITMAP_TYPE_JPEG );
        if( CoverImage )
        {
            if( CoverImage->IsOk() )
            {
                coverpath = CoverFile;
                return CoverImage;
            }
            delete CoverImage;
        }
    }
    DownloadCover( albumid );
    return NULL;
}

// -------------------------------------------------------------------------------- //
void guJamendoPanel::OnCoverDownloaded( wxCommandEvent &event )
{
    int AlbumId =  event.GetInt();
    if( AlbumId )
    {
        ReloadAlbums( false );
        wxPostEvent( wxTheApp->GetTopWindow(), event );
    }
}


// -------------------------------------------------------------------------------- //
void guJamendoPanel::OnAlbumDownloadCoverClicked( wxCommandEvent &event )
{
    wxArrayInt Albums = m_AlbumListCtrl->GetSelectedItems();
    if( Albums.Count() )
    {
        DownloadCover( Albums[ 0 ] );
    }
}

// -------------------------------------------------------------------------------- //
void guJamendoPanel::OnAlbumSelectCoverClicked( wxCommandEvent &event )
{
    wxArrayInt Albums = m_AlbumListCtrl->GetSelectedItems();
    if( Albums.Count() )
    {
        int AlbumId = Albums[ 0 ];
        guSelCoverFile * SelCoverFile = new guSelCoverFile( this, m_Db, AlbumId );
        if( SelCoverFile )
        {
            if( SelCoverFile->ShowModal() == wxID_OK )
            {
                wxString CoverFile = SelCoverFile->GetSelFile();
                if( !CoverFile.IsEmpty() )
                {
                    guConfig * Config = ( guConfig * ) guConfig::Get();
                    wxArrayString SearchCovers = Config->ReadAStr( wxT( "Word" ), wxEmptyString, wxT( "CoverSearch" ) );
                    wxString CoverName = wxGetHomeDir() + wxT( "/.guayadeque/Jamendo/Covers/" );
                    CoverName += wxString::Format( wxT( "%u.jpg" ), AlbumId );

                    wxURI Uri( CoverFile );
                    if( Uri.IsReference() )
                    {
                        wxImage CoverImage( CoverFile );
                        if( CoverImage.IsOk() )
                        {
                            if( ( CoverFile == CoverName ) || CoverImage.SaveFile( CoverName, wxBITMAP_TYPE_JPEG ) )
                            {
                                m_Db->SetAlbumCover( AlbumId, CoverName );
                                ReloadAlbums( false );
                            }
                        }
                        else
                        {
                            guLogError( wxT( "Could not load the imate '%s'" ), CoverFile.c_str() );
                        }
                    }
                    else
                    {
                        if( DownloadImage( CoverFile, CoverName ) )
                        {
                            m_Db->SetAlbumCover( AlbumId, CoverName );
                            ReloadAlbums( false );
                        }
                        else
                        {
                            guLogError( wxT( "Failed to download file '%s'" ), CoverFile.c_str() );
                        }
                    }
                }
            }
            delete SelCoverFile;
        }
    }
}



// -------------------------------------------------------------------------------- //
// guJamendoDownloadCoverThread
// -------------------------------------------------------------------------------- //
guJamendoDownloadCoverThread::guJamendoDownloadCoverThread( guJamendoPanel * jamendopanel )
{
    m_JamendoPanel = jamendopanel;
    m_Db = jamendopanel->GetJamendoDb();

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guJamendoDownloadCoverThread::~guJamendoDownloadCoverThread()
{
}

// -------------------------------------------------------------------------------- //
void guJamendoDownloadCoverThread::AddAlbum( const int albumid )
{
    m_AlbumsMutex.Lock();
    m_Albums.Add( albumid );
    m_AlbumsMutex.Unlock();
}

// -------------------------------------------------------------------------------- //
guJamendoDownloadCoverThread::ExitCode guJamendoDownloadCoverThread::Entry()
{
    int Count;
    int LoopCount = 0;
    while( !TestDestroy() )
    {
        m_AlbumsMutex.Lock();
        Count = m_Albums.Count();
        m_AlbumsMutex.Unlock();

        size_t CurTime = wxGetLocalTimeMillis().GetLo();
        if( Count )
        {
            LoopCount = 0;
            wxString CoverFile = wxGetHomeDir() + wxT( "/.guayadeque/Jamendo/Covers/" );
            CoverFile += wxString::Format( wxT( "%u.jpg" ), m_Albums[ 0 ] );

            if( !wxFileExists( CoverFile ) )
            {
                if( !wxDirExists( wxPathOnly( CoverFile ) + wxT( "/" ) ) )
                {
                    wxMkdir( wxPathOnly( CoverFile ) + wxT( "/" ), 0770 );
                }
                wxString CoverUrl = wxString::Format( guJAMENDO_COVER_DOWNLOAD_URL, m_Albums[ 0 ], 300 );
                guLogMessage( wxT( "Downloading: %s" ), CoverUrl.c_str() );
                DownloadImage( CoverUrl, CoverFile, 300 );
            }

            if( wxFileExists( CoverFile ) )
            {
                int CoverId = m_Db->AddCoverFile( CoverFile );

                wxString query = wxString::Format( wxT( "UPDATE songs SET song_coverid = %u WHERE song_albumid = %u" ),
                                    CoverId, m_Albums[ 0 ] );

                m_Db->ExecuteUpdate( query );

                // Notify the panel that the cover is downloaded
                wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_JAMENDO_COVER_DOWNLAODED );
                event.SetInt( m_Albums[ 0 ] );
                wxPostEvent( m_JamendoPanel, event );
            }
            else
            {
                guLogMessage( wxT( "Could not get the jamendo cover art %s" ), CoverFile.c_str() );
            }

            m_AlbumsMutex.Lock();
            m_Albums.RemoveAt( 0 );
            m_AlbumsMutex.Unlock();
        }
        else
        {
            LoopCount++;
            if( LoopCount > 3 )
            {
                break;
            }
        }

        size_t Elapsed = wxGetLocalTimeMillis().GetLo() - CurTime;
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
guJamendoUpdateThread::guJamendoUpdateThread( guJamendoLibrary * db, int action, int gaugeid )
{
    m_Db = db;
    m_MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
    m_Action = action;
    m_GaugeId = gaugeid;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    //m_LastUpdate = Config->ReadNum( wxT( "JamendoLastUpdate" ), 0, wxT( "General" ) );
    m_AllowedGenres = Config->ReadANum( wxT( "Genre" ), 0, wxT( "JamendoGenres" ) );

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 10 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guJamendoUpdateThread::~guJamendoUpdateThread()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        wxDateTime Now = wxDateTime::Now();
        Config->WriteNum( wxT( "JamendoLastUpdate" ), Now.GetTicks(), wxT( "General" ) );
        Config->Flush();
    }

    guMainFrame * MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_JAMENDO_UPDATE_FINISHED );
    event.SetEventObject( ( wxObject * ) this );
    wxPostEvent( MainFrame, event );

    //
    event.SetId( ID_GAUGE_REMOVE );
    event.SetInt( m_GaugeId );
    wxPostEvent( MainFrame, event );
}

#if 0
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
#endif


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
            track->m_Length = Id;
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
                db->CreateNewSong( track );
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
bool guJamendoUpdateThread::UpdateDatabase( void )
{
    if( DownloadFile( guJAMENDO_DATABASE_DUMP_URL, wxGetHomeDir() + wxT( "/.guayadeque/Jamendo/dbdump_artistalbumtrack.xml.gz" ) ) )
    {
        wxFileInputStream Ins( wxGetHomeDir() + wxT( "/.guayadeque/Jamendo/dbdump_artistalbumtrack.xml.gz" ) );
        if( Ins.IsOk() )
        {
            wxZlibInputStream ZIn( Ins );
            if( ZIn.IsOk() )
            {
                wxFileOutputStream ZOuts( wxGetHomeDir() + wxT( "/.guayadeque/Jamendo/dbdump_artistalbumtrack.xml" ) );
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
wxString GetNextArtistChunk( wxFile &xmlfile, wxFileOffset &CurPos )
{
//    #define XMLREAD_BUFFER_SIZE     2000000
//    wxString RetVal;
//    char * Buffer = ( char * ) malloc( XMLREAD_BUFFER_SIZE );
//    wxFileOffset CurPos = xmlfile.Tell();
//    int ReadCnt = xmlfile.Read( Buffer, XMLREAD_BUFFER_SIZE - 1 );
//    if( ReadCnt != wxInvalidOffset )
//    {
//        Buffer[ ReadCnt ] = 0;
//        char * StartPos = strstr( Buffer, "<artist>" );
//        char * EndPos;
//        if( StartPos )
//        {
//            EndPos = strstr( StartPos, "</artist>" );
//            if( EndPos )
//            {
//                EndPos[ 9 ] = 0;
//                xmlfile.Seek( CurPos + ( EndPos - Buffer + 9 ) );
//                //guLogMessage( wxT( "CurPos : %llu  => %llu" ), CurPos, CurPos + ( EndPos - Buffer + 9 ) );
//                RetVal = wxString( StartPos, wxConvUTF8 );
//            }
//            else
//            {
//                guLogMessage( wxT( "Could not find the artist end tag" ) );
//            }
//        }
//        else
//        {
//            guLogMessage( wxT( "Could not find the artist start tag" ) );
//        }
//    }
//    free( Buffer );

    #define XMLREAD_BUFFER_SIZE     2048
    wxString RetVal;
    //wxFileOffset CurPos = xmlfile.Tell();
    CurPos = xmlfile.Tell();
    wxFileOffset StartPos = wxNOT_FOUND;
    wxFileOffset EndPos = wxNOT_FOUND;
    char * Buffer = ( char * ) malloc( XMLREAD_BUFFER_SIZE + 1 );
    if( Buffer )
    {
        while( StartPos == wxNOT_FOUND )
        {
            int ReadCount = xmlfile.Read( Buffer, XMLREAD_BUFFER_SIZE );
            if( ReadCount != wxInvalidOffset && ReadCount > 0 )
            {
                Buffer[ ReadCount ] = 0;
            }
            else
            {
                break;
            }
            char * StartString = strstr( Buffer, "<artist>" );
            if( StartString )
            {
                StartPos = CurPos + ( StartString - Buffer );
                break;
            }
            else
            {
                CurPos += ReadCount;
            }
        }

        if( StartPos != wxNOT_FOUND )
        {
            xmlfile.Seek( StartPos );
            CurPos = StartPos;
            while( EndPos == wxNOT_FOUND )
            {
                int ReadCount = xmlfile.Read( Buffer, XMLREAD_BUFFER_SIZE );
                if( ReadCount != wxInvalidOffset && ReadCount > 0 )
                {
                    Buffer[ ReadCount ] = 0;
                }
                else
                {
                    break;
                }
                char * EndString = strstr( Buffer, "</artist>" );
                if( EndString )
                {
                    EndPos = CurPos + ( EndString - Buffer ) + 9;
                    break;
                }
                else
                {
                    // Prevent that </artist was partially included
                    CurPos += ReadCount - 8;
                    xmlfile.Seek( -8, wxFromCurrent );
                }
            }

            if( EndPos != wxNOT_FOUND )
            {
                //guLogMessage( wxT( "Found From %lli => %lli  (%lli)" ), StartPos, EndPos, EndPos - StartPos );
                int BufferSize = EndPos - StartPos;
                if( BufferSize )
                {
                    char * BufferString = ( char * ) malloc( BufferSize + 1 );
                    if( BufferString )
                    {
                        xmlfile.Seek( StartPos );
                        int ReadCount = xmlfile.Read( BufferString, BufferSize );
                        if( ReadCount != wxInvalidOffset )
                        {
                            BufferString[ ReadCount ] = 0;
                            RetVal = wxString( BufferString, wxConvUTF8 );
                            //guLogMessage( wxT( "%s" ), RetVal.c_str() );
                        }

                        free( BufferString );
                    }
                }
            }
        }

        free( Buffer );
    }


    return RetVal;
}

// -------------------------------------------------------------------------------- //
guJamendoUpdateThread::ExitCode guJamendoUpdateThread::Entry()
{
    wxString query;

    wxCommandEvent evtup( wxEVT_COMMAND_MENU_SELECTED, ID_GAUGE_UPDATE );
    evtup.SetInt( m_GaugeId );

    wxCommandEvent evtmax( wxEVT_COMMAND_MENU_SELECTED, ID_GAUGE_SETMAX );
    evtmax.SetInt( m_GaugeId );

    if( m_Action == guJAMENDO_ACTION_UPGRADE &&
        !wxFileExists( wxGetHomeDir() + wxT( "/.guayadeque/Jamendo/dbdump_artistalbumtrack.xml" ) ) )
    {
        m_Action = guJAMENDO_ACTION_UPDATE;
    }

    guLogMessage( wxT( "Starting the Jamendo Update process..." ) );
    if( !TestDestroy() && ( m_Action == guJAMENDO_ACTION_UPGRADE || UpdateDatabase() ) )
    {
        wxFile XmlFile( wxGetHomeDir() + wxT( "/.guayadeque/Jamendo/dbdump_artistalbumtrack.xml" ), wxFile::read );
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

            if( GenresToDel.Count() )
            {
                query = wxT( "DELETE FROM songs WHERE " ) + ArrayToFilter( GenresToDel, wxT( "song_genreid" ) );
                guLogMessage( wxT( "%s" ), query.c_str() );
                m_Db->ExecuteUpdate( query );
            }

            evtmax.SetExtraLong( XmlFile.Length() );
            wxPostEvent( wxTheApp->GetTopWindow(), evtmax );

            wxFileOffset CurPos;
            query = wxT( "BEGIN TRANSACTION" );
            m_Db->ExecuteUpdate( query );

            wxString ArtistChunk = GetNextArtistChunk( XmlFile, CurPos );
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

                ArtistChunk = GetNextArtistChunk( XmlFile, CurPos );
                evtup.SetExtraLong( CurPos );
                wxPostEvent( wxTheApp->GetTopWindow(), evtup );
            }

            query = wxT( "END TRANSACTION" );
            m_Db->ExecuteUpdate( query );

            XmlFile.Close();
        }
    }

    return 0;
}

// -------------------------------------------------------------------------------- //
