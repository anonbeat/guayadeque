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
bool guPodcastChannel::ReadContent( void )
{
    bool RetVal = false;
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
            RetVal = ReadXml( XmlNode->GetChildren() );
        }
        free( Buffer );
    }
    else
    {
        guLogError( wxT( "Could not get podcast content for %s" ), m_Url.c_str() );
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
bool guPodcastChannel::ReadXml( wxXmlNode * XmlNode )
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
        return true;
    }
    return false;
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
int guPodcastChannel::GetUpdateItems( DbLibrary * db, guPodcastItemArray * items )
{
  wxASSERT( items );

  wxString query;
  wxSQLite3ResultSet dbRes;

  // If the download type is set to manual there is nothign to download
  if( m_DownloadType == guPODCAST_DOWNLOAD_MANUALLY )
    return 0;

  query = wxString::Format( wxT( "SELECT podcastitem_id, podcastitem_chid, podcastitem_title, "
            "podcastitem_summary, podcastitem_author, podcastitem_enclosure, podcastitem_time, "
            "podcastitem_file, podcastitem_filesize, podcastitem_length, "
            "podcastitem_playcount, podcastitem_addeddate, podcastitem_lastplay, "
            "podcastitem_status, "
            "podcastch_title, podcastch_category "
            "FROM podcastitems, podcastchs "
            "WHERE podcastitem_chid = podcastch_id "
            "AND podcastitem_chid = %u " ), m_Id );

  query += wxT( "AND podcastitem_status IN ( 0, 5 ) " );

  if( m_DownloadType == guPODCAST_DOWNLOAD_FILTER )
  {
    wxArrayString Words;
    if( !m_DownloadText.IsEmpty() )
    {
        Words = guSplitWords( m_DownloadText );
    }
    int Index;
    int Count;
    if( ( Count = Words.Count() ) )
    {
        query += wxT( "AND ( " );
        for( Index = 0; Index < Count; Index++ )
        {
            query += wxString::Format( wxT( "podcastitem_title LIKE '%%%s%%' OR "
                                            "podcastitem_summary LIKE '%%%s%%'" ),
                                        Words[ Index ].c_str(),
                                        Words[ Index ].c_str() );
        }
        query += wxT( " );" );
    }
  }

  guLogMessage( wxT( "GetUpdateItems : %s" ), query.c_str() );

  dbRes = db->ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    guPodcastItem * Item = new guPodcastItem();
    Item->m_Id = dbRes.GetInt( 0 );
    Item->m_ChId = dbRes.GetInt( 1 );
    Item->m_Title = dbRes.GetString( 2 );
    Item->m_Summary = dbRes.GetString( 3 );
    Item->m_Author = dbRes.GetString( 4 );
    Item->m_Enclosure = dbRes.GetString( 5 );
    Item->m_Time = dbRes.GetInt( 6 );
    Item->m_FileName = dbRes.GetString( 7 );
    Item->m_FileSize = dbRes.GetInt( 8 );
    Item->m_Length = dbRes.GetInt( 9 );
    Item->m_PlayCount = dbRes.GetInt( 10 );
    Item->m_AddedDate = dbRes.GetInt( 11 );
    Item->m_LastPlay = dbRes.GetInt( 12 );
    Item->m_Status = dbRes.GetInt( 13 );

    Item->m_Channel = dbRes.GetString( 14 );
    Item->m_Category = dbRes.GetString( 15 );
    items->Add( Item );
  }
  dbRes.Finalize();
  return items->Count();
}

// -------------------------------------------------------------------------------- //
int guPodcastChannel::CheckDownloadItems( DbLibrary * db, guMainFrame * mainframe )
{
    if( m_DownloadType != guPODCAST_DOWNLOAD_MANUALLY )
    {
        guPodcastItemArray UpdatePodcasts;

        // If there was items to be downloaded
        int Count;
        if( ( Count = GetUpdateItems( db, &UpdatePodcasts ) ) )
        {
            mainframe->AddPodcastsDownloadItems( &UpdatePodcasts );
            return Count;
        }
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
void guPodcastChannel::Update( DbLibrary * db, guMainFrame * mainframe )
{
    guLogMessage( wxT( "The address is %s" ), m_Url.c_str() );

    CheckDir();

    if( ReadContent() )
    {
        CheckLogo();

        // Save only the new items in the channel
        db->SavePodcastChannel( this, true );

        CheckDownloadItems( db, mainframe );
    }
}

// -------------------------------------------------------------------------------- //
void guPodcastChannel::CheckDir( void )
{
    // Save the Splitter positions into the main config
    guConfig * Config = ( guConfig * ) guConfig::Get();

    // Check that the directory to store podcasts are created
    wxString PodcastsPath = Config->ReadStr( wxT( "Path" ), wxGetHomeDir() + wxT( ".guayadeque/Podcasts" ), wxT( "Podcasts" ) );

    // Create the channel dir
    wxFileName ChannelDir = wxFileName( PodcastsPath + wxT( "/" ) + m_Title );
    if( ChannelDir.Normalize( wxPATH_NORM_ALL | wxPATH_NORM_CASE ) )
    {
        if( !wxDirExists( ChannelDir.GetFullPath() ) )
        {
            wxMkdir( ChannelDir.GetFullPath(), 0770 );
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

}

// -------------------------------------------------------------------------------- //
void guPodcastDownloadQueueThread::SendUpdateEvent( guPodcastItem * podcastitem )
{
    guLogMessage( wxT( "Sending the update event..." ) );
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
        Lock();
        if( !TestDestroy() )
        {
            guLogMessage( wxT( "2) Adding the items to the download thread... %u" ), Count );
            for( Index = 0; Index < Count; Index++ )
            {
                if( TestDestroy() )
                    break;
                if( priority )
                    m_Items.Insert( new guPodcastItem( items->Item( Index ) ), m_CurPos );
                else
                    m_Items.Add( new guPodcastItem( items->Item( Index ) ) );
            }
            guLogMessage( wxT( "2) Added the items to the download thread..." ) );
        }
        Unlock();
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
//    int IdleCount = 0;
    while( !TestDestroy() )
    {
        Lock();
        Count = m_Items.Count();
        Unlock();

        guLogMessage( wxT( "DownloadThread %u of %u" ), m_CurPos, Count );
        if( m_CurPos < Count )
        {
//            IdleCount = 0;
            //
            guPodcastItem * PodcastItem = &m_Items[ m_CurPos ];

            guLogMessage( wxT( "Ok so we have one item to download... %u %s" ),
                   m_CurPos, PodcastItem->m_Enclosure.c_str() );
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

                    wxFileName::Mkdir( m_PodcastsPath + wxT( "/" ) +
                                       PodcastItem->m_Channel, 0770, wxPATH_MKDIR_FULL );

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
            Lock();
            if( ( m_CurPos == m_Items.Count() ) )
            {
                if( m_CurPos )
                {
                    m_CurPos = 0;
                    m_Items.Clear();
                }
//                else
//                {
//                    if( ++IdleCount > 10 )
//                    {
//                        Unlock();
//                        return 0;
//                    }
//                }
            }
            Unlock();
        }
        Sleep( 200 );
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
