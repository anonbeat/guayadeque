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
#include "Podcasts.h"

#include <wx/arrimpl.cpp>

WX_DEFINE_OBJARRAY(guPodcastChannelArray);
WX_DEFINE_OBJARRAY(guPodcastItemArray);

// -------------------------------------------------------------------------------- //
int StrLengthToInt( const wxString &length )
{
    int RetVal = 0;
    int Factor[] = { 1, 60, 3600, 86400 };
    int FactorIndex = 0;

    if( !length.IsEmpty() )
    {
        // 1:02:03:04
        wxString Rest = length.Strip( wxString::both );
        int element;
        do {
            Rest.AfterLast( wxT( ':' ) ).ToLong( ( long * ) &element );
            if( !element )
                break;
            RetVal += Factor[ FactorIndex ] * element;
            FactorIndex++;
            if( ( FactorIndex > 3 ) )
                break;
            Rest = Rest.BeforeLast( wxT( ':' ) );
        } while( !Rest.IsEmpty() );
    }
//    guLogMessage( wxT( "%s -> %i" ), length.c_str(), RetVal );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
guPodcastChannel::guPodcastChannel( const wxString &url )
{
    m_Url = url;
    ReadContent();
}

// -------------------------------------------------------------------------------- //
void guPodcastChannel::ReadContent( void )
{
    wxCurlHTTP  http;
    guLogMessage( wxT( "The address is %s" ), EntryDialog->GetValue().c_str() );

    http.AddHeader( wxT( "User-Agent: Mozilla/5.0 (X11; U; Linux i686; es-ES; rv:1.9.0.5) Gecko/2008121622 Ubuntu/8.10 (intrepid) Firefox/3.0.5" ) );
    http.AddHeader( wxT( "Accept: */*" ) );
    http.AddHeader( wxT( "Accept-Charset: utf-8;iso-8859-1" ) );
    char * Buffer = NULL;
    http.Get( Buffer, EntryDialog->GetValue() );
    if( Buffer )
    {
        wxMemoryInputStream ins( Buffer, Strlen( Buffer ) );
        wxXmlDocument XmlDoc( ins );
        //wxSt
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        if( XmlNode && XmlNode->GetName() == wxT( "rss" ) )
        {
            ReadXml( XmlNode->GetChildren() );
        }
        free( Buffer );
    }
    else
    {
        guLogError( wxT( "Could not get podcast content for %s" ), EntryDialog->GetValue().c_str() );
    }
}

// -------------------------------------------------------------------------------- //
void guPodcastChannel::ReadXml( wxXmlNode * XmlNode )
{
    if( XmlNode && XmlNode->GetName() == wxT( "channel" ) )
    {
        XmlNode = XmlNode->GetChildren();
        while( XmlNode )
        {
            if( XmlNode->GetName() == wxT( "title" ) )
            {
                m_Title = XmlNode->GetNodeContent();
            }
            else if( XmlNode->GetName() == wxT( "link" ) )
            {
                m_Link = XmlNode->GetNodeContent();
            }
            else if( XmlNode->GetName() == wxT( "language" ) )
            {
                m_Lang = XmlNode->GetNodeContent();
            }
            else if( XmlNode->GetName() == wxT( "description" ) )
            {
                m_Description = XmlNode->GetNodeContent();
            }
            else if( XmlNode->GetName() == wxT( "itunes:author" ) )
            {
                m_Author = XmlNode->GetNodeContent();
            }
            else if( XmlNode->GetName() == wxT( "itunes:owner" ) )
            {
                ReadXmlOwner( XmlNode->GetChildren() );
            }
            else if( XmlNode->GetName() == wxT( "itunes:image" ) )
            {
                XmlNode->GetPropVal( wxT( "href" ), &m_Image );
            }
            else if( XmlNode->GetName() == wxT( "itunes:category" ) )
            {
                XmlNode->GetPropVal( wxT( "text" ), &m_Category );
            }
            else if( XmlNode->GetName() == wxT( "itunes:summary" ) )
            {
                m_Summary = XmlNode->GetNodeContent();
            }
            else if( XmlNode->GetName() == wxT( "item" ) )
            {
                guPodcastItem * PodcastItem = new guPodcastItem();
                PodcastItem->ReadXml( XmlNode->GetChildren() );
                //guLogMessage( wxT( "Item Length: %i" ), PodcastItem->m_Length );
                m_Items.Add( PodcastItem );
            }
            XmlNode = XmlNode->GetNext();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPodcastChannel::ReadXmlOwner( wxXmlNode * XmlNode )
{
    while( XmlNode )
    {
        if( XmlNode->GetName() == wxT( "itunes:name" ) )
        {
            m_OwnerName = XmlNode->GetNodeContent();
        }
        else if( XmlNode->GetName() == wxT( "itunes:email" ) )
        {
            m_OwnerEmail = XmlNode->GetNodeContent();
        }
        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void guPodcastItem::ReadXml( wxXmlNode * XmlNode )
{
    while( XmlNode )
    {
        if( XmlNode->GetName() == wxT( "title" ) )
        {
            m_Title = XmlNode->GetNodeContent();
            guLogMessage( wxT( "Item: '%s'" ), m_Title.c_str() );
        }
        else if( XmlNode->GetName() == wxT( "enclosure" ) )
        {
            XmlNode->GetPropVal( wxT( "url" ), &m_Enclosure );
            wxString LenStr;
            XmlNode->GetPropVal( wxT( "length" ), &LenStr );
            LenStr.ToULong( ( unsigned long * ) &m_FileSize );
        }
        else if( item->m_Summary.IsEmpty() && ( XmlNode->GetName() == wxT( "itunes:summary" ) ) ||
                 ( XmlNode->GetName() == wxT( "description" ) ) )
        {
            m_Summary= XmlNode->GetNodeContent();
        }
        else if( XmlNode->GetName() == wxT( "pubDate" ) )
        {
            wxDateTime DateTime;
            DateTime.ParseRfc822Date( XmlNode->GetNodeContent() );
            m_Time = DateTime.GetTicks();
        }
        else if( XmlNode->GetName() == wxT( "itunes:duration" ) )
        {
            m_Length = StrLengthToInt( XmlNode->GetNodeContent() );
        }
        else if( XmlNode->GetName() == wxT( "itunes:author" ) )
        {
            m_Author = XmlNode->GetNodeContent();
        }
        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
