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
#ifndef __PODCASTSPANEL_H__
#define __PODCASTSPANEL_H__

#include "AuiManagerPanel.h"
#include "AuiNotebook.h"
#include "DbLibrary.h"
#include "PlayerPanel.h"

#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/hyperlink.h>
#include <wx/splitter.h>
#include <wx/statline.h>

#include "ItemListBox.h"

namespace Guayadeque {

#define     guPANEL_PODCASTS_CHANNELS           ( 1 << 0 )
#define     guPANEL_PODCASTS_DETAILS            ( 1 << 1 )

#define     guPANEL_PODCASTS_VISIBLE_DEFAULT    ( guPANEL_PODCASTS_CHANNELS | guPANEL_PODCASTS_DETAILS )

#define guPODCASTS_COLUMN_STATUS        0
#define guPODCASTS_COLUMN_TITLE         1
#define guPODCASTS_COLUMN_CHANNEL       2
#define guPODCASTS_COLUMN_CATEGORY      3
#define guPODCASTS_COLUMN_DATE          4
#define guPODCASTS_COLUMN_LENGTH        5
#define guPODCASTS_COLUMN_AUTHOR        6
#define guPODCASTS_COLUMN_PLAYCOUNT     7
#define guPODCASTS_COLUMN_LASTPLAY      8
#define guPODCASTS_COLUMN_ADDEDDATE     9

#define guPODCASTS_COLUMN_COUNT         10

// -------------------------------------------------------------------------------- //
enum guPodcastSorting {
    guPODCAST_CHANNEL_SORT_NONE,
    guPODCAST_CHANNEL_SORT_TITLE,
    guPODCAST_CHANNEL_SORT_TIME,
    guPODCAST_CHANNEL_SORT_AUTHOR,
    guPODCAST_CHANNEL_SORT_CATEGORY
};

// -------------------------------------------------------------------------------- //
class guDbPodcasts : public guDb
{
  protected :

  public :
    guDbPodcasts( const wxString &dbname );
    ~guDbPodcasts();

    int                     GetPodcastChannels( guPodcastChannelArray * channels );
    void                    SavePodcastChannel( guPodcastChannel * channel, bool onlynew = false );
    int                     SavePodcastChannels( guPodcastChannelArray * channels, bool onlynew = false );
    int                     GetPodcastChannelUrl( const wxString &url, guPodcastChannel * channel = NULL );
    int                     GetPodcastChannelId( const int id, guPodcastChannel * channel = NULL );
    void                    DelPodcastChannel( const int id );

    int                     GetPodcastItems( guPodcastItemArray * items, const wxArrayInt &filters, const int order, const bool desc );
    void                    GetPodcastCounters( const wxArrayInt &filters, wxLongLong * count, wxLongLong * len, wxLongLong * size );
    int                     GetPodcastItems( const wxArrayInt &ids, guPodcastItemArray * items, const int order, const bool desc );
    void                    SavePodcastItem( const int channelid, guPodcastItem * item, bool onlynew = false );
    void                    SavePodcastItems( const int channelid, guPodcastItemArray * items, bool onlynew = false );
    void                    SetPodcastItemStatus( const int itemid, const int status );
    void                    SetPodcastItemPlayCount( const int itemid, const int playcount );
    void                    UpdatePodcastItemLength( const int itemid, const int length );
    int                     GetPodcastItemEnclosure( const wxString &enclosure, guPodcastItem * item = NULL );
    int                     GetPodcastItemId( const int itemid, guPodcastItem * item = NULL );
    int                     GetPodcastItemFile( const wxString &filename, guPodcastItem * item = NULL );
    void                    DelPodcastItem( const int itemid );
    void                    DelPodcastItems( const int channelid );
    //void                    SetPodcastChannelFilters( const wxArrayInt &filters );
    //void                    SetPodcastOrder( int order );
    int                     GetPendingPodcasts( guPodcastItemArray * items );
    int                     GetPodcastFiles( const wxArrayInt &channelid, guDataObjectComposite * files );

    void                    UpdateItemPaths( const wxString &oldpath, const wxString &newpath );

};

// -------------------------------------------------------------------------------- //
class guChannelsListBox : public guListBox
{
  protected :
    guPodcastChannelArray m_PodChannels;

    virtual void    GetItemsList( void );
    virtual void    CreateContextMenu( wxMenu * Menu ) const;

    virtual int     GetDragFiles( guDataObjectComposite * files );
    virtual void    OnKeyDown( wxKeyEvent &event );

  public :
    guChannelsListBox( wxWindow * parent, guDbPodcasts * db, const wxString &label ) :
      guListBox( parent, ( guDbLibrary * ) db, label, wxLB_MULTIPLE | guLISTVIEW_HIDE_HEADER )
    {
        ReloadItems();
    };

