// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2013 J.Rios
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
#include "Google.h"

#include "CoverEdit.h"
#include "Utils.h"

#include <wx/arrimpl.cpp>
#include <wx/curl/http.h>
#include <wx/statline.h>
#include <wx/html/htmlpars.h>

//#define GOOGLE_IMAGES_SEARCH_URL    wxT( "http://images.google.com/images?imgsz=large|xlarge&q=%s&start=%u" )
//#define GOOGLE_IMAGES_SEARCH_URL    wxT( "http://images.google.com/images?&q=%s&sout=1&start=%u" )
#define GOOGLE_IMAGES_SEARCH_URL    wxT( "http://ajax.googleapis.com/ajax/services/search/images?v=1.0&q=%s&rsz=8&start=%d" )
#define GOOGLE_COVERS_PER_PAGE      8

// -------------------------------------------------------------------------------- //
guGoogleCoverFetcher::guGoogleCoverFetcher( guFetchCoverLinksThread * mainthread, guArrayStringArray * coverlinks,
                                    const wxChar * artist, const wxChar * album ) :
    guCoverFetcher( mainthread, coverlinks, artist, album )
{
}

// -------------------------------------------------------------------------------- //
void guGoogleCoverFetcher::ExtractImageInfo( const wxString &content )
{
    //guLogMessage( wxT( "ExtractImageInfo: '%s'" ), content.c_str() );
    wxArrayString CurImageInfo;
    CurImageInfo.Add( ExtractString( content, wxT( "\"url\":\"" ), wxT( "\",\"" ) ) );
    wxString ImgInfo = ExtractString( content, wxT( "\"width\":\"" ), wxT( "\",\"" ) );
    if( !ImgInfo.IsEmpty() )
        ImgInfo += wxT( " x " ) + ExtractString( content, wxT( "\"height\":\"" ), wxT( "\",\"" ) );
    CurImageInfo.Add( ImgInfo );
    m_CoverLinks->Add( CurImageInfo );
}

// -------------------------------------------------------------------------------- //
int guGoogleCoverFetcher::ExtractImagesInfo( wxString &content, int count )
{
    int ImageIndex = 0;
    while( !m_MainThread->TestDestroy() )
    {
        //guLogMessage( wxT( "%s\n" ), content.c_str() );

        int FindPos = content.Find( wxT( "{\"GsearchResultClass\":" ) );
        if( FindPos == wxNOT_FOUND )
        {
            break;
        }
        content = content.Mid( FindPos );

        FindPos = content.Find( wxT( "}," ) );
        if( FindPos == wxNOT_FOUND )
            break;

        ExtractImageInfo( content.Mid( 0, FindPos + 1 ) );
        ImageIndex++;
        if( ImageIndex > count )
            break;

        content = content.Mid( FindPos + 1 );
        if( content.IsEmpty() )
            break;
    }

    return ( ImageIndex == GOOGLE_COVERS_PER_PAGE ) ? ImageIndex : 0;
}

// -------------------------------------------------------------------------------- //
int guGoogleCoverFetcher::AddCoverLinks( int pagenum )
{
    wxString SearchString = wxString::Format( wxT( "\"%s\" \"%s\"" ), m_Artist.c_str(), m_Album.c_str() );
    //guLogMessage( wxT( "URL: %u %s" ), m_CurrentPage, m_SearchString.c_str() );
    wxString SearchUrl = wxString::Format( GOOGLE_IMAGES_SEARCH_URL, guURLEncode( SearchString ).c_str(), ( pagenum * GOOGLE_COVERS_PER_PAGE ) );
    //guLogMessage( wxT( "URL: %u %s" ), pagenum, SearchUrl.c_str() );
    if( !m_MainThread->TestDestroy() )
    {
        wxString Content = GetUrlContent( SearchUrl );
        if( Content.Length() )
        {
            if( !m_MainThread->TestDestroy() )
            {
                //guLogMessage( wxT( "Google:====>>>>\n%s\n<<<<====" ), Content.c_str() );
                return ExtractImagesInfo( Content, GOOGLE_COVERS_PER_PAGE );
            }
        }
        else
        {
            guLogError( wxT( "Could not get the remote data from connection" ) );
        }
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
