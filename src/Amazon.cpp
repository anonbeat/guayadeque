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
#include "Amazon.h"

#include "CoverEdit.h"
#include "Utils.h"

#include "Base64.h"
#include "sha2.h"
#include "hmac_sha2.h"

#include <wx/arrimpl.cpp>
#include <wx/statline.h>
#include <wx/sstream.h>

// see http://docs.amazonwebservices.com/AWSEcommerceService/2005-03-23/
//    http://webservices.amazon.com/onca/xml?Service=AWSECommerceService
//    &SubscriptionId=[YourSubscription IDHere]
//    &Operation=ItemSearch
//    &Artist=[The artist name]
//    &Keywords=[A Keywords String]
//    &SearchIndex=Music
//    &ItemPage=[The page Num {1..x}]

#define AMAZON_SEARCH_APIKEY    "AKIAI3VJGDYXLU7N2HKQ"
#define AMAZON_ASSOCIATE_TAG    "guaymusiplay-20"

#define AMAZON_SEARCH_URL       wxT( "http://webservices.amazon.com/onca/xml?" )
#define AMAZON_SEARCH_PARAMS    wxT( "AWSAccessKeyId=" AMAZON_SEARCH_APIKEY\
                                     "&Artist=%s"\
                                     "&AssociateTag=guaymusiplay-20"\
                                     "&ItemPage=%u"\
                                     "&Keywords=%s"\
                                     "&Operation=ItemSearch"\
                                     "&ResponseGroup=Images,Small"\
                                     "&SearchIndex=Music"\
                                     "&Service=AWSECommerceService"\
                                     "&Timestamp=%s" )



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
void inline HMACSha256( char * key, unsigned int key_size,
          char * message, unsigned int message_len,
          char * mac, unsigned mac_size )
{
    return hmac_sha256( ( unsigned char * ) key, key_size,
                        ( unsigned char * ) message, message_len,
                        ( unsigned char * ) mac, mac_size );
}

// -------------------------------------------------------------------------------- //
wxString percentEncodeRfc3986( const wxString &text )
{
    wxString RetVal = text;
    RetVal.Replace( wxT( "+" ), wxT( "%20" ) );
    RetVal.Replace( wxT( "*" ), wxT( "%2A" ) );
    RetVal.Replace( wxT( "%7E" ), wxT( "~" ) );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
wxString GetAmazonSign( const wxString &text )
{
#define AMAZON_SEARCH_SECRET    wxT( "ICsfRx7YNpBBamJyJcolN0qGKH6bBG7NlA9kLqhq" )
    wxString Str = wxT( "GET\nwebservices.amazon.com\n/onca/xml\n" ) + text;

    //guLogMessage( wxT( "String : '%s'" ), Str.c_str() );
    wxString Key = AMAZON_SEARCH_SECRET;
    char * Output = ( char * ) malloc( 1024 );

    HMACSha256( Key.char_str(), Key.Length(), Str.char_str(), Str.Length(), Output, SHA256_DIGEST_SIZE );

    wxString Sign = guBase64Encode( Output, SHA256_DIGEST_SIZE );
    //guLogMessage( wxT( "Signature: '%s'" ), Sign.c_str() );
    Sign.Replace( wxT( "+" ), wxT( "%2B" ) );
    Sign.Replace( wxT( "=" ), wxT( "%3D" ) );

    //guLogMessage( wxT( "Encoded: '%s'" ), Sign.c_str() );
    free( Output );
    return Sign;
}

// -------------------------------------------------------------------------------- //
int guAmazonCoverFetcher::AddCoverLinks( int pagenum )
{
    wxDateTime CurTime = wxDateTime::Now();

    wxString SearchParams = wxString::Format( AMAZON_SEARCH_PARAMS,
        percentEncodeRfc3986( guURLEncode( m_Artist ) ).c_str(),
        pagenum + 1,
        percentEncodeRfc3986( guURLEncode( m_Album ) ).c_str(),
        guURLEncode( CurTime.ToUTC().Format( wxT( "%Y-%m-%dT%H:%M:%S.000Z" ) ) ).c_str() );

    SearchParams.Replace( wxT( "," ), wxT( "%2C" ) );

    wxString SignText = GetAmazonSign( SearchParams );

    wxString SearchUrl = AMAZON_SEARCH_URL + SearchParams + wxT( "&Signature=" ) + SignText;

    //guLogMessage( wxT( "URL: %u %s" ), pagenum, SearchUrl.c_str() );
    if( !m_MainThread->TestDestroy() )
    {
        //printf( "Buffer:\n%s\n", Buffer );
        wxString Content = GetUrlContent( SearchUrl );
        //Content = http.GetContent( SearchUrl, 60 );
        //guLogMessage( wxT( "Amazon Response:\n%s" ), Content.c_str() );
        if( Content.Length() )
        {
            if( !m_MainThread->TestDestroy() )
            {
                return ExtractImagesInfo( Content );
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
