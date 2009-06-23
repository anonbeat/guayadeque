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

#include <wx/wx.h>
#include <wx/vlbox.h>
#include <wx/dnd.h>
#include <wx/dir.h>
#include <wx/arrimpl.cpp>

#include "DbLibrary.h"

wxString LenToString( int Len );

class guPlayerPanel;

// -------------------------------------------------------------------------------- //
class guPlayList : public wxVListBox
{
  private :
    DbLibrary *     m_Db;
    guTrackArray    m_Items;
    long            m_CurItem;
    long            m_TotalLen;
    long            m_SmartPlayMaxPlayListTracks;

    size_t          m_DragOverItem;
    bool            m_DragOverAfter;
    bool            m_DragSelfItems;
    wxPoint         m_DragStart;
    int             m_DragCount;
    wxBitmap *      m_PlayBitmap;

    wxColor         m_PlayBgColor;
    wxColor         m_PlayFgColor;
    wxBrush         m_DragBgColor;
    wxColor         m_SelBgColor;
    wxColor         m_SelFgColor;
    wxColor         m_OddBgColor;
    wxColor         m_EveBgColor;
    wxColor         m_TextFgColor;
//        wxColor         m_SepColor;

    void            OnDragOver( const wxCoord x, const wxCoord y );
    void            OnDrawItem( wxDC &dc, const wxRect &rect, size_t n ) const;
    wxCoord         OnMeasureItem( size_t n ) const;
    void            OnDrawBackground( wxDC &dc, const wxRect &rect, size_t n ) const;
    void            OnKeyDown( wxKeyEvent &event );
    void            OnBeginDrag( wxMouseEvent &event );
    void            OnMouse( wxMouseEvent &event );
    void            RemoveSelected();
    void            MoveSelected();
    void            OnContextMenu( wxContextMenuEvent& event );


    void            OnClearClicked( wxCommandEvent &event );
    void            OnRemoveClicked( wxCommandEvent &event );
    void            OnSaveClicked( wxCommandEvent &event );
    void            OnCopyToClicked( wxCommandEvent &event );
    void            OnEditLabelsClicked( wxCommandEvent &event );
    void            OnCommandClicked( wxCommandEvent &event );

    DECLARE_EVENT_TABLE()

  public :
    guPlayList( wxWindow * parent, DbLibrary * db );
    ~guPlayList();

    void            AddItem( const guTrack &NewItem );
    void            AddItem( const guTrack * NewItem );
    void            AddPlayListItem( const wxString &FileName, bool AddPath = false );

    void            RefreshItems();
    void            UpdateView( bool Scroll = true );

    guTrack *       GetItem( size_t item );
    long            GetCount();
    guTrack *       GetCurrent();
    int             GetCurItem();
    void            SetCurrent( const int NewCurItem );
    guTrack *       GetNext( bool bLoop );
    guTrack *       GetPrev( bool bLoop );
    void            ClearItems();
    long            GetLength( void ) const;
    wxString        GetLengthStr( void ) const;
    void            AddToPlayList( const guTrackArray &NewItems, const bool DeleteOld = false );
    void            SetPlayList( const guTrackArray &NewItems );
    wxArrayInt      GetSelectedItems();
    wxString        FindCoverFile( const wxString &DirName );
    void            Randomize( void );
    int             GetCaps();
    void            RemoveItem( int itemnum );

  friend class guAddDropFilesThread;
  friend class guPlayListDropTarget;

};

class guPlayListDropTarget;

// -------------------------------------------------------------------------------- //
class guAddDropFilesThread : public wxThread
{
  private:
    guPlayList *            m_PlayList;
    guPlayListDropTarget *  m_PlayListDropTarget;
    wxSortedArrayString     m_Files;

  public:
    guAddDropFilesThread( guPlayListDropTarget * playlistdroptarget,
                                 guPlayList * playlist, const wxArrayString &files );
    ~guAddDropFilesThread();

    void AddDropFiles( const wxString &DirName );
    virtual ExitCode Entry();
};

// -------------------------------------------------------------------------------- //
class guPlayListDropTarget : public wxFileDropTarget
{
  private:
    guPlayList *            m_PlayList;
    guAddDropFilesThread *  m_AddDropFilesThread;

    void ClearAddDropFilesThread( void ) { m_AddDropFilesThread = NULL; };

  public:
    guPlayListDropTarget( guPlayList * NewPlayList );
    ~guPlayListDropTarget();

    virtual bool OnDropFiles( wxCoord WXUNUSED( x ), wxCoord WXUNUSED( y ), const wxArrayString &files );

    virtual wxDragResult OnDragOver( wxCoord x, wxCoord y, wxDragResult def );

    friend class guAddDropFilesThread;

};

#endif // PLAYLIST_H
// -------------------------------------------------------------------------------- //
