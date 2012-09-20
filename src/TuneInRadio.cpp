// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2012 J.Rios
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
#include "TuneInRadio.h"

#include "Accelerators.h"
#include "DbCache.h"
#include "DbRadios.h"
#include "Images.h"
#include "RadioPanel.h"
#include "RadioEditor.h"

#include "curl/http.h"

#include <wx/wfstream.h>
#include <wx/tokenzr.h>
#include <wx/xml/xml.h>

// -------------------------------------------------------------------------------- //
guTuneInRadioProvider::guTuneInRadioProvider( guRadioPanel * radiopanel, guDbRadios * dbradios ) :
    guRadioProvider( radiopanel, dbradios )
{
    m_ReadStationsThread = NULL;
}

// -------------------------------------------------------------------------------- //
guTuneInRadioProvider::~guTuneInRadioProvider()
{
}

// -------------------------------------------------------------------------------- //
bool guTuneInRadioProvider::OnContextMenu( wxMenu * menu, const wxTreeItemId &itemid, const bool forstations, const int selcount )
{
    return true;
}

// -------------------------------------------------------------------------------- //
void guTuneInRadioProvider::RegisterImages( wxImageList * imagelist )
{
    imagelist->Add( guImage( guIMAGE_INDEX_tiny_tunein ) );
    m_ImageIds.Add( imagelist->GetImageCount() - 1 );
}

// -------------------------------------------------------------------------------- //
void guTuneInRadioProvider::RegisterItems( guRadioGenreTreeCtrl * genretreectrl, wxTreeItemId &rootitem )
{
    guRadioItemData * TuneInData = new guRadioItemData( -1, guRADIO_SOURCE_TUNEIN, wxT( "tunein" ), wxT( guTUNEIN_BASE_URL ), 0 );
    m_TuneInId = genretreectrl->AppendItem( rootitem, wxT( "tunein" ), m_ImageIds[ 0 ], m_ImageIds[ 0 ], TuneInData );
}

// -------------------------------------------------------------------------------- //
bool guTuneInRadioProvider::HasItemId( const wxTreeItemId &itemid )
{
    wxTreeItemId ItemId = itemid;
    while( ItemId.IsOk() )
    {
        if( ItemId == m_TuneInId )
            return true;
        ItemId = m_RadioPanel->GetItemParent( ItemId );
    }
    return false;
}

// -------------------------------------------------------------------------------- //
void guTuneInRadioProvider::EndReadStationsThread( void )
{
    m_ReadStationsThread = NULL;

    m_RadioPanel->EndLoadingStations();
}

// -------------------------------------------------------------------------------- //
int guTuneInRadioProvider::GetStations( guRadioStations * stations, const long minbitrate )
{
    m_PendingItems.Empty();
    guRadioItemData * ItemData = m_RadioPanel->GetSelectedData();
    if( ItemData )
    {
        //AddStations( ItemData->GetUrl(), stations, minbitrate );
        CancellSearchStations();

        m_ReadStationsThread = new guTuneInReadStationsThread( this, m_RadioPanel, ItemData->GetUrl(), stations, minbitrate );
    }
    return stations->Count();
}

// -------------------------------------------------------------------------------- //
void guTuneInRadioProvider::CancellSearchStations( void )
{
    if( m_ReadStationsThread )
    {
        m_ReadStationsThread->Pause();
        m_ReadStationsThread->Delete();
    }
}

// -------------------------------------------------------------------------------- //
void guTuneInRadioProvider::SetSearchText( const wxArrayString &texts )
{
    m_SearchTexts = texts;
}


