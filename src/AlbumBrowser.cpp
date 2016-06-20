// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2016 J.Rios anonbeat@gmail.com
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
//    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#include "AlbumBrowser.h"

#include "Accelerators.h"
#include "EventCommandIds.h"
#include "Config.h"
#include "CoverEdit.h"
#include "Images.h"
#include "LabelEditor.h"
#include "LibPanel.h"
#include "LibUpdate.h"
#include "MainApp.h"
#include "MainFrame.h"
#include "MediaViewer.h"
#include "OnlineLinks.h"
#include "PlayListAppend.h"
#include "SelCoverFile.h"
#include "Settings.h"
#include "StaticBitmap.h"
#include "ShowImage.h"
#include "TagInfo.h"
#include "TrackEdit.h"
#include "Utils.h"

#include <wx/arrimpl.cpp>
#include "wx/clipbrd.h"
#include <wx/dnd.h>
#include <wx/menu.h>

namespace Guayadeque {

WX_DEFINE_OBJARRAY( guAlbumBrowserItemArray );

#define guALBUMBROWSER_REFRESH_DELAY            60
#define guALBUMBROWSER_GRID_SIZE_WIDTH          140
//#define guALBUMBROWSER_GRID_SIZE_HEIGHT         180

static int guALBUMBROWSER_GRID_SIZE_HEIGHT = -1;

#define guALBUMBROWSER_TIMER_ID_REFRESH         3
#define guALBUMBROWSER_TIMER_ID_BITMAP_CLICKED  4

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
                    Db->GetAlbumDetails( m_AlbumBrowser->m_AlbumItems[ Index ].m_AlbumId,
                        ( int * ) &m_AlbumBrowser->m_AlbumItems[ Index ].m_Year,
                        ( int * ) &m_AlbumBrowser->m_AlbumItems[ Index ].m_TrackCount );

                    if( TestDestroy() )
                        break;
                }
            }

            if( !TestDestroy() )
            {
                wxCommandEvent event( wxEVT_MENU, ID_ALBUMBROWSER_UPDATEDETAILS );
                //event.SetInt( Index );
                wxPostEvent( m_AlbumBrowser, event );
                //guLogMessage( wxT( "Sent Details %i %i for %i" ), m_AlbumBrowser->m_AlbumItems[ Index ].m_Year, m_AlbumBrowser->m_AlbumItems[ Index ].m_TrackCount, Index );
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
    guAlbumBrowserItem * albumitem ) : wxPanel( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS )
{
    m_AlbumBrowserItem = albumitem;
    m_AlbumBrowser = ( guAlbumBrowser * ) parent;

    wxFont CurrentFont = GetFont();

    // GUI
	m_MainSizer = new wxBoxSizer( wxVERTICAL );

	m_Bitmap = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize( GUCOVER_IMAGE_SIZE, GUCOVER_IMAGE_SIZE ) );
    m_MainSizer->Add( m_Bitmap, 0, wxRIGHT, 2 );

    m_AlbumLabel = new guAutoScrollText( this, wxEmptyString, wxSize( 100, 16 ) );
	CurrentFont.SetWeight( wxFONTWEIGHT_BOLD );
    CurrentFont.SetPointSize( CurrentFont.GetPointSize() - 1 );
	m_AlbumLabel->SetFont( CurrentFont );
    m_MainSizer->Add( m_AlbumLabel, 0, wxEXPAND, 5 );

    m_ArtistLabel = new guAutoScrollText( this, wxEmptyString, wxSize( 100, 16 ) );
	CurrentFont.SetWeight( wxFONTWEIGHT_NORMAL );
    m_ArtistLabel->SetFont( CurrentFont );
    m_MainSizer->Add( m_ArtistLabel, 0, wxEXPAND, 5 );

    m_TracksLabel = new guAutoScrollText( this, wxEmptyString, wxSize( 100, 16 ) );
	m_TracksLabel->SetFont( CurrentFont );
    m_MainSizer->Add( m_TracksLabel, 0, wxEXPAND, 5 );

	SetSizer( m_MainSizer );
	Layout();
	m_MainSizer->Fit( this );
    m_MainSizer->SetSizeHints( this );

    SetDropTarget( new guAlbumBrowserDropTarget( m_AlbumBrowser->m_MediaViewer, this ) );

    m_BitmapTimer.SetOwner( this );

    Bind( wxEVT_CONTEXT_MENU, &guAlbumBrowserItemPanel::OnContextMenu, this, wxID_ANY );

    Bind( wxEVT_MENU, &guAlbumBrowserItemPanel::OnSearchLinkClicked, this, ID_LINKS_BASE, ID_LINKS_BASE + guLINKS_MAXCOUNT );
    Bind( wxEVT_MENU, &guAlbumBrowserItemPanel::OnCommandClicked, this, ID_COMMANDS_BASE, ID_COMMANDS_BASE + guCOMMANDS_MAXCOUNT );

    m_Bitmap->Bind( wxEVT_LEFT_DCLICK, &guAlbumBrowserItemPanel::OnAlbumDClicked, this );
    m_Bitmap->Bind( wxEVT_LEFT_DOWN, &guAlbumBrowserItemPanel::OnBitmapClicked, this );

    Bind( wxEVT_MENU, &guAlbumBrowserItemPanel::OnPlayClicked, this, ID_ALBUMBROWSER_PLAY );
    Bind( wxEVT_MENU, &guAlbumBrowserItemPanel::OnEnqueueClicked, this, ID_ALBUMBROWSER_ENQUEUE_AFTER_ALL, ID_ALBUMBROWSER_ENQUEUE_AFTER_ARTIST );
    Bind( wxEVT_MENU, &guAlbumBrowserItemPanel::OnAlbumEditLabelsClicked, this, ID_ALBUMBROWSER_EDITLABELS );
    Bind( wxEVT_MENU, &guAlbumBrowserItemPanel::OnAlbumEditTracksClicked, this, ID_ALBUMBROWSER_EDITTRACKS );
    Bind( wxEVT_MENU, &guAlbumBrowserItemPanel::OnCopyToClipboard, this, ID_ALBUMBROWSER_COPYTOCLIPBOARD );
    Bind( wxEVT_MENU, &guAlbumBrowserItemPanel::OnAlbumSelectName, this, ID_ALBUM_SELECTNAME );
    Bind( wxEVT_MENU, &guAlbumBrowserItemPanel::OnArtistSelectName, this, ID_ARTIST_SELECTNAME );

    Bind( wxEVT_MENU, &guAlbumBrowserItemPanel::OnAlbumDownloadCoverClicked, this, ID_ALBUMBROWSER_SEARCHCOVER );
    Bind( wxEVT_MENU, &guAlbumBrowserItemPanel::OnAlbumSelectCoverClicked, this, ID_ALBUMBROWSER_SELECTCOVER );
    Bind( wxEVT_MENU, &guAlbumBrowserItemPanel::OnAlbumDeleteCoverClicked, this, ID_ALBUMBROWSER_DELETECOVER );
    Bind( wxEVT_MENU, &guAlbumBrowserItemPanel::OnAlbumEmbedCoverClicked, this, ID_ALBUMBROWSER_EMBEDCOVER );
    Bind( wxEVT_MENU, &guAlbumBrowserItemPanel::OnAlbumCopyToClicked, this, ID_COPYTO_BASE, ID_COPYTO_BASE + guCOPYTO_MAXCOUNT );

    m_Bitmap->Bind( wxEVT_LEFT_DOWN, &guAlbumBrowserItemPanel::OnMouse, this );
    m_Bitmap->Bind( wxEVT_LEFT_UP, &guAlbumBrowserItemPanel::OnMouse, this );
    m_Bitmap->Bind( wxEVT_MOTION, &guAlbumBrowserItemPanel::OnMouse, this );

    Bind( wxEVT_MENU, &guAlbumBrowserItemPanel::OnBeginDrag, this, ID_ALBUMBROWSER_BEGINDRAG );
    Bind( wxEVT_MENU, &guAlbumBrowserItemPanel::OnCoverBeginDrag, this, ID_ALBUMBROWSER_COVER_BEGINDRAG );

    Bind( wxEVT_TIMER, &guAlbumBrowserItemPanel::OnTimer, this );

    m_Bitmap->Bind( wxEVT_KEY_DOWN, &guAlbumBrowser::OnKeyDown, m_AlbumBrowser );
    m_AlbumLabel->Bind( wxEVT_KEY_DOWN, &guAlbumBrowser::OnKeyDown, m_AlbumBrowser );
    m_ArtistLabel->Bind( wxEVT_KEY_DOWN, &guAlbumBrowser::OnKeyDown, m_AlbumBrowser );
    m_TracksLabel->Bind( wxEVT_KEY_DOWN, &guAlbumBrowser::OnKeyDown, m_AlbumBrowser );
}

