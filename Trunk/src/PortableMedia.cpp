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
#include "PortableMedia.h"

#include "CoverEdit.h"
#include "MainFrame.h"
#include "MD5.h"
#include "SelCoverFile.h"
#include "TagInfo.h"
#include "Transcode.h"
#include "Utils.h"


#include <wx/tokenzr.h>

//// -------------------------------------------------------------------------------- //
//wxString guPortableMediaAudioFormatString[] = {
//    wxT( "Keep format" ),
//    wxT( "mp3" ),
//    wxT( "ogg" ),
//    wxT( "flac" ),
//    wxT( "m4a" ),
//    wxT( "wma" )
//};
//
// -------------------------------------------------------------------------------- //
wxString guPortablePlaylistFormatString[] = {
    wxT( "m3u" ),
    wxT( "pls" ),
    wxT( "xspf" ),
    wxT( "asx" )
};

// -------------------------------------------------------------------------------- //
wxString guPortableCoverFormatString[] = {
    wxT( "embedded" ),
    wxT( "jpeg" ),
    wxT( "png" ),
    wxT( "bmp" ),
    wxT( "gif" )
};


// -------------------------------------------------------------------------------- //
int guGetTranscodeFileFormat( const wxString &filetype )
{
    if( filetype == wxT( "mp3" ) )
    {
        return guPORTABLEMEDIA_AUDIO_FORMAT_MP3;
    }
    else if( filetype == wxT( "ogg" ) || filetype == wxT( "oga" ) )
    {
        return guPORTABLEMEDIA_AUDIO_FORMAT_OGG;
    }
    else if( filetype == wxT( "flac" ) )
    {
        return guPORTABLEMEDIA_AUDIO_FORMAT_FLAC;
    }
    else if( filetype == wxT( "m4a" ) || filetype == wxT( "m4b" ) ||
             filetype == wxT( "aac" ) || filetype == wxT( "mp4" ) ||
             filetype == wxT( "m4p" ) )
    {
        return guPORTABLEMEDIA_AUDIO_FORMAT_AAC;
    }
    else if( filetype == wxT( "wma" ) )
    {
        return guPORTABLEMEDIA_AUDIO_FORMAT_WMA;
    }
    return guTRANSCODE_FORMAT_KEEP;
}

// -------------------------------------------------------------------------------- //
int MimeStrToAudioFormat( const wxString &mimestr )
{
    int Formats = 0;
    int Index;
    int Count;
    wxArrayString AudioFormats = wxStringTokenize( mimestr, wxT( "," ) );
    if( ( Count = AudioFormats.Count() ) )
    {
        for( Index = 0; Index < Count; Index++ )
        {
            AudioFormats[ Index ].Trim( true ).Trim( false );
            if( AudioFormats[ Index ] == wxT( "audio/mpeg" ) )
            {
                Formats |= guPORTABLEMEDIA_AUDIO_FORMAT_MP3;
            }
            else if( AudioFormats[ Index ] == wxT( "audio/ogg" ) )
            {
                Formats |= guPORTABLEMEDIA_AUDIO_FORMAT_OGG;
            }
            else if( AudioFormats[ Index ] == wxT( "audio/flac" ) )
            {
                Formats |= guPORTABLEMEDIA_AUDIO_FORMAT_FLAC;
            }
            else if( AudioFormats[ Index ] == wxT( "audio/mp4a-latm" ) )
            {
                Formats |= guPORTABLEMEDIA_AUDIO_FORMAT_AAC;
            }
            else if( AudioFormats[ Index ] == wxT( "audio/x-ms-wma" ) )
            {
                Formats |= guPORTABLEMEDIA_AUDIO_FORMAT_WMA;
            }
            else
            {
                guLogMessage( wxT( "Unknown audio mime type : '%s'" ), AudioFormats[ Index ].c_str() );
            }
        }
    }
    return Formats;
}

// -------------------------------------------------------------------------------- //
wxString AudioFormatsToMimeStr( const int formats )
{
    wxString MimeStr;
    if( formats & guPORTABLEMEDIA_AUDIO_FORMAT_MP3 )
    {
        MimeStr += wxT( "audio/mpeg" );
    }

    if( formats & guPORTABLEMEDIA_AUDIO_FORMAT_OGG )
    {
        if( !MimeStr.IsEmpty() )
        {
            MimeStr += wxT( ", " );
        }
        MimeStr += wxT( "audio/ogg" );
    }

    if( formats & guPORTABLEMEDIA_AUDIO_FORMAT_FLAC )
    {
        if( !MimeStr.IsEmpty() )
        {
            MimeStr += wxT( ", " );
        }
        MimeStr += wxT( "audio/flac" );
    }

    if( formats & guPORTABLEMEDIA_AUDIO_FORMAT_AAC )
    {
        if( !MimeStr.IsEmpty() )
        {
            MimeStr += wxT( ", " );
        }
        MimeStr += wxT( "audio/mp4a-latm" );
    }

    if( formats & guPORTABLEMEDIA_AUDIO_FORMAT_WMA )
    {
        if( !MimeStr.IsEmpty() )
        {
            MimeStr += wxT( ", " );
        }
        MimeStr += wxT( "audio/x-ms-wma" );
    }
    return MimeStr;
}

// -------------------------------------------------------------------------------- //
int MimeStrToPlaylistFormat( const wxString &mimestr )
{
    int Formats = 0;
    int Index;
    int Count;
    wxArrayString PlaylistFormats = wxStringTokenize( mimestr, wxT( "," ) );
    if( ( Count = PlaylistFormats.Count() ) )
    {
        for( Index = 0; Index < Count; Index++ )
        {
            PlaylistFormats[ Index ].Trim( true ).Trim( false );
            if( ( PlaylistFormats[ Index ] == wxT( "audio/mpegurl" ) ) ||
                     ( PlaylistFormats[ Index ] == wxT( "audio/x-mpegurl" ) ) )
            {
                Formats |= guPORTABLEMEDIA_PLAYLIST_FORMAT_M3U;
            }
            else if( PlaylistFormats[ Index ] == wxT( "audio/x-scpls" ) )
            {
                Formats |= guPORTABLEMEDIA_PLAYLIST_FORMAT_PLS;
            }
            else if( PlaylistFormats[ Index ] == wxT( "application/xspf+xml" ) )
            {
                Formats |= guPORTABLEMEDIA_PLAYLIST_FORMAT_XSPF;
            }
            else if( PlaylistFormats[ Index ] == wxT( "video/x-ms-asf" ) )
            {
                Formats |= guPORTABLEMEDIA_PLAYLIST_FORMAT_ASX;
            }
            else
            {
                guLogMessage( wxT( "Unknown playlist mime type : '%s'" ), PlaylistFormats[ Index ].c_str() );
            }
        }
    }
    return Formats;
}

// -------------------------------------------------------------------------------- //
wxString PlaylistFormatsToMimeStr( const int formats )
{
    wxString MimeStr;

    if( formats & guPORTABLEMEDIA_PLAYLIST_FORMAT_M3U )
    {
        MimeStr += wxT( "audio/mpegurl" );
    }

    if( formats & guPORTABLEMEDIA_PLAYLIST_FORMAT_PLS )
    {
        if( !MimeStr.IsEmpty() )
        {
            MimeStr += wxT( ", " );
        }
        MimeStr += wxT( "audio/x-scpls" );
    }

    if( formats & guPORTABLEMEDIA_PLAYLIST_FORMAT_XSPF )
    {
        if( !MimeStr.IsEmpty() )
        {
            MimeStr += wxT( ", " );
        }
        MimeStr += wxT( "application/xspf+xml" );
    }

    if( formats & guPORTABLEMEDIA_PLAYLIST_FORMAT_ASX )
    {
        if( !MimeStr.IsEmpty() )
        {
            MimeStr += wxT( ", " );
        }
        MimeStr += wxT( "video/x-ms-asf" );
    }

    return MimeStr;
}

// -------------------------------------------------------------------------------- //
int MimeStrToCoverFormat( const wxString &mimestr )
{
    int Formats = 0;
    int Index;
    int Count;
    wxArrayString CoverFormats = wxStringTokenize( mimestr, wxT( "," ) );
    if( ( Count = CoverFormats.Count() ) )
    {
        for( Index = 0; Index < Count; Index++ )
        {
            CoverFormats[ Index ].Trim( true ).Trim( false );
            if( CoverFormats[ Index ] == wxT( "image/jpeg" ) )
            {
                Formats |= guPORTABLEMEDIA_COVER_FORMAT_JPEG;
            }
            else if( ( CoverFormats[ Index ] == wxT( "image/png" ) ) )
            {
                Formats |= guPORTABLEMEDIA_COVER_FORMAT_PNG;
            }
            else if( CoverFormats[ Index ] == wxT( "image/bmp" ) )
            {
                Formats |= guPORTABLEMEDIA_COVER_FORMAT_BMP;
            }
            else if( CoverFormats[ Index ] == wxT( "image/gif" ) )
            {
                Formats |= guPORTABLEMEDIA_COVER_FORMAT_GIF;
            }
            else
            {
                guLogMessage( wxT( "Unknown playlist mime type : '%s'" ), CoverFormats[ Index ].c_str() );
            }
        }
    }
    return Formats;
}

// -------------------------------------------------------------------------------- //
wxString CoverFormatsToMimeStr( const int formats )
{
    wxString MimeStr;

    if( formats & guPORTABLEMEDIA_COVER_FORMAT_JPEG )
    {
        MimeStr += wxT( "image/jpeg" );
    }

    if( formats & guPORTABLEMEDIA_COVER_FORMAT_PNG )
    {
        if( !MimeStr.IsEmpty() )
        {
            MimeStr += wxT( ", " );
        }
        MimeStr += wxT( "image/png" );
    }

    if( formats & guPORTABLEMEDIA_COVER_FORMAT_BMP )
    {
        if( !MimeStr.IsEmpty() )
        {
            MimeStr += wxT( ", " );
        }
        MimeStr += wxT( "image/bmp" );
    }

    if( formats & guPORTABLEMEDIA_COVER_FORMAT_GIF )
    {
        if( !MimeStr.IsEmpty() )
        {
            MimeStr += wxT( ", " );
        }
        MimeStr += wxT( "image/gif" );
    }

    return MimeStr;
}




// -------------------------------------------------------------------------------- //
// guPortableMediaDevice
// -------------------------------------------------------------------------------- //
guPortableMediaDevice::guPortableMediaDevice( guGIO_Mount * mount )
{
    m_Mount = mount;
    m_Type = guPORTABLEMEDIA_TYPE_OTHER;

    guConfig * Config = new guConfig( m_Mount->GetMountPath() + wxT( ".is_audio_player" ) );
    guMD5 MD5;
    m_Id = Config->ReadStr( wxT( "audio_player_id" ), MD5.MD5( wxString::Format( wxT( "%i" ), wxGetLocalTime() ) ).Right( 6 ) );
    m_Pattern = Config->ReadStr( wxT( "audio_file_pattern" ), wxT( "{a} - {b}/{n} - {a} - {t}" ) );
    m_AudioFormats = MimeStrToAudioFormat( Config->ReadStr( wxT( "output_formats" ), wxT( "mp3" ) ) );
    m_TranscodeFormat = Config->ReadNum( wxT( "transcode_format" ), guTRANSCODE_FORMAT_KEEP );
    m_TranscodeScope = Config->ReadNum( wxT( "transcode_scope" ), guPORTABLEMEDIA_TRANSCODE_SCOPE_NOT_SUPPORTED );
    m_TranscodeQuality = Config->ReadNum( wxT( "transcode_quality" ), guTRANSCODE_QUALITY_KEEP );
    m_AudioFolders = Config->ReadStr( wxT( "audio_folders" ), wxT( "/" ) );
    m_PlaylistFormats = MimeStrToPlaylistFormat( Config->ReadStr( wxT( "playlist_format" ), wxEmptyString ) );
    m_PlaylistFolder = Config->ReadStr( wxT( "playlist_path" ), wxEmptyString );
    m_CoverFormats = MimeStrToCoverFormat( Config->ReadStr( wxT( "cover_art_file_type" ), wxT( "jpeg" ) ) );
    m_CoverName = Config->ReadStr( wxT( "cover_art_file_name" ), wxT( "cover" ) );
    m_CoverSize = Config->ReadNum( wxT( "cover_art_size" ), 100 );

    UpdateDiskFree();

    Config->WriteStr( wxT( "audio_player_id" ), m_Id );
    Config->Flush();

    delete Config;
}

// -------------------------------------------------------------------------------- //
guPortableMediaDevice::~guPortableMediaDevice()
{
}

// -------------------------------------------------------------------------------- //
void guPortableMediaDevice::WriteConfig( void )
{
    guConfig * Config = new guConfig( m_Mount->GetMountPath() + wxT( ".is_audio_player" ) );
    Config->WriteStr( wxT( "audio_file_pattern" ), m_Pattern );
    Config->WriteStr( wxT( "output_formats" ), AudioFormatsToMimeStr( m_AudioFormats ) );
    Config->WriteNum( wxT( "transcode_format" ), m_TranscodeFormat );
    Config->WriteNum( wxT( "transcode_scope" ), m_TranscodeScope );
    Config->WriteNum( wxT( "transcode_quality" ), m_TranscodeQuality );
    Config->WriteStr( wxT( "audio_folders" ), m_AudioFolders );
    Config->WriteStr( wxT( "playlist_format" ), PlaylistFormatsToMimeStr( m_PlaylistFormats ) );
    Config->WriteStr( wxT( "playlist_path" ), m_PlaylistFolder );
    Config->WriteStr( wxT( "cover_art_file_type" ), CoverFormatsToMimeStr( m_CoverFormats ) );
    Config->WriteStr( wxT( "cover_art_file_name" ), m_CoverName );
    Config->WriteNum( wxT( "cover_art_size" ), m_CoverSize );
    Config->Flush();

    delete Config;

}

