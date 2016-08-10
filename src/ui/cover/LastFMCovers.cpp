// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2016 J.Rios anonbeat@gmail.com
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
#include "LastFMCovers.h"

#include "LastFM.h"
#include "Utils.h"

#include <wx/string.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
guLastFMCoverFetcher::guLastFMCoverFetcher( guFetchCoverLinksThread * mainthread, guArrayStringArray * coverlinks,
                                    const wxChar * artist, const wxChar * album ) :
    guCoverFetcher( mainthread, coverlinks, artist, album )
{
}

// -------------------------------------------------------------------------------- //
int guLastFMCoverFetcher::AddCoverLinks( int pagenum )
{
    guLastFM * LastFM;
    wxString AlbumName;
    guAlbumInfo AlbumInfo;

    if( pagenum > 0 )
        return 0;

    LastFM = new guLastFM();
    if( LastFM )
    {
        // Remove from album name expressions like (cd1),(cd2) etc
        AlbumName = RemoveSearchFilters( m_Album );

        AlbumInfo = LastFM->AlbumGetInfo( m_Artist, AlbumName );

        if( LastFM->IsOk() )
        {
            if( !AlbumInfo.m_ImageLink.IsEmpty() )
            {
                wxArrayString ImageInfo;
                ImageInfo.Add( AlbumInfo.m_ImageLink );
                ImageInfo.Add( wxEmptyString );
                m_CoverLinks->Add( ImageInfo );
            }
        }
        else
        {
            // There was en error...
            guLogError( wxT( "Error getting the cover for %s - %s (%u)" ),
                     m_Artist.c_str(),
                     AlbumName.c_str(),
                     LastFM->GetLastError() );
        }
        delete LastFM;
    }

    return 1;
}

}

// -------------------------------------------------------------------------------- //
