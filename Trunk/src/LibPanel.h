// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2010 J.Rios
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

#include "AAListBox.h"
#include "AlListBox.h"
#include "ArListBox.h"
#include "CoListBox.h"
#include "DbLibrary.h"
#include "GeListBox.h"
#include "PcListBox.h"
#include "PlayerPanel.h"
#include "RaListBox.h"
#include "SoListBox.h"
#include "TaListBox.h"
#include "YeListBox.h"

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
//#define     guPANEL_LIBRARY_TRACKS          ( 1 << 5 )
#define     guPANEL_LIBRARY_YEARS           ( 1 << 6 )
#define     guPANEL_LIBRARY_RATINGS         ( 1 << 7 )
#define     guPANEL_LIBRARY_PLAYCOUNT       ( 1 << 8 )
#define     guPANEL_LIBRARY_COMPOSERS       ( 1 << 9 )
#define     guPANEL_LIBRARY_ALBUMARTISTS    ( 1 << 10 )

#define     guPANEL_LIBRARY_VISIBLE_DEFAULT ( guPANEL_LIBRARY_TEXTSEARCH | guPANEL_LIBRARY_LABELS |\
                                              guPANEL_LIBRARY_GENRES | guPANEL_LIBRARY_ARTISTS |\
                                              guPANEL_LIBRARY_ALBUMS )

// -------------------------------------------------------------------------------- //
class guLibPanel : public wxPanel
{
  protected :
    wxAuiManager        m_AuiManager;
    unsigned int        m_VisiblePanels;

    wxSearchCtrl *      m_InputTextCtrl;
    guGeListBox *       m_GenreListCtrl;
    guTaListBox *       m_LabelsListCtrl;
    guArListBox *       m_ArtistListCtrl;
    guAlListBox *       m_AlbumListCtrl;
    guSoListBox *       m_SongListCtrl;
    guYeListBox *       m_YearListCtrl;
    guRaListBox *       m_RatingListCtrl;
    guPcListBox *       m_PlayCountListCtrl;
    guCoListBox *       m_ComposerListCtrl;
    guAAListBox *       m_AlbumArtistListCtrl;

    //
    guDbLibrary *       m_Db;
    bool                m_UpdateLock;
    guPlayerPanel *     m_PlayerPanel;
    wxTimer             m_SelChangedTimer;
    wxTimer             m_TextChangedTimer;
    int                 m_SelChangedObject;

    // Search Str events
    void OnSearchActivated( wxCommandEvent &event );
    void OnSearchCancelled( wxCommandEvent &event );
    void OnSearchSelected( wxCommandEvent &event );

    // LabelsListBox Events
    void OnLabelListActivated( wxListEvent &event );
    void OnLabelListSelected( wxListEvent &event );
    void OnLabelPlayClicked( wxCommandEvent &event );
    void OnLabelQueueClicked( wxCommandEvent &event );
    void OnLabelQueueAsNextClicked( wxCommandEvent &event );
//    void OnLabelClearSelectClicked( wxCommandEvent &event );
    void OnLabelCopyToClicked( wxCommandEvent &event );

    // GenreListBox Events
    void OnGenreListActivated( wxListEvent &event );
    void OnGenreListSelected( wxListEvent &event );
    void OnGenrePlayClicked( wxCommandEvent &event );
    void OnGenreQueueClicked( wxCommandEvent &event );
    void OnGenreQueueAsNextClicked( wxCommandEvent &event );
    void OnGenreCopyToClicked( wxCommandEvent &event );

    // ArtistsListBox Events
    void OnArtistListActivated( wxListEvent &event );
    void OnArtistListSelected( wxListEvent &event );
    void OnArtistPlayClicked( wxCommandEvent &event );
    void OnArtistQueueClicked( wxCommandEvent &event );
    void OnArtistQueueAsNextClicked( wxCommandEvent &event );
    void OnArtistEditLabelsClicked( wxCommandEvent &event );
    void OnArtistEditTracksClicked( wxCommandEvent &event );
    void OnArtistCopyToClicked( wxCommandEvent &event );

    // AlbumsListBoxEvents
    void OnAlbumListActivated( wxListEvent &event );
    void OnAlbumListSelected( wxListEvent &event );
    void OnAlbumPlayClicked( wxCommandEvent &event );
    void OnAlbumQueueClicked( wxCommandEvent &event );
    void OnAlbumQueueAsNextClicked( wxCommandEvent &event );
    void OnAlbumEditLabelsClicked( wxCommandEvent &event );
    void OnAlbumEditTracksClicked( wxCommandEvent &event );
    void OnAlbumDownloadCoverClicked( wxCommandEvent &event );
    void OnAlbumSelectCoverClicked( wxCommandEvent &event );
    void OnAlbumDeleteCoverClicked( wxCommandEvent &event );
    void OnAlbumCopyToClicked( wxCommandEvent &event );

