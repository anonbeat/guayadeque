// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2012 J.Rios
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
#ifndef GUTREEVIEWFILTER_H
#define GUTREEVIEWFILTER_H

#include <wx/dynarray.h>

// -------------------------------------------------------------------------------- //
class guTreeViewFilterItem
{
  public :
    int         m_Id;
    int         m_Type;

    guTreeViewFilterItem( const int type, const int id ) { m_Id = id; m_Type = type; };

    int         Id( void ) { return m_Id; };
    void        Id( int id ) { m_Id = id; };
    int         Type( void ) { return m_Type; };
    void        Type( int type ) { m_Type = type; };
};

WX_DECLARE_OBJARRAY(guTreeViewFilterItem, guTreeViewFilterEntry);
WX_DECLARE_OBJARRAY(guTreeViewFilterEntry, guTreeViewFilterArray);

#endif
// -------------------------------------------------------------------------------- //
