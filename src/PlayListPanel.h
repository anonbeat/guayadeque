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
#ifndef PLAYLISTPANEL_H
#define PLAYLISTPANEL_H

#include "AuiManagedPanel.h"
#include "DbLibrary.h"
#include "PlayerPanel.h"
#include "PLSoListBox.h"
#include "SoListBox.h"


#include <wx/aui/aui.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/statbmp.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/statline.h>
#include <wx/listctrl.h>
#include <wx/splitter.h>
#include <wx/frame.h>
#include <wx/treectrl.h>
#include <wx/imaglist.h>
#include <wx/srchctrl.h>

#define     guPANEL_PLAYLIST_TEXTSEARCH        ( 1 << 0 )

#define     guPANEL_PLAYLIST_VISIBLE_DEFAULT   ( 0 )

class guPLNamesDropTarget;
class guPlayListPanel;

// -------------------------------------------------------------------------------- //
// guPLNamesTreeCtrl
// -------------------------------------------------------------------------------- //
class guPLNamesTreeCtrl : public wxTreeCtrl
{
  protected :
    guDbLibrary *       m_Db;
    guPlayListPanel *   m_PlayListPanel;
    wxArrayString       m_TextSearchFilter;
    wxImageList *       m_ImageList;
    wxTreeItemId        m_RootId;
    wxTreeItemId        m_StaticId;
    wxTreeItemId        m_DynamicId;

    wxTreeItemId    m_DragOverItem;
    wxArrayInt      m_DropIds;

    void            OnContextMenu( wxTreeEvent &event );

    void            OnBeginDrag( wxTreeEvent &event );
    void            OnDragOver( const wxCoord x, const wxCoord y );
    void            OnDropFile( const wxString &filename );
    void            OnDropEnd( void );
    void            OnKeyDown( wxKeyEvent &event );

    void            OnConfigUpdated( wxCommandEvent &event );
    void            CreateAcceleratorTable( void );

  public :
    guPLNamesTreeCtrl( wxWindow * parent, guDbLibrary * db, guPlayListPanel * playlistpanel );
    ~guPLNamesTreeCtrl();

    void            ReloadItems( void );

    DECLARE_EVENT_TABLE()

    friend class guPLNamesDropTarget;
    friend class guPLNamesDropFilesThread;
    friend class guPlayListPanel;
};

// -------------------------------------------------------------------------------- //
class guPLNamesDropFilesThread : public wxThread
{
  protected :
    guPLNamesTreeCtrl *     m_PLNamesTreeCtrl;          // To add the files
    guPLNamesDropTarget *   m_PLNamesDropTarget;        // To clear the thread pointer once its finished
    wxArrayString           m_Files;

    void AddDropFiles( const wxString &DirName );

  public :
    guPLNamesDropFilesThread( guPLNamesDropTarget * plnamesdroptarget,
                                 guPLNamesTreeCtrl * plnamestreectrl, const wxArrayString &files );
    ~guPLNamesDropFilesThread();

    virtual ExitCode Entry();
};

// -------------------------------------------------------------------------------- //
class guPLNamesDropTarget : public wxFileDropTarget
{
  private:
    guPLNamesTreeCtrl *             m_PLNamesTreeCtrl;
    guPLNamesDropFilesThread *      m_PLNamesDropFilesThread;

    void ClearPlayListFilesThread( void ) { m_PLNamesDropFilesThread = NULL; };

  public:
    guPLNamesDropTarget( guPLNamesTreeCtrl * plnamestreectrl );
    ~guPLNamesDropTarget();

    virtual bool OnDropFiles( wxCoord x, wxCoord y, const wxArrayString &files );

    virtual wxDragResult OnDragOver( wxCoord x, wxCoord y, wxDragResult def );


    friend class guPLNamesDropFilesThread;
};

// -------------------------------------------------------------------------------- //
class guPlayListPanel : public guAuiManagedPanel
{
  protected :
    guDbLibrary *       m_Db;
    guPlayerPanel *     m_PlayerPanel;

    wxSplitterWindow *  m_MainSplitter;
    guPLNamesTreeCtrl * m_NamesTreeCtrl;
    guPLSoListBox *     m_PLTracksListBox;

    wxSearchCtrl *      m_InputTextCtrl;

    wxTimer             m_TextChangedTimer;
    wxString            m_ExportLastFolder;

