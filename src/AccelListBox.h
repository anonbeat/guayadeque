// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2016 J.Rios
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
#ifndef ACCELLISTBOX_H
#define ACCELLISTBOX_H

#include <wx/wx.h>

#include "ItemListBox.h"

class guLibPanel;

// -------------------------------------------------------------------------------- //
class guAccelListBox : public guListBox
{
  protected :
    virtual void    OnConfigUpdated( wxCommandEvent &event );
    virtual void    CreateAcceleratorTable() {}

  public :
    guAccelListBox( wxWindow * parent, guDbLibrary * db, const wxString &label );
    ~guAccelListBox();

};

#endif
// -------------------------------------------------------------------------------- //
