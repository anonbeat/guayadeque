// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2011 J.Rios
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
#include "Magnatune.h"

#include "TagInfo.h"
#include "MainFrame.h"
#include "SelCoverFile.h"
#include "StatusBar.h"
#include "Utils.h"

#include <wx/wfstream.h>
#include <wx/mstream.h>
#include <wx/tokenzr.h>
#include <wx/zstream.h>
#include <wx/xml/xml.h>

#include <id3v1genres.h>


// -------------------------------------------------------------------------------- //
// guMagnatuneLibrary
// -------------------------------------------------------------------------------- //
guMagnatuneLibrary::guMagnatuneLibrary( const wxString &dbname ) :
    guDbLibrary( dbname )
{
}

// -------------------------------------------------------------------------------- //
guMagnatuneLibrary::~guMagnatuneLibrary()
{
}

// -------------------------------------------------------------------------------- //
void guMagnatuneLibrary::UpdateArtistsLabels( const guArrayListItems &labelsets )
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
void guMagnatuneLibrary::UpdateSongsLabels( const guArrayListItems &labelsets )
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
void guMagnatuneLibrary::UpdateAlbumsLabels( const guArrayListItems &labelsets )
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
void guMagnatuneLibrary::CreateNewSong( guTrack * track, const wxString &albumsku, const wxString &coverlink )
{
    wxString query;
    //wxSQLite3ResultSet dbRes;

    track->m_SongId = FindTrack( track->m_ArtistName, track->m_SongName );
    if( track->m_SongId != wxNOT_FOUND )
    {
      query = wxString::Format( wxT( "UPDATE songs SET song_name = '%s', "
                                 "song_genreid = %u, song_genre = '%s', "
                                 "song_artistid = %u, song_artist = '%s', "
                                 "song_albumid = %u, song_album = '%s', "
                                 "song_pathid = %u, song_path = '%s', "
                                 "song_filename = '%s', "
                                 "song_number = %u, song_year = %u, "
                                 "song_length = %u, "
                                 "song_albumsku = '%s', "
                                 "song_coverlink = '%s' "
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
                    escape_query_str( albumsku ).c_str(),
                    escape_query_str( coverlink ).c_str(),
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
                    "song_filesize, song_albumsku, song_coverlink ) VALUES( NULL, 0, %u, '%s', %u, '%s', %u, '%s', %u, '%s', "
                    "%u, '%s', '%s', 'mp3,ogg', %u, %u, %u, '%s', %u, 0, 0, -1, 0, '%s', '%s' )" ),
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
                    track->m_Length,
                    escape_query_str( albumsku ).c_str(),
                    escape_query_str( coverlink ).c_str() );

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
int guMagnatuneLibrary::GetTrackId( const wxString &url, guTrack * track )
{
    wxString query;
    wxSQLite3ResultSet dbRes;
    int RetVal = wxNOT_FOUND;

    query = GU_TRACKS_QUERYSTR +
            wxString::Format( wxT( " WHERE song_filename = '%s' LIMIT 1;" ),
                    escape_query_str( url.BeforeLast( '.' ) ).c_str() );
    //guLogMessage( wxT( "Searching:\n%s" ), query.c_str() );
    dbRes = ExecuteQuery( query );
    if( dbRes.NextRow() )
    {
      RetVal = dbRes.GetInt( 0 );
      if( track )
      {
          FillTrackFromDb( track, &dbRes );
      }
    }
    dbRes.Finalize();

    return RetVal;
}

// -------------------------------------------------------------------------------- //
wxString guMagnatuneLibrary::GetAlbumSku( const int albumid )
{
    wxString query;
    wxSQLite3ResultSet dbRes;
    wxString RetVal;

    query = wxString::Format( wxT( "SELECT song_albumsku FROM songs WHERE song_albumid = %i LIMIT 1;" ), albumid );
    //guLogMessage( wxT( "Searching:\n%s" ), query.c_str() );
    dbRes = ExecuteQuery( query );
    if( dbRes.NextRow() )
    {
      RetVal = dbRes.GetString( 0 );
    }
    dbRes.Finalize();

    return RetVal;
}


// -------------------------------------------------------------------------------- //
// guMagnatunePanel
// -------------------------------------------------------------------------------- //
guMagnatunePanel::guMagnatunePanel( wxWindow * parent, guMagnatuneLibrary * db, guPlayerPanel * playerpanel, const wxString &prefix ) :
    guLibPanel( parent, db, playerpanel, prefix )
{
    SetBaseCommand( ID_MENU_VIEW_MAGNATUNE );

    InitPanelData();

    m_ContextMenuFlags = ( guLIBRARY_CONTEXTMENU_DOWNLOAD_COVERS | guLIBRARY_CONTEXTMENU_LINKS );

    m_UpdateThread = NULL;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->RegisterObject( this ); // Get notified when configuration changes

    Connect( ID_MAGNATUNE_EDIT_GENRES, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMagnatunePanel::OnEditSetup ), NULL, this );
    Connect( ID_MAGNATUNE_SETUP, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMagnatunePanel::OnEditSetup ), NULL, this );
    Connect( ID_MAGNATUNE_UPDATE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMagnatunePanel::OnUpdate ), NULL, this );
    Connect( ID_MAGNATUNE_UPGRADE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMagnatunePanel::OnUpgrade ), NULL, this );
    Connect( ID_MAGNATUNE_DOWNLOAD_DIRECT_ALBUM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMagnatunePanel::OnDownloadAlbum ), NULL, this );
    Connect( ID_MAGNATUNE_DOWNLOAD_DIRECT_TRACK_ALBUM, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMagnatunePanel::OnDownloadTrackAlbum ), NULL, this );

    Connect( ID_MAGNATUNE_COVER_DOWNLAODED, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guMagnatunePanel::OnCoverDownloaded ), NULL, this );

    Connect( ID_CONFIG_UPDATED, guConfigUpdatedEvent, wxCommandEventHandler( guMagnatunePanel::OnConfigUpdated ), NULL, this );

    m_Membership = Config->ReadNum( wxT( "Membership" ), 0, wxT( "Magnatune" ) );
    m_UserName = Config->ReadStr( wxT( "Username" ), wxEmptyString, wxT( "Magnatune" ) );
    m_Password = Config->ReadStr( wxT( "Password" ), wxEmptyString, wxT( "Magnatune" ) );
    if( m_UserName.IsEmpty() || m_Password.IsEmpty() )
    {
        m_Membership = 0;
    }
    wxArrayString AllowedGenres = Config->ReadAStr( wxT( "Genre" ), wxEmptyString, wxT( "MagnatuneGenres" ) );
    if( AllowedGenres.IsEmpty() )
    {
        wxCommandEvent event;
        OnEditSetup( event );
    }
}

