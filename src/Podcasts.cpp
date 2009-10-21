// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2009 J.Rios
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
#include "Podcasts.h"

#include "Config.h"
#include "DbLibrary.h"
#include "MainFrame.h"
#include "TagInfo.h"
#include "Utils.h"

#include <wx/arrimpl.cpp>

WX_DEFINE_OBJARRAY(guPodcastChannelArray);
WX_DEFINE_OBJARRAY(guPodcastItemArray);

const wxEventType guPodcastEvent = wxNewEventType();

// -------------------------------------------------------------------------------- //
int StrLengthToInt( const wxString &length )
{
    int RetVal = 0;
    int Factor[] = { 1, 60, 3600, 86400 };
    int FactorIndex = 0;

    if( !length.IsEmpty() )
    {
        // 1:02:03:04
        wxString Rest = length.Strip( wxString::both );
        int element;
        do {
            Rest.AfterLast( wxT( ':' ) ).ToLong( ( long * ) &element );
            if( !element )
                break;
            RetVal += Factor[ FactorIndex ] * element;
            FactorIndex++;
            if( ( FactorIndex > 3 ) )
                break;
            Rest = Rest.BeforeLast( wxT( ':' ) );
        } while( !Rest.IsEmpty() );
    }
//    guLogMessage( wxT( "%s -> %i" ), length.c_str(), RetVal );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
guPodcastChannel::guPodcastChannel( const wxString &url )
{
    m_Url = url;
    ReadContent();
}

// -------------------------------------------------------------------------------- //
void guPodcastChannel::ReadContent( void )
{
    wxCurlHTTP  http;
    guLogMessage( wxT( "The address is %s" ), m_Url.c_str() );

    http.AddHeader( wxT( "User-Agent: Mozilla/5.0 (X11; U; Linux i686; es-ES; rv:1.9.0.5) Gecko/2008121622 Ubuntu/8.10 (intrepid) Firefox/3.0.5" ) );
    http.AddHeader( wxT( "Accept: */*" ) );
    http.AddHeader( wxT( "Accept-Charset: utf-8;iso-8859-1" ) );
    char * Buffer = NULL;
    http.Get( Buffer, m_Url );
    if( Buffer )
    {
        wxMemoryInputStream ins( Buffer, Strlen( Buffer ) );
        wxXmlDocument XmlDoc( ins );
        //wxSt
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        if( XmlNode && XmlNode->GetName() == wxT( "rss" ) )
        {
            ReadXml( XmlNode->GetChildren() );
        }
        free( Buffer );
    }
    else
    {
        guLogError( wxT( "Could not get podcast content for %s" ), m_Url.c_str() );
    }
}

// -------------------------------------------------------------------------------- //
void guPodcastChannel::ReadXml( wxXmlNode * XmlNode )
{
    if( XmlNode && XmlNode->GetName() == wxT( "channel" ) )
    {
        XmlNode = XmlNode->GetChildren();
        while( XmlNode )
        {
            if( XmlNode->GetName() == wxT( "title" ) )
            {
                m_Title = XmlNode->GetNodeContent();
            }
            else if( XmlNode->GetName() == wxT( "link" ) )
            {
                m_Link = XmlNode->GetNodeContent();
            }
            else if( XmlNode->GetName() == wxT( "language" ) )
            {
                m_Lang = XmlNode->GetNodeContent();
            }
            else if( XmlNode->GetName() == wxT( "description" ) )
            {
                m_Description = XmlNode->GetNodeContent();
            }
            else if( XmlNode->GetName() == wxT( "itunes:author" ) )
            {
                m_Author = XmlNode->GetNodeContent();
            }
            else if( XmlNode->GetName() == wxT( "itunes:owner" ) )
            {
                ReadXmlOwner( XmlNode->GetChildren() );
            }
            else if( XmlNode->GetName() == wxT( "itunes:image" ) )
            {
                XmlNode->GetPropVal( wxT( "href" ), &m_Image );
            }
            else if( XmlNode->GetName() == wxT( "itunes:category" ) )
            {
                XmlNode->GetPropVal( wxT( "text" ), &m_Category );
            }
            else if( XmlNode->GetName() == wxT( "itunes:summary" ) )
            {
                m_Summary = XmlNode->GetNodeContent();
            }
            else if( XmlNode->GetName() == wxT( "item" ) )
            {
                guPodcastItem * PodcastItem = new guPodcastItem( XmlNode->GetChildren() );
                //PodcastItem->ReadXml( XmlNode->GetChildren() );
                //guLogMessage( wxT( "Item Length: %i" ), PodcastItem->m_Length );
                m_Items.Add( PodcastItem );
            }
            XmlNode = XmlNode->GetNext();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPodcastChannel::ReadXmlOwner( wxXmlNode * XmlNode )
{
    while( XmlNode )
    {
        if( XmlNode->GetName() == wxT( "itunes:name" ) )
        {
            m_OwnerName = XmlNode->GetNodeContent();
        }
        else if( XmlNode->GetName() == wxT( "itunes:email" ) )
        {
            m_OwnerEmail = XmlNode->GetNodeContent();
        }
        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void guPodcastChannel::CheckLogo( void )
{
    if( !m_Image.IsEmpty() )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();

        wxString PodcastsPath = Config->ReadStr( wxT( "Path" ),
                                    wxGetHomeDir() + wxT( ".guayadeque/Podcasts" ), wxT( "Podcasts" ) );

        guLogMessage( wxT( "Downloading the Image..." ) );
        wxFileName ImageFile = wxFileName( PodcastsPath + wxT( "/" ) +
                                           m_Title + wxT( "/" ) +
                                           m_Title + wxT( ".jpg" ) );
        if( ImageFile.Normalize( wxPATH_NORM_ALL|wxPATH_NORM_CASE ) )
        {
            if( !wxFileExists( ImageFile.GetFullPath() ) )
            {
                if( !DownloadImage( m_Image, ImageFile.GetFullPath(), 60, 60 ) )
                    guLogMessage( wxT( "Download image failed..." ) );
            }
            else
            {
                guLogMessage( wxT( "Image File already exists" ) );
            }
        }
        else
        {
            guLogMessage( wxT( "Error in normalize..." ) );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPodcastChannel::Update( DbLibrary * db, guMainFrame * mainframe )
{
    wxCurlHTTP  http;

    guLogMessage( wxT( "The address is %s" ), m_Url.c_str() );

    ReadContent();

    CheckLogo();

    // Save only the new items in the channel
    db->SavePodcastChannel( this, true );

    if( m_DownloadType != guPODCAST_DOWNLOAD_MANUALLY )
    {
        int Index;
        int Count = m_Items.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            guPodcastItem PodcastItem;
            db->GetPodcastItemEnclosure( m_Items[ Index ].m_Enclosure, &PodcastItem );
            if( m_DownloadType == guPODCAST_DOWNLOAD_ALL )
            {

            }
        }
    }
}

// -------------------------------------------------------------------------------- //
// guPodcastItem
// -------------------------------------------------------------------------------- //
guPodcastItem::guPodcastItem( wxXmlNode * XmlNode )
{
    m_Id = 0;
    m_ChId = 0;
    m_Time = 0;
    m_Length = 0;
    m_PlayCount = 0;
    m_LastPlay = 0;
    m_Status = 0;

    ReadXml( XmlNode );
}

// -------------------------------------------------------------------------------- //
void guPodcastItem::ReadXml( wxXmlNode * XmlNode )
{
    while( XmlNode )
    {
        if( XmlNode->GetName() == wxT( "title" ) )
        {
            m_Title = XmlNode->GetNodeContent();
            guLogMessage( wxT( "Item: '%s'" ), m_Title.c_str() );
        }
        else if( XmlNode->GetName() == wxT( "enclosure" ) )
        {
            XmlNode->GetPropVal( wxT( "url" ), &m_Enclosure );
            wxString LenStr;
            XmlNode->GetPropVal( wxT( "length" ), &LenStr );
            LenStr.ToULong( ( unsigned long * ) &m_FileSize );
        }
        else if( m_Summary.IsEmpty() && ( XmlNode->GetName() == wxT( "itunes:summary" ) ) ||
                 ( XmlNode->GetName() == wxT( "description" ) ) )
        {
            m_Summary= XmlNode->GetNodeContent();
        }
        else if( XmlNode->GetName() == wxT( "pubDate" ) )
        {
            wxDateTime DateTime;
            DateTime.ParseRfc822Date( XmlNode->GetNodeContent() );
            m_Time = DateTime.GetTicks();
        }
        else if( XmlNode->GetName() == wxT( "itunes:duration" ) )
        {
            m_Length = StrLengthToInt( XmlNode->GetNodeContent() );
        }
        else if( XmlNode->GetName() == wxT( "itunes:author" ) )
        {
            m_Author = XmlNode->GetNodeContent();
        }
        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
// guPodcastDownloadQueueThread
// -------------------------------------------------------------------------------- //
guPodcastDownloadQueueThread::guPodcastDownloadQueueThread( guMainFrame * mainframe )
{
    m_MainFrame = mainframe;
    m_CurPos = 0;
    guConfig * Config = ( guConfig * ) guConfig::Get();

    m_GaugeId = wxNOT_FOUND;

    // Check that the directory to store podcasts are created
    m_PodcastsPath = Config->ReadStr( wxT( "Path" ), wxGetHomeDir() + wxT( ".guayadeque/Podcasts" ), wxT( "Podcasts" ) );

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
    }
}

// -------------------------------------------------------------------------------- //
guPodcastDownloadQueueThread::~guPodcastDownloadQueueThread()
{
    m_MainFrame->ClearPodcastsDownloadThread();

//    if( m_GaugeId != wxNOT_FOUND )
//    {
//        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_GAUGE_REMOVE );
//        event.SetInt( m_GaugeId );
//        wxPostEvent( m_MainFrame, event );
//    }
}

// -------------------------------------------------------------------------------- //
void guPodcastDownloadQueueThread::SendUpdateEvent( guPodcastItem * podcastitem )
{
    guLogMessage( wxT( "Received the update event..." ) );
    wxCommandEvent event( guPodcastEvent, guPODCAST_EVENT_UPDATE_ITEM );
    event.SetClientData( new guPodcastItem( * podcastitem ) );
    wxPostEvent( m_MainFrame, event );
}

// -------------------------------------------------------------------------------- //
void guPodcastDownloadQueueThread::AddPodcastItems( guPodcastItemArray * items, bool priority )
{
    wxASSERT( items );

    int Index;
    int Count = items->Count();
    if( Count )
    {
        guLogMessage( wxT( "2) Adding the items to the download thread... %u" ), Count );
        m_ItemsMutex.Lock();
        for( Index = 0; Index < Count; Index++ )
        {
            if( priority )
                m_Items.Insert( new guPodcastItem( items->Item( Index ) ), m_CurPos + 1 );
            else
                m_Items.Add( new guPodcastItem( items->Item( Index ) ) );
        }

        m_ItemsMutex.Unlock();
        guLogMessage( wxT( "2) Added the items to the download thread..." ) );
    }

    if( !IsRunning() )
    {
        guLogMessage( wxT( "Launching download thread..." ) );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guPodcastDownloadQueueThread::ExitCode guPodcastDownloadQueueThread::Entry()
{
    int Count;
    while( !TestDestroy() )
    {
        m_ItemsMutex.Lock();
        Count = m_Items.Count();
        m_ItemsMutex.Unlock();

        if( m_CurPos < Count )
        {
//            if( m_GaugeId == wxNOT_FOUND )
//            {
//                m_GaugeId = ( ( guStatusBar * ) m_MainFrame->GetStatusBar() )->AddGauge();
//
//                wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_GAUGE_SETMAX );
//                event.SetInt( m_GaugeId );
//                event.SetExtraLong( Count );
//                wxPostEvent( m_MainFrame, event );
//            }

//            wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_GAUGE_UPDATE );
//            event.SetInt( m_GaugeId );
//            event.SetExtraLong( m_CurPos );
//            wxPostEvent( m_MainFrame, event );

            //
            guPodcastItem * PodcastItem = &m_Items[ m_CurPos ];
            if( PodcastItem->m_Enclosure.IsEmpty() )
            {
                PodcastItem->m_Status = guPODCAST_STATUS_ERROR;
                PodcastItem->m_FileName = wxEmptyString;
                SendUpdateEvent( PodcastItem );
            }
            else
            {
                wxURI Uri( PodcastItem->m_Enclosure );
                wxDateTime PodcastTime;
                PodcastTime.Set( ( time_t ) PodcastItem->m_Time );

                wxFileName PodcastFile = wxFileName( m_PodcastsPath + wxT( "/" ) +
                                            PodcastItem->m_Channel + wxT( "/" ) +
                                            PodcastTime.Format( wxT( "%Y%m%d%H%M%S-" ) ) +
                                            Uri.GetPath().AfterLast( wxT( '/' ) ) );
                if( PodcastFile.Normalize( wxPATH_NORM_ALL|wxPATH_NORM_CASE ) )
                {
                    PodcastItem->m_FileName = PodcastFile.GetFullPath();

                    if( !wxFileExists( PodcastFile.GetFullPath() ) ||
                        !( guGetFileSize( PodcastFile.GetFullPath() ) != PodcastItem->m_FileSize ) )
                    {
                        PodcastItem->m_Status = guPODCAST_STATUS_DOWNLOADING;
                        SendUpdateEvent( PodcastItem );

                        if( guIsValidAudioFile( PodcastItem->m_Enclosure ) &&
                            DownloadFile( PodcastItem->m_Enclosure, PodcastFile.GetFullPath() ) )
                        {
                            PodcastItem->m_Status = guPODCAST_STATUS_READY;
                            guLogMessage( wxT( "Finished downloading the file %s" ), PodcastFile.GetFullPath().c_str() );
                        }
                        else
                        {
                            PodcastItem->m_Status = guPODCAST_STATUS_ERROR;
                            guLogMessage( wxT( "Podcast download failed..." ) );
                        }
                        SendUpdateEvent( PodcastItem );
                    }
                    else if( PodcastItem->m_Status != guPODCAST_STATUS_READY )
                    {
                        PodcastItem->m_Status = guPODCAST_STATUS_READY;
                        guLogMessage( wxT( "Podcast File already exists" ) );
                        SendUpdateEvent( PodcastItem );
                    }
                }
                else
                {
                    guLogMessage( wxT( "Error in normalizing the podcast filename..." ) );
                }
            }

            //
            m_CurPos++;
        }
        else
        {
            m_ItemsMutex.Lock();
            if( ( m_CurPos == m_Items.Count() ) && m_CurPos )
            {
                m_CurPos = 0;
                m_Items.Clear();

//                wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_GAUGE_REMOVE );
//                event.SetInt( m_GaugeId );
//                wxPostEvent( m_MainFrame, event );
//                m_GaugeId = wxNOT_FOUND;
            }
            m_ItemsMutex.Unlock();
            Sleep( 800 );
        }
        Sleep( 200 );
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
