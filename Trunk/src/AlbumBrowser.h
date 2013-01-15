// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2012 J.Rios
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
#ifndef ALBUMBROWSER_H
#define ALBUMBROWSER_H

#include "AutoScrollText.h"
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
#include <wx/srchctrl.h>
#include <wx/panel.h>

// -------------------------------------------------------------------------------- //
class guAlbumBrowserItem
{
  public :
    unsigned int  m_AlbumId;
    wxString      m_ArtistName;
    unsigned int  m_ArtistId;
    wxString      m_AlbumName;
    unsigned int  m_Year;
    unsigned int  m_TrackCount;
    unsigned int  m_CoverId;
    wxBitmap *    m_CoverBitmap;

    guAlbumBrowserItem()
    {
        m_AlbumId = wxNOT_FOUND;
        m_Year = 0;
        m_ArtistId = wxNOT_FOUND;
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
class guMediaViewer;

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
    guAutoScrollText *      m_ArtistLabel;
    guAutoScrollText *      m_AlbumLabel;
    guAutoScrollText *      m_TracksLabel;
    wxTimer                 m_BitmapTimer;

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
    void                    OnAlbumEmbedCoverClicked( wxCommandEvent &event );
    void                    OnAlbumCopyToClicked( wxCommandEvent &event );
    void                    OnAlbumEditLabelsClicked( wxCommandEvent &event );
    void                    OnAlbumEditTracksClicked( wxCommandEvent &event );
    void                    OnAlbumDClicked( wxMouseEvent &event );

    void                    OnMouse( wxMouseEvent &event );
    void                    OnBeginDrag( wxMouseEvent &event );
    void                    OnCoverBeginDrag( wxMouseEvent &event );

    void                    OnBitmapClicked( wxMouseEvent &event );
    void                    OnTimer( wxTimerEvent &event );

  public :
    guAlbumBrowserItemPanel( wxWindow * parent, const int index, guAlbumBrowserItem * albumitem = NULL );
    ~guAlbumBrowserItemPanel();

    void SetAlbumItem( const int index, guAlbumBrowserItem * albumitem, wxBitmap * blankcd );
    void UpdateDetails( void );
    void SetAlbumCover( const wxString &cover );

};
WX_DEFINE_ARRAY_PTR( guAlbumBrowserItemPanel *, guAlbumBrowserItemPanelArray );

WX_DEFINE_ARRAY_PTR( wxStaticText *, guStaticTextArray );

class guUpdateAlbumDetails;

// -------------------------------------------------------------------------------- //
class guAlbumBrowser : public wxPanel
{
  protected :
    guMediaViewer *                 m_MediaViewer;
    guDbLibrary *                   m_Db;
    guPlayerPanel *                 m_PlayerPanel;
    guAlbumBrowserItemArray         m_AlbumItems;
    wxMutex                         m_AlbumItemsMutex;
    guAlbumBrowserItemPanelArray    m_ItemPanels;
    wxMutex                         m_ItemPanelsMutex;
    guUpdateAlbumDetails *          m_UpdateDetails;
    wxMutex                         m_UpdateDetailsMutex;
    unsigned int                    m_ItemStart;        // The first element in a page
    unsigned int                    m_LastItemStart;
    unsigned int                    m_ItemCount;        // The number of elements in a page
    unsigned int                    m_AlbumsCount;      // The number of albums in database
    unsigned int                    m_PagesCount;       // The number of pages to scroll
    wxBitmap *                      m_BlankCD;
    wxTimer                         m_RefreshTimer;
    guDynPlayList                   m_DynFilter;
    guAlbumBrowserItem *            m_LastAlbumBrowserItem;
    int                             m_DragCount;
    wxPoint                         m_DragStart;
    bool                            m_MouseWasLeftUp;
    bool                            m_MouseSelecting;

    wxString                        m_LastSearchString;

    int                             m_SortSelected;

    wxString                        m_ConfigPath;

    wxTimer                         m_BitmapClickTimer;

