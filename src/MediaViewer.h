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
#ifndef guMEDIAVIEWER_H
#define guMEDIAVIEWER_H

#include "AlbumBrowser.h"
#include "AuiNotebook.h"
#include "Collections.h"
#include "LibPanel.h"
#include "PlayerPanel.h"
#include "PlayListPanel.h"
#include "Preferences.h"
#include "TreePanel.h"

#include <wx/dynarray.h>

enum guMediaViewerMode {
    guMEDIAVIEWER_MODE_NONE = -1,
    guMEDIAVIEWER_MODE_LIBRARY,
    guMEDIAVIEWER_MODE_ALBUMBROWSER,
    guMEDIAVIEWER_MODE_TREEVIEW,
    guMEDIAVIEWER_MODE_PLAYLISTS
};

enum guMediaViewerCommand {
    guMEDIAVIEWER_SHOW_LIBRARY,
    guMEDIAVIEWER_SHOW_ALBUMBROWSER,
    guMEDIAVIEWER_SHOW_TREEVIEW,
    guMEDIAVIEWER_SHOW_PLAYLISTS,
    guMEDIAVIEWER_SHOW_TEXTSEARCH,
    guMEDIAVIEWER_SHOW_LABELS,
    guMEDIAVIEWER_SHOW_GENRES,
    guMEDIAVIEWER_SHOW_ARTISTS,
    guMEDIAVIEWER_SHOW_COMPOSERS,
    guMEDIAVIEWER_SHOW_ALBUMARTISTS,
    guMEDIAVIEWER_SHOW_ALBUMS,
    guMEDIAVIEWER_SHOW_YEARS,
    guMEDIAVIEWER_SHOW_RATINGS,
    guMEDIAVIEWER_SHOW_PLAYCOUNTS
};

enum guMediaViewerSelect {
    guMEDIAVIEWER_SELECT_TRACK,
    guMEDIAVIEWER_SELECT_ARTIST,
    guMEDIAVIEWER_SELECT_ALBUM,
    guMEDIAVIEWER_SELECT_ALBUMARTIST,
    guMEDIAVIEWER_SELECT_COMPOSER,
    guMEDIAVIEWER_SELECT_YEAR,
    guMEDIAVIEWER_SELECT_GENRE
};

#define     guCONTEXTMENU_EDIT_TRACKS       ( 1 << 0 )
#define     guCONTEXTMENU_DOWNLOAD_COVERS   ( 1 << 1 )
#define     guCONTEXTMENU_EMBED_COVERS      ( 1 << 2 )
#define     guCONTEXTMENU_COPY_TO           ( 1 << 3 )
#define     guCONTEXTMENU_LINKS             ( 1 << 4 )
#define     guCONTEXTMENU_COMMANDS          ( 1 << 5 )
#define     guCONTEXTMENU_DELETEFROMLIBRARY ( 1 << 6 )

#define     guCONTEXTMENU_DEFAULT           ( guCONTEXTMENU_EDIT_TRACKS | guCONTEXTMENU_DOWNLOAD_COVERS |\
                                              guCONTEXTMENU_EMBED_COVERS | guCONTEXTMENU_COPY_TO |\
                                              guCONTEXTMENU_LINKS | guCONTEXTMENU_COMMANDS |\
                                              guCONTEXTMENU_DELETEFROMLIBRARY )



class guMainFrame;
class guLibUpdateThread;
class guLibCleanThread;
class guCopyToAction;
class guUpdateCoversThread;

// -------------------------------------------------------------------------------- //
class guMediaViewer : public wxPanel
{
  protected :
    bool                    m_IsDefault;
    guMediaCollection *     m_MediaCollection;
    guDbLibrary *           m_Db;
    guMainFrame *           m_MainFrame;
    guPlayerPanel *         m_PlayerPanel;
    int                     m_BaseCommand;
    int                     m_ViewMode;
    wxThread *              m_UpdateThread;
    guLibCleanThread *      m_CleanThread;
    wxBoxSizer *            m_FiltersSizer;
    guCopyToPattern *       m_CopyToPattern;
    guUpdateCoversThread *  m_UpdateCoversThread;

    wxChoice *              m_FilterChoice;
    wxBitmapButton *        m_AddFilterButton;
    wxBitmapButton *        m_DelFilterButton;
    wxBitmapButton *        m_EditFilterButton;
    wxSearchCtrl *          m_SearchTextCtrl;
    wxBitmapButton *        m_LibrarySelButton;
    wxBitmapButton *        m_AlbumBrowserSelButton;
    wxBitmapButton *        m_TreeViewSelButton;
    wxBitmapButton *        m_PlaylistsSelButton;

    guLibPanel *            m_LibPanel;
    guAlbumBrowser *        m_AlbumBrowser;
    guTreeViewPanel *       m_TreeViewPanel;
    guPlayListPanel *       m_PlayListPanel;

    wxTimer                 m_TextChangedTimer;
    bool                    m_DoneClearSearchText;
    bool                    m_InstantSearchEnabled;
    bool                    m_EnterSelectSearchEnabled;

