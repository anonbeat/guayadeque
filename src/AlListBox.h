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

#include "DbLibrary.h"
#include "ItemListBox.h"

// -------------------------------------------------------------------------------- //
class guAlListBox : public  guListView
{
  private :
    DbLibrary * m_Db;
    guAlbumItems * m_Items;

    void                OnAlbumListActivated( wxListEvent &event );
    void                OnAlbumListSelected( wxListEvent &event );
    virtual void        DrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const;
    wxCoord             OnMeasureItem( size_t n ) const;
    virtual void        CreateContextMenu( wxMenu * menu ) const;

    virtual void        GetItemsList( void );
    void                OnSearchLinkClicked( wxCommandEvent &event );
    void                OnCommandClicked( wxCommandEvent &event );
    wxString            GetSearchText( int item ) const;

    virtual int         GetDragFiles( wxFileDataObject * files );

  public :
                        guAlListBox( wxWindow * parent, DbLibrary * db, const wxString &label );
                        ~guAlListBox();
    bool                SelectAlbumName( const wxString &AlbumName );
//    void                ReloadItems( const bool reset = true );
//    wxArrayInt          GetSelection() const;
    virtual int             GetSelectedSongs( guTrackArray * tracks ) const;
    virtual wxString inline GetItemName( const int item ) const;
    virtual int inline      GetItemId( const int item ) const;
    virtual void            ReloadItems( bool reset = true );
    virtual void            SetSelectedItems( const wxArrayInt &selection );


};

#endif
// -------------------------------------------------------------------------------- //