// -------------------------------------------------------------------------------- //
guAlbumBrowserItemPanel::~guAlbumBrowserItemPanel()
{
    Unbind( wxEVT_CONTEXT_MENU, &guAlbumBrowserItemPanel::OnContextMenu, this, wxID_ANY );

    Unbind( wxEVT_MENU, &guAlbumBrowserItemPanel::OnSearchLinkClicked, this, ID_LINKS_BASE, ID_LINKS_BASE + guLINKS_MAXCOUNT );
    Unbind( wxEVT_MENU, &guAlbumBrowserItemPanel::OnCommandClicked, this, ID_COMMANDS_BASE, ID_COMMANDS_BASE + guCOMMANDS_MAXCOUNT );

    m_Bitmap->Unbind( wxEVT_LEFT_DCLICK, &guAlbumBrowserItemPanel::OnAlbumDClicked, this );
    m_Bitmap->Unbind( wxEVT_LEFT_DOWN, &guAlbumBrowserItemPanel::OnBitmapClicked, this );

    Unbind( wxEVT_MENU, &guAlbumBrowserItemPanel::OnPlayClicked, this, ID_ALBUMBROWSER_PLAY );
    Unbind( wxEVT_MENU, &guAlbumBrowserItemPanel::OnEnqueueClicked, this, ID_ALBUMBROWSER_ENQUEUE_AFTER_ALL, ID_ALBUMBROWSER_ENQUEUE_AFTER_ARTIST );
    Unbind( wxEVT_MENU, &guAlbumBrowserItemPanel::OnAlbumEditLabelsClicked, this, ID_ALBUMBROWSER_EDITLABELS );
    Unbind( wxEVT_MENU, &guAlbumBrowserItemPanel::OnAlbumEditTracksClicked, this, ID_ALBUMBROWSER_EDITTRACKS );
    Unbind( wxEVT_MENU, &guAlbumBrowserItemPanel::OnCopyToClipboard, this, ID_ALBUMBROWSER_COPYTOCLIPBOARD );
    Unbind( wxEVT_MENU, &guAlbumBrowserItemPanel::OnAlbumSelectName, this, ID_ALBUM_SELECTNAME );
    Unbind( wxEVT_MENU, &guAlbumBrowserItemPanel::OnArtistSelectName, this, ID_ARTIST_SELECTNAME );

    Unbind( wxEVT_MENU, &guAlbumBrowserItemPanel::OnAlbumDownloadCoverClicked, this, ID_ALBUMBROWSER_SEARCHCOVER );
    Unbind( wxEVT_MENU, &guAlbumBrowserItemPanel::OnAlbumSelectCoverClicked, this, ID_ALBUMBROWSER_SELECTCOVER );
    Unbind( wxEVT_MENU, &guAlbumBrowserItemPanel::OnAlbumDeleteCoverClicked, this, ID_ALBUMBROWSER_DELETECOVER );
    Unbind( wxEVT_MENU, &guAlbumBrowserItemPanel::OnAlbumEmbedCoverClicked, this, ID_ALBUMBROWSER_EMBEDCOVER );
    Unbind( wxEVT_MENU, &guAlbumBrowserItemPanel::OnAlbumCopyToClicked, this, ID_COPYTO_BASE, ID_COPYTO_BASE + guCOPYTO_MAXCOUNT );

    m_Bitmap->Unbind( wxEVT_LEFT_DOWN, &guAlbumBrowserItemPanel::OnMouse, this );
    m_Bitmap->Unbind( wxEVT_LEFT_UP, &guAlbumBrowserItemPanel::OnMouse, this );
    m_Bitmap->Unbind( wxEVT_MOTION, &guAlbumBrowserItemPanel::OnMouse, this );

    Unbind( wxEVT_MENU, &guAlbumBrowserItemPanel::OnBeginDrag, this, ID_ALBUMBROWSER_BEGINDRAG );
    Unbind( wxEVT_MENU, &guAlbumBrowserItemPanel::OnCoverBeginDrag, this, ID_ALBUMBROWSER_COVER_BEGINDRAG );

    Unbind( wxEVT_TIMER, &guAlbumBrowserItemPanel::OnTimer, this );

    m_Bitmap->Unbind( wxEVT_KEY_DOWN, &guAlbumBrowser::OnKeyDown, m_AlbumBrowser );
    m_AlbumLabel->Unbind( wxEVT_KEY_DOWN, &guAlbumBrowser::OnKeyDown, m_AlbumBrowser );
    m_ArtistLabel->Unbind( wxEVT_KEY_DOWN, &guAlbumBrowser::OnKeyDown, m_AlbumBrowser );
    m_TracksLabel->Unbind( wxEVT_KEY_DOWN, &guAlbumBrowser::OnKeyDown, m_AlbumBrowser );
}

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
        m_TracksLabel->SetToolTip( Label );
        //m_MainSizer->Layout();
    }
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowserItemPanel::OnContextMenu( wxContextMenuEvent &event )
{
    m_AlbumBrowser->SetLastAlbumItem( m_AlbumBrowserItem );

    if( !m_AlbumBrowserItem )
        return;

    //m_Bitmap->StopTimer();

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


    if( m_AlbumBrowserItem->m_AlbumId != ( unsigned int ) wxNOT_FOUND )
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
                                wxString( _( "Current Track" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_TRACK ),
                                _( "Add current selected tracks to playlist after the current track" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );

        MenuItem = new wxMenuItem( EnqueueMenu, ID_ALBUMBROWSER_ENQUEUE_AFTER_ALBUM,
                                wxString( _( "Current Album" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ALBUM ),
                                _( "Add current selected tracks to playlist after the current album" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );

        MenuItem = new wxMenuItem( EnqueueMenu, ID_ALBUMBROWSER_ENQUEUE_AFTER_ARTIST,
                                wxString( _( "Current Artist" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ARTIST ),
                                _( "Add current selected tracks to playlist after the current artist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );

        Menu.Append( wxID_ANY, _( "Enqueue After" ), EnqueueMenu );

        Menu.AppendSeparator();

        MenuItem = new wxMenuItem( &Menu, ID_ALBUMBROWSER_EDITLABELS, _( "Edit Labels" ), _( "Edit the labels assigned to the selected albums" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tags ) );
        Menu.Append( MenuItem );

        if( ContextMenuFlags & guCONTEXTMENU_EDIT_TRACKS )
        {
            MenuItem = new wxMenuItem( &Menu, ID_ALBUMBROWSER_EDITTRACKS, _( "Edit Songs" ), _( "Edit the selected songs" ) );
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

        if( ContextMenuFlags & guCONTEXTMENU_DOWNLOAD_COVERS )
        {
            Menu.AppendSeparator();

            MenuItem = new wxMenuItem( &Menu, ID_ALBUMBROWSER_SEARCHCOVER, _( "Download Cover" ), _( "Download cover for the current selected album" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_download_covers ) );
            Menu.Append( MenuItem );

            MenuItem = new wxMenuItem( &Menu, ID_ALBUMBROWSER_SELECTCOVER, _( "Select Cover" ), _( "Select the cover image file from disk" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_download_covers ) );
            Menu.Append( MenuItem );

            MenuItem = new wxMenuItem( &Menu, ID_ALBUMBROWSER_DELETECOVER, _( "Delete Cover" ), _( "Delete the cover for the selected album" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_delete ) );
            Menu.Append( MenuItem );
        }

        if( ContextMenuFlags & guCONTEXTMENU_EMBED_COVERS )
        {
            MenuItem = new wxMenuItem( &Menu, ID_ALBUMBROWSER_EMBEDCOVER, _( "Embed Cover" ), _( "Embed the cover to the selected album tracks" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_doc_save ) );
            Menu.Append( MenuItem );
        }

        Menu.AppendSeparator();

        wxMenu * SortMenu = new wxMenu();

        int SortSelected = m_AlbumBrowser->GetSortSelected();

        MenuItem = new wxMenuItem( SortMenu, ID_ALBUMBROWSER_ORDER_NAME, _( "Name" ), _( "Sort albums by name" ), wxITEM_CHECK );
        SortMenu->Append( MenuItem );
        MenuItem->Check( SortSelected == guALBUMS_ORDER_NAME );

        MenuItem = new wxMenuItem( SortMenu, ID_ALBUMBROWSER_ORDER_YEAR, _( "Year" ), _( "Sort albums by year" ), wxITEM_CHECK );
        SortMenu->Append( MenuItem );
        MenuItem->Check( SortSelected == guALBUMS_ORDER_YEAR );

        MenuItem = new wxMenuItem( SortMenu, ID_ALBUMBROWSER_ORDER_YEAR_REVERSE, _( "Year Descending" ), _( "Sort albums by year descendant" ), wxITEM_CHECK );
        SortMenu->Append( MenuItem );
        MenuItem->Check( SortSelected == guALBUMS_ORDER_YEAR_REVERSE );

        MenuItem = new wxMenuItem( SortMenu, ID_ALBUMBROWSER_ORDER_ARTIST, _( "Artist Name" ), _( "Sort albums by artist name" ), wxITEM_CHECK );
        SortMenu->Append( MenuItem );
        MenuItem->Check( SortSelected == guALBUMS_ORDER_ARTIST_NAME );

        MenuItem = new wxMenuItem( SortMenu, ID_ALBUMBROWSER_ORDER_ARTIST_YEAR, _( "Artist, Year" ), _( "Sort albums by artist, year" ), wxITEM_CHECK );
        SortMenu->Append( MenuItem );
        MenuItem->Check( SortSelected == guALBUMS_ORDER_ARTIST_YEAR );

        MenuItem = new wxMenuItem( SortMenu, ID_ALBUMBROWSER_ORDER_ARTIST_YEAR_REVERSE, _( "Artist, Year Descending" ), _( "Sort albums by artist, year descendant" ), wxITEM_CHECK );
        SortMenu->Append( MenuItem );
        MenuItem->Check( SortSelected == guALBUMS_ORDER_ARTIST_YEAR_REVERSE );

        MenuItem = new wxMenuItem( SortMenu, ID_ALBUMBROWSER_ORDER_ADDEDTIME, _( "Added" ), _( "Sort albums by year descendant" ), wxITEM_CHECK );
        SortMenu->Append( MenuItem );
        MenuItem->Check( SortSelected == guALBUMS_ORDER_ADDEDTIME );

        Menu.AppendSubMenu( SortMenu, _( "Sort By" ), _( "Set the albums sorting" ) );

        Menu.AppendSeparator();

        MenuItem = new wxMenuItem( &Menu, ID_ALBUMBROWSER_COPYTOCLIPBOARD, _( "Copy to Clipboard" ), _( "Copy the album info to clipboard" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit_copy ) );
        Menu.Append( MenuItem );


        if( ( ContextMenuFlags & guCONTEXTMENU_COPY_TO ) ||
            ( ContextMenuFlags & guCONTEXTMENU_LINKS ) ||
            ( ContextMenuFlags & guCONTEXTMENU_COMMANDS ) )
        {
            Menu.AppendSeparator();

            if( ContextMenuFlags & guCONTEXTMENU_COPY_TO )
            {
                guMainFrame * MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
                MainFrame->CreateCopyToMenu( &Menu );
            }

            if( ContextMenuFlags & guCONTEXTMENU_LINKS )
            {
                AddOnlineLinksMenu( &Menu );
            }

            if( ContextMenuFlags & guCONTEXTMENU_COMMANDS )
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
    ExecuteOnlineLink( event.GetId(), m_AlbumBrowserItem->m_ArtistName + wxT( " " ) +
                                                      m_AlbumBrowserItem->m_AlbumName );
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
            if( m_BitmapTimer.IsRunning() )
                m_BitmapTimer.Stop();

            wxCommandEvent DragEvent( wxEVT_MENU, ID_ALBUMBROWSER_BEGINDRAG );
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
int guAlbumBrowserItemPanel::GetDragFiles( guDataObjectComposite * files )
{
    guTrackArray Tracks;
    wxArrayString Filenames;
    int Index;
    int Count = m_AlbumBrowser->GetAlbumTracks( m_AlbumBrowserItem->m_AlbumId, &Tracks );
    for( Index = 0; Index < Count; Index++ )
    {
        if( Tracks[ Index ].m_Offset )
            continue;
        Filenames.Add( guFileDnDEncode( Tracks[ Index ].m_FileName ) );
    }
    files->SetTracks( Tracks );
    files->SetFiles( Filenames );
    return Count;
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowserItemPanel::OnBeginDrag( wxCommandEvent &event )
{
    if( !m_AlbumBrowserItem )
        return;

    guDataObjectComposite Files;

    if( GetDragFiles( &Files ) )
    {
        wxDropSource source( Files, this );

        wxDragResult Result = source.DoDragDrop();
        if( Result )
        {
        }
    }
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowserItemPanel::OnCoverBeginDrag( wxCommandEvent &event )
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
void guAlbumBrowserItemPanel::OnAlbumDClicked( wxMouseEvent &event )
{
    if( m_BitmapTimer.IsRunning() )
        m_BitmapTimer.Stop();

    if( m_AlbumBrowserItem )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        m_AlbumBrowser->SelectAlbum( m_AlbumBrowserItem->m_AlbumId,
            Config->ReadBool( wxT( "DefaultActionEnqueue" ), false , wxT( "general" ) ) );
    }
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowserItemPanel::OnBitmapClicked( wxMouseEvent &event )
{
    if( m_BitmapTimer.IsRunning() )
        m_BitmapTimer.Stop();

    m_BitmapTimer.Start( 300, wxTIMER_ONE_SHOT );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowserItemPanel::OnTimer( wxTimerEvent &event )
{
    if( m_AlbumBrowserItem )
        m_AlbumBrowser->OnBitmapClicked( m_AlbumBrowserItem, ClientToScreen( m_Bitmap->GetPosition() ) );
}




// -------------------------------------------------------------------------------- //
// guAlbumBrowser
// -------------------------------------------------------------------------------- //
guAlbumBrowser::guAlbumBrowser( wxWindow * parent, guMediaViewer * mediaviewer ) :
    wxPanel( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS ),
    m_RefreshTimer( this, guALBUMBROWSER_TIMER_ID_REFRESH ),
    m_BitmapClickTimer( this, guALBUMBROWSER_TIMER_ID_BITMAP_CLICKED )
{
    m_MediaViewer = mediaviewer;
    m_Db = mediaviewer->GetDb();
    m_PlayerPanel = mediaviewer->GetPlayerPanel();
    m_UpdateDetails = NULL;
    m_ItemStart = 0;
    m_LastItemStart = wxNOT_FOUND;
    m_ItemCount = 1;
    m_BlankCD = new wxBitmap( guImage( guIMAGE_INDEX_no_cover ) );
    m_ConfigPath = mediaviewer->ConfigPath() + wxT( "/albumbrowser" );
    m_BigCoverShowed = false;
    m_BigCoverTracksContextMenu = false;
    m_DragCount = 0;
    m_MouseWasLeftUp = false;
    m_MouseSelecting = false;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->RegisterObject( this );


    if( guALBUMBROWSER_GRID_SIZE_HEIGHT == -1 )
    {
        CalculateMaxItemHeight();
    }

    CreateControls();

    CreateAcceleratorTable();

    RefreshCount();

}

// -------------------------------------------------------------------------------- //
guAlbumBrowser::~guAlbumBrowser()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->UnRegisterObject( this );

    Config->WriteNum( wxT( "Sort" ), m_SortSelected, m_ConfigPath );

    if( m_BlankCD )
        delete m_BlankCD;

    Unbind( wxEVT_SIZE, &guAlbumBrowser::OnChangedSize, this );
    Unbind( wxEVT_KEY_DOWN, &guAlbumBrowser::OnKeyDown, this );

    m_NavSlider->Unbind( wxEVT_SCROLL_CHANGED, &guAlbumBrowser::OnChangingPosition, this );
    m_NavSlider->Unbind( wxEVT_SCROLL_THUMBTRACK, &guAlbumBrowser::OnChangingPosition, this );
    Unbind( wxEVT_TIMER, &guAlbumBrowser::OnRefreshTimer, this, guALBUMBROWSER_TIMER_ID_REFRESH );
    Unbind( wxEVT_MENU, &guAlbumBrowser::OnUpdateDetails, this, ID_ALBUMBROWSER_UPDATEDETAILS );
    Unbind( wxEVT_MOUSEWHEEL, &guAlbumBrowser::OnMouseWheel, this );
    Unbind( wxEVT_MENU, &guAlbumBrowser::OnGoToSearch, this, ID_ALBUMBROWSER_SEARCH );
    Unbind( guConfigUpdatedEvent, &guAlbumBrowser::OnConfigUpdated, this, ID_CONFIG_UPDATED );

    Unbind( wxEVT_MENU, &guAlbumBrowser::OnSortSelected, this, ID_ALBUMBROWSER_ORDER_NAME, ID_ALBUMBROWSER_ORDER_ADDEDTIME );

    m_BigCoverBackBtn->Unbind( wxEVT_BUTTON, &guAlbumBrowser::OnBigCoverBackClicked, this );
    m_BigCoverBitmap->Unbind( wxEVT_LEFT_DOWN, &guAlbumBrowser::OnBigCoverBitmapClicked, this );
    m_BigCoverBitmap->Unbind( wxEVT_LEFT_DCLICK, &guAlbumBrowser::OnBigCoverBitmapDClicked, this );
    m_BigCoverTracksListBox->Unbind( wxEVT_LISTBOX_DCLICK, &guAlbumBrowser::OnBigCoverTracksDClicked, this );
    m_BigCoverBitmap->Unbind( wxEVT_CONTEXT_MENU, &guAlbumBrowser::OnBigCoverContextMenu, this );
    m_BigCoverTracksListBox->Unbind( wxEVT_CONTEXT_MENU, &guAlbumBrowser::OnBigCoverTracksContextMenu, this );

    Unbind( wxEVT_TIMER, &guAlbumBrowser::OnTimerTimeout, this, guALBUMBROWSER_TIMER_ID_BITMAP_CLICKED );

    Unbind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverPlayClicked, this, ID_ALBUMBROWSER_PLAY );
    Unbind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverEnqueueClicked, this, ID_ALBUMBROWSER_ENQUEUE_AFTER_ALL, ID_ALBUMBROWSER_ENQUEUE_AFTER_ARTIST );
    Unbind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverEditLabelsClicked, this, ID_ALBUMBROWSER_EDITLABELS );
    Unbind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverEditTracksClicked, this, ID_ALBUMBROWSER_EDITTRACKS );
    Unbind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverCopyToClipboard, this, ID_ALBUMBROWSER_COPYTOCLIPBOARD );
    Unbind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverAlbumSelectName, this, ID_ALBUM_SELECTNAME );
    Unbind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverArtistSelectName, this, ID_ARTIST_SELECTNAME );
    Unbind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverDownloadCoverClicked, this, ID_ALBUMBROWSER_SEARCHCOVER );
    Unbind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverSelectCoverClicked, this, ID_ALBUMBROWSER_SELECTCOVER );
    Unbind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverDeleteCoverClicked, this, ID_ALBUMBROWSER_DELETECOVER );
    Unbind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverEmbedCoverClicked, this, ID_ALBUMBROWSER_EMBEDCOVER );
    Unbind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverCopyToClicked, this, ID_COPYTO_BASE, ID_COPYTO_BASE + guCOPYTO_MAXCOUNT );
    Unbind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverCommandClicked, this, ID_COMMANDS_BASE, ID_COMMANDS_BASE + guCOMMANDS_MAXCOUNT );
    Unbind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverSearchLinkClicked, this, ID_LINKS_BASE, ID_LINKS_BASE + guLINKS_MAXCOUNT );

    Unbind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverTracksPlayClicked, this, ID_ALBUMBROWSER_TRACKS_PLAY );
    Unbind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverTracksEnqueueClicked, this, ID_ALBUMBROWSER_TRACKS_ENQUEUE_AFTER_ALL, ID_ALBUMBROWSER_TRACKS_ENQUEUE_AFTER_ARTIST );
    Unbind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverTracksEditLabelsClicked, this, ID_ALBUMBROWSER_TRACKS_EDITLABELS );
    Unbind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverTracksEditTracksClicked, this, ID_ALBUMBROWSER_TRACKS_EDITTRACKS );
    m_BigCoverTracksListBox->Unbind( wxEVT_LEFT_DOWN, &guAlbumBrowser::OnBigCoverTracksMouseMoved, this );
    m_BigCoverTracksListBox->Unbind( wxEVT_LEFT_UP, &guAlbumBrowser::OnBigCoverTracksMouseMoved, this );
    m_BigCoverTracksListBox->Unbind( wxEVT_MOTION, &guAlbumBrowser::OnBigCoverTracksMouseMoved, this );
    Unbind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverTracksBeginDrag, this, ID_ALBUMBROWSER_TRACKS_BEGINDRAG );

    Unbind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverTracksPlaylistSave, this, ID_ALBUMBROWSER_TRACKS_PLAYLIST_SAVE );
    Unbind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverTracksSmartPlaylist, this, ID_ALBUMBROWSER_TRACKS_SMART_PLAYLIST );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::CreateControls( void )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();

    m_SortSelected = Config->ReadNum( wxT( "Sort" ), guALBUMS_ORDER_ARTIST_YEAR_REVERSE, m_ConfigPath );

    // GUI
	SetMinSize( wxSize( 120, 150 ) );

    m_MainSizer = new wxBoxSizer( wxVERTICAL );

    // AlbumItems
	m_AlbumBrowserSizer = new wxBoxSizer( wxVERTICAL );

	m_AlbumsSizer = new wxGridSizer( 1, 1, 0, 0 );

	m_ItemPanels.Add( new guAlbumBrowserItemPanel( this, 0 ) );

	m_AlbumsSizer->Add( m_ItemPanels[ 0 ], 1, wxEXPAND|wxALL, 5 );

	m_AlbumBrowserSizer->Add( m_AlbumsSizer, 1, wxEXPAND, 5 );

	// Navigator
	wxBoxSizer * NavigatorSizer = new wxBoxSizer( wxVERTICAL );

	m_NavLabel = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_NavLabel->Wrap( -1 );
	NavigatorSizer->Add( m_NavLabel, 0, wxRIGHT|wxLEFT|wxEXPAND, 5 );

	m_NavSlider = new wxSlider( this, wxID_ANY, 0, 0, 1, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	m_NavSlider->SetFocus();
	NavigatorSizer->Add( m_NavSlider, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxRIGHT|wxLEFT, 5 );

	m_AlbumBrowserSizer->Add( NavigatorSizer, 0, wxEXPAND, 5 );

    m_MainSizer->Add( m_AlbumBrowserSizer, 1, wxEXPAND, 5 );


    // BigCover
    // ------------------------------------------------------------------------------------------------------------------
	m_BigCoverSizer = new wxBoxSizer( wxVERTICAL );

	m_BigCoverSizer->Add( 0, 0, 1, wxEXPAND, 5 );

    wxFlexGridSizer * BigCoverCenterSizer = new wxFlexGridSizer( 5, 0, 0 );
	BigCoverCenterSizer->AddGrowableCol( 0 );
	BigCoverCenterSizer->AddGrowableCol( 2 );
	BigCoverCenterSizer->AddGrowableCol( 3 );
	BigCoverCenterSizer->AddGrowableCol( 4 );
	BigCoverCenterSizer->SetFlexibleDirection( wxBOTH );
	BigCoverCenterSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	BigCoverCenterSizer->Add( 0, 0, 2, wxEXPAND, 5 );

	wxBoxSizer * BigCoverSizer = new wxBoxSizer( wxVERTICAL );

	m_BigCoverBitmap = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize( 300, 300 ), 0 );
	BigCoverSizer->Add( m_BigCoverBitmap, 0, wxALIGN_CENTER_HORIZONTAL, 5 );

	m_BigCoverAlbumLabel = new guAutoScrollText( this, wxEmptyString, wxSize( 290, -1 ) );
	wxFont CurFont = GetFont();
	CurFont.SetWeight( wxFONTWEIGHT_BOLD );
	m_BigCoverAlbumLabel->SetFont( CurFont );
	BigCoverSizer->Add( m_BigCoverAlbumLabel, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP|wxRIGHT|wxLEFT, 5 );

	m_BigCoverArtistLabel = new guAutoScrollText( this, wxEmptyString, wxSize( 290, -1 ) );
	//m_BigCoverArtistLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );
	BigCoverSizer->Add( m_BigCoverArtistLabel, 0, wxALIGN_CENTER_HORIZONTAL|wxRIGHT|wxLEFT, 5 );

	m_BigCoverDetailsLabel = new guAutoScrollText( this, wxEmptyString, wxSize( 290, -1 ) );
    CurFont = GetFont();
	CurFont.SetPointSize( CurFont.GetPointSize() - 2 );
	m_BigCoverDetailsLabel->SetFont( CurFont );
	BigCoverSizer->Add( m_BigCoverDetailsLabel, 0, wxALIGN_CENTER_HORIZONTAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	BigCoverCenterSizer->Add( BigCoverSizer, 1, wxEXPAND|wxRIGHT|wxLEFT, 5 );

	BigCoverCenterSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	m_BigCoverTracksListBox = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_MULTIPLE );
	m_BigCoverTracksListBox->SetBackgroundColour( m_BigCoverDetailsLabel->GetBackgroundColour() );
	m_BigCoverTracksListBox->SetMaxSize( wxSize( 450, -1 ) );

	BigCoverCenterSizer->Add( m_BigCoverTracksListBox, 1, wxALIGN_CENTER_HORIZONTAL|wxEXPAND|wxRIGHT, 5 );

	BigCoverCenterSizer->Add( 0, 0, 2, wxEXPAND, 5 );

	m_BigCoverSizer->Add( BigCoverCenterSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxEXPAND, 5 );

	m_BigCoverSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	wxBoxSizer * BigCoverBtnSizer = new wxBoxSizer( wxVERTICAL );

	m_BigCoverBackBtn = new wxButton( this, wxID_ANY, _( "Back" ), wxDefaultPosition, wxDefaultSize, 0 );
	BigCoverBtnSizer->Add( m_BigCoverBackBtn, 0, wxALIGN_CENTER_HORIZONTAL, 5 );

	m_BigCoverSizer->Add( BigCoverBtnSizer, 0, wxEXPAND, 5 );

	m_BigCoverSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	m_MainSizer->Add( m_BigCoverSizer, 1, wxEXPAND, 5 );
    // ------------------------------------------------------------------------------------------------------------------

    this->SetSizer( m_MainSizer );
	BigCoverSizer->Layout();

	m_MainSizer->Hide( m_BigCoverSizer, true );
	m_MainSizer->FitInside( this );

    Bind( wxEVT_SIZE, &guAlbumBrowser::OnChangedSize, this );
    Bind( wxEVT_KEY_DOWN, &guAlbumBrowser::OnKeyDown, this );

    m_NavSlider->Bind( wxEVT_SCROLL_CHANGED, &guAlbumBrowser::OnChangingPosition, this );
    m_NavSlider->Bind( wxEVT_SCROLL_THUMBTRACK, &guAlbumBrowser::OnChangingPosition, this );
    Bind( wxEVT_TIMER, &guAlbumBrowser::OnRefreshTimer, this, guALBUMBROWSER_TIMER_ID_REFRESH );
    Bind( wxEVT_MENU, &guAlbumBrowser::OnUpdateDetails, this, ID_ALBUMBROWSER_UPDATEDETAILS );
    Bind( wxEVT_MOUSEWHEEL, &guAlbumBrowser::OnMouseWheel, this );
    Bind( wxEVT_MENU, &guAlbumBrowser::OnGoToSearch, this, ID_ALBUMBROWSER_SEARCH );
    Bind( guConfigUpdatedEvent, &guAlbumBrowser::OnConfigUpdated, this, ID_CONFIG_UPDATED );

    Bind( wxEVT_MENU, &guAlbumBrowser::OnSortSelected, this, ID_ALBUMBROWSER_ORDER_NAME, ID_ALBUMBROWSER_ORDER_ADDEDTIME );

    m_BigCoverBackBtn->Bind( wxEVT_BUTTON, &guAlbumBrowser::OnBigCoverBackClicked, this );
    m_BigCoverBitmap->Bind( wxEVT_LEFT_DOWN, &guAlbumBrowser::OnBigCoverBitmapClicked, this );
    m_BigCoverBitmap->Bind( wxEVT_LEFT_DCLICK, &guAlbumBrowser::OnBigCoverBitmapDClicked, this );
    m_BigCoverTracksListBox->Bind( wxEVT_LISTBOX_DCLICK, &guAlbumBrowser::OnBigCoverTracksDClicked, this );
    m_BigCoverBitmap->Bind( wxEVT_CONTEXT_MENU, &guAlbumBrowser::OnBigCoverContextMenu, this );
    m_BigCoverTracksListBox->Bind( wxEVT_CONTEXT_MENU, &guAlbumBrowser::OnBigCoverTracksContextMenu, this );

    Bind( wxEVT_TIMER, &guAlbumBrowser::OnTimerTimeout, this, guALBUMBROWSER_TIMER_ID_BITMAP_CLICKED );

    Bind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverPlayClicked, this, ID_ALBUMBROWSER_PLAY );
    Bind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverEnqueueClicked, this, ID_ALBUMBROWSER_ENQUEUE_AFTER_ALL, ID_ALBUMBROWSER_ENQUEUE_AFTER_ARTIST );
    Bind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverEditLabelsClicked, this, ID_ALBUMBROWSER_EDITLABELS );
    Bind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverEditTracksClicked, this, ID_ALBUMBROWSER_EDITTRACKS );
    Bind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverCopyToClipboard, this, ID_ALBUMBROWSER_COPYTOCLIPBOARD );
    Bind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverAlbumSelectName, this, ID_ALBUM_SELECTNAME );
    Bind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverArtistSelectName, this, ID_ARTIST_SELECTNAME );
    Bind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverDownloadCoverClicked, this, ID_ALBUMBROWSER_SEARCHCOVER );
    Bind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverSelectCoverClicked, this, ID_ALBUMBROWSER_SELECTCOVER );
    Bind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverDeleteCoverClicked, this, ID_ALBUMBROWSER_DELETECOVER );
    Bind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverEmbedCoverClicked, this, ID_ALBUMBROWSER_EMBEDCOVER );
    Bind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverCopyToClicked, this, ID_COPYTO_BASE, ID_COPYTO_BASE + guCOPYTO_MAXCOUNT );
    Bind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverCommandClicked, this, ID_COMMANDS_BASE, ID_COMMANDS_BASE + guCOMMANDS_MAXCOUNT );
    Bind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverSearchLinkClicked, this, ID_LINKS_BASE, ID_LINKS_BASE + guLINKS_MAXCOUNT );

    Bind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverTracksPlayClicked, this, ID_ALBUMBROWSER_TRACKS_PLAY );
    Bind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverTracksEnqueueClicked, this, ID_ALBUMBROWSER_TRACKS_ENQUEUE_AFTER_ALL, ID_ALBUMBROWSER_TRACKS_ENQUEUE_AFTER_ARTIST );
    Bind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverTracksEditLabelsClicked, this, ID_ALBUMBROWSER_TRACKS_EDITLABELS );
    Bind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverTracksEditTracksClicked, this, ID_ALBUMBROWSER_TRACKS_EDITTRACKS );
    m_BigCoverTracksListBox->Bind( wxEVT_LEFT_DOWN, &guAlbumBrowser::OnBigCoverTracksMouseMoved, this );
    m_BigCoverTracksListBox->Bind( wxEVT_LEFT_UP, &guAlbumBrowser::OnBigCoverTracksMouseMoved, this );
    m_BigCoverTracksListBox->Bind( wxEVT_MOTION, &guAlbumBrowser::OnBigCoverTracksMouseMoved, this );
    Bind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverTracksBeginDrag, this, ID_ALBUMBROWSER_TRACKS_BEGINDRAG );

    Bind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverTracksPlaylistSave, this, ID_ALBUMBROWSER_TRACKS_PLAYLIST_SAVE );
    Bind( wxEVT_MENU, &guAlbumBrowser::OnBigCoverTracksSmartPlaylist, this, ID_ALBUMBROWSER_TRACKS_SMART_PLAYLIST );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnConfigUpdated( wxCommandEvent &event )
{
    int Flags = event.GetInt();
    if( Flags & guPREFERENCE_PAGE_FLAG_ACCELERATORS )
    {
        CreateAcceleratorTable();
    }
}

