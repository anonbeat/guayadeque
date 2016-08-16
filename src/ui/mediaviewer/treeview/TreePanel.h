// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2016 J.Rios anonbeat@gmail.com
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
#ifndef __TREEPANEL_H__
#define __TREEPANEL_H__

#include "AuiManagerPanel.h"
#include "DbLibrary.h"
#include "PlayerPanel.h"
#include "LibPanel.h"
#include "SoListBox.h"
#include "TreeViewFilter.h"
#include "TVSoListBox.h"

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

namespace Guayadeque {

#define     guPANEL_TREEVIEW_TEXTSEARCH        ( 1 << 0 )

#define     guPANEL_TREEVIEW_VISIBLE_DEFAULT   ( 0 )


class guTreeViewPanel;

// -------------------------------------------------------------------------------- //
class guTreeViewData : public wxTreeItemData
{
  public :
    int         m_Id;
    int         m_Type;

    guTreeViewData( const int id, const int type ) { m_Id = id; m_Type = type; }

    int         GetData( void ) { return m_Id; }
    void        SetData( int id ) { m_Id = id; }
    int         GetType( void ) { return m_Type; }
    void        SetType( int type ) { m_Type = type; }
};

// -------------------------------------------------------------------------------- //
// guTreeViewTreeCtrl
// -------------------------------------------------------------------------------- //
class guTreeViewTreeCtrl : public wxTreeCtrl
{
  protected :
    guDbLibrary *       m_Db;
    guTreeViewPanel *   m_TreeViewPanel;

    wxArrayString       m_TextFilters;

    wxImageList *       m_ImageList;
    wxTreeItemId        m_RootId;
    wxTreeItemId        m_FiltersId;
    wxTreeItemId        m_LibraryId;

    wxArrayString       m_FilterEntries;
    wxArrayInt          m_FilterLayout;
    int                 m_CurrentFilter;

    wxString            m_ConfigPath;

    void                OnContextMenu( wxTreeEvent &event );

    void                OnBeginDrag( wxTreeEvent &event );

    void                OnConfigUpdated( wxCommandEvent &event );
    void                CreateAcceleratorTable( void );

    void                LoadFilterLayout( void );

    void                OnSearchLinkClicked( wxCommandEvent &event );
    void                OnCommandClicked( wxCommandEvent &event );

  public :
    guTreeViewTreeCtrl( wxWindow * parent, guDbLibrary * db, guTreeViewPanel * treeviewpanel );
    ~guTreeViewTreeCtrl();

    void                ReloadItems( void );
    void                ReloadFilters( void );

    virtual int         GetContextMenuFlags( void );

    void                SetCurrentFilter( const int curfilter ) { m_CurrentFilter = curfilter; LoadFilterLayout(); ReloadFilters(); }

    wxString            GetFilterEntry( const int index ) { return m_FilterEntries[ index ]; }
    void                AppendFilterEntry( const wxString &filterentry ) { m_FilterEntries.Add( filterentry ); ReloadFilters(); }
    void                DeleteFilterEntry( const int index ) { if( m_FilterEntries.Count() == 1 ) return; m_FilterEntries.RemoveAt( index ); ReloadFilters(); }
    void                SetFilterEntry( const int index, const wxString &filterentry ) { m_FilterEntries[ index ] = filterentry; ReloadFilters(); }

    void                GetItemFilterEntry( const wxTreeItemId &treeitemid, guTreeViewFilterEntry &filterentry );

    bool                IsFilterItem( const wxTreeItemId &item );
    bool                IsFilterEntry( const wxTreeItemId &item ) { return GetItemParent( item ) == m_FiltersId; }

    void                SetTextFilters( const wxArrayString &textfilters ) { m_TextFilters = textfilters; }
    void                ClearTextFilters( void ) { m_TextFilters.Clear(); }

    DECLARE_EVENT_TABLE()

    friend class guTreeViewPanel;
};

// -------------------------------------------------------------------------------- //
class guTreeViewPanel : public guAuiManagerPanel
{
  protected :
    guMediaViewer *         m_MediaViewer;
    guDbLibrary *           m_Db;
    guPlayerPanel *         m_PlayerPanel;

    wxSplitterWindow *      m_MainSplitter;
    guTreeViewTreeCtrl *    m_TreeViewCtrl;
    guTVSoListBox *         m_TVTracksListBox;