    wxString                m_ConfigPath;
    wxArrayString           m_DynFilterArray;
    wxString                m_SearchText;

    int                     m_ContextMenuFlags;

    void                    OnViewChanged( wxCommandEvent &event );

    // Search Str events
    void                    OnSearchActivated( wxCommandEvent &event );
    void                    OnSearchCancelled( wxCommandEvent &event );
    void                    OnSearchSelected( wxCommandEvent &event );
    void                    OnTextChangedTimer( wxTimerEvent &event );
    virtual bool            DoTextSearch( void );


    virtual void            CreateControls( void );

    virtual void            PlayAllTracks( const bool enqueue );

    virtual void            OnConfigUpdated( wxCommandEvent &event );

    void                    OnAddFilterClicked( wxCommandEvent &event );
    void                    OnDelFilterClicked( wxCommandEvent &event );
    void                    OnEditFilterClicked( wxCommandEvent &event );
    void                    OnFilterSelected( wxCommandEvent &event );
    void                    SetFilter( const wxString &filter );

    void                    OnCleanFinished( wxCommandEvent &event ) { CleanFinished(); }
    void                    OnLibraryUpdated( wxCommandEvent &event ) { LibraryUpdated(); }

    void                    OnAddPath( void );

    virtual void            LoadMediaDb( void );

    virtual void            CreateAcceleratorTable( void );

    virtual void            OnGenreSetSelection( wxCommandEvent &event );
    virtual void            OnAlbumArtistSetSelection( wxCommandEvent &event );
    virtual void            OnComposerSetSelection( wxCommandEvent &event );
    virtual void            OnArtistSetSelection( wxCommandEvent &event );
    virtual void            OnAlbumSetSelection( wxCommandEvent &event );

    virtual void            OnUpdateLabels( wxCommandEvent &event );

  public :
    guMediaViewer( wxWindow * parent, guMediaCollection & mediacollection, const int basecommand, guMainFrame * mainframe, const int mode, guPlayerPanel * playerpanel );
    ~guMediaViewer();

    virtual void            InitMediaViewer( const int mode );

    virtual wxString        ConfigPath( void ) { return m_ConfigPath; }

    int                     GetBaseCommand( void ) { return m_BaseCommand; }

    bool                    IsDefault( void ) { return m_IsDefault; }
    void                    SetDefault( const bool isdefault ) { m_IsDefault = isdefault; }

    void                    ClearSearchText( void );

    void                    GoToSearch( void );

    virtual int             GetContextMenuFlags( void ) { return m_ContextMenuFlags; }
    virtual void            CreateContextMenu( wxMenu * menu, const int windowid = wxNOT_FOUND );
    virtual void            CreateCopyToMenu( wxMenu * menu );

    int                     GetViewMode( void ) { return m_ViewMode; }
    virtual void            SetViewMode( const int mode );

    guDbLibrary *           GetDb( void ) { return m_Db; }
    guPlayerPanel *         GetPlayerPanel( void ) { return m_PlayerPanel; }
    void                    SetPlayerPanel( guPlayerPanel * playerpanel );
    guMainFrame *           GetMainFrame( void ) { return m_MainFrame; }
    guLibPanel *            GetLibPanel( void ) { return m_LibPanel; }
    guAlbumBrowser *        GetAlbumBrowser( void ) { return m_AlbumBrowser; }
    guTreeViewPanel *       GetTreeViewPanel( void ) { return m_TreeViewPanel; }
    guPlayListPanel *       GetPlayListPanel( void ) { return m_PlayListPanel; }

    virtual bool            CreateLibraryView( void );
    virtual bool            CreateAlbumBrowserView( void );
    virtual bool            CreateTreeView( void );
    virtual bool            CreatePlayListView( void );

    virtual void            SetMenuState( const bool enabled = true );
    virtual void            ShowPanel( const int id, const bool enabled );

    virtual void            HandleCommand( const int command );

    // Collections related
    guMediaCollection *     GetMediaCollection( void ) { return m_MediaCollection; }
    virtual void            SetCollection( guMediaCollection &collection, const int basecommand );
    virtual wxString        GetUniqueId( void ) { return m_MediaCollection->m_UniqueId; }
    virtual int             GetType( void ) { return m_MediaCollection->m_Type; }
    virtual wxString        GetName( void ) { return m_MediaCollection->m_Name; }
    virtual wxArrayString   GetPaths( void ) { return m_MediaCollection->m_Paths; }
    virtual wxArrayString   GetCoverWords( void ) { return m_MediaCollection->m_CoverWords; }
    virtual bool            GetUpdateOnStart( void ) { return m_MediaCollection->m_UpdateOnStart; }
    virtual bool            GetScanPlaylists( void ) { return m_MediaCollection->m_ScanPlaylists; }
    virtual bool            GetScanFollowSymLinks( void ) { return m_MediaCollection->m_ScanFollowSymLinks; }
    virtual bool            GetScanEmbeddedCovers( void ) { return m_MediaCollection->m_ScanEmbeddedCovers; }
    virtual bool            GetEmbeddMetadata( void ) { return m_MediaCollection->m_EmbeddMetadata; }
    virtual wxString        GetDefaultCopyAction( void ) { return m_MediaCollection->m_DefaultCopyAction; }
    virtual int             GetLastUpdate( void ) { return m_MediaCollection->m_LastUpdate; }
    virtual void            SetLastUpdate( void );

