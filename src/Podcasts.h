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
#ifndef PODCASTS_H
#define PODCASTS_H

#include "curl/http.h"

#include <wx/dynarray.h>
#include <wx/xml/xml.h>

// -------------------------------------------------------------------------------- //
typedef enum {
    guPODCAST_STATUS_NORMAL,
    guPODCAST_STATUS_PENDING,
    guPODCAST_STATUS_DOWNLOADING,
    guPODCAST_STATUS_READY,
    guPODCAST_STATUS_DELETED,
    guPODCAST_STATUS_ERROR
} guPodcastStatus;

typedef enum {
    guPODCAST_DOWNLOAD_MANUALLY,
    guPODCAST_DOWNLOAD_FILTER,
    guPODCAST_DOWNLOAD_ALL
} guPodcastDownload;

// -------------------------------------------------------------------------------- //
class guPodcastItem
{
  protected :
    void ReadXml( wxXmlNode * XmlNode );

  public :
    int         m_Id;
    wxString    m_Title;
    wxString    m_Author;
    int         m_ChId;
    wxString    m_Channel;
    //wxString    m_Link;
    wxString    m_Summary;
    wxString    m_Enclosure;
    int         m_Time;
    int         m_Length;
    wxString    m_FileName;
    int         m_FileSize;
    wxString    m_Category;

    int         m_PlayCount;
    int         m_LastPlay;
    int         m_Status;

    guPodcastItem()
    {
        m_Id = 0;
        m_ChId = 0;
        m_Time = 0;
        m_Length = 0;
        m_PlayCount = 0;
        m_LastPlay = 0;
        m_Status = 0;
    }

    guPodcastItem( wxXmlNode * XmlNode );

};
WX_DECLARE_OBJARRAY(guPodcastItem, guPodcastItemArray);

class DbLibrary;
class guMainFrame;

// -------------------------------------------------------------------------------- //
class guPodcastChannel
{
  protected :
    void        ReadContent( void );
    void        ReadXml( wxXmlNode * XmlNode );
    void        ReadXmlOwner( wxXmlNode * XmlNode );

  public :
    int                 m_Id;
    wxString            m_Url;
    wxString            m_Title;
    wxString            m_Link;
    wxString            m_Description;
    wxString            m_Lang;
    wxString            m_Summary;
    wxString            m_Category;
    wxString            m_Image;
    wxString            m_Author;
    wxString            m_OwnerName;
    wxString            m_OwnerEmail;
    guPodcastItemArray  m_Items;
    int                 m_DownloadType;
    wxString            m_DownloadText;
    bool                m_AllowDelete;

    guPodcastChannel() {};
    guPodcastChannel( const wxString &url );
    void Update( DbLibrary * db, guMainFrame * mainframe );

};
WX_DECLARE_OBJARRAY(guPodcastChannel, guPodcastChannelArray);


extern const wxEventType guPodcastEvent;
#define guPODCAST_EVENT_UPDATE_ITEM         1000

// -------------------------------------------------------------------------------- //
class guPodcastDownloadQueueThread : public wxThread
{
  protected :
    guMainFrame *       m_MainFrame;
    wxString            m_PodcastsPath;
    guPodcastItemArray  m_Items;
    wxMutex             m_ItemsMutex;
    int                 m_CurPos;
    int                 m_GaugeId;

    void SendUpdateEvent( guPodcastItem * podcastitem );

  public :
    guPodcastDownloadQueueThread( guMainFrame * mainframe );
    ~guPodcastDownloadQueueThread();

    ExitCode Entry();
    void AddPodcastItems( guPodcastItemArray * items, bool priority = false );

};

#endif
// -------------------------------------------------------------------------------- //
