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
#ifndef LIBPANEL_H
#define LIBPANEL_H

#include "AlListBox.h"
#include "ArListBox.h"
#include "DbLibrary.h"
#include "GeListBox.h"
#include "PlayerPanel.h"
#include "SoListBox.h"
#include "TaListBox.h"

#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/statline.h>
#include <wx/notebook.h>
#include <wx/splitter.h>
#include <wx/srchctrl.h>

// -------------------------------------------------------------------------------- //
#define     guPANEL_LIBRARY_TEXTSEARCH      ( 1 << 0 )
#define     guPANEL_LIBRARY_LABELS          ( 1 << 1 )
#define     guPANEL_LIBRARY_GENRES          ( 1 << 2 )
#define     guPANEL_LIBRARY_ARTISTS         ( 1 << 3 )
#define     guPANEL_LIBRARY_ALBUMS          ( 1 << 4 )
#define     guPANEL_LIBRARY_TRACKS          ( 1 << 5 )
#define     guPANEL_LIBRARY_YEARS           ( 1 << 6 )
#define     guPANEL_LIBRARY_RATINGS         ( 1 << 7 )
#define     guPANEL_LIBRARY_COVERBROWSER    ( 1 << 8 )
#define     guPANEL_LIBRARY_COVERFLOW       ( 1 << 9 )

// -------------------------------------------------------------------------------- //
class guLibPanel : public wxPanel
{
  protected :
    wxAuiManager        m_AuiManager;

    wxSearchCtrl *      m_InputTextCtrl;
    guGeListBox *       m_GenreListCtrl;
    guTaListBox *       m_LabelsListCtrl;
    guArListBox *       m_ArtistListCtrl;
    guAlListBox *       m_AlbumListCtrl;
    guSoListBox *       m_SongListCtrl;

    //
    guDbLibrary *       m_Db;
    bool                m_UpdateLock;
    guPlayerPanel *     m_PlayerPanel;

    unsigned int        m_VisiblePanels;

    // Search Str events
    void OnSearchActivated( wxCommandEvent &event );
    void OnSearchCancelled( wxCommandEvent &event );

    // GenreListBox Events
    void OnGenreListActivated( wxListEvent &event );
    void OnGenreListSelected( wxListEvent &event );
    void OnGenrePlayClicked( wxCommandEvent &event );
    void OnGenreQueueClicked( wxCommandEvent &event );
    void OnGenreCopyToClicked( wxCommandEvent &event );

    // LabelsListBox Events
    void OnLabelListActivated( wxListEvent &event );
    void OnLabelListSelected( wxListEvent &event );
    void OnLabelPlayClicked( wxCommandEvent &event );
    void OnLabelQueueClicked( wxCommandEvent &event );
//    void OnLabelClearSelectClicked( wxCommandEvent &event );
    void OnLabelCopyToClicked( wxCommandEvent &event );

    // ArtistsListBox Events
    void OnArtistListActivated( wxListEvent &event );
    void OnArtistListSelected( wxListEvent &event );
    void OnArtistPlayClicked( wxCommandEvent &event );
    void OnArtistQueueClicked( wxCommandEvent &event );
    void OnArtistEditLabelsClicked( wxCommandEvent &event );
    void OnArtistEditTracksClicked( wxCommandEvent &event );
    void OnArtistCopyToClicked( wxCommandEvent &event );

    // AlbumsListBoxEvents
    void OnAlbumListActivated( wxListEvent &event );
    void OnAlbumListSelected( wxListEvent &event );
    void OnAlbumPlayClicked( wxCommandEvent &event );
    void OnAlbumQueueClicked( wxCommandEvent &event );
    void OnAlbumEditLabelsClicked( wxCommandEvent &event );
    void OnAlbumEditTracksClicked( wxCommandEvent &event );
    void OnAlbumDownloadCoverClicked( wxCommandEvent &event );
    void OnAlbumSelectCoverClicked( wxCommandEvent &event );
    void OnAlbumDeleteCoverClicked( wxCommandEvent &event );
    void OnAlbumCopyToClicked( wxCommandEvent &event );

    // SongsListBox Events
    void OnSongListActivated( wxListEvent &event );
    void OnSongPlayClicked( wxCommandEvent &event );
    void OnSongPlayAllClicked( wxCommandEvent &event );
    void OnSongQueueClicked( wxCommandEvent &event );
    void OnSongQueueAllClicked( wxCommandEvent &event );
    void OnSongsEditLabelsClicked( wxCommandEvent &event );
    void OnSongsEditTracksClicked( wxCommandEvent &event );
    void OnSongCopyToClicked( wxCommandEvent &event );
    void OnSongSavePlayListClicked( wxCommandEvent &event );
    void OnSongListColClicked( wxListEvent &event );
    void OnSongSelectGenre( wxCommandEvent &event );
    void OnSongSelectArtist( wxCommandEvent &event );
    void OnSongSelectAlbum( wxCommandEvent &event );

  public :
    guLibPanel( wxWindow * parent, guDbLibrary * NewDb, guPlayerPanel * NewPlayerPanel );
    ~guLibPanel();
    void ReloadControls( wxCommandEvent &event );
    void UpdateLabels( void );

    void SelectAlbumName( const wxString &album );
    void SelectArtistName( const wxString &artist );
    void SelectGenres( wxArrayInt * genres );
    void SelectArtists( wxArrayInt * artits );
    void SelectAlbums( wxArrayInt * albums );

    bool IsPanelShown( const int panelid ) const;
    void ShowPanel( const int panelid, bool show );

};

#endif
// -------------------------------------------------------------------------------- //