// -------------------------------------------------------------------------------- //
guMagnatunePanel::~guMagnatunePanel()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->UnRegisterObject( this );
}

// -------------------------------------------------------------------------------- //
void guMagnatunePanel::InitPanelData( void )
{
    m_PanelCmdIds.Empty();
    m_PanelCmdIds.Add( ID_MENU_VIEW_MAGNATUNE_TEXTSEARCH );
    m_PanelCmdIds.Add( ID_MENU_VIEW_MAGNATUNE_LABELS );
    m_PanelCmdIds.Add( ID_MENU_VIEW_MAGNATUNE_GENRES );
    m_PanelCmdIds.Add( ID_MENU_VIEW_MAGNATUNE_ARTISTS );
    m_PanelCmdIds.Add( ID_MENU_VIEW_MAGNATUNE_COMPOSERS );
    m_PanelCmdIds.Add( ID_MENU_VIEW_MAGNATUNE_ALBUMARTISTS );
    m_PanelCmdIds.Add( ID_MENU_VIEW_MAGNATUNE_ALBUMS );
    m_PanelCmdIds.Add( ID_MENU_VIEW_MAGNATUNE_YEARS );
    m_PanelCmdIds.Add( ID_MENU_VIEW_MAGNATUNE_RATINGS );
    m_PanelCmdIds.Add( ID_MENU_VIEW_MAGNATUNE_PLAYCOUNT );
}