    virtual int     GetSelectedSongs( guTrackArray * Songs, const bool isdrag = false ) const;
    int             FindItem( const int channelid );
};

class guPodcastPanel;

// -------------------------------------------------------------------------------- //
class guPodcastListBox : public guListView
{
  protected :
    guDbPodcasts *              m_Db;
    guPodcastItemArray          m_PodItems;
    wxArrayInt                  m_PodChFilters;
    int                         m_Order;
    bool                        m_OrderDesc;
    wxImage *                   m_Images[ guPODCAST_STATUS_ERROR + 1 ];
    wxArrayString               m_ColumnNames;

    virtual void                DrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const;
    virtual void                CreateContextMenu( wxMenu * Menu ) const;
    virtual wxString            OnGetItemText( const int row, const int column ) const;
    virtual void                GetItemsList( void );
    virtual int                 GetSelectedSongs( guTrackArray * tracks, const bool isdrag = false ) const;

    virtual void                OnKeyDown( wxKeyEvent &event );

    void                        OnConfigUpdated( wxCommandEvent &event );
    void                        CreateAcceleratorTable();

  public :
    guPodcastListBox( wxWindow * parent, guDbPodcasts * NewDb );
    ~guPodcastListBox();

    virtual void                ReloadItems( bool reset = true );

    virtual wxString inline     GetItemName( const int item ) const;
    virtual int inline          GetItemId( const int item ) const;

    void                        SetOrder( int order );
    int                         GetOrder( void ) { return m_Order; }
    bool                        GetOrderDesc( void ) { return m_OrderDesc; }
    void                        SetFilters( const wxArrayInt &filters );
    wxArrayInt                  GetFilters( void ) { return m_PodChFilters; }

    int                         FindItem( const int podcastid );

    friend class guPodcastPanel;
};

class guMainFrame;

// -------------------------------------------------------------------------------- //
class guPodcastPanel : public guAuiManagerPanel
{
  private:
    void                MainSplitterOnIdle( wxIdleEvent& );
    void                OnChannelsSelected( wxCommandEvent &event );
    void                OnChannelsActivated( wxCommandEvent &event );
    void                UpdatePodcastInfo( int itemid );
    void                UpdateChannelInfo( int itemid );
    void                OnPodcastsColClick( wxListEvent &event );
    void                OnPodcastItemSelected( wxCommandEvent &event );
    void                OnPodcastItemUpdated( wxCommandEvent &event );
    void                OnPodcastItemActivated( wxCommandEvent &event );

    void                OnPodcastItemPlay( wxCommandEvent &event );
    void                OnPodcastItemEnqueue( wxCommandEvent &event );
    void                OnPodcastItemDelete( wxCommandEvent &event );
    void                OnPodcastItemDownload( wxCommandEvent &event );
    void                OnPodcastItemCopyTo( wxCommandEvent &event );


  protected:
    guDbPodcasts *              m_Db;
    guMainFrame *               m_MainFrame;
    guPlayerPanel *             m_PlayerPanel;
    wxString                    m_PodcastsPath;
    int                         m_LastChannelInfoId;
    int                         m_LastPodcastInfoId;

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

    void ProcessChannel( guPodcastChannel * channel );

    void AddChannel( wxCommandEvent &event );
    void DeleteChannels( wxCommandEvent &event );
    void ChannelProperties( wxCommandEvent &event );
    void ChannelsCopyTo( wxCommandEvent &event );
    void UpdateChannels( wxCommandEvent &event );

    void ClearDownloadThread( void );
    void OnSelectPodcasts( bool enqueue = false, const int aftercurrent = 0 );

    void OnConfigUpdated( wxCommandEvent &event );

public:
    guPodcastPanel( wxWindow * parent, guDbPodcasts * db, guMainFrame * mainframe, guPlayerPanel * playerpanel );
    ~guPodcastPanel();

    virtual void                InitPanelData( void );
    void                        SetSelection( const int type, const int id );

    void GetCounters( wxLongLong * count, wxLongLong * len, wxLongLong * size )
    {
        ( ( guDbPodcasts * ) m_Db )->GetPodcastCounters( m_PodcastsListBox->GetFilters(), count, len, size );
    }

    virtual int                 GetListViewColumnCount( void ) { return guPODCASTS_COLUMN_COUNT; }
    virtual bool                GetListViewColumnData( const int id, int * index, int * width, bool * enabled ) { return m_PodcastsListBox->GetColumnData( id, index, width, enabled ); }
    virtual bool                SetListViewColumnData( const int id, const int index, const int width, const bool enabled, const bool refresh = false ) { return m_PodcastsListBox->SetColumnData( id, index, width, enabled, refresh ); }


    friend class guPodcastDownloadQueueThread;
};

}

#endif
// -------------------------------------------------------------------------------- //
