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
#include "MediaRecordCtrl.h"

#include "Config.h"
#include "FileRenamer.h"
#include "MediaCtrl.h"
#include "PlayerPanel.h"
#include "TagInfo.h"

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
guMediaRecordCtrl::guMediaRecordCtrl( guPlayerPanel * playerpanel, guMediaCtrl * mediactrl )
{
    m_PlayerPanel = playerpanel;
    m_MediaCtrl = mediactrl;
    m_Recording = false;
    m_FirstChange = false;

    UpdatedConfig();
}

// -------------------------------------------------------------------------------- //
guMediaRecordCtrl::~guMediaRecordCtrl()
{
    guLogDebug( "guMediaRecordCtrl::~guMediaRecordCtrl" );
}

// -------------------------------------------------------------------------------- //
void guMediaRecordCtrl::UpdatedConfig( void )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    m_MainPath = Config->ReadStr( CONFIG_KEY_RECORD_PATH, guPATH_DEFAULT_RECORDINGS, CONFIG_PATH_RECORD );
    m_Format = Config->ReadNum( CONFIG_KEY_RECORD_FORMAT, guRECORD_FORMAT_MP3, CONFIG_PATH_RECORD );
    m_Quality = Config->ReadNum( CONFIG_KEY_RECORD_QUALITY, guRECORD_QUALITY_NORMAL, CONFIG_PATH_RECORD );
    m_SplitTracks = Config->ReadBool( CONFIG_KEY_RECORD_SPLIT, false, CONFIG_PATH_RECORD );
    m_DeleteTracks = Config->ReadBool( CONFIG_KEY_RECORD_DELETE, false, CONFIG_PATH_RECORD );
    m_DeleteTime = Config->ReadNum( CONFIG_KEY_RECORD_DELETE_TIME, 55, CONFIG_PATH_RECORD ) * 1000;

    if( !m_MainPath.EndsWith( wxT( "/" ) ) )
        m_MainPath += wxT( "/" );

    switch( m_Format )
    {
        case guRECORD_FORMAT_MP3 :
            m_Ext = wxT( ".mp3" );
            break;
        case guRECORD_FORMAT_OGG :
            m_Ext = wxT( ".ogg" );
            break;
        case guRECORD_FORMAT_FLAC :
            m_Ext = wxT( ".flac" );
            break;
    }

    //guLogDebug( wxT( "Record to '%s' %i, %i '%s'" ), m_MainPath.c_str(), m_Format, m_Quality, m_Ext.c_str() );
}

// -------------------------------------------------------------------------------- //
bool guMediaRecordCtrl::Start( const guTrack * track )
{
    //guLogDebug( wxT( "guMediaRecordCtrl::Start" ) );
    m_TrackInfo = * track;

    if( m_TrackInfo.m_SongName.IsEmpty() )
        m_TrackInfo.m_SongName = wxT( "Record" );

    m_FileName = GenerateRecordFileName();

    wxFileName::Mkdir( wxPathOnly( m_FileName ), 0770, wxPATH_MKDIR_FULL );

    m_Recording = m_MediaCtrl->EnableRecord( m_FileName, m_Format, m_Quality );

    return m_Recording;
}

// -------------------------------------------------------------------------------- //
bool guMediaRecordCtrl::Stop( void )
{
    //guLogDebug( wxT( "guMediaRecordCtrl::Stop" ) );
    if( m_Recording )
    {
        m_MediaCtrl->DisableRecord();
        m_Recording = false;

        SaveTagInfo( m_PrevFileName, &m_PrevTrack );
        m_PrevFileName = wxEmptyString;
    }
    return true;
}

// -------------------------------------------------------------------------------- //
wxString guMediaRecordCtrl::GenerateRecordFileName( void )
{
    wxString FileName = m_MainPath;

    if( !m_TrackInfo.m_AlbumName.IsEmpty() )
    {
        FileName += NormalizeField( m_TrackInfo.m_AlbumName );
    }
    else
    {
        wxURI Uri( m_TrackInfo.m_FileName );
        FileName += NormalizeField( Uri.GetServer() + wxT( "-" ) + Uri.GetPath() );
    }

    FileName += wxT( "/" );
    if( !m_TrackInfo.m_ArtistName.IsEmpty() )
    {
        FileName += NormalizeField( m_TrackInfo.m_ArtistName ) + wxT( " - " );
    }
    FileName += NormalizeField( m_TrackInfo.m_SongName ) + m_Ext;

    //guLogDebug( wxT( "The New Record Location is : '%s'" ), FileName.c_str() );
    return FileName;
}

// -------------------------------------------------------------------------------- //
void guMediaRecordCtrl::SplitTrack( void )
{
    guLogMessage( wxT( "guMediaRecordCtrl::SplitTrack" ) );

    m_FileName = GenerateRecordFileName();
    m_MediaCtrl->SetRecordFileName( m_FileName );

    SaveTagInfo( m_PrevFileName, &m_PrevTrack );

    m_PrevFileName = m_FileName;
    m_PrevTrack = m_TrackInfo;
}

// -------------------------------------------------------------------------------- //
bool guMediaRecordCtrl::SaveTagInfo( const wxString &filename, const guTrack * Track )
{
    bool RetVal = true;
    guTagInfo * TagInfo;

    if( !filename.IsEmpty() && wxFileExists( filename ) )
    {
        TagInfo = guGetTagInfoHandler( filename );

        if( TagInfo )
        {
            if( m_DeleteTracks )
            {
                TagInfo->Read();
                if( TagInfo->m_Length < m_DeleteTime )
                {
                    wxRemoveFile( filename );
                    delete TagInfo;
                    return true;
                }
            }

            TagInfo->m_AlbumName = Track->m_AlbumName;
            TagInfo->m_ArtistName = Track->m_ArtistName;
            TagInfo->m_GenreName = Track->m_GenreName;
            TagInfo->m_TrackName = Track->m_SongName;

            if( !( RetVal = TagInfo->Write( guTRACK_CHANGED_DATA_TAGS ) ) )
            {
                guLogError( wxT( "Could not set tags to the record track" ) );
            }

            delete TagInfo;
        }
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guMediaRecordCtrl::SetTrack( const guTrack &track )
{
    guLogMessage( wxT( "guMediaRecordCtrl::SetTrack" ) );
    m_TrackInfo = track;
    SplitTrack();
}

// -------------------------------------------------------------------------------- //
void guMediaRecordCtrl::SetTrackName( const wxString &artistname, const wxString &trackname )
{
    bool NeedSplit = false;

    // If its the first file Set it so the tags are saved
    if( m_PrevFileName.IsEmpty() )
    {
        m_PrevFileName = m_FileName;
        m_PrevTrack = m_TrackInfo;
    }
    if( m_TrackInfo.m_ArtistName != artistname )
    {
        m_TrackInfo.m_ArtistName = artistname;
        NeedSplit = true;
    }
    if( m_TrackInfo.m_SongName != trackname )
    {
        m_TrackInfo.m_SongName = trackname;
        NeedSplit = true;
    }
    if( NeedSplit )
    {
        SplitTrack();
    }
}

// -------------------------------------------------------------------------------- //
void guMediaRecordCtrl::SetStation( const wxString &station )
{
    guLogMessage( wxT( "guMediaRecordCtrl::SetStation" ) );
    if( m_TrackInfo.m_AlbumName != station )
    {
        m_TrackInfo.m_AlbumName = station;
        SplitTrack();
    }
}

}

// -------------------------------------------------------------------------------- //

