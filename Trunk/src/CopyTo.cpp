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
#include "CopyTo.h"

#include "Images.h"
#include "TagInfo.h"
#include "Transcode.h"

#include <wx/tokenzr.h>

#include <wx/arrimpl.cpp>

WX_DEFINE_OBJARRAY( guCopyToActionArray );

// -------------------------------------------------------------------------------- //
// guCopyToAction
// -------------------------------------------------------------------------------- //
guCopyToAction::guCopyToAction()
{
    m_Type = guCOPYTO_ACTION_NONE;
    m_Tracks = NULL;
    m_Format = wxNOT_FOUND;
    m_Quality = wxNOT_FOUND;
    m_MoveFiles = false;
    m_Db = NULL;
    m_PortableMediaViewCtrl = NULL;
}

// -------------------------------------------------------------------------------- //
guCopyToAction::guCopyToAction( guTrackArray * tracks, guLibPanel * libpanel, const wxString &destdir, const wxString &pattern, int format, int quality, bool movefiles )
{
    m_Type = guCOPYTO_ACTION_COPYTO;
    m_Tracks = tracks;
    m_LibPanel = libpanel;
    m_Db = libpanel->GetDb();
    m_PlayListFile = NULL;
    m_DestDir = destdir;
    m_Pattern = pattern;
    m_Format = format;
    m_Quality = quality;
    m_MoveFiles = movefiles;
    m_PortableMediaViewCtrl = NULL;

    if( !m_DestDir.EndsWith( wxT( "/" ) ) )
        m_DestDir.Append( wxT( "/" ) );
}

// -------------------------------------------------------------------------------- //
guCopyToAction::guCopyToAction( guTrackArray * tracks, guDbLibrary * db, guPortableMediaViewCtrl * portablemediaviewctrl )
{
    m_Type = guCOPYTO_ACTION_COPYTODEVICE;
    m_Tracks = tracks;
    m_MoveFiles = false;
    m_Db = db;
    m_PlayListFile = NULL;
    m_PortableMediaViewCtrl = portablemediaviewctrl;
    guPortableMediaDevice * PortableMediaDevice = m_PortableMediaViewCtrl->MediaDevice();
    if( PortableMediaDevice->Type() == guPORTABLEMEDIA_TYPE_IPOD )
        m_Type = guCOPYTO_ACTION_COPYTOIPOD;
    m_Format = PortableMediaDevice->TranscodeFormat();
    m_Quality = PortableMediaDevice->TranscodeQuality();
    m_Pattern = PortableMediaDevice->Pattern();
    m_DestDir = PortableMediaDevice->MountPath();
    wxArrayString AudioFolders = wxStringTokenize( PortableMediaDevice->AudioFolders(), wxT( "," ) );
    m_DestDir += AudioFolders[ 0 ].Trim( true ).Trim( false );
    if( m_DestDir.EndsWith( wxT( "//" ) ) )
        m_DestDir.RemoveLast();
    else if( !m_DestDir.EndsWith( wxT( "/" ) ) )
        m_DestDir.Append( wxT( "/" ) );
}

// -------------------------------------------------------------------------------- //
guCopyToAction::guCopyToAction( wxString * playlistpath, guDbLibrary * db, guPortableMediaViewCtrl * portablemediaviewctrl )
{
    m_Type = guCOPYTO_ACTION_COPYTODEVICE;
    m_Tracks = new guTrackArray();
    m_MoveFiles = false;
    m_Db = db;
    m_PortableMediaViewCtrl = portablemediaviewctrl;
    m_PlayListFile = new guPlayListFile( * playlistpath );
    m_PlayListFile->SetName( * playlistpath );
    guPortableMediaDevice * PortableMediaDevice = m_PortableMediaViewCtrl->MediaDevice();
    if( PortableMediaDevice->Type() == guPORTABLEMEDIA_TYPE_IPOD )
        m_Type = guCOPYTO_ACTION_COPYTOIPOD;
    m_Format = PortableMediaDevice->TranscodeFormat();
    m_Quality = PortableMediaDevice->TranscodeQuality();
    m_Pattern = PortableMediaDevice->Pattern();
    m_DestDir = PortableMediaDevice->MountPath();
    wxArrayString AudioFolders = wxStringTokenize( PortableMediaDevice->AudioFolders(), wxT( "," ) );
    m_DestDir += AudioFolders[ 0 ].Trim( true ).Trim( false );
    if( m_DestDir.EndsWith( wxT( "//" ) ) )
        m_DestDir.RemoveLast();
    else if( !m_DestDir.EndsWith( wxT( "/" ) ) )
        m_DestDir.Append( wxT( "/" ) );

    int Index;
    int Count = m_PlayListFile->Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guTrack * CurTrack = new guTrack();
        if( m_Db->FindTrackFile( m_PlayListFile->GetItem( Index ).m_Location, CurTrack ) )
        {
            m_Tracks->Add( CurTrack );
        }
        else
        {
            delete CurTrack;
        }
    }

    //
    delete playlistpath;
}

