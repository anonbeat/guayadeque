// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2016 J.Rios anonbeat@gmail.com
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
//    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#ifndef GUAALISTBOX_H
#define GUAALISTBOX_H

#include <wx/wx.h>

#include "AccelListBox.h"

class guLibPanel;

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
class guAAListBox : public guAccelListBox
{
  protected :
    guLibPanel *    m_LibPanel;

    virtual void    GetItemsList( void );
    virtual void    CreateContextMenu( wxMenu * menu ) const;

    void            OnSearchLinkClicked( wxCommandEvent &event );
    void            OnCommandClicked( wxCommandEvent &event );
    wxString        GetSearchText( int Item ) const;

    virtual void    CreateAcceleratorTable( void );

  public :
                    guAAListBox( wxWindow * parent, guLibPanel * libpanel, guDbLibrary * db, const wxString &label );
                    ~guAAListBox();
    virtual int     GetSelectedSongs( guTrackArray * songs, const bool isdrag = false ) const;

    int             FindAlbumArtist( const wxString &albumartist );

};

}

#endif
// -------------------------------------------------------------------------------- //

