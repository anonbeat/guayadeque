// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2011 J.Rios
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
#ifndef LIBPANEL_H
#define LIBPANEL_H

#include "AAListBox.h"
#include "AlListBox.h"
#include "ArListBox.h"
#include "AuiManagedPanel.h"
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
enum guLibraryElement {
    guLIBRARY_ELEMENT_TEXTSEARCH = 1,
    guLIBRARY_ELEMENT_LABELS,
    guLIBRARY_ELEMENT_GENRES,
    guLIBRARY_ELEMENT_ARTISTS,
    guLIBRARY_ELEMENT_COMPOSERS,
    guLIBRARY_ELEMENT_ALBUMARTISTS,
    guLIBRARY_ELEMENT_ALBUMS,
    guLIBRARY_ELEMENT_YEARS,
    guLIBRARY_ELEMENT_RATINGS,
    guLIBRARY_ELEMENT_PLAYCOUNT,
    guLIBRARY_ELEMENT_TRACKS
};

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


#define     guLIBRARY_CONTEXTMENU_EDIT_TRACKS       ( 1 << 0 )
#define     guLIBRARY_CONTEXTMENU_DOWNLOAD_COVERS   ( 1 << 1 )
#define     guLIBRARY_CONTEXTMENU_EMBED_COVERS      ( 1 << 2 )
#define     guLIBRARY_CONTEXTMENU_COPY_TO           ( 1 << 3 )
#define     guLIBRARY_CONTEXTMENU_LINKS             ( 1 << 4 )
#define     guLIBRARY_CONTEXTMENU_COMMANDS          ( 1 << 5 )
#define     guLIBRARY_CONTEXTMENU_DELETEFROMLIBRARY ( 1 << 6 )

#define     guLIBRARY_CONTEXTMENU_DEFAULT           ( guLIBRARY_CONTEXTMENU_EDIT_TRACKS | guLIBRARY_CONTEXTMENU_DOWNLOAD_COVERS |\
                                                    guLIBRARY_CONTEXTMENU_EMBED_COVERS | guLIBRARY_CONTEXTMENU_COPY_TO |\
                                                    guLIBRARY_CONTEXTMENU_LINKS | guLIBRARY_CONTEXTMENU_COMMANDS |\
                                                    guLIBRARY_CONTEXTMENU_DELETEFROMLIBRARY )

class guLibPanel;
class guImagePtrArray;

// -------------------------------------------------------------------------------- //
class guLibPanelDropTarget : public wxFileDropTarget
{
  protected :
    guLibPanel * m_LibPanel;

  public :
    guLibPanelDropTarget( guLibPanel * libpanel );
    ~guLibPanelDropTarget();

    virtual bool OnDropFiles( wxCoord x, wxCoord y, const wxArrayString &filenames );
};

// -------------------------------------------------------------------------------- //
class guLibPanel : public guAuiManagedPanel
{
  protected :
//    wxAuiManager            m_AuiManager;
//    unsigned int            m_VisiblePanels;

    wxSearchCtrl *          m_InputTextCtrl;
    guGeListBox *           m_GenreListCtrl;
    guTaListBox *           m_LabelsListCtrl;
    guArListBox *           m_ArtistListCtrl;
    guAlListBox *           m_AlbumListCtrl;
    guSoListBox *           m_SongListCtrl;
    guYeListBox *           m_YearListCtrl;
    guRaListBox *           m_RatingListCtrl;
    guPcListBox *           m_PlayCountListCtrl;
    guCoListBox *           m_ComposerListCtrl;
    guAAListBox *           m_AlbumArtistListCtrl;

    //
    guDbLibrary *           m_Db;
    bool                    m_UpdateLock;
    guPlayerPanel *         m_PlayerPanel;
    wxTimer                 m_SelChangedTimer;
    wxTimer                 m_TextChangedTimer;
    int                     m_SelChangedObject;
    bool                    m_DoneClearSearchText;

    int                     m_BaseCommand;
    wxString                m_ConfigPrefixVarName;
    int                     m_ContextMenuFlags;

    bool                    m_InstantSearchEnabled;
    bool                    m_EnterSelectSearchEnabled;

    // Search Str events
    virtual void            OnSearchActivated( wxCommandEvent &event );
    virtual void            OnSearchCancelled( wxCommandEvent &event );
    virtual void            OnSearchSelected( wxCommandEvent &event );
    virtual void            ClearSearchText( void );
    bool                    DoTextSearch( void );