// -------------------------------------------------------------------------------- //
wxString guPortableMediaDevice::AudioFormatsStr( const int formats )
{
    wxString AudioFormats;
    if( formats & guPORTABLEMEDIA_AUDIO_FORMAT_MP3 )
    {
        AudioFormats += wxT( "mp3" );
    }

    if( formats & guPORTABLEMEDIA_AUDIO_FORMAT_OGG )
    {
        if( !AudioFormats.IsEmpty() )
            AudioFormats += wxT( ", " );
        AudioFormats += wxT( "ogg" );
    }

    if( formats & guPORTABLEMEDIA_AUDIO_FORMAT_FLAC )
    {
        if( !AudioFormats.IsEmpty() )
            AudioFormats += wxT( ", " );
        AudioFormats += wxT( "flac" );
    }

    if( formats & guPORTABLEMEDIA_AUDIO_FORMAT_AAC )
    {
        if( !AudioFormats.IsEmpty() )
            AudioFormats += wxT( ", " );
        AudioFormats += wxT( "m4a" );
    }

    if( formats & guPORTABLEMEDIA_AUDIO_FORMAT_WMA )
    {
        if( !AudioFormats.IsEmpty() )
            AudioFormats += wxT( ", " );
        AudioFormats += wxT( "wma" );
    }
    return AudioFormats;
}


// -------------------------------------------------------------------------------- //
wxString guPortableMediaDevice::PlaylistFormatsStr( const int formats )
{
    wxString Formats;
    if( formats & guPORTABLEMEDIA_PLAYLIST_FORMAT_M3U )
    {
        Formats += wxT( "m3u" );
    }

    if( formats & guPORTABLEMEDIA_PLAYLIST_FORMAT_PLS )
    {
        if( !Formats.IsEmpty() )
            Formats += wxT( ", " );
        Formats += wxT( "pls" );
    }

    if( formats & guPORTABLEMEDIA_PLAYLIST_FORMAT_XSPF )
    {
        if( !Formats.IsEmpty() )
            Formats += wxT( ", " );
        Formats += wxT( "xspf" );
    }

    if( formats & guPORTABLEMEDIA_PLAYLIST_FORMAT_ASX )
    {
        if( !Formats.IsEmpty() )
            Formats += wxT( ", " );
        Formats += wxT( "asx" );
    }

    return Formats;
}

// -------------------------------------------------------------------------------- //
wxString guPortableMediaDevice::CoverFormatsStr( const int formats )
{
    wxString Formats;

    if( formats & guPORTABLEMEDIA_COVER_FORMAT_EMBEDDED )
    {
        Formats += _( "embedded" );
    }

    if( formats & guPORTABLEMEDIA_COVER_FORMAT_JPEG )
    {
        if( !Formats.IsEmpty() )
            Formats += wxT( ", " );
        Formats += wxT( "jpeg" );
    }

    if( formats & guPORTABLEMEDIA_COVER_FORMAT_PNG )
    {
        if( !Formats.IsEmpty() )
            Formats += wxT( ", " );
        Formats += wxT( "png" );
    }

    if( formats & guPORTABLEMEDIA_COVER_FORMAT_BMP )
    {
        if( !Formats.IsEmpty() )
            Formats += wxT( ", " );
        Formats += wxT( "bmp" );
    }

    if( formats & guPORTABLEMEDIA_COVER_FORMAT_GIF )
    {
        if( !Formats.IsEmpty() )
            Formats += wxT( ", " );
        Formats += wxT( "gif" );
    }

    return Formats;
}

// -------------------------------------------------------------------------------- //
// guPortableMediaLibrary
// -------------------------------------------------------------------------------- //
guPortableMediaLibrary::guPortableMediaLibrary( const wxString &dbname, guPortableMediaDevice * portablemediadevice ) :
    guDbLibrary( dbname )
{
    m_PortableMediaDevice = portablemediadevice;
}

// -------------------------------------------------------------------------------- //
guPortableMediaLibrary::~guPortableMediaLibrary()
{
}

// -------------------------------------------------------------------------------- //
void guPortableMediaLibrary::UpdateStaticPlayListFile( const int plid )
{
    guLogMessage( wxT( "guPortableMediaLibrary::UpdateStaticPlayListFile" ) );
    int PlayListFormats = m_PortableMediaDevice->PlaylistFormats();
    if( PlayListFormats )
    {
        wxString PlayListName = GetPlayListName( plid );

        wxString PlayListFile = m_PortableMediaDevice->MountPath() + m_PortableMediaDevice->PlaylistFolder();
        PlayListFile += wxT( "/" ) + PlayListName;
        if( PlayListFormats & guPORTABLEMEDIA_PLAYLIST_FORMAT_M3U )
        {
            PlayListFile += wxT( ".m3u" );
        }
        else if( PlayListFormats & guPORTABLEMEDIA_PLAYLIST_FORMAT_PLS )
        {
            PlayListFile += wxT( ".pls" );
        }
        else if( PlayListFormats & guPORTABLEMEDIA_PLAYLIST_FORMAT_XSPF )
        {
            PlayListFile += wxT( ".xspf" );
        }
        else if( PlayListFormats & guPORTABLEMEDIA_PLAYLIST_FORMAT_ASX )
        {
            PlayListFile += wxT( ".asx" );
        }
        else
        {
            PlayListFile += wxT( ".m3u" );
        }

        wxFileName FileName( PlayListFile );
        if( FileName.Normalize() )
        {
            guTrackArray Tracks;
            GetPlayListSongs( plid, guPLAYLIST_TYPE_STATIC, &Tracks, NULL, NULL );
            guPlayListFile PlayListFile;
            PlayListFile.SetName( FileName.GetFullPath() );
            int Index;
            int Count = Tracks.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                PlayListFile.AddItem( Tracks[ Index ].m_FileName,
                    Tracks[ Index ].m_ArtistName + wxT( " - " ) + Tracks[ Index ].m_SongName );
            }

            PlayListFile.Save( FileName.GetFullPath() );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPortableMediaLibrary::DeletePlayList( const int plid )
{
    int PlayListFormats = m_PortableMediaDevice->PlaylistFormats();
    if( PlayListFormats )
    {
        wxString PlayListName = GetPlayListName( plid );

        wxString PlayListFile = m_PortableMediaDevice->MountPath() + m_PortableMediaDevice->PlaylistFolder();
        PlayListFile += wxT( "/" ) + PlayListName;
        if( PlayListFormats & guPORTABLEMEDIA_PLAYLIST_FORMAT_M3U )
        {
            PlayListFile += wxT( ".m3u" );
        }
        else if( PlayListFormats & guPORTABLEMEDIA_PLAYLIST_FORMAT_PLS )
        {
            PlayListFile += wxT( ".pls" );
        }
        else if( PlayListFormats & guPORTABLEMEDIA_PLAYLIST_FORMAT_XSPF )
        {
            PlayListFile += wxT( ".xspf" );
        }
        else if( PlayListFormats & guPORTABLEMEDIA_PLAYLIST_FORMAT_ASX )
        {
            PlayListFile += wxT( ".asx" );
        }
        else
        {
            PlayListFile += wxT( ".m3u" );
        }

        wxFileName FileName( PlayListFile );
        if( FileName.Normalize() )
        {
            if( FileName.FileExists() )
            {
                if( !wxRemoveFile( FileName.GetFullPath() ) )
                {
                    guLogError( wxT( "Could not delete the playlist file '%s'" ), FileName.GetFullPath().c_str() );
                }
            }
        }
    }

    guDbLibrary::DeletePlayList( plid );
}

// -------------------------------------------------------------------------------- //
// guPortableMediaLibPanel
// -------------------------------------------------------------------------------- //
guPortableMediaLibPanel::guPortableMediaLibPanel( wxWindow * parent, guPortableMediaLibrary * db, guPlayerPanel * playerpanel, const wxString &prefix ) :
    guLibPanel( parent, db, playerpanel, prefix )
{
    //SetBaseCommand( ID_MENU_VIEW_PORTABLE_DEVICES );

    m_ContextMenuFlags = guLIBRARY_CONTEXTMENU_DEFAULT ^ guLIBRARY_CONTEXTMENU_DELETEFROMLIBRARY;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->RegisterObject( this ); // Get notified when configuration changes

    Connect( ID_PORTABLEDEVICE_UPDATE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPortableMediaLibPanel::OnPortableLibraryUpdate ), NULL, this );
    Connect( ID_PORTABLEDEVICE_UNMOUNT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPortableMediaLibPanel::OnPortableUnmount ), NULL, this );
    Connect( ID_PORTABLEDEVICE_PROPERTIES, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPortableMediaLibPanel::OnPortableProperties ), NULL, this );

}

