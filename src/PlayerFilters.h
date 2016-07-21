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
#ifndef __PLAYERFILTERS_H__
#define __PLAYERFILTERS_H__

#include "DbLibrary.h"

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/choice.h>
#include <wx/sizer.h>
#include <wx/panel.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
class guPlayerFilters : public wxPanel
{
  protected :
	wxChoice *      m_FilterAllowChoice;
	wxChoice *      m_FilterDenyChoice;

    guListItems     m_FilterPlayLists;
    guDbLibrary *   m_Db;


  public :
    guPlayerFilters( wxWindow * parent, guDbLibrary * db );
    ~guPlayerFilters();

    void            UpdateFilters( void );
    void            EnableFilters( const bool enable );
    int             GetAllowSelection( void );
    int             GetDenySelection( void );
    int             GetAllowFilterId( void );
    int             GetDenyFilterId( void );
    void            SetAllowFilterId( const int id );
    void            SetDenyFilterId( const int id );

};

}

#endif
// -------------------------------------------------------------------------------- //
