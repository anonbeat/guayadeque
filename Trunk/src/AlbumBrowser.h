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
#ifndef ALBUMBROWSER_H
#define ALBUMBROWSER_H

#include "DbLibrary.h"
#include "DynamicPlayList.h"
#include "PlayerPanel.h"

#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/statbmp.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/panel.h>

// -------------------------------------------------------------------------------- //
class guAlbumBrowserItem
{
  public :
    unsigned int  m_AlbumId;
    wxString      m_ArtistName;
    wxString      m_AlbumName;
    unsigned int  m_Year;
    unsigned int  m_TrackCount;
    unsigned int  m_CoverId;
    wxBitmap *    m_CoverBitmap;

    guAlbumBrowserItem()
    {
        m_AlbumId = wxNOT_FOUND;
        m_Year = 0;
        m_TrackCount = 0;
        m_CoverBitmap = NULL;
    };

    ~guAlbumBrowserItem()
    {
        if( m_CoverBitmap )
            delete m_CoverBitmap;
    };
};
WX_DECLARE_OBJARRAY( guAlbumBrowserItem, guAlbumBrowserItemArray );

class guAlbumBrowser;

// -------------------------------------------------------------------------------- //
class guAlbumBrowserItemPanel : public wxPanel
{
  protected :
    int                     m_Index;
    guAlbumBrowserItem *    m_AlbumBrowserItem;
    guAlbumBrowser *        m_AlbumBrowser;

    wxPoint                 m_DragStart;
    int                     m_DragCount;

    // GUI
    wxBoxSizer *            m_MainSizer;
    wxStaticBitmap *        m_Bitmap;
    wxStaticText *          m_ArtistLabel;
    wxStaticText *          m_AlbumLabel;
    wxStaticText *          m_TracksLabel;

    void                    OnContextMenu( wxContextMenuEvent &event );

    void                    OnSearchLinkClicked( wxCommandEvent &event );
    void                    OnPlayClicked( wxCommandEvent &event );
    void                    OnEnqueueClicked( wxCommandEvent &event );
    void                    OnCopyToClipboard( wxCommandEvent &event );
    void                    OnAlbumSelectName( wxCommandEvent &event );
    void                    OnArtistSelectName( wxCommandEvent &event );
    void                    OnCommandClicked( wxCommandEvent &event );
    void                    OnAlbumDownloadCoverClicked( wxCommandEvent &event );
    void                    OnAlbumSelectCoverClicked( wxCommandEvent &event );
    void                    OnAlbumDeleteCoverClicked( wxCommandEvent &event );
    void                    OnAlbumCopyToClicked( wxCommandEvent &event );
    void                    OnAlbumEditLabelsClicked( wxCommandEvent &event );
    void                    OnAlbumEditTracksClicked( wxCommandEvent &event );
    void                    OnAlbumDClicked( wxMouseEvent &event );

    void                    OnMouse( wxMouseEvent &event );
    void                    OnBeginDrag( wxMouseEvent &event );
    void                    OnCoverBeginDrag( wxMouseEvent &event );

  public :
    guAlbumBrowserItemPanel( wxWindow * parent, const int index, guAlbumBrowserItem * albumitem = NULL );
    ~guAlbumBrowserItemPanel();

    void SetAlbumItem( const int index, guAlbumBrowserItem * albumitem, wxBitmap * blankcd );
    void UpdateDetails( void );
    void SetAlbumCover( const wxString &cover );

};
WX_DEFINE_ARRAY_PTR( guAlbumBrowserItemPanel *, guAlbumBrowserItemPanelArray );

class guUpdateAlbumDetails;

// -------------------------------------------------------------------------------- //
class guAlbumBrowser : public wxPanel
{
  protected :
    guDbLibrary *                   m_Db;
    guPlayerPanel *                 m_PlayerPanel;
    guAlbumBrowserItemArray         m_AlbumItems;
    guAlbumBrowserItemPanelArray    m_ItemPanels;
    guUpdateAlbumDetails *          m_UpdateDetails;
    wxMutex                         m_UpdateDetailsMutex;
    unsigned int                    m_ItemStart;        // The first element in a page
    unsigned int                    m_LastItemStart;
    unsigned int                    m_ItemCount;        // The number of elements in a page
    unsigned int                    m_AlbumsCount;      // The number of albums in database
    unsigned int                    m_PagesCount;       // The number of pages to scroll
    wxBitmap *                      m_BlankCD;
    wxTimer                         m_RefreshTimer;
    guDynPlayList                   m_DynPlayList;


