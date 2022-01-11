// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2022 J.Rios anonbeat@gmail.com
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
//    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
//    Boston, MA 02110-1301 USA.
//
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#ifndef __TVSOLISTBOX_H__
#define __TVSOLISTBOX_H__

#include "SoListBox.h"
#include "TreeViewFilter.h"

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
class guTVSoListBox : public guSoListBox
{
  protected :
    guTreeViewFilterArray   m_Filters;

    wxLongLong              m_TracksSize;
    wxLongLong              m_TracksLength;

    wxArrayString           m_TextFilters;

    virtual void        GetItemsList( void );
    virtual wxString    GetSearchText( int item ) const;


    virtual void        ItemsCheckRange( const int start, const int end ) { m_ItemsFirst = 0; m_ItemsLast = 0; }

    virtual void        CreateAcceleratorTable();

  public :
    guTVSoListBox( wxWindow * parent, guMediaViewer * mediaviewer, wxString confname, int style = 0 );
    ~guTVSoListBox();

    void                SetFilters( guTreeViewFilterArray &filters );

    virtual int         GetSelectedSongs( guTrackArray * Songs, const bool isdrag = false ) const;
    virtual void        GetAllSongs( guTrackArray * Songs );

    virtual int         GetItemId( const int row ) const;
    virtual wxString    GetItemName( const int row ) const;

    void                GetCounters( wxLongLong * count, wxLongLong * len, wxLongLong * size );

    virtual void        SetTracksOrder( const int order )
    {
        if( m_TracksOrder != order )
            m_TracksOrder = order;
        else
            m_TracksOrderDesc = !m_TracksOrderDesc;
    }

    void                SetTextFilters( const wxArrayString &textfilters ) { m_TextFilters = textfilters; }
    void                ClearTextFilters( void ) { m_TextFilters.Clear(); }
};

}

#endif
// -------------------------------------------------------------------------------- //
