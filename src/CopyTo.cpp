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
#include "CopyTo.h"

#include "FileRenamer.h"    // NormalizeField
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
    m_Db = NULL;
    m_PortableMediaPanel = NULL;
}

// -------------------------------------------------------------------------------- //
guCopyToAction::guCopyToAction( guTrackArray * tracks, const wxString &destdir, const wxString &pattern, int format, int quality )
{
    m_Type = guCOPYTO_ACTION_COPYTO;
    m_Tracks = tracks;
    m_DestDir = destdir;
    m_Pattern = pattern;
    m_Format = format;
    m_Quality = quality;
    m_Db = NULL;
    m_PortableMediaPanel = NULL;

    if( !m_DestDir.EndsWith( wxT( "/" ) ) )
        m_DestDir.Append( wxT( "/" ) );
}

// -------------------------------------------------------------------------------- //
guCopyToAction::guCopyToAction( guTrackArray * tracks, guDbLibrary * db, guPortableMediaPanel * portablemediapanel )
{
    m_Type = guCOPYTO_ACTION_COPYTODEVICE;
    m_Tracks = tracks;
    m_Db = db;
    m_PortableMediaPanel = portablemediapanel;
}

// -------------------------------------------------------------------------------- //
guCopyToAction::~guCopyToAction()
{
    if( m_Tracks )
    {
        delete m_Tracks;
    }
}

// -------------------------------------------------------------------------------- //
wxString guCopyToAction::DestDir( void )
{
    if( ( m_Type == guCOPYTO_ACTION_COPYTODEVICE ) && m_DestDir.IsEmpty() )
    {
        guPortableMediaDevice * PortableMediaDevice = m_PortableMediaPanel->PortableMediaDevice();
        m_DestDir = PortableMediaDevice->MountPath();
        wxArrayString AudioFolders = wxStringTokenize( PortableMediaDevice->AudioFolders(), wxT( "," ) );
        m_DestDir += AudioFolders[ 0 ].Trim( true ).Trim( false );
        if( !m_DestDir.EndsWith( wxT( "/" ) ) )
            m_DestDir.Append( wxT( "/" ) );
    }

    return m_DestDir;
}

// -------------------------------------------------------------------------------- //
wxString guCopyToAction::Pattern( void )
{
    if( ( m_Type == guCOPYTO_ACTION_COPYTODEVICE ) && m_Pattern.IsEmpty() )
    {
        m_Pattern = m_PortableMediaPanel->PortableMediaDevice()->Pattern();
    }
    return m_Pattern;
}

// -------------------------------------------------------------------------------- //
int guCopyToAction::Format( void )
{
    if( ( m_Type == guCOPYTO_ACTION_COPYTODEVICE ) && ( m_Format == wxNOT_FOUND ) )
    {
        m_Format = m_PortableMediaPanel->PortableMediaDevice()->TranscodeFormat();
    }
    return m_Format;
}

// -------------------------------------------------------------------------------- //
int guCopyToAction::Quality( void )
{
    if( ( m_Type == guCOPYTO_ACTION_COPYTODEVICE ) && ( m_Quality == wxNOT_FOUND ) )
    {
        m_Quality = m_PortableMediaPanel->PortableMediaDevice()->TranscodeQuality();
    }
    return m_Quality;
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
    wxCommandEvent Event( wxEVT_COMMAND_MENU_SELECTED, ID_GAUGE_REMOVE );
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
void guCopyToThread::AddAction( guTrackArray * tracks, const wxString &destdir, const wxString &pattern, int format, int quality )
{
    guCopyToAction * CopyToAction = new guCopyToAction( tracks, destdir, pattern, format, quality );
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
void guCopyToThread::AddAction( guTrackArray * tracks, guDbLibrary * db, guPortableMediaPanel * portablemediapanel )
{
    guCopyToAction * CopyToAction = new guCopyToAction( tracks, db, portablemediapanel );
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
void guCopyToThread::CopyFile( const wxString &from, const wxString &to )
{
    guLogMessage( wxT( "Copy %s =>> %s" ), from.c_str(), to.c_str() );
    if( wxFileName::Mkdir( wxPathOnly( to ), 0777, wxPATH_MKDIR_FULL ) )
    {
        if( !wxCopyFile( from, to ) )
        {
            guLogError( wxT( "Could not copy the file '%s'" ), from.c_str() );
        }
    }
    else
    {
        guLogError( wxT( "Could not create path for copy the file '%s'" ), from.c_str() );
    }
}

// -------------------------------------------------------------------------------- //
void guCopyToThread::TranscodeFile( const wxString &source, const wxString &target, int format, int quality )
{
    guLogMessage( wxT( "guCopyToDeviceThread::TranscodeFile\n%s\n%s" ), source.c_str(), target.c_str() );
    wxString OutFile = target + wxT( "." ) + guGetTranscodeFormatString( format );
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
    }
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

        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_GAUGE_SETMAX );
        event.SetInt( m_GaugeId );
        event.SetExtraLong( m_FileCount );
        wxPostEvent( m_MainFrame, event );


        event.SetId( ID_GAUGE_UPDATE );
        event.SetInt( m_GaugeId );
        event.SetExtraLong( m_CurrentFile );
        wxPostEvent( m_MainFrame, event );

        if( CurTrack->m_Type == guTRACK_TYPE_RADIOSTATION )
            continue;

        FileName = FilePattern;
        FileName.Replace( wxT( "{a}" ), NormalizeField( CurTrack->m_ArtistName ) );
        FileName.Replace( wxT( "{aa}" ), NormalizeField( CurTrack->m_AlbumArtist ) );
        FileName.Replace( wxT( "{b}" ), NormalizeField( CurTrack->m_AlbumName ) );
        FileName.Replace( wxT( "{c}" ), NormalizeField( CurTrack->m_Composer ) );
        FileName.Replace( wxT( "{f}" ), wxFileNameFromPath( CurTrack->m_FileName ) );
        FileName.Replace( wxT( "{g}" ), NormalizeField( CurTrack->m_GenreName ) );
        FileName.Replace( wxT( "{n}" ), wxString::Format( wxT( "%02u" ), CurTrack->m_Number ) );
        FileName.Replace( wxT( "{t}" ), NormalizeField( CurTrack->m_SongName ) );
        FileName.Replace( wxT( "{y}" ), wxString::Format( wxT( "%u" ), CurTrack->m_Year ) );
        FileName.Replace( wxT( "{d}" ), NormalizeField( CurTrack->m_Disk ) );

        FileName = DestDir + FileName;

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
                    //guLogMessage( wxT( "Its an unsupported format... Transcoding" ) );
                    ActionIsCopy = false;
                }
                else    // The file is supported
                {
                    //guLogMessage( wxT( "Its a supported format" ) );
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
            CopyFile( CurTrack->m_FileName, FileName );
            m_SizeCounter += CurTrack->m_FileSize;
        }
        else
        {
            TranscodeFile( CurTrack->m_FileName, FileName, copytoaction.Format(), copytoaction.Quality() );
        }

        // If have cover assigned
        if( CurTrack->m_CoverId )
        {
            if( copytoaction.Type() == guCOPYTO_ACTION_COPYTO )
            {
            }
            else
            {
                guPortableMediaDevice * MediaDevice  = copytoaction.PortableMediaDevice();
                guDbLibrary * Db = copytoaction.Db();
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
                                }
                            }
                        }

                        delete CoverImage;
                    }
                }
            }
        }