    bool                m_InstantSearchEnabled;
    bool                m_EnterSelectSearchEnabled;

    void                OnPLNamesSelected( wxTreeEvent &event );
    void                OnPLNamesActivated( wxTreeEvent &event );
    void                OnPLNamesPlay( wxCommandEvent &event );
    void                OnPLNamesEnqueue( wxCommandEvent &event );
    void                OnPLNamesEnqueueAsNext( wxCommandEvent &event );

    void                OnPLNamesNewPlaylist( wxCommandEvent &event );
    void                OnPLNamesEditPlaylist( wxCommandEvent &event );
    void                OnPLNamesRenamePlaylist( wxCommandEvent &event );
    void                OnPLNamesDeletePlaylist( wxCommandEvent &event );
    void                OnPLNamesCopyTo( wxCommandEvent &event );

    void                OnPLNamesImport( wxCommandEvent &event );
    void                OnPLNamesExport( wxCommandEvent &event );

    virtual void        OnPLTracksActivated( wxListEvent &event );
    void                OnPLTracksPlayClicked( wxCommandEvent &event );
    void                OnPLTracksPlayAllClicked( wxCommandEvent &event );
    void                OnPLTracksQueueClicked( wxCommandEvent &event );
    void                OnPLTracksQueueAsNextClicked( wxCommandEvent &event );
    void                OnPLTracksQueueAllClicked( wxCommandEvent &event );
    void                OnPLTracksQueueAllAsNextClicked( wxCommandEvent &event );
    void                OnPLTracksDeleteClicked( wxCommandEvent &event );
    void                OnPLTracksEditLabelsClicked( wxCommandEvent &event );
    void                OnPLTracksEditTracksClicked( wxCommandEvent &event );
    void                OnPLTracksCopyToClicked( wxCommandEvent &event );
    void                OnPLTracksSavePlayListClicked( wxCommandEvent &event );
    void                OnPLTracksSetRating( wxCommandEvent &event );
    void                OnPLTracksSetField( wxCommandEvent &event );
    void                OnPLTracksEditField( wxCommandEvent &event );

    void                OnPLTracksSelectGenre( wxCommandEvent &event );
    void                OnPLTracksSelectAlbumArtist( wxCommandEvent &event );
    void                OnPLTracksSelectArtist( wxCommandEvent &event );
    void                OnPLTracksSelectAlbum( wxCommandEvent &event );

    void                OnSearchActivated( wxCommandEvent &event );
    void                OnSearchSelected( wxCommandEvent &event );
    void                OnSearchCancelled( wxCommandEvent &event );

    void                OnTextChangedTimer( wxTimerEvent &event );

    void                DeleteCurrentPlayList( void );

    void                OnPLTracksDeleteLibrary( wxCommandEvent &event );
    void                OnPLTracksDeleteDrive( wxCommandEvent &event );

    virtual void        NormalizeTracks( guTrackArray * tracks, const bool isdrag = false ) {};
    virtual void        SendPlayListUpdatedEvent( void );

    void                OnGoToSearch( wxCommandEvent &event );
    bool                DoTextSearch( void );

    void                OnSetAllowDenyFilter( wxCommandEvent &event );

    void                OnConfigUpdated( wxCommandEvent &event );

  public :
    guPlayListPanel( wxWindow * parent, guDbLibrary * db, guPlayerPanel * playerpanel );
    ~guPlayListPanel();

    virtual void        InitPanelData( void );

    void                PlayListUpdated( void );

    bool                GetPlayListCounters( wxLongLong * count, wxLongLong * len, wxLongLong * size );

    void inline         UpdatedTracks( const guTrackArray * tracks ) { m_PLTracksListBox->UpdatedTracks( tracks ); };
    void inline         UpdatedTrack( const guTrack * track ) { m_PLTracksListBox->UpdatedTrack( track ); };

    virtual int         GetListViewColumnCount( void ) { return guSONGS_COLUMN_COUNT; }
    virtual bool        GetListViewColumnData( const int id, int * index, int * width, bool * enabled ) { return m_PLTracksListBox->GetColumnData( id, index, width, enabled ); }
    virtual bool        SetListViewColumnData( const int id, const int index, const int width, const bool enabled, const bool refresh = false ) { return m_PLTracksListBox->SetColumnData( id, index, width, enabled, refresh ); }

    friend class guPLNamesTreeCtrl;
};

#endif
// -------------------------------------------------------------------------------- //