    // GUI
    wxToggleButton *                m_FilterBtn;
    wxBitmapButton *                m_EditFilterBtn;
    wxChoice *                      m_OrderChoice;
	wxGridSizer *                   m_AlbumsSizer;
	wxStaticText *                  m_NavLabel;
	wxSlider *                      m_NavSlider;
	wxSize                          m_LastSize;

    void                            OnChangedSize( wxSizeEvent &event );
    void                            OnChangingPosition( wxScrollEvent& event );
    void                            OnRefreshTimer( wxTimerEvent &event );

    void                            OnFilterSelected( wxCommandEvent &event );
    void                            OnEditFilterClicked( wxCommandEvent &event );
    void                            OnOrderSelected( wxCommandEvent &event );
    void                            OnUpdateDetails( wxCommandEvent &event );

    void                            OnMouseWheel( wxMouseEvent& event );
//    void                            OnBeginDrag( wxCommandEvent &event );

  public :
    guAlbumBrowser( wxWindow * parent, guDbLibrary * db, guPlayerPanel * playerpanel );
    ~guAlbumBrowser();

    void RefreshCount( void )
    {
        m_AlbumsCount = m_Db->GetAlbumsCount( m_FilterBtn->GetValue() ? &m_DynPlayList : NULL );
        m_ItemStart = 0;
        RefreshPageCount();
    }

    void ClearUpdateDetailsThread( void )
    {
        m_UpdateDetailsMutex.Lock();
        m_UpdateDetails = NULL;
        m_UpdateDetailsMutex.Unlock();
    }

    void RefreshPageCount( void )
    {
        if( m_ItemCount && m_AlbumsCount )
        {
            m_PagesCount = m_AlbumsCount / m_ItemCount;
            if( ( m_PagesCount * m_ItemCount ) < m_AlbumsCount )
                m_PagesCount++;
        }
        else
        {
            m_PagesCount = 0;
        }

        //guLogMessage( wxT( "RefreshPageCount: Albums: %i   Items: %i  Pages: %i"  ), m_AlbumsCount, m_ItemCount, m_PagesCount );
        UpdateNavLabel( 0 );

        m_NavSlider->Enable( m_PagesCount > 1 );
        if( m_PagesCount > 1 )
            m_NavSlider->SetRange( 0, m_PagesCount - 1 );
        //else
        //    m_NavSlider->SetRange( 0, 0 );
    }

    void                            ReloadItems( void );
    void                            RefreshAll( void );

    void UpdateNavLabel( const int page )
    {
        m_NavLabel->SetLabel( _( "Page" ) + wxString::Format( wxT( " %i / %i" ), page + 1, m_PagesCount ) );
    }

    void                            SelectAlbum( const int albumid, const bool append );
    int                             GetAlbumTracks( const int albumid, guTrackArray * tracks );
    void                            OnCommandClicked( const int commandid, const int albumid );
    void                            OnAlbumDownloadCoverClicked( const int albumid );
    void                            OnAlbumSelectCoverClicked( const int albumid );
    void                            OnAlbumDeleteCoverClicked( const int albumid );
    void                            OnAlbumCopyToClicked( const int albumid );
    void                            OnAlbumEditLabelsClicked( const int albumid );
    void                            OnAlbumEditTracksClicked( const int albumid );

    void                            LibraryUpdated( void );

    wxString                        GetAlbumCoverFile( const int albumid );

    void                            SetAlbumCover( const int albumid, const wxString &cover );


    friend class guUpdateAlbumDetails;
};

// -------------------------------------------------------------------------------- //
class guAlbumBrowserDropTarget : public wxFileDropTarget
{
  private:
    guAlbumBrowserItemPanel *       m_AlbumBrowserItemPanel;

  public:
    guAlbumBrowserDropTarget( guAlbumBrowserItemPanel * itempanel );
    ~guAlbumBrowserDropTarget() {};

    virtual bool OnDropFiles( wxCoord x, wxCoord y, const wxArrayString &files );

//    virtual wxDragResult OnDragOver( wxCoord x, wxCoord y, wxDragResult def );
};

#endif
// -------------------------------------------------------------------------------- //