// -------------------------------------------------------------------------------- //
guTuneInReadStationsThread::guTuneInReadStationsThread( guTuneInRadioProvider * tuneinprovider,
    guRadioPanel * radiopanel, const wxString &url, guRadioStations * stations, const long minbitrate ) :
    wxThread()
{
    m_TuneInProvider = tuneinprovider;
    m_RadioPanel = radiopanel;
    m_RadioStations = stations;
    m_Url = url;
    m_MinBitRate = minbitrate;

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 10 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guTuneInReadStationsThread::~guTuneInReadStationsThread()
{
    if( !TestDestroy() )
    {
        m_TuneInProvider->EndReadStationsThread();
    }
}

// -------------------------------------------------------------------------------- //
wxString GetTuneInUrl( const wxString &url )
{
    guDbCache * DbCache = guDbCache::GetDbCache();
    wxString Content = DbCache->GetContent( url );

    if( Content.IsEmpty() )
    {
        char *      Buffer = NULL;
        wxCurlHTTP  http;

        // Only with a UserAgent is accepted the Charset requested
        //http.AddHeader( wxT( "User-Agent: " "Dalvik/1.6.0.(Linux;.U;.Android.4.1.1;.Galaxy.Nexus.Build/JRO03L)" ) );
        http.AddHeader( wxT( "User-Agent: " ) guDEFAULT_BROWSER_USER_AGENT );
        http.AddHeader( wxT( "Accept: text/html" ) );
        http.AddHeader( wxT( "Accept-Charset: utf-8" ) );
        http.SetOpt( CURLOPT_FOLLOWLOCATION, 1 );
        http.Get( Buffer, url );

        if( Buffer )
        {
            Content = wxString( Buffer, wxConvUTF8 );

            if( !Content.IsEmpty() )
            {
                DbCache->SetContent( url, Content, guDBCACHE_TYPE_TUNEIN );
            }

            free( Buffer );
        }
    }

    return Content;
}

// -------------------------------------------------------------------------------- //
static int wxCMPFUNC_CONV CompareNameA( guRadioStation ** item1, guRadioStation ** item2 )
{
    return ( * item1 )->m_Name.Cmp( ( * item2 )->m_Name );
}

// -------------------------------------------------------------------------------- //
static int wxCMPFUNC_CONV CompareNameD( guRadioStation ** item1, guRadioStation ** item2 )
{
    return ( * item2 )->m_Name.Cmp( ( * item1 )->m_Name );
}

// -------------------------------------------------------------------------------- //
static int wxCMPFUNC_CONV CompareBitRateA( guRadioStation ** item1, guRadioStation ** item2 )
{
    if( ( * item1 )->m_BitRate == ( * item2 )->m_BitRate )
        return 0;
    else if( ( * item1 )->m_BitRate > ( * item2 )->m_BitRate )
        return 1;
    else
        return -1;
}

// -------------------------------------------------------------------------------- //
static int wxCMPFUNC_CONV CompareBitRateD( guRadioStation ** item1, guRadioStation ** item2 )
{
    if( ( * item1 )->m_BitRate == ( * item2 )->m_BitRate )
        return 0;
    else if( ( * item2 )->m_BitRate > ( * item1 )->m_BitRate )
        return 1;
    else
        return -1;
}

// -------------------------------------------------------------------------------- //
static int wxCMPFUNC_CONV CompareTypeA( guRadioStation ** item1, guRadioStation ** item2 )
{
    return ( * item1 )->m_Type.Cmp( ( * item2 )->m_Type );
}

// -------------------------------------------------------------------------------- //
static int wxCMPFUNC_CONV CompareTypeD( guRadioStation ** item1, guRadioStation ** item2 )
{
    return ( * item2 )->m_Type.Cmp( ( * item1 )->m_Type );
}

// -------------------------------------------------------------------------------- //
static int wxCMPFUNC_CONV CompareNowPlayingA( guRadioStation ** item1, guRadioStation ** item2 )
{
    return ( * item1 )->m_NowPlaying.Cmp( ( * item2 )->m_NowPlaying );
}

// -------------------------------------------------------------------------------- //
static int wxCMPFUNC_CONV CompareNowPlayingD( guRadioStation ** item1, guRadioStation ** item2 )
{
    return ( * item2 )->m_NowPlaying.Cmp( ( * item1 )->m_NowPlaying );
}

// -------------------------------------------------------------------------------- //
void guTuneInReadStationsThread::SortStations( void )
{
    int     StationsOrder = m_RadioPanel->GetStationsOrder();
    bool    StationsOrderDesc = m_RadioPanel->GetStationsOrderDesc();

    switch( StationsOrder )
    {
        case guRADIOSTATIONS_COLUMN_NAME :
            m_RadioStations->Sort( StationsOrderDesc ? CompareNameD : CompareNameA );
            break;

        case guRADIOSTATIONS_COLUMN_BITRATE :
            m_RadioStations->Sort( StationsOrderDesc ? CompareBitRateD : CompareBitRateA );

        case guRADIOSTATIONS_COLUMN_LISTENERS :
            break;

        case guRADIOSTATIONS_COLUMN_TYPE :
            m_RadioStations->Sort( StationsOrderDesc ? CompareTypeD : CompareTypeA );
            break;

        case guRADIOSTATIONS_COLUMN_NOWPLAYING :
            m_RadioStations->Sort( StationsOrderDesc ? CompareNowPlayingD : CompareNowPlayingA );
            break;
    }
}

// -------------------------------------------------------------------------------- //
bool SearchFilterTexts( wxArrayString &texts, const wxString &name )
{
    int Index;
    int Count = texts.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        //guLogMessage( wxT( "%s = > '%s'" ), name.c_str(), texts[ Index ].Lower().c_str() );
        if( name.Find( texts[ Index ].Lower() ) == wxNOT_FOUND )
            return false;
    }
    return true;
}