// -------------------------------------------------------------------------------- //
int guAlbumBrowser::GetContextMenuFlags( void )
{
    return m_MediaViewer->GetContextMenuFlags();
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
    m_MediaViewer->GoToSearch();
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::CalculateMaxItemHeight( void )
{
    int w;
    int h;
    int d;
    GetTextExtent( wxT("Hg"), &w, &h, &d );

    guALBUMBROWSER_GRID_SIZE_HEIGHT = GUCOVER_IMAGE_SIZE + 4 + ( 3 * h ) + 5;
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnChangedSize( wxSizeEvent &event )
{
    wxSize Size = event.GetSize();
    if( ( Size != m_LastSize ) )
    {
        size_t ColItems = Size.GetWidth() / guALBUMBROWSER_GRID_SIZE_WIDTH;
        size_t RowItems = Size.GetHeight() / guALBUMBROWSER_GRID_SIZE_HEIGHT;
        //guLogMessage( wxT( "Row: %i  Col:%i" ), RowItems, ColItems );
        if( ColItems * RowItems != m_ItemPanels.Count() )
        {
            size_t OldCount = m_ItemPanels.Count();
            m_ItemCount = ColItems * RowItems;
            //guLogMessage( wxT( "We need to reassign the panels from %li to %i" ), OldCount, m_ItemCount );
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
                        if( m_BigCoverShowed )
                            m_ItemPanels[ Index ]->Hide();
                        m_AlbumsSizer->Add( m_ItemPanels[ Index ], 1, wxEXPAND|wxALL, 2 );
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
void guAlbumBrowser::RefreshCount( void )
{
    m_AlbumsCount = m_Db->GetAlbumsCount( m_DynFilter.IsEmpty() ? NULL : &m_DynFilter, m_TextSearchFilter );
    //m_ItemStart = 0;
    RefreshPageCount();
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::ClearUpdateDetailsThread( void )
{
    m_UpdateDetailsMutex.Lock();
    m_UpdateDetails = NULL;
    m_UpdateDetailsMutex.Unlock();
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::RefreshPageCount( void )
{
    if( m_ItemCount && m_AlbumsCount )
    {
        m_PagesCount = m_AlbumsCount / m_ItemCount;
        if( ( m_PagesCount * m_ItemCount ) < m_AlbumsCount )
            m_PagesCount++;
    }
    else
    {
        m_PagesCount = 0;
    }

    //guLogMessage( wxT( "RefreshPageCount: Albums: %i   Items: %i  Pages: %i"  ), m_AlbumsCount, m_ItemCount, m_PagesCount );
    UpdateNavLabel( 0 );

    m_NavSlider->Enable( m_PagesCount > 1 );
    if( m_PagesCount > 1 )
        m_NavSlider->SetRange( 0, m_PagesCount - 1 );
    //else
    //    m_NavSlider->SetRange( 0, 0 );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::SetCurrentPage( int page )
{
    m_NavSlider->SetValue( page );

    wxScrollEvent ScrollEvent( wxEVT_SCROLL_CHANGED );
    ScrollEvent.SetPosition( page );
    wxPostEvent( m_NavSlider, ScrollEvent );
}

// -------------------------------------------------------------------------------- //
bool guAlbumBrowser::DoTextSearch( const wxString &searchtext )
{
    if( m_LastSearchString != searchtext )
    {
        m_LastSearchString = searchtext; //m_SearchTextCtrl->GetLineText( 0 );
        if( !m_LastSearchString.IsEmpty() )
        {
            if( m_LastSearchString.Length() > 0 )
            {
                m_TextSearchFilter = guSplitWords( m_LastSearchString );

                RefreshCount();

                m_LastItemStart = wxNOT_FOUND;
                SetCurrentPage( 0 );

//                m_SearchTextCtrl->ShowCancelButton( true );
            }
            return true;
        }
        else
        {
            m_TextSearchFilter.Clear();

            RefreshCount();

            m_LastItemStart = wxNOT_FOUND;
            SetCurrentPage( 0 );

//            m_SearchTextCtrl->ShowCancelButton( false );
        }
    }
    return false;
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::ReloadItems( void )
{
    m_AlbumItemsMutex.Lock();
    m_AlbumItems.Empty();
    m_Db->GetAlbums( &m_AlbumItems, m_DynFilter.IsEmpty() ? NULL : &m_DynFilter,
            m_TextSearchFilter, m_ItemStart, m_ItemCount, m_SortSelected );
    m_AlbumItemsMutex.Unlock();
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
        //guLogMessage( wxT( "%li %s " ), Index, m_AlbumItems[ Index ].m_AlbumName.c_str() );
        if( Index < m_AlbumItems.Count() )
            m_ItemPanels[ Index ]->SetAlbumItem( Index, &m_AlbumItems[ Index ], m_BlankCD );
        else
            m_ItemPanels[ Index ]->SetAlbumItem( Index, NULL, m_BlankCD );
    }
    m_AlbumItemsMutex.Unlock();
    Layout();

    wxCommandEvent event( wxEVT_MENU, ID_MAINFRAME_UPDATE_SELINFO );
    AddPendingEvent( event );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnChangingPosition( wxScrollEvent& event )
{
    int CurPage = event.GetPosition(); //( ( event.GetPosition() * m_PagesCount ) / 100 );
    m_ItemStart = CurPage * m_ItemCount;
    //guLogMessage( wxT( "ChangePosition: %i -> %i     Albums(%i / %i)" ), m_ItemStart, m_LastItemStart, CurPage, m_AlbumsCount );
    if( m_LastItemStart != m_ItemStart )
    {
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
    if( m_Db->GetAlbumsSongs( Selections, &Tracks, true ) )
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
    return m_Db->GetAlbumsSongs( Albums, tracks, true );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::SetFilter( const wxString &filterstr )
{
    m_DynFilter.FromString( filterstr.AfterFirst( wxT( ':' ) ) );
    RefreshCount();

    m_LastItemStart = wxNOT_FOUND;
    SetCurrentPage( 0 );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnSortSelected( wxCommandEvent &event )
{
    m_SortSelected = event.GetId() - ID_ALBUMBROWSER_ORDER_NAME;

    m_LastItemStart = wxNOT_FOUND;
    SetCurrentPage( 0 );
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

        guConfig * Config = ( guConfig * ) guConfig::Get();
        if( Config )
        {
            wxArrayString Commands = Config->ReadAStr( wxT( "Exec" ), wxEmptyString, wxT( "commands/execs" ) );

            index -= ID_COMMANDS_BASE;
            wxString CurCmd = Commands[ index ];
            if( CurCmd.Find( guCOMMAND_ALBUMPATH ) != wxNOT_FOUND )
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
                CurCmd.Replace( guCOMMAND_ALBUMPATH, Paths.Trim( false ) );
            }

            if( CurCmd.Find( guCOMMAND_COVERPATH ) != wxNOT_FOUND )
            {
                int CoverId = m_Db->GetAlbumCoverId( Selection[ 0 ] );
                wxString CoverPath = wxEmptyString;
                if( CoverId > 0 )
                {
                    CoverPath = wxT( "\"" ) + m_Db->GetCoverPath( CoverId ) + wxT( "\"" );
                }
                CurCmd.Replace( guCOMMAND_COVERPATH, CoverPath );
            }

            if( CurCmd.Find( guCOMMAND_TRACKPATH ) != wxNOT_FOUND )
            {
                guTrackArray Songs;
                wxString SongList = wxEmptyString;
                if( m_Db->GetAlbumsSongs( Selection, &Songs, true ) )
                {
                    NormalizeTracks( &Songs );
                    count = Songs.Count();
                    for( index = 0; index < count; index++ )
                    {
                        SongList += wxT( " \"" ) + Songs[ index ].m_FileName + wxT( "\"" );
                    }
                    CurCmd.Replace( guCOMMAND_TRACKPATH, SongList.Trim( false ) );
                }
            }

            //guLogMessage( wxT( "Execute Command '%s'" ), CurCmd.c_str() );
            guExecute( CurCmd );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnCommandClicked( const int cmdid, const guTrackArray &tracks )
{
    int Index;
    int Count;
    if( tracks.Count() )
    {
        Index = cmdid;

        guConfig * Config = ( guConfig * ) guConfig::Get();
        if( Config )
        {
            wxArrayString Commands = Config->ReadAStr( wxT( "Exec" ), wxEmptyString, wxT( "commands/execs" ) );

            Index -= ID_COMMANDS_BASE;
            wxString CurCmd = Commands[ Index ];

            if( CurCmd.Find( guCOMMAND_ALBUMPATH ) != wxNOT_FOUND )
            {
                wxArrayString AlbumPaths;
                Count = tracks.Count();
                for( Index = 0; Index < Count; Index++ )
                {
                    wxString Path = wxPathOnly( tracks[ Index ].m_FileName ) + wxT( "/" );
                    if( AlbumPaths.Index( Path ) == wxNOT_FOUND )
                        AlbumPaths.Add( Path );
                }
                wxString Paths = wxEmptyString;
                Count = AlbumPaths.Count();
                for( Index = 0; Index < Count; Index++ )
                {
                    AlbumPaths[ Index ].Replace( wxT( " " ), wxT( "\\ " ) );
                    Paths += wxT( " " ) + AlbumPaths[ Index ];
                }
                CurCmd.Replace( guCOMMAND_ALBUMPATH, Paths.Trim( false ) );
            }

            if( CurCmd.Find( guCOMMAND_COVERPATH ) != wxNOT_FOUND )
            {
                wxArrayInt AlbumList;
                AlbumList.Add( tracks[ 0 ].m_AlbumId );
                int CoverId = m_Db->GetAlbumCoverId( AlbumList[ 0 ] );
                wxString CoverPath = wxEmptyString;
                if( CoverId > 0 )
                {
                    CoverPath = wxT( "\"" ) + m_Db->GetCoverPath( CoverId ) + wxT( "\"" );
                }
                CurCmd.Replace( guCOMMAND_COVERPATH, CoverPath );
            }

            if( CurCmd.Find( guCOMMAND_TRACKPATH ) != wxNOT_FOUND )
            {
                wxString SongList = wxEmptyString;
                Count = tracks.Count();
                if( Count )
                {
                    for( Index = 0; Index < Count; Index++ )
                    {
                        SongList += wxT( " \"" ) + tracks[ Index ].m_FileName + wxT( "\"" );
                    }
                    CurCmd.Replace( guCOMMAND_TRACKPATH, SongList.Trim( false ) );
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
    m_MediaViewer->DownloadAlbumCover( albumid );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnAlbumSelectCoverClicked( const int albumid )
{
    m_MediaViewer->SelectAlbumCover( albumid );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnAlbumDeleteCoverClicked( const int albumid )
{
    m_MediaViewer->DeleteAlbumCover( albumid );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnAlbumEmbedCoverClicked( const int albumid )
{
    m_MediaViewer->EmbedAlbumCover( albumid );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnAlbumCopyToClicked( const int albumid, const int commandid )
{
    guTrackArray * Tracks = new guTrackArray();
    wxArrayInt Albums;
    Albums.Add( albumid );

    m_Db->GetAlbumsSongs( Albums, Tracks, true );
    NormalizeTracks( Tracks );

    wxCommandEvent event( wxEVT_MENU, ID_MAINFRAME_COPYTO );
    int Index = commandid - ID_COPYTO_BASE;
    if( Index >= guCOPYTO_DEVICE_BASE )
    {
        Index -= guCOPYTO_DEVICE_BASE;
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
    //guLogMessage( wxT( "OnUpdateDetails %i - %li" ), event.GetInt(), m_ItemPanels.GetCount() );
    int Index;
    int Count = m_ItemPanels.GetCount();
    for( Index = 0; Index < Count; Index++ )
    {
        m_ItemPanels[ Index ]->UpdateDetails();
    }
    Layout();
    m_AlbumItemsMutex.Unlock();
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnKeyDown( wxKeyEvent &event )
{
    //guLogMessage( wxT( "OnKeyDown %i" ), event.GetKeyCode() );
    int CurPos = m_NavSlider->GetValue();
    int KeyCode = event.GetKeyCode();
    switch( KeyCode )
    {
        case WXK_HOME :
            CurPos = m_NavSlider->GetMin();
            break;

        case WXK_END :
            CurPos = m_NavSlider->GetMax();
            break;

        case WXK_DOWN :
        case WXK_RIGHT :
        case WXK_NUMPAD_DOWN :
            CurPos++;
            break;

        case WXK_UP :
        case WXK_LEFT :
        case WXK_NUMPAD_UP :
            CurPos--;
            break;

        case WXK_PAGEDOWN :
        case WXK_NUMPAD_PAGEDOWN :
            CurPos += m_NavSlider->GetPageSize();
            break;

        case WXK_PAGEUP :
        case WXK_NUMPAD_PAGEUP :
            CurPos -= m_NavSlider->GetPageSize();
            break;
    }

    if( CurPos > m_NavSlider->GetMax() )
        CurPos = m_NavSlider->GetMax();

    if( CurPos < m_NavSlider->GetMin() )
        CurPos = m_NavSlider->GetMin();

    SetCurrentPage( CurPos );

    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnMouseWheel( wxMouseEvent& event )
{
    if( m_BigCoverShowed || !m_NavSlider->IsEnabled() )
    {
        event.Skip();
        return;
    }

    int Rotation = event.GetWheelRotation() / event.GetWheelDelta() * -1;
    //guLogMessage( wxT( "Got MouseWheel %i " ), Rotation );
    int CurPos = m_NavSlider->GetValue() + Rotation;

    if( CurPos > m_NavSlider->GetMax() )
        CurPos = m_NavSlider->GetMax();

    if( CurPos < m_NavSlider->GetMin() )
        CurPos = m_NavSlider->GetMin();

    SetCurrentPage( CurPos );

    event.Skip();
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
            wxArrayString SearchCovers = Config->ReadAStr( wxT( "Word" ), wxEmptyString, wxT( "coversearch" ) );
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
        wxCommandEvent evt( wxEVT_MENU, ID_ALBUM_COVER_CHANGED );
        evt.SetInt( albumid );
        evt.SetClientData( NULL );
        wxPostEvent( wxTheApp->GetTopWindow(), evt );
    }
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnBitmapClicked( guAlbumBrowserItem * albumitem, const wxPoint &position )
{
    if( albumitem && albumitem->m_AlbumId )
    {
        wxImage * Image = NULL;
        wxString CoverFile = GetAlbumCoverFile( albumitem->m_CoverId );
        if( !CoverFile.IsEmpty() )
        {
            Image = new wxImage( CoverFile, wxBITMAP_TYPE_ANY );
            if( Image && !Image->IsOk() )
            {
                delete Image;
                Image = NULL;
            }
        }

        if( !Image )
        {
            Image = new wxImage( guImage( guIMAGE_INDEX_no_cover ) );
        }

        guImageResize( Image, 300, true );

        m_BigCoverBitmap->SetBitmap( wxBitmap( * Image ) );

        if( Image )
        {
            delete Image;
        }

        m_BigCoverTracks.Empty();
        wxArrayInt Albums;
        Albums.Add( albumitem->m_AlbumId );

        m_Db->GetAlbumsSongs( Albums, &m_BigCoverTracks, true );
        m_MediaViewer->NormalizeTracks( &m_BigCoverTracks );

        m_BigCoverTracksListBox->Clear();
        m_BigCoverTracksListBox->Append( _( "All" ) );
        wxUint64 AlbumLength = 0;
        int Index;
        int Count = m_BigCoverTracks.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            const guTrack &Track = m_BigCoverTracks[ Index ];
            AlbumLength += Track.m_Length;
            wxString TrackName = wxString::Format( wxT( "%02u - " ), Track.m_Number );
            if( !Track.m_AlbumArtist.IsEmpty() )
            {
                TrackName += Track.m_ArtistName + wxT( " - " );
            }
            TrackName += Track.m_SongName;

            m_BigCoverTracksListBox->Append( TrackName );
        }

        m_BigCoverAlbumLabel->SetLabel( albumitem->m_AlbumName );
        m_BigCoverArtistLabel->SetLabel( albumitem->m_ArtistName );
        wxString Details;
        if( albumitem->m_Year )
        {
            Details += wxString::Format( wxT( "%04u" ), albumitem->m_Year );
        }
        Details += wxString::Format( wxT( "   %02lu " ), m_BigCoverTracks.Count() );
        Details += _( "Tracks" );
        if( AlbumLength )
        {
            Details += wxT( "   " ) + LenToString( AlbumLength );
        }
        m_BigCoverDetailsLabel->SetLabel( Details );

        m_LastAlbumBrowserItem = albumitem;
        m_BigCoverShowed = true;
        m_MainSizer->Hide( m_AlbumBrowserSizer, true );
        m_MainSizer->Show( m_BigCoverSizer, true );

        Layout();

        wxCommandEvent event( wxEVT_MENU, ID_MAINFRAME_UPDATE_SELINFO );
        AddPendingEvent( event );
    }
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnAlbumSelectName( const int albumid )
{
    wxCommandEvent evt( wxEVT_MENU, ID_MAINFRAME_SELECT_ALBUM );
    evt.SetInt( albumid );
    evt.SetClientData( m_MediaViewer );
    //evt.SetExtraLong( guTRACK_TYPE_DB );
    wxPostEvent( wxTheApp->GetTopWindow(), evt );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnArtistSelectName( const int artistid )
{
    wxCommandEvent evt( wxEVT_MENU, ID_MAINFRAME_SELECT_ARTIST );
    evt.SetInt( artistid );
    evt.SetClientData( m_MediaViewer );
    //evt.SetExtraLong( guTRACK_TYPE_DB );
    wxPostEvent( wxTheApp->GetTopWindow(), evt );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::UpdateNavLabel( const int page )
{
    m_NavLabel->SetLabel( _( "Page" ) + wxString::Format( wxT( " %i / %i" ), page + 1, m_PagesCount ) );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::NormalizeTracks( guTrackArray * tracks, const bool isdrag )
{
    m_MediaViewer->NormalizeTracks( tracks, isdrag );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::AlbumCoverChanged( const int albumid )
{
    if( m_BigCoverShowed && ( m_LastAlbumBrowserItem->m_AlbumId == ( unsigned int ) albumid ) )
    {
        wxImage * Image = NULL;
        wxString CoverFile = GetAlbumCoverFile( m_LastAlbumBrowserItem->m_CoverId );
        if( !CoverFile.IsEmpty() )
        {
            Image = new wxImage( CoverFile, wxBITMAP_TYPE_ANY );
            if( Image && !Image->IsOk() )
            {
                delete Image;
                Image = NULL;
            }
        }

        if( !Image )
        {
            Image = new wxImage( guImage( guIMAGE_INDEX_no_cover ) );
        }

        guImageResize( Image, 300, true );

        m_BigCoverBitmap->SetBitmap( wxBitmap( * Image ) );

        if( Image )
        {
            delete Image;
        }
    }

    ReloadItems();

    // When we do the ReloadItems the m_LastAlbumBrowser pointer is invalid so find it again
    int Index;
    int Count = m_AlbumItems.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( m_AlbumItems[ Index ].m_AlbumId == ( unsigned int ) albumid )
        {
            m_LastAlbumBrowserItem = &m_AlbumItems[ Index ];
        }
    }

    RefreshAll();
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::CreateContextMenu( wxMenu * menu )
{
    m_MediaViewer->CreateContextMenu( menu, guLIBRARY_ELEMENT_ALBUMS );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnBigCoverBackClicked( wxCommandEvent &event )
{
    DoBackToAlbums();
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnBigCoverBitmapClicked( wxMouseEvent &event )
{
    guLogMessage( wxT( "OnBigCoverBitmapClicked..." ) );
    if( m_BitmapClickTimer.IsRunning() )
        m_BitmapClickTimer.Stop();

    m_BitmapClickTimer.Start( 300, wxTIMER_ONE_SHOT );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnBigCoverBitmapDClicked( wxMouseEvent &event )
{
    if( m_BitmapClickTimer.IsRunning() )
        m_BitmapClickTimer.Stop();

    DoSelectTracks( m_BigCoverTracks, false );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnTimerTimeout( wxTimerEvent &event )
{
    guLogMessage( wxT( "OnBigCoverBitmapClicked...Timeout..." ) );
    DoBackToAlbums();
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::DoBackToAlbums( void )
{
    m_BigCoverShowed = false;
    m_MainSizer->Hide( m_BigCoverSizer, true );
    m_MainSizer->Show( m_AlbumBrowserSizer, true );
    Layout();
    m_NavSlider->SetFocus();

    wxCommandEvent event( wxEVT_MENU, ID_MAINFRAME_UPDATE_SELINFO );
    AddPendingEvent( event );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnBigCoverTracksDClicked( wxCommandEvent &event )
{
    guLogMessage( wxT( "TracksDClicked..." ) );
    guTrackArray Tracks;
    if( GetSelectedTracks( &Tracks ) )
    {
        DoSelectTracks( Tracks, false );
    }
}

// -------------------------------------------------------------------------------- //
int guAlbumBrowser::GetSelectedTracks( guTrackArray * tracks )
{
    wxArrayInt Selections;
    int Index;
    int Count = m_BigCoverTracksListBox->GetSelections( Selections );
    if( Selections.Index( 0 ) != wxNOT_FOUND )
    {
        * tracks = m_BigCoverTracks;
    }
    else
    {
        for( Index = 0; Index < Count; Index++ )
        {
            tracks->Add( new guTrack( m_BigCoverTracks[ Selections[ Index ] - 1 ] ) );
        }
    }
    return tracks->Count();
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::DoSelectTracks( const guTrackArray &tracks, const bool append, const int aftercurrent )
{
    if( append )
        m_PlayerPanel->AddToPlayList( tracks, true, aftercurrent );
    else
        m_PlayerPanel->SetPlayList( tracks );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnBigCoverContextMenu( wxContextMenuEvent &event )
{
    m_BigCoverTracksContextMenu = false;
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


    int ContextMenuFlags = m_MediaViewer->GetContextMenuFlags();

    MenuItem = new wxMenuItem( &Menu, ID_ALBUMBROWSER_PLAY, _( "Play" ), _( "Play the album tracks" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
    Menu.Append( MenuItem );

    MenuItem = new wxMenuItem( &Menu, ID_ALBUMBROWSER_ENQUEUE_AFTER_ALL, _( "Enqueue" ), _( "Enqueue the album tracks to the playlist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    Menu.Append( MenuItem );

    wxMenu * EnqueueMenu = new wxMenu();

    MenuItem = new wxMenuItem( EnqueueMenu, ID_ALBUMBROWSER_ENQUEUE_AFTER_TRACK,
                            wxString( _( "Current Track" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_TRACK ),
                            _( "Add current selected tracks to playlist after the current track" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    EnqueueMenu->Append( MenuItem );

    MenuItem = new wxMenuItem( EnqueueMenu, ID_ALBUMBROWSER_ENQUEUE_AFTER_ALBUM,
                            wxString( _( "Current Album" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ALBUM ),
                            _( "Add current selected tracks to playlist after the current album" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    EnqueueMenu->Append( MenuItem );

    MenuItem = new wxMenuItem( EnqueueMenu, ID_ALBUMBROWSER_ENQUEUE_AFTER_ARTIST,
                            wxString( _( "Current Artist" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ARTIST ),
                            _( "Add current selected tracks to playlist after the current artist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    EnqueueMenu->Append( MenuItem );

    Menu.Append( wxID_ANY, _( "Enqueue After" ), EnqueueMenu );

    Menu.AppendSeparator();

    MenuItem = new wxMenuItem( &Menu, ID_ALBUMBROWSER_EDITLABELS, _( "Edit Labels" ), _( "Edit the labels assigned to the selected albums" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tags ) );
    Menu.Append( MenuItem );

    if( ContextMenuFlags & guCONTEXTMENU_EDIT_TRACKS )
    {
        MenuItem = new wxMenuItem( &Menu, ID_ALBUMBROWSER_EDITTRACKS, _( "Edit Songs" ), _( "Edit the selected songs" ) );
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

    if( ContextMenuFlags & guCONTEXTMENU_DOWNLOAD_COVERS )
    {
        Menu.AppendSeparator();

        MenuItem = new wxMenuItem( &Menu, ID_ALBUMBROWSER_SEARCHCOVER, _( "Download Cover" ), _( "Download cover for the current selected album" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_download_covers ) );
        Menu.Append( MenuItem );

        MenuItem = new wxMenuItem( &Menu, ID_ALBUMBROWSER_SELECTCOVER, _( "Select Cover" ), _( "Select the cover image file from disk" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_download_covers ) );
        Menu.Append( MenuItem );

        MenuItem = new wxMenuItem( &Menu, ID_ALBUMBROWSER_DELETECOVER, _( "Delete Cover" ), _( "Delete the cover for the selected album" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_delete ) );
        Menu.Append( MenuItem );
    }

    if( ContextMenuFlags & guCONTEXTMENU_EMBED_COVERS )
    {
        MenuItem = new wxMenuItem( &Menu, ID_ALBUMBROWSER_EMBEDCOVER, _( "Embed Cover" ), _( "Embed the cover to the selected album tracks" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_doc_save ) );
        Menu.Append( MenuItem );
    }

    Menu.AppendSeparator();

    MenuItem = new wxMenuItem( &Menu, ID_ALBUMBROWSER_COPYTOCLIPBOARD, _( "Copy to Clipboard" ), _( "Copy the album info to clipboard" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit_copy ) );
    Menu.Append( MenuItem );


    if( ( ContextMenuFlags & guCONTEXTMENU_COPY_TO ) ||
        ( ContextMenuFlags & guCONTEXTMENU_LINKS ) ||
        ( ContextMenuFlags & guCONTEXTMENU_COMMANDS ) )
    {
        Menu.AppendSeparator();

        if( ContextMenuFlags & guCONTEXTMENU_COPY_TO )
        {
            guMainFrame * MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
            MainFrame->CreateCopyToMenu( &Menu );
        }

        if( ContextMenuFlags & guCONTEXTMENU_LINKS )
        {
            AddOnlineLinksMenu( &Menu );
        }

        if( ContextMenuFlags & guCONTEXTMENU_COMMANDS )
        {
            AddAlbumCommands( &Menu, 1 );
        }
    }

    CreateContextMenu( &Menu );

    // Add Links and Commands
    PopupMenu( &Menu, Point.x, Point.y );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnBigCoverTracksContextMenu( wxContextMenuEvent &event )
{
    m_BigCoverTracksContextMenu = true;
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


    int ContextMenuFlags = m_MediaViewer->GetContextMenuFlags();

    MenuItem = new wxMenuItem( &Menu, ID_ALBUMBROWSER_TRACKS_PLAY, _( "Play" ), _( "Play the album tracks" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
    Menu.Append( MenuItem );

    MenuItem = new wxMenuItem( &Menu, ID_ALBUMBROWSER_TRACKS_ENQUEUE_AFTER_ALL, _( "Enqueue" ), _( "Enqueue the album tracks to the playlist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    Menu.Append( MenuItem );

    wxMenu * EnqueueMenu = new wxMenu();

    MenuItem = new wxMenuItem( EnqueueMenu, ID_ALBUMBROWSER_TRACKS_ENQUEUE_AFTER_TRACK,
                            wxString( _( "Current Track" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_TRACK ),
                            _( "Add current selected tracks to playlist after the current track" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    EnqueueMenu->Append( MenuItem );

    MenuItem = new wxMenuItem( EnqueueMenu, ID_ALBUMBROWSER_TRACKS_ENQUEUE_AFTER_ALBUM,
                            wxString( _( "Current Album" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ALBUM ),
                            _( "Add current selected tracks to playlist after the current album" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    EnqueueMenu->Append( MenuItem );

    MenuItem = new wxMenuItem( EnqueueMenu, ID_ALBUMBROWSER_TRACKS_ENQUEUE_AFTER_ARTIST,
                            wxString( _( "Current Artist" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ARTIST ),
                            _( "Add current selected tracks to playlist after the current artist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    EnqueueMenu->Append( MenuItem );

    Menu.Append( wxID_ANY, _( "Enqueue After" ), EnqueueMenu );

    Menu.AppendSeparator();

    MenuItem = new wxMenuItem( &Menu, ID_ALBUMBROWSER_TRACKS_EDITLABELS, _( "Edit Labels" ), _( "Edit the labels assigned to the selected albums" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tags ) );
    Menu.Append( MenuItem );

    if( ContextMenuFlags & guCONTEXTMENU_EDIT_TRACKS )
    {
        MenuItem = new wxMenuItem( &Menu, ID_ALBUMBROWSER_TRACKS_EDITTRACKS, _( "Edit Songs" ), _( "Edit the selected songs" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit ) );
        Menu.Append( MenuItem );
    }

    Menu.AppendSeparator();

    MenuItem = new wxMenuItem( &Menu, ID_ALBUMBROWSER_TRACKS_PLAYLIST_SAVE,
                        wxString( _( "Save to Playlist" ) ) +  guAccelGetCommandKeyCodeString( ID_PLAYER_PLAYLIST_SAVE ),
                        _( "Save the selected tracks to playlist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_doc_save ) );
    Menu.Append( MenuItem );

    MenuItem = new wxMenuItem( &Menu, ID_ALBUMBROWSER_TRACKS_SMART_PLAYLIST, _( "Create Smart Playlist" ), _( "Create a smart playlist from this track" ) );
    Menu.Append( MenuItem );

    if( ( ContextMenuFlags & guCONTEXTMENU_COPY_TO ) ||
        ( ContextMenuFlags & guCONTEXTMENU_LINKS ) ||
        ( ContextMenuFlags & guCONTEXTMENU_COMMANDS ) )
    {
        Menu.AppendSeparator();

        if( ContextMenuFlags & guCONTEXTMENU_COPY_TO )
        {
            guMainFrame * MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
            MainFrame->CreateCopyToMenu( &Menu );
        }

        if( ContextMenuFlags & guCONTEXTMENU_LINKS )
        {
            AddOnlineLinksMenu( &Menu );
        }

        if( ContextMenuFlags & guCONTEXTMENU_COMMANDS )
        {
            AddAlbumCommands( &Menu, 1 );
        }
    }

    CreateContextMenu( &Menu );

    // Add Links and Commands
    PopupMenu( &Menu, Point.x, Point.y );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnBigCoverTracksPlayClicked( wxCommandEvent &event )
{
    guTrackArray Tracks;
    GetSelectedTracks( &Tracks );
    DoSelectTracks( Tracks, false );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnBigCoverTracksEnqueueClicked( wxCommandEvent &event )
{
    guTrackArray Tracks;
    GetSelectedTracks( &Tracks );
    DoSelectTracks( Tracks, true, event.GetId() - ID_ALBUMBROWSER_TRACKS_ENQUEUE_AFTER_ALL );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnBigCoverTracksEditLabelsClicked( wxCommandEvent &event )
{
    guTrackArray Tracks;
    GetSelectedTracks( &Tracks );
    if( Tracks.Count() )
    {
        guListItems TrackItems;
        wxArrayInt TrackIds;
        int Index;
        int Count = Tracks.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            TrackItems.Add( new guListItem( Tracks[ Index ].m_SongId, Tracks[ Index ].m_SongName ) );
            TrackIds.Add( Tracks[ Index ].m_SongId );
        }
        guArrayListItems LabelSets = m_Db->GetSongsLabels( TrackIds );

        guLabelEditor * LabelEditor = new guLabelEditor( this, m_Db, _( "Tracks Labels Editor" ), false, &TrackItems, &LabelSets );
        if( LabelEditor )
        {
            if( LabelEditor->ShowModal() == wxID_OK )
            {
                // Update the labels in the files
                m_Db->UpdateSongsLabels( LabelSets );
            }

            LabelEditor->Destroy();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnBigCoverTracksEditTracksClicked( wxCommandEvent &event )
{
    guTrackArray Tracks;
    GetSelectedTracks( &Tracks );
    for( int Index = Tracks.Count() - 1; Index >= 0; Index-- )
    {
        if( Tracks[ Index ].m_Offset )
            Tracks.RemoveAt( Index );
    }
    if( Tracks.Count() )
    {
        guImagePtrArray Images;
        wxArrayString Lyrics;
        wxArrayInt ChangedFlags;

        guTrackEditor * TrackEditor = new guTrackEditor( this, m_Db, &Tracks, &Images, &Lyrics, &ChangedFlags );
        if( TrackEditor )
        {
            if( TrackEditor->ShowModal() == wxID_OK )
            {
                m_MediaViewer->UpdateTracks( Tracks, Images, Lyrics, ChangedFlags );

                // Update the track in database, playlist, etc
                m_MediaViewer->UpdatedTracks( guUPDATED_TRACKS_NONE, &Tracks );
            }

            guImagePtrArrayClean( &Images );

            TrackEditor->Destroy();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnBigCoverPlayClicked( wxCommandEvent &event )
{
    SelectAlbum( m_LastAlbumBrowserItem->m_AlbumId, false );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnBigCoverEnqueueClicked( wxCommandEvent &event )
{
    SelectAlbum( m_LastAlbumBrowserItem->m_AlbumId, true, event.GetId() - ID_ALBUMBROWSER_ENQUEUE_AFTER_ALL );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnBigCoverCopyToClipboard( wxCommandEvent &event )
{
    wxTheClipboard->UsePrimarySelection( false );
    if( wxTheClipboard->Open() )
    {
        wxTheClipboard->Clear();
        if( !wxTheClipboard->AddData( new wxTextDataObject(
            m_LastAlbumBrowserItem->m_ArtistName + wxT( " " ) + m_LastAlbumBrowserItem->m_AlbumName ) ) )
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
void guAlbumBrowser::OnBigCoverAlbumSelectName( wxCommandEvent &event )
{
    OnAlbumSelectName( m_LastAlbumBrowserItem->m_AlbumId );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnBigCoverArtistSelectName( wxCommandEvent &event )
{
    OnArtistSelectName( m_LastAlbumBrowserItem->m_ArtistId );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnBigCoverCommandClicked( wxCommandEvent &event )
{
    if( !m_BigCoverTracksContextMenu )
    {
        OnCommandClicked( event.GetId(), m_LastAlbumBrowserItem->m_AlbumId );
    }
    else
    {
        guTrackArray Tracks;
        GetSelectedTracks( &Tracks );
        OnCommandClicked( event.GetId(), Tracks );
    }
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnBigCoverDownloadCoverClicked( wxCommandEvent &event )
{
    OnAlbumDownloadCoverClicked( m_LastAlbumBrowserItem->m_AlbumId );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnBigCoverSelectCoverClicked( wxCommandEvent &event )
{
    OnAlbumSelectCoverClicked( m_LastAlbumBrowserItem->m_AlbumId );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnBigCoverDeleteCoverClicked( wxCommandEvent &event )
{
    OnAlbumDeleteCoverClicked( m_LastAlbumBrowserItem->m_AlbumId );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnBigCoverEmbedCoverClicked( wxCommandEvent &event )
{
    OnAlbumEmbedCoverClicked( m_LastAlbumBrowserItem->m_AlbumId );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnBigCoverCopyToClicked( wxCommandEvent &event )
{
    if( !m_BigCoverTracksContextMenu )
    {
        OnAlbumCopyToClicked( m_LastAlbumBrowserItem->m_AlbumId, event.GetId() );
    }
    else
    {
        guTrackArray * Tracks = new guTrackArray();
        GetSelectedTracks( Tracks );

        wxCommandEvent CopyEvent( wxEVT_MENU, ID_MAINFRAME_COPYTO );
        int Index = event.GetId() - ID_COPYTO_BASE;
        if( Index >= guCOPYTO_DEVICE_BASE )
        {
            Index -= guCOPYTO_DEVICE_BASE;
            CopyEvent.SetId( ID_MAINFRAME_COPYTODEVICE_TRACKS );
        }
        CopyEvent.SetInt( Index );
        CopyEvent.SetClientData( ( void * ) Tracks );
        wxPostEvent( wxTheApp->GetTopWindow(), CopyEvent );
    }
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnBigCoverEditLabelsClicked( wxCommandEvent &event )
{
    OnAlbumEditLabelsClicked( m_LastAlbumBrowserItem );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnBigCoverEditTracksClicked( wxCommandEvent &event )
{
    OnAlbumEditTracksClicked( m_LastAlbumBrowserItem->m_AlbumId );
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnBigCoverSearchLinkClicked( wxCommandEvent &event )
{
    if( !m_BigCoverTracksContextMenu )
    {
        ExecuteOnlineLink( event.GetId(), m_LastAlbumBrowserItem->m_ArtistName + wxT( " " ) +
                                                      m_LastAlbumBrowserItem->m_AlbumName );
    }
    else
    {
        guTrackArray Tracks;
        GetSelectedTracks( &Tracks );
        int Index;
        int Count = Tracks.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            ExecuteOnlineLink( event.GetId(), Tracks[ Index ].m_ArtistName + wxT( " \"" ) +
                                              Tracks[ Index ].m_SongName + wxT( "\"" ) );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnBigCoverTracksMouseMoved( wxMouseEvent &event )
{
    int Item = m_BigCoverTracksListBox->HitTest( event.GetPosition() );
    bool ResetVals = false;

    // We want to get a better experience for dragging as before
    // when you click over selected items the items was unselected
    // even when you tried to drag then.
    // Here we check if the item was selected and if so wait for the button up
    // to unselecte the item
    //guLogMessage( wxT( "ID: %u LD: %i LU: %i SD: %i CD: %i WasLeftUp: %i  Selecting: %i " ), event.GetId(), event.LeftDown(), event.LeftUp(), event.ShiftDown(), event.ControlDown(), m_MouseWasLeftUp, m_MouseSelecting );
    if( !m_MouseWasLeftUp && !event.ShiftDown() && !event.ControlDown() )
    {
        m_MouseWasLeftUp = event.LeftUp();
        if( ( event.LeftDown() || m_MouseWasLeftUp ) )
        {
            if( Item != wxNOT_FOUND )
            {
                bool Selected = m_BigCoverTracksListBox->IsSelected( Item );
                if( Selected )
                {
                    if( !m_MouseSelecting && event.LeftUp() )
                    {
                        // Its a LeftUp event
                        event.SetEventType( wxEVT_LEFT_DOWN );
                        event.m_leftDown = true;
                        m_BigCoverTracksListBox->GetEventHandler()->AddPendingEvent( event );
                    }
                    return;
                }
                m_MouseSelecting = event.LeftDown();
            }
        }
        else
        {
            ResetVals = true;
        }
    }
    else
    {
        ResetVals = true;
    }

    if( event.Dragging() )
    {
        if( !m_DragCount )
        {
            m_DragStart = event.GetPosition();
        }

        if( ++m_DragCount == 3 )
        {
            wxCommandEvent DragEvent( wxEVT_MENU, ID_ALBUMBROWSER_TRACKS_BEGINDRAG );
            DragEvent.SetEventObject( this );
            GetEventHandler()->ProcessEvent( DragEvent );
        }
        return;
    }
    else
    {
      m_DragCount = 0;
    }

    if( ResetVals )
    {
        m_MouseWasLeftUp = false;
        m_MouseSelecting = false;
    }

    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnBigCoverTracksBeginDrag( wxCommandEvent &event )
{
    int Index;
    int Count;
    //guLogMessage( wxT( "On BeginDrag event..." ) );
    guTrackArray Tracks;
    GetSelectedTracks( &Tracks );
    if( ( Count = Tracks.Count() ) )
    {
        guDataObjectComposite Files;

        wxArrayString Filenames;
        for( Index = 0; Index < Count; Index++ )
        {
            if( Tracks[ Index ].m_Offset )
                continue;
            Filenames.Add( guFileDnDEncode( Tracks[ Index ].m_FileName ) );
        }
        Files.SetTracks( Tracks );
        Files.SetFiles( Filenames );

        wxDropSource source( Files, this );

        wxDragResult Result = source.DoDragDrop( wxDrag_CopyOnly );
        if( Result )
        {
        }
    }
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnBigCoverTracksPlaylistSave( wxCommandEvent &event )
{
    guTrackArray Tracks;
    GetSelectedTracks( &Tracks );
    if( Tracks.Count() )
    {
        wxArrayInt TrackIds;
        int Index;
        int Count = Tracks.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            TrackIds.Add( Tracks[ Index ].m_SongId );
        }
        guListItems PlayLists;
        m_Db->GetPlayLists( &PlayLists, guPLAYLIST_TYPE_STATIC );

        guPlayListAppend * PlayListAppendDlg = new guPlayListAppend( wxTheApp->GetTopWindow(), m_Db, &TrackIds, &PlayLists );

        if( PlayListAppendDlg->ShowModal() == wxID_OK )
        {
            int PLId;
            int Selected = PlayListAppendDlg->GetSelectedPlayList();
            if( Selected == -1 )
            {
                wxString PLName = PlayListAppendDlg->GetPlaylistName();
                if( PLName.IsEmpty() )
                {
                    PLName = _( "UnNamed" );
                }
                PLId = m_Db->CreateStaticPlayList( PLName, TrackIds );
            }
            else
            {
                PLId = PlayLists[ Selected ].m_Id;
                wxArrayInt OldSongs;
                m_Db->GetPlayListSongIds( PLId, &OldSongs );
                if( PlayListAppendDlg->GetSelectedPosition() == 0 ) // BEGIN
                {
                    m_Db->UpdateStaticPlayList( PLId, TrackIds );
                    m_Db->AppendStaticPlayList( PLId, OldSongs );
                }
                else                                                // END
                {
                    m_Db->AppendStaticPlayList( PLId, TrackIds );
                }
            }
            m_Db->UpdateStaticPlayListFile( PLId );

            m_MediaViewer->UpdatePlaylists();
        }
        PlayListAppendDlg->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guAlbumBrowser::OnBigCoverTracksSmartPlaylist( wxCommandEvent &event )
{
    guTrackArray Tracks;
    GetSelectedTracks( &Tracks );
    if( Tracks.Count() )
    {
        m_MediaViewer->CreateSmartPlaylist( Tracks[ 0 ].m_ArtistName, Tracks[ 0 ].m_SongName );
    }
}

// -------------------------------------------------------------------------------- //
wxString guAlbumBrowser::GetSelInfo( void )
{
    if( m_BigCoverShowed )
    {
        return wxString::Format( wxT( "%lu " ), m_BigCoverTracks.Count() ) + _( "Tracks" );
    }
    else
    {
        if( m_AlbumsCount > 0 )
        {
            wxString SelInfo = _( "Albums" ) + wxString::Format( wxT( " %u " ), m_ItemStart + 1 );
            SelInfo += _( "to" ) + wxString::Format( wxT( " %u " ), wxMin( m_ItemStart + m_ItemCount, m_AlbumsCount ) );
            SelInfo += _( "of" ) + wxString::Format( wxT( " %u " ), m_AlbumsCount );
            return SelInfo;
        }
    }
    return wxEmptyString;
}




// -------------------------------------------------------------------------------- //
// guAlbumBrowserDropTarget
// -------------------------------------------------------------------------------- //
guAlbumBrowserDropTarget::guAlbumBrowserDropTarget( guMediaViewer * mediaviewer, guAlbumBrowserItemPanel * itempanel ) :
    wxDropTarget()
{
    m_MediaViewer = mediaviewer;
    m_AlbumBrowserItemPanel = itempanel;

    wxDataObjectComposite * DataObject = new wxDataObjectComposite();
    wxCustomDataObject * TracksDataObject = new wxCustomDataObject( wxDataFormat( wxT( "x-gutracks/guayadeque-copied-tracks" ) ) );
    DataObject->Add( TracksDataObject, true );
    wxFileDataObject * FileDataObject = new wxFileDataObject();
    DataObject->Add( FileDataObject, false );
    SetDataObject( DataObject );
}

// -------------------------------------------------------------------------------- //
wxDragResult guAlbumBrowserDropTarget::OnData( wxCoord x, wxCoord y, wxDragResult def )
{
    //guLogMessage( wxT( "guAlbumBrowserDropTarget::OnData" ) );

    if( def == wxDragError || def == wxDragNone || def == wxDragCancel )
        return def;

    if( !GetData() )
    {
        guLogMessage( wxT( "Error getting drop data" ) );
        return wxDragError;
    }

    guDataObjectComposite * DataObject = ( guDataObjectComposite * ) m_dataObject;

    wxDataFormat ReceivedFormat = DataObject->GetReceivedFormat();
    //guLogMessage( wxT( "ReceivedFormat: '%s'" ), ReceivedFormat.GetId().c_str() );
    if( ReceivedFormat == wxDataFormat( wxT( "x-gutracks/guayadeque-copied-tracks" ) ) )
    {
        guTrackArray * Tracks;
        if( !DataObject->GetDataHere( ReceivedFormat, &Tracks ) )
        {
          guLogMessage( wxT( "Error getting tracks data..." ) );
        }
        else
        {
            m_MediaViewer->ImportFiles( new guTrackArray( * Tracks ) );
        }
    }
    else if( ReceivedFormat == wxDataFormat( wxDF_FILENAME ) )
    {
        wxFileDataObject * FileDataObject = ( wxFileDataObject * ) DataObject->GetDataObject( wxDataFormat( wxDF_FILENAME ) );
        if( FileDataObject )
        {
            wxArrayString Files = FileDataObject->GetFilenames();
            if( Files.Count() )
            {
                wxString CoverFile = Files[ 0 ];
                if( !CoverFile.IsEmpty() && wxFileExists( CoverFile ) &&
                    guIsValidImageFile( CoverFile.Lower() ) )
                {
                    m_AlbumBrowserItemPanel->SetAlbumCover( CoverFile );
                }
                else
                {
                    m_MediaViewer->ImportFiles( Files );
                }
            }
        }
    }

    return def;
}

}

// -------------------------------------------------------------------------------- //
