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
#ifndef GUPLSOLISTBOX_H
#define GUPLSOLISTBOX_H

#include "SoListBox.h"

// -------------------------------------------------------------------------------- //
class guPLSoListBox : public guSoListBox
{
  protected :
    int             m_PLId;     // The PlayList id
    int             m_PLType;   // The PlayList type
    wxArrayInt      m_PLSetIds; // The array containing the list of plset_id
    wxArrayInt      m_DropIds;  // The array containing the id of the songs dropped
    wxLongLong      m_TracksSize;
    wxLongLong      m_TracksLength;

    virtual void                GetItemsList( void );
    virtual void                CreateContextMenu( wxMenu * Menu ) const;

    virtual void                OnKeyDown( wxKeyEvent &event );

    virtual void                OnDropFile( const wxString &filename );
    virtual void                OnDropEnd( void );
    virtual void                MoveSelection( void );

  public :
    guPLSoListBox( wxWindow * parent, guDbLibrary * NewDb, wxString confname, int style = 0 );
    ~guPLSoListBox();

    void    SetPlayList( int plid, int pltype );
    int     GetPlayListSetIds( wxArrayInt * setids ) const;

    void    GetCounters( wxLongLong * count, wxLongLong * len, wxLongLong * size )
    {
        * count = GetItemCount();
        * len   = m_TracksLength;
        * size  = m_TracksSize;
    }

};

#endif
// -------------------------------------------------------------------------------- //