    // GUI
    wxBoxSizer *                    m_MainSizer;
    wxBoxSizer *                    m_AlbumBrowserSizer;
	wxGridSizer *                   m_AlbumsSizer;
	wxStaticText *                  m_NavLabel;
	wxSlider *                      m_NavSlider;
	wxSize                          m_LastSize;
    wxArrayString                   m_TextSearchFilter;

    wxBoxSizer *                    m_BigCoverSizer;
    wxButton *                      m_BigCoverBackBtn;
    wxStaticBitmap *                m_BigCoverBitmap;
    guAutoScrollText *              m_BigCoverAlbumLabel;
    guAutoScrollText *              m_BigCoverArtistLabel;
    guAutoScrollText *              m_BigCoverDetailsLabel;
    wxListBox *                     m_BigCoverTracksListBox;
    guStaticTextArray               m_BigCoverTracksItems;
    guTrackArray                    m_BigCoverTracks;
    bool                            m_BigCoverShowed;
    bool                            m_BigCoverTracksContextMenu;

    int                             GetItemStart( void ) { return m_ItemStart; }
    int                             GetItemCount( void ) { return m_ItemCount; }
    int                             GetAlbumsCount( void ) { return m_AlbumsCount; }

    void                            OnChangedSize( wxSizeEvent &event );
    void                            OnChangingPosition( wxScrollEvent& event );
    void                            OnRefreshTimer( wxTimerEvent &event );

    void                            OnAddFilterClicked( wxCommandEvent &event );
    void                            OnDelFilterClicked( wxCommandEvent &event );
    void                            OnEditFilterClicked( wxCommandEvent &event );

    void                            OnSortSelected( wxCommandEvent &event );
    void                            OnUpdateDetails( wxCommandEvent &event );

    void                            OnMouseWheel( wxMouseEvent& event );

    virtual void                    NormalizeTracks( guTrackArray * tracks, const bool isdrag = false );

    virtual void                    OnBitmapClicked( guAlbumBrowserItem * albumitem, const wxPoint &position );

    virtual void                    OnAlbumSelectName( const int albumid );
    virtual void                    OnArtistSelectName( const int artistid );

    void                            OnGoToSearch( wxCommandEvent &event );
    void                            OnConfigUpdated( wxCommandEvent &event );
    void                            CreateAcceleratorTable( void );

    bool                            DoTextSearch( const wxString &searchtext );

    void                            OnBigCoverBackClicked( wxCommandEvent &event );
    void                            OnBigCoverBitmapClicked( wxMouseEvent &event );
    void                            OnBigCoverBitmapDClicked( wxMouseEvent &event );
    void                            OnBigCoverTracksDClicked( wxCommandEvent &event );
    void                            DoBackToAlbums( void );
    int                             GetSelectedTracks( guTrackArray * tracks );
    void                            DoSelectTracks( const guTrackArray &tracks, const bool append, const int aftercurrent = 0 );
    void                            OnBigCoverContextMenu( wxContextMenuEvent &event );
    void                            OnBigCoverTracksContextMenu( wxContextMenuEvent &event );

    void                            OnBigCoverPlayClicked( wxCommandEvent &event );
    void                            OnBigCoverEnqueueClicked( wxCommandEvent &event );
    void                            OnBigCoverCopyToClipboard( wxCommandEvent &event );
    void                            OnBigCoverAlbumSelectName( wxCommandEvent &event );
    void                            OnBigCoverArtistSelectName( wxCommandEvent &event );
    void                            OnBigCoverCommandClicked( wxCommandEvent &event );
    void                            OnBigCoverDownloadCoverClicked( wxCommandEvent &event );
    void                            OnBigCoverSelectCoverClicked( wxCommandEvent &event );
    void                            OnBigCoverDeleteCoverClicked( wxCommandEvent &event );
    void                            OnBigCoverEmbedCoverClicked( wxCommandEvent &event );
    void                            OnBigCoverCopyToClicked( wxCommandEvent &event );
    void                            OnBigCoverEditLabelsClicked( wxCommandEvent &event );
    void                            OnBigCoverEditTracksClicked( wxCommandEvent &event );
    void                            OnBigCoverSearchLinkClicked( wxCommandEvent &event );