// -------------------------------------------------------------------------------- //
guCopyToAction::~guCopyToAction()
{
    if( m_Tracks )
    {
        delete m_Tracks;
    }

    if( m_PlayListFile )
    {
        delete m_PlayListFile;
    }
}

// -------------------------------------------------------------------------------- //
// guCopyToThread
// -------------------------------------------------------------------------------- //
guCopyToThread::guCopyToThread( guMainFrame * mainframe, int gaugeid )
{
    m_MainFrame = mainframe;
    m_GaugeId = gaugeid;
    m_CopyToActions = new guCopyToActionArray();
    m_CurrentFile = 0;
    m_FileCount = 0;

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
    }
}


// -------------------------------------------------------------------------------- //
guCopyToThread::~guCopyToThread()
{
    wxCommandEvent Event( wxEVT_COMMAND_MENU_SELECTED, ID_STATUSBAR_GAUGE_REMOVE );
    Event.SetInt( m_GaugeId );
    wxPostEvent( m_MainFrame, Event );

    if( m_CopyToActions )
    {
        delete m_CopyToActions;
    }

    if( !TestDestroy() )
    {
        m_MainFrame->CopyToThreadFinished();
    }
}

// -------------------------------------------------------------------------------- //
void guCopyToThread::AddAction( guTrackArray * tracks, guLibPanel * libpanel, const wxString &destdir, const wxString &pattern, int format, int quality, bool movefiles )
{
    guCopyToAction * CopyToAction = new guCopyToAction( tracks, libpanel, destdir, pattern, format, quality, movefiles );
    if( CopyToAction )
    {
        m_CopyToActionsMutex.Lock();
        m_CopyToActions->Add( CopyToAction );
        m_FileCount += CopyToAction->Count();
        m_CopyToActionsMutex.Unlock();

        if( !IsRunning() )
        {
            Run();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guCopyToThread::AddAction( guTrackArray * tracks, guDbLibrary * db, guPortableMediaViewCtrl * portablemediaviewctrl )
{
    guCopyToAction * CopyToAction = new guCopyToAction( tracks, db, portablemediaviewctrl );
    if( CopyToAction )
    {
        m_CopyToActionsMutex.Lock();
        m_CopyToActions->Add( CopyToAction );
        m_FileCount += CopyToAction->Count();
        m_CopyToActionsMutex.Unlock();

        if( !IsRunning() )
        {
            Run();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guCopyToThread::AddAction( wxString * playlistpath, guDbLibrary * db, guPortableMediaViewCtrl * portablemediaviewctrl )
{
    guCopyToAction * CopyToAction = new guCopyToAction( playlistpath, db, portablemediaviewctrl );
    if( CopyToAction )
    {
        m_CopyToActionsMutex.Lock();
        m_CopyToActions->Add( CopyToAction );
        m_FileCount += CopyToAction->Count();
        m_CopyToActionsMutex.Unlock();

        if( !IsRunning() )
        {
            Run();
        }
    }
}

// -------------------------------------------------------------------------------- //
bool guCopyToThread::CopyFile( const wxString &from, const wxString &to )
{
    bool RetVal = true;
    guLogMessage( wxT( "Copy %s =>> %s" ), from.c_str(), to.c_str() );
    if( wxFileName::Mkdir( wxPathOnly( to ), 0777, wxPATH_MKDIR_FULL ) )
    {
        if( !wxCopyFile( from, to ) )
        {
            RetVal = false;
            guLogError( wxT( "Could not copy the file '%s'" ), from.c_str() );
        }
    }
    else
    {
        RetVal = false;
        guLogError( wxT( "Could not create path for copy the file '%s'" ), from.c_str() );
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
bool guCopyToThread::TranscodeFile( const wxString &source, const wxString &target, int format, int quality )
{
    guLogMessage( wxT( "guCopyToDeviceThread::TranscodeFile\n%s\n%s" ), source.c_str(), target.c_str() );
    bool RetVal = false;
    wxString OutFile = target + wxT( "." ) + guTranscodeFormatString( format );
    if( wxFileName::Mkdir( wxPathOnly( target ), 0777, wxPATH_MKDIR_FULL ) )
    {
        guTranscodeThread * TranscodeThread = new guTranscodeThread( source, OutFile, format, quality );
        if( TranscodeThread && TranscodeThread->IsOk() )
        {
            // TODO : Make a better aproach to be sure its running
            Sleep( 1000 );
            while( TranscodeThread->IsTranscoding() )
            {
                Sleep( 200 );
            }

            m_SizeCounter += guGetFileSize( OutFile );
        }
        RetVal = TranscodeThread->IsOk();
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guCopyToThread::DoCopyToAction( guCopyToAction &copytoaction )
{
    int             Index;
    int             Count = copytoaction.Count();
    wxString        FileName;
    wxString        FilePattern;
    wxString        DestDir;
    bool            ActionIsCopy = true;

    FilePattern = copytoaction.Pattern();
    guLogMessage( wxT( "Using pattern '%s'" ), FilePattern.c_str() );

    DestDir = copytoaction.DestDir();

    for( Index = 0; Index < Count; Index++ )
    {

        if( TestDestroy() )
            break;

        guTrack * CurTrack = copytoaction.Track( Index );

        //
        m_CurrentFile++;

        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_STATUSBAR_GAUGE_SETMAX );
        event.SetInt( m_GaugeId );
        event.SetExtraLong( m_FileCount );
        wxPostEvent( m_MainFrame, event );


        event.SetId( ID_STATUSBAR_GAUGE_UPDATE );
        event.SetInt( m_GaugeId );
        event.SetExtraLong( m_CurrentFile );
        wxPostEvent( m_MainFrame, event );

        if( CurTrack->m_Type == guTRACK_TYPE_RADIOSTATION )
            continue;

        bool Result;
        FileName = wxEmptyString;

#ifdef WITH_LIBGPOD_SUPPORT
        if( copytoaction.Type() == guCOPYTO_ACTION_COPYTOIPOD )
        {
            int FileSize = copytoaction.PortableMediaViewCtrl()->CopyTo( CurTrack, FileName );

            if( FileSize == wxNOT_FOUND )
                Result = false;
            else
            {
                Result = true;
                m_SizeCounter += FileSize;

                // Add the file to the files to add list so the library update it when this job is done
                m_FilesToAdd.Add( FileName );
            }
        }
        else
#endif
        {

            FileName = DestDir + guExpandTrackMacros( FilePattern, CurTrack, m_CurrentFile - 1 );

            // Replace all the special chars < > : " / \ | ? *
            FileName.Replace( wxT( "<" ), wxT( "_" ) );
            FileName.Replace( wxT( ">" ), wxT( "_" ) );
            FileName.Replace( wxT( ":" ), wxT( "_" ) );
            FileName.Replace( wxT( "\"" ), wxT( "_" ) );
            FileName.Replace( wxT( "|" ), wxT( "_" ) );
            FileName.Replace( wxT( "?" ), wxT( "_" ) );
            FileName.Replace( wxT( "*" ), wxT( "_" ) );


            if( copytoaction.Format() == guTRANSCODE_FORMAT_KEEP )
            {
                ActionIsCopy = true;
            }
            else
            {
                int FileFormat = guGetTranscodeFileFormat( CurTrack->m_FileName.Lower().AfterLast( wxT( '.' ) ) );

                if( copytoaction.Type() == guCOPYTO_ACTION_COPYTO )
                {
                    if( FileFormat == copytoaction.Format() )
                    {
                        if( copytoaction.Quality() == guTRANSCODE_QUALITY_KEEP )
                        {
                            ActionIsCopy = true;
                        }
                        else
                        {
                            int FileBitrate = 0;
                            switch( FileFormat )
                            {
                                case guPORTABLEMEDIA_AUDIO_FORMAT_MP3 :
                                case guPORTABLEMEDIA_AUDIO_FORMAT_AAC :
                                case guPORTABLEMEDIA_AUDIO_FORMAT_WMA :
                                {
                                    FileBitrate = guGetMp3QualityBitRate( copytoaction.Quality() );
                                    break;
                                }

                                case guPORTABLEMEDIA_AUDIO_FORMAT_OGG :
                                {
                                    FileBitrate = guGetOggQualityBitRate( copytoaction.Quality() );
                                    break;
                                }

                                case guPORTABLEMEDIA_AUDIO_FORMAT_FLAC :
                                {
                                    FileBitrate = 0;
                                    break;
                                }
                            }

                            if( CurTrack->m_Bitrate > FileBitrate )
                            {
                                ActionIsCopy = false;
                            }
                            else
                            {
                                ActionIsCopy = true;
                            }
                        }
                    }
                    else
                    {
                        ActionIsCopy = false;
                    }
                }
                else
                {
                    //guLogMessage( wxT( "AudioFormats: %08X  %08X" ), m_Device->AudioFormats(), FileFormat );
                    guPortableMediaDevice * MediaDevice  = copytoaction.PortableMediaDevice();
                    // If the file is not supported then need to transcode it
                    if( !( MediaDevice->AudioFormats() & FileFormat ) )
                    {
                        guLogMessage( wxT( "Its an unsupported format... Transcoding" ) );
                        ActionIsCopy = false;
                    }
                    else    // The file is supported
                    {
                        guLogMessage( wxT( "Its a supported format" ) );
                        // The file is supported and we dont need to trasncode in all cases so copy the file
                        if( MediaDevice->TranscodeScope() != guPORTABLEMEDIA_TRANSCODE_SCOPE_ALWAYS )
                        {
                            ActionIsCopy = true;
                        }
                        else
                        {
                            //guLogMessage( wxT( "TranscodeFOrmat: %u      FileFormat: %i" ), m_Device->TranscodeFormat(), FileFormat );
                            // The file is the same selected in the format so check if the bitrate is
                            if( MediaDevice->TranscodeFormat() == FileFormat )
                            {
                                if( MediaDevice->TranscodeQuality() != guTRANSCODE_QUALITY_KEEP )
                                {
                                    int FileBitrate = 0;
                                    switch( FileFormat )
                                    {
                                        case guPORTABLEMEDIA_AUDIO_FORMAT_MP3 :
                                        case guPORTABLEMEDIA_AUDIO_FORMAT_AAC :
                                        case guPORTABLEMEDIA_AUDIO_FORMAT_WMA :
                                        {
                                            FileBitrate = guGetMp3QualityBitRate( MediaDevice->TranscodeQuality() );
                                            break;
                                        }

                                        case guPORTABLEMEDIA_AUDIO_FORMAT_OGG :
                                        {
                                            FileBitrate = guGetOggQualityBitRate( MediaDevice->TranscodeQuality() );
                                            break;
                                        }

                                        case guPORTABLEMEDIA_AUDIO_FORMAT_FLAC :
                                        {
                                            FileBitrate = 0;
                                            break;
                                        }
                                    }

                                    if( CurTrack->m_Bitrate > FileBitrate )
                                    {
                                        ActionIsCopy = false;
                                    }
                                    else
                                    {
                                        ActionIsCopy = true;
                                    }


                                }
                                else
                                {
                                    ActionIsCopy = true;
                                }
                            }
                            else
                            {
                                ActionIsCopy = false;
                            }
                        }
                    }
                }
            }

            if( ActionIsCopy )
            {
                FileName += wxT( '.' ) + CurTrack->m_FileName.Lower().AfterLast( wxT( '.' ) );
                Result = CopyFile( CurTrack->m_FileName, FileName );
                m_SizeCounter += CurTrack->m_FileSize;
            }
            else
            {
                Result = TranscodeFile( CurTrack->m_FileName, FileName, copytoaction.Format(), copytoaction.Quality() );
            }

            // Add the file to the files to add list so the library update it when this job is done
            m_FilesToAdd.Add( FileName );

            // If have cover assigned
            if( CurTrack->m_CoverId )
            {
                guDbLibrary * Db = copytoaction.Db();

                if( copytoaction.Type() == guCOPYTO_ACTION_COPYTO )
                {
                    wxString CoverPath = Db->GetCoverPath( CurTrack->m_CoverId );
                    wxString NewCoverFile = wxPathOnly( FileName ) + wxT( "/" ) + CoverPath.AfterLast( wxT( '/' ) );
                    //guLogMessage( wxT( "COpying file %s" ), NewCoverFile.c_str() );
                    if( !wxFileExists( NewCoverFile ) )
                    {
                        if( !wxCopyFile( CoverPath, NewCoverFile ) )
                        {
                            guLogMessage( wxT( "Could not copy the cover %s" ), NewCoverFile.c_str() );
                        }
                        else
                        {
                            m_CoversToAdd.Add( NewCoverFile );
                        }
                    }
                }
                else
                {
                    guPortableMediaDevice * MediaDevice  = copytoaction.PortableMediaDevice();
                    //
                    // If the device supports covers
                    //
                    int DevCoverFormats = MediaDevice->CoverFormats();
                    if( DevCoverFormats ) // if has cover handling enabled
                    {
                        wxString CoverPath = Db->GetCoverPath( CurTrack->m_CoverId );
                        wxImage * CoverImage = new wxImage( CoverPath );
                        if( CoverImage )
                        {
                            if( CoverImage->IsOk() )
                            {
                                int DevCoverSize = MediaDevice->CoverSize();
                                if( DevCoverSize && ( ( CoverImage->GetWidth() != DevCoverSize ) || ( CoverImage->GetHeight() != DevCoverSize ) ) )
                                {
                                    CoverImage->Rescale( DevCoverSize, DevCoverSize, wxIMAGE_QUALITY_HIGH );
                                }

                                bool CoverAdded = false;
                                if( DevCoverFormats & guPORTABLEMEDIA_COVER_FORMAT_EMBEDDED )
                                {
                                    if( !guTagSetPicture( FileName, CoverImage ) )
                                    {
                                        guLogMessage( wxT( "Couldnt set the picture to %s" ), FileName.c_str() );
                                    }
                                }

                                wxString DevCoverName = MediaDevice->CoverName();
                                if( DevCoverName.IsEmpty() )
                                {
                                    DevCoverName = wxT( "cover" );
                                }
                                DevCoverName = wxPathOnly( FileName ) + wxT( "/" ) + DevCoverName;

                                if( DevCoverFormats & guPORTABLEMEDIA_COVER_FORMAT_JPEG )
                                {
                                    if( !wxFileExists( DevCoverName + wxT( ".jpg" ) ) )
                                    {
                                        if( !CoverImage->SaveFile( DevCoverName + wxT( ".jpg" ), wxBITMAP_TYPE_JPEG ) )
                                        {
                                            guLogError( wxT( "Could not copy the cover to %s" ), DevCoverName.c_str() );
                                        }
                                        else
                                        {
                                            m_CoversToAdd.Add( DevCoverName + wxT( ".jpg" ) );
                                            CoverAdded = true;
                                        }
                                    }
                                }

                                if( DevCoverFormats & guPORTABLEMEDIA_COVER_FORMAT_PNG )
                                {
                                    if( !wxFileExists( DevCoverName + wxT( ".png" ) ) )
                                    {
                                        if( !CoverImage->SaveFile( DevCoverName + wxT( ".png" ), wxBITMAP_TYPE_PNG ) )
                                        {
                                            guLogError( wxT( "Could not copy the cover to %s" ), DevCoverName.c_str() );
                                        }
                                        else if( !CoverAdded )
                                        {
                                            m_CoversToAdd.Add( DevCoverName + wxT( ".png" ) );
                                            CoverAdded = true;
                                        }
                                    }
                                }

                                if( DevCoverFormats & guPORTABLEMEDIA_COVER_FORMAT_GIF )
                                {
                                    if( !wxFileExists( DevCoverName + wxT( ".gif" ) ) )
                                    {
                                        if( !CoverImage->SaveFile( DevCoverName + wxT( ".gif" ), wxBITMAP_TYPE_GIF ) )
                                        {
                                            guLogError( wxT( "Could not copy the cover to %s" ), DevCoverName.c_str() );
                                        }
                                        else if( !CoverAdded )
                                        {
                                            m_CoversToAdd.Add( DevCoverName + wxT( ".gif" ) );
                                            CoverAdded = true;
                                        }
                                    }
                                }

                                if( DevCoverFormats & guPORTABLEMEDIA_COVER_FORMAT_BMP )
                                {
                                    if( !wxFileExists( DevCoverName + wxT( ".bmp" ) ) )
                                    {
                                        if( !CoverImage->SaveFile( DevCoverName + wxT( ".bmp" ), wxBITMAP_TYPE_BMP ) )
                                        {
                                            guLogError( wxT( "Could not copy the cover to %s" ), DevCoverName.c_str() );
                                        }
                                        else if( !CoverAdded )
                                        {
                                            m_CoversToAdd.Add( DevCoverName + wxT( ".bmp" ) );
                                            //CoverAdded = true;
                                        }
                                    }
                                }
                            }

                            delete CoverImage;
                        }
                    }
                }
            }
        }

        if( Result && copytoaction.MoveFiles() )
        {
            // Need to delete the old files
            //CopyToAction.LibPanel()->DeleteTracks( CopyToAction.Tracks() );
            m_DeleteTracks.Add( new guTrack( * CurTrack ) );
        }
    }
}

#ifdef WITH_LIBGPOD_SUPPORT
// -------------------------------------------------------------------------------- //
bool guMoreCopyToIpodActions( guCopyToActionArray * actions )
{
    if( actions->Count() > 1 )
    {
        int Index;
        int Count = actions->Count();
        for( Index = 1; Index < Count; Index++ )
        {
            guCopyToAction &CopyToAction = actions->Item( Index );
            if( CopyToAction.Type() == guCOPYTO_ACTION_COPYTOIPOD )
                return true;
        }
    }
    return false;
}
#endif

// -------------------------------------------------------------------------------- //
guCopyToThread::ExitCode guCopyToThread::Entry()
{
	long IdleTime = wxNOT_FOUND;
	unsigned int TimeCounter = wxNOT_FOUND;
	unsigned int FileCounter = 0;
    while( !TestDestroy() )
    {
        if( m_CopyToActions->Count() )
        {
            IdleTime = wxNOT_FOUND;
            if( ( int ) TimeCounter == wxNOT_FOUND )
            {
                TimeCounter = wxGetLocalTime();
                m_SizeCounter = 0;
                FileCounter = 0;
            }

            m_FilesToAdd.Empty();
            m_DeleteTracks.Empty();
            m_CoversToAdd.Empty();
            m_CopyToActionsMutex.Lock();
            guCopyToAction &CopyToAction = m_CopyToActions->Item( 0 );
            m_CopyToActionsMutex.Unlock();

            DoCopyToAction( CopyToAction );

            FileCounter += CopyToAction.Count();


#ifdef WITH_LIBGPOD_SUPPORT
            if( CopyToAction.Type() == guCOPYTO_ACTION_COPYTOIPOD )
            {
                guPlayListFile * PlayListFile = CopyToAction.PlayListFile();
                guIpodLibrary * IpodDb = ( guIpodLibrary *  ) ( ( guIpodMediaLibPanel * ) CopyToAction.PortableMediaViewCtrl()->LibPanel() )->GetDb();
                if( PlayListFile )
                {
                    guLogMessage( wxT( "It was a playlist...") );
                    IpodDb->CreateiPodPlayList( PlayListFile->GetName(), m_FilesToAdd );
                }
                if( !guMoreCopyToIpodActions( m_CopyToActions ) )
                {
                    IpodDb->iPodFlush();
                    ( ( guIpodMediaLibPanel * ) CopyToAction.PortableMediaViewCtrl()->LibPanel() )->DoUpdate( true );
                }
            }
            else
#endif
            if( CopyToAction.Type() == guCOPYTO_ACTION_COPYTODEVICE )
            {
                int Index;
                int Count;
                // Update the files
                guPortableMediaLibrary * PortableMediaDb = CopyToAction.PortableMediaViewCtrl()->Db();
                PortableMediaDb->AddFiles( m_FilesToAdd );

                guPlayListFile * PlayListFile = CopyToAction.PlayListFile();
                if( PlayListFile )
                {
                    guLogMessage( wxT( "Normal device and is from a playlist..." ) );
                    wxArrayInt TrackIds;
                    // The tracks just copied was part of a playlist we need to create too
                    Count = m_FilesToAdd.Count();
                    for( Index = 0; Index < Count; Index++ )
                    {
                        int TrackId = PortableMediaDb->FindTrackFile( m_FilesToAdd[ Index ], NULL );
                        if( TrackId )
                            TrackIds.Add( TrackId );
                    }
                    if( TrackIds.Count() )
                    {
                        int PLId = PortableMediaDb->CreateStaticPlayList( PlayListFile->GetName(), TrackIds );
                        PortableMediaDb->UpdateStaticPlayListFile( PLId );

                        wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYLIST_UPDATED );
                        evt.SetClientData( ( void * ) CopyToAction.PortableMediaViewCtrl()->LibPanel() );
                        wxPostEvent( wxTheApp->GetTopWindow(), evt );
                    }
                }

                // Update the covers
                wxString DevCoverName = CopyToAction.PortableMediaDevice()->CoverName();
                if( DevCoverName.IsEmpty() )
                {
                    DevCoverName = wxT( "cover" );
                }
                DevCoverName += wxT( ".jpg" );
                Count = m_CoversToAdd.Count();
                for( Index = 0; Index < Count; Index++ )
                {
                    CopyToAction.PortableMediaViewCtrl()->Db()->UpdateImageFile( m_CoversToAdd[ Index ].ToUTF8(), DevCoverName.ToUTF8() );
                }

                if( CopyToAction.PortableMediaViewCtrl()->LibPanel() )
                {
                    wxCommandEvent Event( wxEVT_COMMAND_MENU_SELECTED, ID_LIBRARY_RELOADCONTROLS );
                    Event.SetClientData( CopyToAction.PortableMediaViewCtrl()->LibPanel() );
                    m_MainFrame->AddPendingEvent( Event );
                }
            }

            if( m_DeleteTracks.Count() )
            {
                // Need to delete the old files
                CopyToAction.LibPanel()->DeleteTracks( &m_DeleteTracks );
            }

            m_CopyToActionsMutex.Lock();
            m_CopyToActions->RemoveAt( 0 );
            m_CopyToActionsMutex.Unlock();

            // If was the last items in the list we send the notification
            if( !m_CopyToActions->Count() )
            {
                // Send notification that the job finished
                guDBusNotify * NotifySrv = m_MainFrame->GetNotifyObject();
                if( NotifySrv )
                {
                    TimeCounter = wxGetLocalTime() - TimeCounter;
                    wxString FinishMsg = wxString::Format( _( "Copied %u files (%s)\nin %s seconds" ),
                        FileCounter,
                        SizeToString( m_SizeCounter ).c_str(),
                        LenToString( TimeCounter ).c_str() );
                    wxImage IconImg = guImage( guIMAGE_INDEX_guayadeque );
                    NotifySrv->Notify( wxEmptyString, _( "Finished copying files" ), FinishMsg, &IconImg );
                }
                //
                TimeCounter = wxNOT_FOUND;
            }
        }
        else
        {
            if( IdleTime == wxNOT_FOUND )
            {
                IdleTime = wxGetLocalTime();

                m_FileCount = 0;
                m_CurrentFile = 0;
            }

            // we have been waiting in idle for more than 10 seconds now
            if( wxGetLocalTime() - IdleTime > 10 )
            {
                break;
            }
            Sleep( 1000 );
        }
    }

    return 0;
}

// -------------------------------------------------------------------------------- //
