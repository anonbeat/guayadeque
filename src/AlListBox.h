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
#ifndef GUALLISTBOX_H
#define GUALLISTBOX_H

#include <wx/listctrl.h>
#include <wx/scrolwin.h>
#include <wx/vlbox.h>

#include "DbLibrary.h"

class guAlbumListBox;
class guAlbumListBoxHeader;

class guAlListBox : public wxScrolledWindow
{
  private :
    guAlbumListBoxHeader * m_Header;
    guAlbumListBox *       m_ListBox;

    void            OnChangedSize( wxSizeEvent &event );
    void            OnAlbumListActivated( wxListEvent &event );
    void            OnAlbumListSelected( wxListEvent &event );

  public :
                    guAlListBox( wxWindow * parent, DbLibrary * db, const wxString &label );
                    ~guAlListBox();
    bool            SelectAlbumName( const wxString &AlbumName );
    void            ReloadItems( const bool reset = true );
    wxArrayInt      GetSelection() const;
    int             GetSelectedSongs( guTrackArray * tracks ) const;

};

#endif
// -------------------------------------------------------------------------------- //
