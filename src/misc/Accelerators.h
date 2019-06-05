// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2019 J.Rios anonbeat@gmail.com
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
#ifndef __ACCELERATORS_H__
#define __ACCELERATORS_H__

#include <wx/wx.h>

namespace Guayadeque {

void        guAccelInit( void );
void        guAccelOnConfigUpdated( void );
int         guAccelGetActionNames( wxArrayString &actionnames );
int         guAccelGetDefaultKeys( wxArrayInt &accelkeys );
wxString    guAccelGetKeyCodeString( const int keycode );
wxString    guAccelGetKeyCodeMenuString( const int keycode );
wxString    guAccelGetCommandKeyCodeString( const int cmd );
int         guAccelGetCommandKeyCode( const int cmd );
int         guAccelDoAcceleratorTable( const wxArrayInt &aliascmds, const wxArrayInt &realcmds, wxAcceleratorTable &acceltable );

}

#endif
// -------------------------------------------------------------------------------- //
