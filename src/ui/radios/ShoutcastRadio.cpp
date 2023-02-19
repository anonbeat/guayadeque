// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2023 J.Rios anonbeat@gmail.com
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
#include "ShoutcastRadio.h"

#include "Accelerators.h"
#include "Config.h"
#include "DbCache.h"
#include "DbRadios.h"
#include "Http.h"
#include "Images.h"
#include "MainFrame.h"
#include "RadioPanel.h"
#include "RadioEditor.h"
#include "RadioGenreEditor.h"
#include "StatusBar.h"

#include <wx/wfstream.h>
#include <wx/tokenzr.h>
#include <wx/xml/xml.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
guShoutcastRadioProvider::guShoutcastRadioProvider( guRadioPanel * radiopanel, guDbRadios * dbradios ) :
    guRadioProvider( radiopanel, dbradios )
{
    radiopanel->Bind( wxEVT_MENU, &guShoutcastRadioProvider::OnGenreAdd, this, ID_RADIO_GENRE_ADD );
    radiopanel->Bind( wxEVT_MENU, &guShoutcastRadioProvider::OnGenreEdit, this, ID_RADIO_GENRE_EDIT );
    radiopanel->Bind( wxEVT_MENU, &guShoutcastRadioProvider::OnGenreDelete, this, ID_RADIO_GENRE_DELETE );
    radiopanel->Bind( wxEVT_MENU, &guShoutcastRadioProvider::OnSearchAdd, this, ID_RADIO_SEARCH_ADD );
    radiopanel->Bind( wxEVT_MENU, &guShoutcastRadioProvider::OnSearchEdit, this, ID_RADIO_SEARCH_EDIT );
    radiopanel->Bind( wxEVT_MENU, &guShoutcastRadioProvider::OnSearchDelete, this, ID_RADIO_SEARCH_DELETE );
}

// -------------------------------------------------------------------------------- //
guShoutcastRadioProvider::~guShoutcastRadioProvider()
{
}

