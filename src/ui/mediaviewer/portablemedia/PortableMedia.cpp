// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2022 J.Rios anonbeat@gmail.com
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
#include "PortableMedia.h"

#include "CoverEdit.h"
#include "MainFrame.h"
#include "MD5.h"
#include "SelCoverFile.h"
#include "TagInfo.h"
#include "Transcode.h"
#include "ShowImage.h"
#include "Utils.h"


#include <wx/tokenzr.h>

namespace Guayadeque {

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
int GetPortableMediaType( const wxString &path )
{
#ifdef WITH_LIBGPOD_SUPPORT
    Itdb_iTunesDB * iPodDb = itdb_parse( path.mb_str( wxConvFile ), NULL );
    if( iPodDb )
    {
        itdb_free( iPodDb );
        return guPORTABLE_MEDIA_TYPE_IPOD;
    }
    else
#endif
    return guPORTABLE_MEDIA_TYPE_MSC;
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
            if( CoverFormats[ Index ] == wxT( "embedded" ) )
            {
                Formats |= guPORTABLEMEDIA_COVER_FORMAT_EMBEDDED;
            }
            else if( CoverFormats[ Index ] == wxT( "image/jpeg" ) )
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

    if( formats & guPORTABLEMEDIA_COVER_FORMAT_EMBEDDED )
    {
        MimeStr += wxT( "embedded" );
    }

    if( formats & guPORTABLEMEDIA_COVER_FORMAT_JPEG )
    {
        if( !MimeStr.IsEmpty() )
        {
            MimeStr += wxT( ", " );
        }
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

    m_Type = guPORTABLE_MEDIA_TYPE_MSC;
    // On mtp devices the config file must be inside the device folder ('Internal storage', 'SD Card', etc)
    m_ConfigPath = m_Mount->GetMountPath() + guPORTABLEMEDIA_CONFIG_FILE;
    guLogMessage( wxT( "Using config path '%s'" ), m_ConfigPath.c_str() );

    wxFileConfig * Config = new wxFileConfig( wxEmptyString, wxEmptyString, m_ConfigPath );
    m_Id = mount->GetId(); //Config->Read( wxT( "audio_player_id" ), wxString::Format( wxT( "%08lX" ), wxGetLocalTime() ) );
    m_Pattern = Config->Read( wxT( "audio_file_pattern" ), wxT( "{A} - {b}/{n} - {a} - {t}" ) );
    m_AudioFormats = MimeStrToAudioFormat( Config->Read( wxT( "output_formats" ), wxT( "audio/mpeg" ) ) );
    m_TranscodeFormat = Config->Read( wxT( "transcode_format" ), guTRANSCODE_FORMAT_KEEP );
    m_TranscodeScope = Config->Read( wxT( "transcode_scope" ), guPORTABLEMEDIA_TRANSCODE_SCOPE_NOT_SUPPORTED );
    m_TranscodeQuality = Config->Read( wxT( "transcode_quality" ), guTRANSCODE_QUALITY_KEEP );
    m_AudioFolders = Config->Read( wxT( "audio_folders" ), wxT( "" ) );
    m_PlaylistFormats = MimeStrToPlaylistFormat( Config->Read( wxT( "playlist_format" ), wxEmptyString ) );
    m_PlaylistFolder = Config->Read( wxT( "playlist_path" ), wxEmptyString );
    m_CoverFormats = MimeStrToCoverFormat( Config->Read( wxT( "cover_art_file_type" ), wxT( "image/jpeg" ) ) );
    m_CoverName = Config->Read( wxT( "cover_art_file_name" ), wxT( "cover" ) );
    m_CoverSize = Config->Read( wxT( "cover_art_size" ), 100 );

    UpdateDiskFree();

    Config->Write( wxT( "audio_player_id" ), m_Id );
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
    wxFileConfig * Config = new wxFileConfig( wxEmptyString, wxEmptyString, m_ConfigPath );
    Config->Write( wxT( "audio_file_pattern" ), m_Pattern );
    Config->Write( wxT( "output_formats" ), AudioFormatsToMimeStr( m_AudioFormats ) );
    Config->Write( wxT( "transcode_format" ), m_TranscodeFormat );
    Config->Write( wxT( "transcode_scope" ), m_TranscodeScope );
    Config->Write( wxT( "transcode_quality" ), m_TranscodeQuality );
    Config->Write( wxT( "audio_folders" ), m_AudioFolders );
    Config->Write( wxT( "playlist_format" ), PlaylistFormatsToMimeStr( m_PlaylistFormats ) );
    Config->Write( wxT( "playlist_path" ), m_PlaylistFolder );
    Config->Write( wxT( "cover_art_file_type" ), CoverFormatsToMimeStr( m_CoverFormats ) );
    Config->Write( wxT( "cover_art_file_name" ), m_CoverName );
    Config->Write( wxT( "cover_art_size" ), m_CoverSize );
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
    m_PortableDevice = portablemediadevice;
}

// -------------------------------------------------------------------------------- //
guPortableMediaLibrary::~guPortableMediaLibrary()
{
}

// -------------------------------------------------------------------------------- //
void guPortableMediaLibrary::DeletePlayList( const int plid )
{
    int PlayListFormats = m_PortableDevice->PlaylistFormats();
    if( PlayListFormats )
    {
        wxString PlayListName = GetPlayListName( plid );

        wxString PlayListFile = m_PortableDevice->MountPath() + m_PortableDevice->PlaylistFolder();
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
        if( FileName.Normalize( wxPATH_NORM_DOTS | wxPATH_NORM_TILDE | wxPATH_NORM_ABSOLUTE | wxPATH_NORM_LONG | wxPATH_NORM_SHORTCUT ) )
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
void guPortableMediaLibrary::UpdateStaticPlayListFile( const int plid )
{
    guLogMessage( wxT( "guPortableMediaLibrary::UpdateStaticPlayListFile" ) );
    int PlayListFormats = m_PortableDevice->PlaylistFormats();
    if( PlayListFormats )
    {
        wxString PlayListName = GetPlayListName( plid );

        wxString PlayListFile = m_PortableDevice->MountPath() + m_PortableDevice->PlaylistFolder();
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
        if( FileName.Normalize( wxPATH_NORM_DOTS | wxPATH_NORM_TILDE | wxPATH_NORM_ABSOLUTE | wxPATH_NORM_LONG | wxPATH_NORM_SHORTCUT ) )
        {
            guTrackArray Tracks;
            GetPlayListSongs( plid, guPLAYLIST_TYPE_STATIC, &Tracks, NULL, NULL );
            guPlaylistFile PlayListFile;
            PlayListFile.SetName( FileName.GetFullPath() );
            int Index;
            int Count = Tracks.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                const guTrack &Track = Tracks[ Index ];

                PlayListFile.AddItem( Track.m_FileName,
                    Track.m_ArtistName + wxT( " - " ) + Track.m_SongName,
                    Track.m_Length );
            }

            PlayListFile.Save( FileName.GetFullPath(), true );
        }
    }
}

// -------------------------------------------------------------------------------- //
// guPortableMediaLibPanel
// -------------------------------------------------------------------------------- //
guPortableMediaLibPanel::guPortableMediaLibPanel( wxWindow * parent, guMediaViewerPortableDevice * mediaviewer ) :
    guLibPanel( parent, mediaviewer )
{
}

// -------------------------------------------------------------------------------- //
guPortableMediaLibPanel::~guPortableMediaLibPanel()
{
}

// -------------------------------------------------------------------------------- //
wxString guPortableMediaLibPanel::GetName( void )
{
    return m_PortableDevice->DeviceName();
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
    wxScrolledWindow *          PMAudioPanel;
    wxStaticText *              NamePatternLabel;

    wxStaticText *              AudioFolderLabel;
    wxStaticText *              AudioFormatLabel;

    wxScrolledWindow *          PMPlaylistPanel;
    wxStaticText *              PlaylistFormatLabel;
    wxStaticText *              PlaylistFolderLabel;
    wxScrolledWindow *          PMCoversPanel;
    wxStaticText *              CoverFormatLabel;
    wxStaticText *              CoverNameLabel;

    wxStaticText *              CoverSizeLabel;
    wxStdDialogButtonSizer *    BtnSizer;
    wxButton *                  BtnSizerOK;
    wxButton *                  BtnSizerCancel;


    m_PortableDevice = mediadevice;
    m_PortableDevice->UpdateDiskFree();

    guConfig * Config = ( guConfig * ) guConfig::Get();
    wxPoint WindowPos;
    WindowPos.x = Config->ReadNum( CONFIG_KEY_POSITIONS_PMPROPERTIES_POSX, -1, CONFIG_PATH_POSITIONS );
    WindowPos.y = Config->ReadNum( CONFIG_KEY_POSITIONS_PMPROPERTIES_POSY, -1, CONFIG_PATH_POSITIONS );
    wxSize WindowSize;
    WindowSize.x = Config->ReadNum( CONFIG_KEY_POSITIONS_PMPROPERTIES_WIDTH, 570, CONFIG_PATH_POSITIONS );
    WindowSize.y = Config->ReadNum( CONFIG_KEY_POSITIONS_PMPROPERTIES_HEIGHT, 420, CONFIG_PATH_POSITIONS );

    Create( parent, wxID_ANY, _( "Portable Media Properties" ), WindowPos, WindowSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER  | wxMAXIMIZE_BOX );

	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer * MainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer * PMBoxSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _( " Properties " ) ), wxVERTICAL );

    wxFlexGridSizer * PMFlexSizer = new wxFlexGridSizer( 2, 0, 0 );
	PMFlexSizer->AddGrowableCol( 1 );
	PMFlexSizer->SetFlexibleDirection( wxBOTH );
	PMFlexSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	NameLabel = new wxStaticText( this, wxID_ANY, _("Name:"), wxDefaultPosition, wxDefaultSize, 0 );
	NameLabel->Wrap( -1 );
  NameLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString ) );
  PMFlexSizer->Add( NameLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxTOP, 5 );

	m_NameText = new wxStaticText( this, wxID_ANY, mediadevice->DeviceName(), wxDefaultPosition, wxDefaultSize, 0 );
	m_NameText->Wrap( -1 );
	PMFlexSizer->Add( m_NameText, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT|wxTOP, 5 );

	MountPathLabel = new wxStaticText( this, wxID_ANY, _("Path:"), wxDefaultPosition, wxDefaultSize, 0 );
	MountPathLabel->Wrap( -1 );
	MountPathLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxEmptyString ) );
	PMFlexSizer->Add( MountPathLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5 );

	m_MountPathText = new wxStaticText( this, wxID_ANY, mediadevice->MountPath(), wxDefaultPosition, wxDefaultSize, 0 );
	m_MountPathText->Wrap( -1 );
	PMFlexSizer->Add( m_MountPathText, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5 );

	UsedLabel = new wxStaticText( this, wxID_ANY, _("Used:"), wxDefaultPosition, wxDefaultSize, 0 );
	UsedLabel->Wrap( -1 );
	PMFlexSizer->Add( UsedLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxRIGHT|wxBOTTOM, 5 );

	m_UsedGauge = new wxGauge( this, wxID_ANY, 100, wxDefaultPosition, wxSize( -1, 17 ), wxGA_HORIZONTAL );
	m_UsedGauge->SetValue( ( m_PortableDevice->DiskSize() - m_PortableDevice->DiskFree() ) * double( 100 ) / m_PortableDevice->DiskSize() );
	PMFlexSizer->Add( m_UsedGauge, 1, wxEXPAND|wxBOTTOM|wxRIGHT, 5 );
	//guLogMessage( wxT( "Disk %.0f free of %.0f" ), m_PortableDevice->DiskFree(), m_PortableDevice->DiskSize() );

	PMFlexSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	m_UsedLabel = new wxStaticText( this, wxID_ANY, wxString::Format( _( "%s free of %s" ),
                                    SizeToString( m_PortableDevice->DiskFree() ).c_str(),
                                    SizeToString( m_PortableDevice->DiskSize() ).c_str() ), wxDefaultPosition, wxDefaultSize, 0 );
	m_UsedLabel->Wrap( -1 );
	PMFlexSizer->Add( m_UsedLabel, 0, wxRIGHT|wxALIGN_RIGHT, 5 );

	PMBoxSizer->Add( PMFlexSizer, 0, wxEXPAND, 5 );

	PMNotebook = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );

	PMAudioPanel = new wxScrolledWindow( PMNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );

    PMFlexSizer = new wxFlexGridSizer( 3, 0, 0 );
	PMFlexSizer->AddGrowableCol( 1 );
	PMFlexSizer->SetFlexibleDirection( wxBOTH );
	PMFlexSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_IsIpod = ( m_PortableDevice->Type() == guPORTABLE_MEDIA_TYPE_IPOD );

    if( !m_IsIpod )
    {
        NamePatternLabel = new wxStaticText( PMAudioPanel, wxID_ANY, _("Name Pattern:"), wxDefaultPosition, wxDefaultSize, 0 );
        NamePatternLabel->Wrap( -1 );
        PMFlexSizer->Add( NamePatternLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

        m_NamePatternText = new wxTextCtrl( PMAudioPanel, wxID_ANY, mediadevice->Pattern(), wxDefaultPosition, wxDefaultSize, 0 );
        m_NamePatternText->SetToolTip( _("{a}\t: Artist\t\t\t{aa} : Album Artist\n{b}\t: Album\t\t\t{d}\t : Disk\n{f}\t: Filename\t\t{g}\t : Genre\n{n}\t: Number\t\t\t{t}\t : Title\n{y}\t: Year") );
        PMFlexSizer->Add( m_NamePatternText, 0, wxEXPAND|wxTOP|wxBOTTOM, 5 );

        PMFlexSizer->Add( 0, 0, 0, wxEXPAND, 5 );

        AudioFolderLabel = new wxStaticText( PMAudioPanel, wxID_ANY, _("Audio Folders:"), wxDefaultPosition, wxDefaultSize, 0 );
        AudioFolderLabel->Wrap( -1 );
        PMFlexSizer->Add( AudioFolderLabel, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 5 );

        m_AudioFolderText = new wxTextCtrl( PMAudioPanel, wxID_ANY, mediadevice->AudioFolders(), wxDefaultPosition, wxDefaultSize, 0 );
        PMFlexSizer->Add( m_AudioFolderText, 0, wxEXPAND|wxTOP|wxBOTTOM, 5 );

        m_AudioFolderBtn = new wxButton( PMAudioPanel, wxID_ANY, wxT("..."), wxDefaultPosition, wxSize( 28,-1 ), 0 );
        PMFlexSizer->Add( m_AudioFolderBtn, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );
    }

	AudioFormatLabel = new wxStaticText( PMAudioPanel, wxID_ANY, _("Audio Formats:"), wxDefaultPosition, wxDefaultSize, 0 );
	AudioFormatLabel->Wrap( -1 );
	PMFlexSizer->Add( AudioFormatLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_AudioFormats = mediadevice->AudioFormats();
	m_AudioFormatText = new wxTextCtrl( PMAudioPanel, wxID_ANY, mediadevice->AudioFormatsStr(), wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	PMFlexSizer->Add( m_AudioFormatText, 0, wxEXPAND|wxTOP|wxBOTTOM, 5 );

	m_AudioFormatBtn = new wxButton( PMAudioPanel, wxID_ANY, wxT("..."), wxDefaultPosition, wxSize( 28,-1 ), 0 );
	PMFlexSizer->Add( m_AudioFormatBtn, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );

	wxStaticText * TransFormatLabel = new wxStaticText( PMAudioPanel, wxID_ANY, _("Transcode to:"), wxDefaultPosition, wxDefaultSize, 0 );
	TransFormatLabel->Wrap( -1 );
	PMFlexSizer->Add( TransFormatLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	wxBoxSizer* TranscodeSizer;
	TranscodeSizer = new wxBoxSizer( wxHORIZONTAL );

	m_TransFormatChoice = new wxChoice( PMAudioPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, guTranscodeFormatStrings( m_IsIpod ), 0 );

	if( m_IsIpod )
	{
	    guLogMessage( wxT( "Transcode Format : %i" ), mediadevice->TranscodeFormat() );
        m_TransFormatChoice->SetSelection( mediadevice->TranscodeFormat() == guTRANSCODE_FORMAT_AAC );
	}
	else
	{
        m_TransFormatChoice->SetSelection( mediadevice->TranscodeFormat() );
	}

	TranscodeSizer->Add( m_TransFormatChoice, 1, wxEXPAND|wxTOP|wxBOTTOM, 5 );

	wxString m_TransScopeChoiceChoices[] = { _( "Unsupported formats only" ), _( "always" ) };
	int m_TransScopeChoiceNChoices = sizeof( m_TransScopeChoiceChoices ) / sizeof( wxString );
	m_TransScopeChoice = new wxChoice( PMAudioPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_TransScopeChoiceNChoices, m_TransScopeChoiceChoices, 0 );
	m_TransScopeChoice->SetSelection( mediadevice->TranscodeScope() );
	if( !m_IsIpod )
        m_TransScopeChoice->Enable( mediadevice->TranscodeFormat() != guTRANSCODE_FORMAT_KEEP );
	TranscodeSizer->Add( m_TransScopeChoice, 0, wxTOP|wxBOTTOM|wxLEFT, 5 );

	PMFlexSizer->Add( TranscodeSizer, 1, wxEXPAND, 5 );


	PMFlexSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	wxStaticText * TransQualityLabel = new wxStaticText( PMAudioPanel, wxID_ANY, _("Quality:"), wxDefaultPosition, wxDefaultSize, 0 );
	TransQualityLabel->Wrap( -1 );
	PMFlexSizer->Add( TransQualityLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_TransQualityChoice = new wxChoice( PMAudioPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, guTranscodeQualityStrings(), 0 );
	m_TransQualityChoice->SetSelection( mediadevice->TranscodeQuality() );
	if( !m_IsIpod )
	{
        m_TransQualityChoice->Enable( ( m_PortableDevice->TranscodeFormat() != guTRANSCODE_FORMAT_KEEP ) );
	}
	PMFlexSizer->Add( m_TransQualityChoice, 0, wxEXPAND|wxTOP|wxBOTTOM|wxALIGN_CENTER_VERTICAL, 5 );


	PMFlexSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	PMAudioPanel->SetSizer( PMFlexSizer );
	PMAudioPanel->Layout();
	PMFlexSizer->Fit( PMAudioPanel );
	PMNotebook->AddPage( PMAudioPanel, _("Audio"), true );

    PMAudioPanel->SetScrollRate( 20, 20 );

    if( !m_IsIpod )
    {
        PMPlaylistPanel = new wxScrolledWindow( PMNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
        PMFlexSizer = new wxFlexGridSizer( 3, 0, 0 );
        PMFlexSizer->AddGrowableCol( 1 );
        PMFlexSizer->SetFlexibleDirection( wxBOTH );
        PMFlexSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

        PlaylistFolderLabel = new wxStaticText( PMPlaylistPanel, wxID_ANY, _("Playlist Folders:"), wxDefaultPosition, wxDefaultSize, 0 );
        PlaylistFolderLabel->Wrap( -1 );
        PMFlexSizer->Add( PlaylistFolderLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxALIGN_RIGHT, 5 );

        m_PlaylistFolderText = new wxTextCtrl( PMPlaylistPanel, wxID_ANY, mediadevice->PlaylistFolder(), wxDefaultPosition, wxDefaultSize, 0 );
        PMFlexSizer->Add( m_PlaylistFolderText, 0, wxEXPAND|wxTOP|wxBOTTOM|wxALIGN_CENTER_VERTICAL, 5 );

        m_PlaylistFolderBtn = new wxButton( PMPlaylistPanel, wxID_ANY, wxT("..."), wxDefaultPosition, wxSize( 28,-1 ), 0 );
        PMFlexSizer->Add( m_PlaylistFolderBtn, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxRIGHT, 5 );

        PlaylistFormatLabel = new wxStaticText( PMPlaylistPanel, wxID_ANY, _("Playlist Format:"), wxDefaultPosition, wxDefaultSize, 0 );
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
        PMPlaylistPanel->SetScrollRate( 20, 20 );
    }

	PMCoversPanel = new wxScrolledWindow( PMNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    PMFlexSizer = new wxFlexGridSizer( 3, 0, 0 );
	PMFlexSizer->AddGrowableCol( 1 );
	PMFlexSizer->SetFlexibleDirection( wxBOTH );
	PMFlexSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	CoverFormatLabel = new wxStaticText( PMCoversPanel, wxID_ANY, _("Cover Format:"), wxDefaultPosition, wxDefaultSize, 0 );
	CoverFormatLabel->Wrap( -1 );
	PMFlexSizer->Add( CoverFormatLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    m_CoverFormats = mediadevice->CoverFormats();
	m_CoverFormatText = new wxTextCtrl( PMCoversPanel, wxID_ANY, mediadevice->CoverFormatsStr(), wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	PMFlexSizer->Add( m_CoverFormatText, 0, wxEXPAND|wxTOP|wxBOTTOM, 5 );

	m_CoverFormatBtn = new wxButton( PMCoversPanel, wxID_ANY, wxT("..."), wxDefaultPosition, wxSize( 28,-1 ), 0 );
	PMFlexSizer->Add( m_CoverFormatBtn, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );

    if( !m_IsIpod )
    {
        CoverNameLabel = new wxStaticText( PMCoversPanel, wxID_ANY, _("Cover Name:"), wxDefaultPosition, wxDefaultSize, 0 );
        CoverNameLabel->Wrap( -1 );
        PMFlexSizer->Add( CoverNameLabel, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );

        m_CoverNameText = new wxTextCtrl( PMCoversPanel, wxID_ANY, mediadevice->CoverName(), wxDefaultPosition, wxDefaultSize, 0 );
        PMFlexSizer->Add( m_CoverNameText, 0, wxEXPAND|wxTOP|wxBOTTOM|wxALIGN_CENTER_VERTICAL, 5 );


        PMFlexSizer->Add( 0, 0, 0, wxEXPAND, 5 );
    }

	CoverSizeLabel = new wxStaticText( PMCoversPanel, wxID_ANY, _("Covers Size (pixels):"), wxDefaultPosition, wxDefaultSize, 0 );
	CoverSizeLabel->Wrap( -1 );
	PMFlexSizer->Add( CoverSizeLabel, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	m_CoverSizeText = new wxTextCtrl( PMCoversPanel, wxID_ANY, wxString::Format( wxT( "%u" ), mediadevice->CoverSize() ), wxDefaultPosition, wxDefaultSize, 0 );
	PMFlexSizer->Add( m_CoverSizeText, 0, wxTOP|wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	PMCoversPanel->SetSizer( PMFlexSizer );
	PMCoversPanel->Layout();
	PMFlexSizer->Fit( PMCoversPanel );
	PMNotebook->AddPage( PMCoversPanel, _("Covers"), false );

    PMCoversPanel->SetScrollRate( 20, 20 );

	PMBoxSizer->Add( PMNotebook, 1, wxEXPAND | wxLEFT|wxRIGHT|wxBOTTOM, 5 );

	MainSizer->Add( PMBoxSizer, 1, wxEXPAND|wxALL, 5 );

	BtnSizer = new wxStdDialogButtonSizer();
	BtnSizerOK = new wxButton( this, wxID_OK );
	BtnSizer->AddButton( BtnSizerOK );
	BtnSizerCancel = new wxButton( this, wxID_CANCEL );
	BtnSizer->AddButton( BtnSizerCancel );
	BtnSizer->SetAffirmativeButton( BtnSizerOK );
	BtnSizer->SetCancelButton( BtnSizerCancel );
	BtnSizer->Realize();
	MainSizer->Add( BtnSizer, 0, wxEXPAND|wxALL, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

	BtnSizerOK->SetDefault();

	if( !m_IsIpod )
        m_AudioFolderBtn->Bind( wxEVT_BUTTON, &guPortableMediaProperties::OnAudioFolderBtnClick, this );
    m_AudioFormatBtn->Bind( wxEVT_BUTTON, &guPortableMediaProperties::OnAudioFormatBtnClick, this );

    m_TransFormatChoice->Bind( wxEVT_CHOICE, &guPortableMediaProperties::OnTransFormatChanged, this );

    if( !m_IsIpod )
    {
        m_PlaylistFolderBtn->Bind( wxEVT_BUTTON, &guPortableMediaProperties::OnPlaylistFolderBtnClick, this );
        m_PlaylistFormatBtn->Bind( wxEVT_BUTTON, &guPortableMediaProperties::OnPlaylistFormatBtnClick, this );
    }

    m_CoverFormatBtn->Bind( wxEVT_BUTTON, &guPortableMediaProperties::OnCoverFormatBtnClick, this );

    m_NameText->SetFocus();
}

// -------------------------------------------------------------------------------- //
guPortableMediaProperties::~guPortableMediaProperties()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    wxPoint WindowPos = GetPosition();
    Config->WriteNum( CONFIG_KEY_POSITIONS_PMPROPERTIES_POSX, WindowPos.x, CONFIG_PATH_POSITIONS );
    Config->WriteNum( CONFIG_KEY_POSITIONS_PMPROPERTIES_POSY, WindowPos.y, CONFIG_PATH_POSITIONS );
    wxSize WindowSize = GetSize();
    Config->WriteNum( CONFIG_KEY_POSITIONS_PMPROPERTIES_WIDTH, WindowSize.x, CONFIG_PATH_POSITIONS );
    Config->WriteNum( CONFIG_KEY_POSITIONS_PMPROPERTIES_HEIGHT, WindowSize.y, CONFIG_PATH_POSITIONS );
    Config->Flush();

    if( !m_IsIpod )
        m_AudioFolderBtn->Unbind( wxEVT_BUTTON, &guPortableMediaProperties::OnAudioFolderBtnClick, this );
    m_AudioFormatBtn->Unbind( wxEVT_BUTTON, &guPortableMediaProperties::OnAudioFormatBtnClick, this );

    m_TransFormatChoice->Unbind( wxEVT_CHOICE, &guPortableMediaProperties::OnTransFormatChanged, this );

    if( !m_IsIpod )
    {
        m_PlaylistFolderBtn->Unbind( wxEVT_BUTTON, &guPortableMediaProperties::OnPlaylistFolderBtnClick, this );
        m_PlaylistFormatBtn->Unbind( wxEVT_BUTTON, &guPortableMediaProperties::OnPlaylistFormatBtnClick, this );
    }

    m_CoverFormatBtn->Unbind( wxEVT_BUTTON, &guPortableMediaProperties::OnCoverFormatBtnClick, this );
}

// -------------------------------------------------------------------------------- //
void guPortableMediaProperties::WriteConfig( void )
{
    if( !m_IsIpod )
    {
        m_PortableDevice->SetPattern( m_NamePatternText->GetValue() );
        m_PortableDevice->SetAudioFolders( m_AudioFolderText->GetValue() );
    }
    m_PortableDevice->SetAudioFormats( m_AudioFormats );
    if( !m_IsIpod )
    {
        m_PortableDevice->SetTranscodeFormat( m_TransFormatChoice->GetSelection() );
    }
    else
    {
        m_PortableDevice->SetTranscodeFormat( m_TransFormatChoice->GetSelection() ? guTRANSCODE_FORMAT_AAC : guTRANSCODE_FORMAT_MP3 );
    }
    m_PortableDevice->SetTranscodeScope( m_TransScopeChoice->GetSelection() );
    m_PortableDevice->SetTranscodeQuality( m_TransQualityChoice->GetSelection() );
    if( !m_IsIpod )
    {
        m_PortableDevice->SetPlaylistFormats( m_PlaylistFormats );
        m_PortableDevice->SetPlaylistFolder( m_PlaylistFolderText->GetValue() );
    }
    m_PortableDevice->SetCoverFormats( m_CoverFormats );
    if( !m_IsIpod )
    {
        m_PortableDevice->SetCoverName( m_CoverNameText->GetValue() );
    }
    long CoverSize;
    m_CoverSizeText->GetValue().ToLong( &CoverSize );
    m_PortableDevice->SetCoverSize( CoverSize );

    m_PortableDevice->WriteConfig();
}

// -------------------------------------------------------------------------------- //
void guPortableMediaProperties::OnAudioFolderBtnClick( wxCommandEvent &event )
{
    wxString MountPath = m_PortableDevice->MountPath();
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
    if( !m_IsIpod )
    {
        Items.Add( guTranscodeFormatString( 2 ) );
        Items.Add( guTranscodeFormatString( 3 ) );
    }
    Items.Add( guTranscodeFormatString( 4 ) );

    if( !m_IsIpod )
    {
        Items.Add( guTranscodeFormatString( 5 ) );
    }

    wxArrayInt ItemFlags;
    ItemFlags.Add( guPORTABLEMEDIA_AUDIO_FORMAT_MP3 );
    if( !m_IsIpod )
    {
        ItemFlags.Add( guPORTABLEMEDIA_AUDIO_FORMAT_OGG );
        ItemFlags.Add( guPORTABLEMEDIA_AUDIO_FORMAT_FLAC );
    }
    ItemFlags.Add( guPORTABLEMEDIA_AUDIO_FORMAT_AAC );
    if( !m_IsIpod )
    {
        ItemFlags.Add( guPORTABLEMEDIA_AUDIO_FORMAT_WMA );
    }

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
            m_AudioFormatText->SetValue( m_PortableDevice->AudioFormatsStr( m_AudioFormats ) );
        }
        ListCheckOptionsDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guPortableMediaProperties::OnPlaylistFolderBtnClick( wxCommandEvent &event )
{
    wxString MountPath = m_PortableDevice->MountPath();
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
            m_PlaylistFormatText->SetValue( m_PortableDevice->PlaylistFormatsStr( m_PlaylistFormats ) );
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
    if( !m_IsIpod )
    {
        Items.Add( guPortableCoverFormatString[ 1 ] );
        Items.Add( guPortableCoverFormatString[ 2 ] );
        Items.Add( guPortableCoverFormatString[ 3 ] );
        Items.Add( guPortableCoverFormatString[ 4 ] );
    }

    wxArrayInt ItemFlags;
    ItemFlags.Add( guPORTABLEMEDIA_COVER_FORMAT_EMBEDDED );
    if( !m_IsIpod )
    {
        ItemFlags.Add( guPORTABLEMEDIA_COVER_FORMAT_JPEG );
        ItemFlags.Add( guPORTABLEMEDIA_COVER_FORMAT_PNG );
        ItemFlags.Add( guPORTABLEMEDIA_COVER_FORMAT_BMP );
        ItemFlags.Add( guPORTABLEMEDIA_COVER_FORMAT_GIF );
    }

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
            m_CoverFormatText->SetValue( m_PortableDevice->CoverFormatsStr( m_CoverFormats ) );
        }
        ListCheckOptionsDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guPortableMediaProperties::OnTransFormatChanged( wxCommandEvent& event )
{
    bool IsEnabled = ( event.GetInt() != guTRANSCODE_FORMAT_KEEP ) || m_IsIpod;

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
	ButtonSizer->SetAffirmativeButton( ButtonSizerOK );
	ButtonSizer->SetCancelButton( ButtonSizerCancel );
	ButtonSizer->Realize();
	MainSizer->Add( ButtonSizer, 0, wxEXPAND|wxALL, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

	ButtonSizerOK->SetDefault();

	m_CheckListBox->SetFocus();
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




//// -------------------------------------------------------------------------------- //
//// guPortableMediaViewCtrl
//// -------------------------------------------------------------------------------- //
//guPortableMediaViewCtrl::guPortableMediaViewCtrl( guMainFrame * mainframe, guGIO_Mount * mount, int basecommand )
//{
//    m_MainFrame = mainframe;
//    m_BaseCommand = basecommand;
//    m_PortableDevice = new guPortableMediaDevice( mount );
//
//    wxString DeviceDbPath = wxGetHomeDir() + wxT( "/.guayadeque/Devices/" ) + m_PortableDevice->DevicePath() + wxT( "/guayadeque.db" );
//    wxFileName::Mkdir( wxPathOnly( DeviceDbPath ), 0775, wxPATH_MKDIR_FULL );
//
////////#ifdef WITH_LIBGPOD_SUPPORT
////////    Itdb_iTunesDB * iPodDb = itdb_parse( m_PortableDevice->MountPath().mb_str( wxConvFile ), NULL );
////////    if( iPodDb )
////////    {
////////        m_PortableDevice->SetType( guPORTABLE_MEDIA_TYPE_IPOD );
////////        m_Db = ( guPortableMediaLibrary * ) new guIpodLibrary( DeviceDbPath, m_PortableDevice, iPodDb );
////////
////////        guLogMessage( wxT( "Detected an Ipod with %i tracks and %i playlists" ),  itdb_tracks_number( iPodDb ), itdb_playlists_number( iPodDb ) );
////////        Itdb_Device * iPodDevice = iPodDb->device;
////////        if( iPodDevice )
////////        {
////////            //guLogMessage( wxT( "Artwork support : %i" ), itdb_device_supports_artwork( iPodDevice ) );
////////            const Itdb_IpodInfo * iPodInfo = itdb_device_get_ipod_info( iPodDevice );
////////            if( iPodInfo )
////////            {
////////                guLogMessage( wxT( "Model Number    : %s" ), wxString( iPodInfo->model_number, wxConvUTF8 ).c_str() );
////////                guLogMessage( wxT( "Capacity        : %.0f" ), iPodInfo->capacity );
////////                guLogMessage( wxT( "Model Name      : %s" ), wxString( itdb_info_get_ipod_model_name_string( iPodInfo->ipod_model ), wxConvUTF8 ).c_str() );
////////                guLogMessage( wxT( "Generation      : %s" ), wxString( itdb_info_get_ipod_generation_string( iPodInfo->ipod_generation ), wxConvUTF8 ).c_str() );
////////            }
////////        }
////////    }
////////    else
////////#endif
//    {
//        m_Db = new guPortableMediaLibrary( DeviceDbPath, m_PortableDevice );
////        m_Db->SetLibPath( wxStringTokenize( m_PortableDevice->AudioFolders(), wxT( "," ) ) );
//    }
//
//    m_LibPanel = NULL;
//
//    m_PlayListPanel = NULL;
//    m_AlbumBrowserPanel = NULL;
//    m_VisiblePanels = 0;
//}
//
//// -------------------------------------------------------------------------------- //
//guPortableMediaViewCtrl::~guPortableMediaViewCtrl()
//{
//    if( m_LibPanel )
//        delete m_LibPanel;
//
//    if( m_PlayListPanel )
//        delete m_PlayListPanel;
//
//    if( m_AlbumBrowserPanel )
//        delete m_AlbumBrowserPanel;
//
//    if( m_PortableDevice )
//        delete m_PortableDevice;
//
//    if( m_Db )
//        delete m_Db;
//}
//
//// -------------------------------------------------------------------------------- //
//guPortableMediaLibPanel * guPortableMediaViewCtrl::CreateLibPanel( wxWindow * parent, guPlayerPanel * playerpanel )
//{
//    if( !m_LibPanel )
//    {
////////#ifdef WITH_LIBGPOD_SUPPORT
////////        if( m_PortableDevice->Type() == guPORTABLE_MEDIA_TYPE_IPOD )
////////        {
////////            m_LibPanel = new guIpodMediaLibPanel( parent, ( guIpodLibrary * ) m_Db, playerpanel );
////////        }
////////        else
////////#endif
//        {
////            m_LibPanel = new guPortableMediaLibPanel( parent, m_Db, playerpanel );
//        }
//        m_LibPanel->SetPortableMediaDevice( m_PortableDevice );
//        m_LibPanel->SetBaseCommand( m_BaseCommand );
//    }
//    m_VisiblePanels |= guPANEL_MAIN_LIBRARY;
//    return m_LibPanel;
//}
//
//// -------------------------------------------------------------------------------- //
//void guPortableMediaViewCtrl::DestroyLibPanel( void )
//{
////    if( m_LibPanel )
////    {
////        delete m_LibPanel;
////        m_LibPanel = NULL;
//        m_VisiblePanels ^= guPANEL_MAIN_LIBRARY;
////        m_LibPanel->SetPanelActive( wxNOT_FOUND );
////    }
//}
//
//// -------------------------------------------------------------------------------- //
//guPortableMediaAlbumBrowser * guPortableMediaViewCtrl::CreateAlbumBrowser( wxWindow * parent, guPlayerPanel * playerpanel )
//{
////////#ifdef WITH_LIBGPOD_SUPPORT
////////    if( m_PortableDevice->Type() == guPORTABLE_MEDIA_TYPE_IPOD )
////////    {
////////        m_AlbumBrowserPanel = new guIpodAlbumBrowser( parent, ( guIpodLibrary * ) m_Db, playerpanel, ( guIpodMediaLibPanel * ) m_LibPanel );
////////    }
////////    else
////////#endif
//    {
//        m_AlbumBrowserPanel = new guPortableMediaAlbumBrowser( parent, m_Db, playerpanel, m_LibPanel );
//    }
//    //m_AlbumBrowserPanel->SetPortableMediaDevice( m_PortableDevice );
//    //m_LibPanel->SetBaseCommand( m_BaseCommand );
//    m_VisiblePanels |= guPANEL_MAIN_ALBUMBROWSER;
//    return m_AlbumBrowserPanel;
//}
//
//// -------------------------------------------------------------------------------- //
//void guPortableMediaViewCtrl::DestroyAlbumBrowser( void )
//{
//    if( m_AlbumBrowserPanel )
//    {
//        delete m_AlbumBrowserPanel;
//        m_AlbumBrowserPanel = NULL;
//        m_VisiblePanels ^= guPANEL_MAIN_ALBUMBROWSER;
//    }
//}
//
//// -------------------------------------------------------------------------------- //
//guPortableMediaPlayListPanel * guPortableMediaViewCtrl::CreatePlayListPanel( wxWindow * parent, guPlayerPanel * playerpanel )
//{
////////#ifdef WITH_LIBGPOD_SUPPORT
////////    if( m_PortableDevice->Type() == guPORTABLE_MEDIA_TYPE_IPOD )
////////    {
////////        m_PlayListPanel = new guIpodPlayListPanel( parent, ( guIpodLibrary * ) m_Db, playerpanel, ( guIpodMediaLibPanel * ) m_LibPanel );
////////    }
////////    else
////////#endif
//    {
//        m_PlayListPanel = new guPortableMediaPlayListPanel( parent, m_Db, playerpanel, m_LibPanel );
//    }
//    m_VisiblePanels |= guPANEL_MAIN_PLAYLISTS;
//    return m_PlayListPanel;
//}
//
//// -------------------------------------------------------------------------------- //
//void guPortableMediaViewCtrl::DestroyPlayListPanel( void )
//{
//    if( m_PlayListPanel )
//    {
//        delete m_PlayListPanel;
//        m_PlayListPanel = NULL;
//        m_VisiblePanels ^= guPANEL_MAIN_PLAYLISTS;
//    }
//}


// -------------------------------------------------------------------------------- //
// guMediaViewerPortableDeviceBase
// -------------------------------------------------------------------------------- //
guMediaViewerPortableDeviceBase::guMediaViewerPortableDeviceBase( wxWindow * parent, guMediaCollection &mediacollection,
                          const int basecmd, guMainFrame * mainframe, const int mode,
                          guPlayerPanel * playerpanel, guGIO_Mount * mount ) :
    guMediaViewer( parent, mediacollection, basecmd, mainframe, mode, playerpanel )
{
    m_PortableDevice = new guPortableMediaDevice( mount );
    //m_PortableDevice->SetId( m_MediaCollection->m_UniqueId );
    //m_PortableDevice->WriteConfig();

}

// -------------------------------------------------------------------------------- //
guMediaViewerPortableDeviceBase::~guMediaViewerPortableDeviceBase()
{
    if( m_PortableDevice )
        delete m_PortableDevice;
}

// -------------------------------------------------------------------------------- //
wxString guMediaViewerPortableDeviceBase::GetCoverName( const int albumid )
{
    wxString CoverName = m_PortableDevice->CoverName();
    if( CoverName.IsEmpty() )
    {
        CoverName = wxT( "cover" );
    }

//    int DevCoverFormats = m_PortableDevice->CoverFormats();
//
//    if( DevCoverFormats & guPORTABLEMEDIA_COVER_FORMAT_JPEG )
//    {
//        CoverName += wxT( ".jpg" );
//    }
//    else if( DevCoverFormats & guPORTABLEMEDIA_COVER_FORMAT_PNG )
//    {
//        CoverName += wxT( ".png" );
//    }
//    else if( DevCoverFormats & guPORTABLEMEDIA_COVER_FORMAT_GIF )
//    {
//        CoverName += wxT( ".gif" );
//    }
//    else if( DevCoverFormats & guPORTABLEMEDIA_COVER_FORMAT_BMP )
//    {
//        CoverName += wxT( ".bmp" );
//    }
//    else
//    {
//        CoverName += wxT( ".jpg" );
//    }

    return CoverName;
}

// -------------------------------------------------------------------------------- //
wxString guMediaViewerPortableDeviceBase::AudioPath( void )
{
    wxString Path =  m_PortableDevice->MountPath();
    wxArrayString AudioFolders = wxStringTokenize( m_PortableDevice->AudioFolders(), wxT( "," ) );
    if( AudioFolders.Count() )
    {
        Path += AudioFolders[ 0 ].Trim( true ).Trim( false );;
    }
    return Path;
}

// -------------------------------------------------------------------------------- //
wxBitmapType guMediaViewerPortableDeviceBase::GetCoverType( void )
{
    int CoverFormats = m_PortableDevice->CoverFormats();

    if( CoverFormats & guPORTABLEMEDIA_COVER_FORMAT_JPEG )
        return wxBITMAP_TYPE_JPEG;

    if( CoverFormats & guPORTABLEMEDIA_COVER_FORMAT_PNG )
        return wxBITMAP_TYPE_PNG;

    if( CoverFormats & guPORTABLEMEDIA_COVER_FORMAT_BMP )
        return wxBITMAP_TYPE_BMP;

    return wxBITMAP_TYPE_JPEG;
}

// -------------------------------------------------------------------------------- //
// guMediaViewerPortableDevice
// -------------------------------------------------------------------------------- //
guMediaViewerPortableDevice::guMediaViewerPortableDevice( wxWindow * parent, guMediaCollection &mediacollection,
                          const int basecmd, guMainFrame * mainframe, const int mode,
                          guPlayerPanel * playerpanel, guGIO_Mount * mount ) :
    guMediaViewerPortableDeviceBase( parent, mediacollection, basecmd, mainframe, mode, playerpanel, mount )
{
    UpdateCollectionProperties();

    InitMediaViewer( mode );
}

// -------------------------------------------------------------------------------- //
guMediaViewerPortableDevice::~guMediaViewerPortableDevice()
{
}

// -------------------------------------------------------------------------------- //
void guMediaViewerPortableDevice::LoadMediaDb( void )
{
    m_Db = new guPortableMediaLibrary( guPATH_COLLECTIONS + m_MediaCollection->m_UniqueId + wxT( "/guayadeque.db" ), m_PortableDevice );
    m_Db->SetMediaViewer( this );
}

// -------------------------------------------------------------------------------- //
void guMediaViewerPortableDevice::UpdateCollectionProperties( void )
{
    m_MediaCollection->m_CoverWords.Empty();
    m_MediaCollection->m_CoverWords.Add( m_PortableDevice->CoverName() );
    m_MediaCollection->m_UpdateOnStart = true;
    //m_MediaCollection->m_ScanEmbeddedCovers = false;
}

// -------------------------------------------------------------------------------- //
wxArrayString guMediaViewerPortableDevice::GetPaths( void )
{
    wxArrayString Paths = wxStringTokenize( m_PortableDevice->AudioFolders(), wxT( "," ) );
    int Index;
    int Count = Paths.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        Paths[ Index ] = m_PortableDevice->MountPath() + Paths[ Index ];
        Paths[ Index ].Replace( wxT( "//" ), wxT( "/" ) );
    }
    return Paths;
}

// -------------------------------------------------------------------------------- //
bool guMediaViewerPortableDevice::CreateLibraryView( void )
{
    m_LibPanel = new guPortableMediaLibPanel( this, this );
    m_LibPanel->SetBaseCommand( m_BaseCommand + 1 );
    return true;
}

// -------------------------------------------------------------------------------- //
void guMediaViewerPortableDevice::CreateContextMenu( wxMenu * menu, const int windowid )
{
    wxMenu * Menu = new wxMenu();
    wxMenuItem * MenuItem;

    int BaseCommand = GetBaseCommand();

    MenuItem = new wxMenuItem( Menu, BaseCommand + guCOLLECTION_ACTION_UPDATE_LIBRARY, _( "Update" ), _( "Update the collection library" ), wxITEM_NORMAL );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, BaseCommand + guCOLLECTION_ACTION_RESCAN_LIBRARY, _( "Rescan" ), _( "Rescan the collection library" ), wxITEM_NORMAL );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, BaseCommand + guCOLLECTION_ACTION_SEARCH_COVERS, _( "Search Covers" ), _( "Search the collection missing covers" ), wxITEM_NORMAL );
    Menu->Append( MenuItem );

    Menu->AppendSeparator();

    MenuItem = new wxMenuItem( menu, BaseCommand + guCOLLECTION_ACTION_UNMOUNT, _( "Unmount" ), _( "Unmount the device" ) );
    Menu->Append( MenuItem );

    Menu->AppendSeparator();

    MenuItem = new wxMenuItem( Menu, BaseCommand + guCOLLECTION_ACTION_VIEW_PROPERTIES, _( "Properties" ), _( "Show collection properties" ), wxITEM_NORMAL );
    Menu->Append( MenuItem );

    menu->AppendSeparator();
    menu->AppendSubMenu( Menu, m_MediaCollection->m_Name );
}

// -------------------------------------------------------------------------------- //
void guMediaViewerPortableDevice::HandleCommand( const int command )
{
    if( command == guCOLLECTION_ACTION_UNMOUNT )
    {
        if( m_PortableDevice->CanUnmount() )
        {
            m_PortableDevice->Unmount();
        }
    }
    else
    {
        guMediaViewer::HandleCommand( command );
    }
}

// -------------------------------------------------------------------------------- //
void guMediaViewerPortableDevice::EditProperties( void )
{
    guPortableMediaProperties * PortableMediaProperties = new guPortableMediaProperties( this, m_PortableDevice );
    if( PortableMediaProperties )
    {
        if( PortableMediaProperties->ShowModal() == wxID_OK )
        {
            PortableMediaProperties->WriteConfig();
            //
            UpdateCollectionProperties();
        }
        PortableMediaProperties->Destroy();
    }
}

}

// -------------------------------------------------------------------------------- //
