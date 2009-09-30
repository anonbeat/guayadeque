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

#define guPODCASTS_COLUMN_TITLE         0
#define guPODCASTS_COLUMN_CHANNEL       1
#define guPODCASTS_COLUMN_CATEGORY      2
#define guPODCASTS_COLUMN_DATE          3
#define guPODCASTS_COLUMN_LENGTH        4
#define guPODCASTS_COLUMN_AUTHOR        5
#define guPODCASTS_COLUMN_PLAYCOUNT     6
#define guPODCASTS_COLUMN_LASTPLAY      7
#define guPODCASTS_COLUMN_STATUS        8

// -------------------------------------------------------------------------------- //
class guPodcastListBox : public guListView
{
  protected :
    DbLibrary *         m_Db;
    guPodcastItemArray  m_PodItems;
    int                 m_Order;
    bool                m_OrderDesc;

    virtual void                CreateContextMenu( wxMenu * Menu ) const;
    virtual wxString            OnGetItemText( const int row, const int column ) const;
    virtual void                GetItemsList( void );

  public :
    guPodcastListBox( wxWindow * parent, DbLibrary * NewDb );
    ~guPodcastListBox();

    virtual void                ReloadItems( bool reset = true );

    virtual int inline          GetItemId( const int row ) const;
    virtual wxString inline     GetItemName( const int row ) const;

    void                        SetOrder( int order );
};

// -------------------------------------------------------------------------------- //
class guPodcastPanel : public wxPanel
{
  private:
    void                MainSplitterOnIdle( wxIdleEvent& );
    void                OnChangedSize( wxSizeEvent& event );
    void                OnChannelsSelected( wxListEvent &event );
    void                UpdateChannelInfo( int index );
    void                OnPodcastsColClick( wxListEvent &event );
    void                OnPodcastItemSelected( wxListEvent &event );

  protected:
    DbLibrary *         m_Db;
    guPlayerPanel *     m_PlayerPanel;
    wxSplitterWindow *  m_MainSplitter;
	guChannelsListBox * m_ChannelsListBox;
    guPodcastListBox *  m_PodcastsListBox;
    wxStaticBitmap *    m_DetailImage;
    wxStaticText *      m_DetailChannelTitle;
    wxStaticText *      m_DetailDescText;
    wxStaticText *      m_DetailAuthorText;
    wxStaticText *      m_DetailOwnerText;
    wxStaticText *      m_DetailEmailText;
    wxStaticText *      m_DetailItemTitleText;
    wxStaticText *      m_DetailItemSumaryText;
    wxStaticText *      m_DetailItemDateText;
    wxStaticText *      m_DetailItemLengthText;

    void AddChannel( wxCommandEvent &event );

public:
    guPodcastPanel( wxWindow * parent, DbLibrary * db, guPlayerPanel * playerpanel );
    ~guPodcastPanel();

};

#endif
// -------------------------------------------------------------------------------- //
