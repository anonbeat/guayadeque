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
#ifndef GUSOLISTBOX_H
#define GUSOLISTBOX_H

#include <wx/wx.h>

#include "DbLibrary.h"

#define guSONGS_COLUMN_NUMBER       0
#define guSONGS_COLUMN_TITLE        1
#define guSONGS_COLUMN_ARTIST       2
#define guSONGS_COLUMN_ALBUM        3
#define guSONGS_COLUMN_LENGTH       4
#define guSONGS_COLUMN_YEAR         5
#define guSONGS_COLUMN_BITRATE      6
#define guSONGS_COLUMN_RATING       7
#define guSONGS_COLUMN_PLAYCOUNT    8
#define guSONGS_COLUMN_LASTPLAY     9
#define guSONGS_COLUMN_ADDEDDATE    10

// -------------------------------------------------------------------------------- //
class guSoListBox : public wxListCtrl
{
  protected :
    DbLibrary *         m_Db;
    wxArrayInt          m_Columns;
    guTrackArray        m_Songs;

    wxString            m_ConfName;
    wxListItemAttr      m_OddAttr;
    wxListItemAttr      m_EveAttr;
    int                 m_PrevColSize;

    wxString            OnGetItemText( long item, long column ) const;
    wxListItemAttr *    OnGetItemAttr( long item ) const;
    void                OnBeginDrag( wxMouseEvent &event );
    void                OnContextMenu( wxContextMenuEvent& event );
    void                OnSearchLinkClicked( wxCommandEvent &event );
    void                OnCommandClicked( wxCommandEvent &event );

    wxString            GetSearchText( int Item );
    virtual void        FillTracks();

  public :
                        guSoListBox( wxWindow * parent, DbLibrary * NewDb, wxString confname );
                        ~guSoListBox();
    void                ReloadItems();
    wxArrayInt          GetSelection( void ) const;
    guTrackArray        GetSelectedSongs() const;
    guTrackArray        GetAllSongs() const;
    void                ClearSelection();

};

#endif
// -------------------------------------------------------------------------------- //