    // YearsListBoxEvents
    void OnYearListSelected( wxListEvent &event );
    void OnYearListActivated( wxListEvent &event );
    void OnYearListPlayClicked( wxCommandEvent &event );
    void OnYearListQueueClicked( wxCommandEvent &event );
    void OnYearListQueueAsNextClicked( wxCommandEvent &event );
    void OnYearListEditTracksClicked( wxCommandEvent &event );
    void OnYearListCopyToClicked( wxCommandEvent &event );

    // RatingsListBoxEvents
    void OnRatingListSelected( wxListEvent &event );
    void OnRatingListActivated( wxListEvent &event );
    void OnRatingListPlayClicked( wxCommandEvent &event );
    void OnRatingListQueueClicked( wxCommandEvent &event );
    void OnRatingListQueueAsNextClicked( wxCommandEvent &event );
    void OnRatingListEditTracksClicked( wxCommandEvent &event );
    void OnRatingListCopyToClicked( wxCommandEvent &event );

    // PlayCountListBoxEvents
    void OnPlayCountListSelected( wxListEvent &event );
    void OnPlayCountListActivated( wxListEvent &event );
    void OnPlayCountListPlayClicked( wxCommandEvent &event );
    void OnPlayCountListQueueClicked( wxCommandEvent &event );
    void OnPlayCountListQueueAsNextClicked( wxCommandEvent &event );
    void OnPlayCountListEditTracksClicked( wxCommandEvent &event );
    void OnPlayCountListCopyToClicked( wxCommandEvent &event );

    // ComposersListBoxEvents
    void OnComposerListSelected( wxListEvent &event );
    void OnComposerListActivated( wxListEvent &event );
    void OnComposerListPlayClicked( wxCommandEvent &event );
    void OnComposerListQueueClicked( wxCommandEvent &event );
    void OnComposerListQueueAsNextClicked( wxCommandEvent &event );
    void OnComposerListEditTracksClicked( wxCommandEvent &event );
    void OnComposerListCopyToClicked( wxCommandEvent &event );

    // AlbumArtistsListBoxEvents
    void OnAlbumArtistListSelected( wxListEvent &event );
    void OnAlbumArtistListActivated( wxListEvent &event );
    void OnAlbumArtistListPlayClicked( wxCommandEvent &event );
    void OnAlbumArtistListQueueClicked( wxCommandEvent &event );
    void OnAlbumArtistListQueueAsNextClicked( wxCommandEvent &event );
    void OnAlbumArtistListEditTracksClicked( wxCommandEvent &event );
    void OnAlbumArtistListCopyToClicked( wxCommandEvent &event );

    // SongsListBox Events
    void OnSongListActivated( wxListEvent &event );
    void OnSongPlayClicked( wxCommandEvent &event );
    void OnSongPlayAllClicked( wxCommandEvent &event );
    void OnSongQueueClicked( wxCommandEvent &event );
    void OnSongQueueAsNextClicked( wxCommandEvent &event );
    void OnSongQueueAllClicked( wxCommandEvent &event );
    void OnSongQueueAllAsNextClicked( wxCommandEvent &event );
    void OnSongsEditLabelsClicked( wxCommandEvent &event );
    void OnSongsEditTracksClicked( wxCommandEvent &event );
    void OnSongCopyToClicked( wxCommandEvent &event );
    void OnSongSavePlayListClicked( wxCommandEvent &event );
    void OnSongListColClicked( wxListEvent &event );
    void OnSongSelectGenre( wxCommandEvent &event );
    void OnSongSelectArtist( wxCommandEvent &event );
    void OnSongSelectAlbum( wxCommandEvent &event );

    //
    void OnPaneClose( wxAuiManagerEvent &event );

    //
    void OnSelChangedTimer( wxTimerEvent &event );
    void OnTextChangedTimer( wxTimerEvent &event );
    void DoSelectionChanged( void );

  public :
    guLibPanel( wxWindow * parent, guDbLibrary * NewDb, guPlayerPanel * NewPlayerPanel );
    ~guLibPanel();
    void ReloadControls( wxCommandEvent &event );
    void UpdateLabels( void );

    void SelectTrack( const int trackid );
    void SelectAlbum( const int albumid );
    void SelectArtist( const int artistid );
    void SelectYear( const int year );
    void SelectAlbumName( const wxString &album );
    void SelectArtistName( const wxString &artist );
    void SelectGenres( wxArrayInt * genres );
    void SelectArtists( wxArrayInt * artits );
    void SelectAlbums( wxArrayInt * albums );

    bool IsPanelShown( const int panelid ) const;
    void ShowPanel( const int panelid, bool show );

    void inline UpdatedTracks( const guTrackArray * tracks )
    {
        if( m_SongListCtrl )
            m_SongListCtrl->UpdatedTracks( tracks );
    }

    void inline UpdatedTrack( const guTrack * track )
    {
        if( m_SongListCtrl )
            m_SongListCtrl->UpdatedTrack( track );
    }

};

#endif
// -------------------------------------------------------------------------------- //
