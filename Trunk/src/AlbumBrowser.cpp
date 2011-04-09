// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2011 J.Rios
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
#include "AlbumBrowser.h"

#include "Accelerators.h"
#include "Commands.h"
#include "Config.h"
#include "CoverEdit.h"
#include "Images.h"
#include "LabelEditor.h"
#include "LibPanel.h"
#include "LibUpdate.h"
#include "MainApp.h"
#include "MainFrame.h"
#include "OnlineLinks.h"
#include "SelCoverFile.h"
#include "StaticBitmap.h"
#include "ShowImage.h"
#include "TagInfo.h"
#include "TrackEdit.h"
#include "Utils.h"

#include <wx/arrimpl.cpp>
#include "wx/clipbrd.h"
#include <wx/dnd.h>
#include <wx/menu.h>

WX_DEFINE_OBJARRAY( guAlbumBrowserItemArray );

#define guALBUMBROWSER_REFRESH_DELAY            60
#define guALBUMBROWSER_GRID_SIZE_WIDTH          150
#define guALBUMBROWSER_GRID_SIZE_HEIGHT         180

#define guALBUMBROWSER_TIMER_ID_REFRESH         3
#define guALBUMBROWSER_TIMER_ID_TEXTSEARCH      4

#define guALBUMBROWSER_TIMER_TEXTSEARCH_DELAY   500

void AddAlbumCommands( wxMenu * Menu, int SelCount );

// -------------------------------------------------------------------------------- //
class guUpdateAlbumDetails : public wxThread
{
  protected :
    guAlbumBrowser *        m_AlbumBrowser;

  public :
    guUpdateAlbumDetails( guAlbumBrowser * albumbrowser );
    ~guUpdateAlbumDetails();

    virtual ExitCode Entry();

};

// -------------------------------------------------------------------------------- //
guUpdateAlbumDetails::guUpdateAlbumDetails( guAlbumBrowser * albumbrowser )
{
    m_AlbumBrowser = albumbrowser;

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guUpdateAlbumDetails::~guUpdateAlbumDetails()
{
    if( !TestDestroy() )
    {
        m_AlbumBrowser->ClearUpdateDetailsThread();
    }
}

// -------------------------------------------------------------------------------- //
guUpdateAlbumDetails::ExitCode guUpdateAlbumDetails::Entry()
{
    if( !TestDestroy() )
    {
        //guLogMessage( wxT( "Searching details..." ) );
        guDbLibrary * Db = m_AlbumBrowser->m_Db;
        int Index;
        m_AlbumBrowser->m_AlbumItemsMutex.Lock();
        int Count = m_AlbumBrowser->m_AlbumItems.Count();
        if( Count )
        {
            for( Index = 0; Index < Count; Index++ )
            {
                if( TestDestroy() )
                    break;

                if( m_AlbumBrowser->m_AlbumItems[ Index ].m_AlbumId != ( unsigned int ) wxNOT_FOUND )
                {
                    m_AlbumBrowser->m_AlbumItems[ Index ].m_Year = Db->GetAlbumYear( m_AlbumBrowser->m_AlbumItems[ Index ].m_AlbumId );
                    m_AlbumBrowser->m_AlbumItems[ Index ].m_TrackCount = Db->GetAlbumTrackCount( m_AlbumBrowser->m_AlbumItems[ Index ].m_AlbumId );

                    if( TestDestroy() )
                        break;

                    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_ALBUMBROWSER_UPDATEDETAILS );
                    event.SetInt( Index );
                    wxPostEvent( m_AlbumBrowser, event );
                    //guLogMessage( wxT( "Sent Details %i %i for %i" ), m_AlbumBrowser->m_AlbumItems[ Index ].m_Year, m_AlbumBrowser->m_AlbumItems[ Index ].m_TrackCount, Index );
                }
            }
        }
        m_AlbumBrowser->m_AlbumItemsMutex.Unlock();
    }
    return 0;
}


// -------------------------------------------------------------------------------- //
// guAlbumBrowserItemPanel
// -------------------------------------------------------------------------------- //
guAlbumBrowserItemPanel::guAlbumBrowserItemPanel( wxWindow * parent, const int index,
    guAlbumBrowserItem * albumitem ) : wxPanel( parent, wxID_ANY )
{
    m_AlbumBrowserItem = albumitem;
    m_AlbumBrowser = ( guAlbumBrowser * ) parent;

    wxFont CurrentFont = GetFont();
    // GUI
	m_MainSizer = new wxBoxSizer( wxVERTICAL );

	m_Bitmap = new guStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize( 100, 100 ), 0 );
	m_MainSizer->Add( m_Bitmap, 0, wxTOP|wxRIGHT|wxLEFT, 5 );

	//m_AlbumLabel = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
	//m_AlbumLabel->Wrap( 120 );
	m_AlbumLabel = new guAutoScrollText( this, wxEmptyString );
	CurrentFont.SetWeight( wxFONTWEIGHT_BOLD );
	CurrentFont.SetPointSize( CurrentFont.GetPointSize() - 1 );
	m_AlbumLabel->SetFont( CurrentFont );
	m_MainSizer->Add( m_AlbumLabel, 0, wxRIGHT|wxLEFT|wxEXPAND, 5 );

//	m_ArtistLabel = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
//	m_ArtistLabel->Wrap( 120 );
	m_ArtistLabel = new guAutoScrollText( this, wxEmptyString );
	CurrentFont.SetWeight( wxFONTWEIGHT_NORMAL );
	m_ArtistLabel->SetFont( CurrentFont );
	m_MainSizer->Add( m_ArtistLabel, 0, wxRIGHT|wxLEFT|wxEXPAND, 5 );

//	m_TracksLabel = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
//	m_TracksLabel->Wrap( 120 );
	m_TracksLabel = new guAutoScrollText( this, wxEmptyString );
	m_TracksLabel->SetFont( CurrentFont );
	m_MainSizer->Add( m_TracksLabel, 0, wxBOTTOM|wxRIGHT|wxLEFT|wxEXPAND, 5 );

	SetSizer( m_MainSizer );
	Layout();
	m_MainSizer->Fit( this );

    SetDropTarget( new guAlbumBrowserDropTarget( this ) );

    Connect( wxEVT_CONTEXT_MENU, wxContextMenuEventHandler( guAlbumBrowserItemPanel::OnContextMenu ), NULL, this );

    Connect( ID_ALBUM_COMMANDS, ID_ALBUM_COMMANDS + 99, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumBrowserItemPanel::OnCommandClicked ) );
    Connect( ID_LASTFM_SEARCH_LINK, ID_LASTFM_SEARCH_LINK + 999, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumBrowserItemPanel::OnSearchLinkClicked ) );

	m_Bitmap->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guAlbumBrowserItemPanel::OnAlbumDClicked ), NULL, this );

    Connect( ID_ALBUMBROWSER_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumBrowserItemPanel::OnPlayClicked ), NULL, this );
    Connect( ID_ALBUMBROWSER_ENQUEUE_AFTER_ALL, ID_ALBUMBROWSER_ENQUEUE_AFTER_ARTIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumBrowserItemPanel::OnEnqueueClicked ), NULL, this );
    Connect( ID_ALBUMBROWSER_EDITLABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumBrowserItemPanel::OnAlbumEditLabelsClicked ), NULL, this );
    Connect( ID_ALBUMBROWSER_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumBrowserItemPanel::OnAlbumEditTracksClicked ), NULL, this );
    Connect( ID_ALBUMBROWSER_COPYTOCLIPBOARD, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumBrowserItemPanel::OnCopyToClipboard ), NULL, this );
    Connect( ID_ALBUM_SELECTNAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumBrowserItemPanel::OnAlbumSelectName ), NULL, this );
    Connect( ID_ARTIST_SELECTNAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumBrowserItemPanel::OnArtistSelectName ), NULL, this );

    Connect( ID_ALBUMBROWSER_SEARCHCOVER, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumBrowserItemPanel::OnAlbumDownloadCoverClicked ), NULL, this );
    Connect( ID_ALBUMBROWSER_SELECTCOVER, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumBrowserItemPanel::OnAlbumSelectCoverClicked ), NULL, this );
    Connect( ID_ALBUMBROWSER_DELETECOVER, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumBrowserItemPanel::OnAlbumDeleteCoverClicked ), NULL, this );
    Connect( ID_ALBUMBROWSER_EMBEDCOVER, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumBrowserItemPanel::OnAlbumEmbedCoverClicked ), NULL, this );
    Connect( ID_ALBUMBROWSER_COPYTO, ID_ALBUMBROWSER_COPYTO + 199, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumBrowserItemPanel::OnAlbumCopyToClicked ), NULL, this );

	m_Bitmap->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( guAlbumBrowserItemPanel::OnMouse ), NULL, this );
	m_Bitmap->Connect( wxEVT_LEFT_UP, wxMouseEventHandler( guAlbumBrowserItemPanel::OnMouse ), NULL, this );
	m_Bitmap->Connect( wxEVT_MOTION, wxMouseEventHandler( guAlbumBrowserItemPanel::OnMouse ), NULL, this );

    Connect( ID_ALBUMBROWSER_BEGINDRAG, wxEVT_COMMAND_MENU_SELECTED, wxMouseEventHandler( guAlbumBrowserItemPanel::OnBeginDrag ), NULL, this );
    Connect( ID_ALBUMBROWSER_COVER_BEGINDRAG, wxEVT_COMMAND_MENU_SELECTED, wxMouseEventHandler( guAlbumBrowserItemPanel::OnCoverBeginDrag ), NULL, this );

    Connect( guEVT_STATICBITMAP_MOUSE_OVER, guStaticBitmapMouseOverEvent, wxCommandEventHandler( guAlbumBrowserItemPanel::OnBitmapMouseOver ), NULL, this );

}

