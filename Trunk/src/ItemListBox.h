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
#ifndef GUITEMLISTBOX_H
#define GUITEMLISTBOX_H

#include "ListView.h"
#include "DbLibrary.h"

// -------------------------------------------------------------------------------- //
class guListBox : public guListView
{
  protected :
    DbLibrary *         m_Db;
    guListItems *       m_Items;

    virtual wxString    OnGetItemText( const int row, const int col ) const;
    void                OnBeginDrag( wxMouseEvent &event );
    virtual wxString    GetSearchText( const int item ) const;

  public :
    guListBox( wxWindow * parent, DbLibrary * db, const wxString &label = wxEmptyString );
    ~guListBox();

    virtual wxString inline GetItemName( const int item ) const;
    virtual int inline      GetItemId( const int item ) const;
    virtual void            ReloadItems( bool reset = true );

};

#endif
// -------------------------------------------------------------------------------- //
