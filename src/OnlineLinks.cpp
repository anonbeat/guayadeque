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
#include "OnlineLinks.h"

#include "Commands.h"
#include "Config.h"
#include "Images.h"
#include "MainApp.h"
#include "Settings.h"
#include "Utils.h"

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

        guConfig * Config = ( guConfig * ) guConfig::Get();
        wxArrayString Links = Config->ReadAStr( wxT( "Link" ), wxEmptyString, wxT( "searchlinks/links" ) );
        wxArrayString Names = Config->ReadAStr( wxT( "Name" ), wxEmptyString, wxT( "searchlinks/names" ) );
        if( ( count = Links.Count() ) )
        {
            for( index = 0; index < count; index++ )
            {
                wxURI Uri( Links[ index ] );
                MenuItem = new wxMenuItem( Menu, ID_LINKS_BASE + index, Names[ index ], Links[ index ] );
                wxString IconFile = guPATH_LINKICONS + Uri.GetServer() + wxT( ".ico" );
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

            SubMenu->AppendSeparator();
        }
        else
        {
            MenuItem = new wxMenuItem( Menu, ID_MENU_PREFERENCES_LINKS, _( "Preferences" ), _( "Add search links in preferences" ) );
            SubMenu->Append( MenuItem );
        }
        Menu->AppendSubMenu( SubMenu, _( "Links" ) );
    }
}

// -------------------------------------------------------------------------------- //
void ExecuteOnlineLink( const int linkid, const wxString &text )
{
    int index = linkid - ID_LINKS_BASE;
    //guLogMessage( wxT( "ExecuteOnlineLink( %i, '%s' )" ), index, text.c_str() );

    guConfig * Config = ( guConfig * ) guConfig::Get();
    wxArrayString Links = Config->ReadAStr( wxT( "Link" ), wxEmptyString, wxT( "searchlinks/links" ) );

    if( index >= 0 && ( index < ( int ) Links.Count() ) )
    {
        wxString SearchLink = Links[ index ];
        wxString Lang = Config->ReadStr( wxT( "Language" ), wxT( "en" ), wxT( "lastfm" ) );
        if( Lang.IsEmpty() )
        {
            Lang = ( ( guMainApp * ) wxTheApp )->GetLocale()->GetCanonicalName().Mid( 0, 2 );
            //guLogMessage( wxT( "Locale: %s" ), ( ( guMainApp * ) wxTheApp )->GetLocale()->GetCanonicalName().c_str() );
        }
        SearchLink.Replace( wxT( "{lang}" ), Lang );
        SearchLink.Replace( wxT( "{text}" ), guURLEncode( text ) );
        guWebExecute( SearchLink );
    }
    else
    {
        guLogMessage( wxT( "Online Link out of rante %i" ), index );
    }
}

// -------------------------------------------------------------------------------- //
