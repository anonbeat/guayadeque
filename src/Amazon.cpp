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
#include "Amazon.h"

#include "CoverEdit.h"
#include "Utils.h"

#include <wx/arrimpl.cpp>
#include <wx/curl/http.h>
#include <wx/statline.h>


// -------------------------------------------------------------------------------- //
guAmazonCoverFetcher::guAmazonCoverFetcher( guFetchCoverLinksThread * mainthread, guArrayStringArray * coverlinks,
                                    const wxChar * artist, const wxChar * album ) :
    guCoverFetcher( mainthread, coverlinks, artist, album )
{
}

//url = "http://" + _supportedLocales[locale][1] + "/onca/xml?Service=AWSECommerceService"
//    url += "&AWSAccessKeyId=%s" % license_key.strip()
//    url += "&Operation=ItemSearch"
//    url += "&SearchIndex=Music"
//    if artist and len(artist):
//        url += "&Artist=%s" % (urllib.quote(artist))
//    if album and len(album):
//        url += "&Keywords=%s" % (urllib.quote(album))
//    # just return the image information
//    url += "&ResponseGroup=Images,Small"

// -------------------------------------------------------------------------------- //
int guAmazonCoverFetcher::AddCoverLinks( int pagenum )
{
}

// -------------------------------------------------------------------------------- //
