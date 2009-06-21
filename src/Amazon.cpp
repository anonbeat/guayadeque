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

// see http://docs.amazonwebservices.com/AWSEcommerceService/2005-03-23/
//    http://webservices.amazon.com/onca/xml?Service=AWSECommerceService
//    &SubscriptionId=[YourSubscription IDHere]
//    &Operation=ItemSearch
//    &Artist=[The artist name]
//    &Keywords=[A Keywords String]
//    &SearchIndex=Music
//    &ItemPage=[The page Num {1..x}]

#define AMAZON_SEARCH_APIKEY    wxT( "AKIAI3VJGDYXLU7N2HKQ" )
#define AMAZON_SEARCH_URL       wxT( "http://webservices.amazon.com/onca/xml?Service=AWSECommerceService&SubscriptionId=" )\
                                AMAZON_SEARCH_APIKEY\
                                wxT( "&Operation=ItemSearch&SearchIndex=Music&ResponseGroup=Images,Small&Artist=%s&Keywords=%s&ItemPage=%u" )


// -------------------------------------------------------------------------------- //
guAmazonCoverFetcher::guAmazonCoverFetcher( guFetchCoverLinksThread * mainthread, guArrayStringArray * coverlinks,
                                    const wxChar * artist, const wxChar * album ) :
    guCoverFetcher( mainthread, coverlinks, artist, album )
{
}

// -------------------------------------------------------------------------------- //
wxArrayString guAmazonCoverFetcher::GetImageInfo( wxXmlNode * XmlNode )
{
    wxArrayString RetVal;
    wxString ImageUrl = wxEmptyString;
    wxString Height = wxEmptyString;
    wxString Width = wxEmptyString;
    wxXmlNode * XmlSubNode;
    XmlNode = XmlNode->GetChildren();
    while( XmlNode )
    {
        if( XmlNode->GetName() == wxT( "SmallImage" ) && ImageUrl.IsEmpty() )
        {
            XmlSubNode = XmlNode->GetChildren();
            while( XmlSubNode )
            {
                if( XmlSubNode->GetName() == wxT( "URL" ) )
                {
                    ImageUrl = XmlSubNode->GetNodeContent();
                }
                else if( XmlSubNode->GetName() == wxT( "Height" ) )
                {
                    Height = XmlSubNode->GetNodeContent();
                }
                else if( XmlSubNode->GetName() == wxT( "Width" ) )
                {
                    Width = XmlSubNode->GetNodeContent();
                }
                XmlSubNode = XmlSubNode->GetNext();
            }
        }
        else if( XmlNode->GetName() == wxT( "LargeImage" ) )
        {
            XmlSubNode = XmlNode->GetChildren();
            while( XmlSubNode )
            {
                if( XmlSubNode->GetName() == wxT( "URL" ) )
                {
                    ImageUrl = XmlSubNode->GetNodeContent();
                }
                else if( XmlSubNode->GetName() == wxT( "Height" ) )
                {
                    Height = XmlSubNode->GetNodeContent();
                }
                else if( XmlSubNode->GetName() == wxT( "Width" ) )
                {
                    Width = XmlSubNode->GetNodeContent();
                }
                XmlSubNode = XmlSubNode->GetNext();
            }
        }
        XmlNode = XmlNode->GetNext();
    }
    //guLogMessage( wxT( "Found cover %s x %s '%s'" ), Width.c_str(), Height.c_str(), ImageUrl.c_str() );
    if( !ImageUrl.IsEmpty() )
    {
        RetVal.Add( ImageUrl );
        RetVal.Add( Width + wxT( " x " ) + Height );
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
int guAmazonCoverFetcher::ExtractImagesInfo( wxString &content )
{
    int RetVal = 0;
    wxStringInputStream ins( content );
    wxXmlDocument XmlDoc( ins );
    wxXmlNode * XmlSubNode;
    wxXmlNode * XmlNode = XmlDoc.GetRoot();
    if( XmlNode && XmlNode->GetName() == wxT( "ItemSearchResponse" ) )
    {
        XmlNode = XmlNode->GetChildren();
        while( XmlNode )
        {
            if( XmlNode->GetName() == wxT( "Items" ) )
            {
                XmlNode = XmlNode->GetChildren();
                while( XmlNode )
                {
                    if( XmlNode->GetName() == wxT( "Request" ) )
                    {
                        XmlSubNode = XmlNode->GetChildren();
                        while( XmlSubNode )
                        {
                            if( XmlSubNode->GetName() == wxT( "IsValid" ) )
                            {
                                if( XmlSubNode->GetNodeContent() != wxT( "True" ) )
                                {
                                    guLogError( wxT( "There was an error in the amazon search request." ) );
                                }
                                break;
                            }
                            XmlSubNode = XmlSubNode->GetNext();
                        }
                    }
                    else if( XmlNode->GetName() == wxT( "Errors" ) )
                    {
                        XmlSubNode = XmlNode->GetChildren();
                        while( XmlSubNode )
                        {
                            if( XmlSubNode->GetName() == wxT( "Error" ) )
                            {
                                XmlSubNode = XmlSubNode->GetChildren();
                                while( XmlSubNode )
                                {
                                    if( XmlSubNode->GetName() == wxT( "Message" ) )
                                    {
                                        guLogError( wxT( "Error: %s" ), XmlSubNode->GetNodeContent().c_str() );
                                        break;
                                    }
                                    XmlSubNode = XmlSubNode->GetNext();
                                }
                                return 0;
                            }
                            XmlSubNode = XmlSubNode->GetNext();
                        }
                    }
                    else if( XmlNode->GetName() == wxT( "Item" ) )
                    {
                        wxArrayString CurItem = GetImageInfo( XmlNode );
                        if( CurItem.Count() )
                        {
                            m_CoverLinks->Add( CurItem );
                            RetVal++;
                        }
                    }
                    XmlNode = XmlNode->GetNext();
                }
                break;
            }
            XmlNode = XmlNode->GetNext();
        }
    }
    return RetVal;
}


// -------------------------------------------------------------------------------- //
int guAmazonCoverFetcher::AddCoverLinks( int pagenum )
{
    wxString SearchUrl = wxString::Format( AMAZON_SEARCH_URL,
        guURLEncode( m_Artist ).c_str(),
        guURLEncode( m_Album ).c_str(),
        pagenum + 1 );

    //guLogMessage( wxT( "URL: %u %s" ), pagenum, SearchUrl.c_str() );

    char * Buffer = NULL;
    wxCurlHTTP http;
    http.AddHeader( wxT( "User-Agent: Mozilla/5.0 (X11; U; Linux i686; es-ES; rv:1.9.0.5) Gecko/2008121622 Ubuntu/8.10 (intrepid) Firefox/3.0.5" ) );
    http.AddHeader( wxT( "Accept: text/html" ) );
    http.AddHeader( wxT( "Accept-Charset: utf-8" ) );
    http.SetVerbose( true );
    http.Get( Buffer, SearchUrl );
    if( Buffer )
    {
        if( !m_MainThread->TestDestroy() )
        {
            //printf( "Buffer:\n%s\n", Buffer );
            wxString Content = wxString( Buffer, wxConvUTF8 );
            //Content = http.GetContent( SearchUrl, 60 );
            if( Content.Length() )
            {
                if( !m_MainThread->TestDestroy() )
                {
                    //guLogMessage( Content );
                    return ExtractImagesInfo( Content );
                }
            }
            else
            {
                guLogError( wxT( "Could not get the remote data from connection" ) );
            }
        }
        free( Buffer );
    }
    else
    {
        guLogWarning( wxT( "No data received when searching for images" ) );
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