    // LabelsListBox Events
    virtual void            OnLabelListActivated( wxListEvent &event );
    virtual void            OnLabelListSelected( wxListEvent &event );
    virtual void            OnLabelPlayClicked( wxCommandEvent &event );
    virtual void            OnLabelQueueClicked( wxCommandEvent &event );
    virtual void            OnLabelCopyToClicked( wxCommandEvent &event );
    virtual void            OnLabelSavePlayListClicked( wxCommandEvent &event );

    // GenreListBox Events
    virtual void            OnGenreListActivated( wxListEvent &event );
    virtual void            OnGenreListSelected( wxListEvent &event );
    virtual void            OnGenrePlayClicked( wxCommandEvent &event );
    virtual void            OnGenreQueueClicked( wxCommandEvent &event );
    virtual void            OnGenreCopyToClicked( wxCommandEvent &event );
    virtual void            OnGenreSavePlayListClicked( wxCommandEvent &event );

    // ArtistsListBox Events
    virtual void            OnArtistListActivated( wxListEvent &event );
    virtual void            OnArtistListSelected( wxListEvent &event );
    virtual void            OnArtistPlayClicked( wxCommandEvent &event );
    virtual void            OnArtistQueueClicked( wxCommandEvent &event );
    virtual void            OnArtistEditLabelsClicked( wxCommandEvent &event );
    virtual void            OnArtistEditTracksClicked( wxCommandEvent &event );
    virtual void            OnArtistCopyToClicked( wxCommandEvent &event );
    virtual void            OnArtistSavePlayListClicked( wxCommandEvent &event );

    // AlbumsListBoxEvents
    virtual void            OnAlbumListActivated( wxListEvent &event );
    virtual void            OnAlbumListSelected( wxListEvent &event );
    virtual void            OnAlbumPlayClicked( wxCommandEvent &event );
    virtual void            OnAlbumQueueClicked( wxCommandEvent &event );
    virtual void            OnAlbumEditLabelsClicked( wxCommandEvent &event );
    virtual void            OnAlbumEditTracksClicked( wxCommandEvent &event );
    virtual void            OnAlbumDownloadCoverClicked( wxCommandEvent &event );
    virtual void            OnAlbumSelectCoverClicked( wxCommandEvent &event );
    virtual void            OnAlbumDeleteCoverClicked( wxCommandEvent &event );
    virtual void            OnAlbumEmbedCoverClicked( wxCommandEvent &event );
    virtual void            OnAlbumCopyToClicked( wxCommandEvent &event );
    virtual void            OnAlbumSavePlayListClicked( wxCommandEvent &event );

    virtual void            DoDeleteAlbumCover( const int albumid );

    // YearsListBoxEvents
    virtual void            OnYearListSelected( wxListEvent &event );
    virtual void            OnYearListActivated( wxListEvent &event );
    virtual void            OnYearListPlayClicked( wxCommandEvent &event );
    virtual void            OnYearListQueueClicked( wxCommandEvent &event );
    virtual void            OnYearListEditTracksClicked( wxCommandEvent &event );
    virtual void            OnYearListCopyToClicked( wxCommandEvent &event );
    virtual void            OnYearSavePlayListClicked( wxCommandEvent &event );

    // RatingsListBoxEvents
    virtual void            OnRatingListSelected( wxListEvent &event );
    virtual void            OnRatingListActivated( wxListEvent &event );
    virtual void            OnRatingListPlayClicked( wxCommandEvent &event );
    virtual void            OnRatingListQueueClicked( wxCommandEvent &event );
    virtual void            OnRatingListEditTracksClicked( wxCommandEvent &event );
    virtual void            OnRatingListCopyToClicked( wxCommandEvent &event );
    virtual void            OnRatingSavePlayListClicked( wxCommandEvent &event );

    // PlayCountListBoxEvents
    virtual void            OnPlayCountListSelected( wxListEvent &event );
    virtual void            OnPlayCountListActivated( wxListEvent &event );
    virtual void            OnPlayCountListPlayClicked( wxCommandEvent &event );
    virtual void            OnPlayCountListQueueClicked( wxCommandEvent &event );
    virtual void            OnPlayCountListEditTracksClicked( wxCommandEvent &event );
    virtual void            OnPlayCountListCopyToClicked( wxCommandEvent &event );
    virtual void            OnPlayCountSavePlayListClicked( wxCommandEvent &event );

    // ComposersListBoxEvents
    virtual void            OnComposerListSelected( wxListEvent &event );
    virtual void            OnComposerListActivated( wxListEvent &event );
    virtual void            OnComposerListPlayClicked( wxCommandEvent &event );
    virtual void            OnComposerListQueueClicked( wxCommandEvent &event );
    virtual void            OnComposerListEditTracksClicked( wxCommandEvent &event );
    virtual void            OnComposerListCopyToClicked( wxCommandEvent &event );
    virtual void            OnComposerSavePlayListClicked( wxCommandEvent &event );

