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
#ifndef PLAYLIST_H
#define PLAYLIST_H

#include "DbLibrary.h"
#include "ListView.h"

#include <wx/wx.h>
#include <wx/vlbox.h>
#include <wx/dnd.h>
#include <wx/dir.h>
#include <wx/arrimpl.cpp>
#include <wx/thread.h>

wxString LenToString( int Len );

class guPlayerPanel;

// -------------------------------------------------------------------------------- //
class guPlayList : public guListView
{
  private :
    guDbLibrary *   m_Db;
    guPlayerPanel * m_PlayerPanel;
    guTrackArray    m_Items;
    wxMutex         m_ItemsMutex;
    bool            m_StartPlaying;
    long            m_CurItem;
    long            m_TotalLen;
    long            m_MaxPlayedTracks;
    int             m_MinPlayListTracks;
    bool            m_DelTracksPLayed;
    int             m_ItemHeight;

    wxBitmap *      m_PlayBitmap;
    wxBitmap *      m_GreyStar;
    wxBitmap *      m_YellowStar;
    wxCoord         m_SecondLineOffset;


    virtual wxCoord             OnMeasureItem( size_t row ) const;

//    void                        OnDragOver( const wxCoord x, const wxCoord y );
//    void                        OnDrawItem( wxDC &dc, const wxRect &rect, size_t n ) const;
    virtual int                 GetDragFiles( wxFileDataObject * files );
    virtual void                OnDropFile( const wxString &filename );
    virtual void                OnDropBegin( void );
    virtual void                OnDropEnd( void );



//    void                        OnMouse( wxMouseEvent &event );
    void                        RemoveSelected();
    virtual void                MoveSelection( void );

    void                        OnClearClicked( wxCommandEvent &event );
    void                        OnRemoveClicked( wxCommandEvent &event );
    void                        OnSaveClicked( wxCommandEvent &event );
    void                        OnAppendToPlaylistClicked( wxCommandEvent &event );
    void                        OnCopyToClicked( wxCommandEvent &event );
    void                        OnEditLabelsClicked( wxCommandEvent &event );
    void                        OnEditTracksClicked( wxCommandEvent &event );
    void                        OnSearchLinkClicked( wxCommandEvent &event );
    void                        OnCommandClicked( wxCommandEvent &event );
    wxString                    GetSearchText( int item ) const;

  protected:
    virtual void                OnKeyDown( wxKeyEvent &event );
    virtual void                DrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const;
    virtual void                DrawBackground( wxDC &dc, const wxRect &rect, const int row, const int col ) const;
    virtual void                CreateContextMenu( wxMenu * Menu ) const;
    virtual wxString            OnGetItemText( const int row, const int column ) const;
    virtual void                GetItemsList( void );
    virtual void                OnMouse( wxMouseEvent &event );

    void                        OnConfigUpdated( wxCommandEvent &event );

  public :
    guPlayList( wxWindow * parent, guDbLibrary * db, guPlayerPanel * playerpanel );
    ~guPlayList();

    void                        AddItem( const guTrack &NewItem );
    void                        AddItem( const guTrack * NewItem );
    void                        AddPlayListItem( const wxString &FileName, bool AddPath = false );

    virtual void                ReloadItems( bool reset = true );

    virtual int inline          GetItemId( const int row ) const;
    virtual wxString inline     GetItemName( const int row ) const;


    guTrack *                   GetItem( size_t item );
    long                        GetCount();
    guTrack *                   GetCurrent();
    int                         GetCurItem();
    void                        SetCurrent( int curitem, bool delold = false );
    guTrack *                   GetNext( bool bLoop );
    guTrack *                   GetPrev( bool bLoop );
    void                        ClearItems();
    long                        GetLength( void ) const;
    wxString                    GetLengthStr( void ) const;
    void                        AddToPlayList( const guTrackArray &NewItems, const bool DeleteOld = false );
    void                        SetPlayList( const guTrackArray &NewItems );
    wxString                    FindCoverFile( const wxString &DirName );
    void                        Randomize( void );
    int                         GetCaps();
    void                        RemoveItem( int itemnum );
    void                        UpdatedTracks( const guTrackArray * tracks );
    void                        UpdatedTrack( const guTrack * track );
    bool                        StartPlaying( void ) { return m_StartPlaying; }


  friend class guAddDropFilesThread;
  friend class guPlayListDropTarget;

};

class guPlayListDropTarget;

#endif // PLAYLIST_H
// -------------------------------------------------------------------------------- //