// -------------------------------------------------------------------------------- //
bool guShoutcastRadioProvider::OnContextMenu( wxMenu * menu, const wxTreeItemId &itemid, const bool forstations, const int selcount )
{

    guRadioItemData * ItemData = m_RadioPanel->GetSelectedData( itemid );
    wxMenuItem * MenuItem;

    if( ( ItemData && ( ItemData->GetSource() == guRADIO_SOURCE_SHOUTCAST_GENRE ) ) || ( itemid == m_ShoutcastGenreId ) )
    {
        if( selcount )
        {
            menu->AppendSeparator();
        }
        MenuItem = new wxMenuItem( menu, ID_RADIO_GENRE_ADD, _( "Add Genre" ), _( "Create a new genre" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        menu->Append( MenuItem );

        if( ItemData )
        {
            MenuItem = new wxMenuItem( menu, ID_RADIO_GENRE_EDIT, _( "Edit genre" ), _( "Change the selected genre" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit ) );
            menu->Append( MenuItem );

            MenuItem = new wxMenuItem( menu, ID_RADIO_GENRE_DELETE, _( "Delete genre" ), _( "Delete the selected search" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit_clear ) );
            menu->Append( MenuItem );
        }
        if( !selcount )
        {
            menu->AppendSeparator();
        }
    }
    else if( ( ItemData && ( ItemData->GetSource() == guRADIO_SOURCE_SHOUTCAST_SEARCH ) ) || ( itemid == m_ShoutcastSearchId ) )
    {
        if( selcount )
        {
            menu->AppendSeparator();
        }
        MenuItem = new wxMenuItem( menu, ID_RADIO_SEARCH_ADD, _( "Add Search" ), _( "Create a new search" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        menu->Append( MenuItem );

        if( ItemData )
        {
            MenuItem = new wxMenuItem( menu, ID_RADIO_SEARCH_EDIT, _( "Edit search" ), _( "Change the selected search" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit ) );
            menu->Append( MenuItem );

            MenuItem = new wxMenuItem( menu, ID_RADIO_SEARCH_DELETE, _( "Delete search" ), _( "Delete the selected search" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit_clear ) );
            menu->Append( MenuItem );
        }
        if( !selcount )
        {
            menu->AppendSeparator();
        }
    }

    if( selcount )
    {
        menu->AppendSeparator();
    }

    MenuItem = new wxMenuItem( menu, ID_RADIO_DOUPDATE, _( "Update Radio Stations" ), _( "Update the radio station lists" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_reload ) );
    menu->Append( MenuItem );

    return true;
}

// -------------------------------------------------------------------------------- //
void guShoutcastRadioProvider::RegisterImages( wxImageList * imagelist )
{
    imagelist->Add( guImage( guIMAGE_INDEX_tiny_shoutcast ) );
    m_ImageIds.Add( imagelist->GetImageCount() - 1 );
    imagelist->Add( guImage( guIMAGE_INDEX_search ) );
    m_ImageIds.Add( imagelist->GetImageCount() - 1 );
}

// -------------------------------------------------------------------------------- //
void guShoutcastRadioProvider::RegisterItems( guRadioGenreTreeCtrl * genretreectrl, wxTreeItemId &rootitem )
{
    m_ShoutcastId = genretreectrl->AppendItem( rootitem, wxT( "Shoutcast" ), m_ImageIds[ 0 ], m_ImageIds[ 0 ], NULL );
    m_ShoutcastGenreId = genretreectrl->AppendItem( m_ShoutcastId, _( "Genre" ), m_ImageIds[ 0 ], m_ImageIds[ 0 ], NULL );
    m_ShoutcastSearchId = genretreectrl->AppendItem( m_ShoutcastId, _( "Searches" ), m_ImageIds[ 1 ], m_ImageIds[ 1 ], NULL );
}

// -------------------------------------------------------------------------------- //
bool guShoutcastRadioProvider::HasItemId( const wxTreeItemId &itemid )
{
    wxTreeItemId ItemId = itemid;
    while( ItemId.IsOk() )
    {
        if( ItemId == m_ShoutcastId )
            return true;
        ItemId = m_RadioPanel->GetItemParent( ItemId );
    }
    return false;
}

// -------------------------------------------------------------------------------- //
int guShoutcastRadioProvider::GetStations( guRadioStations * stations, const long minbitrate )
{
    wxTreeItemId SelectedItem = m_RadioPanel->GetSelectedGenre();
    guRadioGenreTreeCtrl * RadioTreeCtrl = m_RadioPanel->GetTreeCtrl();

    if( SelectedItem == m_ShoutcastId )
    {
        m_Db->SetRadioSourceFilter( guRADIO_SOURCE_SHOUTCAST_GENRE );
    }
    else if( SelectedItem == m_ShoutcastGenreId )
    {
        RadioTreeCtrl->DeleteChildren( m_ShoutcastGenreId );

        guListItems RadioGenres;
        m_Db->GetRadioGenres( guRADIO_SOURCE_SHOUTCAST_GENRE, &RadioGenres );

        int Index;
        int Count = RadioGenres.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            RadioTreeCtrl->AppendItem( m_ShoutcastGenreId, RadioGenres[ Index ].m_Name, -1, -1,
                new guRadioItemData( RadioGenres[ Index ].m_Id, guRADIO_SOURCE_SHOUTCAST_GENRE, RadioGenres[ Index ].m_Name, 0 ) );
        }

        m_Db->SetRadioSourceFilter( guRADIO_SOURCE_SHOUTCAST_GENRE );
    }
    else if( SelectedItem == m_ShoutcastSearchId )
    {
        RadioTreeCtrl->DeleteChildren( m_ShoutcastSearchId );
        guListItems RadioSearchs;
        wxArrayInt RadioFlags;
        m_Db->GetRadioGenres( guRADIO_SOURCE_SHOUTCAST_SEARCH, &RadioSearchs, true, &RadioFlags );
        int Index;
        int Count = RadioSearchs.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            RadioTreeCtrl->AppendItem( m_ShoutcastSearchId, RadioSearchs[ Index ].m_Name, -1, -1,
                new guRadioItemData( RadioSearchs[ Index ].m_Id, guRADIO_SOURCE_SHOUTCAST_SEARCH, RadioSearchs[ Index ].m_Name, RadioFlags[ Index ] ) );
        }

        m_Db->SetRadioSourceFilter( guRADIO_SOURCE_SHOUTCAST_SEARCH );
    }
    else
    {
        guRadioItemData * ItemData = m_RadioPanel->GetSelectedData( SelectedItem );
        if( ItemData )
        {
            m_Db->SetRadioSourceFilter( ItemData->GetSource() );

            wxArrayInt RadioGenres;
            RadioGenres.Add( ItemData->GetId() );
            m_Db->SetRadioGenresFilters( RadioGenres );
        }
    }

//    if( RadioTreeCtrl->ItemHasChildren( SelectedItem ) )
//    {
//        if( !RadioTreeCtrl->IsExpanded( SelectedItem ) )
//            RadioTreeCtrl->Expand( SelectedItem );
//    }

    m_Db->GetRadioStations( stations );

    m_RadioPanel->EndLoadingStations();

    return stations->Count();
}

// -------------------------------------------------------------------------------- //
void guShoutcastRadioProvider::DoUpdate( void )
{
    guLogMessage( wxT( "ShoutcastRadioProvider::DoUpdate" ) );
    wxTreeItemId            ItemId;
    wxArrayInt              GenresIds;
    guRadioItemData *   ItemData;
    int Source = guRADIO_SOURCE_SHOUTCAST_GENRE;

    ItemId = m_RadioPanel->GetSelectedGenre();
    if( ItemId.IsOk() )
    {
        ItemData = m_RadioPanel->GetSelectedData( ItemId );
        if( ItemData )
        {
            Source = ItemData->GetSource();
            GenresIds.Add( ItemData->GetId() );
        }
        else if( ItemId == m_ShoutcastSearchId )
        {
            Source = guRADIO_SOURCE_SHOUTCAST_SEARCH;
            guListItems Genres;
            m_Db->GetRadioGenres( Source, &Genres );
            int index;
            int count = Genres.Count();
            for( index = 0; index < count; index++ )
            {
                GenresIds.Add( Genres[ index ].m_Id );
            }
        }
    }

    if( !GenresIds.Count() )
    {
        guListItems Genres;
        m_Db->GetRadioGenres( guRADIO_SOURCE_SHOUTCAST_GENRE, &Genres );
        int index;
        int count = Genres.Count();
        for( index = 0; index < count; index++ )
        {
            GenresIds.Add( Genres[ index ].m_Id );
        }
    }

    if( GenresIds.Count() )
    {
        guMainFrame * MainFrame = ( guMainFrame * ) guMainFrame::GetMainFrame();
        int GaugeId = ( ( guStatusBar * ) MainFrame->GetStatusBar() )->AddGauge( _( "Radios" ) );
        guShoutcastUpdateThread * UpdateRadiosThread = new guShoutcastUpdateThread( m_Db, m_RadioPanel, GenresIds, Source, GaugeId );
        if( !UpdateRadiosThread )
        {
            guLogError( wxT( "Could not create the radio update thread" ) );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guShoutcastRadioProvider::SetSearchText( const wxArrayString &texts )
{
    guLogMessage( wxT( "SetSearchText" ) );
    m_Db->SetRaTeFilters( texts );
}

// -------------------------------------------------------------------------------- //
void guShoutcastRadioProvider::OnGenreAdd( wxCommandEvent &event )
{
    int Index;
    int Count;
    guRadioGenreEditor * RadioGenreEditor = new guRadioGenreEditor( m_RadioPanel, m_Db );
    if( RadioGenreEditor )
    {
        bool NeedReload = false;
        //
        if( RadioGenreEditor->ShowModal() == wxID_OK )
        {
            wxArrayString NewGenres;
            wxArrayInt DelGenres;
            RadioGenreEditor->GetGenres( NewGenres, DelGenres );
            if( ( Count = NewGenres.Count() ) )
            {
                //
                for( Index = 0; Index < Count; Index++ )
                {
                    m_Db->AddRadioGenre( NewGenres[ Index ], guRADIO_SOURCE_SHOUTCAST_GENRE, guRADIO_SEARCH_FLAG_NONE );
                }
                NeedReload = true;
            }

            if( ( Count = DelGenres.Count() ) )
            {
                for( Index = 0; Index < Count; Index++ )
                {
                    m_Db->DelRadioGenre( DelGenres[ Index ] );
                }
                NeedReload = true;
            }

            if( NeedReload )
                m_RadioPanel->ReloadStations();
        }
        //
        RadioGenreEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guShoutcastRadioProvider::OnGenreEdit( wxCommandEvent &event )
{
    wxTreeItemId ItemId = m_RadioPanel->GetSelectedGenre();

    if( ItemId.IsOk() )
    {
        guRadioItemData * RadioGenreData = m_RadioPanel->GetSelectedData( ItemId );

        if( RadioGenreData )
        {
            // Get the Index of the First Selected Item
            wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( m_RadioPanel, _( "Genre Name: " ), _( "Enter the new Genre Name" ), RadioGenreData->GetName() );
            if( EntryDialog->ShowModal() == wxID_OK )
            {
                m_Db->SetRadioGenre( RadioGenreData->GetId(), EntryDialog->GetValue() );

                guRadioGenreTreeCtrl * RadioTreeCtrl = m_RadioPanel->GetTreeCtrl();
                RadioTreeCtrl->SelectItem( m_ShoutcastGenreId );

                m_RadioPanel->ReloadStations();
            }

            EntryDialog->Destroy();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guShoutcastRadioProvider::OnGenreDelete( wxCommandEvent &event )
{
    wxArrayTreeItemIds ItemIds;
    int index;
    int count;

    guRadioGenreTreeCtrl * RadioTreeCtrl = m_RadioPanel->GetTreeCtrl();

    if( ( count = RadioTreeCtrl->GetSelections( ItemIds ) ) )
    {
        if( wxMessageBox( _( "Are you sure to delete the selected radio genres?" ),
                          _( "Confirm" ),
                          wxICON_QUESTION|wxYES_NO|wxNO_DEFAULT, m_RadioPanel ) == wxYES )
        {
            guRadioItemData * RadioGenreData;
            for( index = 0; index < count; index++ )
            {
                RadioGenreData = m_RadioPanel->GetSelectedData( ItemIds[ index ] );
                if( RadioGenreData )
                {
                    m_Db->DelRadioGenre( RadioGenreData->GetId() );
                }
            }

            RadioTreeCtrl->SelectItem( m_ShoutcastGenreId );

            m_RadioPanel->ReloadStations();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guShoutcastRadioProvider::OnSearchAdd( wxCommandEvent &event )
{
    guRadioItemData ShoutcastItem( 0, guRADIO_SOURCE_SHOUTCAST_SEARCH, wxEmptyString, guRADIO_SEARCH_FLAG_DEFAULT );
    guShoutcastSearch * ShoutcastSearch = new guShoutcastSearch( m_RadioPanel, &ShoutcastItem );
    if( ShoutcastSearch )
    {
        if( ShoutcastSearch->ShowModal() == wxID_OK )
        {
            int RadioSearchId = m_Db->AddRadioGenre( ShoutcastItem.GetName(), guRADIO_SOURCE_SHOUTCAST_SEARCH, ShoutcastItem.GetFlags() );

            m_RadioPanel->ReloadStations();

            guRadioGenreTreeCtrl * RadioTreeCtrl = m_RadioPanel->GetTreeCtrl();
            RadioTreeCtrl->Expand( m_ShoutcastSearchId );

            wxTreeItemId ItemId = RadioTreeCtrl->GetItemId( &m_ShoutcastSearchId, RadioSearchId );
            if( ItemId.IsOk() )
            {
                RadioTreeCtrl->SelectItem( ItemId );

//                OnRadioUpdate( event );
            }
        }
        ShoutcastSearch->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guShoutcastRadioProvider::OnSearchEdit( wxCommandEvent &event )
{
    guRadioItemData * ItemData = m_RadioPanel->GetSelectedData();
    if( ItemData && ItemData->GetSource() == guRADIO_SOURCE_SHOUTCAST_SEARCH )
    {
        guShoutcastSearch * ShoutcastSearch = new guShoutcastSearch( m_RadioPanel, ItemData );
        if( ShoutcastSearch )
        {
            if( ShoutcastSearch->ShowModal() == wxID_OK )
            {
                m_Db->SetRadioGenre( ItemData->GetId(), ItemData->GetName(), guRADIO_SOURCE_SHOUTCAST_SEARCH, ItemData->GetFlags() );

                m_RadioPanel->ReloadStations();
            }

            ShoutcastSearch->Destroy();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guShoutcastRadioProvider::OnSearchDelete( wxCommandEvent &event )
{
    wxArrayTreeItemIds ItemIds;
    int index;
    int count;

    guRadioGenreTreeCtrl * RadioTreeCtrl = m_RadioPanel->GetTreeCtrl();
    if( ( count = RadioTreeCtrl->GetSelections( ItemIds ) ) )
    {
        if( wxMessageBox( _( "Are you sure to delete the selected radio searches?" ),
                          _( "Confirm" ),
                          wxICON_QUESTION|wxYES_NO|wxNO_DEFAULT, m_RadioPanel ) == wxYES )
        {
            guRadioItemData * RadioGenreData;
            for( index = 0; index < count; index++ )
            {
                RadioGenreData = m_RadioPanel->GetSelectedData( ItemIds[ index ] );
                if( RadioGenreData )
                {
                    m_Db->DelRadioGenre( RadioGenreData->GetId() );
                }
            }

            m_RadioPanel->ReloadStations();
        }
    }
}




// -------------------------------------------------------------------------------- //
// guShoutcastUpdateThread
// -------------------------------------------------------------------------------- //
bool inline guListItemsSearchName( guListItems &items, const wxString &name )
{
    int Index;
    int Count = items.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( items[ Index ].m_Name.Lower() == name )
        {
            return true;
        }
    }
    return false;
}

// -------------------------------------------------------------------------------- //
guShoutcastUpdateThread::guShoutcastUpdateThread( guDbRadios * db, guRadioPanel * radiopanel,
                                const wxArrayInt &ids, const int source, int gaugeid )
{
    m_Db = db;
    m_RadioPanel = radiopanel;
    m_Ids = ids;
    m_Source = source;
    m_GaugeId = gaugeid;

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}


// -------------------------------------------------------------------------------- //
void guShoutcastUpdateThread::CheckRadioStationsFilters( const int flags, const wxString &text, guRadioStations &stations )
{
    guListItems RadioGenres;
    m_Db->GetRadioGenres( guRADIO_SOURCE_SHOUTCAST_GENRE, &RadioGenres );

    int Index;
    int Count = stations.Count();
    if( Count )
    {
        for( Index = Count - 1; Index >= 0; Index-- )
        {
            if( ( flags & guRADIO_SEARCH_FLAG_NOWPLAYING ) )
            {
                if( stations[ Index ].m_NowPlaying.Lower().Find( text ) == wxNOT_FOUND )
                {
                    stations.RemoveAt( Index );
                    continue;
                }
            }

            if( ( flags & guRADIO_SEARCH_FLAG_GENRE ) )
            {
                if( stations[ Index ].m_GenreName.Lower().Find( text ) == wxNOT_FOUND )
                {
                    stations.RemoveAt( Index );
                    continue;
                }
            }

            if( ( flags & guRADIO_SEARCH_FLAG_STATION ) )
            {
                if( stations[ Index ].m_Name.Lower().Find( text ) == wxNOT_FOUND )
                {
                    stations.RemoveAt( Index );
                    continue;
                }
            }

            // Need to check if the station genre already existed
            if( !( flags & guRADIO_SEARCH_FLAG_ALLGENRES ) )
            {
                if( !guListItemsSearchName( RadioGenres, stations[ Index ].m_GenreName.Lower() ) )
                {
                    stations.RemoveAt( Index );
                }
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
guShoutcastUpdateThread::ExitCode guShoutcastUpdateThread::Entry()
{
//    guListItems Genres;
    int index;
    int count;
    wxCommandEvent event( wxEVT_MENU, ID_STATUSBAR_GAUGE_SETMAX );
    guShoutCast * ShoutCast = new guShoutCast();
    guRadioStations RadioStations;
    if( ShoutCast )
    {
        //
        guConfig * Config = ( guConfig * ) guConfig::Get();
        long MinBitRate;
        MinBitRate = Config->ReadNum( CONFIG_KEY_RADIOS_MIN_BITRATE, 128, CONFIG_PATH_RADIOS );

//        m_Db->GetRadioGenres( &Genres, false );
//        guLogMessage( wxT ( "Loaded the genres" ) );
        guListItems Genres;
        wxArrayInt  Flags;
        m_Db->GetRadioGenresList( m_Source, m_Ids, &Genres, &Flags );

        //
        m_Db->DelRadioStations( m_Source, m_Ids );
        //guLogMessage( wxT( "Deleted all radio stations" ) );
        count = Genres.Count();

        event.SetInt( m_GaugeId );
        event.SetExtraLong( count );
        wxPostEvent( guMainFrame::GetMainFrame(), event );

        for( index = 0; index < count; index++ )
        {
            guLogMessage( wxT( "Updating radiostations for genre '%s'" ), Genres[ index ].m_Name.c_str() );
            ShoutCast->GetStations( m_Source, Flags[ index ], Genres[ index ].m_Name, Genres[ index ].m_Id, &RadioStations, MinBitRate );

            if( m_Source == guRADIO_SOURCE_SHOUTCAST_SEARCH )
            {
                CheckRadioStationsFilters( Flags[ index ], Genres[ index ].m_Name.Lower(), RadioStations );
            }

            m_Db->SetRadioStations( &RadioStations );

            RadioStations.Clear();

            //wxCommandEvent event( wxEVT_MENU, ID_RADIO_UPDATED );
            event.SetId( ID_RADIO_UPDATED );
            wxPostEvent( m_RadioPanel, event );
            Sleep( 30 ); // Its wxThread::Sleep

//            wxCommandEvent event2( wxEVT_MENU, ID_STATUSBAR_GAUGE_UPDATE );
            event.SetId( ID_STATUSBAR_GAUGE_UPDATE );
            event.SetInt( m_GaugeId );
            event.SetExtraLong( index );
            wxPostEvent( guMainFrame::GetMainFrame(), event );
        }

        delete ShoutCast;
    }
//    wxCommandEvent event( wxEVT_MENU, ID_RADIO_UPDATE_END );
    event.SetId( ID_RADIO_UPDATE_END );
    wxPostEvent( m_RadioPanel, event );
//    wxMilliSleep( 1 );

//    wxCommandEvent event2( wxEVT_MENU, ID_STATUSBAR_GAUGE_REMOVE );
    event.SetId( ID_STATUSBAR_GAUGE_REMOVE );
    event.SetInt( m_GaugeId );
    wxPostEvent( guMainFrame::GetMainFrame(), event );
    //
    return 0;
}

}

// -------------------------------------------------------------------------------- //
