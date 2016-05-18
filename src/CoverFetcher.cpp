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
#include "CoverFetcher.h"

// -------------------------------------------------------------------------------- //
wxString ExtractString( const wxString &source, const wxString &start, const wxString &end )
{
    int StartPos = source.Find( start );
    int EndPos;
    if( StartPos != wxNOT_FOUND )
    {
        wxString SearchStr = source.Mid( StartPos + start.Length() );
        EndPos = SearchStr.Find( end );
        if( EndPos != wxNOT_FOUND )
        {
            return SearchStr.Mid( 0, EndPos );
        }
    }
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
// guCoverFetcher
// -------------------------------------------------------------------------------- //
guCoverFetcher::guCoverFetcher( guFetchCoverLinksThread * mainthread, guArrayStringArray * coverlinks,
                                      const wxChar * artist, const wxChar * album )
{
    m_MainThread = mainthread;
    m_CoverLinks = coverlinks;
    m_Artist = wxString( artist );
    m_Album = wxString( album );
}

// -------------------------------------------------------------------------------- //
guCoverFetcher::~guCoverFetcher()
{
}

// -------------------------------------------------------------------------------- //
