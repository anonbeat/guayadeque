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
    guPodcastChannelArray m_Channels;

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

// -------------------------------------------------------------------------------- //
class guPodcastListBox : public guListBox
{
  protected :
    virtual void GetItemsList( void );
    virtual void CreateContextMenu( wxMenu * menu ) const;

  public :
    guPodcastListBox( wxWindow * parent, DbLibrary * db, const wxString &label ) :
        guListBox( parent, db, label )
    {
        ReloadItems();
    };

    virtual int GetSelectedSongs( guTrackArray * Songs ) const;
};

// -------------------------------------------------------------------------------- //
class guPodcastPanel : public wxPanel
{
  private:
    void                MainSplitterOnIdle( wxIdleEvent& );
    void                OnChangedSize( wxSizeEvent& event );

  protected:
    DbLibrary *         m_Db;
    guPlayerPanel *     m_PlayerPanel;
    wxSplitterWindow *  m_MainSplitter;
	guChannelsListBox * m_ChannelsListBox;
    guPodcastListBox *  m_Podcasts;
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