// -------------------------------------------------------------------------------- //
void guTuneInReadStationsThread::ReadStations( wxXmlNode * xmlnode, wxTreeItemId parentitem, guRadioGenreTreeCtrl * radiogenretree, guRadioStations * stations, const long minbitrate )
{
    wxString MoreStationsUrl;
    while( xmlnode && !TestDestroy() )
    {
        wxString Type;
        wxString Name;
        wxString Url;
        xmlnode->GetPropVal( wxT( "type" ), &Type );
        if( Type == wxT( "" ) )
        {
            ReadStations( xmlnode->GetChildren(), parentitem, radiogenretree, stations, minbitrate );
        }
        else if( Type == wxT( "link" ) )
        {
            xmlnode->GetPropVal( wxT( "text" ), &Name );
            xmlnode->GetPropVal( wxT( "URL" ), &Url );

            if( Name == wxT( "Find by Name" ) )
            {
            }
            else if( Name == wxT( "More Stations" ) )
            {
                MoreStationsUrl = Url;
            }
            else
            {
                //guLogMessage( wxT( "AddPendingItem '%s' '%s'" ), Name.c_str(), Url.c_str() );
                m_TuneInProvider->AddPendingItem( Name + wxT( "|" ) + Url );
                wxCommandEvent Event( wxEVT_COMMAND_MENU_SELECTED, ID_RADIO_CREATE_TREE_ITEM );
                wxPostEvent( m_RadioPanel, Event );
                Sleep( 50 );
            }
        }
        else if( Type == wxT( "audio" ) )
        {
            //    <outline type="audio"
            //        text="Talk Radio Europe (Cartagena)"
            //        URL="http://opml.radiotime.com/Tune.ashx?id=s111270"
            //        bitrate="64"
            //        reliability="96"
            //        guide_id="s111270"
            //        subtext="your voice in spain"
            //        genre_id="g32"
            //        formats="mp3"
            //        item="station"
            //        image="http://radiotime-logos.s3.amazonaws.com/s111270q.png"
            //        now_playing_id="s111270"
            //        preset_id="s111270"/>
            guRadioStation * RadioStation = new guRadioStation();

            long lBitRate;
            wxString BitRate;
            xmlnode->GetPropVal( wxT( "bitrate" ), &BitRate );
            BitRate.ToLong( &lBitRate );
            xmlnode->GetPropVal( wxT( "text" ), &RadioStation->m_Name );
            if( ( BitRate.IsEmpty() || ( lBitRate >= minbitrate ) ) && SearchFilterTexts( m_TuneInProvider->GetSearchTexts(), RadioStation->m_Name.Lower() ) )
            {
                RadioStation->m_BitRate = lBitRate;
                RadioStation->m_Id = -1;
                RadioStation->m_SCId = wxNOT_FOUND;
                xmlnode->GetPropVal( wxT( "URL" ), &RadioStation->m_Link );
                xmlnode->GetPropVal( wxT( "formats" ), &RadioStation->m_Type );
                xmlnode->GetPropVal( wxT( "subtext" ), &RadioStation->m_NowPlaying );
                RadioStation->m_Source = guRADIO_SOURCE_TUNEIN;
                RadioStation->m_Listeners = 0;

                stations->Add( RadioStation );
                //guLogMessage( wxT( "Adding station %s" ), RadioStation->m_Name.c_str() );
            }
            else
            {
                delete RadioStation;
            }
        }

        xmlnode = xmlnode->GetNext();
    }

    SortStations();

    wxCommandEvent Event( wxEVT_COMMAND_MENU_SELECTED, ID_RADIO_UPDATED );
    wxPostEvent( m_RadioPanel, Event );

//    if( m_TuneInProvider->GetPendingItemsCount() )
//    {
//        wxCommandEvent Event( wxEVT_COMMAND_MENU_SELECTED, ID_RADIO_CREATE_TREE_ITEM );
//        wxPostEvent( m_RadioPanel, Event );
//    }

    if( !MoreStationsUrl.IsEmpty() )
    {
        AddStations( MoreStationsUrl, stations, minbitrate );
    }
}

