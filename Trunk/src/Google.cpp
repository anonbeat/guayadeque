// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2010 J.Rios
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
#include "Google.h"

#include "CoverEdit.h"
#include "Utils.h"

#include <wx/arrimpl.cpp>
#include <wx/curl/http.h>
#include <wx/statline.h>
#include <wx/html/htmlpars.h>

//#define GOOGLE_IMAGES_SEARCH_URL    wxT( "http://images.google.com/images?imgsz=large|xlarge&q=%s&start=%u" )
#define GOOGLE_IMAGES_SEARCH_URL    wxT( "http://images.google.com/images?&q=%s&sout=1&start=%u" )

#define GOOGLE_COVERS_PER_PAGE      15
#define GOOGLE_COVERINFO_LINK       3           // 3 -> Link
#define GOOGLE_COVERINFO_COMMENT    6           // 6 -> Comment
#define GOOGLE_COVERINFO_SIZE       9           // 9 -> Size >> 425 x 283 - 130 KB

// -------------------------------------------------------------------------------- //
guGoogleCoverFetcher::guGoogleCoverFetcher( guFetchCoverLinksThread * mainthread, guArrayStringArray * coverlinks,
                                    const wxChar * artist, const wxChar * album ) :
    guCoverFetcher( mainthread, coverlinks, artist, album )
{
}

// -------------------------------------------------------------------------------- //
wxArrayString guGoogleCoverFetcher::ExtractImageInfo( const wxString &content )
{
    wxArrayString RetVal;
    wxString CurParam;
    CurParam = wxEmptyString;
    wxChar CurChar;
    int index;
    int count = content.Length();
    for( index = 0; index < count; index++ )
    {
        CurChar = content[ index ];
        if( CurChar == wxT( '\"' ) )
        {
            index++;
            while( ( CurChar = content[ index ] ) != wxT( '\"' ) && ( index < count ) )
            {
                CurParam.Append( CurChar );
                index++;
            }
        }
        else if( CurChar == wxT( ',' ) /*|| CurChar == wxT( ')' )*/ )
        {
            //guLogMessage( wxT( "%i= '%s'" ), RetVal.Count(), CurParam.c_str() );
            RetVal.Add( CurParam );
            CurParam = wxEmptyString;
        }
        else
        {
            CurParam.Append( CurChar );
        }
    }
    if( !CurParam.IsEmpty() )
        RetVal.Add( CurParam );
    //guLogMessage( wxT( "ImageLink: %s" ), RetVal[ 3 ].c_str() );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
wxString ExtractCoverFromGoogleLink( wxString &link )
{
    int StrPos = link.Find( wxT( "/imgres?imgurl\\x3d" ) );
    if( StrPos != wxNOT_FOUND )
    {
        StrPos += 18;
        wxString RetVal = link.Mid( StrPos );
        StrPos = RetVal.Find( wxT( "\\x26imgrefurl" ) );
        if( StrPos != wxNOT_FOUND )
        {
            return RetVal.Mid( 0, StrPos );
        }
    }
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
int guGoogleCoverFetcher::ExtractImagesInfo( wxString &content, int count )
{
    wxArrayString CurImage;
    wxArrayString GoogleImage;
    int ImageIndex = 0;

    int StrPos = content.Find( wxT( "dyn.setResults([[" ) );

    if( StrPos != wxNOT_FOUND )
        StrPos += 14;
    //guLogMessage( wxT( "Content:\n%s" ), Content.c_str() );
    while( ( StrPos != wxNOT_FOUND ) && !m_MainThread->TestDestroy() )
    {
        content = content.Mid( StrPos + 3 );
        StrPos = content.Find( wxT( "],[" ) );
        if( StrPos == wxNOT_FOUND )
          return 0; //break;
        //guLogMessage( wxT( "%s" ), Content.Mid( 0, StrPos ).c_str() );
        wxHtmlEntitiesParser EntitiesParser;
        GoogleImage = ExtractImageInfo( EntitiesParser.Parse( content.Mid( 0, StrPos ) ) );
        if( GoogleImage.Count() >= GOOGLE_COVERINFO_SIZE )
        {
            //RetVal.Add( CurImage );
            CurImage.Empty();
            if( GoogleImage[ GOOGLE_COVERINFO_LINK ].IsEmpty() )
            {
                GoogleImage[ GOOGLE_COVERINFO_LINK ] = ExtractCoverFromGoogleLink( GoogleImage[ 0 ] );
            }

            if( !GoogleImage[ GOOGLE_COVERINFO_LINK ].IsEmpty() )
            {
                CurImage.Add( GoogleImage[ GOOGLE_COVERINFO_LINK ] );
                CurImage.Add( GoogleImage[ GOOGLE_COVERINFO_SIZE ] );
                m_CoverLinks->Add( CurImage );
                ImageIndex++;
                if( ImageIndex == count )
                    break;
            }
        }
        //guLogMessage( wxT( "Pos: %u" ), StrPos );
    }
    return ImageIndex;
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
        //printf( "Buffer:\n%s\n", Buffer );
        wxString Content = GetUrlContent( SearchUrl );
        if( Content.Length() )
        {
            if( !m_MainThread->TestDestroy() )
            {
                //guLogMessage( Content );
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