    // AlbumArtistsListBoxEvents
    virtual void            OnAlbumArtistListSelected( wxListEvent &event );
    virtual void            OnAlbumArtistListActivated( wxListEvent &event );
    virtual void            OnAlbumArtistListPlayClicked( wxCommandEvent &event );
    virtual void            OnAlbumArtistListQueueClicked( wxCommandEvent &event );
    virtual void            OnAlbumArtistListEditTracksClicked( wxCommandEvent &event );
    virtual void            OnAlbumArtistListCopyToClicked( wxCommandEvent &event );
    virtual void            OnAlbumArtistSavePlayListClicked( wxCommandEvent &event );

    // SongsListBox Events
    virtual void            OnSongListActivated( wxListEvent &event );
    virtual void            OnSongPlayClicked( wxCommandEvent &event );
    virtual void            OnSongPlayAllClicked( wxCommandEvent &event );
    virtual void            OnSongQueueClicked( wxCommandEvent &event );
    virtual void            OnSongsEditLabelsClicked( wxCommandEvent &event );
    virtual void            OnSongsEditTracksClicked( wxCommandEvent &event );
    virtual void            OnSongSavePlayListClicked( wxCommandEvent &event );
    virtual void            OnSongListColClicked( wxListEvent &event );
    virtual void            OnSongSelectGenre( wxCommandEvent &event );
    virtual void            OnSongSelectAlbumArtist( wxCommandEvent &event );
    virtual void            OnSongSelectArtist( wxCommandEvent &event );
    virtual void            OnSongSelectAlbum( wxCommandEvent &event );
    virtual void            OnSongDeleteLibrary( wxCommandEvent &event );
    virtual void            OnSongDeleteDrive( wxCommandEvent &event );
    virtual void            OnSongSetRating( wxCommandEvent &event );
    virtual void            OnSongSetField( wxCommandEvent &event );
    virtual void            OnSongEditField( wxCommandEvent &event );
    virtual void            OnSongCopyToClicked( wxCommandEvent &event );

    //
    void                    OnSelChangedTimer( wxTimerEvent &event );
    void                    OnTextChangedTimer( wxTimerEvent &event );
    void                    DoSelectionChanged( void );


    void                    ReloadLabels( bool reset = true ) { if( m_VisiblePanels & guPANEL_LIBRARY_LABELS ) m_LabelsListCtrl->ReloadItems( reset ); }
    void                    ReloadGenres( bool reset = true ) { if( m_VisiblePanels & guPANEL_LIBRARY_GENRES ) m_GenreListCtrl->ReloadItems( reset ); }
    void                    ReloadAlbumArtists( bool reset = true ) { if( m_VisiblePanels & guPANEL_LIBRARY_ALBUMARTISTS ) m_AlbumArtistListCtrl->ReloadItems( reset ); }
    void                    ReloadArtists( bool reset = true ) { if( m_VisiblePanels & guPANEL_LIBRARY_ARTISTS ) m_ArtistListCtrl->ReloadItems( reset ); }
    void                    ReloadComposers( bool reset = true ) { if( m_VisiblePanels & guPANEL_LIBRARY_COMPOSERS ) m_ComposerListCtrl->ReloadItems( reset ); }
    void                    ReloadAlbums( bool reset = true ) { if( m_VisiblePanels & guPANEL_LIBRARY_ALBUMS ) m_AlbumListCtrl->ReloadItems( reset ); }
    void                    ReloadYears( bool reset = true ) { if( m_VisiblePanels & guPANEL_LIBRARY_YEARS ) m_YearListCtrl->ReloadItems( reset ); }
    void                    ReloadRatings( bool reset = true ) { if( m_VisiblePanels & guPANEL_LIBRARY_RATINGS ) m_RatingListCtrl->ReloadItems( reset ); }
    void                    ReloadPlayCounts( bool reset = true ) { if( m_VisiblePanels & guPANEL_LIBRARY_PLAYCOUNT ) m_PlayCountListCtrl->ReloadItems( reset ); }
    void                    ReloadSongs( bool reset = true ) { m_SongListCtrl->ReloadItems( reset ); }

    virtual bool            OnDropFiles( const wxArrayString &filenames );

    virtual void            UpdatePlaylists( void );

    virtual void            DoEditTracks( guTrackArray &tracks );