// -------------------------------------------------------------------------------- //
guPortableMediaLibPanel::~guPortableMediaLibPanel()
{
    Disconnect( ID_PORTABLEDEVICE_UPDATE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPortableMediaLibPanel::OnPortableLibraryUpdate ), NULL, this );
    Disconnect( ID_PORTABLEDEVICE_UNMOUNT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPortableMediaLibPanel::OnPortableUnmount ), NULL, this );
    Disconnect( ID_PORTABLEDEVICE_PROPERTIES, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPortableMediaLibPanel::OnPortableProperties ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guPortableMediaLibPanel::NormalizeTracks( guTrackArray * tracks, const bool isdrag )
{
    int Index;
    int Count;
    if( tracks && ( Count = tracks->Count() ) )
    {
        for( Index = 0; Index < Count; Index++ )
        {
            guTrack * Track = &( * tracks )[ Index ];
            Track->m_LibPanel = this;
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPortableMediaLibPanel::CreateContextMenu( wxMenu * menu, const int windowid )
{
    wxMenu *     SubMenu = new wxMenu();

    wxMenuItem * MenuItem = new wxMenuItem( menu, ID_PORTABLEDEVICE_UPDATE, _( "Update" ), _( "Update the device library" ) );
    SubMenu->Append( MenuItem );

    MenuItem = new wxMenuItem( menu, ID_PORTABLEDEVICE_PROPERTIES, _( "Properties" ), _( "Set the device properties" ) );
    SubMenu->Append( MenuItem );

    MenuItem = new wxMenuItem( menu, ID_PORTABLEDEVICE_UNMOUNT, _( "Unmount" ), _( "Unmount the device" ) );
    SubMenu->Append( MenuItem );

    menu->AppendSeparator();
    menu->AppendSubMenu( SubMenu, _( "Portable Device" ), _( "Global Jamendo options" ) );
}

// -------------------------------------------------------------------------------- //
wxString guPortableMediaLibPanel::GetName( void )
{
    return m_PortableMediaDevice->DeviceName();
}

// -------------------------------------------------------------------------------- //
wxArrayString guPortableMediaLibPanel::GetLibraryPaths( void )
{
    wxArrayString Paths = wxStringTokenize( m_PortableMediaDevice->AudioFolders(), wxT( "," ) );
    int Index;
    int Count = Paths.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        Paths[ Index ] = m_PortableMediaDevice->MountPath() + Paths[ Index ];
        if( Paths[ Index ].EndsWith( wxT( "//" ) ) )
        {
            Paths[ Index ].RemoveLast();
        }
    }
    return Paths;
}

// -------------------------------------------------------------------------------- //
wxString guPortableMediaLibPanel::GetPlaylistPath( void )
{
    return m_PortableMediaDevice->MountPath() + m_PortableMediaDevice->PlaylistFolder();
}

// -------------------------------------------------------------------------------- //
void guPortableMediaLibPanel::OnPortableLibraryUpdate( wxCommandEvent &event )
{
    DoUpdate();
}

// -------------------------------------------------------------------------------- //
void guPortableMediaLibPanel::OnPortableProperties( wxCommandEvent &event )
{
    guPortableMediaProperties * PortableMediaProperties = new guPortableMediaProperties( this, m_PortableMediaDevice );
    if( PortableMediaProperties )
    {
        if( PortableMediaProperties->ShowModal() == wxID_OK )
        {
            PortableMediaProperties->WriteConfig();
        }
        PortableMediaProperties->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guPortableMediaLibPanel::OnPortableUnmount( wxCommandEvent &event )
{
    if( m_PortableMediaDevice->CanUnmount() )
    {
        m_PortableMediaDevice->Unmount();
    }
}

// -------------------------------------------------------------------------------- //
int guPortableMediaLibPanel::LastUpdate( void )
{
    return 0;
}

// -------------------------------------------------------------------------------- //
void guPortableMediaLibPanel::SetLastUpdate( int lastupdate )
{
    // The portable media devices cant set lastupdate
}

// -------------------------------------------------------------------------------- //
wxArrayString guPortableMediaLibPanel::GetCoverSearchWords( void )
{
    wxArrayString CoverWords = guLibPanel::GetCoverSearchWords();
    if( CoverWords.Index( m_PortableMediaDevice->CoverName() ) == wxNOT_FOUND )
    {
        CoverWords.Add( m_PortableMediaDevice->CoverName() );
    }
    return CoverWords;
}

// -------------------------------------------------------------------------------- //
bool guPortableMediaLibPanel::OnDropFiles( const wxArrayString &filenames )
{
    guTrackArray * CopyTracks = new guTrackArray();
    int Index;
    int Count = filenames.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        wxString CurFile = filenames[ Index ];

        if( guPlayListFile::IsValidPlayList( CurFile ) )
        {
            wxCommandEvent Event( wxEVT_COMMAND_MENU_SELECTED, ID_MAINFRAME_COPYTODEVICE_PLAYLIST );
            Event.SetClientData( new wxString( CurFile ) );
            Event.SetInt( PanelActive() );
            wxPostEvent( wxTheApp->GetTopWindow(), Event );
        }
        else if( guIsValidAudioFile( CurFile ) )
        {
            guTagInfo * TagInfo = guGetTagInfoHandler( CurFile );
            if( TagInfo )
            {
                guTrack * CurTrack = new guTrack();
                if( CurTrack )
                {
                    CurTrack->m_Type = guTRACK_TYPE_NOTDB;

                    TagInfo->Read();

                    CurTrack->m_FileName    = CurFile;
                    CurTrack->m_ArtistName  = TagInfo->m_ArtistName;
                    CurTrack->m_AlbumName   = TagInfo->m_AlbumName;
                    CurTrack->m_SongName    = TagInfo->m_TrackName;
                    CurTrack->m_Number      = TagInfo->m_Track;
                    CurTrack->m_GenreName   = TagInfo->m_GenreName;
                    CurTrack->m_Length      = TagInfo->m_Length;
                    CurTrack->m_Year        = TagInfo->m_Year;
                    CurTrack->m_Comments    = TagInfo->m_Comments;
                    CurTrack->m_Composer    = TagInfo->m_Composer;
                    CurTrack->m_Disk        = TagInfo->m_Disk;
                    CurTrack->m_AlbumArtist = TagInfo->m_AlbumArtist;
                    CurTrack->m_Rating      = wxNOT_FOUND;
                    CurTrack->m_CoverId     = 0;

                    CopyTracks->Add( CurTrack );
                }

                delete TagInfo;
            }
            else
            {
                guLogError( wxT( "Could not read tags from file '%s'" ), CurFile.c_str() );
            }
        }
    }

    if( CopyTracks->Count() )
    {
        wxCommandEvent Event( wxEVT_COMMAND_MENU_SELECTED, ID_MAINFRAME_COPYTODEVICE_TRACKS );
        Event.SetClientData( CopyTracks );
        Event.SetInt( PanelActive() );
        wxPostEvent( wxTheApp->GetTopWindow(), Event );
    }
    else
    {
        delete CopyTracks;
    }

    return true;
}

// -------------------------------------------------------------------------------- //
wxString guPortableMediaLibPanel::GetCoverName( void )
{
    wxString CoverName = m_PortableMediaDevice->CoverName();
    if( CoverName.IsEmpty() )
    {
        CoverName = wxT( "cover" );
    }

    int DevCoverFormats = m_PortableMediaDevice->CoverFormats();

    if( DevCoverFormats & guPORTABLEMEDIA_COVER_FORMAT_JPEG )
    {
        CoverName += wxT( ".jpg" );
    }
    else if( DevCoverFormats & guPORTABLEMEDIA_COVER_FORMAT_PNG )
    {
        CoverName += wxT( ".png" );
    }
    else if( DevCoverFormats & guPORTABLEMEDIA_COVER_FORMAT_GIF )
    {
        CoverName += wxT( ".gif" );
    }
    else if( DevCoverFormats & guPORTABLEMEDIA_COVER_FORMAT_BMP )
    {
        CoverName += wxT( ".bmp" );
    }
    else
    {
        CoverName += wxT( ".jpg" );
    }

    return CoverName;
}

// -------------------------------------------------------------------------------- //
int guPortableMediaLibPanel::GetCoverMaxSize( void )
{
    int CoverSize = m_PortableMediaDevice->CoverSize();
    if( !CoverSize )
        CoverSize = wxNOT_FOUND;
    return CoverSize;
}

// -------------------------------------------------------------------------------- //
void guPortableMediaLibPanel::UpdatePlaylists( void )
{
    wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYLIST_UPDATED );
    evt.SetClientData( this );
    wxPostEvent( wxTheApp->GetTopWindow(), evt );
}

// -------------------------------------------------------------------------------- //
void guPortableMediaLibPanel::DoUpdate( const bool forced )
{
    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, forced ? ID_MENU_UPDATE_LIBRARYFORCED : ID_MENU_UPDATE_LIBRARY );
    event.SetClientData( this );
    wxPostEvent( wxTheApp->GetTopWindow(), event );
}




// -------------------------------------------------------------------------------- //
// guPortableMediaAlbumBrowser
// -------------------------------------------------------------------------------- //
guPortableMediaAlbumBrowser::guPortableMediaAlbumBrowser( wxWindow * parent, guPortableMediaLibrary * db, guPlayerPanel * playerpanel, guPortableMediaLibPanel * libpanel ) :
    guAlbumBrowser( parent, db, playerpanel )
{
    m_LibPanel = libpanel;
}

// -------------------------------------------------------------------------------- //
guPortableMediaAlbumBrowser::~guPortableMediaAlbumBrowser()
{
}

// -------------------------------------------------------------------------------- //
void guPortableMediaAlbumBrowser::NormalizeTracks( guTrackArray * tracks, const bool isdrag )
{
    int Index;
    int Count = tracks->Count();
    for( Index = 0; Index < Count; Index++ )
    {
        tracks->Item( Index ).m_LibPanel = m_LibPanel;
    }
}

// -------------------------------------------------------------------------------- //
void guPortableMediaAlbumBrowser::OnAlbumDownloadCoverClicked( const int albumid )
{
    wxString AlbumName;
    wxString ArtistName;
    wxString AlbumPath;
    if( !m_Db->GetAlbumInfo( albumid, &AlbumName, &ArtistName, &AlbumPath ) )
    {
        wxMessageBox( _( "Could not find the Album in the songs library.\n"\
                         "You should update the library." ), _( "Error" ), wxICON_ERROR | wxOK );
        return;
    }

    AlbumName = RemoveSearchFilters( AlbumName );

    guCoverEditor * CoverEditor = new guCoverEditor( this, ArtistName, AlbumName );
    if( CoverEditor )
    {
        if( CoverEditor->ShowModal() == wxID_OK )
        {
            //guLogMessage( wxT( "About to download cover from selected url" ) );
            wxImage * CoverImage = CoverEditor->GetSelectedCoverImage();
            if( CoverImage )
            {
                m_LibPanel->SetAlbumCover( albumid, AlbumPath, CoverImage );

                ReloadItems();
                RefreshAll();
            }
        }
        CoverEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guPortableMediaAlbumBrowser::OnAlbumSelectCoverClicked( const int albumid )
{
    guSelCoverFile * SelCoverFile = new guSelCoverFile( this, m_Db, albumid );
    if( SelCoverFile )
    {
        if( SelCoverFile->ShowModal() == wxID_OK )
        {
            wxString CoverFile = SelCoverFile->GetSelFile();
            if( !CoverFile.IsEmpty() )
            {
                if( m_LibPanel->SetAlbumCover( albumid, SelCoverFile->GetAlbumPath(), CoverFile ) )
                {
                    ReloadItems();
                    RefreshAll();
                }
            }
        }
        delete SelCoverFile;
    }
}




// -------------------------------------------------------------------------------- //
// guPortableMediaPlayListPanel
// -------------------------------------------------------------------------------- //
guPortableMediaPlayListPanel::guPortableMediaPlayListPanel( wxWindow * parent, guPortableMediaLibrary * db, guPlayerPanel * playerpanel, guPortableMediaLibPanel * libpanel ) :
    guPlayListPanel( parent, db, playerpanel )
{
    m_LibPanel = libpanel;
}

// -------------------------------------------------------------------------------- //
guPortableMediaPlayListPanel::~guPortableMediaPlayListPanel()
{
}

// -------------------------------------------------------------------------------- //
void guPortableMediaPlayListPanel::NormalizeTracks( guTrackArray * tracks, const bool isdrag )
{
    int Index;
    int Count = tracks->Count();
    for( Index = 0; Index < Count; Index++ )
    {
        tracks->Item( Index ).m_LibPanel = m_LibPanel;
    }
}

// -------------------------------------------------------------------------------- //
void guPortableMediaPlayListPanel::SendPlayListUpdatedEvent( void )
{
    wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYLIST_UPDATED );
    evt.SetClientData( m_LibPanel );
    wxPostEvent( wxTheApp->GetTopWindow(), evt );
}



// -------------------------------------------------------------------------------- //
// guPortableMediaProperties
// -------------------------------------------------------------------------------- //
guPortableMediaProperties::guPortableMediaProperties( wxWindow * parent, guPortableMediaDevice * mediadevice )
{
    wxStaticText *              NameLabel;
    wxStaticText *              MountPathLabel;
    wxStaticText *              UsedLabel;
    wxNotebook *                PMNotebook;
    wxPanel *                   PMAudioPanel;
    wxStaticText *              NamePatternLabel;

    wxStaticText *              AudioFolderLabel;
    wxStaticText *              AudioFormatLabel;

    wxPanel *                   PMPlaylistPanel;
    wxStaticText *              PlaylistFormatLabel;
    wxStaticText *              PlaylistFolderLabel;
    wxPanel *                   PMCoversPanel;
    wxStaticText *              CoverFormatLabel;
    wxStaticText *              CoverNameLabel;

    wxStaticText *              CoverSizeLabel;
    wxStdDialogButtonSizer *    BtnSizer;
    wxButton *                  BtnSizerOK;
    wxButton *                  BtnSizerCancel;


    m_PortableMediaDevice = mediadevice;
    m_PortableMediaDevice->UpdateDiskFree();

    guConfig * Config = ( guConfig * ) guConfig::Get();
    wxPoint WindowPos;
    WindowPos.x = Config->ReadNum( wxT( "PMPropertiesPosX" ), -1, wxT( "Positions" ) );
    WindowPos.y = Config->ReadNum( wxT( "PMPropertiesPosY" ), -1, wxT( "Positions" ) );
    wxSize WindowSize;
    WindowSize.x = Config->ReadNum( wxT( "PMPropertiesWidth" ), 570, wxT( "Positions" ) );
    WindowSize.y = Config->ReadNum( wxT( "PMPropertiesHeight" ), 420, wxT( "Positions" ) );

    Create( parent, wxID_ANY, _( "Portable Media Properties" ), WindowPos, WindowSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER );

	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* PMBoxSizer;
	PMBoxSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _( " Properties " ) ), wxVERTICAL );

	wxFlexGridSizer* PMFlexSizer;
	PMFlexSizer = new wxFlexGridSizer( 2, 2, 0, 0 );
	PMFlexSizer->AddGrowableCol( 1 );
	PMFlexSizer->SetFlexibleDirection( wxBOTH );
	PMFlexSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	NameLabel = new wxStaticText( this, wxID_ANY, _("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	NameLabel->Wrap( -1 );
	NameLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );
	PMFlexSizer->Add( NameLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );

	m_NameText = new wxStaticText( this, wxID_ANY, mediadevice->DeviceName(), wxDefaultPosition, wxDefaultSize, 0 );
	m_NameText->Wrap( -1 );
	PMFlexSizer->Add( m_NameText, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP, 5 );

	MountPathLabel = new wxStaticText( this, wxID_ANY, _("Path:"), wxDefaultPosition, wxDefaultSize, 0 );
	MountPathLabel->Wrap( -1 );
	MountPathLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );
	PMFlexSizer->Add( MountPathLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5 );

	m_MountPathText = new wxStaticText( this, wxID_ANY, mediadevice->MountPath(), wxDefaultPosition, wxDefaultSize, 0 );
	m_MountPathText->Wrap( -1 );
	PMFlexSizer->Add( m_MountPathText, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5 );

	UsedLabel = new wxStaticText( this, wxID_ANY, _("Used:"), wxDefaultPosition, wxDefaultSize, 0 );
	UsedLabel->Wrap( -1 );
	PMFlexSizer->Add( UsedLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxBOTTOM, 5 );

	m_UsedGauge = new wxGauge( this, wxID_ANY, 100, wxDefaultPosition, wxSize( -1, 17 ), wxGA_HORIZONTAL );
	m_UsedGauge->SetValue( ( m_PortableMediaDevice->DiskSize() - m_PortableMediaDevice->DiskFree() ) * double( 100 ) / m_PortableMediaDevice->DiskSize() );
	PMFlexSizer->Add( m_UsedGauge, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxBOTTOM|wxRIGHT, 5 );
	//guLogMessage( wxT( "Disk %.0f free of %.0f" ), m_PortableMediaDevice->DiskFree(), m_PortableMediaDevice->DiskSize() );

	PMFlexSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	m_UsedLabel = new wxStaticText( this, wxID_ANY, wxString::Format( _( "%s free of %s" ),
                                    SizeToString( m_PortableMediaDevice->DiskFree() ).c_str(),
                                    SizeToString( m_PortableMediaDevice->DiskSize() ).c_str() ), wxDefaultPosition, wxDefaultSize, 0 );
	m_UsedLabel->Wrap( -1 );
	PMFlexSizer->Add( m_UsedLabel, 0, wxRIGHT|wxALIGN_RIGHT, 5 );

	PMBoxSizer->Add( PMFlexSizer, 0, wxEXPAND, 5 );

	PMNotebook = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	PMAudioPanel = new wxPanel( PMNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	PMFlexSizer = new wxFlexGridSizer( 5, 3, 0, 0 );
	PMFlexSizer->AddGrowableCol( 1 );
	PMFlexSizer->SetFlexibleDirection( wxBOTH );
	PMFlexSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	NamePatternLabel = new wxStaticText( PMAudioPanel, wxID_ANY, _("Name Pattern:"), wxDefaultPosition, wxDefaultSize, 0 );
	NamePatternLabel->Wrap( -1 );
	PMFlexSizer->Add( NamePatternLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_NamePatternText = new wxTextCtrl( PMAudioPanel, wxID_ANY, mediadevice->Pattern(), wxDefaultPosition, wxDefaultSize, 0 );
	m_NamePatternText->SetToolTip( _("{a}\t: Artist\t\t\t{aa} : Album Artist\n{b}\t: Album\t\t\t{d}\t : Disk\n{f}\t: Filename\t\t{g}\t : Genre\n{n}\t: Number\t\t\t{t}\t : Title\n{y}\t: Year") );
	PMFlexSizer->Add( m_NamePatternText, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5 );


	PMFlexSizer->Add( 0, 0, 0, wxEXPAND, 5 );

	AudioFolderLabel = new wxStaticText( PMAudioPanel, wxID_ANY, _("Audio Folders:"), wxDefaultPosition, wxDefaultSize, 0 );
	AudioFolderLabel->Wrap( -1 );
	PMFlexSizer->Add( AudioFolderLabel, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 5 );

	m_AudioFolderText = new wxTextCtrl( PMAudioPanel, wxID_ANY, mediadevice->AudioFolders(), wxDefaultPosition, wxDefaultSize, 0 );
	PMFlexSizer->Add( m_AudioFolderText, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM, 5 );

	m_AudioFolderBtn = new wxButton( PMAudioPanel, wxID_ANY, wxT("..."), wxDefaultPosition, wxSize( 28,-1 ), 0 );
	PMFlexSizer->Add( m_AudioFolderBtn, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );

	AudioFormatLabel = new wxStaticText( PMAudioPanel, wxID_ANY, _("Audio Formats:"), wxDefaultPosition, wxDefaultSize, 0 );
	AudioFormatLabel->Wrap( -1 );
	PMFlexSizer->Add( AudioFormatLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_AudioFormats = mediadevice->AudioFormats();
	m_AudioFormatText = new wxTextCtrl( PMAudioPanel, wxID_ANY, mediadevice->AudioFormatsStr(), wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	PMFlexSizer->Add( m_AudioFormatText, 0, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxTOP|wxBOTTOM, 5 );

	m_AudioFormatBtn = new wxButton( PMAudioPanel, wxID_ANY, wxT("..."), wxDefaultPosition, wxSize( 28,-1 ), 0 );
	PMFlexSizer->Add( m_AudioFormatBtn, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );

	wxStaticText * TransFormatLabel = new wxStaticText( PMAudioPanel, wxID_ANY, _("Transcode to:"), wxDefaultPosition, wxDefaultSize, 0 );
	TransFormatLabel->Wrap( -1 );
	PMFlexSizer->Add( TransFormatLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	wxBoxSizer* TranscodeSizer;
	TranscodeSizer = new wxBoxSizer( wxHORIZONTAL );

	m_TransFormatChoice = new wxChoice( PMAudioPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, guTranscodeFormatStrings(), 0 );
	m_TransFormatChoice->SetSelection( mediadevice->TranscodeFormat() );
	TranscodeSizer->Add( m_TransFormatChoice, 1, wxEXPAND|wxTOP|wxBOTTOM|wxALIGN_CENTER_VERTICAL, 5 );

	wxString m_TransScopeChoiceChoices[] = { _( "Unsupported formats only" ), _( "always" ) };
	int m_TransScopeChoiceNChoices = sizeof( m_TransScopeChoiceChoices ) / sizeof( wxString );
	m_TransScopeChoice = new wxChoice( PMAudioPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_TransScopeChoiceNChoices, m_TransScopeChoiceChoices, 0 );
	m_TransScopeChoice->SetSelection( mediadevice->TranscodeScope() );
	m_TransScopeChoice->Enable( mediadevice->TranscodeFormat() != guTRANSCODE_FORMAT_KEEP );
	TranscodeSizer->Add( m_TransScopeChoice, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );

	PMFlexSizer->Add( TranscodeSizer, 1, wxEXPAND, 5 );


	PMFlexSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	wxStaticText * TransQualityLabel = new wxStaticText( PMAudioPanel, wxID_ANY, _("Quality:"), wxDefaultPosition, wxDefaultSize, 0 );
	TransQualityLabel->Wrap( -1 );
	PMFlexSizer->Add( TransQualityLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_TransQualityChoice = new wxChoice( PMAudioPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, guTranscodeQualityStrings(), 0 );
	m_TransQualityChoice->SetSelection( mediadevice->TranscodeQuality() );
    m_TransQualityChoice->Enable( ( m_PortableMediaDevice->TranscodeFormat() != guTRANSCODE_FORMAT_KEEP ) &&
                                  ( m_PortableMediaDevice->TranscodeQuality() != guTRANSCODE_QUALITY_KEEP ) );
	PMFlexSizer->Add( m_TransQualityChoice, 0, wxEXPAND|wxTOP|wxBOTTOM|wxALIGN_CENTER_VERTICAL, 5 );


	PMFlexSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	PMAudioPanel->SetSizer( PMFlexSizer );
	PMAudioPanel->Layout();
	PMFlexSizer->Fit( PMAudioPanel );
	PMNotebook->AddPage( PMAudioPanel, _("Audio"), true );
	PMPlaylistPanel = new wxPanel( PMNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	PMFlexSizer = new wxFlexGridSizer( 6, 3, 0, 0 );
	PMFlexSizer->AddGrowableCol( 1 );
	PMFlexSizer->SetFlexibleDirection( wxBOTH );
	PMFlexSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	PlaylistFolderLabel = new wxStaticText( PMPlaylistPanel, wxID_ANY, _("PlayList Folders:"), wxDefaultPosition, wxDefaultSize, 0 );
	PlaylistFolderLabel->Wrap( -1 );
	PMFlexSizer->Add( PlaylistFolderLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxALIGN_RIGHT, 5 );

	m_PlaylistFolderText = new wxTextCtrl( PMPlaylistPanel, wxID_ANY, mediadevice->PlaylistFolder(), wxDefaultPosition, wxDefaultSize, 0 );
	PMFlexSizer->Add( m_PlaylistFolderText, 0, wxEXPAND|wxTOP|wxBOTTOM|wxALIGN_CENTER_VERTICAL, 5 );

	m_PlaylistFolderBtn = new wxButton( PMPlaylistPanel, wxID_ANY, wxT("..."), wxDefaultPosition, wxSize( 28,-1 ), 0 );
	PMFlexSizer->Add( m_PlaylistFolderBtn, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	PlaylistFormatLabel = new wxStaticText( PMPlaylistPanel, wxID_ANY, _("PlayList Format:"), wxDefaultPosition, wxDefaultSize, 0 );
	PlaylistFormatLabel->Wrap( -1 );
	PMFlexSizer->Add( PlaylistFormatLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    m_PlaylistFormats = mediadevice->PlaylistFormats();
	m_PlaylistFormatText = new wxTextCtrl( PMPlaylistPanel, wxID_ANY, mediadevice->PlaylistFormatsStr(), wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	PMFlexSizer->Add( m_PlaylistFormatText, 0, wxEXPAND|wxTOP|wxBOTTOM|wxALIGN_CENTER_VERTICAL, 5 );

	m_PlaylistFormatBtn = new wxButton( PMPlaylistPanel, wxID_ANY, wxT("..."), wxDefaultPosition, wxSize( 28,-1 ), 0 );
	PMFlexSizer->Add( m_PlaylistFormatBtn, 0, wxTOP|wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	PMPlaylistPanel->SetSizer( PMFlexSizer );
	PMPlaylistPanel->Layout();
	PMFlexSizer->Fit( PMPlaylistPanel );
	PMNotebook->AddPage( PMPlaylistPanel, _("Playlists"), false );
	PMCoversPanel = new wxPanel( PMNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	PMFlexSizer = new wxFlexGridSizer( 6, 3, 0, 0 );
	PMFlexSizer->AddGrowableCol( 1 );
	PMFlexSizer->SetFlexibleDirection( wxBOTH );
	PMFlexSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	CoverFormatLabel = new wxStaticText( PMCoversPanel, wxID_ANY, _("Cover Format:"), wxDefaultPosition, wxDefaultSize, 0 );
	CoverFormatLabel->Wrap( -1 );
	PMFlexSizer->Add( CoverFormatLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    m_CoverFormats = mediadevice->CoverFormats();
	m_CoverFormatText = new wxTextCtrl( PMCoversPanel, wxID_ANY, mediadevice->CoverFormatsStr(), wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	PMFlexSizer->Add( m_CoverFormatText, 0, wxALIGN_CENTER_VERTICAL|wxEXPAND|wxTOP|wxBOTTOM, 5 );

	m_CoverFormatBtn = new wxButton( PMCoversPanel, wxID_ANY, wxT("..."), wxDefaultPosition, wxSize( 28,-1 ), 0 );
	PMFlexSizer->Add( m_CoverFormatBtn, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );

	CoverNameLabel = new wxStaticText( PMCoversPanel, wxID_ANY, _("Cover Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	CoverNameLabel->Wrap( -1 );
	PMFlexSizer->Add( CoverNameLabel, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	m_CoverNameText = new wxTextCtrl( PMCoversPanel, wxID_ANY, mediadevice->CoverName(), wxDefaultPosition, wxDefaultSize, 0 );
	PMFlexSizer->Add( m_CoverNameText, 0, wxEXPAND|wxTOP|wxBOTTOM|wxALIGN_CENTER_VERTICAL, 5 );


	PMFlexSizer->Add( 0, 0, 0, wxEXPAND, 5 );

	CoverSizeLabel = new wxStaticText( PMCoversPanel, wxID_ANY, _("Covers Size (pixels):"), wxDefaultPosition, wxDefaultSize, 0 );
	CoverSizeLabel->Wrap( -1 );
	PMFlexSizer->Add( CoverSizeLabel, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	m_CoverSizeText = new wxTextCtrl( PMCoversPanel, wxID_ANY, wxString::Format( wxT( "%u" ), mediadevice->CoverSize() ), wxDefaultPosition, wxDefaultSize, 0 );
	PMFlexSizer->Add( m_CoverSizeText, 0, wxTOP|wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	PMCoversPanel->SetSizer( PMFlexSizer );
	PMCoversPanel->Layout();
	PMFlexSizer->Fit( PMCoversPanel );
	PMNotebook->AddPage( PMCoversPanel, _("Covers"), false );

	PMBoxSizer->Add( PMNotebook, 1, wxEXPAND | wxLEFT|wxRIGHT|wxBOTTOM, 5 );

	MainSizer->Add( PMBoxSizer, 1, wxEXPAND|wxALL, 5 );

	BtnSizer = new wxStdDialogButtonSizer();
	BtnSizerOK = new wxButton( this, wxID_OK );
	BtnSizer->AddButton( BtnSizerOK );
	BtnSizerCancel = new wxButton( this, wxID_CANCEL );
	BtnSizer->AddButton( BtnSizerCancel );
	BtnSizer->Realize();
	MainSizer->Add( BtnSizer, 0, wxEXPAND|wxALL, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

	m_AudioFolderBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPortableMediaProperties::OnAudioFolderBtnClick ), NULL, this );
	m_AudioFormatBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPortableMediaProperties::OnAudioFormatBtnClick ), NULL, this );

	m_TransFormatChoice->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( guPortableMediaProperties::OnTransFormatChanged ), NULL, this );

	m_PlaylistFolderBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPortableMediaProperties::OnPlaylistFolderBtnClick ), NULL, this );
	m_PlaylistFormatBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPortableMediaProperties::OnPlaylistFormatBtnClick ), NULL, this );


	m_CoverFormatBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guPortableMediaProperties::OnCoverFormatBtnClick ), NULL, this );

}

// -------------------------------------------------------------------------------- //
guPortableMediaProperties::~guPortableMediaProperties()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    wxPoint WindowPos = GetPosition();
    Config->WriteNum( wxT( "PMPropertiesPosX" ), WindowPos.x, wxT( "Positions" ) );
    Config->WriteNum( wxT( "PMPropertiesPosY" ), WindowPos.y, wxT( "Positions" ) );
    wxSize WindowSize = GetSize();
    Config->WriteNum( wxT( "PMPropertiesWidth" ), WindowSize.x, wxT( "Positions" ) );
    Config->WriteNum( wxT( "PMPropertiesHeight" ), WindowSize.y, wxT( "Positions" ) );
    Config->Flush();

}

// -------------------------------------------------------------------------------- //
void guPortableMediaProperties::WriteConfig( void )
{
    m_PortableMediaDevice->SetPattern( m_NamePatternText->GetValue() );
    m_PortableMediaDevice->SetAudioFormats( m_AudioFormats );
    m_PortableMediaDevice->SetTranscodeFormat( m_TransFormatChoice->GetSelection() );
    m_PortableMediaDevice->SetTranscodeScope( m_TransScopeChoice->GetSelection() );
    m_PortableMediaDevice->SetTranscodeQuality( m_TransQualityChoice->GetSelection() );
    m_PortableMediaDevice->SetAudioFolders( m_AudioFolderText->GetValue() );
    m_PortableMediaDevice->SetPlaylistFormats( m_PlaylistFormats );
    m_PortableMediaDevice->SetPlaylistFolder( m_PlaylistFolderText->GetValue() );
    m_PortableMediaDevice->SetCoverFormats( m_CoverFormats );
    m_PortableMediaDevice->SetCoverName( m_CoverNameText->GetValue() );
    long CoverSize;
    m_CoverSizeText->GetValue().ToLong( &CoverSize );
    m_PortableMediaDevice->SetCoverSize( CoverSize );

    m_PortableMediaDevice->WriteConfig();
}

// -------------------------------------------------------------------------------- //
void guPortableMediaProperties::OnAudioFolderBtnClick( wxCommandEvent &event )
{
    wxString MountPath = m_PortableMediaDevice->MountPath();
    if( !MountPath.EndsWith( wxT( "/" ) ) )
        MountPath.Append( wxT( "/" ) );

    wxDirDialog * DirDialog = new wxDirDialog( this,
                        _( "Select audio folder" ), MountPath, wxDD_DIR_MUST_EXIST );
    if( DirDialog )
    {
        if( DirDialog->ShowModal() == wxID_OK )
        {
            wxString NewFolder = DirDialog->GetPath() + wxT( "/" );
            guLogMessage( wxT( "MountPath: '%s'\nFolder: '%s'" ), MountPath.c_str(), NewFolder.c_str() );
            if( NewFolder.StartsWith( MountPath ) )
            {
                NewFolder = NewFolder.Mid( MountPath.Length() );

                wxString AudioFolders = m_AudioFolderText->GetValue();

                if( AudioFolders.Find( NewFolder ) == wxNOT_FOUND )
                {
                    if( !AudioFolders.IsEmpty() )
                        AudioFolders.Append( wxT( ", " ) );

                    AudioFolders.Append( NewFolder );

                    m_AudioFolderText->SetValue( AudioFolders );
                }
            }
        }
        DirDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guPortableMediaProperties::OnAudioFormatBtnClick( wxCommandEvent &event )
{
    wxArrayInt Selection;
    wxArrayString Items;
    Items.Add( guTranscodeFormatString( 1 ) );
    Items.Add( guTranscodeFormatString( 2 ) );
    Items.Add( guTranscodeFormatString( 3 ) );
    Items.Add( guTranscodeFormatString( 4 ) );
    Items.Add( guTranscodeFormatString( 5 ) );

    wxArrayInt ItemFlags;
    ItemFlags.Add( guPORTABLEMEDIA_AUDIO_FORMAT_MP3 );
    ItemFlags.Add( guPORTABLEMEDIA_AUDIO_FORMAT_OGG );
    ItemFlags.Add( guPORTABLEMEDIA_AUDIO_FORMAT_FLAC );
    ItemFlags.Add( guPORTABLEMEDIA_AUDIO_FORMAT_AAC );
    ItemFlags.Add( guPORTABLEMEDIA_AUDIO_FORMAT_WMA );

    int Index;
    int Count = ItemFlags.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( m_AudioFormats & ItemFlags[ Index ] )
        {
            Selection.Add( Index );
        }
    }

    guListCheckOptionsDialog * ListCheckOptionsDialog = new guListCheckOptionsDialog( this, _( "Audio format" ), Items, Selection );
    if( ListCheckOptionsDialog )
    {
        if( ListCheckOptionsDialog->ShowModal() == wxID_OK )
        {
            ListCheckOptionsDialog->GetSelectedItems( Selection );
            m_AudioFormats = 0;
            Count = Selection.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                m_AudioFormats |= ItemFlags[ Selection[ Index ] ];
            }
            m_AudioFormatText->SetValue( m_PortableMediaDevice->AudioFormatsStr( m_AudioFormats ) );
        }
        ListCheckOptionsDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guPortableMediaProperties::OnPlaylistFolderBtnClick( wxCommandEvent &event )
{
    wxString MountPath = m_PortableMediaDevice->MountPath();
    if( !MountPath.EndsWith( wxT( "/" ) ) )
        MountPath.Append( wxT( "/" ) );

    wxDirDialog * DirDialog = new wxDirDialog( this,
                        _( "Select audio folder" ), MountPath, wxDD_DIR_MUST_EXIST );
    if( DirDialog )
    {
        if( DirDialog->ShowModal() == wxID_OK )
        {
            wxString NewFolder = DirDialog->GetPath() + wxT( "/" );
            guLogMessage( wxT( "Selecting Folders: '%s'\n'%s'" ),
                NewFolder.c_str(),
                MountPath.c_str() );
            if( NewFolder.StartsWith( MountPath ) )
            {
                NewFolder = NewFolder.Mid( MountPath.Length() );

                if( m_PlaylistFolderText->GetValue() != NewFolder )
                {
                    m_PlaylistFolderText->SetValue( NewFolder );
                }
            }
        }
        DirDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guPortableMediaProperties::OnPlaylistFormatBtnClick( wxCommandEvent &event )
{
    wxArrayInt Selection;
    wxArrayString Items;
    Items.Add( guPortablePlaylistFormatString[ 0 ] );
    Items.Add( guPortablePlaylistFormatString[ 1 ] );
    Items.Add( guPortablePlaylistFormatString[ 2 ] );
    Items.Add( guPortablePlaylistFormatString[ 3 ] );

    wxArrayInt ItemFlags;
    ItemFlags.Add( guPORTABLEMEDIA_PLAYLIST_FORMAT_M3U );
    ItemFlags.Add( guPORTABLEMEDIA_PLAYLIST_FORMAT_PLS );
    ItemFlags.Add( guPORTABLEMEDIA_PLAYLIST_FORMAT_XSPF );
    ItemFlags.Add( guPORTABLEMEDIA_PLAYLIST_FORMAT_ASX );

    int Index;
    int Count = ItemFlags.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( m_PlaylistFormats & ItemFlags[ Index ] )
        {
            Selection.Add( Index );
        }
    }

    guListCheckOptionsDialog * ListCheckOptionsDialog = new guListCheckOptionsDialog( this, _( "Playlist format" ), Items, Selection );
    if( ListCheckOptionsDialog )
    {
        if( ListCheckOptionsDialog->ShowModal() == wxID_OK )
        {
            ListCheckOptionsDialog->GetSelectedItems( Selection );
            m_PlaylistFormats = 0;
            Count = Selection.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                m_PlaylistFormats |= ItemFlags[ Selection[ Index ] ];
            }
            m_PlaylistFormatText->SetValue( m_PortableMediaDevice->PlaylistFormatsStr( m_PlaylistFormats ) );
        }
        ListCheckOptionsDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guPortableMediaProperties::OnCoverFormatBtnClick( wxCommandEvent &event )
{
    wxArrayInt Selection;
    wxArrayString Items;
    Items.Add( _( "embedded" ) );
    Items.Add( guPortableCoverFormatString[ 1 ] );
    Items.Add( guPortableCoverFormatString[ 2 ] );
    Items.Add( guPortableCoverFormatString[ 3 ] );
    Items.Add( guPortableCoverFormatString[ 4 ] );

    wxArrayInt ItemFlags;
    ItemFlags.Add( guPORTABLEMEDIA_COVER_FORMAT_EMBEDDED );
    ItemFlags.Add( guPORTABLEMEDIA_COVER_FORMAT_JPEG );
    ItemFlags.Add( guPORTABLEMEDIA_COVER_FORMAT_PNG );
    ItemFlags.Add( guPORTABLEMEDIA_COVER_FORMAT_BMP );
    ItemFlags.Add( guPORTABLEMEDIA_COVER_FORMAT_GIF );

    int Index;
    int Count = ItemFlags.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( m_CoverFormats & ItemFlags[ Index ] )
        {
            Selection.Add( Index );
        }
    }

    guListCheckOptionsDialog * ListCheckOptionsDialog = new guListCheckOptionsDialog( this, _( "Cover format" ), Items, Selection );
    if( ListCheckOptionsDialog )
    {
        if( ListCheckOptionsDialog->ShowModal() == wxID_OK )
        {
            ListCheckOptionsDialog->GetSelectedItems( Selection );
            m_CoverFormats = 0;
            Count = Selection.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                m_CoverFormats |= ItemFlags[ Selection[ Index ] ];
            }
            m_CoverFormatText->SetValue( m_PortableMediaDevice->CoverFormatsStr( m_CoverFormats ) );
        }
        ListCheckOptionsDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guPortableMediaProperties::OnTransFormatChanged( wxCommandEvent& event )
{
    bool IsEnabled = event.GetInt() != guTRANSCODE_FORMAT_KEEP;

    m_TransScopeChoice->Enable( IsEnabled );
    m_TransQualityChoice->Enable( IsEnabled );
}

// -------------------------------------------------------------------------------- //
// guListCheckOptionsDialog
// -------------------------------------------------------------------------------- //
guListCheckOptionsDialog::guListCheckOptionsDialog( wxWindow * parent, const wxString &title,
    const wxArrayString &items, const wxArrayInt &selection ) :
    wxDialog( parent, wxID_ANY, title, wxDefaultPosition, wxSize( 225, 310 ), wxDEFAULT_DIALOG_STYLE )
{

	SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer * MainSizer = new wxBoxSizer( wxVERTICAL );

	m_CheckListBox = new wxCheckListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, items, wxLB_MULTIPLE );
	int Index;
	int Count = selection.Count();
	for( Index = 0; Index < Count; Index++ )
	{
	    m_CheckListBox->Check( selection[ Index ] );
	}
	MainSizer->Add( m_CheckListBox, 1, wxALL|wxEXPAND, 5 );

    wxStdDialogButtonSizer * ButtonSizer;
    wxButton * ButtonSizerOK;
    wxButton * ButtonSizerCancel;
	ButtonSizer = new wxStdDialogButtonSizer();
	ButtonSizerOK = new wxButton( this, wxID_OK );
	ButtonSizer->AddButton( ButtonSizerOK );
	ButtonSizerCancel = new wxButton( this, wxID_CANCEL );
	ButtonSizer->AddButton( ButtonSizerCancel );
	ButtonSizer->Realize();
	MainSizer->Add( ButtonSizer, 0, wxEXPAND|wxALL, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

}

// -------------------------------------------------------------------------------- //
guListCheckOptionsDialog::~guListCheckOptionsDialog()
{
}

// -------------------------------------------------------------------------------- //
int guListCheckOptionsDialog::GetSelectedItems( wxArrayInt &selection )
{
    selection.Empty();
    int Index;
    int Count = m_CheckListBox->GetCount();
    for( Index = 0; Index < Count; Index++ )
    {
        if( m_CheckListBox->IsChecked( Index ) )
            selection.Add( Index );
    }
    return selection.Count();
}


#ifdef WITH_LIBGPOD_SUPPORT

// -------------------------------------------------------------------------------- //
void inline CheckUpdateField( gchar ** fieldptr, const wxString &newval )
{
    if( wxString( * fieldptr, wxConvUTF8 ) != newval )
    {
        free( * fieldptr );
        * fieldptr = strdup( newval.ToUTF8() );
    }
}

// -------------------------------------------------------------------------------- //
// guIpodLibrary
// -------------------------------------------------------------------------------- //
guIpodLibrary::guIpodLibrary( const wxString &dbpath, guPortableMediaDevice * portablemediadevice, Itdb_iTunesDB * ipoddb )
    : guPortableMediaLibrary( dbpath, portablemediadevice )
{
    m_iPodDb = ipoddb;
}

// -------------------------------------------------------------------------------- //
guIpodLibrary::~guIpodLibrary()
{
    if( m_iPodDb )
    {
        itdb_free( m_iPodDb );
    }
}

// -------------------------------------------------------------------------------- //
int guIpodLibrary::GetAlbumId( const wxString &album, const wxString &artist, const wxString &albumartist, const wxString &disk )
{
    //guLogMessage( wxT( "guIpodLibrary::GetAlbumId" ) );
    wxString query;
    wxSQLite3ResultSet dbRes;
    int RetVal = 0;

    query = wxString::Format( wxT( "SELECT song_albumid FROM songs WHERE song_album = '%s' " ), escape_query_str( album ).c_str() );

    if( !albumartist.IsEmpty() )
    {
        query += wxString::Format( wxT( "AND song_albumartist = '%s' " ), escape_query_str( albumartist ).c_str() );
    }
    else
    {
        query += wxString::Format( wxT( "AND song_artist = '%s' " ), escape_query_str( artist ).c_str() );
    }

    if( !disk.IsEmpty() )
    {
        query += wxString::Format( wxT( "AND song_disk = '%s' " ), escape_query_str( disk ).c_str() );
    }

    query += wxT( "LIMIT 1;" );

    dbRes = m_Db->ExecuteQuery( query );

    if( dbRes.NextRow() )
    {
        RetVal = dbRes.GetInt( 0 );
    }
    else
    {
        dbRes.Finalize();

        query = wxT( "SELECT MAX(song_albumid) FROM songs;" );

        dbRes = ExecuteQuery( query );

        if( dbRes.NextRow() )
        {
          RetVal = dbRes.GetInt( 0 ) + 1;
        }
        else
        {
            RetVal = 1;
        }
    }
    dbRes.Finalize();

    return RetVal;
}

// -------------------------------------------------------------------------------- //
int guIpodLibrary::CreateStaticPlayList( const wxString &name, const wxArrayInt &trackids )
{
    guLogMessage( wxT( "guIpodMediaLibPanel::CreateStaticPlayList( '%s' )" ), name.c_str() );

    wxString PlayListName = name;
    Itdb_Playlist * OldPlayList = itdb_playlist_by_name( m_iPodDb, PlayListName.char_str( wxConvUTF8 ) );
    if( OldPlayList )
    {
        guLogMessage( wxT( "Playlist with same name exists so delete it" ) );
        itdb_playlist_remove( OldPlayList );
    }

    Itdb_Playlist * iPodPlayList = itdb_playlist_new( name.mb_str( wxConvFile ), false );
    if( iPodPlayList )
    {
        guLogMessage( wxT( "Created the playlist" ) );
        itdb_playlist_add( m_iPodDb, iPodPlayList, wxNOT_FOUND );
        guLogMessage( wxT( "Attached to the database" ) );
        int Index;
        int Count = trackids.Count();
        guTrackArray Tracks;
        GetSongs( trackids, &Tracks );
        for( Index = 0; Index < Count; Index++ )
        {
            guLogMessage( wxT( "Searching for track %s" ), Tracks[ Index ].m_FileName.c_str() );
            Itdb_Track * iPodTrack = iPodFindTrack( Tracks[ Index ].m_FileName );
            if( iPodTrack )
            {
                guLogMessage( wxT( "Adding track %s" ), Tracks[ Index ].m_FileName.c_str() );
                itdb_playlist_add_track( iPodPlayList, iPodTrack, wxNOT_FOUND );
            }
        }
        guLogMessage( wxT( "Savint the database" ) );
        iPodFlush();
    }
    else
    {
        guLogMessage( wxT( "Could not create the playlist " ) );
    }
    return guDbLibrary::CreateStaticPlayList( name, trackids );
}

// -------------------------------------------------------------------------------- //
int guIpodLibrary::UpdateStaticPlayList( const int plid, const wxArrayInt &trackids )
{
    guLogMessage( wxT( "guIpodLibrary::UpdateStaticPlayList" ) );
    wxString PlayListName = GetPlayListName( plid );
    Itdb_Playlist * OldPlayList = itdb_playlist_by_name( m_iPodDb, PlayListName.char_str( wxConvUTF8 ) );
    if( OldPlayList )
    {
        GList * Tracks = OldPlayList->members;
        while( Tracks )
        {
            Itdb_Track * CurTrack = ( Itdb_Track * ) Tracks->data;
            if( CurTrack )
                itdb_playlist_remove_track( OldPlayList, CurTrack );

            Tracks = Tracks->next;
        }

        int Index;
        int Count = trackids.Count();
        guTrackArray PlayListTracks;
        GetSongs( trackids, &PlayListTracks );
        for( Index = 0; Index < Count; Index++ )
        {
            Itdb_Track * iPodTrack = iPodFindTrack( PlayListTracks[ Index ].m_FileName );
            if( iPodTrack )
            {
                itdb_playlist_add_track( OldPlayList, iPodTrack, wxNOT_FOUND );
            }
        }
        iPodFlush();
    }

    return guDbLibrary::UpdateStaticPlayList( plid, trackids );
}

// -------------------------------------------------------------------------------- //
int guIpodLibrary::AppendStaticPlayList( const int plid, const wxArrayInt &trackids )
{
    guLogMessage( wxT( "guIpodLibrary::AppendStaticPlayList" ) );
    wxString PlayListName = GetPlayListName( plid );
    Itdb_Playlist * OldPlayList = itdb_playlist_by_name( m_iPodDb, PlayListName.char_str( wxConvUTF8 ) );
    if( OldPlayList )
    {
        int Index;
        int Count = trackids.Count();
        guTrackArray PlayListTracks;
        GetSongs( trackids, &PlayListTracks );
        for( Index = 0; Index < Count; Index++ )
        {
            Itdb_Track * iPodTrack = iPodFindTrack( PlayListTracks[ Index ].m_FileName );
            if( iPodTrack )
            {
                itdb_playlist_add_track( OldPlayList, iPodTrack, wxNOT_FOUND );
            }
        }
        iPodFlush();
    }
    return guDbLibrary::AppendStaticPlayList( plid, trackids );
}

// -------------------------------------------------------------------------------- //
int guIpodLibrary::DelPlaylistSetIds( const int plid, const wxArrayInt &trackids )
{
    guLogMessage( wxT( "guIpodLibrary::DelPlaylistSetIds" ) );
    wxString PlayListName = GetPlayListName( plid );
    Itdb_Playlist * OldPlayList = itdb_playlist_by_name( m_iPodDb, PlayListName.char_str( wxConvUTF8 ) );
    if( OldPlayList )
    {
        int Index;
        int Count = trackids.Count();
        guTrackArray PlayListTracks;
        GetSongs( trackids, &PlayListTracks );
        for( Index = 0; Index < Count; Index++ )
        {
            Itdb_Track * iPodTrack = iPodFindTrack( PlayListTracks[ Index ].m_FileName );
            if( iPodTrack )
            {
                itdb_playlist_remove_track( OldPlayList, iPodTrack );
            }
        }
        iPodFlush();
    }
    return guDbLibrary::DelPlaylistSetIds( plid, trackids );
}

// -------------------------------------------------------------------------------- //
void guIpodLibrary::SetPlayListName( const int plid, const wxString &newname )
{
    guLogMessage( wxT( "guIpodLibrary::SetPlayListName" ) );
    wxString PlayListName = GetPlayListName( plid );
    Itdb_Playlist * OldPlayList = itdb_playlist_by_name( m_iPodDb, PlayListName.char_str( wxConvUTF8 ) );
    if( OldPlayList )
    {
        CheckUpdateField( &OldPlayList->name, newname );
        iPodFlush();
    }
    else
    {
        guLogMessage( wxT( "Could not find the playlist '%s'" ), PlayListName.c_str() );
    }

    guDbLibrary::SetPlayListName( plid, newname );
}

// -------------------------------------------------------------------------------- //
void guIpodLibrary::DeletePlayList( const int plid )
{
    guLogMessage( wxT( "guIpodLibrary::DeletePlayList" ) );
    wxString PlayListName = GetPlayListName( plid );
    Itdb_Playlist * OldPlayList = itdb_playlist_by_name( m_iPodDb, PlayListName.char_str( wxConvUTF8 ) );
    if( OldPlayList )
    {
        itdb_playlist_remove( OldPlayList );
        iPodFlush();
    }

    guDbLibrary::DeletePlayList( plid );
}

// -------------------------------------------------------------------------------- //
void guIpodLibrary::UpdateStaticPlayListFile( const int plid )
{
}

// -------------------------------------------------------------------------------- //
Itdb_Playlist * guIpodLibrary::CreateiPodPlayList( const wxString &path, const wxArrayString &filenames )
{
    guLogMessage( wxT( "guIpodLibrary::CreateiPodPlayList( '%s' )" ), wxFileNameFromPath( path ).BeforeLast( wxT( '.' ) ).c_str() );

    Itdb_Playlist * iPodPlayList = itdb_playlist_new( wxFileNameFromPath( path ).BeforeLast( wxT( '.' ) ).mb_str( wxConvFile ), false );
    if( iPodPlayList )
    {
        itdb_playlist_add( m_iPodDb, iPodPlayList, wxNOT_FOUND );
        int Index;
        int Count = filenames.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            guLogMessage( wxT( "Trying to search for: '%s'" ), filenames[ Index ].c_str() );
            Itdb_Track * iPodTrack = iPodFindTrack( filenames[ Index ] );
            if( iPodTrack )
            {
                guLogMessage( wxT( "Found the track" ) );
                itdb_playlist_add_track( iPodPlayList, iPodTrack, wxNOT_FOUND );
            }
        }
        guLogMessage( wxT( "Playlist: '%s' with %i tracks" ), wxString( iPodPlayList->name, wxConvUTF8 ).c_str(), iPodPlayList->num );
        iPodFlush();
    }
    else
    {
        guLogMessage( wxT( "Could not create the playlist " ) );
    }
    return iPodPlayList;
}

// -------------------------------------------------------------------------------- //
Itdb_Track * guIpodLibrary::iPodFindTrack( const wxString &filename )
{
    Itdb_Track * Track = NULL;
    wxString FileName = filename;

    if( FileName.StartsWith( m_PortableMediaDevice->MountPath() ) )
    {
        FileName = FileName.Mid( m_PortableMediaDevice->MountPath().Length() - 1 );
    }
    FileName.Replace( wxT( "/" ), wxT( ":" ) );
    //guLogMessage( wxT( "Trying to find %s" ), FileName.c_str() );

    GList * Tracks = m_iPodDb->tracks;
    while( Tracks )
    {
        Itdb_Track * CurTrack = ( Itdb_Track * ) Tracks->data;
        //guLogMessage( wxT( "Checking: '%s" ), wxString( CurTrack->ipod_path, wxConvUTF8 ).c_str() );
        if( wxString( CurTrack->ipod_path, wxConvUTF8 ) == FileName )
        {
            Track = CurTrack;
            break;
        }
        Tracks = Tracks->next;
    }
    return Track;
}

// -------------------------------------------------------------------------------- //
Itdb_Track * guIpodLibrary::iPodFindTrack( const wxString &artist, const wxString &albumartist, const wxString &album, const wxString &title )
{
    Itdb_Track * Track = NULL;
    GList * Tracks = m_iPodDb->tracks;
    while( Tracks )
    {
        Itdb_Track * CurTrack = ( Itdb_Track * ) Tracks->data;
        if( ( albumartist.IsEmpty() ? ( wxString( CurTrack->artist, wxConvUTF8 ) == artist ) :
                                    ( wxString( CurTrack->albumartist, wxConvUTF8 ) == albumartist ) ) &&
            ( wxString( CurTrack->album, wxConvUTF8 ) == album ) &&
            ( wxString( CurTrack->title, wxConvUTF8 ) == title ) )
        {
            Track = CurTrack;
            break;
        }
        Tracks = Tracks->next;
    }
    return Track;
}

// -------------------------------------------------------------------------------- //
void guIpodLibrary::iPodRemoveTrack( Itdb_Track * track )
{
    if( track )
    {
        GList * Playlists = m_iPodDb->playlists;
        while( Playlists )
        {
            Itdb_Playlist * CurPlaylist = ( Itdb_Playlist * ) Playlists->data;
            if( itdb_playlist_contains_track( CurPlaylist, track ) )
            {
                itdb_playlist_remove_track( CurPlaylist, track );
            }
            Playlists = Playlists->next;
        }

        itdb_track_remove_thumbnails( track );

        itdb_track_remove( track );
    }
}

// -------------------------------------------------------------------------------- //
void guIpodLibrary::iPodRemoveTrack( const wxString &filename )
{
    Itdb_Track * Track = iPodFindTrack( filename );

    iPodRemoveTrack( Track );

    if( !wxRemoveFile( filename ) )
        guLogMessage( wxT( "Couldnt remove the file '%s'" ), filename.c_str() );
}

// -------------------------------------------------------------------------------- //
int guIpodLibrary::UpdateSong( const guTrack &track, const bool allowrating )
{
    wxString query = wxString::Format( wxT( "UPDATE songs SET song_name = '%s', "
            "song_genreid = %u, song_genre = '%s', "
            "song_artistid = %u, song_artist = '%s', "
            "song_albumartistid = %u, song_albumartist = '%s', "
            "song_albumid = %u, song_album = '%s', "
            "song_pathid = %u, song_path = '%s', "
            "song_filename = '%s', song_format = '%s', "
            "song_number = %u, song_year = %u, "
            "song_composerid = %u, song_composer = '%s', "
            "song_comment = '%s', song_coverid = %i, song_disk = '%s', "
            "song_length = %u, song_offset = %u, song_bitrate = %u, "
            "song_rating = %i, song_filesize = %u, "
            "song_lastplay = %u, song_addedtime = %u, "
            "song_playcount = %u "
            "WHERE song_id = %u;" ),
            escape_query_str( track.m_SongName ).c_str(),
            track.m_GenreId,
            escape_query_str( track.m_GenreName ).c_str(),
            track.m_ArtistId,
            escape_query_str( track.m_ArtistName ).c_str(),
            track.m_AlbumArtistId,
            escape_query_str( track.m_AlbumArtist ).c_str(),
            track.m_AlbumId,
            escape_query_str( track.m_AlbumName ).c_str(),
            track.m_PathId,
            escape_query_str( track.m_Path ).c_str(),
            escape_query_str( track.m_FileName ).c_str(),
            escape_query_str( track.m_FileName.AfterLast( '.' ) ).c_str(),
            track.m_Number,
            track.m_Year,
            track.m_ComposerId, //escape_query_str( track.m_Composer ).c_str(),
            escape_query_str( track.m_Composer ).c_str(),
            escape_query_str( track.m_Comments ).c_str(),
            track.m_CoverId,
            escape_query_str( track.m_Disk ).c_str(),
            track.m_Length,
            0, //track.m_Offset,
            track.m_Bitrate,
            track.m_Rating,
            track.m_FileSize,
            track.m_LastPlay,
            track.m_AddedTime,
            track.m_PlayCount,
            track.m_SongId );

  //guLogMessage( wxT( "%s" ), query.c_str() );
  return ExecuteUpdate( query );
}


// -------------------------------------------------------------------------------- //
// guIpodLibraryUpdate
// -------------------------------------------------------------------------------- //
guIpodLibraryUpdate::guIpodLibraryUpdate( guIpodMediaLibPanel * ipodpanel )
{
    m_iPodPanel = ipodpanel;

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guIpodLibraryUpdate::~guIpodLibraryUpdate()
{
    if( !TestDestroy() )
    {
        m_iPodPanel->UpdateFinished();
    }
}

extern "C" {
extern gboolean gdk_pixbuf_save_to_buffer( GdkPixbuf *pixbuf,
                                           gchar **buffer,
                                           gsize *buffer_size,
                                           const char *type,
                                           GError **error,
                                           ... );
}

// -------------------------------------------------------------------------------- //
guIpodLibraryUpdate::ExitCode guIpodLibraryUpdate::Entry( void )
{
    wxArrayPtrVoid CoveriPodTracks;
    wxArrayInt     CoverAlbumIds;
    guIpodLibrary * Db = ( guIpodLibrary * ) m_iPodPanel->GetDb();
    Itdb_iTunesDB * iPodDb = Db->IpodDb();
    if( iPodDb )
    {
        //Db->ExecuteUpdate( wxT( "DELETE FROM songs" ) );
        guPortableMediaDevice * PortableMediaDevice = m_iPodPanel->PortableMediaDevice();

        // Add any missing track to the database or update the existing ones
        GList * Tracks = iPodDb->tracks;
        while( !TestDestroy() && Tracks )
        {
            Itdb_Track * iPodTrack = ( Itdb_Track * ) Tracks->data;
            if( iPodTrack )
            {
                guTrack Track;
                Track.m_SongName = wxString( iPodTrack->title, wxConvUTF8 );
                Track.m_ArtistName = wxString( iPodTrack->artist, wxConvUTF8 );
                Track.m_ArtistId = Db->GetArtistId( Track.m_ArtistName );
                Track.m_GenreName = wxString( iPodTrack->genre, wxConvUTF8 );
                Track.m_GenreId = Db->GetGenreId( Track.m_GenreName );
                Track.m_Composer = wxString( iPodTrack->composer, wxConvUTF8 );
                Track.m_ComposerId = Db->GetComposerId( Track.m_Composer );
                Track.m_Comments = wxString( iPodTrack->comment, wxConvUTF8 );
                Track.m_AlbumArtist = wxString( iPodTrack->albumartist, wxConvUTF8 );
                Track.m_AlbumArtistId = Db->GetAlbumArtistId( Track.m_AlbumArtist );
                Track.m_AlbumName = wxString( iPodTrack->album, wxConvUTF8 );
                Track.m_FileSize = iPodTrack->size;
                Track.m_Length = iPodTrack->tracklen / 1000;
                if( iPodTrack->cd_nr || iPodTrack->cds )
                {
                    Track.m_Disk = wxString::Format( wxT( "%u/%u" ), iPodTrack->cd_nr, iPodTrack->cds );
                }
                Track.m_Number = iPodTrack->track_nr;
                Track.m_Bitrate = iPodTrack->bitrate;
                Track.m_Year = iPodTrack->year;
                Track.m_Rating = iPodTrack->rating / ITDB_RATING_STEP;
                Track.m_PlayCount = iPodTrack->playcount;
                Track.m_AddedTime = iPodTrack->time_added;
                Track.m_LastPlay = iPodTrack->time_played;

                Track.m_Path = PortableMediaDevice->MountPath() + wxString( iPodTrack->ipod_path, wxConvUTF8 ).Mid( 1 );
                Track.m_Path.Replace( wxT( ":" ), wxT( "/" ) );
                Track.m_FileName = Track.m_Path.AfterLast( wxT( '/' ) );
                Track.m_Path = wxPathOnly( Track.m_Path ) + wxT( "/" );

                Track.m_PathId = Db->GetPathId( Track.m_Path );

                Track.m_SongId = Db->GetSongId( Track.m_FileName, Track.m_PathId );

                Track.m_AlbumId = Db->GetAlbumId( Track.m_AlbumName, Track.m_ArtistName, Track.m_AlbumArtist, Track.m_Disk );

                Db->UpdateSong( Track, true );

                if( itdb_track_has_thumbnails( iPodTrack ) )
                {
                    if( CoverAlbumIds.Index( Track.m_AlbumId ) == wxNOT_FOUND )
                    {
                        CoveriPodTracks.Add( iPodTrack );
                        CoverAlbumIds.Add( Track.m_AlbumId );
                    }
                }
            }
            Tracks = Tracks->next;
        }

        // Find all tracks that have been removed
        wxString FileName;
        wxArrayInt SongsToDel;
        wxSQLite3ResultSet dbRes;

        dbRes = Db->ExecuteQuery( wxT( "SELECT DISTINCT song_id, song_filename, song_path FROM songs " ) );

        while( !TestDestroy() && dbRes.NextRow() )
        {
            FileName = dbRes.GetString( 2 ) + dbRes.GetString( 1 );

            if( !wxFileExists( FileName ) )
            {
                SongsToDel.Add( dbRes.GetInt( 0 ) );
            }
        }
        dbRes.Finalize();

        // Add the covers to the albums
        int Index;
        int Count;
        if( ( Count = CoveriPodTracks.Count() ) )
        {
            for( Index = 0; Index < Count; Index++ )
            {
                if( TestDestroy() )
                    break;

                Itdb_Track * iPodTrack = ( Itdb_Track * ) CoveriPodTracks[ Index ];
                int AlbumId = CoverAlbumIds[ Index ];

                GdkPixbuf * Pixbuf = ( GdkPixbuf * ) itdb_track_get_thumbnail( iPodTrack, -1, -1 );
                if( Pixbuf )
                {
                    void * Buffer = NULL;
                    gsize BufferSize = 0;
                    if( gdk_pixbuf_save_to_buffer( Pixbuf, ( gchar ** ) &Buffer, &BufferSize, "jpeg", NULL, "quality", "100", NULL ) )
                    {
                        wxMemoryInputStream ImageStream( Buffer, BufferSize );
                        wxImage CoverImage( ImageStream, wxBITMAP_TYPE_JPEG );
                        if( CoverImage.IsOk() )
                        {
                            Db->SetAlbumCover( AlbumId, CoverImage );
                        }
                        else
                        {
                            guLogMessage( wxT( "Error in image from ipod..." ) );
                        }
                        g_free( Buffer );
                    }
                    else
                    {
                        guLogMessage( wxT( "Couldnt save to a buffer the pixbuf for album" ), AlbumId );
                    }
                    g_object_unref( Pixbuf );
                }
                else
                {
                    guLogMessage( wxT( "Couldnt get the pixbuf for album %i" ), AlbumId );
                }
            }
        }

        // Delete all playlists
        Db->ExecuteUpdate( wxT( "DELETE FROM playlists;" ) );
        Db->ExecuteUpdate( wxT( "DELETE FROM plsets;" ) );

        GList * Playlists = iPodDb->playlists;
        while( !TestDestroy() && Playlists )
        {
            Itdb_Playlist * Playlist = ( Itdb_Playlist * ) Playlists->data;
            if( Playlist && !Playlist->podcastflag && !Playlist->type )
            {
                wxString PlaylistName = wxString( Playlist->name, wxConvUTF8 );
                guLogMessage( wxT( "Playlist: '%s'" ), PlaylistName.c_str() );

                if( !Playlist->is_spl ) // Its not a smart playlist
                {
                    wxArrayInt PlaylistTrackIds;
                    Tracks = Playlist->members;
                    while( Tracks )
                    {
                        Itdb_Track * PlaylistTrack = ( Itdb_Track * ) Tracks->data;
                        if( PlaylistTrack )
                        {
                            wxString Path = PortableMediaDevice->MountPath() + wxString( PlaylistTrack->ipod_path, wxConvUTF8 ).Mid( 1 );
                            Path.Replace( wxT( ":" ), wxT( "/" ) );
                            wxString FileName = Path.AfterLast( wxT( '/' ) );
                            Path = wxPathOnly( Path ) + wxT( "/" );

                            int PathId = Db->GetPathId( Path );

                            int TrackId = Db->GetSongId( FileName, PathId );
                            if( TrackId )
                            {
                                PlaylistTrackIds.Add( TrackId );
                            }
                        }
                        Tracks = Tracks->next;
                    }

                    Db->CreateStaticPlayList( PlaylistName, PlaylistTrackIds, true );
                }
                else                // Its a dynamic playlist
                {
                    guLogMessage( wxT( "Found a dynamic playlist '%s'" ), PlaylistName.c_str() );

                }
            }
            else
            {
                guLogMessage( wxT( "Playlist was podcast or master" ) );
            }

            Playlists = Playlists->next;
        }

        wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYLIST_UPDATED );
        evt.SetClientData( ( void * ) m_iPodPanel );
        wxPostEvent( wxTheApp->GetTopWindow(), evt );

        // Clean all the deleted items
        if( !TestDestroy() )
        {
            wxArrayString Queries;

            if( SongsToDel.Count() )
            {
                Queries.Add( wxT( "DELETE FROM songs WHERE " ) + ArrayToFilter( SongsToDel, wxT( "song_id" ) ) );
            }

            // Delete all posible orphan entries
            Queries.Add( wxT( "DELETE FROM covers WHERE cover_id NOT IN ( SELECT DISTINCT song_coverid FROM songs );" ) );
            Queries.Add( wxT( "DELETE FROM plsets WHERE plset_type = 0 AND plset_songid NOT IN ( SELECT DISTINCT song_id FROM songs );" ) );
            Queries.Add( wxT( "DELETE FROM settags WHERE settag_songid NOT IN ( SELECT DISTINCT song_id FROM songs );" ) );

            int Index;
            int Count = Queries.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                if( TestDestroy() )
                    break;
                //guLogMessage( wxT( "Executing: '%s'" ), Queries[ Index ].c_str() );
                Db->ExecuteUpdate( Queries[ Index ] );
            }
        }

    }
    return 0;
}




// -------------------------------------------------------------------------------- //
// guIpodMediaLibPanel
// -------------------------------------------------------------------------------- //
guIpodMediaLibPanel::guIpodMediaLibPanel( wxWindow * parent, guIpodLibrary * db, guPlayerPanel * playerpanel ) :
    guPortableMediaLibPanel( parent, db, playerpanel )
{
    m_UpdateThread = NULL;
    m_PortableMediaDevice = NULL;

    m_ContextMenuFlags = ( guLIBRARY_CONTEXTMENU_EDIT_TRACKS |
                           guLIBRARY_CONTEXTMENU_DOWNLOAD_COVERS |
                           guLIBRARY_CONTEXTMENU_COPY_TO |
                           guLIBRARY_CONTEXTMENU_LINKS );
}

// -------------------------------------------------------------------------------- //
guIpodMediaLibPanel::~guIpodMediaLibPanel()
{
    if( m_UpdateThread )
    {
        m_UpdateThread->Pause();
        m_UpdateThread->Delete();
    }
}

// -------------------------------------------------------------------------------- //
void guIpodMediaLibPanel::CreateContextMenu( wxMenu * menu, const int windowid )
{
    wxMenu *     SubMenu = new wxMenu();

    wxMenuItem * MenuItem = new wxMenuItem( menu, ID_PORTABLEDEVICE_UPDATE, _( "Update" ), _( "Update the device library" ) );
    SubMenu->Append( MenuItem );

//    MenuItem = new wxMenuItem( menu, ID_PORTABLEDEVICE_PROPERTIES, _( "Properties" ), _( "Set the device properties" ) );
//    SubMenu->Append( MenuItem );

    MenuItem = new wxMenuItem( menu, ID_PORTABLEDEVICE_UNMOUNT, _( "Unmount" ), _( "Unmount the device" ) );
    SubMenu->Append( MenuItem );

    menu->AppendSeparator();
    menu->AppendSubMenu( SubMenu, _( "Portable Device" ), _( "Global Jamendo options" ) );
}

// -------------------------------------------------------------------------------- //
void guIpodMediaLibPanel::DoUpdate( const bool forced )
{
    if( m_UpdateThread )
        return;
    m_UpdateThread = new guIpodLibraryUpdate( this );
}

// -------------------------------------------------------------------------------- //
void guIpodMediaLibPanel::UpdateFinished( void )
{
    m_UpdateThread = NULL;

    wxCommandEvent Event( wxEVT_COMMAND_MENU_SELECTED, ID_LIBRARY_RELOADCONTROLS );
    Event.SetClientData( ( void * ) this );
    wxPostEvent( wxTheApp->GetTopWindow(), Event );
}

// -------------------------------------------------------------------------------- //
wxArrayString guIpodMediaLibPanel::GetLibraryPaths( void )
{
    wxArrayString RetVal;
    RetVal.Add( m_PortableMediaDevice->MountPath() );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guIpodMediaLibPanel::SetPortableMediaDevice( guPortableMediaDevice * portablemediadevice )
{
    m_PortableMediaDevice = portablemediadevice;

    m_PortableMediaDevice->SetPattern( wxEmptyString );
    m_PortableMediaDevice->SetAudioFormats( guPORTABLEMEDIA_AUDIO_FORMAT_MP3 | guPORTABLEMEDIA_AUDIO_FORMAT_AAC );
    m_PortableMediaDevice->SetTranscodeFormat( guTRANSCODE_FORMAT_MP3 );
    m_PortableMediaDevice->SetTranscodeScope( guPORTABLEMEDIA_TRANSCODE_SCOPE_NOT_SUPPORTED );
    m_PortableMediaDevice->SetTranscodeQuality( guTRANSCODE_QUALITY_KEEP );
    m_PortableMediaDevice->SetAudioFolders( wxT( "/" ) );
    m_PortableMediaDevice->SetPlaylistFormats( 0 );
    m_PortableMediaDevice->SetPlaylistFolder( wxEmptyString );
    m_PortableMediaDevice->SetCoverFormats( 0 );
    m_PortableMediaDevice->SetCoverName( wxEmptyString );
    m_PortableMediaDevice->SetCoverSize( 0 );
}

// -------------------------------------------------------------------------------- //
void guIpodMediaLibPanel::UpdateTracks( const guTrackArray &tracks )
{
    int Index;
    int Count = tracks.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guTrack &Track = tracks[ Index ];
        Itdb_Track * iPodTrack = ( ( guIpodLibrary * ) m_Db )->iPodFindTrack( Track.m_FileName );
        if( iPodTrack )
        {
            CheckUpdateField( &iPodTrack->title, Track.m_SongName );
            CheckUpdateField( &iPodTrack->album, Track.m_AlbumName );
            CheckUpdateField( &iPodTrack->artist, Track.m_ArtistName );
            CheckUpdateField( &iPodTrack->genre, Track.m_GenreName );
            CheckUpdateField( &iPodTrack->comment, Track.m_Comments );
            CheckUpdateField( &iPodTrack->composer, Track.m_Composer );
            CheckUpdateField( &iPodTrack->albumartist, Track.m_AlbumArtist );
            iPodTrack->size = Track.m_FileSize;
            iPodTrack->tracklen = Track.m_Length * 1000;
            iPodTrack->track_nr = Track.m_Number;
            guStrDiskToDiskNum( Track.m_Disk, iPodTrack->cd_nr, iPodTrack->cds );
            iPodTrack->bitrate = Track.m_Bitrate;
            iPodTrack->year = Track.m_Year;
            iPodTrack->BPM = 0;
            iPodTrack->rating = ( Track.m_Rating == wxNOT_FOUND ) ? 0 : Track.m_Rating * ITDB_RATING_STEP;
            iPodTrack->type2 = Track.m_Format == wxT( "mp3" ) ? 1 : 0;
        }
    }
    ( ( guIpodLibrary * ) m_Db )->iPodFlush();

    guPortableMediaLibPanel::UpdateTracks( tracks );
}

// -------------------------------------------------------------------------------- //
int guIpodMediaLibPanel::CopyTo( const guTrack * track, wxString &filename )
{
    if( !track )
        return false;

    Itdb_Track * iPodTrack = itdb_track_new();

    iPodTrack->title = strdup( track->m_SongName.ToUTF8() );
    iPodTrack->album = strdup( track->m_AlbumName.ToUTF8() );
    iPodTrack->artist = strdup( track->m_ArtistName.ToUTF8() );
    iPodTrack->genre = strdup( track->m_GenreName.ToUTF8() );
    iPodTrack->comment = strdup( track->m_Comments.ToUTF8() );
    iPodTrack->composer = strdup( track->m_Composer.ToUTF8() );
    iPodTrack->albumartist = strdup( track->m_AlbumArtist.ToUTF8() );
    iPodTrack->size = track->m_FileSize;
    iPodTrack->tracklen = track->m_Length * 1000;
    iPodTrack->track_nr = track->m_Number;
    guStrDiskToDiskNum( track->m_Disk, iPodTrack->cd_nr, iPodTrack->cds );
    iPodTrack->bitrate = track->m_Bitrate;
    iPodTrack->year = track->m_Year;
    iPodTrack->BPM = 0;
    iPodTrack->rating = ( track->m_Rating == wxNOT_FOUND ) ? 0 : track->m_Rating * ITDB_RATING_STEP;
    iPodTrack->playcount = 0;
    iPodTrack->type1 = 0;
    iPodTrack->type2 = track->m_Format == wxT( "mp3" ) ? 1 : 0;
    iPodTrack->compilation = 0;
    iPodTrack->mediatype = ITDB_MEDIATYPE_AUDIO;

    wxString CoverPath;
    if( track->m_CoverId )
    {
        guDbLibrary * TrackDb;
        if( track->m_LibPanel )
        {
            TrackDb = track->m_LibPanel->GetDb();
        }
        else
        {
            TrackDb = ( ( guMainFrame * ) wxTheApp->GetTopWindow() )->GetDb();
        }
        if( TrackDb )
        {
            CoverPath = TrackDb->GetCoverPath( track->m_CoverId );
        }

        if( wxFileExists( CoverPath ) )
        {
            if( !itdb_track_set_thumbnails( iPodTrack, CoverPath.mb_str( wxConvFile ) ) )
                guLogMessage( wxT( "Could not add cover to the ipod track" ) );
        }
    }

    Itdb_iTunesDB * iPodDb = ( ( guIpodLibrary * ) m_Db )->IpodDb();
    // Add the track to the iPod Database
    itdb_track_add( iPodDb, iPodTrack, wxNOT_FOUND );
    // Add th etrack to the master playlist
    Itdb_Playlist * MasterPlaylist = itdb_playlist_mpl( iPodDb );
    itdb_playlist_add_track( MasterPlaylist, iPodTrack, wxNOT_FOUND );

    int FileFormat = guGetTranscodeFileFormat( track->m_FileName.Lower().AfterLast( wxT( '.' ) ) );

    wxString TmpFile;

    // Copy the track file
    if( !( m_PortableMediaDevice->AudioFormats() & FileFormat ) )
    {
        // We need to transcode the file to a temporary file and then copy it
        guLogMessage( wxT( "guIpodMediaLibPanel Transcode File start" ) );
        TmpFile = wxFileName::CreateTempFileName( wxT( "guTrcde_" ) );
        wxRemoveFile( TmpFile );
        TmpFile += wxT( ".mp3" );

        guTranscodeThread * TranscodeThread = new guTranscodeThread( track->m_FileName, TmpFile, guTRANSCODE_FORMAT_MP3, guTRANSCODE_QUALITY_NORMAL );
        if( TranscodeThread && TranscodeThread->IsOk() )
        {
                // TODO : Make a better aproach to be sure its running
                // This need to be called from a thread
                wxThread::Sleep( 1000 );
                while( TranscodeThread->IsTranscoding() )
                {
                    wxThread::Sleep( 200 );
                }
        }

        if( !TranscodeThread->IsOk() )
        {
            guLogMessage( wxT( "Error transcoding the file '%s'" ), track->m_FileName.c_str() );
            wxRemoveFile( TmpFile );
            return wxNOT_FOUND;
        }

        iPodTrack->bitrate = 128000;
        iPodTrack->size = guGetFileSize( TmpFile );
    }
    else    // The file is supported
    {
        TmpFile = wxFileName::CreateTempFileName( wxT( "guTrcde_" ) );
        wxRemoveFile( TmpFile );
        TmpFile += wxT( "." ) + track->m_FileName.Lower().AfterLast( wxT( '.' ) );
        if( !wxCopyFile( track->m_FileName, TmpFile ) )
        {
            guLogMessage( wxT( "Error copying file '%s' into '%s'" ), track->m_FileName.c_str(), TmpFile.c_str() );
            wxRemoveFile( TmpFile );
            return wxNOT_FOUND;
        }
    }

    if( !CoverPath.IsEmpty() )
    {
        guTagSetPicture( TmpFile, CoverPath );
    }

    if( !itdb_cp_track_to_ipod( iPodTrack, TmpFile.mb_str( wxConvFile ), NULL ) )
    {
        guLogMessage( wxT( "Error trying to copy the file '%s'" ), TmpFile.c_str() );
        wxRemoveFile( TmpFile );
        return wxNOT_FOUND;
    }

    wxRemoveFile( TmpFile );

    filename = wxString( iPodTrack->ipod_path, wxConvUTF8 );
    return iPodTrack->size;
}

// -------------------------------------------------------------------------------- //
void guIpodMediaLibPanel::NormalizeTracks( guTrackArray * tracks, const bool isdrag )
{
    int Index;
    int Count = tracks->Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guTrack &Track = tracks->Item( Index );
        Track.m_LibPanel = this;
        Track.m_Type = guTRACK_TYPE_IPOD;
    }
}

// -------------------------------------------------------------------------------- //
bool guIpodMediaLibPanel::SetAlbumCover( const int albumid, const wxString &albumpath, wxImage * coverimg )
{
    guTrackArray Tracks;
    wxArrayInt   Albums;
    Albums.Add( albumid );
    m_Db->GetAlbumsSongs( Albums, &Tracks );
    int Index;
    int Count;
    if( ( Count = Tracks.Count() ) )
    {
        int MaxSize = GetCoverMaxSize();
        if( MaxSize != wxNOT_FOUND )
        {
            coverimg->Rescale( MaxSize, MaxSize, wxIMAGE_QUALITY_HIGH );
        }

        wxString TmpFile = wxFileName::CreateTempFileName( wxT( "guTmpImg_" ) );
        wxRemoveFile( TmpFile );
        TmpFile += wxT( ".jpg" );
        if( coverimg->SaveFile( TmpFile, wxBITMAP_TYPE_JPEG ) )
        {
            for( Index = 0; Index < Count; Index++ )
            {
                Itdb_Track * iPodTrack = ( ( guIpodLibrary * ) m_Db )->iPodFindTrack( Tracks[ Index ].m_FileName );
                if( iPodTrack )
                {
                    if( itdb_track_has_thumbnails( iPodTrack ) )
                    {
                        itdb_artwork_remove_thumbnails( iPodTrack->artwork );
                    }


                    if( !itdb_track_set_thumbnails( iPodTrack, TmpFile.mb_str( wxConvFile ) ) )
                        guLogMessage( wxT( "Couldnt set the cover image %s" ), TmpFile.c_str() );
                }

                guTagSetPicture( Tracks[ Index ].m_FileName, coverimg );
            }
            ( ( guIpodLibrary * ) m_Db )->iPodFlush();
            wxRemoveFile( TmpFile );

            m_Db->SetAlbumCover( albumid, * coverimg );
        }
        else
        {
            guLogMessage( wxT( "Couldnt save the temporary image file" ) );
        }
    }

    return true;
}

// -------------------------------------------------------------------------------- //
bool guIpodMediaLibPanel::SetAlbumCover( const int albumid, const wxString &albumpath, wxString &coverpath )
{
    wxURI Uri( coverpath );
    if( Uri.IsReference() )
    {
        wxImage CoverImage( coverpath );
        if( CoverImage.IsOk() )
        {
            return SetAlbumCover( albumid, albumpath, &CoverImage );
        }
        else
        {
            guLogError( wxT( "Could not load the imate '%s'" ), coverpath.c_str() );
        }
    }
    else
    {
        wxString TmpFile = wxFileName::CreateTempFileName( wxT( "guTmpImg_" ) );
        wxRemoveFile( TmpFile );
        TmpFile += wxT( ".jpg" );
        if( DownloadImage( coverpath, TmpFile, wxBITMAP_TYPE_JPEG ) )
        {
            bool Result = SetAlbumCover( albumid, albumpath, TmpFile );
            wxRemoveFile( TmpFile );
            return Result;
        }
        else
        {
            guLogError( wxT( "Failed to download file '%s'" ), coverpath.c_str() );
        }
    }

    return false;
}

// -------------------------------------------------------------------------------- //
void guIpodMediaLibPanel::DoDeleteAlbumCover( const int albumid )
{
    guTrackArray Tracks;
    wxArrayInt   Albums;
    Albums.Add( albumid );
    m_Db->GetAlbumsSongs( Albums, &Tracks );
    int Index;
    int Count;
    if( ( Count = Tracks.Count() ) )
    {
        for( Index = 0; Index < Count; Index++ )
        {
            Itdb_Track * iPodTrack = ( ( guIpodLibrary * ) m_Db )->iPodFindTrack( Tracks[ Index ].m_FileName );
            if( iPodTrack )
            {
                guLogMessage( wxT( "Deleting cover for track '%s'" ), wxString( iPodTrack->ipod_path, wxConvUTF8 ).c_str() );
                itdb_artwork_remove_thumbnails( iPodTrack->artwork );
            }
        }
        ( ( guIpodLibrary * ) m_Db )->iPodFlush();
    }

    guPortableMediaLibPanel::DoDeleteAlbumCover( albumid );
}

// -------------------------------------------------------------------------------- //
void guIpodMediaLibPanel::DeleteTracks( guTrackArray * tracks )
{
    int Index;
    int Count;
    if( ( Count = tracks->Count() ) )
    {
        m_Db->DeleteLibraryTracks( tracks, false );
        Itdb_iTunesDB * iPodDb = ( ( guIpodLibrary * ) m_Db )->IpodDb();

        Itdb_Playlist * MasterPlaylist = itdb_playlist_mpl( iPodDb );

        for( Index = 0; Index < Count; Index++ )
        {
            Itdb_Track * iPodTrack = ( ( guIpodLibrary * ) m_Db )->iPodFindTrack( tracks->Item( Index ).m_FileName );
            if( iPodTrack )
            {
                itdb_playlist_remove_track( MasterPlaylist, iPodTrack );
                itdb_track_remove( iPodTrack );
            }
        }
        ( ( guIpodLibrary * ) m_Db )->iPodFlush();
    }
}

// -------------------------------------------------------------------------------- //
// guIpodPlayListPanel
// -------------------------------------------------------------------------------- //
guIpodPlayListPanel::guIpodPlayListPanel( wxWindow * parent, guIpodLibrary * db, guPlayerPanel * playerpanel, guIpodMediaLibPanel * libpanel ) :
    guPortableMediaPlayListPanel( parent, db, playerpanel, libpanel )
{
}

// -------------------------------------------------------------------------------- //
guIpodPlayListPanel::~guIpodPlayListPanel()
{
}

// -------------------------------------------------------------------------------- //
void guIpodPlayListPanel::NormalizeTracks( guTrackArray * tracks, const bool isdrag )
{
    int Index;
    int Count = tracks->Count();
    for( Index = 0; Index < Count; Index++ )
    {
        tracks->Item( Index ).m_LibPanel = m_LibPanel;
    }
}




#endif




// -------------------------------------------------------------------------------- //
// guPortableMediaViewCtrl
// -------------------------------------------------------------------------------- //
guPortableMediaViewCtrl::guPortableMediaViewCtrl( guMainFrame * mainframe, guGIO_Mount * mount, int basecommand )
{
    m_MainFrame = mainframe;
    m_BaseCommand = basecommand;
    m_MediaDevice = new guPortableMediaDevice( mount );

    wxString DeviceDbPath = wxGetHomeDir() + wxT( "/.guayadeque/Devices/" ) + m_MediaDevice->DevicePath() + wxT( "/guayadeque.db" );
    wxFileName::Mkdir( wxPathOnly( DeviceDbPath ), 0775, wxPATH_MKDIR_FULL );

#ifdef WITH_LIBGPOD_SUPPORT
    Itdb_iTunesDB * iPodDb = itdb_parse( m_MediaDevice->MountPath().mb_str( wxConvFile ), NULL );
    if( iPodDb )
    {
        m_MediaDevice->SetType( guPORTABLEMEDIA_TYPE_IPOD );
        m_Db = ( guPortableMediaLibrary * ) new guIpodLibrary( DeviceDbPath, m_MediaDevice, iPodDb );

        guLogMessage( wxT( "Detected an Ipod with %i tracks and %i playlists" ),  itdb_tracks_number( iPodDb ), itdb_playlists_number( iPodDb ) );
        Itdb_Device * iPodDevice = iPodDb->device;
        if( iPodDevice )
        {
            //guLogMessage( wxT( "Artwork support : %i" ), itdb_device_supports_artwork( iPodDevice ) );
            const Itdb_IpodInfo * iPodInfo = itdb_device_get_ipod_info( iPodDevice );
            if( iPodInfo )
            {
                guLogMessage( wxT( "Model Number    : %s" ), wxString( iPodInfo->model_number, wxConvUTF8 ).c_str() );
                guLogMessage( wxT( "Capacity        : %.0f" ), iPodInfo->capacity );
                guLogMessage( wxT( "Model Name      : %s" ), wxString( itdb_info_get_ipod_model_name_string( iPodInfo->ipod_model ), wxConvUTF8 ).c_str() );
                guLogMessage( wxT( "Generation      : %s" ), wxString( itdb_info_get_ipod_generation_string( iPodInfo->ipod_generation ), wxConvUTF8 ).c_str() );
            }
        }
    }
    else
#endif
    {
        m_Db = new guPortableMediaLibrary( DeviceDbPath, m_MediaDevice );
        m_Db->SetLibPath( wxStringTokenize( m_MediaDevice->AudioFolders(), wxT( "," ) ) );
    }

    m_LibPanel = NULL;

    m_PlayListPanel = NULL;
    m_AlbumBrowserPanel = NULL;
    m_VisiblePanels = 0;
}

// -------------------------------------------------------------------------------- //
guPortableMediaViewCtrl::~guPortableMediaViewCtrl()
{
    if( m_LibPanel )
        delete m_LibPanel;

    if( m_PlayListPanel )
        delete m_PlayListPanel;

    if( m_AlbumBrowserPanel )
        delete m_AlbumBrowserPanel;

    if( m_MediaDevice )
        delete m_MediaDevice;

    if( m_Db )
        delete m_Db;
}

// -------------------------------------------------------------------------------- //
guPortableMediaLibPanel * guPortableMediaViewCtrl::CreateLibPanel( wxWindow * parent, guPlayerPanel * playerpanel )
{
    if( !m_LibPanel )
    {
#ifdef WITH_LIBGPOD_SUPPORT
        if( m_MediaDevice->Type() == guPORTABLEMEDIA_TYPE_IPOD )
        {
            m_LibPanel = new guIpodMediaLibPanel( parent, ( guIpodLibrary * ) m_Db, playerpanel );
        }
        else
#endif
        {
            m_LibPanel = new guPortableMediaLibPanel( parent, m_Db, playerpanel, wxT( "PMD" ) );
        }
        m_LibPanel->SetPortableMediaDevice( m_MediaDevice );
        m_LibPanel->SetBaseCommand( m_BaseCommand );
    }
    m_VisiblePanels |= guPANEL_MAIN_LIBRARY;
    return m_LibPanel;
}

// -------------------------------------------------------------------------------- //
void guPortableMediaViewCtrl::DestroyLibPanel( void )
{
//    if( m_LibPanel )
//    {
//        delete m_LibPanel;
//        m_LibPanel = NULL;
        m_VisiblePanels ^= guPANEL_MAIN_LIBRARY;
//    }
}

// -------------------------------------------------------------------------------- //
guPortableMediaAlbumBrowser * guPortableMediaViewCtrl::CreateAlbumBrowser( wxWindow * parent, guPlayerPanel * playerpanel )
{
    m_AlbumBrowserPanel = new guPortableMediaAlbumBrowser( parent, m_Db, playerpanel, m_LibPanel );
    //m_AlbumBrowserPanel->SetPortableMediaDevice( m_MediaDevice );
    //m_LibPanel->SetBaseCommand( m_BaseCommand );
    m_VisiblePanels |= guPANEL_MAIN_ALBUMBROWSER;
    return m_AlbumBrowserPanel;
}

// -------------------------------------------------------------------------------- //
void guPortableMediaViewCtrl::DestroyAlbumBrowser( void )
{
    if( m_AlbumBrowserPanel )
    {
        delete m_AlbumBrowserPanel;
        m_AlbumBrowserPanel = NULL;
        m_VisiblePanels ^= guPANEL_MAIN_ALBUMBROWSER;
    }
}

// -------------------------------------------------------------------------------- //
guPortableMediaPlayListPanel * guPortableMediaViewCtrl::CreatePlayListPanel( wxWindow * parent, guPlayerPanel * playerpanel )
{
#ifdef WITH_LIBGPOD_SUPPORT
    if( m_MediaDevice->Type() == guPORTABLEMEDIA_TYPE_IPOD )
    {
        m_PlayListPanel = new guIpodPlayListPanel( parent, ( guIpodLibrary * ) m_Db, playerpanel, ( guIpodMediaLibPanel * ) m_LibPanel );
    }
    else
#endif
    {
        m_PlayListPanel = new guPortableMediaPlayListPanel( parent, m_Db, playerpanel, m_LibPanel );
    }
    m_VisiblePanels |= guPANEL_MAIN_PLAYLISTS;
    return m_PlayListPanel;
}

// -------------------------------------------------------------------------------- //
void guPortableMediaViewCtrl::DestroyPlayListPanel( void )
{
    if( m_PlayListPanel )
    {
        delete m_PlayListPanel;
        m_PlayListPanel = NULL;
        m_VisiblePanels ^= guPANEL_MAIN_PLAYLISTS;
    }
}

// -------------------------------------------------------------------------------- //
