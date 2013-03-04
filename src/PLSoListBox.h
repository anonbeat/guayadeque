// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2013 J.Rios
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
#ifndef GUPLSOLISTBOX_H
#define GUPLSOLISTBOX_H

#include "SoListBox.h"

// -------------------------------------------------------------------------------- //
class guPLSoListBox : public guSoListBox
{
  protected :
    wxArrayInt          m_PLIds;    // The PlayLists ids
    wxArrayInt          m_PLTypes;  // The PlayLists types
    wxArrayInt          m_PLSetIds; // The array containing the list of plset_id
    wxArrayInt          m_DropIds;  // The array containing the id of the songs dropped
    wxLongLong          m_TracksSize;
    wxLongLong          m_TracksLength;
    bool                m_DisableSorting;

    virtual void        GetItemsList( void );
    virtual void        CreateContextMenu( wxMenu * Menu ) const;

    virtual void        OnKeyDown( wxKeyEvent &event );

    virtual void        OnDropFile( const wxString &filename );
    virtual void        OnDropEnd( void );
    virtual void        MoveSelection( void );

    virtual void        ItemsCheckRange( const int start, const int end ) { m_ItemsFirst = 0; m_ItemsLast = 0; }

    virtual void        CreateAcceleratorTable();
    virtual wxString    GetSearchText( int item ) const;

    void                OnRandomizeTracks( wxCommandEvent &event ) { RandomizeTracks(); }

  public :
    guPLSoListBox( wxWindow * parent, guMediaViewer * mediaviewer, wxString confname, int style = 0 );
    ~guPLSoListBox();

    void                SetPlayList( int plid, int pltype );
    void                SetPlayList( const wxArrayInt &ids, const wxArrayInt &types );
    int                 GetPlayListSetIds( wxArrayInt * setids ) const;

    virtual int         GetSelectedSongs( guTrackArray * Songs, const bool isdrag = false ) const;
    virtual void        GetAllSongs( guTrackArray * Songs );

    virtual int         GetItemId( const int row ) const;
    virtual wxString    GetItemName( const int row ) const;

    void                GetCounters( wxLongLong * count, wxLongLong * len, wxLongLong * size )
    {
        * count = GetItemCount();
        * len   = m_TracksLength;
        * size  = m_TracksSize;
    }

    virtual void        SetTracksOrder( const int order );

    virtual void        RandomizeTracks( void );

};

#endif
// -------------------------------------------------------------------------------- //