    //virtual void            UpdateTracks( const guTrackArray &tracks, const wxArrayInt &changedflags );
    virtual void            UpdateTracks( const guTrackArray &tracks, const guImagePtrArray &images,
                                const wxArrayString &lyrics, const wxArrayInt &changedflags );
    void                    UpdateTracks( const guTrackArray &tracks, const wxArrayInt &changedflags );
    virtual void            UpdateTracksImages( const guTrackArray &tracks, const guImagePtrArray &images, const wxArrayInt &changedflags );
    virtual void            UpdateTracksLyrics( const guTrackArray &tracks, const wxArrayString &lyrics, const wxArrayInt &changedflags );

    virtual void            SaveToPlayList( const wxArrayInt &tracks );

    virtual void            OnGoToSearch( wxCommandEvent &event );

    void                    OnConfigUpdated( wxCommandEvent &event );

  public :
    guLibPanel( wxWindow * parent, guDbLibrary * db, guPlayerPanel * playerpanel, const wxString &prefix = wxT( "Lib" ) );
    ~guLibPanel();

    virtual void            InitPanelData( void );

    virtual void            NormalizeTracks( guTrackArray * tracks, const bool isdrag = false ) {};
    virtual wxString        GetName( void );
    virtual guDbLibrary *   GetDb( void ) { return m_Db; };
    virtual wxArrayString   GetLibraryPaths( void );
    virtual wxString        GetPlaylistPath( void ) { return wxEmptyString; }
    void                    SetBaseCommand( int basecmd ) { m_BaseCommand = basecmd; InitPanelData(); }

    void                    ReloadControls( void );
    void                    UpdateLabels( void );

    void                    SelectTrack( const int trackid );
    void                    SelectAlbum( const int albumid );
    void                    SelectArtist( const int artistid );
    void                    SelectYear( const int year );
    void                    SelectAlbumName( const wxString &album );
    void                    SelectArtistName( const wxString &artist );
    void                    SelectGenres( wxArrayInt * genres );
    void                    SelectArtists( wxArrayInt * artits );
    void                    SelectAlbumArtists( wxArrayInt * ids );
    void                    SelectAlbums( wxArrayInt * albums );

    void                    UpdatedTracks( const guTrackArray * tracks ) { if( m_SongListCtrl ) m_SongListCtrl->UpdatedTracks( tracks ); }
    void                    UpdatedTrack( const guTrack * track ) { if( m_SongListCtrl ) m_SongListCtrl->UpdatedTrack( track ); }

    int                     GetContextMenuFlags( void ) { return m_ContextMenuFlags; }
    virtual void            CreateContextMenu( wxMenu * menu, const int windowid = 0 );
    virtual void            CreateCopyToMenu( wxMenu * menu, const int basecmd );

    virtual int             LastUpdate( void );
    virtual void            SetLastUpdate( int lastupdate = wxNOT_FOUND );

    virtual void            DeleteTracks( guTrackArray * tracks );

    virtual wxArrayString   GetCoverSearchWords( void );

//    bool                    IsPanelShown( const int panelid ) const;
//    void                    ShowPanel( const int panelid, bool show );
//    void                    OnPaneClose( wxAuiManagerEvent &event );
//    int                     VisiblePanels( void ) { return m_VisiblePanels; }
//    wxString                SavePerspective( void ) { return m_AuiManager.SavePerspective(); }
//    void                    LoadPerspective( const wxString &layoutstr, const unsigned int visiblepanels );

    virtual wxString        GetCoverName( void );
    virtual int             GetCoverType( void ) { return wxBITMAP_TYPE_JPEG; }
    virtual int             GetCoverMaxSize( void ) { return wxNOT_FOUND; }

    virtual bool            SetAlbumCover( const int albumid, const wxString &albumpath, wxImage * coverimg );
    virtual bool            SetAlbumCover( const int albumid, const wxString &albumpath, wxString &coverpath );

    virtual wxImage *       GetAlbumCover( const int albumid, wxString &coverpath );

    virtual int             GetListViewColumnCount( void ) { return guSONGS_COLUMN_COUNT; }
    virtual bool            GetListViewColumnData( const int id, int * index, int * width, bool * enabled ) { return m_SongListCtrl->GetColumnData( id, index, width, enabled ); }
    virtual bool            SetListViewColumnData( const int id, const int index, const int width, const bool enabled, const bool refresh = false ) { return m_SongListCtrl->SetColumnData( id, index, width, enabled, refresh ); }

    friend class guLibPanelDropTarget;
};

#endif
// -------------------------------------------------------------------------------- //
