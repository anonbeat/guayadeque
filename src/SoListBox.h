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

#include "DbLibrary.h"
#include "ListView.h"
#include "StatusBar.h"

#define guSONGS_COLUMN_NUMBER       0
#define guSONGS_COLUMN_TITLE        1
#define guSONGS_COLUMN_ARTIST       2
#define guSONGS_COLUMN_ALBUM        3
#define guSONGS_COLUMN_GENRE        4
#define guSONGS_COLUMN_LENGTH       5
#define guSONGS_COLUMN_YEAR         6
#define guSONGS_COLUMN_BITRATE      7
#define guSONGS_COLUMN_RATING       8
#define guSONGS_COLUMN_PLAYCOUNT    9
#define guSONGS_COLUMN_LASTPLAY     10
#define guSONGS_COLUMN_ADDEDDATE    11

#define guSONGS_COLUMN_COUNT        12

// -------------------------------------------------------------------------------- //
class guSoListBox : public guListView
{
  protected :
    guDbLibrary *         m_Db;
    guTrackArray        m_Items;
    wxString            m_ConfName;

    wxBitmap *          m_GreyStar;
    wxBitmap *          m_YellowStar;

    virtual void                DrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const;
    virtual void                CreateContextMenu( wxMenu * Menu ) const;
    virtual wxString            OnGetItemText( const int row, const int column ) const;
    virtual void                GetItemsList( void );

    virtual int                 GetDragFiles( wxFileDataObject * files );

    void                        OnSearchLinkClicked( wxCommandEvent &event );
    void                        OnCommandClicked( wxCommandEvent &event );
    wxString                    GetSearchText( int item ) const;

    void                        OnItemColumnClicked( wxListEvent &event );

  public :
    guSoListBox( wxWindow * parent, guDbLibrary * NewDb, wxString confname, long style = 0 );
    ~guSoListBox();

    virtual void                ReloadItems( bool reset = true );

    virtual int                 GetSelectedSongs( guTrackArray * Songs ) const;
    virtual void                GetAllSongs( guTrackArray * Songs ) const;

    virtual int                 GetItemId( const int row ) const;
    virtual wxString            GetItemName( const int row ) const;

    virtual wxArrayString       GetColumnNames( void );

};

#endif
// -------------------------------------------------------------------------------- //
