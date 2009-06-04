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

#include <wx/wx.h>
#include <wx/menu.h>

#include "DbLibrary.h"

class guItemListBoxTimer;

// -------------------------------------------------------------------------------- //
class guListBox : public wxListCtrl
{
    protected :
      DbLibrary *           m_Db;
      guListItems           m_Items;
      wxListItemAttr        m_EveAttr;
      wxListItemAttr        m_OddAttr;
      wxString              m_Label;
      wxString              m_SearchStr;
      guItemListBoxTimer *  m_SearchStrTimer;

      virtual wxString          OnGetItemText( long item, long column ) const;
      virtual wxListItemAttr *  OnGetItemAttr( long item ) const;
      virtual void              OnChangedSize( wxSizeEvent &event );
      virtual void              GetItemsList( void ) = 0;
      virtual void              OnBeginDrag( wxMouseEvent &event );
      void                      AdjustColumnWidth( int width );
      void                      OnContextMenu( wxContextMenuEvent& event );
      virtual void              GetContextMenu( wxMenu * menu ) const = 0;
      void                      ShowContextMenu( const wxPoint & pos );
      void                      OnKeyDown( wxListEvent &event );

    public :
      guListBox( wxWindow * parent, DbLibrary * db, wxString label = wxEmptyString );
      ~guListBox();

      virtual void              ReloadItems( bool reset = true );
      wxArrayInt                GetSelection( bool reallist = false ) const;
      void                      SetSelection( const wxArrayInt selection );
      void                      ClearSelection();
      virtual int               GetSelectedSongs( guTrackArray * songs ) const = 0;
      virtual wxString          GetItemText( long item ) const;
      virtual int               GetItemData( long item ) const;

    friend class guItemListBoxTimer;
};

// -------------------------------------------------------------------------------- //
class guItemListBoxTimer : public wxTimer
{
public:
    //Ctor
    guItemListBoxTimer( guListBox * listbox ) { m_ItemListBox = listbox; }

    //Called each time the timer's timeout expires
    void Notify(); // { ItemListBox->SearchStr = wxEmptyString; };

    guListBox * m_ItemListBox;       //
};

#endif
// -------------------------------------------------------------------------------- //