    wxTimer                 m_TreeItemSelectedTimer;

    wxString                m_ConfigPath;

    wxString                m_LastSearchString;

    void                    OnTreeViewSelected( wxTreeEvent &event );
    void                    OnTreeViewActivated( wxTreeEvent &event );
    void                    OnTreeViewPlay( wxCommandEvent &event );
    void                    OnTreeViewEnqueue( wxCommandEvent &event );

    void                    OnTreeViewNewFilter( wxCommandEvent &event );
    void                    OnTreeViewEditFilter( wxCommandEvent &event );
    void                    OnTreeViewDeleteFilter( wxCommandEvent &event );
    void                    OnTreeViewEditLabels( wxCommandEvent &event );
    void                    OnTreeViewEditTracks( wxCommandEvent &event );
    void                    OnTreeViewSaveToPlayList( wxCommandEvent &event );
    void                    OnTreeViewCopyTo( wxCommandEvent &event );


    virtual void            OnTVTracksActivated( wxCommandEvent &event );
    void                    OnTVTracksPlayClicked( wxCommandEvent &event );
    void                    OnTVTracksQueueClicked( wxCommandEvent &event );
    void                    OnTVTracksEditLabelsClicked( wxCommandEvent &event );
    void                    OnTVTracksEditTracksClicked( wxCommandEvent &event );
    void                    OnTVTracksCopyToClicked( wxCommandEvent &event );
    void                    OnTVTracksSavePlayListClicked( wxCommandEvent &event );
    void                    OnTVTracksSetRating( wxCommandEvent &event );
    void                    OnTVTracksSetField( wxCommandEvent &event );
    void                    OnTVTracksEditField( wxCommandEvent &event );

    void                    OnTVTracksSelectGenre( wxCommandEvent &event );
    void                    OnTVTracksSelectAlbumArtist( wxCommandEvent &event );
    void                    OnTVTracksSelectComposer( wxCommandEvent &event );
    void                    OnTVTracksSelectArtist( wxCommandEvent &event );
    void                    OnTVTracksSelectAlbum( wxCommandEvent &event );

    void                    OnTreeItemSelectedTimer( wxTimerEvent &event );

    void                    OnTVTracksDeleteLibrary( wxCommandEvent &event );
    void                    OnTVTracksDeleteDrive( wxCommandEvent &event );

    virtual void            NormalizeTracks( guTrackArray * tracks, const bool isdrag = false );

    virtual void            SendPlayListUpdatedEvent( void );

    void                    OnGoToSearch( wxCommandEvent &event );
    bool                    DoTextSearch( const wxString &searchtext );

    void                    OnTrackListColClicked( wxListEvent &event );

    void                    CreateControls( void );

  public :
    guTreeViewPanel( wxWindow * parent, guMediaViewer * mediaviewer );
    ~guTreeViewPanel();

    virtual void            InitPanelData( void );

    virtual int             GetContextMenuFlags( void );
    virtual void            CreateCopyToMenu( wxMenu * menu );
    virtual void            CreateContextMenu( wxMenu * menu, const int windowid );

    void                    GetTreeViewCounters( wxLongLong * count, wxLongLong * len, wxLongLong * size ) { m_TVTracksListBox->GetCounters( count, len, size ); }

    void                    UpdatedTracks( const guTrackArray * tracks );
    void                    UpdatedTrack( const guTrack * track );

    virtual int             GetListViewColumnCount( void ) { return guSONGS_COLUMN_COUNT; }
    virtual bool            GetListViewColumnData( const int id, int * index, int * width, bool * enabled ) { return m_TVTracksListBox->GetColumnData( id, index, width, enabled ); }
    virtual bool            SetListViewColumnData( const int id, const int index, const int width, const bool enabled, const bool refresh = false ) { return m_TVTracksListBox->SetColumnData( id, index, width, enabled, refresh ); }

    void                    GetAllTracks( guTrackArray * tracks ) { m_TVTracksListBox->GetAllSongs( tracks ); }

    void                    RefreshAll( void );

    void                    SetPlayerPanel( guPlayerPanel * playerpanel ) { m_PlayerPanel = playerpanel; }

    wxString                ConfigPath( void ) { return m_ConfigPath; }

    friend class guTreeViewTreeCtrl;
    friend class guMediaViewer;
};

}

#endif
// -------------------------------------------------------------------------------- //