//        event.SetId( ID_GAUGE_UPDATE );
//        event.SetInt( m_GaugeId );
//        event.SetExtraLong( Index + 1 );
//        wxPostEvent( m_MainFrame, event );
    }


#if 0
            // If have cover assigned
            if( CurTrack->m_CoverId )
            {
                //
                // If the device supports covers
                //
                int DevCoverFormats = m_Device->CoverFormats();
                if( DevCoverFormats ) // if has cover handling enabled
                {
                    wxString CoverPath = m_Db->GetCoverPath( CurTrack->m_CoverId );
                    wxImage * CoverImage = new wxImage( CoverPath );
                    if( CoverImage )
                    {
                        if( CoverImage->IsOk() )
                        {
                            int DevCoverSize = m_Device->CoverSize();
                            if( DevCoverSize && ( ( CoverImage->GetWidth() != DevCoverSize ) || ( CoverImage->GetHeight() != DevCoverSize ) ) )
                            {
                                CoverImage->Rescale( DevCoverSize, DevCoverSize, wxIMAGE_QUALITY_HIGH );
                            }

                            if( DevCoverFormats & guPORTABLEMEDIA_COVER_FORMAT_EMBEDDED )
                            {
                                if( !guTagSetPicture( FileName, CoverImage ) )
                                {
                                    guLogMessage( wxT( "Couldnt set the picture to %s" ), FileName.c_str() );
                                }
                            }

                            wxString DevCoverName = m_Device->CoverName();
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
                                }
                            }
                        }

                        delete CoverImage;
                    }
                }
            }
        }

//        m_SizeCounter += CurTrack->m_FileSize;
//        //
        event.SetId( ID_GAUGE_UPDATE );
        event.SetInt( m_GaugeId );
        event.SetExtraLong( Index + 1 );
        wxPostEvent( MainFrame, event );
    }

    //
    event.SetId( ID_GAUGE_REMOVE );
    event.SetInt( m_GaugeId );
    wxPostEvent( MainFrame, event );
    //wxMessageBox( "Copy to dir finished" );

    event.SetId( ID_MENU_UPDATE_LIBRARY );
    event.SetClientData( m_Panel );
    wxPostEvent( MainFrame, event );

    guDBusNotify * NotifySrv = MainFrame->GetNotifyObject();
    if( NotifySrv )
    {
        TimeCounter = wxGetLocalTime() - TimeCounter;
        wxString FinishMsg = wxString::Format( _( "Copied %u files (%s)\nin %s seconds" ),
            Count,
            SizeToString( m_SizeCounter ).c_str(),
            LenToString( TimeCounter ).c_str() );
        wxImage IconImg = guImage( guIMAGE_INDEX_guayadeque );
        NotifySrv->Notify( wxEmptyString, _( "Finished copying files" ), FinishMsg, &IconImg );
    }

#endif
}

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

            m_CopyToActionsMutex.Lock();
            guCopyToAction &CopyToAction = m_CopyToActions->Item( 0 );
            m_CopyToActionsMutex.Unlock();

            DoCopyToAction( CopyToAction );

            FileCounter += CopyToAction.Count();

            if( CopyToAction.Type() == guCOPYTO_ACTION_COPYTODEVICE )
            {
                // Send the update event to the library holding the files to copy
                wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_MENU_UPDATE_LIBRARY );
                event.SetClientData( CopyToAction.PortableMediaPanel() );
                wxPostEvent( m_MainFrame, event );
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
