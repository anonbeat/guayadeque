// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2023 J.Rios anonbeat@gmail.com
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
#ifndef __RALISTBOX_H__
#define __RALISTBOX_H__

#include <wx/wx.h>

#include "AccelListBox.h"

namespace Guayadeque {

class guLibPanel;

// -------------------------------------------------------------------------------- //
class guRaListBox : public guAccelListBox
{
  protected :
    guLibPanel *    m_LibPanel;

    wxBitmap *      m_NormalStar;
    wxBitmap *      m_SelectStar;

    virtual void    GetItemsList( void );
    virtual void    CreateContextMenu( wxMenu * menu ) const;
    wxString        GetSearchText( int Item ) const;

    virtual void    DrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const;

    virtual void    CreateAcceleratorTable( void );

  public :
                    guRaListBox( wxWindow * parent, guLibPanel * libpanel, guDbLibrary * db, const wxString &label );
                    ~guRaListBox();
    virtual int     GetSelectedSongs( guTrackArray * songs, const bool isdrag = false ) const;

};

}

#endif
// -------------------------------------------------------------------------------- //
