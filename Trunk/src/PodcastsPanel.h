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
#ifndef PODCASTSPANEL_H
#define PODCASTSPANEL_H

#include "DbLibrary.h"
#include "PlayerPanel.h"

#include <wx/wx.h>
#include <wx/hyperlink.h>
#include <wx/splitter.h>
#include <wx/statline.h>


#include "ItemListBox.h"

// -------------------------------------------------------------------------------- //
class guChannelsListBox : public guListBox
{
  protected :
    guPodcastChannelArray m_PodChannels;

    virtual void GetItemsList( void );
    virtual void CreateContextMenu( wxMenu * Menu ) const;

  public :
    guChannelsListBox( wxWindow * parent, DbLibrary * db, const wxString &label ) :
      guListBox( parent, db, label )
    {
        ReloadItems();
    };

    virtual int GetSelectedSongs( guTrackArray * Songs ) const;
};

#define guPODCASTS_COLUMN_STATUS        0
#define guPODCASTS_COLUMN_TITLE         1
#define guPODCASTS_COLUMN_CHANNEL       2
#define guPODCASTS_COLUMN_CATEGORY      3
#define guPODCASTS_COLUMN_DATE          4
#define guPODCASTS_COLUMN_LENGTH        5
#define guPODCASTS_COLUMN_AUTHOR        6
#define guPODCASTS_COLUMN_PLAYCOUNT     7
#define guPODCASTS_COLUMN_LASTPLAY      8

typedef enum {
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
class guPodcastListBox : public guListView
{
  protected :
    DbLibrary *         m_Db;
    guPodcastItemArray  m_PodItems;
    int                 m_Order;
    bool                m_OrderDesc;
    wxImage *           m_Images[ guPODCAST_STATUS_ERROR + 1 ];

    virtual void                DrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const;
    virtual void                CreateContextMenu( wxMenu * Menu ) const;
    virtual wxString            OnGetItemText( const int row, const int column ) const;
    virtual void                GetItemsList( void );

  public :
    guPodcastListBox( wxWindow * parent, DbLibrary * NewDb );
    ~guPodcastListBox();

    virtual void                ReloadItems( bool reset = true );

    virtual wxString inline GetItemName( const int item ) const;
    virtual int inline      GetItemId( const int item ) const;

    void                        SetOrder( int order );
};

class guPodcastPanel;

// -------------------------------------------------------------------------------- //
class guPodcastDownloadThread : public wxThread
{
  protected :
    guPodcastPanel *    m_PodcastPanel;
    guPodcastItemArray  m_Items;
    wxMutex             m_ItemsMutex;

  public :
    guPodcastDownloadThread( guPodcastPanel * podcastpanel );
    ~guPodcastDownloadThread();

    ExitCode Entry();

};

// -------------------------------------------------------------------------------- //
class guPodcastPanel : public wxPanel
{
  private:
    void                MainSplitterOnIdle( wxIdleEvent& );
    void                OnChannelsSelected( wxListEvent &event );
    void                UpdatePodcastInfo( int itemid );
    void                UpdateChannelInfo( int itemid );
    void                OnPodcastsColClick( wxListEvent &event );
    void                OnPodcastItemSelected( wxListEvent &event );

  protected:
    DbLibrary *                 m_Db;
    guPlayerPanel *             m_PlayerPanel;
    wxString                    m_PodcastsPath;
    int                         m_LastChannelInfoId;
    int                         m_LastPodcastInfoId;
    guPodcastDownloadThread *   m_DownloadThread;

    wxSplitterWindow *          m_MainSplitter;
    wxSplitterWindow *          m_TopSplitter;
	guChannelsListBox *         m_ChannelsListBox;
    guPodcastListBox *          m_PodcastsListBox;
	wxBoxSizer *                m_DetailMainSizer;
    wxStaticBitmap *            m_DetailImage;
    wxStaticText *              m_DetailChannelTitle;
    wxStaticText *              m_DetailDescText;
    wxStaticText *              m_DetailAuthorText;
    wxStaticText *              m_DetailOwnerText;
    wxHyperlinkCtrl *           m_DetailLinkText;
    wxStaticText *              m_DetailEmailText;
    wxStaticText *              m_DetailItemTitleText;
    wxStaticText *              m_DetailItemSumaryText;
    wxStaticText *              m_DetailItemDateText;
    wxStaticText *              m_DetailItemLengthText;
    wxScrolledWindow *          m_DetailScrolledWindow;
	wxFlexGridSizer *           m_DetailFlexGridSizer;

    void AddChannel( wxCommandEvent &event );
    void DeleteChannels( wxCommandEvent &event );
    void ChannelProperties( wxCommandEvent &event );
    void ChannelsCopyTo( wxCommandEvent &event );

    void ClearDownloadThread( void );

public:
    guPodcastPanel( wxWindow * parent, DbLibrary * db, guPlayerPanel * playerpanel );
    ~guPodcastPanel();

    friend class guPodcastDownloadThread;
};

#endif
// -------------------------------------------------------------------------------- //
