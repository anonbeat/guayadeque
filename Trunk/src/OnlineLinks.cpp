// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2011 J.Rios
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
#include "OnlineLinks.h"

#include "Commands.h"
#include "Config.h"
#include "Images.h"

#include <wx/uri.h>


// -------------------------------------------------------------------------------- //
void AddOnlineLinksMenu( wxMenu * Menu )
{
    wxMenu * SubMenu;
    int index;
    int count;
    wxMenuItem * MenuItem;
    if( Menu )
    {
        SubMenu = new wxMenu();
        wxASSERT( SubMenu );

        guConfig * Config = ( guConfig * ) guConfig::Get();
        wxArrayString Links = Config->ReadAStr( wxT( "Link" ), wxEmptyString, wxT( "SearchLinks" ) );
        wxArrayString Names = Config->ReadAStr( wxT( "Name" ), wxEmptyString, wxT( "SearchLinks" ) );
        if( ( count = Links.Count() ) )
        {
            for( index = 0; index < count; index++ )
            {
                wxURI Uri( Links[ index ] );
                MenuItem = new wxMenuItem( Menu, ID_LASTFM_SEARCH_LINK + index, Names[ index ], Links[ index ] );
                wxString IconFile = wxGetHomeDir() + wxT( "/.guayadeque/LinkIcons/" ) + Uri.GetServer() + wxT( ".ico" );
                if( wxFileExists( IconFile ) )
                {
                    MenuItem->SetBitmap( wxBitmap( IconFile, wxBITMAP_TYPE_ICO ) );
                }
                else
                {
                    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_search ) );
                }
                SubMenu->Append( MenuItem );
            }
        }
        else
        {
            MenuItem = new wxMenuItem( Menu, -1, _( "No search link defined" ), _( "Add search links in preferences" ) );
            SubMenu->Append( MenuItem );
        }
        Menu->AppendSubMenu( SubMenu, _( "Links" ) );
    }
}

// -------------------------------------------------------------------------------- //