    virtual void            UpdateLibrary( void );
    virtual void            UpgradeLibrary( void );
    virtual void            UpdateFinished( void );
    virtual void            CleanLibrary( void );
    virtual void            CleanFinished( void );

    virtual void            UpdatePlaylists( void );

    virtual void            LibraryUpdated( void );

    virtual void            UpdateCovers( void );
    virtual void            UpdateCoversFinished( void );

    virtual void            ImportFiles( void ) { ImportFiles( new guTrackArray() ); }
    virtual void            ImportFiles( guTrackArray * tracks );
    virtual void            ImportFiles( const wxArrayString &files );

    virtual void            SaveLayout( wxXmlNode * xmlnode );
    virtual void            LoadLayout( wxXmlNode * xmlnode );

    virtual wxString        GetSelInfo( void );

    virtual void            UpdatedTrack( const int updatedby, const guTrack * track );
    virtual void            UpdatedTracks( const int updatedby, const guTrackArray * tracks );

    virtual int             CopyTo( guTrack * track, guCopyToAction &copytoaction, wxString &filename, const int index );

    virtual void            NormalizeTracks( guTrackArray * tracks, const bool isdrag = false ) {}

    virtual void            DownloadAlbumCover( const int albumid );
    virtual void            SelectAlbumCover( const int albumid );
    virtual void            EmbedAlbumCover( const int albumid );
    virtual bool            SetAlbumCover( const int albumid, const wxString &coverpath, const bool update = true );
    virtual bool            SetAlbumCover( const int albumid, const wxString &albumpath, wxImage * coverimg );
    virtual bool            SetAlbumCover( const int albumid, const wxString &albumpath, wxString &coverpath );
    virtual void            DeleteAlbumCover( const int albumid );
    virtual void            DeleteAlbumCover( const wxArrayInt &albumids );
    virtual void            AlbumCoverChanged( const int album, const bool deleted = false );

    virtual wxString        GetCoverName( const int albumid );
    virtual int             GetCoverType( void ) { return wxBITMAP_TYPE_JPEG; }
    virtual int             GetCoverMaxSize( void ) { return wxNOT_FOUND; }

    virtual wxImage *       GetAlbumCover( const int albumid, int &coverid, wxString &coverpath,
                                           const wxString &artistname = wxEmptyString, const wxString &albumname = wxEmptyString );
    virtual int             GetAlbumCoverId( const int albumid ) { return m_Db->GetAlbumCoverId( albumid ); }

    virtual bool            FindMissingCover( const int albumid, const wxString &artistname,
                                              const wxString &albumname, const wxString &albumpath  );

    virtual void            SetSelection( const int type, const int id );

    virtual void            PlayListUpdated( void );

    virtual void            EditProperties( void );

    virtual void            DeleteTracks( const guTrackArray * tracks );

    virtual void            UpdateTracks( const guTrackArray &tracks, const guImagePtrArray &images,
                                          const wxArrayString &lyrics, const wxArrayInt &changedflags );

    virtual void            SetTracksRating( guTrackArray &tracks, const int rating );

    // Copy to support functions
    virtual wxString        AudioPath( void );
    virtual wxString        Pattern( void );
    virtual int             AudioFormats( void );
    virtual int             TranscodeFormat( void );
    virtual int             TranscodeScope( void );
    virtual int             TranscodeQuality( void );
    virtual int             PlaylistFormats( void );
    virtual wxString        PlaylistPath( void );
    virtual int             CoverFormats( void ) { return 2; } //guPORTABLEMEDIA_COVER_FORMAT_JPEG
    virtual wxString        CoverName( void ) { return GetCoverName( wxNOT_FOUND ); }
    virtual int             CoverSize( void ) { return 0; }

};
WX_DEFINE_ARRAY_PTR( guMediaViewer *, guMediaViewerArray );

// -------------------------------------------------------------------------------- //
class guUpdateCoversThread : public wxThread
{
  private:
    guMediaViewer *     m_MediaViewer;
    int                 m_GaugeId;

  public:
    guUpdateCoversThread( guMediaViewer * mediaviewer, int gaugeid );
    ~guUpdateCoversThread();

    virtual ExitCode Entry();
};

// -------------------------------------------------------------------------------- //
class guMediaViewerDropTarget : public wxFileDropTarget
{
  protected :
    guMediaViewer * m_MediaViewer;

  public :
    guMediaViewerDropTarget( guMediaViewer * libpanel );
    ~guMediaViewerDropTarget();

    virtual bool OnDropFiles( wxCoord x, wxCoord y, const wxArrayString &filenames );
};

#endif

// -------------------------------------------------------------------------------- //