// -------------------------------------------------------------------------------- //
guAlbumBrowserItemPanel::~guAlbumBrowserItemPanel()
{
    Disconnect( wxEVT_CONTEXT_MENU, wxContextMenuEventHandler( guAlbumBrowserItemPanel::OnContextMenu ), NULL, this );

    Disconnect( ID_ALBUM_COMMANDS, ID_ALBUM_COMMANDS + 99, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumBrowserItemPanel::OnCommandClicked ) );
    Disconnect( ID_LASTFM_SEARCH_LINK, ID_LASTFM_SEARCH_LINK + 999, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumBrowserItemPanel::OnSearchLinkClicked ) );

	m_Bitmap->Disconnect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guAlbumBrowserItemPanel::OnAlbumDClicked ), NULL, this );

    Disconnect( ID_ALBUMBROWSER_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumBrowserItemPanel::OnPlayClicked ), NULL, this );
    Disconnect( ID_ALBUMBROWSER_ENQUEUE_AFTER_ALL, ID_ALBUMBROWSER_ENQUEUE_AFTER_ARTIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumBrowserItemPanel::OnEnqueueClicked ), NULL, this );
    Disconnect( ID_ALBUMBROWSER_EDITLABELS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumBrowserItemPanel::OnAlbumEditLabelsClicked ), NULL, this );
    Disconnect( ID_ALBUMBROWSER_EDITTRACKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumBrowserItemPanel::OnAlbumEditTracksClicked ), NULL, this );
    Disconnect( ID_ALBUMBROWSER_COPYTOCLIPBOARD, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumBrowserItemPanel::OnCopyToClipboard ), NULL, this );
    Disconnect( ID_ALBUM_SELECTNAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumBrowserItemPanel::OnAlbumSelectName ), NULL, this );
    Disconnect( ID_ARTIST_SELECTNAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumBrowserItemPanel::OnArtistSelectName ), NULL, this );

    Disconnect( ID_ALBUMBROWSER_SEARCHCOVER, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumBrowserItemPanel::OnAlbumDownloadCoverClicked ), NULL, this );
    Disconnect( ID_ALBUMBROWSER_SELECTCOVER, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumBrowserItemPanel::OnAlbumSelectCoverClicked ), NULL, this );
    Disconnect( ID_ALBUMBROWSER_DELETECOVER, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumBrowserItemPanel::OnAlbumDeleteCoverClicked ), NULL, this );
    Disconnect( ID_ALBUMBROWSER_EMBEDCOVER, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumBrowserItemPanel::OnAlbumEmbedCoverClicked ), NULL, this );
    Disconnect( ID_ALBUMBROWSER_COPYTO, ID_ALBUMBROWSER_COPYTO + 199, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumBrowserItemPanel::OnAlbumCopyToClicked ), NULL, this );

	m_Bitmap->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( guAlbumBrowserItemPanel::OnMouse ), NULL, this );
	m_Bitmap->Disconnect( wxEVT_LEFT_UP, wxMouseEventHandler( guAlbumBrowserItemPanel::OnMouse ), NULL, this );
	m_Bitmap->Disconnect( wxEVT_MOTION, wxMouseEventHandler( guAlbumBrowserItemPanel::OnMouse ), NULL, this );

    Disconnect( ID_ALBUMBROWSER_BEGINDRAG, wxEVT_COMMAND_MENU_SELECTED, wxMouseEventHandler( guAlbumBrowserItemPanel::OnBeginDrag ), NULL, this );
    Disconnect( ID_ALBUMBROWSER_COVER_BEGINDRAG, wxEVT_COMMAND_MENU_SELECTED, wxMouseEventHandler( guAlbumBrowserItemPanel::OnCoverBeginDrag ), NULL, this );
}

//// -------------------------------------------------------------------------------- //
//wxString inline guCheckLabelString( const wxString &label )
//{
//    wxString Label = label;
//    Label.Replace( wxT( "&" ), wxT( "&&" ) );
//    return Label;
//}