    void                            OnBigCoverTracksPlayClicked( wxCommandEvent &event );
    void                            OnBigCoverTracksEnqueueClicked( wxCommandEvent &event );
    void                            OnBigCoverTracksEditLabelsClicked( wxCommandEvent &event );
    void                            OnBigCoverTracksEditTracksClicked( wxCommandEvent &event );
    void                            OnBigCoverTracksMouseMoved( wxMouseEvent &event );
    void                            OnBigCoverTracksBeginDrag( wxMouseEvent &event );
    void                            OnBigCoverTracksPlaylistSave( wxCommandEvent &event );
    void                            OnBigCoverTracksSmartPlaylist( wxCommandEvent &event );


    void                            OnTimerTimeout( wxTimerEvent &event );

    void                            CreateControls( void );

    void                            CalculateMaxItemHeight( void );

  public :
    //guAlbumBrowser( wxWindow * parent, guDbLibrary * db, guPlayerPanel * playerpanel );
    guAlbumBrowser( wxWindow * parent, guMediaViewer * mediaviewer );
    ~guAlbumBrowser();

    virtual int                     GetContextMenuFlags( void );

    virtual void                    SetLastAlbumItem( guAlbumBrowserItem * item ) { m_LastAlbumBrowserItem = item; }

    virtual void                    CreateContextMenu( wxMenu * Menu );

    void RefreshCount( void )
    {
        m_AlbumsCount = m_Db->GetAlbumsCount( m_DynFilter.IsEmpty() ? NULL : &m_DynFilter, m_TextSearchFilter );
        //m_ItemStart = 0;
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

    void                            UpdateNavLabel( const int page );

    virtual void                    SelectAlbum( const int albumid, const bool append, const int aftercurrent = 0 );
    virtual int                     GetAlbumTracks( const int albumid, guTrackArray * tracks );
    virtual void                    OnCommandClicked( const int commandid, const int albumid );
    virtual void                    OnCommandClicked( const int commandid, const guTrackArray &tracks );
    virtual void                    OnAlbumDownloadCoverClicked( const int albumid );
    virtual void                    OnAlbumSelectCoverClicked( const int albumid );
    virtual void                    OnAlbumDeleteCoverClicked( const int albumid );
    virtual void                    OnAlbumEmbedCoverClicked( const int albumid );
    virtual void                    OnAlbumCopyToClicked( const int albumid, const int commandid );
    virtual void                    OnAlbumEditLabelsClicked( const guAlbumBrowserItem * albumitem );
    virtual void                    OnAlbumEditTracksClicked( const int albumid );

    void                            LibraryUpdated( void );

    wxString                        GetAlbumCoverFile( const int albumid );

    void                            SetAlbumCover( const int albumid, const wxString &cover );

    void                            SetFilter( const wxString &filterstr );

    int                             GetSortSelected( void ) { return m_SortSelected; }

    virtual void                    AlbumCoverChanged( const int albumid );

    void                            SetPlayerPanel( guPlayerPanel * playerpanel ) { m_PlayerPanel = playerpanel; }

    wxString                        GetSelInfo( void );

    friend class guUpdateAlbumDetails;
    friend class guAlbumBrowserItemPanel;
    friend class guMediaViewer;
};

// -------------------------------------------------------------------------------- //
class guAlbumBrowserDropTarget : public wxDropTarget
{
  private:
    guAlbumBrowserItemPanel *       m_AlbumBrowserItemPanel;
    guMediaViewer *                 m_MediaViewer;

  public:
    guAlbumBrowserDropTarget( guMediaViewer * mediaviewer, guAlbumBrowserItemPanel * itempanel );
    ~guAlbumBrowserDropTarget() {};

    virtual wxDragResult OnData( wxCoord x, wxCoord y, wxDragResult def );
};

#endif
// -------------------------------------------------------------------------------- //