// -------------------------------------------------------------------------------- //
void guMagnatunePanel::NormalizeTracks( guTrackArray * tracks, const bool isdrag )
{
    int Index;
    int Count;
    if( tracks && ( Count = tracks->Count() ) )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        int AudioFormat = Config->ReadNum( wxT( "AudioFormat" ), 1, wxT( "Magnatune" ) );
        for( Index = 0; Index < Count; Index++ )
        {
            guTrack * Track = &( * tracks )[ Index ];
            //guLogMessage( wxT( "'%s'" ), Track->m_FileName.c_str() );
            if( !Track->m_FileName.EndsWith( AudioFormat ? guMAGNATUNE_STREAM_FORMAT_OGG : guMAGNATUNE_STREAM_FORMAT_MP3 ) )
            {
                Track->m_FileName = Track->m_FileName.Mid( Track->m_FileName.Find( wxT( "http://he3." ) ) );
                if( m_Membership > guMAGNATUNE_MEMBERSHIP_FREE ) // Streaming or Downloading
                {
                    Track->m_FileName.Replace( wxT( "//he3." ), wxT( "//" ) + m_UserName + wxT( ":" ) +
                        m_Password + ( ( m_Membership == guMAGNATUNE_MEMBERSHIP_STREAM ) ? wxT( "@stream." ) : wxT( "@download." ) ) );

                    Track->m_FileName += wxT( "_nospeech" );
                }

                Track->m_FileName += AudioFormat ? guMAGNATUNE_STREAM_FORMAT_OGG : guMAGNATUNE_STREAM_FORMAT_MP3;
                Track->m_Type = guTRACK_TYPE_MAGNATUNE;
                if( isdrag )
                    Track->m_FileName.Replace( wxT( "http://" ), wxT( "/" ) );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMagnatunePanel::CreateContextMenu( wxMenu * menu, const int windowid )
{
    wxMenu *     SubMenu;
    SubMenu = new wxMenu();

    if( m_Membership == guMAGNATUNE_MEMBERSHIP_DOWNLOAD )
    {
        if( ( windowid == guLIBRARY_ELEMENT_ALBUMS ) )
        {
            wxMenuItem * MenuItem = new wxMenuItem( menu, ID_MAGNATUNE_DOWNLOAD_DIRECT_ALBUM, _( "Download Albums" ), _( "Download the current selected album" ) );
            SubMenu->Append( MenuItem );

            SubMenu->AppendSeparator();
        }
        else if( ( windowid == guLIBRARY_ELEMENT_TRACKS ) )
        {
            wxMenuItem * MenuItem = new wxMenuItem( menu, ID_MAGNATUNE_DOWNLOAD_DIRECT_TRACK_ALBUM, _( "Download Albums" ), _( "Download the current selected album" ) );
            SubMenu->Append( MenuItem );

            SubMenu->AppendSeparator();
        }
    }

    wxMenuItem * MenuItem = new wxMenuItem( menu, ID_MAGNATUNE_UPDATE, _( "Update Database" ), _( "Download the latest Magnatune database" ) );
    SubMenu->Append( MenuItem );

    MenuItem = new wxMenuItem( menu, ID_MAGNATUNE_EDIT_GENRES, _( "Select Genres" ), _( "Selects the enabled Magnatune genres" ) );
    SubMenu->Append( MenuItem );

    MenuItem = new wxMenuItem( menu, ID_MAGNATUNE_SETUP, _( "Preferences" ), _( "Configure the Magnatune options" ) );
    SubMenu->Append( MenuItem );

    menu->AppendSeparator();
    menu->AppendSubMenu( SubMenu, _( "Magnatune" ), _( "Global Magnatune options" ) );
}

// -------------------------------------------------------------------------------- //
void guMagnatunePanel::OnEditSetup( wxCommandEvent &event )
{
    wxCommandEvent CmdEvent( wxEVT_COMMAND_MENU_SELECTED, ID_MENU_PREFERENCES );
    CmdEvent.SetInt( guPREFERENCE_PAGE_MAGNATUNE );
    wxPostEvent( wxTheApp->GetTopWindow(), CmdEvent );
}

// -------------------------------------------------------------------------------- //
void guMagnatunePanel::StartUpdateTracks( const int action )
{
    wxMutexLocker Lock( m_UpdateThreadMutex );

    guMainFrame * MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
    guStatusBar * StatusBar = ( guStatusBar * ) MainFrame->GetStatusBar();
    int GaugeId = StatusBar->AddGauge( _( "Magnatune" ) );
    if( m_UpdateThread )
    {
        m_UpdateThread->Pause();
        m_UpdateThread->Delete();
    }

    m_UpdateThread = new guMagnatuneUpdateThread( ( guMagnatuneLibrary * ) m_Db,
                                action, GaugeId );
    if( !m_UpdateThread )
    {
        guLogError( wxT( "Could not create the Magnatune update thread" ) );
        StatusBar->RemoveGauge( GaugeId );
    }
}

// -------------------------------------------------------------------------------- //
void guMagnatunePanel::EndUpdateThread( void )
{
    wxMutexLocker Lock( m_UpdateThreadMutex );
    m_UpdateThread = NULL;
}

// -------------------------------------------------------------------------------- //
void guMagnatunePanel::OnUpdate( wxCommandEvent &event )
{
    StartUpdateTracks( guMAGNATUNE_ACTION_UPDATE );
}

// -------------------------------------------------------------------------------- //
void guMagnatunePanel::OnUpgrade( wxCommandEvent &event )
{
    StartUpdateTracks( guMAGNATUNE_ACTION_UPGRADE );
}

// -------------------------------------------------------------------------------- //
void guMagnatunePanel::OnConfigUpdated( wxCommandEvent &event )
{
    if( event.GetInt() & guPREFERENCE_PAGE_FLAG_MAGNATUNE )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        m_Membership = Config->ReadNum( wxT( "Membership" ), 0, wxT( "Magnatune" ) );
        m_UserName = Config->ReadStr( wxT( "Username" ), wxEmptyString, wxT( "Magnatune" ) );
        m_Password = Config->ReadStr( wxT( "Password" ), wxEmptyString, wxT( "Magnatune" ) );
        if( m_UserName.IsEmpty() || m_Password.IsEmpty() )
        {
            m_Membership = 0;
        }
        bool DoUpgrade = Config->ReadBool( wxT( "NeedUpgrade" ), false, wxT( "Magnatune" ) );

        if( DoUpgrade )
        {
            OnUpgrade( event );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMagnatunePanel::AddDownload( const int albumid, const wxString &artist, const wxString &album )
{
    guMagnatuneDownloadThread * DownloadThread = new guMagnatuneDownloadThread( this, albumid, artist, album );

    if( !DownloadThread )
    {
        guLogMessage( wxT( "Could not create the magnatune download thread" ) );
    }
}

// -------------------------------------------------------------------------------- //
wxImage * guMagnatunePanel::GetAlbumCover( const int albumid, const wxString &artist, const wxString &album, wxString &coverpath )
{
    wxFileName CoverFile = wxFileName( wxGetHomeDir() + wxT( "/.guayadeque/Magnatune/Covers/" ) +
                 wxString::Format( wxT( "%s-%s.jpg" ), artist.c_str(), album.c_str() ) );

    if( CoverFile.Normalize( wxPATH_NORM_ALL|wxPATH_NORM_CASE ) )
    {
        if( CoverFile.FileExists() )
        {
            wxImage * CoverImage = new wxImage( CoverFile.GetFullPath(), wxBITMAP_TYPE_JPEG );
            if( CoverImage )
            {
                if( CoverImage->IsOk() )
                {
                    coverpath = CoverFile.GetFullPath();
                    return CoverImage;
                }
                delete CoverImage;
            }
        }
    }
    AddDownload( albumid, artist, album );
    return NULL;
}

// -------------------------------------------------------------------------------- //
void guMagnatunePanel::OnCoverDownloaded( wxCommandEvent &event )
{
    int AlbumId =  event.GetInt();
    if( AlbumId )
    {
        ReloadAlbums( false );
        wxPostEvent( wxTheApp->GetTopWindow(), event );
    }
}

// -------------------------------------------------------------------------------- //
void guMagnatunePanel::OnAlbumDownloadCoverClicked( wxCommandEvent &event )
{
    wxArrayInt Albums = m_AlbumListCtrl->GetSelectedItems();
    if( Albums.Count() )
    {
        wxString Artist;
        wxString Album;
        if( m_Db->GetAlbumInfo( Albums[ 0 ], &Album, &Artist, NULL ) )
        {
            AddDownload( Albums[ 0 ], Artist, Album );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMagnatunePanel::OnAlbumSelectCoverClicked( wxCommandEvent &event )
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
                    wxString CoverName = wxGetHomeDir() + wxT( "/.guayadeque/Magnatune/Covers/" );
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

//<RESULT>
//  <CC_AMOUNT>$</CC_AMOUNT>
//  <CC_TRANSID></CC_TRANSID>
//  <DL_PAGE>http://download.magnatune.com/artists/albums/satori-rainsleep/download</DL_PAGE>
//  <URL_WAVZIP>http://download.magnatune.com/member/api_download.php?sku=satori-rainsleep&amp;filename=wav.zip&amp;path=http://download.magnatune.com/artists/music/Ambient/Satori/Sleep%20Under%20The%20Rain/wav.zip</URL_WAVZIP>
//  <URL_128KMP3ZIP>http://download.magnatune.com/member/api_download.php?sku=satori-rainsleep&amp;filename=mp3.zip&amp;path=http://download.magnatune.com/artists/music/Ambient/Satori/Sleep%20Under%20The%20Rain/mp3.zip</URL_128KMP3ZIP>
//  <URL_OGGZIP>http://download.magnatune.com/member/api_download.php?sku=satori-rainsleep&amp;filename=Satori%20-%20Sleep%20Under%20The%20Rain%20-%20ogg.zip&amp;path=http://download.magnatune.com/artists/music/Ambient/Satori/Sleep%20Under%20The%20Rain/satori-rainsleep-ogg.zip</URL_OGGZIP>
//  <URL_VBRZIP>http://download.magnatune.com/member/api_download.php?sku=satori-rainsleep&amp;filename=Satori%20-%20Sleep%20Under%20The%20Rain%20-%20vbr.zip&amp;path=http://download.magnatune.com/artists/music/Ambient/Satori/Sleep%20Under%20The%20Rain/satori-rainsleep-vbr.zip</URL_VBRZIP>
//  <URL_FLACZIP>http://download.magnatune.com/member/api_download.php?sku=satori-rainsleep&amp;filename=Satori%20-%20Sleep%20Under%20The%20Rain%20-%20flac.zip&amp;path=http://download.magnatune.com/artists/music/Ambient/Satori/Sleep%20Under%20The%20Rain/satori-rainsleep-flac.zip</URL_FLACZIP>
//  <DL_MSG>
//Go here to download your music:
//http://download.magnatune.com/artists/albums/satori-rainsleep/download
//----------
//Give this album away for free!
//You can pass these download instructions on to 3 friends. It's completely legal
//and you'll be helping us fight the evil music industry!  Complete details here:
//http://magnatune.com/info/give
//Help spread the word of Magnatune, with our free recruiting cards!
//http://magnatune.com/cards
//Podcasters, use our music:<br>
//http://magnatune.com/info/podcast
//To send us email:
//https://magnatune.com/info/email_us
//  </DL_MSG>
//</RESULT>

// -------------------------------------------------------------------------------- //
void guMagnatunePanel::DownloadAlbums( const wxArrayInt &albumids )
{
    wxString StartLabel[] = {
        wxT( "<DL_PAGE>" ),
        wxT( "<URL_VBRZIP>" ),
        wxT( "<URL_128KMP3ZIP>" ),
        wxT( "<URL_OGGZIP>" ),
        wxT( "<URL_FLACZIP>" ),
        wxT( "<URL_WAVZIP>" ),
    };
    wxString EndLabel[] = {
        wxT( "</DL_PAGE>" ),
        wxT( "</URL_VBRZIP>" ),
        wxT( "</URL_128KMP3ZIP>" ),
        wxT( "</URL_OGGZIP>" ),
        wxT( "</URL_FLACZIP>" ),
        wxT( "</URL_WAVZIP>" ),
    };

    int Index;
    int Count;
    if( ( Count = albumids.Count() ) )
    {
        for( Index = 0; Index < Count; Index++ )
        {
            wxString DownloadUrl = wxString::Format( guMAGNATUNE_DOWNLOAD_URL,
                                m_UserName.c_str(), m_Password.c_str(),
                                ( ( guMagnatuneLibrary * ) m_Db )->GetAlbumSku( albumids[ Index ] ).c_str() );

            //guLogMessage( wxT( "Trying to download %s" ), DownloadUrl.c_str() );
            wxString Content = GetUrlContent( DownloadUrl );
            if( !Content.IsEmpty() )
            {
                //guLogMessage( wxT( "Result:\n%s" ), Content.c_str() );
                guConfig * Config = ( guConfig * ) guConfig::Get();
                int DownloadFormat = Config->ReadNum( wxT( "DownloadFormat" ), 0, wxT( "Magnatune" ) );
                //guLogMessage( wxT( "DownloadFormat: %i %s  %s" ), DownloadFormat,
                //             StartLabel[ DownloadFormat ].c_str(),
                //             EndLabel[ DownloadFormat ].c_str() );

                if( ( DownloadFormat < 0 ) || ( DownloadFormat > 5 ) )
                {
                    DownloadFormat = 0;
                }

                DownloadUrl = wxEmptyString;

                int Pos = Content.Find( StartLabel[ DownloadFormat ] );
                if( Pos != wxNOT_FOUND )
                {
                    DownloadUrl = Content.Mid( Pos );

                    if( DownloadFormat > 0 )
                        DownloadUrl = DownloadUrl.Mid( DownloadUrl.Find( wxT( "path=http://" ) ) + 12 );
                    else
                        DownloadUrl = DownloadUrl.Mid( StartLabel[ DownloadFormat ].Length() + 7 );
                    DownloadUrl = DownloadUrl.Mid( 0, DownloadUrl.Find( EndLabel[ DownloadFormat ] ) );
                }

                if( !DownloadUrl.IsEmpty() )
                {
                    DownloadUrl = wxString::Format( wxT( "http://%s:%s@" ), m_UserName.c_str(), m_Password.c_str() ) + DownloadUrl;
                    //guLogMessage( wxT( "Trying to download the url : '%s'" ), DownloadUrl.c_str() );
                    guWebExecute( DownloadUrl );
                }
            }
            else
            {
                guLogError( wxT( "Could not get the album download info" ) );
            }
        }
    }
}
// -------------------------------------------------------------------------------- //
void guMagnatunePanel::OnDownloadAlbum( wxCommandEvent &event )
{
    wxArrayInt Albums = m_AlbumListCtrl->GetSelectedItems();
    DownloadAlbums( Albums );
}

// -------------------------------------------------------------------------------- //
void guMagnatunePanel::OnDownloadTrackAlbum( wxCommandEvent &event )
{
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

    DownloadAlbums( Albums );
}

// -------------------------------------------------------------------------------- //
// guMagnatuneDownloadThread
// -------------------------------------------------------------------------------- //
guMagnatuneDownloadThread::guMagnatuneDownloadThread( guMagnatunePanel * magnatunepanel,
                const int albumid, const wxString &artist, const wxString &album )
{
    m_MagnatunePanel = magnatunepanel;
    m_Db = magnatunepanel->GetMagnatuneDb();
    m_ArtistName = artist;
    m_AlbumName = album;
    m_AlbumId = albumid;

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guMagnatuneDownloadThread::~guMagnatuneDownloadThread()
{
}

// -------------------------------------------------------------------------------- //
guMagnatuneDownloadThread::ExitCode guMagnatuneDownloadThread::Entry()
{
    wxFileName CoverFile = wxFileName( wxGetHomeDir() + wxT( "/.guayadeque/Magnatune/Covers/" ) +
                 wxString::Format( wxT( "%s-%s.jpg" ), m_ArtistName.c_str(), m_AlbumName.c_str() ) );

    if( CoverFile.Normalize( wxPATH_NORM_ALL|wxPATH_NORM_CASE ) )
    {
        wxString CoverUrl = wxString::Format( wxT( "http://he3.magnatune.com/music/%s/%s/cover_600.jpg" ),
                m_ArtistName.c_str(),
                m_AlbumName.c_str() );

        if( !wxDirExists( wxPathOnly( CoverFile.GetFullPath() ) + wxT( "/" ) ) )
        {
            wxMkdir( wxPathOnly( CoverFile.GetFullPath() ) + wxT( "/" ), 0770 );
        }

        DownloadImage( CoverUrl, CoverFile.GetFullPath(), 300 );

        if( CoverFile.FileExists() )
        {
            int CoverId = m_Db->AddCoverFile( CoverFile.GetFullPath() );

            wxString query = wxString::Format( wxT( "UPDATE songs SET song_coverid = %u WHERE song_albumid = %u" ),
                                CoverId, m_AlbumId );

            m_Db->ExecuteUpdate( query );

            // Notify the panel that the cover is downloaded
            wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_MAGNATUNE_COVER_DOWNLAODED );
            event.SetInt( m_AlbumId );
            wxPostEvent( m_MagnatunePanel, event );
        }
        else
        {
            guLogMessage( wxT( "Could not get the magnatune cover art %s" ), CoverFile.GetFullPath().c_str() );
        }

    }
    return 0;
}

// -------------------------------------------------------------------------------- //
// guMagnatuneUpdateThread
// -------------------------------------------------------------------------------- //
guMagnatuneUpdateThread::guMagnatuneUpdateThread( guMagnatuneLibrary * db, int action, int gaugeid )
{
    m_Db = db;
    m_MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
    m_Action = action;
    m_GaugeId = gaugeid;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    //m_LastUpdate = Config->ReadNum( wxT( "MagnatuneLastUpdate" ), 0, wxT( "General" ) );
    m_AllowedGenres = Config->ReadAStr( wxT( "Genre" ), wxEmptyString, wxT( "MagnatuneGenres" ) );

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 10 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guMagnatuneUpdateThread::~guMagnatuneUpdateThread()
{
    //
    guMainFrame * MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_STATUSBAR_GAUGE_REMOVE );
    event.SetInt( m_GaugeId );
    wxPostEvent( MainFrame, event );

    if( !TestDestroy() )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        if( Config )
        {
            wxDateTime Now = wxDateTime::Now();
            Config->WriteNum( wxT( "MagnatuneLastUpdate" ), Now.GetTicks(), wxT( "General" ) );
            Config->Flush();
        }

        event.SetId( ID_MAGNATUNE_UPDATE_FINISHED );
        event.SetEventObject( ( wxObject * ) this );
        wxPostEvent( MainFrame, event );

        m_MainFrame->GetMagnatunePanel()->EndUpdateThread();
    }
}

#if 0
<AllAlbums>
    <Album>
        <artist>Dr Kuch</artist>
		<artistdesc>Fun electro-poppy dance up and down tempo chill out</artistdesc>
		<cover_small>http://he3.magnatune.com/music/Dr%20Kuch/We%20Can&#39;t%20Stop%20Progress/cover_200.jpg</cover_small>
        <artistphoto>http://magnatune.com//artists/img/drkuch_cover_small.jpg</artistphoto>
		<albumname>We Can&#39;t Stop Progress</albumname>
		<year>2009</year>
		<album_notes></album_notes>
		<mp3genre>(52)</mp3genre>
		<home>http://magnatune.com/artists/dr_kuch</home>
		<buy>https://magnatune.com/artists/buy_album?artist=Dr+Kuch&amp;album=We+Can%27t+Stop+Progress&amp;genre=Electronica</buy>
		<magnatunegenres>Electronica,Euro-Techno</magnatunegenres>
		<launchdate>2009-10-08</launchdate>
		<albumsku>drkuch-progress</albumsku>
        <Track>
            <artist>Dr Kuch</artist>
            <artistphoto>http://magnatune.com//artists/img/drkuch_cover_small.jpg</artistphoto>
            <artistdesc>Fun electro-poppy dance up and down tempo chill out</artistdesc>
            <albumname>We Can&#39;t Stop Progress</albumname>
            <trackname>Intro</trackname>
            <tracknum>1</tracknum>
            <year>2009</year>
            <mp3genre>(52)</mp3genre>
            <magnatunegenres>Electronica,Euro-Techno</magnatunegenres>
            <license>http://creativecommons.org/licenses/by-nc-sa/1.0/</license>
            <seconds>18</seconds>
            <url>http://he3.magnatune.com/all/01-Intro-Dr%20Kuch.mp3</url>
            <mp3lofi>http://he3.magnatune.com/all/01-Intro-Dr%20Kuch-lofi.mp3</mp3lofi>
            <oggurl>http://he3.magnatune.com/all/01-Intro-Dr%20Kuch.ogg</oggurl>
            <buy>https://magnatune.com/artists/buy_album?artist=Dr+Kuch&amp;album=We+Can%27t+Stop+Progress&amp;genre=Electronica</buy>
            <home>http://magnatune.com/artists/dr_kuch</home>
            <launchdate>2009-10-08</launchdate>
            <cover_small>http://he3.magnatune.com/music/Dr%20Kuch/We%20Can&#39;t%20Stop%20Progress/cover_200.jpg</cover_small>
            <albumsku>drkuch-progress</albumsku>
        </Track>
        ...
    </Album>
    ...
</AllAlbums>
#endif
// -------------------------------------------------------------------------------- //
bool inline IsGenreEnabled( const wxArrayString &genrelist, const wxString &current )
{
    wxArrayString CurrentGenres = wxStringTokenize( current, wxT( "," ) );
    int Index;
    int Count = CurrentGenres.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( genrelist.Index( CurrentGenres[ Index ] ) != wxNOT_FOUND )
            return true;
    }
    return false;
}



// -------------------------------------------------------------------------------- //
void guMagnatuneUpdateThread::AddGenres( const wxString &genres )
{
    wxArrayString Genres = wxStringTokenize( genres, wxT( "," ) );
    int Index;
    int Count = Genres.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( m_GenreList.Index( Genres[ Index ] ) == wxNOT_FOUND )
        {
            m_GenreList.Add( Genres[ Index ]  );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMagnatuneUpdateThread::ReadMagnatuneXmlTrack( wxXmlNode * xmlnode )
{
    long Id;
    while( xmlnode && !TestDestroy() )
    {
        wxString ItemName = xmlnode->GetName();
        if( ItemName == wxT( "trackname" ) )
        {
            m_CurrentTrack.m_SongName = xmlnode->GetNodeContent().Trim( true ).Trim( false );
        }
        else if( ItemName == wxT( "tracknum" ) )
        {
            xmlnode->GetNodeContent().ToLong( &Id );
            m_CurrentTrack.m_Number = Id;
        }
        else if( ItemName == wxT( "year" ) )
        {
            xmlnode->GetNodeContent().ToLong( &Id );
            m_CurrentTrack.m_Year = Id;
        }
        else if( ItemName == wxT( "seconds" ) )
        {
            xmlnode->GetNodeContent().ToLong( &Id );
            m_CurrentTrack.m_Length = Id;
        }
        else if( ItemName == wxT( "url" ) )
        {
            m_CurrentTrack.m_FileName = xmlnode->GetNodeContent();
            m_CurrentTrack.m_FileName.Replace( wxT( ".mp3" ), wxT( "" ) );
        }
        else if( ItemName == wxT( "magnatunegenres" ) )
        {
            m_CurrentTrack.m_GenreName = xmlnode->GetNodeContent();
            m_CurrentTrack.m_GenreId = m_Db->GetGenreId( m_CurrentTrack.m_GenreName );
            AddGenres( m_CurrentTrack.m_GenreName );
        }
        else if( ItemName == wxT( "cover_small" ) )
        {
            //<cover_small>http://he3.magnatune.com/music/Dr%20Kuch/We%20Can&#39;t%20Stop%20Progress/cover_200.jpg</cover_small>
            m_CoverLink = xmlnode->GetNodeContent();
            m_CoverLink.Replace( wxT( "cover_200" ), wxT( "cover_600" ) );
        }
        else if( ItemName == wxT( "albumsku" ) )
        {
            m_AlbumSku = xmlnode->GetNodeContent();
        }
        xmlnode = xmlnode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void guMagnatuneUpdateThread::ReadMagnatuneXmlAlbum( wxXmlNode * xmlnode )
{
    long Id;
    while( xmlnode && !TestDestroy() )
    {
        wxString ItemName = xmlnode->GetName();
        if( ItemName == wxT( "artist" ) )
        {
            m_CurrentTrack.m_ArtistName = xmlnode->GetNodeContent().Trim( true ).Trim( false );
            m_CurrentTrack.m_ArtistId = m_Db->GetArtistId( m_CurrentTrack.m_ArtistName );
        }
        else if( ItemName == wxT( "albumname" ) )
        {
            m_CurrentTrack.m_AlbumName = xmlnode->GetNodeContent().Trim( true ).Trim( false );
            m_CurrentTrack.m_AlbumId = 0;
            //guLogMessage( wxT( "%s" ), m_CurrentTrack.m_AlbumName.c_str() );
        }
        else if( ItemName == wxT( "year" ) )
        {
            xmlnode->GetNodeContent().ToLong( &Id );
            m_CurrentTrack.m_Year = Id;
        }
        else if( ItemName == wxT( "magnatunegenres" ) )
        {
            m_CurrentTrack.m_GenreName = xmlnode->GetNodeContent();
            m_CurrentTrack.m_GenreId = m_Db->GetGenreId( m_CurrentTrack.m_GenreName );
            AddGenres( m_CurrentTrack.m_GenreName );
        }
        else if( ItemName == wxT( "Track" ) )
        {
            m_AlbumSku = wxEmptyString;
            m_CoverLink = wxEmptyString;
            if( !m_CurrentTrack.m_AlbumId )
            {
                m_CurrentTrack.m_Path = m_CurrentTrack.m_GenreName + wxT( "/" ) +
                                m_CurrentTrack.m_ArtistName + wxT( "/" ) +
                                m_CurrentTrack.m_AlbumName + wxT( "/" );
                m_CurrentTrack.m_PathId = m_Db->GetPathId( m_CurrentTrack.m_Path );
                m_CurrentTrack.m_AlbumId = m_Db->GetAlbumId( m_CurrentTrack.m_AlbumName, m_CurrentTrack.m_ArtistId, m_CurrentTrack.m_PathId, m_CurrentTrack.m_Path );
            }
            ReadMagnatuneXmlTrack( xmlnode->GetChildren() );

            if( IsGenreEnabled( m_AllowedGenres, m_CurrentTrack.m_GenreName ) )
            {
                m_Db->CreateNewSong( &m_CurrentTrack, m_AlbumSku, m_CoverLink );
            }
        }
        xmlnode = xmlnode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
bool guMagnatuneUpdateThread::UpdateDatabase( void )
{
    if( DownloadFile( guMAGNATUNE_DATABASE_DUMP_URL, wxGetHomeDir() + wxT( "/.guayadeque/Magnatune/album_info_xml.gz" ) ) )
    {
        wxFileInputStream Ins( wxGetHomeDir() + wxT( "/.guayadeque/Magnatune/album_info_xml.gz" ) );
        if( Ins.IsOk() )
        {
            wxZlibInputStream ZIn( Ins );
            if( ZIn.IsOk() )
            {
                wxFileOutputStream ZOuts( wxGetHomeDir() + wxT( "/.guayadeque/Magnatune/album_info.xml" ) );
                if( ZOuts.IsOk() )
                {
                    ZIn.Read( ZOuts );
                    return true;
                }
            }
        }
        else
        {
            guLogError( wxT( "Could not open the Magnatune local database copy" ) );
        }
    }
    return false;
}

// -------------------------------------------------------------------------------- //
guMagnatuneUpdateThread::ExitCode guMagnatuneUpdateThread::Entry()
{
    wxString query;

    wxCommandEvent evtup( wxEVT_COMMAND_MENU_SELECTED, ID_STATUSBAR_GAUGE_UPDATE );
    evtup.SetInt( m_GaugeId );

    wxCommandEvent evtmax( wxEVT_COMMAND_MENU_SELECTED, ID_STATUSBAR_GAUGE_SETMAX );
    evtmax.SetInt( m_GaugeId );

    if( m_Action == guMAGNATUNE_ACTION_UPGRADE &&
        !wxFileExists( wxGetHomeDir() + wxT( "/.guayadeque/Magnatune/album_info.xml" ) ) )
    {
        m_Action = guMAGNATUNE_ACTION_UPDATE;
    }

    guLogMessage( wxT( "Starting the Magnatune Update process..." ) );
    if( !TestDestroy() && ( m_Action == guMAGNATUNE_ACTION_UPGRADE || UpdateDatabase() ) )
    {
        wxFile XmlFile( wxGetHomeDir() + wxT( "/.guayadeque/Magnatune/album_info.xml" ), wxFile::read );
        if( XmlFile.IsOpened() )
        {
            guListItems CurrentGenres;
            m_Db->GetGenres( &CurrentGenres, true );

            wxArrayInt GenresToDel;
            int Index;
            int Count = CurrentGenres.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                if( !IsGenreEnabled( m_AllowedGenres, CurrentGenres[ Index ].m_Name ) )
                {
                    GenresToDel.Add( CurrentGenres[ Index ].m_Id );
                }
            }

            if( GenresToDel.Count() )
            {
                query = wxT( "DELETE FROM songs WHERE " ) + ArrayToFilter( GenresToDel, wxT( "song_genreid" ) );
                //guLogMessage( wxT( "%s" ), query.c_str() );
                m_Db->ExecuteUpdate( query );
            }


            evtmax.SetExtraLong( XmlFile.Length() );
            wxPostEvent( wxTheApp->GetTopWindow(), evtmax );

            wxFileOffset CurPos;
            query = wxT( "BEGIN TRANSACTION" );
            m_Db->ExecuteUpdate( query );

            //query = wxT( "DELETE FROM songs" );
            //m_Db->ExecuteUpdate( query );

            if( m_AllowedGenres.Count() )
            {
                wxString AlbumChunk = guGetNextXMLChunk( XmlFile, CurPos, "<Album>", "</Album>" );
                while( !TestDestroy() && !AlbumChunk.IsEmpty() )
                {
                    wxHtmlEntitiesParser EntitiesParser;
                    AlbumChunk = EntitiesParser.Parse( AlbumChunk );
                    AlbumChunk.Replace( wxT( "&" ), wxT( "&amp;" ) );

                    wxStringInputStream Ins( AlbumChunk );
                    wxXmlDocument XmlDoc( Ins );
                    if( XmlDoc.IsOk() )
                    {
                        wxXmlNode * XmlNode = XmlDoc.GetRoot();

                        if( XmlNode && XmlNode->GetName() == wxT( "Album" ) )
                        {
                            ReadMagnatuneXmlAlbum( XmlNode->GetChildren() );
                        }
                    }
                    else
                    {
                        guLogMessage( wxT( "Error in album chunk:\n%s" ), AlbumChunk.c_str() );
                    }

                    AlbumChunk = guGetNextXMLChunk( XmlFile, CurPos, "<Album>", "</Album>" );
                    evtup.SetExtraLong( CurPos );
                    wxPostEvent( wxTheApp->GetTopWindow(), evtup );
                }

                if( m_GenreList.Count() )
                {
                    guConfig * Config = ( guConfig * ) guConfig::Get();
                    Config->WriteAStr( wxT( "Genre" ), m_GenreList, wxT( "MagnatuneGenreList" ) );
                    Config->Flush();
                }
            }

            query = wxT( "END TRANSACTION" );
            m_Db->ExecuteUpdate( query );

            XmlFile.Close();
        }
    }

    return 0;
}

// -------------------------------------------------------------------------------- //