// -------------------------------------------------------------------------------- //
void guAlbumBrowserItemPanel::SetAlbumItem( const int index, guAlbumBrowserItem * albumitem, wxBitmap * blankcd )
{
    m_Index = index;
    m_AlbumBrowserItem = albumitem;
    if( m_AlbumBrowserItem )
    {
        m_Bitmap->Show();
        //guLogMessage( wxT( "Inserting item %i '%s'" ), index, albumitem->m_AlbumName.c_str() );
        if( m_AlbumBrowserItem->m_CoverBitmap )
            m_Bitmap->SetBitmap( * m_AlbumBrowserItem->m_CoverBitmap );
        else
            m_Bitmap->SetBitmap( * blankcd );


        m_ArtistLabel->SetLabel( m_AlbumBrowserItem->m_ArtistName );
        m_ArtistLabel->SetToolTip( m_AlbumBrowserItem->m_ArtistName );
        m_AlbumLabel->SetLabel( m_AlbumBrowserItem->m_AlbumName );
        m_AlbumLabel->SetToolTip( m_AlbumBrowserItem->m_AlbumName );

        wxString Label;
        if( m_AlbumBrowserItem->m_Year )
        {
            Label += wxString::Format( wxT( "(%u) " ), m_AlbumBrowserItem->m_Year );
        }
        Label += wxString::Format( wxT( "%u " ), m_AlbumBrowserItem->m_TrackCount ) + _( "Tracks" );

        m_TracksLabel->SetLabel( Label );
        m_TracksLabel->SetToolTip( Label );
    }
    else
    {
        //m_Bitmap->SetBitmap( * blankcd );
        m_Bitmap->Hide();
        m_ArtistLabel->SetLabel( wxEmptyString );
        m_AlbumLabel->SetLabel( wxEmptyString );
        m_TracksLabel->SetLabel( wxEmptyString );
        m_ArtistLabel->SetToolTip( wxEmptyString );
        m_AlbumLabel->SetToolTip( wxEmptyString );
        m_TracksLabel->SetToolTip( wxEmptyString );
    }
    //Layout();
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowserItemPanel::UpdateDetails( void )
{
    if( m_AlbumBrowserItem )
    {
        wxString Label;
        if( m_AlbumBrowserItem->m_Year )
        {
            Label += wxString::Format( wxT( "(%u) " ), m_AlbumBrowserItem->m_Year );
        }
        Label += wxString::Format( wxT( "%u " ), m_AlbumBrowserItem->m_TrackCount ) + _( "Tracks" );

        m_TracksLabel->SetLabel( Label );
        m_MainSizer->Layout();
    }
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowserItemPanel::OnContextMenu( wxContextMenuEvent &event )
{
    if( !m_AlbumBrowserItem )
        return;

    m_Bitmap->StopTimer();

    wxMenu Menu;
    wxMenuItem * MenuItem;
    wxPoint Point = event.GetPosition();
    // If from keyboard
    if( Point.x == -1 && Point.y == -1 )
    {
        wxSize Size = GetSize();
        Point.x = Size.x / 2;
        Point.y = Size.y / 2;
    }
    else
    {
        Point = ScreenToClient( Point );
    }


    if( m_AlbumBrowserItem->m_AlbumId != ( size_t ) wxNOT_FOUND )
    {
        int ContextMenuFlags = m_AlbumBrowser->GetContextMenuFlags();

        MenuItem = new wxMenuItem( &Menu, ID_ALBUMBROWSER_PLAY, _( "Play" ), _( "Play the album tracks" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
        Menu.Append( MenuItem );

        MenuItem = new wxMenuItem( &Menu, ID_ALBUMBROWSER_ENQUEUE_AFTER_ALL, _( "Enqueue" ), _( "Enqueue the album tracks to the playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        Menu.Append( MenuItem );

        wxMenu * EnqueueMenu = new wxMenu();

        MenuItem = new wxMenuItem( EnqueueMenu, ID_ALBUMBROWSER_ENQUEUE_AFTER_TRACK,
                                wxString( _( "Current Track" ) ) +  guAccelGetCommandKeyCodeString( ID_SONG_ENQUEUE_AFTER_TRACK ),
                                _( "Add current selected tracks to playlist after the current track" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );

        MenuItem = new wxMenuItem( EnqueueMenu, ID_ALBUMBROWSER_ENQUEUE_AFTER_ALBUM,
                                wxString( _( "Current Album" ) ) +  guAccelGetCommandKeyCodeString( ID_SONG_ENQUEUE_AFTER_ALBUM ),
                                _( "Add current selected tracks to playlist after the current album" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );

        MenuItem = new wxMenuItem( EnqueueMenu, ID_ALBUMBROWSER_ENQUEUE_AFTER_ARTIST,
                                wxString( _( "Current Artist" ) ) +  guAccelGetCommandKeyCodeString( ID_SONG_ENQUEUE_AFTER_ARTIST ),
                                _( "Add current selected tracks to playlist after the current artist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );

        Menu.Append( wxID_ANY, _( "Enqueue after" ), EnqueueMenu );

        Menu.AppendSeparator();

        MenuItem = new wxMenuItem( &Menu, ID_ALBUMBROWSER_EDITLABELS, _( "Edit Labels" ), _( "Edit the labels assigned to the selected albums" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tags ) );
        Menu.Append( MenuItem );

        if( ContextMenuFlags & guLIBRARY_CONTEXTMENU_EDIT_TRACKS )
        {
            MenuItem = new wxMenuItem( &Menu, ID_ALBUMBROWSER_EDITTRACKS, _( "Edit Album songs" ), _( "Edit the selected albums songs" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit ) );
            Menu.Append( MenuItem );
        }

        Menu.AppendSeparator();

        MenuItem = new wxMenuItem( &Menu, ID_ALBUM_SELECTNAME, _( "Search Album" ), _( "Search the album in the library" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_search ) );
        Menu.Append( MenuItem );

        MenuItem = new wxMenuItem( &Menu, ID_ARTIST_SELECTNAME, _( "Search Artist" ), _( "Search the artist in the library" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_search ) );
        Menu.Append( MenuItem );

        if( ContextMenuFlags & guLIBRARY_CONTEXTMENU_DOWNLOAD_COVERS )
        {
            Menu.AppendSeparator();

            MenuItem = new wxMenuItem( &Menu, ID_ALBUMBROWSER_SEARCHCOVER, _( "Download Album cover" ), _( "Download cover for the current selected album" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_download_covers ) );
            Menu.Append( MenuItem );

            MenuItem = new wxMenuItem( &Menu, ID_ALBUMBROWSER_SELECTCOVER, _( "Select cover location" ), _( "Select the cover image file from disk" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_download_covers ) );
            Menu.Append( MenuItem );

            MenuItem = new wxMenuItem( &Menu, ID_ALBUMBROWSER_DELETECOVER, _( "Delete Album cover" ), _( "Delete the cover for the selected album" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_delete ) );
            Menu.Append( MenuItem );
        }

        if( ContextMenuFlags & guLIBRARY_CONTEXTMENU_EMBED_COVERS )
        {
            MenuItem = new wxMenuItem( &Menu, ID_ALBUMBROWSER_EMBEDCOVER, _( "Embed Album cover" ), _( "Embed the cover to the selected album tracks" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_doc_save ) );
            Menu.Append( MenuItem );
        }

        Menu.AppendSeparator();

        MenuItem = new wxMenuItem( &Menu, ID_ALBUMBROWSER_COPYTOCLIPBOARD, _( "Copy to clipboard" ), _( "Copy the album info to clipboard" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit_copy ) );
        Menu.Append( MenuItem );


        if( ( ContextMenuFlags & guLIBRARY_CONTEXTMENU_COPY_TO ) ||
            ( ContextMenuFlags & guLIBRARY_CONTEXTMENU_LINKS ) ||
            ( ContextMenuFlags & guLIBRARY_CONTEXTMENU_COMMANDS ) )
        {
            Menu.AppendSeparator();

            if( ContextMenuFlags & guLIBRARY_CONTEXTMENU_COPY_TO )
            {
                guMainFrame * MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
                MainFrame->CreateCopyToMenu( &Menu, ID_ALBUMBROWSER_COPYTO );
            }

            if( ContextMenuFlags & guLIBRARY_CONTEXTMENU_LINKS )
            {
                AddOnlineLinksMenu( &Menu );
            }

            if( ContextMenuFlags & guLIBRARY_CONTEXTMENU_COMMANDS )
            {
                AddAlbumCommands( &Menu, 1 );
            }
        }
        m_AlbumBrowser->CreateContextMenu( &Menu );
    }

    // Add Links and Commands
    PopupMenu( &Menu, Point.x, Point.y );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowserItemPanel::OnCommandClicked( wxCommandEvent &event )
{
    m_AlbumBrowser->OnCommandClicked( event.GetId(), m_AlbumBrowserItem->m_AlbumId );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowserItemPanel::OnSearchLinkClicked( wxCommandEvent &event )
{
    int index = event.GetId();

    guConfig * Config = ( guConfig * ) Config->Get();
    if( Config )
    {
        wxArrayString Links = Config->ReadAStr( wxT( "Link" ), wxEmptyString, wxT( "SearchLinks" ) );

        index -= ID_LASTFM_SEARCH_LINK;
        wxString SearchLink = Links[ index ];
        wxString Lang = Config->ReadStr( wxT( "Language" ), wxT( "en" ), wxT( "LastFM" ) );
        if( Lang.IsEmpty() )
        {
            Lang = ( ( guMainApp * ) wxTheApp )->GetLocale()->GetCanonicalName().Mid( 0, 2 );
            //guLogMessage( wxT( "Locale: %s" ), ( ( guMainApp * ) wxTheApp )->GetLocale()->GetCanonicalName().c_str() );
        }
        SearchLink.Replace( wxT( "{lang}" ), Lang );
        SearchLink.Replace( wxT( "{text}" ), guURLEncode( m_AlbumBrowserItem->m_ArtistName + wxT( " " ) +
                                                          m_AlbumBrowserItem->m_AlbumName ) );
        guWebExecute( SearchLink );
    }
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowserItemPanel::OnPlayClicked( wxCommandEvent &event )
{
    m_AlbumBrowser->SelectAlbum( m_AlbumBrowserItem->m_AlbumId, false );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowserItemPanel::OnEnqueueClicked( wxCommandEvent &event )
{
    m_AlbumBrowser->SelectAlbum( m_AlbumBrowserItem->m_AlbumId, true, event.GetId() - ID_ALBUMBROWSER_ENQUEUE_AFTER_ALL );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowserItemPanel::OnAlbumDClicked( wxMouseEvent &event )
{
    if( m_AlbumBrowserItem )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        m_AlbumBrowser->SelectAlbum( m_AlbumBrowserItem->m_AlbumId,
            Config->ReadBool( wxT( "DefaultActionEnqueue" ), false , wxT( "General" ) ) );
    }
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowserItemPanel::OnCopyToClipboard( wxCommandEvent &event )
{
    wxTheClipboard->UsePrimarySelection( false );
    if( wxTheClipboard->Open() )
    {
        wxTheClipboard->Clear();
        if( !wxTheClipboard->AddData( new wxTextDataObject(
            m_AlbumBrowserItem->m_ArtistName + wxT( " " ) + m_AlbumBrowserItem->m_AlbumName ) ) )
        {
            guLogError( wxT( "Can't copy data to the clipboard" ) );
        }
        wxTheClipboard->Close();
    }
    else
    {
        guLogError( wxT( "Could not open the clipboard object" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowserItemPanel::OnAlbumSelectName( wxCommandEvent &event )
{
    m_AlbumBrowser->OnAlbumSelectName( m_AlbumBrowserItem->m_AlbumId );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowserItemPanel::OnArtistSelectName( wxCommandEvent &event )
{
    m_AlbumBrowser->OnArtistSelectName( m_AlbumBrowserItem->m_ArtistId );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowserItemPanel::OnAlbumDownloadCoverClicked( wxCommandEvent &event )
{
    m_AlbumBrowser->OnAlbumDownloadCoverClicked( m_AlbumBrowserItem->m_AlbumId );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowserItemPanel::OnAlbumSelectCoverClicked( wxCommandEvent &event )
{
    m_AlbumBrowser->OnAlbumSelectCoverClicked( m_AlbumBrowserItem->m_AlbumId );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowserItemPanel::OnAlbumDeleteCoverClicked( wxCommandEvent &event )
{
    m_AlbumBrowser->OnAlbumDeleteCoverClicked( m_AlbumBrowserItem->m_AlbumId );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowserItemPanel::OnAlbumEmbedCoverClicked( wxCommandEvent &event )
{
    m_AlbumBrowser->OnAlbumEmbedCoverClicked( m_AlbumBrowserItem->m_AlbumId );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowserItemPanel::OnAlbumCopyToClicked( wxCommandEvent &event )
{
    m_AlbumBrowser->OnAlbumCopyToClicked( m_AlbumBrowserItem->m_AlbumId, event.GetId() );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowserItemPanel::OnAlbumEditLabelsClicked( wxCommandEvent &event )
{
    m_AlbumBrowser->OnAlbumEditLabelsClicked( m_AlbumBrowserItem );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowserItemPanel::OnAlbumEditTracksClicked( wxCommandEvent &event )
{
    m_AlbumBrowser->OnAlbumEditTracksClicked( m_AlbumBrowserItem->m_AlbumId );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowserItemPanel::OnMouse( wxMouseEvent &event )
{
    if( event.Dragging() )
    {
        //guLogMessage( wxT( "OnMouse...%i  %i  %i" ), m_DragCount, event.Dragging(), event.ControlDown() );
        if( !m_DragCount )
        {
            m_DragStart = event.GetPosition();
        }

        if( ++m_DragCount == 3 )
        {
            wxCommandEvent DragEvent( wxEVT_COMMAND_MENU_SELECTED, ID_ALBUMBROWSER_BEGINDRAG );
            if( event.ControlDown() )
            {
                DragEvent.SetId( ID_ALBUMBROWSER_COVER_BEGINDRAG );
            }
            DragEvent.SetEventObject( this );
            //DragEvent.SetInt( m_AlbumBrowserItem->m_CoverId );
            GetEventHandler()->ProcessEvent( DragEvent );
        }
        return;
    }
    else
    {
      m_DragCount = 0;
    }

    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowserItemPanel::OnBeginDrag( wxMouseEvent &event )
{
    if( !m_AlbumBrowserItem )
        return;

    int Index;
    int Count;
    //guLogMessage( wxT( "On BeginDrag event..." ) );
    guTrackArray Tracks;

    m_AlbumBrowser->GetAlbumTracks( m_AlbumBrowserItem->m_AlbumId, &Tracks );
    if( ( Count = Tracks.Count() ) )
    {
        m_AlbumBrowser->NormalizeTracks( &Tracks, true );
        wxFileDataObject Files;
        for( Index = 0; Index < Count; Index++ )
        {
            Files.AddFile( Tracks[ Index ].m_FileName );
        }

        wxDropSource source( Files, this );

        wxDragResult Result = source.DoDragDrop( wxDrag_CopyOnly );
        if( Result )
        {
        }
    }
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowserItemPanel::OnCoverBeginDrag( wxMouseEvent &event )
{
    if( !m_AlbumBrowserItem )
        return;

    //guLogMessage( wxT( "On BeginDrag event..." ) );
    wxFileDataObject Files;

    wxString CoverFile = m_AlbumBrowser->GetAlbumCoverFile( m_AlbumBrowserItem->m_CoverId );
    if( !CoverFile.IsEmpty() )
    {
        Files.AddFile( CoverFile );

        wxDropSource source( Files, this );

        wxDragResult Result = source.DoDragDrop( wxDrag_CopyOnly );
        if( Result )
        {
        }
    }
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowserItemPanel::SetAlbumCover( const wxString &cover )
{
    if( m_AlbumBrowserItem )
        m_AlbumBrowser->SetAlbumCover( m_AlbumBrowserItem->m_AlbumId, cover );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowserItemPanel::OnBitmapMouseOver( wxCommandEvent &event )
{
    if( m_AlbumBrowserItem )
        m_AlbumBrowser->OnBitmapMouseOver( m_AlbumBrowserItem->m_CoverId, ClientToScreen( m_Bitmap->GetPosition() ) );
}




// -------------------------------------------------------------------------------- //
// guAlbumBrowser
// -------------------------------------------------------------------------------- //
guAlbumBrowser::guAlbumBrowser( wxWindow * parent, guDbLibrary * db, guPlayerPanel * playerpanel ) :
    wxPanel( parent, wxID_ANY ),
    m_RefreshTimer( this, guALBUMBROWSER_TIMER_ID_REFRESH ),
    m_TextChangedTimer( this, guALBUMBROWSER_TIMER_ID_TEXTSEARCH )
{
    m_Db = db;
    m_PlayerPanel = playerpanel;
    m_UpdateDetails = NULL;
    m_ItemStart = 0;
    m_LastItemStart = wxNOT_FOUND;
    m_ItemCount = 1;
    m_BlankCD = new wxBitmap( guImage( guIMAGE_INDEX_no_cover ) );
    m_ContextMenuFlags = guLIBRARY_CONTEXTMENU_DEFAULT;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->RegisterObject( this );

    m_InstantSearchEnabled = Config->ReadBool( wxT( "InstantTextSearchEnabled" ), true, wxT( "General" ) );
    m_EnterSelectSearchEnabled = !Config->ReadBool( wxT( "TextSearchEnterRelax" ), false, wxT( "General" ) );
    int FilterSelected = Config->ReadNum( wxT( "Filter" ), 0, wxT( "AlbumBrowser" ) );
    m_DynFilterArray = Config->ReadAStr( wxT( "Filter"), wxEmptyString, wxT( "AlbumBrowserFilters") );

    if( FilterSelected > ( int ) m_DynFilterArray.Count() )
        FilterSelected = 0;

    if( FilterSelected )
        m_DynFilter.FromString( m_DynFilterArray[ FilterSelected - 1 ].AfterFirst( wxT( ':' ) ) );

    // GUI
	SetMinSize( wxSize( 120, 150 ) );

	wxBoxSizer* MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

	m_FilterSizer = new wxBoxSizer( wxHORIZONTAL );

	wxArrayString m_FilterNames;
	m_FilterNames.Add( _( "All Albums" ) );

	int Index;
	int Count = m_DynFilterArray.Count();
	for( Index = 0; Index < Count; Index++ )
	{
	    m_FilterNames.Add( unescape_configlist_str( m_DynFilterArray[ Index ].BeforeFirst( wxT( ':' ) ) ) );
	}

	m_FilterChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_FilterNames, 0 );
	m_FilterChoice->SetSelection( FilterSelected );
	//m_FilterChoice->SetMinSize( wxSize( 100, -1 ) );
	m_FilterSizer->Add( m_FilterChoice, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxLEFT, 5 );

	m_AddFilterButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_FilterSizer->Add( m_AddFilterButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP, 5 );

	m_DelFilterButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_del ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_DelFilterButton->Enable( FilterSelected );
	m_FilterSizer->Add( m_DelFilterButton, 0, wxTOP|wxALIGN_CENTER_VERTICAL, 5 );

	m_EditFilterButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_EditFilterButton->Enable( FilterSelected );
	m_FilterSizer->Add( m_EditFilterButton, 0, wxTOP|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	m_SearchTextCtrl = new wxSearchCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	m_FilterSizer->Add( m_SearchTextCtrl, 1, wxTOP|wxLEFT| wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	wxString m_OrderChoiceChoices[] = { _("Order By"), _("Name"), _("Year"), _("Year Desc."), _("Artist, Name"), _("Artist, Year"), _("Artist, Year Desc."), _( "Added time" ) };
	int m_OrderChoiceNChoices = sizeof( m_OrderChoiceChoices ) / sizeof( wxString );
	m_OrderChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_OrderChoiceNChoices, m_OrderChoiceChoices, 0 );
	m_OrderChoice->SetSelection( Config->ReadNum( wxT( "Sort" ), 0, wxT( "AlbumBrowser" ) ) );
	m_FilterSizer->Add( m_OrderChoice, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

	MainSizer->Add( m_FilterSizer, 0, wxEXPAND, 5 );


	m_AlbumsSizer = new wxGridSizer( 1, 1, 0, 0 );

	m_ItemPanels.Add( new guAlbumBrowserItemPanel( this, 0 ) );

	m_AlbumsSizer->Add( m_ItemPanels[ 0 ], 1, wxEXPAND|wxALL, 5 );

	MainSizer->Add( m_AlbumsSizer, 1, wxEXPAND, 5 );

	wxBoxSizer* NavigatorSizer;
	NavigatorSizer = new wxBoxSizer( wxVERTICAL );

	m_NavLabel = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_NavLabel->Wrap( -1 );
	NavigatorSizer->Add( m_NavLabel, 0, wxRIGHT|wxLEFT|wxEXPAND, 5 );

	m_NavSlider = new wxSlider( this, wxID_ANY, 0, 0, 1, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	m_NavSlider->SetFocus();
	NavigatorSizer->Add( m_NavSlider, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	MainSizer->Add( NavigatorSizer, 0, wxEXPAND, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

    Connect( wxEVT_SIZE, wxSizeEventHandler( guAlbumBrowser::OnChangedSize ), NULL, this );

	m_FilterChoice->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( guAlbumBrowser::OnFilterSelected ), NULL, this );
	m_AddFilterButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guAlbumBrowser::OnAddFilterClicked ), NULL, this );
	m_DelFilterButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guAlbumBrowser::OnDelFilterClicked ), NULL, this );
	m_EditFilterButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guAlbumBrowser::OnEditFilterClicked ), NULL, this );
	m_OrderChoice->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( guAlbumBrowser::OnOrderSelected ), NULL, this );

	m_NavSlider->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guAlbumBrowser::OnChangingPosition ), NULL, this );
	m_NavSlider->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( guAlbumBrowser::OnChangingPosition ), NULL, this );
//	m_NavSlider->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( guAlbumBrowser::OnChangingPosition ), NULL, this );
//	m_NavSlider->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( guAlbumBrowser::OnChangingPosition ), NULL, this );
//	m_NavSlider->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( guAlbumBrowser::OnChangingPosition ), NULL, this );
//	m_NavSlider->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( guAlbumBrowser::OnChangingPosition ), NULL, this );
//	m_NavSlider->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( guAlbumBrowser::OnChangingPosition ), NULL, this );
//	m_NavSlider->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( guAlbumBrowser::OnChangingPosition ), NULL, this );
//	m_NavSlider->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( guAlbumBrowser::OnChangingPosition ), NULL, this );

	Connect( guALBUMBROWSER_TIMER_ID_REFRESH, wxEVT_TIMER, wxTimerEventHandler( guAlbumBrowser::OnRefreshTimer ), NULL, this );

	Connect( ID_ALBUMBROWSER_UPDATEDETAILS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumBrowser::OnUpdateDetails ), NULL, this );

	Connect( wxEVT_MOUSEWHEEL, wxMouseEventHandler( guAlbumBrowser::OnMouseWheel ) );

    m_SearchTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( guAlbumBrowser::OnSearchSelected ), NULL, this );
    m_SearchTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guAlbumBrowser::OnSearchTextChanged ), NULL, this );
    m_SearchTextCtrl->Connect( wxEVT_COMMAND_SEARCHCTRL_CANCEL_BTN, wxCommandEventHandler( guAlbumBrowser::OnSearchCancelled ), NULL, this );

	Connect( guALBUMBROWSER_TIMER_ID_TEXTSEARCH, wxEVT_TIMER, wxTimerEventHandler( guAlbumBrowser::OnTextChangedTimer ), NULL, this );

	Connect( ID_ALBUMBROWSER_SEARCH, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumBrowser::OnGoToSearch ), NULL, this );
    Connect( ID_CONFIG_UPDATED, guConfigUpdatedEvent, wxCommandEventHandler( guAlbumBrowser::OnConfigUpdated ), NULL, this );

    CreateAcceleratorTable();

    RefreshCount();
    //ReloadItems();
    //m_RefreshTimer.SetOwner( this );
}

// -------------------------------------------------------------------------------- //
guAlbumBrowser::~guAlbumBrowser()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->UnRegisterObject( this );

    Config->WriteNum( wxT( "Filter" ), m_FilterChoice->GetSelection(), wxT( "AlbumBrowser" ) );
    Config->WriteAStr( wxT( "Filter"), m_DynFilterArray, wxT( "AlbumBrowserFilters") );
    Config->WriteNum( wxT( "Sort" ), m_OrderChoice->GetSelection(), wxT( "AlbumBrowser" ) );

    if( m_BlankCD )
        delete m_BlankCD;

    Disconnect( wxEVT_SIZE, wxSizeEventHandler( guAlbumBrowser::OnChangedSize ), NULL, this );

	m_FilterChoice->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( guAlbumBrowser::OnFilterSelected ), NULL, this );
	m_AddFilterButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guAlbumBrowser::OnAddFilterClicked ), NULL, this );
	m_DelFilterButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guAlbumBrowser::OnDelFilterClicked ), NULL, this );
	m_EditFilterButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guAlbumBrowser::OnEditFilterClicked ), NULL, this );
	m_OrderChoice->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( guAlbumBrowser::OnOrderSelected ), NULL, this );

	m_NavSlider->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( guAlbumBrowser::OnChangingPosition ), NULL, this );
	m_NavSlider->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( guAlbumBrowser::OnChangingPosition ), NULL, this );
//	m_NavSlider->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( guAlbumBrowser::OnChangingPosition ), NULL, this );
//	m_NavSlider->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( guAlbumBrowser::OnChangingPosition ), NULL, this );
//	m_NavSlider->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( guAlbumBrowser::OnChangingPosition ), NULL, this );
//	m_NavSlider->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( guAlbumBrowser::OnChangingPosition ), NULL, this );
//	m_NavSlider->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( guAlbumBrowser::OnChangingPosition ), NULL, this );
//	m_NavSlider->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( guAlbumBrowser::OnChangingPosition ), NULL, this );
//	m_NavSlider->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( guAlbumBrowser::OnChangingPosition ), NULL, this );

	Disconnect( wxEVT_TIMER, wxTimerEventHandler( guAlbumBrowser::OnRefreshTimer ), NULL, this );

	Disconnect( ID_ALBUMBROWSER_UPDATEDETAILS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumBrowser::OnUpdateDetails ), NULL, this );

	Disconnect( wxEVT_MOUSEWHEEL, wxMouseEventHandler( guAlbumBrowser::OnMouseWheel ) );

}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnConfigUpdated( wxCommandEvent &event )
{
    int Flags = event.GetInt();
    if( Flags & guPREFERENCE_PAGE_FLAG_ACCELERATORS )
    {
        CreateAcceleratorTable();
    }
    if( Flags & guPREFERENCE_PAGE_FLAG_GENERAL )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        m_InstantSearchEnabled = Config->ReadBool( wxT( "InstantTextSearchEnabled" ), true, wxT( "General" ) );
        m_EnterSelectSearchEnabled = !Config->ReadBool( wxT( "TextSearchEnterRelax" ), false, wxT( "General" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::CreateAcceleratorTable( void )
{
    wxAcceleratorTable AccelTable;
    wxArrayInt AliasAccelCmds;
    wxArrayInt RealAccelCmds;

    AliasAccelCmds.Add( ID_PLAYER_PLAYLIST_SEARCH );

    RealAccelCmds.Add( ID_ALBUMBROWSER_SEARCH );

    if( guAccelDoAcceleratorTable( AliasAccelCmds, RealAccelCmds, AccelTable ) )
    {
        SetAcceleratorTable( AccelTable );
    }
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnGoToSearch( wxCommandEvent &event )
{
    if( FindFocus() != m_SearchTextCtrl )
        m_SearchTextCtrl->SetFocus();
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnChangedSize( wxSizeEvent &event )
{
    wxSize Size = event.GetSize();
    if( Size != m_LastSize )
    {
        size_t ColItems = Size.GetWidth() / guALBUMBROWSER_GRID_SIZE_WIDTH;
        size_t RowItems = Size.GetHeight() / guALBUMBROWSER_GRID_SIZE_HEIGHT;
        //guLogMessage( wxT( "Row: %i  Col:%i" ), RowItems, ColItems );
        if( ColItems * RowItems != m_ItemPanels.Count() )
        {
            size_t OldCount = m_ItemPanels.Count();
            m_ItemCount = ColItems * RowItems;
            //guLogMessage( wxT( "We need to reassign the panels from %i to %i" ), OldCount, m_ItemCount );
            if( OldCount != m_ItemCount )
            {
                m_AlbumItemsMutex.Lock();

                m_AlbumsSizer->SetCols( ColItems );
                m_AlbumsSizer->SetRows( RowItems );

                if( OldCount < m_ItemCount )
                {
                    while( m_ItemPanels.Count() < m_ItemCount )
                    {
                        int Index = m_ItemPanels.Count();
                        m_ItemPanels.Add( new guAlbumBrowserItemPanel( this, Index ) );
                        m_AlbumsSizer->Add( m_ItemPanels[ Index ], 1, wxEXPAND|wxALL, 5 );
                    }
                }
                else //if( m_ItemCount < OldCount )
                {
                    size_t Index;
                    //size_t Count;
                    for( Index = OldCount; Index > m_ItemCount; Index-- )
                    {
                        guAlbumBrowserItemPanel *  ItemPanel = m_ItemPanels[ Index - 1 ];
                        m_AlbumsSizer->Detach( ItemPanel );
                        m_ItemPanels.RemoveAt( Index - 1 );
                        delete ItemPanel;
                    }
                }
                m_AlbumItemsMutex.Unlock();

                RefreshPageCount();
                ReloadItems();
                RefreshAll();
            }
        }

        m_LastSize = Size;
    }
    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnSearchTextChanged( wxCommandEvent& event )
{
    if( m_TextChangedTimer.IsRunning() )
        m_TextChangedTimer.Stop();

    if( !m_InstantSearchEnabled )
        return;

    m_TextChangedTimer.Start( guALBUMBROWSER_TIMER_TEXTSEARCH_DELAY, wxTIMER_ONE_SHOT );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnSearchCancelled( wxCommandEvent &event ) // CLEAN SEARCH STR
{
    m_SearchTextCtrl->Clear();

    if( !m_InstantSearchEnabled )
        DoTextSearch();
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnSearchSelected( wxCommandEvent& event )
{
    if( m_TextChangedTimer.IsRunning() )
        m_TextChangedTimer.Stop();

    if( !DoTextSearch() || !m_EnterSelectSearchEnabled || !m_InstantSearchEnabled )
        return;

    guTrackArray Tracks;
    wxArrayInt Selections;
    int Index;
    int Count = m_AlbumItems.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        Selections.Add( m_AlbumItems[ Index ].m_AlbumId );
    }

    if( m_Db->GetAlbumsSongs( Selections, &Tracks ) && Tracks.Count() )
    {
        NormalizeTracks( &Tracks );
        guConfig * Config = ( guConfig * ) guConfig::Get();
        if( Config )
        {
            if( Config->ReadBool( wxT( "DefaultActionEnqueue" ), false, wxT( "General" ) ) )
            {
                m_PlayerPanel->AddToPlayList( Tracks, true );
            }
            else
            {
                m_PlayerPanel->SetPlayList( Tracks );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
bool guAlbumBrowser::DoTextSearch( void )
{
    wxString SearchString = m_SearchTextCtrl->GetLineText( 0 );
    if( !SearchString.IsEmpty() )
    {
        if( SearchString.Length() > 1 )
        {
            m_TextSearchFilter = guSplitWords( SearchString );

            RefreshCount();
            ReloadItems();
            m_LastItemStart = wxNOT_FOUND;
            m_NavSlider->SetValue( 0 );
            RefreshAll();

            m_SearchTextCtrl->ShowCancelButton( true );
        }
        return true;
    }
    else
    {
        m_TextSearchFilter.Clear();

        RefreshCount();
        ReloadItems();
        m_LastItemStart = wxNOT_FOUND;
        m_NavSlider->SetValue( 0 );
        RefreshAll();

        m_SearchTextCtrl->ShowCancelButton( false );
    }
    return false;
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnTextChangedTimer( wxTimerEvent &event )
{
    DoTextSearch();
}



// -------------------------------------------------------------------------------- //
void guAlbumBrowser::ReloadItems( void )
{
    m_AlbumItemsMutex.Lock();
    m_AlbumItems.Empty();
    m_Db->GetAlbums( &m_AlbumItems, m_FilterChoice->GetSelection() ? &m_DynFilter : NULL,
            m_TextSearchFilter, m_ItemStart, m_ItemCount, m_OrderChoice->GetSelection() - 1 );
    m_AlbumItemsMutex.Unlock();
//    guLogMessage( wxT( "Read %i items from %i (%i)" ), m_AlbumItems.Count(), m_ItemStart, m_ItemCount );
//    RefreshAll();
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::RefreshAll( void )
{
    m_UpdateDetailsMutex.Lock();
    if( m_UpdateDetails )
    {
        m_UpdateDetails->Pause();
        m_UpdateDetails->Delete();
    }

    m_UpdateDetails = new guUpdateAlbumDetails( this );

    m_UpdateDetailsMutex.Unlock();

    m_AlbumItemsMutex.Lock();
    size_t Index;
    size_t Count = m_ItemCount;
    for( Index = 0; Index < Count; Index++ )
    {
        //guLogMessage( wxT( "%i %s " ), Index, m_AlbumItems[ Index ].m_AlbumName.c_str() );
        if( Index < m_AlbumItems.Count() )
            m_ItemPanels[ Index ]->SetAlbumItem( Index, &m_AlbumItems[ Index ], m_BlankCD );
        else
            m_ItemPanels[ Index ]->SetAlbumItem( Index, NULL, m_BlankCD );
    }
    m_AlbumItemsMutex.Unlock();
    Layout();
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnChangingPosition( wxScrollEvent& event )
{
    int CurPage = event.GetPosition(); //( ( event.GetPosition() * m_PagesCount ) / 100 );
    m_ItemStart = CurPage * m_ItemCount;
    //guLogMessage( wxT( "ChangePosition: %i -> %i     Albums(%i / %i)" ), m_ItemStart, m_LastItemStart, CurPage, m_AlbumsCount );
    if( m_LastItemStart != m_ItemStart )
    {
        if( m_RefreshTimer.IsRunning() )
            m_RefreshTimer.Stop();

        m_RefreshTimer.Start( guALBUMBROWSER_REFRESH_DELAY, wxTIMER_ONE_SHOT );

        UpdateNavLabel( CurPage );
//        ReloadItems();
//        RefreshAll();

        m_LastItemStart = m_ItemStart;
    }
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnRefreshTimer( wxTimerEvent &event )
{
    ReloadItems();
    RefreshAll();
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::SelectAlbum( const int albumid, const bool append, const int aftercurrent )
{
    guTrackArray Tracks;
    wxArrayInt Selections;
    Selections.Add( albumid );
    if( m_Db->GetAlbumsSongs( Selections, &Tracks ) )
    {
        NormalizeTracks( &Tracks );
        if( append )
            m_PlayerPanel->AddToPlayList( Tracks, true, aftercurrent );
        else
            m_PlayerPanel->SetPlayList( Tracks );
    }
}

// -------------------------------------------------------------------------------- //
int guAlbumBrowser::GetAlbumTracks( const int albumid, guTrackArray * tracks )
{
    wxArrayInt Albums;
    Albums.Add( albumid );
    return m_Db->GetAlbumsSongs( Albums, tracks );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnAddFilterClicked( wxCommandEvent &event )
{
    guDynPlayList NewPlayList;

    guDynPlayListEditor * PlayListEditor = new guDynPlayListEditor( this, &NewPlayList, true );
    if( PlayListEditor->ShowModal() == wxID_OK )
    {
        PlayListEditor->FillPlayListEditData();

        wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( this, _( "Filter Name: " ),
          _( "Enter the new filter name" ), _( "New Filter" ) );
        if( EntryDialog->ShowModal() == wxID_OK )
        {
            m_DynFilterArray.Add( escape_configlist_str( EntryDialog->GetValue() ) + wxT( ":" ) + NewPlayList.ToString() );
            m_FilterChoice->Append( EntryDialog->GetValue() );
            m_FilterSizer->Layout();

            m_FilterChoice->SetSelection( m_FilterChoice->GetCount() - 1 );
            event.SetInt( m_FilterChoice->GetCount() - 1 );
            OnFilterSelected( event );
        }
        EntryDialog->Destroy();
    }

    PlayListEditor->Destroy();
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnDelFilterClicked( wxCommandEvent &event )
{
    int Selected = m_FilterChoice->GetSelection();
    if( Selected )
    {
        m_DynFilterArray.RemoveAt( Selected  - 1 );
        m_FilterChoice->Delete( Selected );
        m_FilterSizer->Layout();

        m_FilterChoice->SetSelection( 0 );
        event.SetInt( 0 );
        OnFilterSelected( event );
    }
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnEditFilterClicked( wxCommandEvent &event )
{
    int Selected = m_FilterChoice->GetSelection();
    wxString CurFilterStr = m_DynFilterArray[ Selected - 1 ];

    guDynPlayList EditPlayList = m_DynFilter;

    guDynPlayListEditor * PlayListEditor = new guDynPlayListEditor( this, &EditPlayList, true );
    if( PlayListEditor->ShowModal() == wxID_OK )
    {
        PlayListEditor->FillPlayListEditData();

        m_DynFilter = EditPlayList;

        if( m_DynFilter.m_Filters.Count() )
        {
            wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( this, _( "Filter Name: " ),
              _( "Enter the new filter name" ), unescape_configlist_str( CurFilterStr.BeforeFirst( wxT( ':' ) ) ) );
            if( EntryDialog->ShowModal() == wxID_OK )
            {
                m_DynFilterArray[ Selected - 1 ] = escape_configlist_str( EntryDialog->GetValue() ) + wxT( ":" ) + EditPlayList.ToString();
                m_FilterChoice->SetString( Selected, EntryDialog->GetValue() );

                RefreshCount();
                ReloadItems();
                m_LastItemStart = wxNOT_FOUND;
                m_NavSlider->SetValue( 0 );
                RefreshAll();

            }
            EntryDialog->Destroy();
        }
        else    // This should never happen
        {
            guLogError( wxT( "Empty dynamic playlit?" ) );
        }
    }

    PlayListEditor->Destroy();
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnFilterSelected( wxCommandEvent &event )
{
    int Selected = event.GetInt();
    if( Selected )
    {
        m_DynFilter.FromString( m_DynFilterArray[ Selected - 1 ].AfterFirst( wxT( ':' ) ) );
        m_DelFilterButton->Enable( true );
        m_EditFilterButton->Enable( true );
    }
    else
    {
        m_DelFilterButton->Enable( false );
        m_EditFilterButton->Enable( false );
    }
    RefreshCount();
    ReloadItems();
    m_LastItemStart = wxNOT_FOUND;
    m_NavSlider->SetValue( 0 );
    RefreshAll();
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnOrderSelected( wxCommandEvent &event )
{
    ReloadItems();
    m_LastItemStart = wxNOT_FOUND;
    m_NavSlider->SetValue( 0 );
    RefreshAll();
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnCommandClicked( const int cmdid, const int albumid )
{
    int index;
    int count;
    wxArrayInt Selection;
    Selection.Add( albumid );
    if( Selection.Count() )
    {
        index = cmdid;

        guConfig * Config = ( guConfig * ) Config->Get();
        if( Config )
        {
            wxArrayString Commands = Config->ReadAStr( wxT( "Cmd" ), wxEmptyString, wxT( "Commands" ) );

            index -= ID_ALBUM_COMMANDS;
            wxString CurCmd = Commands[ index ];
            if( CurCmd.Find( wxT( "{bp}" ) ) != wxNOT_FOUND )
            {
                wxArrayString AlbumPaths = m_Db->GetAlbumsPaths( Selection );
                wxString Paths = wxEmptyString;
                count = AlbumPaths.Count();
                for( index = 0; index < count; index++ )
                {
                    AlbumPaths[ index ].Replace( wxT( " " ), wxT( "\\ " ) );
                    //Paths += wxT( " \"" ) + AlbumPaths[ index ] + wxT( "\"" );
                    Paths += wxT( " " ) + AlbumPaths[ index ];
                }
                CurCmd.Replace( wxT( "{bp}" ), Paths.Trim( false ) );
            }

            if( CurCmd.Find( wxT( "{bc}" ) ) != wxNOT_FOUND )
            {
                int CoverId = m_Db->GetAlbumCoverId( Selection[ 0 ] );
                wxString CoverPath = wxEmptyString;
                if( CoverId > 0 )
                {
                    CoverPath = wxT( "\"" ) + m_Db->GetCoverPath( CoverId ) + wxT( "\"" );
                }
                CurCmd.Replace( wxT( "{bc}" ), CoverPath );
            }

            if( CurCmd.Find( wxT( "{tp}" ) ) != wxNOT_FOUND )
            {
                guTrackArray Songs;
                wxString SongList = wxEmptyString;
                if( m_Db->GetAlbumsSongs( Selection, &Songs ) )
                {
                    NormalizeTracks( &Songs );
                    count = Songs.Count();
                    for( index = 0; index < count; index++ )
                    {
                        SongList += wxT( " \"" ) + Songs[ index ].m_FileName + wxT( "\"" );
                    }
                    CurCmd.Replace( wxT( "{tp}" ), SongList.Trim( false ) );
                }
            }

            //guLogMessage( wxT( "Execute Command '%s'" ), CurCmd.c_str() );
            guExecute( CurCmd );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnAlbumDownloadCoverClicked( const int albumid )
{
    wxArrayInt Albums;
    Albums.Add( albumid );
    if( Albums.Count() )
    {
        wxString AlbumName;
        wxString ArtistName;
        wxString AlbumPath;
        if( !m_Db->GetAlbumInfo( Albums[ 0 ], &AlbumName, &ArtistName, &AlbumPath ) )
        {
            wxMessageBox( _( "Could not find the Album in the songs library.\n"\
                             "You should update the library." ), _( "Error" ), wxICON_ERROR | wxOK );
            return;
        }

        AlbumName = RemoveSearchFilters( AlbumName );

        guCoverEditor * CoverEditor = new guCoverEditor( this, ArtistName, AlbumName );
        if( CoverEditor )
        {
            if( CoverEditor->ShowModal() == wxID_OK )
            {
                //guLogMessage( wxT( "About to download cover from selected url" ) );
                wxImage * CoverImage = CoverEditor->GetSelectedCoverImage();
                if( CoverImage )
                {
                    guConfig * Config = ( guConfig * ) guConfig::Get();
                    wxArrayString SearchCovers = Config->ReadAStr( wxT( "Word" ), wxEmptyString, wxT( "CoverSearch" ) );
                    wxString CoverName = AlbumPath + ( SearchCovers.Count() ? SearchCovers[ 0 ] : wxT( "cover" ) ) + wxT( ".jpg" );
                    CoverImage->SaveFile( CoverName, wxBITMAP_TYPE_JPEG );
                    m_Db->SetAlbumCover( Albums[ 0 ], CoverName );

                    ReloadItems();
                    RefreshAll();

                    if( CoverEditor->EmbedToFiles() )
                    {
                        OnAlbumEmbedCoverClicked( albumid );
                    }

                    wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_ALBUM_COVER_CHANGED );
                    evt.SetInt( albumid );
                    evt.SetClientData( NULL );
                    wxPostEvent( wxTheApp->GetTopWindow(), evt );
                }
            }
            CoverEditor->Destroy();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnAlbumSelectCoverClicked( const int albumid )
{
    guSelCoverFile * SelCoverFile = new guSelCoverFile( this, m_Db, albumid );
    if( SelCoverFile )
    {
        if( SelCoverFile->ShowModal() == wxID_OK )
        {
            wxString CoverFile = SelCoverFile->GetSelFile();
            if( !CoverFile.IsEmpty() )
            {
                wxURI Uri( CoverFile );
                if( Uri.IsReference() )
                {
                    SetAlbumCover( albumid, CoverFile );
                }
                else
                {
                    guConfig * Config = ( guConfig * ) guConfig::Get();
                    wxArrayString SearchCovers = Config->ReadAStr( wxT( "Word" ), wxEmptyString, wxT( "CoverSearch" ) );
                    wxString CoverName = SelCoverFile->GetAlbumPath() + ( SearchCovers.Count() ? SearchCovers[ 0 ] : wxT( "cover" ) ) + wxT( ".jpg" );

                    if( DownloadImage( CoverFile, CoverName ) )
                    {
                        m_Db->SetAlbumCover( albumid, CoverName );
                        ReloadItems();
                        RefreshAll();

                        wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_ALBUM_COVER_CHANGED );
                        evt.SetInt( albumid );
                        evt.SetClientData( NULL );
                        wxPostEvent( wxTheApp->GetTopWindow(), evt );
                    }
                    else
                    {
                        guLogError( wxT( "Failed to download file '%s'" ), CoverFile.c_str() );
                    }
                }
            }
        }
        delete SelCoverFile;
    }
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnAlbumDeleteCoverClicked( const int albumid )
{
    wxArrayInt Albums;
    Albums.Add( albumid );
    if( Albums.Count() )
    {
        if( wxMessageBox( _( "Are you sure to delete the selected album cover?" ),
                          _( "Confirm" ),
                          wxICON_QUESTION | wxYES_NO | wxCANCEL, this ) == wxYES )
        {
            int CoverId = m_Db->GetAlbumCoverId( Albums[ 0 ] );
            if( CoverId > 0 )
            {
                wxString CoverPath = m_Db->GetCoverPath( CoverId );
                if( !wxRemoveFile( CoverPath ) )
                {
                    guLogError( wxT( "Could not remove the cover file '%s'" ), CoverPath.c_str() );
                }
            }
            m_Db->SetAlbumCover( Albums[ 0 ], wxEmptyString );
            ReloadItems();
            RefreshAll();

            wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_ALBUM_COVER_CHANGED );
            evt.SetInt( albumid );
            evt.SetClientData( NULL );
            wxPostEvent( wxTheApp->GetTopWindow(), evt );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnAlbumEmbedCoverClicked( const int albumid )
{
    wxImage * CoverImage = NULL;
    int CoverId = m_Db->GetAlbumCoverId( albumid );
    if( CoverId > 0 )
    {
        wxString CoverPath = m_Db->GetCoverPath( CoverId );

        CoverImage = new wxImage( CoverPath, wxBITMAP_TYPE_ANY );
        if( CoverImage )
        {
            if( !CoverImage->IsOk() )
            {
                delete CoverImage;
                return;
            }

            guImageResize( CoverImage, 600 );
        }
        else
            return;
    }

    guTrackArray AlbumTracks;
    wxArrayInt Albums;
    Albums.Add( albumid );
    m_Db->GetAlbumsSongs( Albums, &AlbumTracks );
    NormalizeTracks( &AlbumTracks );

    int TrackIndex;
    int TrackCount;
    if( ( TrackCount = AlbumTracks.Count() ) )
    {
        for( TrackIndex = 0; TrackIndex < TrackCount; TrackIndex++ )
        {
            guTagSetPicture( AlbumTracks[ TrackIndex ].m_FileName, CoverImage );
        }
    }

    if( CoverImage )
        delete CoverImage;

}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnAlbumCopyToClicked( const int albumid, const int commandid )
{
    guTrackArray * Tracks = new guTrackArray();
    wxArrayInt Albums;
    Albums.Add( albumid );

    m_Db->GetAlbumsSongs( Albums, Tracks );
    NormalizeTracks( Tracks );

    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_MAINFRAME_COPYTO );
    int Index = commandid - ID_ALBUMBROWSER_COPYTO;
    if( Index > 99 )
    {
        Index -= 100;
        event.SetId( ID_MAINFRAME_COPYTODEVICE_TRACKS );
    }
    event.SetInt( Index );
    event.SetClientData( ( void * ) Tracks );
    wxPostEvent( wxTheApp->GetTopWindow(), event );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnAlbumEditLabelsClicked( const guAlbumBrowserItem * albumitem )
{
    guListItems Albums;
    Albums.Add( new guListItem( albumitem->m_AlbumId, albumitem->m_AlbumName ) );
//    if( Albums.Count() )
//    {
        wxArrayInt AlbumIds;
        AlbumIds.Add( albumitem->m_AlbumId );
        guArrayListItems LabelSets = m_Db->GetAlbumsLabels( AlbumIds );

        guLabelEditor * LabelEditor = new guLabelEditor( this, m_Db, _( "Albums Labels Editor" ),
                            false, &Albums, &LabelSets );
        if( LabelEditor )
        {
            if( LabelEditor->ShowModal() == wxID_OK )
            {
                m_Db->UpdateAlbumsLabels( LabelSets );
            }

            LabelEditor->Destroy();

//            if( m_FilterBtn->GetValue() ) // If its filtered we may need to refresh as the filter can contains the label condition
            {
                ReloadItems();
                RefreshAll();
            }
        }
//    }
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnAlbumEditTracksClicked( const int albumid )
{
    guTrackArray Songs;
    guImagePtrArray Images;
    wxArrayString Lyrics;
    wxArrayInt ChangedFlags;

    //m_AlbumListCtrl->GetSelectedSongs( &Songs );
    wxArrayInt Albums;
    Albums.Add( albumid );
    m_Db->GetAlbumsSongs( Albums, &Songs, true );
    if( !Songs.Count() )
        return;
    NormalizeTracks( &Songs );
    guTrackEditor * TrackEditor = new guTrackEditor( this, m_Db, &Songs, &Images, &Lyrics, &ChangedFlags );
    if( TrackEditor )
    {
        if( TrackEditor->ShowModal() == wxID_OK )
        {
            guUpdateTracks( Songs, Images, Lyrics, ChangedFlags );
            m_Db->UpdateSongs( &Songs, ChangedFlags );
            //guUpdateLyrics( Songs, Lyrics, ChangedFlags );
            //guUpdateImages( Songs, Images, ChangedFlags );

            m_PlayerPanel->UpdatedTracks( &Songs );
        }
        guImagePtrArrayClean( &Images );
        TrackEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnUpdateDetails( wxCommandEvent &event )
{
    m_AlbumItemsMutex.Lock();
    //guLogMessage( wxT( "OnUpdateDetails %i - %i" ), event.GetInt(), m_ItemPanels.GetCount() );
    int Item = event.GetInt();
    if( ( Item >= 0 ) && Item < ( int ) m_ItemPanels.GetCount() )
        m_ItemPanels[ event.GetInt() ]->UpdateDetails();
    m_AlbumItemsMutex.Unlock();
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnMouseWheel( wxMouseEvent& event )
{
    if( !m_NavSlider->IsEnabled() )
        return;

    int Rotation = event.GetWheelRotation() / event.GetWheelDelta() * -1;
    //guLogMessage( wxT( "Got MouseWheel %i " ), Rotation );
    int CurPos = m_NavSlider->GetValue() + Rotation;

    if( CurPos > m_NavSlider->GetMax() )
        CurPos = m_NavSlider->GetMax();

    if( CurPos < m_NavSlider->GetMin() )
        CurPos = m_NavSlider->GetMin();

    m_NavSlider->SetValue( CurPos );

    wxScrollEvent ScrollEvent( wxEVT_SCROLL_CHANGED );
    ScrollEvent.SetPosition( CurPos );
    wxPostEvent( m_NavSlider, ScrollEvent );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::LibraryUpdated( void )
{
    int CurPage = m_NavSlider->GetValue();

    RefreshCount();

    if( CurPage > m_NavSlider->GetMax() )
        CurPage = m_NavSlider->GetMax();

    m_ItemStart = CurPage * m_ItemCount;
    UpdateNavLabel( CurPage );

    ReloadItems();
    RefreshAll();
}

// -------------------------------------------------------------------------------- //
wxString guAlbumBrowser::GetAlbumCoverFile( const int coverid )
{
    return m_Db->GetCoverPath( coverid );
}

// -------------------------------------------------------------------------------- //
void  guAlbumBrowser::SetAlbumCover( const int albumid, const wxString &cover )
{
    wxString AlbumPath;
    if( m_Db->GetAlbumInfo( albumid, NULL, NULL, &AlbumPath ) )
    {
        wxImage CoverImage( cover );
        if( CoverImage.IsOk() )
        {
            guConfig * Config = ( guConfig * ) guConfig::Get();
            wxArrayString SearchCovers = Config->ReadAStr( wxT( "Word" ), wxEmptyString, wxT( "CoverSearch" ) );
            wxString CoverName = AlbumPath + ( SearchCovers.Count() ? SearchCovers[ 0 ] : wxT( "cover" ) ) + wxT( ".jpg" );
            if( CoverImage.SaveFile( CoverName, wxBITMAP_TYPE_JPEG ) )
            {
                m_Db->SetAlbumCover( albumid, CoverName );
            }
        }
        else
        {
            guLogError( wxT( "Could not load the imate '%s'" ), cover.c_str() );
        }
        ReloadItems();
        RefreshAll();

        //
        wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_ALBUM_COVER_CHANGED );
        evt.SetInt( albumid );
        evt.SetClientData( NULL );
        wxPostEvent( wxTheApp->GetTopWindow(), evt );
    }
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnBitmapMouseOver( const int coverid, const wxPoint &position )
{
    wxString CoverFile = GetAlbumCoverFile( coverid );
    if( !CoverFile.IsEmpty() )
    {
        wxImage * Image = new wxImage( CoverFile, wxBITMAP_TYPE_ANY );
        if( Image )
        {
            if( Image->IsOk() )
            {
                guImageResize( Image, 300, true );

                guShowImage * ShowImage = new guShowImage( this, Image, position );
                if( ShowImage )
                {
                    ShowImage->Show();
                }
            }
            else
            {
                delete Image;
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnAlbumSelectName( const int albumid )
{
    wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_MAINFRAME_SELECT_ALBUM );
    evt.SetInt( albumid );
    evt.SetExtraLong( guTRACK_TYPE_DB );
    wxPostEvent( wxTheApp->GetTopWindow(), evt );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnArtistSelectName( const int artistid )
{
    wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_MAINFRAME_SELECT_ARTIST );
    evt.SetInt( artistid );
    evt.SetExtraLong( guTRACK_TYPE_DB );
    wxPostEvent( wxTheApp->GetTopWindow(), evt );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::UpdateNavLabel( const int page )
{
    m_NavLabel->SetLabel( _( "Page" ) + wxString::Format( wxT( " %i / %i" ), page + 1, m_PagesCount ) );
}




// -------------------------------------------------------------------------------- //
// guAlbumBrowserDropTarget
// -------------------------------------------------------------------------------- //
guAlbumBrowserDropTarget::guAlbumBrowserDropTarget( guAlbumBrowserItemPanel * itempanel ) :
    wxFileDropTarget()
{
    m_AlbumBrowserItemPanel = itempanel;
}

// -------------------------------------------------------------------------------- //
bool guAlbumBrowserDropTarget::OnDropFiles( wxCoord x, wxCoord y, const wxArrayString &files )
{
    if( files.Count() )
    {
        wxString CoverFile = files[ 0 ];
        if( !CoverFile.IsEmpty() && wxFileExists( CoverFile ) )
        {
            if( guIsValidImageFile( CoverFile.Lower() ) )
            {
                m_AlbumBrowserItemPanel->SetAlbumCover( CoverFile );
                return true;
            }
        }
    }
    return false;
}

// -------------------------------------------------------------------------------- //
