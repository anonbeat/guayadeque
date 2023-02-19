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
#include "AccelListBox.h"

#include "Accelerators.h"
#include "Config.h"
#include "LibPanel.h"
#include "Preferences.h"

using namespace Guayadeque;

// -------------------------------------------------------------------------------- //
guAccelListBox::guAccelListBox( wxWindow * parent, guDbLibrary * db, const wxString &label ) :
    guListBox( parent, db, label, wxLB_MULTIPLE | guLISTVIEW_ALLOWDRAG | guLISTVIEW_HIDE_HEADER )
{

    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->RegisterObject( this );

    Bind( guConfigUpdatedEvent, &guAccelListBox::OnConfigUpdated, this, ID_CONFIG_UPDATED );

    CreateAcceleratorTable();
}

// -------------------------------------------------------------------------------- //
guAccelListBox::~guAccelListBox()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->UnRegisterObject( this );

    Unbind( guConfigUpdatedEvent, &guAccelListBox::OnConfigUpdated, this, ID_CONFIG_UPDATED );
}

// -------------------------------------------------------------------------------- //
void guAccelListBox::OnConfigUpdated( wxCommandEvent &event )
{
    int Flags = event.GetInt();
    if( Flags & guPREFERENCE_PAGE_FLAG_ACCELERATORS )
    {
        CreateAcceleratorTable();
    }
}

// -------------------------------------------------------------------------------- //
