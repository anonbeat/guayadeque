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
#include "PLSoListBox.h"

#include "Config.h" // Configuration
#include "Commands.h"
#include "Images.h"
#include "MainApp.h"
#include "OnlineLinks.h"
#include "PlayList.h" // LenToString
#include "Utils.h"
#include "RatingCtrl.h"

// -------------------------------------------------------------------------------- //
guPLSoListBox::guPLSoListBox( wxWindow * parent, DbLibrary * db, wxString confname, int style ) :
             guSoListBox( parent, db, confname, style )
{
    m_PLId = wxNOT_FOUND;
    m_PLType = wxNOT_FOUND;
    ReloadItems();
}

// -------------------------------------------------------------------------------- //
guPLSoListBox::~guPLSoListBox()
{
}

// -------------------------------------------------------------------------------- //
void guPLSoListBox::GetItemsList( void )
{
    if( m_PLId > 0 )
    {
        m_Db->GetPlayListSongs( m_PLId, m_PLType, &m_Items );
    }
    else
        m_Items.Empty();
}

// -------------------------------------------------------------------------------- //
void guPLSoListBox::SetPlayList( int plid, int pltype )
{
    m_PLId = plid;
    m_PLType = pltype;
    ReloadItems();
}

// -------------------------------------------------------------------------------- //
void guPLSoListBox::CreateContextMenu( wxMenu * Menu ) const
{
    wxMenuItem * MenuItem;
    int SelCount = GetSelectedItems().Count();

    if( SelCount )
    {
        MenuItem = new wxMenuItem( Menu, ID_SONG_DELETE, _( "Delete" ), _( "Delete the current selected tracks" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_del ) );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();
    }

    guSoListBox::CreateContextMenu( Menu );
}

// -------------------------------------------------------------------------------- //