// -------------------------------------------------------------------------------- //
int guTuneInReadStationsThread::AddStations( const wxString &url, guRadioStations * stations, const long minbitrate )
{
    wxString Content = GetTuneInUrl( url );
    //guLogMessage( wxT( "AddStations: %s" ), url.c_str() );

    if( !Content.IsEmpty() )
    {
        guRadioGenreTreeCtrl * RadioTreeCtrl = m_RadioPanel->GetTreeCtrl();
        wxTreeItemId SelectedItem = m_RadioPanel->GetSelectedGenre();

        wxStringInputStream Ins( Content );
        wxXmlDocument XmlDoc( Ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        if( XmlNode )
        {
            if( XmlNode->GetName() == wxT( "opml" ) )
            {
                XmlNode = XmlNode->GetChildren();
                while( XmlNode && !TestDestroy() )
                {
                    //guLogMessage( wxT( "XmlNode: '%s'" ), XmlNode->GetName().c_str() );
                    if( XmlNode->GetName() == wxT( "outline" ) )
                    {
                        wxString Type;
                        XmlNode->GetPropVal( wxT( "type" ), &Type );
                        if( Type == wxT( "" ) )
                        {
                            ReadStations( XmlNode->GetChildren(), SelectedItem, RadioTreeCtrl, stations, minbitrate );
                        }
                        else
                        {
                            ReadStations( XmlNode, SelectedItem, RadioTreeCtrl, stations, minbitrate );
                            break;
                        }
                    }
                    else if( XmlNode->GetName() == wxT( "body" ) )
                    {
                        XmlNode = XmlNode->GetChildren();
                        continue;
                    }

                    XmlNode = XmlNode->GetNext();
                }
            }
        }

        //RadioTreeCtrl->Expand( SelectedItem );
    }

    SortStations();

    wxCommandEvent Event( wxEVT_COMMAND_MENU_SELECTED, ID_RADIO_UPDATED );
    wxPostEvent( m_RadioPanel, Event );

    return stations->Count();
}

// -------------------------------------------------------------------------------- //
guTuneInReadStationsThread::ExitCode guTuneInReadStationsThread::Entry()
{
    if( TestDestroy() )
        return 0;


    if( !TestDestroy() )
    {
        guRadioGenreTreeCtrl * RadioTreeCtrl = m_RadioPanel->GetTreeCtrl();
        wxTreeItemId SelectedItem = m_RadioPanel->GetSelectedGenre();
        RadioTreeCtrl->DeleteChildren( SelectedItem );

        AddStations( m_Url, m_RadioStations, m_MinBitRate );
    }

    return 0;
}

// -------------------------------------------------------------------------------- //
