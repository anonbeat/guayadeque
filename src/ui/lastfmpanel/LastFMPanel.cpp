// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2020 J.Rios anonbeat@gmail.com
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
#include "LastFMPanel.h"

#include "EventCommandIds.h"
#include "Http.h"
#include "Images.h"
#include "MainApp.h"
#include "Settings.h"
#include "ShowImage.h"
#include "TagInfo.h"
#include "Utils.h"
#include "AuiNotebook.h"
#include "OnlineLinks.h"
#include "PlayListAppend.h"
#include "MainFrame.h"

#include <wx/arrimpl.cpp>
#include "wx/clipbrd.h"
#include <wx/statline.h>
#include <wx/uri.h>

namespace Guayadeque {

#define GULASTFM_TITLE_FONT_SIZE 12

#define GULASTFM_DOWNLOAD_IMAGE_DELAY    10

WX_DEFINE_OBJARRAY( guLastFMInfoArray )
WX_DEFINE_OBJARRAY( guLastFMSimilarArtistInfoArray )
WX_DEFINE_OBJARRAY( guLastFMTrackInfoArray )
WX_DEFINE_OBJARRAY( guLastFMAlbumInfoArray )
WX_DEFINE_OBJARRAY( guLastFMTopTrackInfoArray )

// -------------------------------------------------------------------------------- //
guHtmlWindow::guHtmlWindow( wxWindow * parent, wxWindowID id, const wxPoint &pos, const wxSize &size, long style ) :
                wxHtmlWindow( parent, id, pos, size, style )
{
    Bind( wxEVT_SIZE, &guHtmlWindow::OnChangedSize, this );
    Bind( wxEVT_MENU, &guHtmlWindow::OnScrollTo, this, guEVT_USER_FIRST );
}

// -------------------------------------------------------------------------------- //
guHtmlWindow::~guHtmlWindow()
{
    Unbind( wxEVT_SIZE, &guHtmlWindow::OnChangedSize, this );
    Unbind( wxEVT_MENU, &guHtmlWindow::OnScrollTo, this, guEVT_USER_FIRST );
}

// -------------------------------------------------------------------------------- //
void guHtmlWindow::OnScrollTo( wxCommandEvent &event )
{
    //guLogMessage( wxT( "Need to scroll to %i, %i" ), event.GetInt(), event.GetExtraLong() );
    Scroll( event.GetInt(), event.GetExtraLong() );
}

// -------------------------------------------------------------------------------- //
void guHtmlWindow::OnChangedSize( wxSizeEvent &event )
{
    //wxSize Size = event.GetSize();
    //wxSize ClientSize = GetClientSize();

    int ScrollX;
    int ScrollY;
    CalcUnscrolledPosition( 0, 0, &ScrollX, &ScrollY );
    //guLogMessage( wxT( "Initial position : %i, %i" ), ScrollX, ScrollY );

    wxHtmlWindow::OnSize( event );

    if( ScrollX || ScrollY )
    {
        //guLogMessage( wxT( "Setting position to %i, %i" ), ScrollX, ScrollY );
        wxCommandEvent SizeEvent( wxEVT_MENU, guEVT_USER_FIRST );
        SizeEvent.SetInt( ScrollX / wxHTML_SCROLL_STEP );
        SizeEvent.SetExtraLong( ScrollY / wxHTML_SCROLL_STEP );
        AddPendingEvent( SizeEvent );
    }
}




// -------------------------------------------------------------------------------- //
// guLastFMInfoCtrl
// -------------------------------------------------------------------------------- //
guLastFMInfoCtrl::guLastFMInfoCtrl( wxWindow * parent, guDbLibrary * db, guDbCache * dbcache, guPlayerPanel * playerpanel, bool createcontrols ) :
    wxPanel( parent, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), wxTAB_TRAVERSAL )
{
    m_DefaultDb = db;
    m_Db = NULL;
    m_MediaViewer = NULL;
    m_DbCache = dbcache;
    m_PlayerPanel = playerpanel;
    m_NormalColor = wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT );
    m_NotFoundColor = wxSystemSettings::GetColour( wxSYS_COLOUR_GRAYTEXT );

    if( createcontrols )
        this->CreateControls( parent );


    Bind( wxEVT_MENU, &guLastFMInfoCtrl::OnSearchLinkClicked, this, ID_LINKS_BASE, ID_LINKS_BASE + guLINKS_MAXCOUNT );
    Bind( wxEVT_MENU, &guLastFMInfoCtrl::OnSearchLinkClicked, this, ID_LASTFM_VISIT_URL );

    Bind( wxEVT_CONTEXT_MENU, &guLastFMInfoCtrl::OnContextMenu, this );
    Bind( wxEVT_MENU, &guLastFMInfoCtrl::OnPlayClicked, this, ID_LASTFM_PLAY );
    Bind( wxEVT_MENU, &guLastFMInfoCtrl::OnEnqueueClicked, this, ID_LASTFM_ENQUEUE_AFTER_ALL, ID_LASTFM_ENQUEUE_AFTER_ARTIST );
    Bind( wxEVT_MENU, &guLastFMInfoCtrl::OnCopyToClipboard, this, ID_LASTFM_COPYTOCLIPBOARD );
    Bind( wxEVT_MENU, &guLastFMInfoCtrl::OnSongSelectName, this, ID_TRACKS_SELECTNAME );
    Bind( wxEVT_MENU, &guLastFMInfoCtrl::OnArtistSelectName, this, ID_ARTIST_SELECTNAME );
    Bind( wxEVT_MENU, &guLastFMInfoCtrl::OnAlbumSelectName, this, ID_ALBUM_SELECTNAME );

    Bind( wxEVT_MOTION, &guLastFMInfoCtrl::OnMouse, this );
    Bind( wxEVT_ENTER_WINDOW, &guLastFMInfoCtrl::OnMouse, this );
    Bind( wxEVT_LEAVE_WINDOW, &guLastFMInfoCtrl::OnMouse, this );
    Bind( wxEVT_RIGHT_DOWN, &guLastFMInfoCtrl::OnMouse, this );
}

// -------------------------------------------------------------------------------- //
guLastFMInfoCtrl::~guLastFMInfoCtrl()
{
    Unbind( wxEVT_MENU, &guLastFMInfoCtrl::OnSearchLinkClicked, this, ID_LINKS_BASE, ID_LINKS_BASE + guLINKS_MAXCOUNT );
    Unbind( wxEVT_MENU, &guLastFMInfoCtrl::OnSearchLinkClicked, this, ID_LASTFM_VISIT_URL );

    Unbind( wxEVT_CONTEXT_MENU, &guLastFMInfoCtrl::OnContextMenu, this );
    Unbind( wxEVT_MENU, &guLastFMInfoCtrl::OnPlayClicked, this, ID_LASTFM_PLAY );
    Unbind( wxEVT_MENU, &guLastFMInfoCtrl::OnEnqueueClicked, this, ID_LASTFM_ENQUEUE_AFTER_ALL, ID_LASTFM_ENQUEUE_AFTER_ARTIST );
    Unbind( wxEVT_MENU, &guLastFMInfoCtrl::OnCopyToClipboard, this, ID_LASTFM_COPYTOCLIPBOARD );
    Unbind( wxEVT_MENU, &guLastFMInfoCtrl::OnSongSelectName, this, ID_TRACKS_SELECTNAME );
    Unbind( wxEVT_MENU, &guLastFMInfoCtrl::OnArtistSelectName, this, ID_ARTIST_SELECTNAME );
    Unbind( wxEVT_MENU, &guLastFMInfoCtrl::OnAlbumSelectName, this, ID_ALBUM_SELECTNAME );

    Unbind( wxEVT_MOTION, &guLastFMInfoCtrl::OnMouse, this );
    Unbind( wxEVT_ENTER_WINDOW, &guLastFMInfoCtrl::OnMouse, this );
    Unbind( wxEVT_LEAVE_WINDOW, &guLastFMInfoCtrl::OnMouse, this );
    Unbind( wxEVT_RIGHT_DOWN, &guLastFMInfoCtrl::OnMouse, this );

    m_Text->Unbind( wxEVT_LEFT_DCLICK, &guLastFMInfoCtrl::OnDoubleClicked, this );
    m_Text->Unbind( wxEVT_MOTION, &guLastFMInfoCtrl::OnMouse, this );
    m_Text->Unbind( wxEVT_ENTER_WINDOW, &guLastFMInfoCtrl::OnMouse, this );
    m_Text->Unbind( wxEVT_LEAVE_WINDOW, &guLastFMInfoCtrl::OnMouse, this );
    m_Text->Unbind( wxEVT_RIGHT_DOWN, &guLastFMInfoCtrl::OnMouse, this );

    m_Bitmap->Unbind( wxEVT_LEFT_DOWN, &guLastFMInfoCtrl::OnBitmapClicked, this );
}

// -------------------------------------------------------------------------------- //
void guLastFMInfoCtrl::CreateControls( wxWindow * parent )
{
	wxBoxSizer* MainSizer;
	MainSizer = new wxBoxSizer( wxHORIZONTAL );

	m_Bitmap = new wxStaticBitmap( this, wxID_ANY, guImage( guIMAGE_INDEX_default_lastfm_image ),
	                                            wxDefaultPosition, wxSize( 50, 50 ), 0 );
    //Bitmap->SetCursor( wxCURSOR_HAND );
	MainSizer->Add( m_Bitmap, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );

	m_Text = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 200, -1 ), 0 );
	m_Text->Wrap( -1 );
	//Text->SetCursor( wxCursor( wxCURSOR_HAND ) );
	//m_Text->SetMaxSize( wxSize( 250, -1 ) );
	MainSizer->Add( m_Text, 1, wxALL|wxEXPAND, 2 );


	SetSizer( MainSizer );
	Layout();
	MainSizer->Fit( this );

    m_Text->Bind( wxEVT_LEFT_DCLICK, &guLastFMInfoCtrl::OnDoubleClicked, this );
    m_Text->Bind( wxEVT_MOTION, &guLastFMInfoCtrl::OnMouse, this );
    m_Text->Bind( wxEVT_ENTER_WINDOW, &guLastFMInfoCtrl::OnMouse, this );
    m_Text->Bind( wxEVT_LEAVE_WINDOW, &guLastFMInfoCtrl::OnMouse, this );
    m_Text->Bind( wxEVT_RIGHT_DOWN, &guLastFMInfoCtrl::OnMouse, this );

    m_Bitmap->Bind( wxEVT_LEFT_DOWN, &guLastFMInfoCtrl::OnBitmapClicked, this );
}

// -------------------------------------------------------------------------------- //
void guLastFMInfoCtrl::SetMediaViewer( guMediaViewer * mediaviewer )
{
    wxMutexLocker Lock( m_DbMutex );
    m_MediaViewer = mediaviewer;
    m_Db = mediaviewer ? mediaviewer->GetDb() : NULL;
}

// -------------------------------------------------------------------------------- //
void guLastFMInfoCtrl::Clear( guMediaViewer * mediaviewer )
{
    m_Bitmap->SetBitmap( guImage( guIMAGE_INDEX_default_lastfm_image ) );
    m_Text->SetLabel( wxEmptyString );
    //
    SetMediaViewer( mediaviewer );
}

// -------------------------------------------------------------------------------- //
void guLastFMInfoCtrl::SetBitmap( const wxImage * image )
{
    if( image )
    {
        m_Bitmap->SetBitmap( wxBitmap( * image ) );
    }
    else
    {
        m_Bitmap->SetBitmap( guImage( guIMAGE_INDEX_default_lastfm_image ) );
    }
    m_Bitmap->Refresh();
}

// -------------------------------------------------------------------------------- //
void guLastFMInfoCtrl::SetLabel( const wxString &label )
{
    wxString Label = label;
    Label.Replace( wxT( "&" ), wxT( "&&" ) );
    m_Text->SetLabel( Label );
//    Layout();
}

// -------------------------------------------------------------------------------- //
void guLastFMInfoCtrl::OnContextMenu( wxContextMenuEvent& event )
{
    wxMenu Menu;
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

    CreateContextMenu( &Menu );
    PopupMenu( &Menu, Point.x, Point.y );
}

// -------------------------------------------------------------------------------- //
void guLastFMInfoCtrl::CreateContextMenu( wxMenu * Menu )
{
    AddOnlineLinksMenu( Menu );
}

// -------------------------------------------------------------------------------- //
void guLastFMInfoCtrl::OnDoubleClicked( wxMouseEvent &event )
{
    guTrackArray Tracks;
    GetSelectedTracks( &Tracks );
    if( Tracks.Count() )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        if( m_PlayerPanel && Config )
        {
            if( Config->ReadBool( CONFIG_KEY_GENERAL_ACTION_ENQUEUE, false, CONFIG_PATH_GENERAL ) )
            {
                m_PlayerPanel->AddToPlayList( Tracks );
            }
            else
            {
                m_PlayerPanel->SetPlayList( Tracks );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guLastFMInfoCtrl::OnSearchLinkClicked( wxCommandEvent &event )
{
    int Index = event.GetId();
    if( Index == ID_LASTFM_VISIT_URL )
    {
        guWebExecute( GetItemUrl() );
    }
    else
    {
        ExecuteOnlineLink( Index, GetSearchText() );
    }
}

// -------------------------------------------------------------------------------- //
void guLastFMInfoCtrl::OnCopyToClipboard( wxCommandEvent &event )
{
    //guLogMessage( wxT( "OnCopyToClipboard : %s" ), GetSearchText().c_str() );
    wxTheClipboard->UsePrimarySelection( false );
    if( wxTheClipboard->Open() )
    {
        wxTheClipboard->Clear();
        if( !wxTheClipboard->AddData( new wxTextDataObject( GetSearchText() ) ) )
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
void guLastFMInfoCtrl::OnPlayClicked( wxCommandEvent &event )
{
    guTrackArray Tracks;
    GetSelectedTracks( &Tracks );
    if( m_PlayerPanel && Tracks.Count() )
    {
        m_PlayerPanel->SetPlayList( Tracks );
    }
}

// -------------------------------------------------------------------------------- //
void guLastFMInfoCtrl::OnEnqueueClicked( wxCommandEvent &event )
{
    guTrackArray Tracks;
    GetSelectedTracks( &Tracks );
    if( m_PlayerPanel && Tracks.Count() )
    {
        m_PlayerPanel->AddToPlayList( Tracks, true, event.GetId() - ID_LASTFM_ENQUEUE_AFTER_ALL );
    }
}

// -------------------------------------------------------------------------------- //
void guLastFMInfoCtrl::OnBitmapClicked( wxMouseEvent &event )
{
    wxBitmapType ImageType;
    wxString ImageUrl = GetBitmapImageUrl();
    if( !ImageUrl.IsEmpty() )
    {
        wxImage * Image = m_DbCache->GetImage( ImageUrl, ImageType, guDBCACHE_TYPE_IMAGE_SIZE_BIG );
        if( Image )
        {
            guShowImage * ShowImage = new guShowImage( GetParent(), Image, ClientToScreen( m_Bitmap->GetPosition() ) );
            if( ShowImage )
            {
                ShowImage->Show();
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
wxString guLastFMInfoCtrl::GetBitmapImageUrl( void )
{
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
void guLastFMInfoCtrl::OnMouse( wxMouseEvent &event )
{
    //guLogMessage( wxT( "Mouse: %i %i" ), event.m_x, event.m_y );
    if( !ItemWasFound() )
    {
        if( event.Entering() || event.Leaving() || event.RightDown() )
        {
            wxString LabelText = m_Text->GetLabel();
            LabelText.Replace( wxT( "&" ), wxT( "&&" ) );
            //guLogMessage( wxT( "Entering..." ) );
            m_Text->SetForegroundColour( event.Entering() ? m_NormalColor : m_NotFoundColor );
            m_Text->SetLabel( LabelText );
            Layout();
        }
    }
    event.Skip();
}




// -------------------------------------------------------------------------------- //
// guArtistInfoCtrl
// -------------------------------------------------------------------------------- //
guArtistInfoCtrl::guArtistInfoCtrl( wxWindow * parent, guDbLibrary * db, guDbCache * dbcache, guPlayerPanel * playerpanel ) :
                 guLastFMInfoCtrl( parent, db, dbcache, playerpanel, false )
{
    m_Info = NULL;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    m_ShowLongBioText = Config->ReadBool( CONFIG_KEY_LASTFM_SHOW_LONG_BIO, false, CONFIG_PATH_LASTFM  );

    CreateControls( parent );

    m_ShowMoreHyperLink->Bind( wxEVT_HYPERLINK, &guArtistInfoCtrl::OnShowMoreLinkClicked, this );
    m_ArtistDetails->Bind( wxEVT_HTML_LINK_CLICKED, &guArtistInfoCtrl::OnHtmlLinkClicked, this );
};

// -------------------------------------------------------------------------------- //
guArtistInfoCtrl::~guArtistInfoCtrl()
{
    if( m_Info )
        delete m_Info;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->WriteBool( CONFIG_KEY_LASTFM_SHOW_LONG_BIO, m_ShowLongBioText, CONFIG_PATH_LASTFM  );

    m_ShowMoreHyperLink->Unbind( wxEVT_HYPERLINK, &guArtistInfoCtrl::OnShowMoreLinkClicked, this );
    m_ArtistDetails->Unbind( wxEVT_HTML_LINK_CLICKED, &guArtistInfoCtrl::OnHtmlLinkClicked, this );

    m_Text->Unbind( wxEVT_LEFT_DCLICK, &guArtistInfoCtrl::OnDoubleClicked, this );
    m_Bitmap->Unbind( wxEVT_LEFT_DOWN, &guArtistInfoCtrl::OnBitmapClicked, this );
}

// -------------------------------------------------------------------------------- //
void guArtistInfoCtrl::CreateControls( wxWindow * parent )
{
	m_MainSizer = new wxBoxSizer( wxHORIZONTAL );

	m_Bitmap = new wxStaticBitmap( this, wxID_ANY, guImage( guIMAGE_INDEX_no_photo ), wxDefaultPosition, wxSize( 100,100 ), 0 );
	m_MainSizer->Add( m_Bitmap, 0, wxALL, 5 );

//	wxBoxSizer * DetailSizer;
	m_DetailSizer = new wxBoxSizer( wxVERTICAL );

	wxSizer * TopSizer = new wxBoxSizer( wxHORIZONTAL );

	m_Text = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_Text->Wrap( -1 );
	wxFont CurrentFont = wxSystemSettings::GetFont( wxSYS_SYSTEM_FONT );
    CurrentFont.SetPointSize( CurrentFont.GetPointSize() + 2 );
	CurrentFont.SetWeight( wxFONTWEIGHT_BOLD );
	m_Text->SetFont( CurrentFont );

	TopSizer->Add( m_Text, 0, wxEXPAND|wxTOP|wxRIGHT|wxLEFT, 5 );

	TopSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	m_ShowMoreHyperLink = new wxHyperlinkCtrl( this, wxID_ANY, m_ShowLongBioText ? _( "Less..." ) : _("More..."), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
	m_ShowMoreHyperLink->SetNormalColour( wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHT ) );
	m_ShowMoreHyperLink->SetVisitedColour( wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHT ) );
	m_ShowMoreHyperLink->SetHoverColour( wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHT ) );
	//m_DetailSizer->Add( m_ShowMoreHyperLink, 0, wxALL|wxALIGN_RIGHT, 5 );

	TopSizer->Add( m_ShowMoreHyperLink, 0, wxTOP|wxRIGHT|wxLEFT, 5 );


	//m_DetailSizer->Add( m_Text, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	m_DetailSizer->Add( TopSizer, 0, wxEXPAND, 5 );

	m_ArtistDetails = new guHtmlWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_AUTO );
	m_ArtistDetails->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT ) );
    CurrentFont.SetPointSize( CurrentFont.GetPointSize() - 2 );
	CurrentFont.SetWeight( wxFONTWEIGHT_NORMAL );
	m_ArtistDetails->SetFonts( CurrentFont.GetFaceName(), wxEmptyString );

	m_ArtistDetails->SetBackgroundColour( m_Text->GetBackgroundColour() );
	m_ArtistDetails->SetBorders( 0 );
	m_DetailSizer->Add( m_ArtistDetails, 1, wxALL|wxEXPAND, 5 );

	m_MainSizer->Add( m_DetailSizer, 1, wxEXPAND, 5 );

	SetSizer( m_MainSizer );
	Layout();
	//MainSizer->Fit( this );

    m_Text->Bind( wxEVT_LEFT_DCLICK, &guArtistInfoCtrl::OnDoubleClicked, this );

    m_Bitmap->Bind( wxEVT_LEFT_DOWN, &guArtistInfoCtrl::OnBitmapClicked, this );
}

// -------------------------------------------------------------------------------- //
void guArtistInfoCtrl::SetInfo( guLastFMArtistInfo * info )
{
    if( m_Info )
        delete m_Info;
    m_Info = info;

    m_DbMutex.Lock();
    m_Info->m_ArtistId = m_Db ? m_Db->FindArtist( m_Info->m_Artist->m_Name ) :
                                m_DefaultDb->FindArtist( m_Info->m_Artist->m_Name );
    m_DbMutex.Unlock();

    m_Text->SetForegroundColour( m_Info->m_ArtistId == wxNOT_FOUND ?
                                        m_NotFoundColor : m_NormalColor );
    SetBitmap( m_Info->m_Image );
    SetLabel( m_Info->m_Artist->m_Name );

    UpdateArtistInfoText();
}

// -------------------------------------------------------------------------------- //
void guArtistInfoCtrl::SetMediaViewer( guMediaViewer * mediaviewer )
{
    wxMutexLocker Lock( m_DbMutex );
    m_MediaViewer = mediaviewer;
    m_Db = mediaviewer ? mediaviewer->GetDb() : NULL;

    if( m_Info )
    {
        m_Info->m_ArtistId = wxNOT_FOUND;

        m_Text->SetForegroundColour( m_NotFoundColor );
    }
}

// -------------------------------------------------------------------------------- //
void guArtistInfoCtrl::Clear( guMediaViewer * mediaviewer )
{
    m_Bitmap->SetBitmap( guImage( guIMAGE_INDEX_no_photo ) );
    m_Text->SetLabel( wxEmptyString );

    if( m_Info )
        delete m_Info;
    m_Info = NULL;

    SetMediaViewer( mediaviewer );

    UpdateArtistInfoText();
}

// -------------------------------------------------------------------------------- //
void guArtistInfoCtrl::SetBitmap( const wxImage * image )
{
    if( image )
    {
        m_Bitmap->SetBitmap( wxBitmap( * image ) );
    }
    else
    {
        m_Bitmap->SetBitmap( guImage( guIMAGE_INDEX_no_photo ) );
    }
    m_Bitmap->Refresh();
}

// -------------------------------------------------------------------------------- //
wxString guArtistInfoCtrl::GetSearchText( void )
{
    return m_Info->m_Artist->m_Name;
}

// -------------------------------------------------------------------------------- //
wxString guArtistInfoCtrl::GetItemUrl( void )
{
    return m_Info->m_Artist->m_Url;
}

// -------------------------------------------------------------------------------- //
void guArtistInfoCtrl::CreateContextMenu( wxMenu * Menu )
{
    wxMenuItem * MenuItem;

    if( m_Info->m_ArtistId != wxNOT_FOUND )
    {
        //guLogMessage( wxT( "The artist id = %i" ), m_Info->m_ArtistId );
        MenuItem = new wxMenuItem( Menu, ID_LASTFM_PLAY, _( "Play" ), _( "Play the artist tracks" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_LASTFM_ENQUEUE_AFTER_ALL, _( "Enqueue" ), _( "Enqueue the artist tracks to the playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        Menu->Append( MenuItem );

        wxMenu * EnqueueMenu = new wxMenu();

        MenuItem = new wxMenuItem( EnqueueMenu, ID_LASTFM_ENQUEUE_AFTER_TRACK,
                                wxString( _( "Current Track" ) ),
                                _( "Add current selected tracks to playlist after the current track" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );

        MenuItem = new wxMenuItem( EnqueueMenu, ID_LASTFM_ENQUEUE_AFTER_ALBUM,
                                wxString( _( "Current Album" ) ),
                                _( "Add current selected tracks to playlist after the current album" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );

        MenuItem = new wxMenuItem( EnqueueMenu, ID_LASTFM_ENQUEUE_AFTER_ARTIST,
                                wxString( _( "Current Artist" ) ),
                                _( "Add current selected tracks to playlist after the current artist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );

        Menu->Append( wxID_ANY, _( "Enqueue After" ), EnqueueMenu );

        Menu->AppendSeparator();
        if( m_MediaViewer )
        {
            MenuItem = new wxMenuItem( Menu, ID_ARTIST_SELECTNAME, _( "Search Artist" ), _( "Search the artist in the library" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_search ) );
            Menu->Append( MenuItem );
            Menu->AppendSeparator();
        }
    }

    if( !GetSearchText().IsEmpty() )
    {
        MenuItem = new wxMenuItem( Menu, ID_LASTFM_COPYTOCLIPBOARD, _( "Copy to Clipboard" ), _( "Copy the artist name to clipboard" ) );
        //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_copy ) );
        Menu->Append( MenuItem );
        Menu->AppendSeparator();
    }

    if( !m_Info->m_Artist->m_Url.IsEmpty() )
    {
        MenuItem = new wxMenuItem( Menu, ID_LASTFM_VISIT_URL, wxT( "Last.fm" ), _( "Visit last.fm page for this item" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_lastfm_as_on ) );
        Menu->Append( MenuItem );
        Menu->AppendSeparator();
    }

    guLastFMInfoCtrl::CreateContextMenu( Menu );
}

// -------------------------------------------------------------------------------- //
void guArtistInfoCtrl::OnContextMenu( wxContextMenuEvent& event )
{
    // If the item have not been set we do nothing
    if( !m_Info )
        return;

    guLastFMInfoCtrl::OnContextMenu( event );
}

// -------------------------------------------------------------------------------- //
void guArtistInfoCtrl::UpdateArtistInfoText( void )
{
    wxString Content;
    if( !m_Info )
    {
        Content = _( "There is no information available for this artist." );
    }
    else
    {
        Content = m_ShowLongBioText ? m_Info->m_Artist->m_BioContent.c_str() :
                                    m_Info->m_Artist->m_BioSummary.c_str();
    }
    //guLogMessage( wxT( "HTML:\n%s\n" ), Content.c_str() );
    while( Content.EndsWith( wxT( "\n" ) ) )
    {
        Content.Truncate( 1 );
    }
    Content.Replace( wxT( "\n" ), wxT( "<br>" ) );

    wxString DetailsContent = wxT( "<html><body style=\"background-color: " ) +
        m_Text->GetBackgroundColour().GetAsString( wxC2S_CSS_SYNTAX ) +
        wxT( "; color: " ) +
        wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT ).GetAsString( wxC2S_CSS_SYNTAX ) +
        wxT( "; a:link " ) +
        wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHT ).GetAsString( wxC2S_CSS_SYNTAX ) +
        wxT( ";\">" ) +
        Content +
        wxT( "</body></html>" );
    m_ArtistDetails->SetPage( DetailsContent );

    wxSize Size; // = wxDefaultSize; //ArtistDetails->GetSize();
    wxHtmlContainerCell * Cell = m_ArtistDetails->GetInternalRepresentation();
    Size.SetHeight( Cell->GetHeight() + 15 ); // This makes the scroll bar to not appear if not needed
    Size.SetWidth( 100 );
    if( Size.y > 300 )
    {
        Size.SetHeight( 300 );
    }
    m_ArtistDetails->SetMinSize( Size );
    //guLogMessage( wxT( "*Size : %i - %i" ), Size.x, Size.y );
    m_DetailSizer->Fit( m_ArtistDetails );
    //Size = ArtistDetails->GetMinSize();
    //guLogMessage( wxT( " Size : %i - %i" ), Size.x, Size.y );
/////////    MainSizer->FitInside( this );
    //DetailSizer->Layout();
    Layout();
    ( ( guLastFMPanel * ) GetParent() )->UpdateLayout();
}

// -------------------------------------------------------------------------------- //
void guArtistInfoCtrl::OnShowMoreLinkClicked( wxHyperlinkEvent &event )
{
    //guLogMessage( wxT( "OnShowMoreLinkClicked" ) );
    m_ShowLongBioText = !m_ShowLongBioText;
    m_ShowMoreHyperLink->SetLabel( m_ShowLongBioText ? _( "Less..." ) : _( "More..." ) );
    UpdateArtistInfoText();
}

// -------------------------------------------------------------------------------- //
void guArtistInfoCtrl::OnHtmlLinkClicked( wxHtmlLinkEvent& event )
{
    //guLogMessage( wxT( "OnHtmlLinkClicked" ) );
    wxHtmlLinkInfo LinkInfo = event.GetLinkInfo();
    //wxLaunchDefaultBrowser( LinkInfo.GetHref() );
    guWebExecute( LinkInfo.GetHref().c_str() );
}

// -------------------------------------------------------------------------------- //
int guArtistInfoCtrl::GetSelectedTracks( guTrackArray * tracks )
{
    if( m_Info->m_ArtistId != wxNOT_FOUND )
    {
        wxArrayInt Selections;
        Selections.Add( m_Info->m_ArtistId );

        wxMutexLocker Lock( m_DbMutex );
        return m_Db ? m_Db->GetArtistsSongs( Selections, tracks ) :
                      m_DefaultDb->GetArtistsSongs( Selections, tracks );
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
void guArtistInfoCtrl::OnArtistSelectName( wxCommandEvent &event )
{
    if( m_MediaViewer )
    {
        wxCommandEvent evt( wxEVT_MENU, ID_MAINFRAME_SELECT_ARTIST );
        evt.SetInt( m_Info->m_ArtistId );
        evt.SetExtraLong( guTRACK_TYPE_DB );
        evt.SetClientData( m_MediaViewer );
        wxPostEvent( m_MediaViewer, evt );
    }
}

// -------------------------------------------------------------------------------- //
void guArtistInfoCtrl::OnCopyToClipboard( wxCommandEvent &event )
{
    if( !m_Info || !m_Info->m_Artist )
        return;

    wxTheClipboard->UsePrimarySelection( false );
    if( wxTheClipboard->Open() )
    {
        wxTheClipboard->Clear();
        wxString CopyText = m_ArtistDetails->SelectionToText();
        if( CopyText.IsEmpty() )
        {
            CopyText = m_Info->m_Artist->m_Name + wxT( "\n" ) +
                        ( m_ShowLongBioText ? m_Info->m_Artist->m_BioContent :
                                              m_Info->m_Artist->m_BioSummary );
        }
        if( !wxTheClipboard->AddData( new wxTextDataObject( CopyText ) ) )
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
// guAlbumInfoCtrl
// -------------------------------------------------------------------------------- //
guAlbumInfoCtrl::guAlbumInfoCtrl( wxWindow * parent, guDbLibrary * db, guDbCache * dbcache, guPlayerPanel * playerpanel ) :
                 guLastFMInfoCtrl( parent, db, dbcache, playerpanel )
{
    m_Info = NULL;
};

// -------------------------------------------------------------------------------- //
guAlbumInfoCtrl::~guAlbumInfoCtrl()
{
    if( m_Info )
        delete m_Info;
}

// -------------------------------------------------------------------------------- //
void guAlbumInfoCtrl::SetInfo( guLastFMAlbumInfo * info )
{
    if( m_Info )
        delete m_Info;
    m_Info = info;

    m_DbMutex.Lock();
    m_Info->m_AlbumId = m_Db ? m_Db->FindAlbum( m_Info->m_Album->m_Artist, m_Info->m_Album->m_Name ) :
                               m_DefaultDb->FindAlbum( m_Info->m_Album->m_Artist, m_Info->m_Album->m_Name );
    m_DbMutex.Unlock();
    m_Text->SetForegroundColour( m_Info->m_AlbumId == wxNOT_FOUND ?
                                        m_NotFoundColor : m_NormalColor );

    SetBitmap( m_Info->m_Image );
    SetLabel( m_Info->m_Album->m_Name );
}

// -------------------------------------------------------------------------------- //
void guAlbumInfoCtrl::SetMediaViewer( guMediaViewer * mediaviewer )
{
    wxMutexLocker Lock( m_DbMutex );
    m_MediaViewer = mediaviewer;
    m_Db = mediaviewer ? mediaviewer->GetDb() : NULL;

    if( m_Info )
    {
        m_Info->m_AlbumId = wxNOT_FOUND;
        m_Text->SetForegroundColour( m_NotFoundColor );
    }
}

// -------------------------------------------------------------------------------- //
void guAlbumInfoCtrl::Clear( guMediaViewer * mediaviewer )
{
    if( m_Info )
        delete m_Info;
    m_Info = NULL;

    guLastFMInfoCtrl::Clear( mediaviewer );
}

// -------------------------------------------------------------------------------- //
wxString guAlbumInfoCtrl::GetSearchText( void )
{
    return wxString::Format( wxT( "%s %s" ), m_Info->m_Album->m_Artist.c_str(), m_Info->m_Album->m_Name.c_str() );
}

// -------------------------------------------------------------------------------- //
wxString guAlbumInfoCtrl::GetItemUrl( void )
{
    return m_Info->m_Album->m_Url;
}

// -------------------------------------------------------------------------------- //
void guAlbumInfoCtrl::CreateContextMenu( wxMenu * Menu )
{
    wxMenuItem * MenuItem;

    if( m_Info->m_AlbumId != wxNOT_FOUND )
    {
        MenuItem = new wxMenuItem( Menu, ID_LASTFM_PLAY, _( "Play" ), _( "Play the album tracks" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_LASTFM_ENQUEUE_AFTER_ALL, _( "Enqueue" ), _( "Enqueue the album tracks to the playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        Menu->Append( MenuItem );

        wxMenu * EnqueueMenu = new wxMenu();

        MenuItem = new wxMenuItem( EnqueueMenu, ID_LASTFM_ENQUEUE_AFTER_TRACK,
                                wxString( _( "Current Track" ) ),
                                _( "Add current selected tracks to playlist after the current track" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );

        MenuItem = new wxMenuItem( EnqueueMenu, ID_LASTFM_ENQUEUE_AFTER_ALBUM,
                                wxString( _( "Current Album" ) ),
                                _( "Add current selected tracks to playlist after the current album" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );

        MenuItem = new wxMenuItem( EnqueueMenu, ID_LASTFM_ENQUEUE_AFTER_ARTIST,
                                wxString( _( "Current Artist" ) ),
                                _( "Add current selected tracks to playlist after the current artist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );

        Menu->Append( wxID_ANY, _( "Enqueue After" ), EnqueueMenu );

        Menu->AppendSeparator();

        if( m_MediaViewer )
        {
            MenuItem = new wxMenuItem( Menu, ID_ALBUM_SELECTNAME, _( "Search Album" ), _( "Search the album in the library" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_search ) );
            Menu->Append( MenuItem );
            Menu->AppendSeparator();
        }
    }

    if( !GetSearchText().IsEmpty() )
    {
        MenuItem = new wxMenuItem( Menu, ID_LASTFM_COPYTOCLIPBOARD, _( "Copy to Clipboard" ), _( "Copy the album info to clipboard" ) );
        //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_copy ) );
        Menu->Append( MenuItem );
        Menu->AppendSeparator();
    }

    if( !m_Info->m_Album->m_Url.IsEmpty() )
    {
        MenuItem = new wxMenuItem( Menu, ID_LASTFM_VISIT_URL, wxT( "Last.fm" ), _( "Visit last.fm page for this item" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_lastfm_as_on ) );
        Menu->Append( MenuItem );
        Menu->AppendSeparator();
    }

    guLastFMInfoCtrl::CreateContextMenu( Menu );
}

// -------------------------------------------------------------------------------- //
void guAlbumInfoCtrl::OnContextMenu( wxContextMenuEvent& event )
{
    // If the item have not been set we do nothing
    if( !m_Info )
        return;

    guLastFMInfoCtrl::OnContextMenu( event );
}

// -------------------------------------------------------------------------------- //
int guAlbumInfoCtrl::GetSelectedTracks( guTrackArray * tracks )
{
    if( m_Info->m_AlbumId != wxNOT_FOUND )
    {
        wxArrayInt Selections;
        Selections.Add( m_Info->m_AlbumId );
        wxMutexLocker Lock( m_DbMutex );
        return m_Db ? m_Db->GetAlbumsSongs( Selections, tracks ) :
                      m_DefaultDb->GetAlbumsSongs( Selections, tracks );
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
void guAlbumInfoCtrl::OnAlbumSelectName( wxCommandEvent &event )
{
    if( m_MediaViewer )
    {
        wxCommandEvent evt( wxEVT_MENU, ID_MAINFRAME_SELECT_ALBUM );
        evt.SetInt( m_Info->m_AlbumId );
        evt.SetExtraLong( guTRACK_TYPE_DB );
        evt.SetClientData( m_MediaViewer );
        wxPostEvent( m_MediaViewer, evt );
    }
}

// -------------------------------------------------------------------------------- //
// guSimilarArtistInfoCtrl
// -------------------------------------------------------------------------------- //
guSimilarArtistInfoCtrl::guSimilarArtistInfoCtrl( wxWindow * parent, guDbLibrary * db,
        guDbCache * dbcache, guPlayerPanel * playerpanel ) :
            guLastFMInfoCtrl( parent, db, dbcache, playerpanel )
{
    m_Info = NULL;

    Bind( wxEVT_MENU, &guSimilarArtistInfoCtrl::OnSelectArtist, this, ID_LASTFM_SELECT_ARTIST );
}

// -------------------------------------------------------------------------------- //
guSimilarArtistInfoCtrl::~guSimilarArtistInfoCtrl()
{
    if( m_Info )
        delete m_Info;

    Unbind( wxEVT_MENU, &guSimilarArtistInfoCtrl::OnSelectArtist, this, ID_LASTFM_SELECT_ARTIST );
}

// -------------------------------------------------------------------------------- //
void guSimilarArtistInfoCtrl::SetInfo( guLastFMSimilarArtistInfo * info )
{
    if( m_Info )
        delete m_Info;
    m_Info = info;

    m_DbMutex.Lock();
    m_Info->m_ArtistId = m_Db ? m_Db->FindArtist( m_Info->m_Artist->m_Name ) :
                                m_DefaultDb->FindArtist( m_Info->m_Artist->m_Name );
    m_DbMutex.Unlock();
    //guLogMessage( wxT("Artist '%s' id: %i"), Info->Artist->Name.c_str(), Info->ArtistId );
    m_Text->SetForegroundColour( m_Info->m_ArtistId == wxNOT_FOUND ?
                                       m_NotFoundColor : m_NormalColor );
    SetBitmap( m_Info->m_Image );
    double Match;

    if( !m_Info->m_Artist->m_Match.ToDouble( &Match ) )
    {
        m_Info->m_Artist->m_Match.Replace( wxT( "." ), wxT( "," ) );
        m_Info->m_Artist->m_Match.ToDouble( &Match );
        //guLogError( wxT( "Error converting %s to float" ), m_Info->m_Artist->m_Match.c_str() );
    }

    SetLabel( wxString::Format( wxT( "%s\n%i%%" ), m_Info->m_Artist->m_Name.c_str(), int( Match * 100 ) ) );
}

// -------------------------------------------------------------------------------- //
void guSimilarArtistInfoCtrl::SetMediaViewer( guMediaViewer * mediaviewer )
{
    wxMutexLocker Lock( m_DbMutex );
    m_MediaViewer = mediaviewer;
    m_Db = mediaviewer ? mediaviewer->GetDb() : NULL;

    if( m_Info )
    {
        m_Info->m_ArtistId = wxNOT_FOUND;
        m_Text->SetForegroundColour( m_NotFoundColor );
    }
}

// -------------------------------------------------------------------------------- //
void guSimilarArtistInfoCtrl::Clear( guMediaViewer * mediaviewer )
{
    if( m_Info )
        delete m_Info;
    m_Info = NULL;

    guLastFMInfoCtrl::Clear( mediaviewer );
}

// -------------------------------------------------------------------------------- //
void guSimilarArtistInfoCtrl::OnSelectArtist( wxCommandEvent &event )
{
    guLastFMPanel * LastFMPanel = ( guLastFMPanel * ) GetParent();
    LastFMPanel->SetUpdateEnable( false );

    guTrackChangeInfo TrackChangeInfo( GetSearchText(), wxEmptyString, LastFMPanel->GetMediaViewer() );

    LastFMPanel->AppendTrackChangeInfo( &TrackChangeInfo );
    LastFMPanel->ShowCurrentTrack();
}

// -------------------------------------------------------------------------------- //
wxString guSimilarArtistInfoCtrl::GetSearchText( void )
{
    return m_Info->m_Artist->m_Name;
}

// -------------------------------------------------------------------------------- //
wxString guSimilarArtistInfoCtrl::GetItemUrl( void )
{
    return m_Info->m_Artist->m_Url;
}

// -------------------------------------------------------------------------------- //
void guSimilarArtistInfoCtrl::CreateContextMenu( wxMenu * Menu )
{
    wxMenuItem * MenuItem;

    if( m_Info->m_ArtistId != wxNOT_FOUND )
    {
        MenuItem = new wxMenuItem( Menu, ID_LASTFM_PLAY, _( "Play" ), _( "Play the artist tracks" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_LASTFM_ENQUEUE_AFTER_ALL, _( "Enqueue" ), _( "Enqueue the artist tracks to the playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        Menu->Append( MenuItem );

        wxMenu * EnqueueMenu = new wxMenu();

        MenuItem = new wxMenuItem( EnqueueMenu, ID_LASTFM_ENQUEUE_AFTER_TRACK,
                                wxString( _( "Current Track" ) ),
                                _( "Add current selected tracks to playlist after the current track" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );

        MenuItem = new wxMenuItem( EnqueueMenu, ID_LASTFM_ENQUEUE_AFTER_ALBUM,
                                wxString( _( "Current Album" ) ),
                                _( "Add current selected tracks to playlist after the current album" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );

        MenuItem = new wxMenuItem( EnqueueMenu, ID_LASTFM_ENQUEUE_AFTER_ARTIST,
                                wxString( _( "Current Artist" ) ),
                                _( "Add current selected tracks to playlist after the current artist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );

        Menu->Append( wxID_ANY, _( "Enqueue After" ), EnqueueMenu );

        Menu->AppendSeparator();

        if( m_MediaViewer )
        {
            MenuItem = new wxMenuItem( Menu, ID_ARTIST_SELECTNAME, _( "Search Artist" ), _( "Search the artist in the library" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_search ) );
            Menu->Append( MenuItem );
            Menu->AppendSeparator();
        }
    }

    if( !GetSearchText().IsEmpty() )
    {
        MenuItem = new wxMenuItem( Menu, ID_LASTFM_SELECT_ARTIST, _( "Show Artist Info" ), _( "Update the information with the current selected artist" ) );
        Menu->Append( MenuItem );
        MenuItem = new wxMenuItem( Menu, ID_LASTFM_COPYTOCLIPBOARD, _( "Copy to Clipboard" ), _( "Copy the artist info to clipboard" ) );
        //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_copy ) );
        Menu->Append( MenuItem );
        Menu->AppendSeparator();
    }

    if( !m_Info->m_Artist->m_Url.IsEmpty() )
    {
        MenuItem = new wxMenuItem( Menu, ID_LASTFM_VISIT_URL, wxT( "Last.fm" ), _( "Visit last.fm page for this item" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_lastfm_as_on ) );
        Menu->Append( MenuItem );
        Menu->AppendSeparator();
    }
    guLastFMInfoCtrl::CreateContextMenu( Menu );
}

// -------------------------------------------------------------------------------- //
void guSimilarArtistInfoCtrl::OnContextMenu( wxContextMenuEvent& event )
{
    // If the item have not been set we do nothing
    if( !m_Info )
        return;

    guLastFMInfoCtrl::OnContextMenu( event );
}

// -------------------------------------------------------------------------------- //
int guSimilarArtistInfoCtrl::GetSelectedTracks( guTrackArray * tracks )
{
    if( m_Info->m_ArtistId != wxNOT_FOUND )
    {
        wxArrayInt Selections;
        Selections.Add( m_Info->m_ArtistId );
        wxMutexLocker Lock( m_DbMutex );
        return m_Db ? m_Db->GetArtistsSongs( Selections, tracks ) :
                      m_DefaultDb->GetArtistsSongs( Selections, tracks );
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
void guSimilarArtistInfoCtrl::OnArtistSelectName( wxCommandEvent &event )
{
    if( m_MediaViewer )
    {
        wxCommandEvent evt( wxEVT_MENU, ID_MAINFRAME_SELECT_ARTIST );
        evt.SetInt( m_Info->m_ArtistId );
        evt.SetExtraLong( guTRACK_TYPE_DB );
        evt.SetClientData( m_MediaViewer );
        wxPostEvent( m_MediaViewer, evt );
    }
}

// -------------------------------------------------------------------------------- //
// guTrackInfoCtrl
// -------------------------------------------------------------------------------- //
guTrackInfoCtrl::guTrackInfoCtrl( wxWindow * parent, guDbLibrary * db, guDbCache * dbcache, guPlayerPanel * playerpanel ) :
                 guLastFMInfoCtrl( parent, db, dbcache, playerpanel )
{
    m_Info = NULL;

    Bind( wxEVT_MENU, &guTrackInfoCtrl::OnSelectArtist, this, ID_LASTFM_SELECT_ARTIST );
}

// -------------------------------------------------------------------------------- //
guTrackInfoCtrl::~guTrackInfoCtrl()
{
    if( m_Info )
        delete m_Info;

    Unbind( wxEVT_MENU, &guTrackInfoCtrl::OnSelectArtist, this, ID_LASTFM_SELECT_ARTIST );
}

// -------------------------------------------------------------------------------- //
void guTrackInfoCtrl::SetInfo( guLastFMTrackInfo * info )
{
    if( m_Info )
        delete m_Info;
    m_Info = info;

    m_DbMutex.Lock();
    guDbLibrary * Db = m_Db ? m_Db : m_DefaultDb;

    m_Info->m_TrackId = Db->FindTrack( m_Info->m_Track->m_ArtistName, m_Info->m_Track->m_TrackName );
    m_Info->m_ArtistId = Db->FindArtist( m_Info->m_Track->m_ArtistName );
    m_DbMutex.Unlock();
    m_Text->SetForegroundColour( m_Info->m_TrackId == wxNOT_FOUND ?
                                       m_NotFoundColor : m_NormalColor );

    SetBitmap( m_Info->m_Image );

    double Match;
    if( !m_Info->m_Track->m_Match.ToDouble( &Match ) )
    {
        m_Info->m_Track->m_Match.Replace( wxT( "." ), wxT( "," ) );
        m_Info->m_Track->m_Match.ToDouble( &Match );
        //guLogError( wxT( "Error converting %s to float" ), m_Info->m_Track->m_Match.c_str() );
    }

    SetLabel( wxString::Format( wxT( "%s\n%s\n%i%%" ),
        m_Info->m_Track->m_TrackName.c_str(),
        m_Info->m_Track->m_ArtistName.c_str(),
        int( Match * 100 ) ) );
}

// -------------------------------------------------------------------------------- //
void guTrackInfoCtrl::SetMediaViewer( guMediaViewer * mediaviewer )
{
    wxMutexLocker Lock( m_DbMutex );
    m_MediaViewer = mediaviewer;
    m_Db = mediaviewer ? mediaviewer->GetDb() : NULL;

    if( m_Info )
    {
        m_Info->m_TrackId = wxNOT_FOUND;
        m_Info->m_ArtistId = wxNOT_FOUND;
        m_Text->SetForegroundColour( m_NotFoundColor );
    }
}

// -------------------------------------------------------------------------------- //
void guTrackInfoCtrl::Clear( guMediaViewer * mediaviewer )
{
    if( m_Info )
        delete m_Info;
    m_Info = NULL;

    guLastFMInfoCtrl::Clear( mediaviewer );
}

// -------------------------------------------------------------------------------- //
void guTrackInfoCtrl::OnSelectArtist( wxCommandEvent &event )
{
    guLastFMPanel * LastFMPanel = ( guLastFMPanel * ) GetParent();
    LastFMPanel->SetUpdateEnable( false );

    guTrackChangeInfo TrackChangeInfo( m_Info->m_Track->m_ArtistName, wxEmptyString, LastFMPanel->GetMediaViewer() );

    LastFMPanel->AppendTrackChangeInfo( &TrackChangeInfo );
    LastFMPanel->ShowCurrentTrack();
}

// -------------------------------------------------------------------------------- //
wxString guTrackInfoCtrl::GetSearchText( void )
{
    return wxString::Format( wxT( "%s %s" ), m_Info->m_Track->m_ArtistName.c_str(), m_Info->m_Track->m_TrackName.c_str() );
}

// -------------------------------------------------------------------------------- //
wxString guTrackInfoCtrl::GetItemUrl( void )
{
    return m_Info->m_Track->m_Url;
}

// -------------------------------------------------------------------------------- //
void guTrackInfoCtrl::CreateContextMenu( wxMenu * Menu )
{
    wxMenuItem * MenuItem;

    if( m_Info->m_TrackId != wxNOT_FOUND )
    {
        MenuItem = new wxMenuItem( Menu, ID_LASTFM_PLAY, _( "Play" ), _( "Play the artist tracks" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_LASTFM_ENQUEUE_AFTER_ALL, _( "Enqueue" ), _( "Enqueue the artist tracks to the playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        Menu->Append( MenuItem );

        wxMenu * EnqueueMenu = new wxMenu();

        MenuItem = new wxMenuItem( EnqueueMenu, ID_LASTFM_ENQUEUE_AFTER_TRACK,
                                wxString( _( "Current Track" ) ),
                                _( "Add current selected tracks to playlist after the current track" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );

        MenuItem = new wxMenuItem( EnqueueMenu, ID_LASTFM_ENQUEUE_AFTER_ALBUM,
                                wxString( _( "Current Album" ) ),
                                _( "Add current selected tracks to playlist after the current album" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );

        MenuItem = new wxMenuItem( EnqueueMenu, ID_LASTFM_ENQUEUE_AFTER_ARTIST,
                                wxString( _( "Current Artist" ) ),
                                _( "Add current selected tracks to playlist after the current artist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );

        Menu->Append( wxID_ANY, _( "Enqueue After" ), EnqueueMenu );

        Menu->AppendSeparator();

        if( m_MediaViewer )
        {
            MenuItem = new wxMenuItem( Menu, ID_TRACKS_SELECTNAME, _( "Search Track" ), _( "Search the track in the library" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_search ) );
            Menu->Append( MenuItem );

            MenuItem = new wxMenuItem( Menu, ID_ARTIST_SELECTNAME, _( "Search Artist" ), _( "Search the artist in the library" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_search ) );
            Menu->Append( MenuItem );
            Menu->AppendSeparator();
        }
    }

    if( !GetSearchText().IsEmpty() )
    {
        MenuItem = new wxMenuItem( Menu, ID_LASTFM_SELECT_ARTIST, _( "Show Artist Info" ), _( "Update the information with the current selected artist" ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_LASTFM_COPYTOCLIPBOARD, _( "Copy to Clipboard" ), _( "Copy the track info to clipboard" ) );
        //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_copy ) );
        Menu->Append( MenuItem );
        Menu->AppendSeparator();
    }

    if( !m_Info->m_Track->m_Url.IsEmpty() )
    {
        MenuItem = new wxMenuItem( Menu, ID_LASTFM_VISIT_URL, wxT( "Last.fm" ), _( "Visit last.fm page for this item" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_lastfm_as_on ) );
        Menu->Append( MenuItem );
        Menu->AppendSeparator();
    }

    guLastFMInfoCtrl::CreateContextMenu( Menu );
}

// -------------------------------------------------------------------------------- //
void guTrackInfoCtrl::OnContextMenu( wxContextMenuEvent& event )
{
    // If the item have not been set we do nothing
    if( !m_Info )
        return;

    guLastFMInfoCtrl::OnContextMenu( event );
}

// -------------------------------------------------------------------------------- //
int guTrackInfoCtrl::GetSelectedTracks( guTrackArray * tracks )
{
    if( m_Info->m_TrackId != wxNOT_FOUND )
    {
        wxArrayInt Selections;
        Selections.Add( m_Info->m_TrackId );
        wxMutexLocker Lock( m_DbMutex );
        return m_Db ? m_Db->GetSongs( Selections, tracks ) :
                      m_DefaultDb->GetSongs( Selections, tracks );
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
void guTrackInfoCtrl::OnSongSelectName( wxCommandEvent &event )
{
    if( m_MediaViewer )
    {
        wxCommandEvent evt( wxEVT_MENU, ID_MAINFRAME_SELECT_TRACK );
        evt.SetInt( m_Info->m_TrackId );
        evt.SetExtraLong( guTRACK_TYPE_DB );
        evt.SetClientData( m_MediaViewer );
        wxPostEvent( m_MediaViewer, evt );
    }
}

// -------------------------------------------------------------------------------- //
void guTrackInfoCtrl::OnArtistSelectName( wxCommandEvent &event )
{
    if( m_MediaViewer )
    {
        wxCommandEvent evt( wxEVT_MENU, ID_MAINFRAME_SELECT_ARTIST );
        evt.SetInt( m_Info->m_ArtistId );
        evt.SetExtraLong( guTRACK_TYPE_DB );
        evt.SetClientData( m_MediaViewer );
        wxPostEvent( m_MediaViewer, evt );
    }
}

// -------------------------------------------------------------------------------- //
// guTopTrackInfoCtrl
// -------------------------------------------------------------------------------- //
guTopTrackInfoCtrl::guTopTrackInfoCtrl( wxWindow * parent, guDbLibrary * db, guDbCache * dbcache, guPlayerPanel * playerpanel ) :
                 guLastFMInfoCtrl( parent, db, dbcache, playerpanel )
{
    m_Info = NULL;

    Bind( wxEVT_MENU, &guTopTrackInfoCtrl::OnSelectArtist, this, ID_LASTFM_SELECT_ARTIST );
}

// -------------------------------------------------------------------------------- //
guTopTrackInfoCtrl::~guTopTrackInfoCtrl()
{
    if( m_Info )
        delete m_Info;

    Unbind( wxEVT_MENU, &guTopTrackInfoCtrl::OnSelectArtist, this, ID_LASTFM_SELECT_ARTIST );
}

// -------------------------------------------------------------------------------- //
void guTopTrackInfoCtrl::SetInfo( guLastFMTopTrackInfo * info )
{
    if( m_Info )
        delete m_Info;
    m_Info = info;

    m_DbMutex.Lock();
    guDbLibrary * Db = m_Db ? m_Db : m_DefaultDb;

    m_Info->m_TrackId = Db->FindTrack( m_Info->m_TopTrack->m_ArtistName, m_Info->m_TopTrack->m_TrackName );
    m_Info->m_ArtistId = Db->FindArtist( m_Info->m_TopTrack->m_ArtistName );
    m_DbMutex.Unlock();
    m_Text->SetForegroundColour( m_Info->m_TrackId == wxNOT_FOUND ?
                                       m_NotFoundColor : m_NormalColor );

    SetBitmap( m_Info->m_Image );

    SetLabel( wxString::Format( _( "%s\n%i plays by %u users" ),
        m_Info->m_TopTrack->m_TrackName.c_str(),
        m_Info->m_TopTrack->m_PlayCount,
        m_Info->m_TopTrack->m_Listeners ) );
}

// -------------------------------------------------------------------------------- //
void guTopTrackInfoCtrl::SetMediaViewer( guMediaViewer * mediaviewer )
{
    wxMutexLocker Lock( m_DbMutex );
    m_MediaViewer = mediaviewer;
    m_Db = mediaviewer ? mediaviewer->GetDb() : NULL;

    if( m_Info )
    {
        m_Info->m_TrackId = wxNOT_FOUND;
        m_Info->m_ArtistId = wxNOT_FOUND;
        m_Text->SetForegroundColour( m_NotFoundColor );
    }
}

// -------------------------------------------------------------------------------- //
void guTopTrackInfoCtrl::Clear( guMediaViewer * mediaviewer )
{
    if( m_Info )
        delete m_Info;
    m_Info = NULL;

    guLastFMInfoCtrl::Clear( mediaviewer );
}

// -------------------------------------------------------------------------------- //
void guTopTrackInfoCtrl::OnSelectArtist( wxCommandEvent &event )
{
    guLastFMPanel * LastFMPanel = ( guLastFMPanel * ) GetParent();
    LastFMPanel->SetUpdateEnable( false );

    guTrackChangeInfo TrackChangeInfo( m_Info->m_TopTrack->m_ArtistName, wxEmptyString, LastFMPanel->GetMediaViewer() );

    LastFMPanel->AppendTrackChangeInfo( &TrackChangeInfo );
    LastFMPanel->ShowCurrentTrack();
}

// -------------------------------------------------------------------------------- //
wxString guTopTrackInfoCtrl::GetSearchText( void )
{
    return wxString::Format( wxT( "%s %s" ), m_Info->m_TopTrack->m_ArtistName.c_str(), m_Info->m_TopTrack->m_TrackName.c_str() );
}

// -------------------------------------------------------------------------------- //
wxString guTopTrackInfoCtrl::GetItemUrl( void )
{
    return m_Info->m_TopTrack->m_Url;
}

// -------------------------------------------------------------------------------- //
void guTopTrackInfoCtrl::CreateContextMenu( wxMenu * Menu )
{
    wxMenuItem * MenuItem;

    if( m_Info->m_TrackId != wxNOT_FOUND )
    {
        MenuItem = new wxMenuItem( Menu, ID_LASTFM_PLAY, _( "Play" ), _( "Play the artist tracks" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_LASTFM_ENQUEUE_AFTER_ALL, _( "Enqueue" ), _( "Enqueue the artist tracks to the playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        Menu->Append( MenuItem );

        wxMenu * EnqueueMenu = new wxMenu();

        MenuItem = new wxMenuItem( EnqueueMenu, ID_LASTFM_ENQUEUE_AFTER_TRACK,
                                wxString( _( "Current Track" ) ),
                                _( "Add current selected tracks to playlist after the current track" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );

        MenuItem = new wxMenuItem( EnqueueMenu, ID_LASTFM_ENQUEUE_AFTER_ALBUM,
                                wxString( _( "Current Album" ) ),
                                _( "Add current selected tracks to playlist after the current album" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );

        MenuItem = new wxMenuItem( EnqueueMenu, ID_LASTFM_ENQUEUE_AFTER_ARTIST,
                                wxString( _( "Current Artist" ) ),
                                _( "Add current selected tracks to playlist after the current artist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );

        Menu->Append( wxID_ANY, _( "Enqueue After" ), EnqueueMenu );

        Menu->AppendSeparator();

        if( m_MediaViewer )
        {
            MenuItem = new wxMenuItem( Menu, ID_TRACKS_SELECTNAME, _( "Search Track" ), _( "Search the track in the library" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_search ) );
            Menu->Append( MenuItem );

            MenuItem = new wxMenuItem( Menu, ID_ARTIST_SELECTNAME, _( "Search Artist" ), _( "Search the artist in the library" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_search ) );
            Menu->Append( MenuItem );
            Menu->AppendSeparator();
        }
    }

    if( !GetSearchText().IsEmpty() )
    {
        MenuItem = new wxMenuItem( Menu, ID_LASTFM_COPYTOCLIPBOARD, _( "Copy to Clipboard" ), _( "Copy the track info to clipboard" ) );
        //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_copy ) );
        Menu->Append( MenuItem );
        Menu->AppendSeparator();
    }

    if( !m_Info->m_TopTrack->m_Url.IsEmpty() )
    {
        MenuItem = new wxMenuItem( Menu, ID_LASTFM_VISIT_URL, wxT( "Last.fm" ), _( "Visit last.fm page for this item" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_lastfm_as_on ) );
        Menu->Append( MenuItem );
        Menu->AppendSeparator();
    }

    guLastFMInfoCtrl::CreateContextMenu( Menu );
}

// -------------------------------------------------------------------------------- //
void guTopTrackInfoCtrl::OnContextMenu( wxContextMenuEvent& event )
{
    // If the item have not been set we do nothing
    if( !m_Info )
        return;

    guLastFMInfoCtrl::OnContextMenu( event );
}

// -------------------------------------------------------------------------------- //
int guTopTrackInfoCtrl::GetSelectedTracks( guTrackArray * tracks )
{
    if( m_Info->m_TrackId != wxNOT_FOUND )
    {
        wxArrayInt Selections;
        Selections.Add( m_Info->m_TrackId );
        wxMutexLocker Lock( m_DbMutex );
        return m_Db ? m_Db->GetSongs( Selections, tracks ) :
                      m_DefaultDb->GetSongs( Selections, tracks );
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
void guTopTrackInfoCtrl::OnSongSelectName( wxCommandEvent &event )
{
    if( m_MediaViewer )
    {
        wxCommandEvent evt( wxEVT_MENU, ID_MAINFRAME_SELECT_TRACK );
        evt.SetInt( m_Info->m_TrackId );
        evt.SetExtraLong( guTRACK_TYPE_DB );
        evt.SetClientData( m_MediaViewer );
        wxPostEvent( m_MediaViewer, evt );
    }
}

// -------------------------------------------------------------------------------- //
void guTopTrackInfoCtrl::OnArtistSelectName( wxCommandEvent &event )
{
    if( m_MediaViewer )
    {
        wxCommandEvent evt( wxEVT_MENU, ID_MAINFRAME_SELECT_ARTIST );
        evt.SetInt( m_Info->m_ArtistId );
        evt.SetExtraLong( guTRACK_TYPE_DB );
        evt.SetClientData( m_MediaViewer );
        wxPostEvent( m_MediaViewer, evt );
    }
}

// -------------------------------------------------------------------------------- //
// guEventInfoCtrl
// -------------------------------------------------------------------------------- //
guEventInfoCtrl::guEventInfoCtrl( wxWindow * parent, guDbLibrary * db, guDbCache * dbcache, guPlayerPanel * playerpanel ) :
                 guLastFMInfoCtrl( parent, db, dbcache, playerpanel )
{
    m_Info = NULL;
}

// -------------------------------------------------------------------------------- //
guEventInfoCtrl::~guEventInfoCtrl()
{
    if( m_Info )
        delete m_Info;
}

// -------------------------------------------------------------------------------- //
void guEventInfoCtrl::SetInfo( guLastFMEventInfo * info )
{
    if( m_Info )
        delete m_Info;
    m_Info = info;

    SetBitmap( m_Info->m_Image );
    SetLabel( wxString::Format( wxT( "%s\n%s (%s)\n%s %s" ),
        m_Info->m_Event->m_Title.c_str(),
        m_Info->m_Event->m_LocationCity.c_str(),
        m_Info->m_Event->m_LocationCountry.c_str(),
        m_Info->m_Event->m_Date.c_str(),
        m_Info->m_Event->m_Time.c_str() ) );
}

// -------------------------------------------------------------------------------- //
void guEventInfoCtrl::Clear( guMediaViewer * mediaviewer )
{
    if( m_Info )
        delete m_Info;
    m_Info = NULL;

    guLastFMInfoCtrl::Clear( mediaviewer );
}

// -------------------------------------------------------------------------------- //
wxString guEventInfoCtrl::GetSearchText( void )
{
    return m_Info->m_Event->m_Title + wxT( " " ) +
           m_Info->m_Event->m_LocationName + wxT( " " ) +
           m_Info->m_Event->m_LocationCity + wxT( " " ) +
           m_Info->m_Event->m_LocationCountry;
}

// -------------------------------------------------------------------------------- //
wxString guEventInfoCtrl::GetItemUrl( void )
{
    return m_Info->m_Event->m_Url;
}

// -------------------------------------------------------------------------------- //
void guEventInfoCtrl::CreateContextMenu( wxMenu * Menu )
{
    wxMenuItem * MenuItem;

    if( !GetSearchText().IsEmpty() )
    {
        MenuItem = new wxMenuItem( Menu, ID_LASTFM_COPYTOCLIPBOARD, _( "Copy to Clipboard" ), _( "Copy the event info to clipboard" ) );
        //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_copy ) );
        Menu->Append( MenuItem );
        Menu->AppendSeparator();
    }

    if( !m_Info->m_Event->m_Url.IsEmpty() )
    {
        MenuItem = new wxMenuItem( Menu, ID_LASTFM_VISIT_URL, wxT( "Last.fm" ), _( "Visit last.fm page for this item" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_lastfm_as_on ) );
        Menu->Append( MenuItem );
        Menu->AppendSeparator();
    }

    guLastFMInfoCtrl::CreateContextMenu( Menu );
}

// -------------------------------------------------------------------------------- //
void guEventInfoCtrl::OnContextMenu( wxContextMenuEvent& event )
{
    // If the item have not been set we do nothing
    if( !m_Info )
        return;

    guLastFMInfoCtrl::OnContextMenu( event );
}

// -------------------------------------------------------------------------------- //
int guEventInfoCtrl::GetSelectedTracks( guTrackArray * tracks )
{
    return 0;
}

// -------------------------------------------------------------------------------- //
// guLastFMPanel
// -------------------------------------------------------------------------------- //
guLastFMPanel::guLastFMPanel( wxWindow * Parent, guDbLibrary * db,
                                guDbCache * dbcache, guPlayerPanel * playerpanel ) :
    wxScrolledWindow( Parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL|wxTAB_TRAVERSAL )
{
    m_DefaultDb = db;
    m_Db = NULL;
    m_DbCache = dbcache;
    m_PlayerPanel = playerpanel;
    m_MediaViewer = NULL;

    m_CurrentTrackInfo = wxNOT_FOUND;

    m_ShowArtistDetails = true;
    m_ShowAlbums = true;
    m_ShowTopTracks = true;
    m_ShowSimArtists = true;
    m_ShowSimTracks = true;
    m_ShowEvents = true;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        m_ShowArtistDetails = Config->ReadBool( CONFIG_KEY_LASTFM_SHOW_ARTIST_INFO, true, CONFIG_PATH_LASTFM  );
        m_ShowAlbums = Config->ReadBool( CONFIG_KEY_LASTFM_SHOW_ALBUMS, true, CONFIG_PATH_LASTFM  );
        m_ShowTopTracks = Config->ReadBool( CONFIG_KEY_LASTFM_SHOW_TOP_TRACKS, true, CONFIG_PATH_LASTFM  );
        m_ShowSimArtists = Config->ReadBool( CONFIG_KEY_LASTFM_SHOW_ARTISTS, true, CONFIG_PATH_LASTFM  );
        m_ShowSimTracks = Config->ReadBool( CONFIG_KEY_LASTFM_SHOW_TRACKS, true, CONFIG_PATH_LASTFM  );
        m_ShowEvents = Config->ReadBool( CONFIG_KEY_LASTFM_SHOW_EVENTS, true, CONFIG_PATH_LASTFM  );
    }
    m_UpdateEnabled = Config->ReadBool( CONFIG_KEY_LASTFM_FOLLOW_PLAYER, true, CONFIG_PATH_LASTFM );

    wxFont CurrentFont = wxSystemSettings::GetFont( wxSYS_SYSTEM_FONT );

	m_MainSizer = new wxBoxSizer( wxVERTICAL );

    // Manual Artist Editor
	wxBoxSizer * EditorSizer;
	EditorSizer = new wxBoxSizer( wxHORIZONTAL );

	m_UpdateCheckBox = new wxCheckBox( this, wxID_ANY, _( "Follow player" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_UpdateCheckBox->SetValue( m_UpdateEnabled );
	EditorSizer->Add( m_UpdateCheckBox, 0, wxEXPAND|wxTOP|wxLEFT, 5 );

	m_PrevButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_left ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_PrevButton->Enable( false );
	EditorSizer->Add( m_PrevButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxLEFT, 5 );

	m_NextButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_right ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_NextButton->Enable( false );
	EditorSizer->Add( m_NextButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP, 5 );

	m_ReloadButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_reload ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_ReloadButton->Enable( false );
	EditorSizer->Add( m_ReloadButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

	wxStaticText * ArtistStaticText;
	ArtistStaticText = new wxStaticText( this, wxID_ANY, _( "Artist:" ), wxDefaultPosition, wxDefaultSize, 0 );
	ArtistStaticText->Wrap( -1 );
	EditorSizer->Add( ArtistStaticText, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxLEFT, 5 );

	m_ArtistTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	m_ArtistTextCtrl->Enable( !m_UpdateEnabled );
	EditorSizer->Add( m_ArtistTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT, 5 );

	wxStaticText * TrackStaticText;
	TrackStaticText = new wxStaticText( this, wxID_ANY, _( "Track:" ), wxDefaultPosition, wxDefaultSize, 0 );
	TrackStaticText->Wrap( -1 );

	EditorSizer->Add( TrackStaticText, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxLEFT, 5 );

	m_TrackTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	m_TrackTextCtrl->Enable( false );
	EditorSizer->Add( m_TrackTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT, 5 );

	m_MainSizer->Add( EditorSizer, 0, wxEXPAND, 5 );

	wxStaticLine * TopStaticLine;
	TopStaticLine = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	m_MainSizer->Add( TopStaticLine, 0, wxEXPAND | wxALL, 5 );


    // Artist Info
	wxBoxSizer * ArInfoTitleSizer;
	ArInfoTitleSizer = new wxBoxSizer( wxHORIZONTAL );

	m_ArtistDetailsStaticText = new wxStaticText( this, wxID_ANY, _("Artist Info"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ArtistDetailsStaticText->Wrap( -1 );
    CurrentFont.SetPointSize( GULASTFM_TITLE_FONT_SIZE );
    CurrentFont.SetWeight( wxFONTWEIGHT_BOLD );
    //wxFont( GULASTFM_TITLE_FONT_SIZE, 70, 90, 92, false, wxEmptyString ) );
	m_ArtistDetailsStaticText->SetFont( CurrentFont );

	ArInfoTitleSizer->Add( m_ArtistDetailsStaticText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	wxStaticLine * ArInfoStaticLine = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	ArInfoTitleSizer->Add( ArInfoStaticLine, 1, wxEXPAND | wxALL, 5 );

	m_MainSizer->Add( ArInfoTitleSizer, 0, wxEXPAND, 5 );

	m_ArtistInfoMainSizer = new wxBoxSizer( wxVERTICAL );

	m_ArtistInfoCtrl = new guArtistInfoCtrl( this, m_DefaultDb, m_DbCache, m_PlayerPanel );

	m_ArtistInfoMainSizer->Add( m_ArtistInfoCtrl, 1, wxEXPAND, 5 );

	m_MainSizer->Add( m_ArtistInfoMainSizer, 0, wxEXPAND, 5 );

    if( m_ShowArtistDetails )
        m_MainSizer->Show( m_ArtistInfoMainSizer );
    else
        m_MainSizer->Hide( m_ArtistInfoMainSizer );

    m_MainSizer->FitInside( this );

    //
    // Top Albmus
    //
	wxBoxSizer* AlTitleSizer;
	AlTitleSizer = new wxBoxSizer( wxHORIZONTAL );

	m_AlbumsStaticText = new wxStaticText( this, wxID_ANY, _("Top Albums"), wxDefaultPosition, wxDefaultSize, 0 );
	m_AlbumsStaticText->Wrap( -1 );
	//AlbumsStaticText->SetCursor( wxCURSOR_HAND );
	m_AlbumsStaticText->SetFont( CurrentFont ); //wxFont( GULASTFM_TITLE_FONT_SIZE, 70, 90, 92, false, wxEmptyString ) );

	AlTitleSizer->Add( m_AlbumsStaticText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	wxStaticLine * AlStaticLine = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	AlTitleSizer->Add( AlStaticLine, 1, wxEXPAND | wxALL, 5 );

    m_AlbumsRangeLabel = new wxStaticText( this, wxID_ANY, wxT( "0 - 0 / 0" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_AlbumsStaticText->Wrap( -1 );
	AlTitleSizer->Add( m_AlbumsRangeLabel, 0, wxLEFT|wxTOP|wxBOTTOM, 5 );

    m_AlbumsPrevBtn = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_left ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_AlbumsPrevBtn->Enable( false );
	AlTitleSizer->Add( m_AlbumsPrevBtn, 0, wxLEFT, 5 );

    m_AlbumsNextBtn = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_right ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_AlbumsNextBtn->Enable( false );
	AlTitleSizer->Add( m_AlbumsNextBtn, 0, wxRIGHT, 5 );

	m_MainSizer->Add( AlTitleSizer, 0, wxEXPAND, 5 );

	m_AlbumsSizer = new wxGridSizer( 4, 3, 0, 0 );
    int index;
    for( index = 0; index < GULASTFMINFO_MAXITEMS; index++ )
    {
        m_AlbumsInfoCtrls.Add( new guAlbumInfoCtrl( this, m_DefaultDb, m_DbCache, m_PlayerPanel ) );
        m_AlbumsSizer->Add( m_AlbumsInfoCtrls[ index ], 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
    }

    m_MainSizer->Add( m_AlbumsSizer, 0, wxEXPAND, 5 );

    SetTopAlbumsVisible();

    //
    // Top Tracks
    //
	wxBoxSizer * TopTracksTitleSizer;
	TopTracksTitleSizer = new wxBoxSizer( wxHORIZONTAL );

	m_TopTracksStaticText = new wxStaticText( this, wxID_ANY, _("Top Tracks"), wxDefaultPosition, wxDefaultSize, 0 );
	m_TopTracksStaticText->Wrap( -1 );
	m_TopTracksStaticText->SetFont( CurrentFont ); //wxFont( GULASTFM_TITLE_FONT_SIZE, 70, 90, 92, false, wxEmptyString ) );

	TopTracksTitleSizer->Add( m_TopTracksStaticText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	wxStaticLine * TopTrStaticLine = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	TopTracksTitleSizer->Add( TopTrStaticLine, 1, wxEXPAND | wxALL, 5 );

    m_TopTracksRangeLabel = new wxStaticText( this, wxID_ANY, wxT( "0 - 0 / 0" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_AlbumsStaticText->Wrap( -1 );
	TopTracksTitleSizer->Add( m_TopTracksRangeLabel, 0, wxLEFT|wxTOP|wxBOTTOM, 5 );

    m_TopTracksPrevBtn = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_left ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_TopTracksPrevBtn->Enable( false );
	TopTracksTitleSizer->Add( m_TopTracksPrevBtn, 0, wxLEFT, 5 );

    m_TopTracksNextBtn = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_right ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_TopTracksNextBtn->Enable( false );
	TopTracksTitleSizer->Add( m_TopTracksNextBtn, 0, wxRIGHT, 5 );

	m_MainSizer->Add( TopTracksTitleSizer, 0, wxEXPAND, 5 );

	m_TopTracksSizer = new wxGridSizer( 4, 3, 0, 0 );

    for( index = 0; index < GULASTFMINFO_MAXITEMS; index++ )
    {
        m_TopTrackInfoCtrls.Add( new guTopTrackInfoCtrl( this, m_DefaultDb, m_DbCache, m_PlayerPanel ) );
        m_TopTracksSizer->Add( m_TopTrackInfoCtrls[ index ], 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
    }

	m_MainSizer->Add( m_TopTracksSizer, 0, wxEXPAND, 5 );
	//MainSizer->Add( new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL ), 0, wxEXPAND, 5 );
    SetTopTracksVisible();

    //
    // Similar Artists
    //
	wxBoxSizer* SimArTitleSizer;
	SimArTitleSizer = new wxBoxSizer( wxHORIZONTAL );

	m_SimArtistsStaticText = new wxStaticText( this, wxID_ANY, _("Similar Artists"), wxDefaultPosition, wxDefaultSize, 0 );
	m_SimArtistsStaticText->Wrap( -1 );
	m_SimArtistsStaticText->SetFont( CurrentFont ); //wxFont( GULASTFM_TITLE_FONT_SIZE, 70, 90, 92, false, wxEmptyString ) );

	SimArTitleSizer->Add( m_SimArtistsStaticText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	wxStaticLine * SimArStaticLine = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	SimArTitleSizer->Add( SimArStaticLine, 1, wxEXPAND | wxALL, 5 );

    m_SimArtistsRangeLabel = new wxStaticText( this, wxID_ANY, wxT( "0 - 0 / 0" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_AlbumsStaticText->Wrap( -1 );
	SimArTitleSizer->Add( m_SimArtistsRangeLabel, 0, wxLEFT|wxTOP|wxBOTTOM, 5 );

    m_SimArtistsPrevBtn = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_left ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_SimArtistsPrevBtn->Enable( false );
	SimArTitleSizer->Add( m_SimArtistsPrevBtn, 0, wxLEFT, 5 );

    m_SimArtistsNextBtn = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_right ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_SimArtistsNextBtn->Enable( false );
	SimArTitleSizer->Add( m_SimArtistsNextBtn, 0, wxRIGHT, 5 );

	m_MainSizer->Add( SimArTitleSizer, 0, wxEXPAND, 5 );

	m_SimArtistsSizer = new wxGridSizer( 4, 3, 0, 0 );

    for( index = 0; index < GULASTFMINFO_MAXITEMS; index++ )
    {
        m_SimArtistsInfoCtrls.Add( new guSimilarArtistInfoCtrl( this, m_DefaultDb, m_DbCache, m_PlayerPanel ) );
        m_SimArtistsSizer->Add( m_SimArtistsInfoCtrls[ index ], 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
    }

	m_MainSizer->Add( m_SimArtistsSizer, 0, wxEXPAND, 5 );

    SetSimArtistsVisible();

    //
    // Similar Tracks
    //
	wxBoxSizer * SimTracksTitleSizer;
	SimTracksTitleSizer = new wxBoxSizer( wxHORIZONTAL );

	m_SimTracksStaticText = new wxStaticText( this, wxID_ANY, _("Similar Tracks"), wxDefaultPosition, wxDefaultSize, 0 );
	m_SimTracksStaticText->Wrap( -1 );
	m_SimTracksStaticText->SetFont( CurrentFont ); //wxFont( GULASTFM_TITLE_FONT_SIZE, 70, 90, 92, false, wxEmptyString ) );

	SimTracksTitleSizer->Add( m_SimTracksStaticText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	wxStaticLine * SimTrStaticLine = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	SimTracksTitleSizer->Add( SimTrStaticLine, 1, wxEXPAND | wxALL, 5 );

    m_SimTracksRangeLabel = new wxStaticText( this, wxID_ANY, wxT( "0 - 0 / 0" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_AlbumsStaticText->Wrap( -1 );
	SimTracksTitleSizer->Add( m_SimTracksRangeLabel, 0, wxLEFT|wxTOP|wxBOTTOM, 5 );

    m_SimTracksPrevBtn = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_left ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_SimTracksPrevBtn->Enable( false );
	SimTracksTitleSizer->Add( m_SimTracksPrevBtn, 0, wxLEFT, 5 );

    m_SimTracksNextBtn = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_right ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_SimTracksNextBtn->Enable( false );
	SimTracksTitleSizer->Add( m_SimTracksNextBtn, 0, wxRIGHT, 5 );

	m_MainSizer->Add( SimTracksTitleSizer, 0, wxEXPAND, 5 );

	m_SimTracksSizer = new wxGridSizer( 4, 3, 0, 0 );

    for( index = 0; index < GULASTFMINFO_MAXITEMS; index++ )
    {
        m_SimTracksInfoCtrls.Add( new guTrackInfoCtrl( this, m_DefaultDb, m_DbCache, m_PlayerPanel ) );
        m_SimTracksSizer->Add( m_SimTracksInfoCtrls[ index ], 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
    }

	m_MainSizer->Add( m_SimTracksSizer, 0, wxEXPAND, 5 );
	//MainSizer->Add( new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL ), 0, wxEXPAND, 5 );
	SetSimTracksVisible();

    //
    // Artist Events
    //
	wxBoxSizer * EventsTitleSizer;
	EventsTitleSizer = new wxBoxSizer( wxHORIZONTAL );

	m_EventsStaticText = new wxStaticText( this, wxID_ANY, _( "Events" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_EventsStaticText->Wrap( -1 );
	m_EventsStaticText->SetFont( CurrentFont ); //wxFont( GULASTFM_TITLE_FONT_SIZE, 70, 90, 92, false, wxEmptyString ) );

	EventsTitleSizer->Add( m_EventsStaticText, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	wxStaticLine * EventsStaticLine = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	EventsTitleSizer->Add( EventsStaticLine, 1, wxEXPAND | wxALL, 5 );

    m_EventsRangeLabel = new wxStaticText( this, wxID_ANY, wxT( "0 - 0 / 0" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_AlbumsStaticText->Wrap( -1 );
	EventsTitleSizer->Add( m_EventsRangeLabel, 0, wxLEFT|wxTOP|wxBOTTOM, 5 );

    m_EventsPrevBtn = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_left ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_EventsPrevBtn->Enable( false );
	EventsTitleSizer->Add( m_EventsPrevBtn, 0, wxLEFT, 5 );

    m_EventsNextBtn = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_right ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_EventsNextBtn->Enable( false );
	EventsTitleSizer->Add( m_EventsNextBtn, 0, wxRIGHT, 5 );

	m_MainSizer->Add( EventsTitleSizer, 0, wxEXPAND, 5 );

	m_EventsSizer = new wxGridSizer( 4, 3, 0, 0 );

    for( index = 0; index < GULASTFMINFO_MAXITEMS; index++ )
    {
        m_EventsInfoCtrls.Add( new guEventInfoCtrl( this, m_DefaultDb, m_DbCache, m_PlayerPanel ) );
        m_EventsSizer->Add( m_EventsInfoCtrls[ index ], 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
    }

	m_MainSizer->Add( m_EventsSizer, 0, wxEXPAND, 5 );
	//MainSizer->Add( new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL ), 0, wxEXPAND, 5 );
    SetEventsVisible();

	this->SetSizer( m_MainSizer );
	this->Layout();

	SetScrollRate( 21, 21 );

    m_ContextMenuObject = NULL;

    m_ArtistsUpdateThread = NULL;
    m_AlbumsUpdateThread = NULL;
    m_TopTracksUpdateThread = NULL;
    m_SimTracksUpdateThread = NULL;
    m_SimArtistsUpdateThread = NULL;
    m_EventsUpdateThread = NULL;

    m_AlbumsCount = 0;
    m_AlbumsPageStart = 0;
    m_TopTracksCount = 0;
    m_TopTracksPageStart = 0;
    m_SimArtistsCount = 0;
    m_SimArtistsPageStart = 0;
    m_SimTracksCount = 0;
    m_SimTracksPageStart = 0;

    SetDropTarget( new guLastFMPanelDropTarget( this ) );

    Bind( wxEVT_MENU, &guLastFMPanel::OnUpdateArtistInfo, this, ID_LASTFM_UPDATE_ARTISTINFO );

    Bind( wxEVT_MENU, &guLastFMPanel::OnUpdateAlbumItem, this, ID_LASTFM_UPDATE_ALBUMINFO );
    Bind( wxEVT_MENU, &guLastFMPanel::OnAlbumsCountUpdated, this, ID_LASTFM_UPDATE_ALBUM_COUNT );
    m_AlbumsPrevBtn->Bind( wxEVT_BUTTON, &guLastFMPanel::OnAlbumsPrevClicked, this );
    m_AlbumsNextBtn->Bind( wxEVT_BUTTON, &guLastFMPanel::OnAlbumsNextClicked, this );

    Bind( wxEVT_MENU, &guLastFMPanel::OnUpdateArtistItem, this, ID_LASTFM_UPDATE_SIMARTIST );
    Bind( wxEVT_MENU, &guLastFMPanel::OnSimArtistsCountUpdated, this, ID_LASTFM_UPDATE_SIMARTIST_COUNT );
    m_SimArtistsPrevBtn->Bind( wxEVT_BUTTON, &guLastFMPanel::OnSimArtistsPrevClicked, this );
    m_SimArtistsNextBtn->Bind( wxEVT_BUTTON, &guLastFMPanel::OnSimArtistsNextClicked, this );

    Bind( wxEVT_MENU, &guLastFMPanel::OnUpdateTrackItem, this, ID_LASTFM_UPDATE_SIMTRACK );
    Bind( wxEVT_MENU, &guLastFMPanel::OnSimTracksCountUpdated, this, ID_LASTFM_UPDATE_SIMTRACK_COUNT );
    m_SimTracksPrevBtn->Bind( wxEVT_BUTTON, &guLastFMPanel::OnSimTracksPrevClicked, this );
    m_SimTracksNextBtn->Bind( wxEVT_BUTTON, &guLastFMPanel::OnSimTracksNextClicked, this );

    Bind( wxEVT_MENU, &guLastFMPanel::OnUpdateTopTrackItem, this, ID_LASTFM_UPDATE_TOPTRACKS );
    Bind( wxEVT_MENU, &guLastFMPanel::OnTopTracksCountUpdated, this, ID_LASTFM_UPDATE_TOPTRACKS_COUNT );
    m_TopTracksPrevBtn->Bind( wxEVT_BUTTON, &guLastFMPanel::OnTopTracksPrevClicked, this );
    m_TopTracksNextBtn->Bind( wxEVT_BUTTON, &guLastFMPanel::OnTopTracksNextClicked, this );

    Bind( wxEVT_MENU, &guLastFMPanel::OnUpdateEventItem, this, ID_LASTFM_UPDATE_EVENTINFO );
    Bind( wxEVT_MENU, &guLastFMPanel::OnEventsCountUpdated, this, ID_LASTFM_UPDATE_EVENTS_COUNT );
    m_EventsPrevBtn->Bind( wxEVT_BUTTON, &guLastFMPanel::OnEventsPrevClicked, this );
    m_EventsNextBtn->Bind( wxEVT_BUTTON, &guLastFMPanel::OnEventsNextClicked, this );

    m_ArtistDetailsStaticText->Bind( wxEVT_LEFT_DCLICK, &guLastFMPanel::OnArInfoTitleDClicked, this );
    m_UpdateCheckBox->Bind( wxEVT_CHECKBOX, &guLastFMPanel::OnUpdateChkBoxClick, this );
    m_PrevButton->Bind( wxEVT_BUTTON, &guLastFMPanel::OnPrevBtnClick, this );
    m_NextButton->Bind( wxEVT_BUTTON, &guLastFMPanel::OnNextBtnClick, this );
    m_ReloadButton->Bind( wxEVT_BUTTON, &guLastFMPanel::OnReloadBtnClick, this );
    m_ArtistTextCtrl->Bind( wxEVT_TEXT, &guLastFMPanel::OnTextUpdated, this );
    m_TrackTextCtrl->Bind( wxEVT_TEXT, &guLastFMPanel::OnTextUpdated, this );
    m_ArtistTextCtrl->Bind( wxEVT_KEY_DOWN, &guLastFMPanel::OnTextCtrlKeyDown, this );
    m_TrackTextCtrl->Bind( wxEVT_KEY_DOWN, &guLastFMPanel::OnTextCtrlKeyDown, this );
    m_ArtistTextCtrl->Bind( wxEVT_TEXT_ENTER, &guLastFMPanel::OnSearchSelected, this );
    m_TrackTextCtrl->Bind( wxEVT_TEXT_ENTER, &guLastFMPanel::OnSearchSelected, this );
    m_AlbumsStaticText->Bind( wxEVT_LEFT_DCLICK, &guLastFMPanel::OnTopAlbumsTitleDClick, this );
    m_TopTracksStaticText->Bind( wxEVT_LEFT_DCLICK, &guLastFMPanel::OnTopTracksTitleDClick, this );
    m_SimArtistsStaticText->Bind( wxEVT_LEFT_DCLICK, &guLastFMPanel::OnSimArTitleDClick, this );
    m_SimTracksStaticText->Bind( wxEVT_LEFT_DCLICK, &guLastFMPanel::OnSimTrTitleDClick, this );
    m_EventsStaticText->Bind( wxEVT_LEFT_DCLICK, &guLastFMPanel::OnEventsTitleDClick, this );

    m_AlbumsStaticText->Bind( wxEVT_CONTEXT_MENU, &guLastFMPanel::OnContextMenu, this );
    m_TopTracksStaticText->Bind( wxEVT_CONTEXT_MENU, &guLastFMPanel::OnContextMenu, this );
    m_SimArtistsStaticText->Bind( wxEVT_CONTEXT_MENU, &guLastFMPanel::OnContextMenu, this );
    m_SimTracksStaticText->Bind( wxEVT_CONTEXT_MENU, &guLastFMPanel::OnContextMenu, this );

    Bind( wxEVT_MENU, &guLastFMPanel::OnPlayClicked, this, ID_LASTFM_PLAY );
    Bind( wxEVT_MENU, &guLastFMPanel::OnEnqueueClicked, this, ID_LASTFM_ENQUEUE_AFTER_ALL, ID_LASTFM_ENQUEUE_AFTER_ARTIST );
    Bind( wxEVT_MENU, &guLastFMPanel::OnSaveClicked, this, ID_LASTFM_SAVETOPLAYLIST );
    Bind( wxEVT_MENU, &guLastFMPanel::OnCopyToClicked, this, ID_COPYTO_BASE, ID_COPYTO_BASE + guCOPYTO_MAXCOUNT );
}

// -------------------------------------------------------------------------------- //
guLastFMPanel::~guLastFMPanel()
{
    m_ArtistsUpdateThreadMutex.Lock();
    if( m_ArtistsUpdateThread )
    {
        m_ArtistsUpdateThread->Pause();
        m_ArtistsUpdateThread->Delete();
        m_ArtistsUpdateThread = NULL;
    }
    m_ArtistsUpdateThreadMutex.Unlock();

    m_AlbumsUpdateThreadMutex.Lock();
    if( m_AlbumsUpdateThread )
    {
        m_AlbumsUpdateThread->Pause();
        m_AlbumsUpdateThread->Delete();
        m_AlbumsUpdateThread = NULL;
    }
    m_AlbumsUpdateThreadMutex.Unlock();

    m_TopTracksUpdateThreadMutex.Lock();
    if( m_TopTracksUpdateThread )
    {
        m_TopTracksUpdateThread->Pause();
        m_TopTracksUpdateThread->Delete();
        m_TopTracksUpdateThread = NULL;
    }
    m_TopTracksUpdateThreadMutex.Unlock();

    m_SimTracksUpdateThreadMutex.Lock();
    if( m_SimTracksUpdateThread )
    {
        m_SimTracksUpdateThread->Pause();
        m_SimTracksUpdateThread->Delete();
        m_SimTracksUpdateThread = NULL;
    }
    m_SimTracksUpdateThreadMutex.Unlock();

    m_SimArtistsUpdateThreadMutex.Lock();
    if( m_SimArtistsUpdateThread )
    {
        m_SimArtistsUpdateThread->Pause();
        m_SimArtistsUpdateThread->Delete();
        m_SimArtistsUpdateThread = NULL;
    }
    m_SimArtistsUpdateThreadMutex.Unlock();

    m_EventsUpdateThreadMutex.Lock();
    if( m_EventsUpdateThread )
    {
        m_EventsUpdateThread->Pause();
        m_EventsUpdateThread->Delete();
        m_EventsUpdateThread = NULL;
    }
    m_EventsUpdateThreadMutex.Unlock();

    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        Config->WriteBool( CONFIG_KEY_LASTFM_SHOW_ARTIST_INFO, m_ShowArtistDetails, CONFIG_PATH_LASTFM );
        Config->WriteBool( CONFIG_KEY_LASTFM_SHOW_ALBUMS, m_ShowAlbums, CONFIG_PATH_LASTFM );
        Config->WriteBool( CONFIG_KEY_LASTFM_SHOW_TOP_TRACKS, m_ShowTopTracks, CONFIG_PATH_LASTFM );
        Config->WriteBool( CONFIG_KEY_LASTFM_SHOW_ARTISTS, m_ShowSimArtists, CONFIG_PATH_LASTFM );
        Config->WriteBool( CONFIG_KEY_LASTFM_SHOW_TRACKS, m_ShowSimTracks, CONFIG_PATH_LASTFM );
        Config->WriteBool( CONFIG_KEY_LASTFM_SHOW_EVENTS, m_ShowEvents, CONFIG_PATH_LASTFM );
        Config->WriteBool( CONFIG_KEY_LASTFM_FOLLOW_PLAYER, m_UpdateCheckBox->GetValue(), CONFIG_PATH_LASTFM );
    }

    Unbind( wxEVT_MENU, &guLastFMPanel::OnUpdateArtistInfo, this, ID_LASTFM_UPDATE_ARTISTINFO );

    Unbind( wxEVT_MENU, &guLastFMPanel::OnUpdateAlbumItem, this, ID_LASTFM_UPDATE_ALBUMINFO );
    Unbind( wxEVT_MENU, &guLastFMPanel::OnAlbumsCountUpdated, this, ID_LASTFM_UPDATE_ALBUM_COUNT );
    m_AlbumsPrevBtn->Unbind( wxEVT_BUTTON, &guLastFMPanel::OnAlbumsPrevClicked, this );
    m_AlbumsNextBtn->Unbind( wxEVT_BUTTON, &guLastFMPanel::OnAlbumsNextClicked, this );

    Unbind( wxEVT_MENU, &guLastFMPanel::OnUpdateArtistItem, this, ID_LASTFM_UPDATE_SIMARTIST );
    Unbind( wxEVT_MENU, &guLastFMPanel::OnSimArtistsCountUpdated, this, ID_LASTFM_UPDATE_SIMARTIST_COUNT );
    m_SimArtistsPrevBtn->Unbind( wxEVT_BUTTON, &guLastFMPanel::OnSimArtistsPrevClicked, this );
    m_SimArtistsNextBtn->Unbind( wxEVT_BUTTON, &guLastFMPanel::OnSimArtistsNextClicked, this );

    Unbind( wxEVT_MENU, &guLastFMPanel::OnUpdateTrackItem, this, ID_LASTFM_UPDATE_SIMTRACK );
    Unbind( wxEVT_MENU, &guLastFMPanel::OnSimTracksCountUpdated, this, ID_LASTFM_UPDATE_SIMTRACK_COUNT );
    m_SimTracksPrevBtn->Unbind( wxEVT_BUTTON, &guLastFMPanel::OnSimTracksPrevClicked, this );
    m_SimTracksNextBtn->Unbind( wxEVT_BUTTON, &guLastFMPanel::OnSimTracksNextClicked, this );

    Unbind( wxEVT_MENU, &guLastFMPanel::OnUpdateTopTrackItem, this, ID_LASTFM_UPDATE_TOPTRACKS );
    Unbind( wxEVT_MENU, &guLastFMPanel::OnTopTracksCountUpdated, this, ID_LASTFM_UPDATE_TOPTRACKS_COUNT );
    m_TopTracksPrevBtn->Unbind( wxEVT_BUTTON, &guLastFMPanel::OnTopTracksPrevClicked, this );
    m_TopTracksNextBtn->Unbind( wxEVT_BUTTON, &guLastFMPanel::OnTopTracksNextClicked, this );

    Unbind( wxEVT_MENU, &guLastFMPanel::OnUpdateEventItem, this, ID_LASTFM_UPDATE_EVENTINFO );
    Unbind( wxEVT_MENU, &guLastFMPanel::OnEventsCountUpdated, this, ID_LASTFM_UPDATE_EVENTS_COUNT );
    m_EventsPrevBtn->Unbind( wxEVT_BUTTON, &guLastFMPanel::OnEventsPrevClicked, this );
    m_EventsNextBtn->Unbind( wxEVT_BUTTON, &guLastFMPanel::OnEventsNextClicked, this );

    m_ArtistDetailsStaticText->Unbind( wxEVT_LEFT_DCLICK, &guLastFMPanel::OnArInfoTitleDClicked, this );
    m_UpdateCheckBox->Unbind( wxEVT_CHECKBOX, &guLastFMPanel::OnUpdateChkBoxClick, this );
    m_PrevButton->Unbind( wxEVT_BUTTON, &guLastFMPanel::OnPrevBtnClick, this );
    m_NextButton->Unbind( wxEVT_BUTTON, &guLastFMPanel::OnNextBtnClick, this );
    m_ReloadButton->Unbind( wxEVT_BUTTON, &guLastFMPanel::OnReloadBtnClick, this );
    m_ArtistTextCtrl->Unbind( wxEVT_TEXT, &guLastFMPanel::OnTextUpdated, this );
    m_TrackTextCtrl->Unbind( wxEVT_TEXT, &guLastFMPanel::OnTextUpdated, this );
    m_ArtistTextCtrl->Unbind( wxEVT_KEY_DOWN, &guLastFMPanel::OnTextCtrlKeyDown, this );
    m_TrackTextCtrl->Unbind( wxEVT_KEY_DOWN, &guLastFMPanel::OnTextCtrlKeyDown, this );
    m_ArtistTextCtrl->Unbind( wxEVT_TEXT_ENTER, &guLastFMPanel::OnSearchSelected, this );
    m_TrackTextCtrl->Unbind( wxEVT_TEXT_ENTER, &guLastFMPanel::OnSearchSelected, this );
    m_AlbumsStaticText->Unbind( wxEVT_LEFT_DCLICK, &guLastFMPanel::OnTopAlbumsTitleDClick, this );
    m_TopTracksStaticText->Unbind( wxEVT_LEFT_DCLICK, &guLastFMPanel::OnTopTracksTitleDClick, this );
    m_SimArtistsStaticText->Unbind( wxEVT_LEFT_DCLICK, &guLastFMPanel::OnSimArTitleDClick, this );
    m_SimTracksStaticText->Unbind( wxEVT_LEFT_DCLICK, &guLastFMPanel::OnSimTrTitleDClick, this );
    m_EventsStaticText->Unbind( wxEVT_LEFT_DCLICK, &guLastFMPanel::OnEventsTitleDClick, this );

    m_AlbumsStaticText->Unbind( wxEVT_CONTEXT_MENU, &guLastFMPanel::OnContextMenu, this );
    m_TopTracksStaticText->Unbind( wxEVT_CONTEXT_MENU, &guLastFMPanel::OnContextMenu, this );
    m_SimArtistsStaticText->Unbind( wxEVT_CONTEXT_MENU, &guLastFMPanel::OnContextMenu, this );
    m_SimTracksStaticText->Unbind( wxEVT_CONTEXT_MENU, &guLastFMPanel::OnContextMenu, this );

    Unbind( wxEVT_MENU, &guLastFMPanel::OnPlayClicked, this, ID_LASTFM_PLAY );
    Unbind( wxEVT_MENU, &guLastFMPanel::OnEnqueueClicked, this, ID_LASTFM_ENQUEUE_AFTER_ALL, ID_LASTFM_ENQUEUE_AFTER_ARTIST );
    Unbind( wxEVT_MENU, &guLastFMPanel::OnSaveClicked, this, ID_LASTFM_SAVETOPLAYLIST );
    Unbind( wxEVT_MENU, &guLastFMPanel::OnCopyToClicked, this, ID_COPYTO_BASE, ID_COPYTO_BASE + guCOPYTO_MAXCOUNT );
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::ShowCurrentTrack( void )
{
    m_ArtistName = m_TrackChangeItems[ m_CurrentTrackInfo ].m_ArtistName;
    m_TrackName = m_TrackChangeItems[ m_CurrentTrackInfo ].m_TrackName;
    m_MediaViewer = m_TrackChangeItems[ m_CurrentTrackInfo ].m_MediaViewer;
    m_Db = m_MediaViewer ? m_MediaViewer->GetDb() : NULL;
    //guLogMessage( wxT( ">> LastFMPanel:ShowCurrentTrack( '%s', '%s' )" ), m_ArtistName.c_str(), m_TrackName.c_str() );

    if( m_LastArtistName != m_ArtistName )
    {
        m_ArtistsUpdateThreadMutex.Lock();
        if( m_ArtistsUpdateThread )
        {
            m_ArtistsUpdateThread->Pause();
            m_ArtistsUpdateThread->Delete();
            m_ArtistsUpdateThread = NULL;
        }

        m_AlbumsUpdateThreadMutex.Lock();
        if( m_AlbumsUpdateThread )
        {
            m_AlbumsUpdateThread->Pause();
            m_AlbumsUpdateThread->Delete();
            m_AlbumsUpdateThread = NULL;
        }

        m_TopTracksUpdateThreadMutex.Lock();
        if( m_TopTracksUpdateThread )
        {
            m_TopTracksUpdateThread->Pause();
            m_TopTracksUpdateThread->Delete();
            m_TopTracksUpdateThread = NULL;
        }

        m_SimArtistsUpdateThreadMutex.Lock();
        if( m_SimArtistsUpdateThread )
        {
            m_SimArtistsUpdateThread->Pause();
            m_SimArtistsUpdateThread->Delete();
            m_SimArtistsUpdateThread = NULL;
        }

        m_EventsUpdateThreadMutex.Lock();
        if( m_EventsUpdateThread )
        {
            m_EventsUpdateThread->Pause();
            m_EventsUpdateThread->Delete();
            m_EventsUpdateThread = NULL;
        }

        // Clear the LastFM controls to default values
        m_ArtistInfoCtrl->Clear( m_MediaViewer );

        for( int index = 0; index < GULASTFMINFO_MAXITEMS; index++ )
        {
            m_AlbumsInfoCtrls[ index ]->Clear( m_MediaViewer );
            m_TopTrackInfoCtrls[ index ]->Clear( m_MediaViewer );
            m_SimArtistsInfoCtrls[ index ]->Clear( m_MediaViewer );
            m_EventsInfoCtrls[ index ]->Clear( m_MediaViewer );
        }

        m_AlbumsCount = 0;
        m_AlbumsPageStart = 0;
        m_AlbumsPrevBtn->Enable( false );
        m_AlbumsNextBtn->Enable( false );
        UpdateAlbumsRangeLabel();

        m_TopTracksCount = 0;
        m_TopTracksPageStart = 0;
        m_TopTracksPrevBtn->Enable( false );
        m_TopTracksNextBtn->Enable( false );
        UpdateTopTracksRangeLabel();

        m_SimArtistsCount = 0;
        m_SimArtistsPageStart = 0;
        m_SimArtistsPrevBtn->Enable( false );
        m_SimArtistsNextBtn->Enable( false );
        UpdateSimArtistsRangeLabel();

        m_EventsCount = 0;
        m_EventsPageStart = 0;
        m_EventsPrevBtn->Enable( false );
        m_EventsNextBtn->Enable( false );
        UpdateEventsRangeLabel();

        m_LastArtistName = m_ArtistName;

        if( !m_ArtistName.IsEmpty() )
        {
            m_ArtistsUpdateThread = new guFetchArtistInfoThread( this, m_DbCache, m_ArtistName.c_str() );
            m_AlbumsUpdateThread = new guFetchAlbumInfoThread( this, m_DbCache, m_ArtistName.c_str(), m_AlbumsPageStart );
            m_TopTracksUpdateThread = new guFetchTopTracksInfoThread( this, m_DbCache, m_ArtistName.c_str(), m_TopTracksPageStart );
            m_SimArtistsUpdateThread = new guFetchSimilarArtistInfoThread( this, m_DbCache, m_ArtistName.c_str(), m_TopTracksPageStart );
            m_EventsUpdateThread = new guFetchEventsInfoThread( this, m_DbCache, m_ArtistName.c_str(), m_EventsPageStart );
        }

        m_ArtistTextCtrl->SetValue( m_ArtistName );

        m_EventsUpdateThreadMutex.Unlock();
        m_SimArtistsUpdateThreadMutex.Unlock();
        m_TopTracksUpdateThreadMutex.Unlock();
        m_AlbumsUpdateThreadMutex.Unlock();
        m_ArtistsUpdateThreadMutex.Unlock();
    }

    if( m_LastTrackName != m_TrackName )
    {
        m_SimTracksUpdateThreadMutex.Lock();
        if( m_SimTracksUpdateThread )
        {
            m_SimTracksUpdateThread->Pause();
            m_SimTracksUpdateThread->Delete();
            m_SimTracksUpdateThread = NULL;
        }

        for( int index = 0; index < GULASTFMINFO_MAXITEMS; index++ )
        {
            m_SimTracksInfoCtrls[ index ]->Clear( m_MediaViewer );
        }

        m_SimTracksCount = 0;
        m_SimTracksPageStart = 0;
        m_SimTracksPrevBtn->Enable( false );
        m_SimTracksNextBtn->Enable( false );
        UpdateSimTracksRangeLabel();

        m_LastTrackName = m_TrackName;
        if( !m_ArtistName.IsEmpty() && !m_TrackName.IsEmpty() )
        {
            m_SimTracksUpdateThread = new guFetchSimTracksInfoThread( this, m_DbCache,
                m_ArtistName.c_str(), m_TrackName.c_str(), m_SimTracksPageStart );
        }
        m_TrackTextCtrl->SetValue( m_TrackName );

        m_SimTracksUpdateThreadMutex.Unlock();
    }
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::UpdateAlbumsRangeLabel( void )
{
    if( m_AlbumsCount )
    {
        int StartAlbumIndex = m_AlbumsPageStart * GULASTFMINFO_MAXITEMS;
        m_AlbumsRangeLabel->SetLabel( wxString::Format( wxT( "%i - %i / %i" ),
                                StartAlbumIndex + 1,
                                wxMin( StartAlbumIndex + GULASTFMINFO_MAXITEMS, m_AlbumsCount ),
                                m_AlbumsCount ) );
    }
    else
    {
        m_AlbumsRangeLabel->SetLabel( wxT( "0 - 0 / 0"));
    }
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnAlbumsCountUpdated( wxCommandEvent &event )
{
    m_AlbumsCount = event.GetInt();
    UpdateAlbumsRangeLabel();
    m_AlbumsPrevBtn->Enable( m_AlbumsPageStart );
    m_AlbumsNextBtn->Enable( ( ( m_AlbumsPageStart + 1 )  * GULASTFMINFO_MAXITEMS ) < m_AlbumsCount );
}

// -------------------------------------------------------------------------------- //
void  guLastFMPanel::OnAlbumsPrevClicked( wxCommandEvent &event )
{
    if( m_AlbumsPageStart )
    {
        m_AlbumsUpdateThreadMutex.Lock();
        if( m_AlbumsUpdateThread )
        {
            m_AlbumsUpdateThread->Pause();
            m_AlbumsUpdateThread->Delete();
            m_AlbumsUpdateThread = NULL;
        }

        m_AlbumsPageStart--;
        m_AlbumsPrevBtn->Enable( m_AlbumsPageStart );
        m_AlbumsNextBtn->Enable( ( ( m_AlbumsPageStart + 1 )  * GULASTFMINFO_MAXITEMS ) < m_AlbumsCount );

        int index;
        for( index = 0; index < GULASTFMINFO_MAXITEMS; index++ )
        {
            m_AlbumsInfoCtrls[ index ]->Clear( m_MediaViewer );
        }

        m_AlbumsUpdateThread = new guFetchAlbumInfoThread( this, m_DbCache, m_ArtistName.c_str(), m_AlbumsPageStart );

        m_AlbumsUpdateThreadMutex.Unlock();
    }
}

// -------------------------------------------------------------------------------- //
void  guLastFMPanel::OnAlbumsNextClicked( wxCommandEvent &event )
{
    if( ( ( m_AlbumsPageStart + 1 ) * GULASTFMINFO_MAXITEMS ) < m_AlbumsCount )
    {
        m_AlbumsUpdateThreadMutex.Lock();
        if( m_AlbumsUpdateThread )
        {
            m_AlbumsUpdateThread->Pause();
            m_AlbumsUpdateThread->Delete();
            m_AlbumsUpdateThread = NULL;
        }

        m_AlbumsPageStart++;
        m_AlbumsPrevBtn->Enable( m_AlbumsPageStart );
        m_AlbumsNextBtn->Enable( ( ( m_AlbumsPageStart + 1 )  * GULASTFMINFO_MAXITEMS ) < m_AlbumsCount );

        int index;
        for( index = 0; index < GULASTFMINFO_MAXITEMS; index++ )
        {
            m_AlbumsInfoCtrls[ index ]->Clear( m_MediaViewer );
        }

        m_AlbumsUpdateThread = new guFetchAlbumInfoThread( this, m_DbCache, m_ArtistName.c_str(), m_AlbumsPageStart );

        m_AlbumsUpdateThreadMutex.Unlock();
    }
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::UpdateTopTracksRangeLabel( void )
{
    if( m_TopTracksCount )
    {
        int StartIndex = m_TopTracksPageStart * GULASTFMINFO_MAXITEMS;
        m_TopTracksRangeLabel->SetLabel( wxString::Format( wxT( "%i - %i / %i" ),
                                StartIndex + 1,
                                wxMin( StartIndex + GULASTFMINFO_MAXITEMS, m_TopTracksCount ),
                                m_TopTracksCount ) );
    }
    else
    {
        m_TopTracksRangeLabel->SetLabel( wxT( "0 - 0 / 0"));
    }
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnTopTracksCountUpdated( wxCommandEvent &event )
{
    m_TopTracksCount = event.GetInt();
    UpdateTopTracksRangeLabel();
    m_TopTracksPrevBtn->Enable( m_TopTracksPageStart );
    m_TopTracksNextBtn->Enable( ( ( m_TopTracksPageStart + 1 )  * GULASTFMINFO_MAXITEMS ) < m_TopTracksCount );
}

// -------------------------------------------------------------------------------- //
void  guLastFMPanel::OnTopTracksPrevClicked( wxCommandEvent &event )
{
    if( m_TopTracksPageStart )
    {
        m_TopTracksUpdateThreadMutex.Lock();
        if( m_TopTracksUpdateThread )
        {
            m_TopTracksUpdateThread->Pause();
            m_TopTracksUpdateThread->Delete();
            m_TopTracksUpdateThread = NULL;
        }

        m_TopTracksPageStart--;
        m_TopTracksPrevBtn->Enable( m_TopTracksPageStart );
        m_TopTracksNextBtn->Enable( ( ( m_TopTracksPageStart + 1 )  * GULASTFMINFO_MAXITEMS ) < m_TopTracksCount );

        int index;
        for( index = 0; index < GULASTFMINFO_MAXITEMS; index++ )
        {
            m_TopTrackInfoCtrls[ index ]->Clear( m_MediaViewer );
        }

        m_TopTracksUpdateThread = new guFetchTopTracksInfoThread( this, m_DbCache, m_ArtistName.c_str(), m_TopTracksPageStart );

        m_TopTracksUpdateThreadMutex.Unlock();
    }
}

// -------------------------------------------------------------------------------- //
void  guLastFMPanel::OnTopTracksNextClicked( wxCommandEvent &event )
{
    if( ( ( m_TopTracksPageStart + 1 ) * GULASTFMINFO_MAXITEMS ) < m_TopTracksCount )
    {
        m_TopTracksUpdateThreadMutex.Lock();
        if( m_TopTracksUpdateThread )
        {
            m_TopTracksUpdateThread->Pause();
            m_TopTracksUpdateThread->Delete();
            m_TopTracksUpdateThread = NULL;
        }

        m_TopTracksPageStart++;
        m_TopTracksPrevBtn->Enable( m_TopTracksPageStart );
        m_TopTracksNextBtn->Enable( ( ( m_TopTracksPageStart + 1 )  * GULASTFMINFO_MAXITEMS ) < m_TopTracksCount );

        int index;
        for( index = 0; index < GULASTFMINFO_MAXITEMS; index++ )
        {
            m_TopTrackInfoCtrls[ index ]->Clear( m_MediaViewer );
        }

        m_TopTracksUpdateThread = new guFetchTopTracksInfoThread( this, m_DbCache, m_ArtistName.c_str(), m_TopTracksPageStart );

        m_TopTracksUpdateThreadMutex.Unlock();
    }
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::UpdateSimArtistsRangeLabel( void )
{
    if( m_SimArtistsCount )
    {
        int StartIndex = m_SimArtistsPageStart * GULASTFMINFO_MAXITEMS;
        m_SimArtistsRangeLabel->SetLabel( wxString::Format( wxT( "%i - %i / %i" ),
                                StartIndex + 1,
                                wxMin( StartIndex + GULASTFMINFO_MAXITEMS, m_SimArtistsCount ),
                                m_SimArtistsCount ) );
    }
    else
    {
        m_SimArtistsRangeLabel->SetLabel( wxT( "0 - 0 / 0"));
    }
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnSimArtistsCountUpdated( wxCommandEvent &event )
{
    m_SimArtistsCount = event.GetInt();
    UpdateSimArtistsRangeLabel();
    m_SimArtistsPrevBtn->Enable( m_SimArtistsPageStart );
    m_SimArtistsNextBtn->Enable( ( ( m_SimArtistsPageStart + 1 )  * GULASTFMINFO_MAXITEMS ) < m_SimArtistsCount );
}

// -------------------------------------------------------------------------------- //
void  guLastFMPanel::OnSimArtistsPrevClicked( wxCommandEvent &event )
{
    if( m_SimArtistsPageStart )
    {
        m_SimArtistsUpdateThreadMutex.Lock();
        if( m_SimArtistsUpdateThread )
        {
            m_SimArtistsUpdateThread->Pause();
            m_SimArtistsUpdateThread->Delete();
            m_SimArtistsUpdateThread = NULL;
        }

        m_SimArtistsPageStart--;
        m_SimArtistsPrevBtn->Enable( m_SimArtistsPageStart );
        m_SimArtistsNextBtn->Enable( ( ( m_SimArtistsPageStart + 1 )  * GULASTFMINFO_MAXITEMS ) < m_SimArtistsCount );

        int index;
        for( index = 0; index < GULASTFMINFO_MAXITEMS; index++ )
        {
            m_SimArtistsInfoCtrls[ index ]->Clear( m_MediaViewer );
        }

        m_SimArtistsUpdateThread = new guFetchSimilarArtistInfoThread( this, m_DbCache, m_ArtistName.c_str(), m_SimArtistsPageStart );

        m_SimArtistsUpdateThreadMutex.Unlock();
    }
}

// -------------------------------------------------------------------------------- //
void  guLastFMPanel::OnSimArtistsNextClicked( wxCommandEvent &event )
{
    if( ( ( m_SimArtistsPageStart + 1 ) * GULASTFMINFO_MAXITEMS ) < m_SimArtistsCount )
    {
        m_SimArtistsUpdateThreadMutex.Lock();
        if( m_SimArtistsUpdateThread )
        {
            m_SimArtistsUpdateThread->Pause();
            m_SimArtistsUpdateThread->Delete();
            m_SimArtistsUpdateThread = NULL;
        }

        m_SimArtistsPageStart++;
        m_SimArtistsPrevBtn->Enable( m_SimArtistsPageStart );
        m_SimArtistsNextBtn->Enable( ( ( m_SimArtistsPageStart + 1 )  * GULASTFMINFO_MAXITEMS ) < m_SimArtistsCount );

        int index;
        for( index = 0; index < GULASTFMINFO_MAXITEMS; index++ )
        {
            m_SimArtistsInfoCtrls[ index ]->Clear( m_MediaViewer );
        }

        m_SimArtistsUpdateThread = new guFetchSimilarArtistInfoThread( this, m_DbCache, m_ArtistName.c_str(), m_SimArtistsPageStart );

        m_SimArtistsUpdateThreadMutex.Unlock();
    }
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::UpdateEventsRangeLabel( void )
{
    if( m_EventsCount )
    {
        int StartIndex = m_EventsPageStart * GULASTFMINFO_MAXITEMS;
        m_EventsRangeLabel->SetLabel( wxString::Format( wxT( "%i - %i / %i" ),
                                StartIndex + 1,
                                wxMin( StartIndex + GULASTFMINFO_MAXITEMS, m_EventsCount ),
                                m_EventsCount ) );
    }
    else
    {
        m_EventsRangeLabel->SetLabel( wxT( "0 - 0 / 0"));
    }
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnEventsCountUpdated( wxCommandEvent &event )
{
    m_EventsCount = event.GetInt();
    UpdateEventsRangeLabel();
    m_EventsPrevBtn->Enable( m_EventsPageStart );
    m_EventsNextBtn->Enable( ( ( m_EventsPageStart + 1 )  * GULASTFMINFO_MAXITEMS ) < m_EventsCount );
}

// -------------------------------------------------------------------------------- //
void  guLastFMPanel::OnEventsPrevClicked( wxCommandEvent &event )
{
    if( m_EventsPageStart )
    {
        m_EventsUpdateThreadMutex.Lock();
        if( m_EventsUpdateThread )
        {
            m_EventsUpdateThread->Pause();
            m_EventsUpdateThread->Delete();
            m_EventsUpdateThread = NULL;
        }

        m_EventsPageStart--;
        m_EventsPrevBtn->Enable( m_EventsPageStart );
        m_EventsNextBtn->Enable( ( ( m_EventsPageStart + 1 )  * GULASTFMINFO_MAXITEMS ) < m_EventsCount );

        int index;
        for( index = 0; index < GULASTFMINFO_MAXITEMS; index++ )
        {
            m_EventsInfoCtrls[ index ]->Clear( m_MediaViewer );
        }

        m_EventsUpdateThread = new guFetchEventsInfoThread( this, m_DbCache, m_ArtistName.c_str(), m_EventsPageStart );

        m_EventsUpdateThreadMutex.Unlock();
    }
}

// -------------------------------------------------------------------------------- //
void  guLastFMPanel::OnEventsNextClicked( wxCommandEvent &event )
{
    if( ( ( m_EventsPageStart + 1 ) * GULASTFMINFO_MAXITEMS ) < m_EventsCount )
    {
        m_EventsUpdateThreadMutex.Lock();
        if( m_EventsUpdateThread )
        {
            m_EventsUpdateThread->Pause();
            m_EventsUpdateThread->Delete();
            m_EventsUpdateThread = NULL;
        }

        m_EventsPageStart++;
        m_EventsPrevBtn->Enable( m_EventsPageStart );
        m_EventsNextBtn->Enable( ( ( m_EventsPageStart + 1 )  * GULASTFMINFO_MAXITEMS ) < m_EventsCount );

        int index;
        for( index = 0; index < GULASTFMINFO_MAXITEMS; index++ )
        {
            m_EventsInfoCtrls[ index ]->Clear( m_MediaViewer );
        }

        m_EventsUpdateThread = new guFetchEventsInfoThread( this, m_DbCache, m_ArtistName.c_str(), m_EventsPageStart );

        m_EventsUpdateThreadMutex.Unlock();
    }
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::UpdateSimTracksRangeLabel( void )
{
    if( m_SimTracksCount )
    {
        int StartIndex = m_SimTracksPageStart * GULASTFMINFO_MAXITEMS;
        m_SimTracksRangeLabel->SetLabel( wxString::Format( wxT( "%i - %i / %i" ),
                                StartIndex + 1,
                                wxMin( StartIndex + GULASTFMINFO_MAXITEMS, m_SimTracksCount ),
                                m_SimTracksCount ) );
    }
    else
    {
        m_SimTracksRangeLabel->SetLabel( wxT( "0 - 0 / 0"));
    }
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnSimTracksCountUpdated( wxCommandEvent &event )
{
    m_SimTracksCount = event.GetInt();
    UpdateSimTracksRangeLabel();
    m_SimTracksPrevBtn->Enable( m_SimTracksPageStart );
    m_SimTracksNextBtn->Enable( ( ( m_SimTracksPageStart + 1 )  * GULASTFMINFO_MAXITEMS ) < m_SimTracksCount );
}

// -------------------------------------------------------------------------------- //
void  guLastFMPanel::OnSimTracksPrevClicked( wxCommandEvent &event )
{
    if( m_SimTracksPageStart )
    {
        m_SimTracksUpdateThreadMutex.Lock();
        if( m_SimTracksUpdateThread )
        {
            m_SimTracksUpdateThread->Pause();
            m_SimTracksUpdateThread->Delete();
            m_SimTracksUpdateThread = NULL;
        }

        m_SimTracksPageStart--;
        m_SimTracksPrevBtn->Enable( m_SimTracksPageStart );
        m_SimTracksNextBtn->Enable( ( ( m_SimTracksPageStart + 1 )  * GULASTFMINFO_MAXITEMS ) < m_SimTracksCount );

        int index;
        for( index = 0; index < GULASTFMINFO_MAXITEMS; index++ )
        {
            m_SimTracksInfoCtrls[ index ]->Clear( m_MediaViewer );
        }

        m_SimTracksUpdateThread = new guFetchSimTracksInfoThread( this, m_DbCache, m_ArtistName.c_str(), m_TrackName.c_str(), m_SimTracksPageStart );

        m_SimTracksUpdateThreadMutex.Unlock();
    }
}

// -------------------------------------------------------------------------------- //
void  guLastFMPanel::OnSimTracksNextClicked( wxCommandEvent &event )
{
    if( ( ( m_SimTracksPageStart + 1 ) * GULASTFMINFO_MAXITEMS ) < m_SimTracksCount )
    {
        m_SimTracksUpdateThreadMutex.Lock();
        if( m_SimTracksUpdateThread )
        {
            m_SimTracksUpdateThread->Pause();
            m_SimTracksUpdateThread->Delete();
            m_SimTracksUpdateThread = NULL;
        }

        m_SimTracksPageStart++;
        m_SimTracksPrevBtn->Enable( m_SimTracksPageStart );
        m_SimTracksNextBtn->Enable( ( ( m_SimTracksPageStart + 1 )  * GULASTFMINFO_MAXITEMS ) < m_SimTracksCount );

        int index;
        for( index = 0; index < GULASTFMINFO_MAXITEMS; index++ )
        {
            m_SimTracksInfoCtrls[ index ]->Clear( m_MediaViewer );
        }

        m_SimTracksUpdateThread = new guFetchSimTracksInfoThread( this, m_DbCache, m_ArtistName.c_str(), m_TrackName.c_str(), m_SimTracksPageStart );

        m_SimTracksUpdateThreadMutex.Unlock();
    }
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::SetUpdateEnable( bool value )
{
    m_UpdateCheckBox->SetValue( value );
    m_UpdateEnabled = value;

    wxCommandEvent event;
    OnUpdateChkBoxClick( event );
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnUpdateChkBoxClick( wxCommandEvent &event )
{
    m_UpdateEnabled = m_UpdateCheckBox->IsChecked();
    if( m_UpdateEnabled )
    {
        wxCommandEvent UpdateEvent( wxEVT_MENU, ID_MAINFRAME_REQUEST_CURRENTTRACK );
        UpdateEvent.SetClientData( this );
        wxPostEvent( guMainFrame::GetMainFrame(), UpdateEvent );
    }
    //
    m_ArtistTextCtrl->Enable( !m_UpdateEnabled );
    m_TrackTextCtrl->Enable( !m_UpdateEnabled );
//    m_SearchButton->Enable( !m_UpdateEnabled &&
//                !m_ArtistTextCtrl->IsEmpty() );
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnTextUpdated( wxCommandEvent& event )
{
    m_TrackTextCtrl->Enable( !m_UpdateCheckBox->IsChecked() && !event.GetString().IsEmpty() );
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::AppendTrackChangeInfo( const guTrackChangeInfo * trackchangeinfo )
{
    // Even when m_CurrentTrackInfo == -1 (wxNOT_FOUND)
    // This is valid because its not < than 0 which is the count
    // and it will be incremented to 0

    // Delete all the itesm after the one we are showing
    while( m_CurrentTrackInfo < ( int ) ( m_TrackChangeItems.Count() - 1 ) )
    {
        m_TrackChangeItems.RemoveAt( m_TrackChangeItems.Count() - 1 );
    }

    // Add the item
    m_TrackChangeItems.Add( new guTrackChangeInfo( * trackchangeinfo ) );
    m_CurrentTrackInfo++;

    if( m_TrackChangeItems.Count() > guTRACKCHANGEINFO_MAXCOUNT )
    {
        m_TrackChangeItems.RemoveAt( 0 );
        m_CurrentTrackInfo--;
    }

    UpdateTrackChangeButtons();
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::UpdateTrackChangeButtons( void )
{
    m_PrevButton->Enable( m_CurrentTrackInfo > 0 );
    m_NextButton->Enable( m_CurrentTrackInfo < ( int ) ( m_TrackChangeItems.Count() - 1 ) );
    m_ReloadButton->Enable( m_TrackChangeItems.Count() );
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnUpdatedTrack( wxCommandEvent &event )
{
    if( !m_UpdateEnabled )
        return;

    // Player informs there is a new track playing
    //guLogMessage( wxT( "Received LastFMPanel::UpdateTrack event" ) );

    const guTrack * Track = ( guTrack * ) event.GetClientData();
    guTrackChangeInfo ChangeInfo;

    if( Track )
    {
        ChangeInfo.m_ArtistName = Track->m_ArtistName;
        ChangeInfo.m_TrackName = Track->m_SongName;
        ChangeInfo.m_MediaViewer = Track->m_MediaViewer;
    }
    //guLogMessage( wxT( "%s - %s" ), TrackChangeInfo->m_ArtistName.c_str(), TrackChangeInfo->m_TrackName.c_str() );
    AppendTrackChangeInfo( &ChangeInfo );
    ShowCurrentTrack();
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnPrevBtnClick( wxCommandEvent& event )
{
    SetUpdateEnable( false );

    if( m_CurrentTrackInfo > 0 )
    {
        m_CurrentTrackInfo--;
    }

    UpdateTrackChangeButtons();
    ShowCurrentTrack();
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnNextBtnClick( wxCommandEvent& event )
{
    SetUpdateEnable( false );

    if( m_CurrentTrackInfo < ( int ) ( m_TrackChangeItems.Count() - 1 ) )
    {
        m_CurrentTrackInfo++;
    }

    UpdateTrackChangeButtons();
    ShowCurrentTrack();
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnReloadBtnClick( wxCommandEvent& event )
{
    m_LastArtistName = wxEmptyString;
    m_LastTrackName = wxEmptyString;

    ShowCurrentTrack();
}

//// -------------------------------------------------------------------------------- //
//void guLastFMPanel::OnSearchBtnClick( wxCommandEvent& event )
//{
//    guTrackChangeInfo TrackChangeInfo( m_ArtistTextCtrl->GetValue(), m_TrackTextCtrl->GetValue() );
//    AppendTrackChangeInfo( &TrackChangeInfo );
//    ShowCurrentTrack();
//}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnTextCtrlKeyDown( wxKeyEvent &event )
{
    if( event.GetKeyCode() == WXK_RETURN )
    {
        wxCommandEvent CmdEvent( wxEVT_TEXT_ENTER );
        m_ArtistTextCtrl->GetEventHandler()->AddPendingEvent( CmdEvent );
        return;
    }
    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnSearchSelected( wxCommandEvent &event )
{
    if( !m_ArtistTextCtrl->IsEmpty() )
    {
        guTrackChangeInfo TrackChangeInfo( m_ArtistTextCtrl->GetValue(), m_TrackTextCtrl->GetValue(), m_MediaViewer );
        AppendTrackChangeInfo( &TrackChangeInfo );
        ShowCurrentTrack();
    }
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnUpdateArtistInfo( wxCommandEvent &event )
{
    wxMutexLocker Locker( m_UpdateInfoMutex );
    //guLogMessage( wxT( "Got Event for BIO updating" ) );
    guLastFMArtistInfo * ArtistInfo = ( guLastFMArtistInfo * ) event.GetClientData();
    if( ArtistInfo )
    {
        m_ArtistInfoCtrl->SetInfo( ArtistInfo );
        m_MainSizer->FitInside( this );
    }
    else
    {
        guLogError( wxT( "Received a LastFMInfo bio event with NULL Data" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnUpdateAlbumItem( wxCommandEvent &event )
{
    wxMutexLocker Locker( m_UpdateInfoMutex );
    // Player informs there is a new track playing
    int index = event.GetInt();
    //guLogMessage( wxT( "Received LastFMInfoItem %u" ), index );
    guLastFMAlbumInfo * AlbumInfo = ( guLastFMAlbumInfo * ) event.GetClientData();
    if( AlbumInfo )
    {
        m_AlbumsInfoCtrls[ index ]->SetInfo( AlbumInfo );
        //Layout();
        m_MainSizer->FitInside( this );
    }
    else
    {
        guLogError( wxT( "Received a LastFMInfo album event with NULL Data" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnUpdateArtistItem( wxCommandEvent &event )
{
    wxMutexLocker Locker( m_UpdateInfoMutex );
    // Player informs there is a new track playing
    int index = event.GetInt();
    //guLogMessage( wxT( "Received LastFMInfoItem %u" ), index );
    guLastFMSimilarArtistInfo * ArtistInfo = ( guLastFMSimilarArtistInfo * ) event.GetClientData();
    if( ArtistInfo )
    {
        m_SimArtistsInfoCtrls[ index ]->SetInfo( ArtistInfo );
        //Layout();
        m_MainSizer->FitInside( this );
    }
    else
    {
        guLogError( wxT( "Received a LastFMInfo artist event with NULL Data" ) );
    }
}


// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnUpdateTrackItem( wxCommandEvent &event )
{
    wxMutexLocker Locker( m_UpdateInfoMutex );
    // Player informs there is a new track playing
    int index = event.GetInt();
    //guLogMessage( wxT( "Received LastFMInfoItem %u" ), index );
    guLastFMTrackInfo * TrackInfo = ( guLastFMTrackInfo * ) event.GetClientData();
    if( TrackInfo )
    {
        m_SimTracksInfoCtrls[ index ]->SetInfo( TrackInfo );
        //Layout();
        m_MainSizer->FitInside( this );
    }
    else
    {
        guLogError( wxT( "Received a LastFMInfo track event with NULL Data" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnUpdateTopTrackItem( wxCommandEvent &event )
{
    wxMutexLocker Locker( m_UpdateInfoMutex );
    // Player informs there is a new track playing
    int index = event.GetInt();
    //guLogMessage( wxT( "Received LastFMInfoItem %u" ), index );
    guLastFMTopTrackInfo * TrackInfo = ( guLastFMTopTrackInfo * ) event.GetClientData();
    if( TrackInfo )
    {
        m_TopTrackInfoCtrls[ index ]->SetInfo( TrackInfo );
        //Layout();
        m_MainSizer->FitInside( this );
    }
    else
    {
        guLogError( wxT( "Received a LastFMInfo top track event with NULL Data" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnUpdateEventItem( wxCommandEvent &event )
{
    wxMutexLocker Locker( m_UpdateInfoMutex );
    // Player informs there is a new track playing
    int index = event.GetInt();
    //guLogMessage( wxT( "Received LastFMInfoItem %u" ), index );
    guLastFMEventInfo * EventInfo = ( guLastFMEventInfo * ) event.GetClientData();
    if( EventInfo )
    {
        m_EventsInfoCtrls[ index ]->SetInfo( EventInfo );
        //Layout();
        m_MainSizer->FitInside( this );
    }
    else
    {
        guLogError( wxT( "Received a LastFMInfo Event event with NULL Data" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnArInfoTitleDClicked( wxMouseEvent &event )
{
    m_ShowArtistDetails = !m_ShowArtistDetails;
    //ArtistDetailsSizer->Show( ShowArtistDetails );
    if( m_ShowArtistDetails )
        m_MainSizer->Show( m_ArtistInfoMainSizer );
    else
        m_MainSizer->Hide( m_ArtistInfoMainSizer );
    m_MainSizer->FitInside( this );
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::SetTopAlbumsVisible( const bool dolayout )
{
    if( m_ShowAlbums )
        m_MainSizer->Show( m_AlbumsSizer );
    else
        m_MainSizer->Hide( m_AlbumsSizer );

    m_AlbumsRangeLabel->Show( m_ShowAlbums );
    m_AlbumsPrevBtn->Show( m_ShowAlbums );
    m_AlbumsNextBtn->Show( m_ShowAlbums );

    if( dolayout )
        UpdateLayout();
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::SetTopTracksVisible( const bool dolayout )
{
    if( m_ShowTopTracks )
        m_MainSizer->Show( m_TopTracksSizer );
    else
        m_MainSizer->Hide( m_TopTracksSizer );

    m_TopTracksRangeLabel->Show( m_ShowTopTracks );
    m_TopTracksPrevBtn->Show( m_ShowTopTracks );
    m_TopTracksNextBtn->Show( m_ShowTopTracks );

    if( dolayout )
        UpdateLayout();
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::SetSimArtistsVisible( const bool dolayout )
{
    if( m_ShowSimArtists )
        m_MainSizer->Show( m_SimArtistsSizer );
    else
        m_MainSizer->Hide( m_SimArtistsSizer );

    m_SimArtistsRangeLabel->Show( m_ShowSimArtists );
    m_SimArtistsPrevBtn->Show( m_ShowSimArtists );
    m_SimArtistsNextBtn->Show( m_ShowSimArtists );

    if( dolayout )
        UpdateLayout();
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::SetSimTracksVisible( const bool dolayout )
{
    if( m_ShowSimTracks )
        m_MainSizer->Show( m_SimTracksSizer );
    else
        m_MainSizer->Hide( m_SimTracksSizer );

    m_SimTracksRangeLabel->Show( m_ShowSimTracks );
    m_SimTracksPrevBtn->Show( m_ShowSimTracks );
    m_SimTracksNextBtn->Show( m_ShowSimTracks );

    if( dolayout )
        UpdateLayout();
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::SetEventsVisible( const bool dolayout )
{
    if( m_ShowEvents )
        m_MainSizer->Show( m_EventsSizer );
    else
        m_MainSizer->Hide( m_EventsSizer );

    m_EventsRangeLabel->Show( m_ShowEvents );
    m_EventsPrevBtn->Show( m_ShowEvents );
    m_EventsNextBtn->Show( m_ShowEvents );

    if( dolayout )
        UpdateLayout();
}


// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnTopAlbumsTitleDClick( wxMouseEvent &event )
{
    m_ShowAlbums = !m_ShowAlbums;
    SetTopAlbumsVisible( true );
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnTopTracksTitleDClick( wxMouseEvent &event )
{
    m_ShowTopTracks = !m_ShowTopTracks;
    SetTopTracksVisible( true );
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnSimArTitleDClick( wxMouseEvent &event )
{
    m_ShowSimArtists = !m_ShowSimArtists;
    SetSimArtistsVisible( true );
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnSimTrTitleDClick( wxMouseEvent &event )
{
    m_ShowSimTracks = !m_ShowSimTracks;
    SetSimTracksVisible( true );
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnEventsTitleDClick( wxMouseEvent &event )
{
    m_ShowEvents = !m_ShowEvents;
    SetEventsVisible( true );
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnDropFiles( const wxArrayString &files )
{
    //guLogMessage( wxT( "guLastFMPanelDropTarget::OnDropFiles" ) );
    if( !files.Count() )
        return;

    guTrackChangeInfo ChangeInfo;
    guTrack Track;
    guDbLibrary * Db = m_Db ? m_Db : m_DefaultDb;
    if( !Db->FindTrackFile( files[ 0 ], &Track ) )
    {
        if( Track.ReadFromFile( files[ 0 ] ) )
        {
            Track.m_Type = guTRACK_TYPE_NOTDB;
            Track.m_MediaViewer = NULL;
        }
        else
        {
            return;
        }
    }
    else
    {
        Track.m_MediaViewer = m_MediaViewer;
    }

    ChangeInfo.m_ArtistName = Track.m_ArtistName;
    ChangeInfo.m_TrackName = Track.m_SongName;
    ChangeInfo.m_MediaViewer = Track.m_MediaViewer;

    SetUpdateEnable( false );
    AppendTrackChangeInfo( &ChangeInfo );
    ShowCurrentTrack();
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnDropFiles( const guTrackArray * tracks )
{
    //guLogMessage( wxT( "guLastFMPanelDropTarget::OnDropFiles" ) );
    if( tracks && tracks->Count() )
    {
        guTrackChangeInfo ChangeInfo;

        const guTrack & Track = tracks->Item( 0 );

        if( Track.m_MediaViewer )
        {
            m_MediaViewer = Track.m_MediaViewer;
            m_Db = m_MediaViewer->GetDb();
        }
        else
        {
            m_MediaViewer = NULL;
            m_Db = NULL;
        }

        ChangeInfo.m_ArtistName = Track.m_ArtistName;
        ChangeInfo.m_TrackName = Track.m_SongName;

        SetUpdateEnable( false );
        AppendTrackChangeInfo( &ChangeInfo );
        ShowCurrentTrack();
    }
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnContextMenu( wxContextMenuEvent &event )
{
    m_ContextMenuObject = ( wxStaticText * ) event.GetEventObject();

    wxMenu Menu;
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
        CalcUnscrolledPosition( Point.x, Point.y, &Point.x, &Point.y );
        Point = ScreenToClient( Point );
    }

    wxMenuItem * MenuItem = new wxMenuItem( &Menu, ID_LASTFM_PLAY, _( "Play" ), _( "Play the artist tracks" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
    Menu.Append( MenuItem );

    MenuItem = new wxMenuItem( &Menu, ID_LASTFM_ENQUEUE_AFTER_ALL, _( "Enqueue" ), _( "Enqueue the artist tracks to the playlist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    Menu.Append( MenuItem );

    wxMenu * EnqueueMenu = new wxMenu();

    MenuItem = new wxMenuItem( EnqueueMenu, ID_LASTFM_ENQUEUE_AFTER_TRACK,
                            wxString( _( "Current Track" ) ),
                            _( "Add current selected tracks to playlist after the current track" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    EnqueueMenu->Append( MenuItem );

    MenuItem = new wxMenuItem( EnqueueMenu, ID_LASTFM_ENQUEUE_AFTER_ALBUM,
                            wxString( _( "Current Album" ) ),
                            _( "Add current selected tracks to playlist after the current album" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    EnqueueMenu->Append( MenuItem );

    MenuItem = new wxMenuItem( EnqueueMenu, ID_LASTFM_ENQUEUE_AFTER_ARTIST,
                            wxString( _( "Current Artist" ) ),
                            _( "Add current selected tracks to playlist after the current artist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    EnqueueMenu->Append( MenuItem );

    Menu.Append( wxID_ANY, _( "Enqueue After" ), EnqueueMenu );

    Menu.AppendSeparator();

    MenuItem = new wxMenuItem( &Menu, ID_LASTFM_SAVETOPLAYLIST, _( "Save to Playlist" ), _( "Save the selected tracks to playlist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_doc_save ) );
    Menu.Append( MenuItem );

    Menu.AppendSeparator();

    guMainFrame * MainFrame = ( guMainFrame * ) guMainFrame::GetMainFrame();
    MainFrame->CreateCopyToMenu( &Menu );

    PopupMenu( &Menu, Point.x, Point.y );
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::GetContextMenuTracks( guTrackArray * tracks )
{
    guLastFM LastFM;

    guDbLibrary * Db = m_Db ? m_Db : m_DefaultDb;

    if( m_ContextMenuObject == m_AlbumsStaticText )
    {
        // Top Albums
        guAlbumInfoArray TopAlbums = LastFM.ArtistGetTopAlbums( m_ArtistName );
        wxArrayInt AlbumIds;
        int Index;
        int Count = TopAlbums.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            int AlbumId = Db->FindAlbum( m_ArtistName, TopAlbums[ Index ].m_Name );
            if( AlbumId != wxNOT_FOUND )
            {
                AlbumIds.Empty();
                AlbumIds.Add( AlbumId );
                Db->GetAlbumsSongs( AlbumIds, tracks );
            }
        }
    }
    else if( m_ContextMenuObject == m_TopTracksStaticText )
    {
        // Top Tracks
        guTopTrackInfoArray TopTracks = LastFM.ArtistGetTopTracks( m_ArtistName );
        int Index;
        int Count = TopTracks.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            guTrack * Track = Db->FindSong( m_ArtistName, TopTracks[ Index ].m_TrackName, 0, 0 );
            if( Track )
            {
                tracks->Add( Track );
            }
        }
    }
    else if( m_ContextMenuObject == m_SimArtistsStaticText )
    {
        // Similar Artists
        guSimilarArtistInfoArray SimilarArtists = LastFM.ArtistGetSimilar( m_ArtistName );
        wxArrayInt ArtistIds;
        int Index;
        int Count = SimilarArtists.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            int ArtistId = Db->FindArtist( SimilarArtists[ Index ].m_Name );
            if( ArtistId != wxNOT_FOUND )
            {
                ArtistIds.Empty();
                ArtistIds.Add( ArtistId );
                Db->GetArtistsSongs( ArtistIds, tracks );
            }
        }
    }
    else if( m_ContextMenuObject == m_SimTracksStaticText )
    {
        // Similar Tracks
        guSimilarTrackInfoArray SimilarTracks = LastFM.TrackGetSimilar( m_ArtistName, m_TrackName );
        int Index;
        int Count = SimilarTracks.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            guTrack * Track = Db->FindSong( SimilarTracks[ Index ].m_ArtistName, SimilarTracks[ Index ].m_TrackName, 0, 0 );
            if( Track )
            {
                tracks->Add( Track );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnPlayClicked( wxCommandEvent &event )
{
    guTrackArray Tracks;

    GetContextMenuTracks( &Tracks );

    if( m_PlayerPanel && Tracks.Count() )
    {
        m_PlayerPanel->SetPlayList( Tracks );
    }
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnEnqueueClicked( wxCommandEvent &event )
{
    guTrackArray Tracks;

    GetContextMenuTracks( &Tracks );

    if( m_PlayerPanel && Tracks.Count() )
    {
        m_PlayerPanel->AddToPlayList( Tracks, true, event.GetId() - ID_LASTFM_ENQUEUE_AFTER_ALL );
    }
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnSaveClicked( wxCommandEvent &event )
{
    guTrackArray Tracks;

    GetContextMenuTracks( &Tracks );

    int Index;
    int Count = Tracks.Count();
    if( Count )
    {
        wxArrayInt TrackIds;
        for( Index = 0; Index < Count; Index++ )
        {
            TrackIds.Add( Tracks[ Index ].m_SongId );
        }

        guListItems PlayLists;
        guDbLibrary * Db = m_Db ? m_Db : m_DefaultDb;
        Db->GetPlayLists( &PlayLists, guPLAYLIST_TYPE_STATIC );

        guPlayListAppend * PlayListAppendDlg = new guPlayListAppend( guMainFrame::GetMainFrame(), Db, &TrackIds, &PlayLists );

        if( PlayListAppendDlg->ShowModal() == wxID_OK )
        {
            int Selected = PlayListAppendDlg->GetSelectedPlayList();
            if( Selected == -1 )
            {
                wxString PLName = PlayListAppendDlg->GetPlaylistName();
                if( PLName.IsEmpty() )
                {
                    PLName = _( "Unnamed" );
                }
                Db->CreateStaticPlayList( PLName, TrackIds );
            }
            else
            {
                int PLId = PlayLists[ Selected ].m_Id;
                wxArrayInt OldSongs;
                Db->GetPlayListSongIds( PLId, &OldSongs );
                if( PlayListAppendDlg->GetSelectedPosition() == 0 ) // BEGIN
                {
                    Db->UpdateStaticPlayList( PLId, TrackIds );
                    Db->AppendStaticPlayList( PLId, OldSongs );
                }
                else                                                // END
                {
                    Db->AppendStaticPlayList( PLId, TrackIds );
                }
            }

            wxCommandEvent evt( wxEVT_MENU, ID_PLAYLIST_UPDATED );
            wxPostEvent( guMainFrame::GetMainFrame(), evt );
        }
        PlayListAppendDlg->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnCopyToClicked( wxCommandEvent &event )
{
    guTrackArray * Tracks = new guTrackArray();

    GetContextMenuTracks( Tracks );

    wxCommandEvent CmdEvent( wxEVT_MENU, ID_MAINFRAME_COPYTO );
    int Index = event.GetId() - ID_COPYTO_BASE;
    if( Index >= guCOPYTO_DEVICE_BASE )
    {
        Index -= guCOPYTO_DEVICE_BASE;
        CmdEvent.SetId( ID_MAINFRAME_COPYTODEVICE_TRACKS );
    }
    CmdEvent.SetInt( Index );
    CmdEvent.SetClientData( ( void * ) Tracks );
    wxPostEvent( guMainFrame::GetMainFrame(), CmdEvent );
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::SetMediaViewer( guMediaViewer * mediaviewer )
{
    m_MediaViewer = mediaviewer;
    m_Db = mediaviewer ? mediaviewer->GetDb() : NULL;
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::MediaViewerClosed( guMediaViewer * mediaviewer )
{
    //
    if( m_MediaViewer == mediaviewer )
    {
        guLogMessage( wxT( "Got MediaViewer Closed..." ) );
        SetMediaViewer( NULL );

        // Clear the LastFM controls to default values
        m_ArtistInfoCtrl->SetMediaViewer( NULL );

        //
        int Index;
        for( Index = 0; Index < GULASTFMINFO_MAXITEMS; Index++ )
        {
            m_AlbumsInfoCtrls[ Index ]->SetMediaViewer( NULL );
            m_TopTrackInfoCtrls[ Index ]->SetMediaViewer( NULL );
            m_SimArtistsInfoCtrls[ Index ]->SetMediaViewer( NULL );
            m_EventsInfoCtrls[ Index ]->SetMediaViewer( NULL );
            m_SimTracksInfoCtrls[ Index ]->SetMediaViewer( NULL  );
        }
    }
}




// -------------------------------------------------------------------------------- //
// guFetchLastFMInfoThread
// -------------------------------------------------------------------------------- //
guFetchLastFMInfoThread::guFetchLastFMInfoThread( guLastFMPanel * lastfmpanel ) :
    wxThread( wxTHREAD_DETACHED )
{
    m_LastFMPanel = lastfmpanel;
}

// -------------------------------------------------------------------------------- //
guFetchLastFMInfoThread::~guFetchLastFMInfoThread()
{
    int index;
    int count;
    m_DownloadThreadsMutex.Lock();
    if( ( count = m_DownloadThreads.Count() ) )
    {
        for( index = 0; index < count; index++ )
        {
            m_DownloadThreads[ index ]->Pause();
            m_DownloadThreads[ index ]->Delete();
        }
    }
    m_DownloadThreadsMutex.Unlock();
}

// -------------------------------------------------------------------------------- //
void guFetchLastFMInfoThread::WaitDownloadThreads( void )
{
    int count;
    while( !TestDestroy() )
    {
        m_DownloadThreadsMutex.Lock();
        count = m_DownloadThreads.Count();
        m_DownloadThreadsMutex.Unlock();
        if( !count )
            break;
        Sleep( GULASTFM_DOWNLOAD_IMAGE_DELAY );
    }
}


// -------------------------------------------------------------------------------- //
// guDownloadImageThread
// -------------------------------------------------------------------------------- //
guDownloadImageThread::guDownloadImageThread( guLastFMPanel * lastfmpanel,
    guFetchLastFMInfoThread * mainthread, guDbCache * dbcache, const int index, const wxChar * imageurl,
    int commandid, void * commanddata, wxImage ** pimage, const int imagesize ) :
    wxThread( wxTHREAD_DETACHED )
{
    m_DbCache     = dbcache;
    m_LastFMPanel = lastfmpanel;
    m_MainThread  = mainthread;
    m_CommandId   = commandid;
    m_CommandData = commanddata;
    m_pImage      = pimage;
    m_Index       = index;
    m_ImageUrl    = wxString( imageurl );
    m_ImageSize   = imagesize;
    // We dont need to lock here as its locked in the Mainthread when the thread is created
    m_MainThread->m_DownloadThreads.Add( this );

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guDownloadImageThread::~guDownloadImageThread()
{
    if( !TestDestroy() )
    {
        m_MainThread->m_DownloadThreadsMutex.Lock();
        if( !TestDestroy() )
        {
            m_MainThread->m_DownloadThreads.Remove( this );
        }
        m_MainThread->m_DownloadThreadsMutex.Unlock();
    }
}

// -------------------------------------------------------------------------------- //
guDownloadImageThread::ExitCode guDownloadImageThread::Entry()
{
    wxBitmapType ImageType;
    wxImage * Image = NULL;

    if( !TestDestroy() && !m_ImageUrl.IsEmpty() )
    {
        // We could be running while the database have been closed
        // in this case we are leaving the app so just leave thread
        try {
            Image = m_DbCache->GetImage( m_ImageUrl, ImageType, m_ImageSize );

            if( !TestDestroy() && !Image )
            {
                Image = guGetRemoteImage( m_ImageUrl, ImageType );

                if( Image )
                    m_DbCache->SetImage( m_ImageUrl, Image, ImageType );
                else
                    guLogMessage( wxT( "Could not get '%s'" ), m_ImageUrl.c_str() );

                if( !TestDestroy() && Image )
                {
                    int Size;
                    switch( m_ImageSize )
                    {
                        case guDBCACHE_TYPE_IMAGE_SIZE_TINY  :
                        {
                            Size = 50;
                            break;
                        }

                        case guDBCACHE_TYPE_IMAGE_SIZE_MID   :
                        {
                            Size = 100;
                            break;
                        }

                        default : //case guDBCACHE_TYPE_IMAGE_SIZE_BIG    :
                        {
                            Size = 150;
                            break;
                        }
                    }

                    guImageResize( Image, Size );
                }
            }

        }
        catch(...)
        {
            return 0;
        }
    }

    //
    if( !TestDestroy() )
    {
        if( m_pImage )
        {
            * m_pImage = Image;
        }
        //
        m_LastFMPanel->m_UpdateEventsMutex.Lock();
        wxCommandEvent event( wxEVT_MENU, m_CommandId );
        event.SetInt( m_Index );
        event.SetClientData( m_CommandData );
        wxPostEvent( m_LastFMPanel, event );
        m_LastFMPanel->m_UpdateEventsMutex.Unlock();
    }
    else
    {
        if( Image )
            delete Image;

        if( m_CommandData )
        {
            switch( m_CommandId )
            {
                case ID_LASTFM_UPDATE_ALBUMINFO :
                {
                    delete ( guLastFMAlbumInfo * ) m_CommandData;
                    break;
                }

                case ID_LASTFM_UPDATE_ARTISTINFO :
                {
                    delete ( guLastFMArtistInfo * ) m_CommandData;
                    break;
                }

                case ID_LASTFM_UPDATE_SIMARTIST :
                {
                    delete ( guLastFMSimilarArtistInfo * ) m_CommandData;
                    break;
                }

                case ID_LASTFM_UPDATE_EVENTINFO :
                {
                    delete ( guLastFMEventInfo * ) m_CommandData;
                    break;
                }

                case ID_LASTFM_UPDATE_TOPTRACKS :
                {
                    delete ( guLastFMTopTrackInfo * ) m_CommandData;
                    break;
                }

                case ID_LASTFM_UPDATE_SIMTRACK :
                {
                    delete ( guLastFMTrackInfo * ) m_CommandData;
                    break;
                }

            }
        }
    }
    return 0;

}

// -------------------------------------------------------------------------------- //
// guFetchAlbumInfoThread
// -------------------------------------------------------------------------------- //
guFetchAlbumInfoThread::guFetchAlbumInfoThread( guLastFMPanel * lastfmpanel,
           guDbCache * dbcache, const wxChar * artistname, const int start ) :
    guFetchLastFMInfoThread( lastfmpanel )
{
    m_DbCache = dbcache;
    m_ArtistName = wxString( artistname );
    m_Start = start * GULASTFMINFO_MAXITEMS;
    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guFetchAlbumInfoThread::~guFetchAlbumInfoThread()
{
    if( !TestDestroy() )
    {
        m_LastFMPanel->m_AlbumsUpdateThreadMutex.Lock();
        if( !TestDestroy() )
        {
            m_LastFMPanel->m_AlbumsUpdateThread = NULL;
        }
        m_LastFMPanel->m_AlbumsUpdateThreadMutex.Unlock();
    }
}

// -------------------------------------------------------------------------------- //
guFetchAlbumInfoThread::ExitCode guFetchAlbumInfoThread::Entry()
{
    guLastFM * LastFM = new guLastFM();
    if( LastFM )
    {
        if( !TestDestroy() )
        {
            //guLogMessage( wxT( "==== Getting Top Albums ====" ) );
            guAlbumInfoArray TopAlbums = LastFM->ArtistGetTopAlbums( m_ArtistName );
            int count = TopAlbums.Count();

            wxCommandEvent CountEvent( wxEVT_MENU, ID_LASTFM_UPDATE_ALBUM_COUNT );
            CountEvent.SetInt( count );
            wxPostEvent( m_LastFMPanel, CountEvent );

            if( count )
            {
                count = wxMin( count, m_Start + GULASTFMINFO_MAXITEMS );

                for( int index = m_Start; index < count; index++ )
                {
                    m_DownloadThreadsMutex.Lock();
                    if( !TestDestroy() )
                    {
                        guLastFMAlbumInfo * LastFMAlbumInfo = new guLastFMAlbumInfo( index - m_Start, NULL,
                              new guAlbumInfo( TopAlbums[ index ] ) );
                        if( LastFMAlbumInfo )
                        {
                            LastFMAlbumInfo->m_ImageUrl = TopAlbums[ index ].m_ImageLink;
                            guDownloadImageThread * DownloadImageThread = new guDownloadImageThread(
                                    m_LastFMPanel,
                                    this,
                                    m_DbCache,
                                    index - m_Start,
                                    TopAlbums[ index ].m_ImageLink.c_str(),
                                    ID_LASTFM_UPDATE_ALBUMINFO,
                                    LastFMAlbumInfo,
                                    &LastFMAlbumInfo->m_Image,
                                    guDBCACHE_TYPE_IMAGE_SIZE_TINY );
                            if( !DownloadImageThread )
                            {
                                guLogError( wxT( "Could not create the album image download thread %u" ), index );
                            }
                        }
                    }
                    m_DownloadThreadsMutex.Unlock();
                    if( TestDestroy() )
                        break;
                    Sleep( GULASTFM_DOWNLOAD_IMAGE_DELAY );
                }
            }
        }
        delete LastFM;
        //
        WaitDownloadThreads();
    }
    return 0;
}


// -------------------------------------------------------------------------------- //
// guFetchTopTracksInfoThread
// -------------------------------------------------------------------------------- //
guFetchTopTracksInfoThread::guFetchTopTracksInfoThread( guLastFMPanel * lastfmpanel,
           guDbCache * dbcache, const wxChar * artistname, const int startpage ) :
    guFetchLastFMInfoThread( lastfmpanel )
{
    m_DbCache = dbcache;
    m_ArtistName = wxString( artistname );
    m_Start = startpage * GULASTFMINFO_MAXITEMS;
    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guFetchTopTracksInfoThread::~guFetchTopTracksInfoThread()
{
    if( !TestDestroy() )
    {
        m_LastFMPanel->m_TopTracksUpdateThreadMutex.Lock();
        if( !TestDestroy() )
        {
            m_LastFMPanel->m_TopTracksUpdateThread = NULL;
        }
        m_LastFMPanel->m_TopTracksUpdateThreadMutex.Unlock();
    }
}

// -------------------------------------------------------------------------------- //
guFetchTopTracksInfoThread::ExitCode guFetchTopTracksInfoThread::Entry()
{
    int index;
    int count;
    guLastFM * LastFM = new guLastFM();
    if( LastFM )
    {
        if( !TestDestroy() )
        {
            //guLogMessage( wxT( "==== Getting Top Albums ====" ) );
            guTopTrackInfoArray TopTracks = LastFM->ArtistGetTopTracks( m_ArtistName );
            count = TopTracks.Count();

            wxCommandEvent CountEvent( wxEVT_MENU, ID_LASTFM_UPDATE_TOPTRACKS_COUNT );
            CountEvent.SetInt( count );
            wxPostEvent( m_LastFMPanel, CountEvent );

            if( count )
            {
                count = wxMin( count, m_Start + GULASTFMINFO_MAXITEMS );

                for( index = m_Start; index < count; index++ )
                {
                    m_DownloadThreadsMutex.Lock();
                    if( !TestDestroy() )
                    {
                        guLastFMTopTrackInfo * LastFMTopTrackInfo = new guLastFMTopTrackInfo( index - m_Start, NULL,
                              new guTopTrackInfo( TopTracks[ index ] ) );
                        if( LastFMTopTrackInfo )
                        {
                            LastFMTopTrackInfo->m_ImageUrl = TopTracks[ index ].m_ImageLink;
                            guDownloadImageThread * DownloadImageThread = new guDownloadImageThread(
                                    m_LastFMPanel,
                                    this,
                                    m_DbCache,
                                    index - m_Start,
                                    TopTracks[ index ].m_ImageLink.c_str(),
                                    ID_LASTFM_UPDATE_TOPTRACKS,
                                    LastFMTopTrackInfo,
                                    &LastFMTopTrackInfo->m_Image,
                                    guDBCACHE_TYPE_IMAGE_SIZE_TINY );
                            if( !DownloadImageThread )
                            {
                                guLogError( wxT( "Could not create the album image download thread %u" ), index );
                            }
                        }
                    }
                    m_DownloadThreadsMutex.Unlock();
                    if( TestDestroy() )
                        break;
                    Sleep( GULASTFM_DOWNLOAD_IMAGE_DELAY );
                }
            }
        }
        delete LastFM;
        //
        WaitDownloadThreads();
    }
    return 0;
}


// -------------------------------------------------------------------------------- //
// guFetchArtistInfoThread
// -------------------------------------------------------------------------------- //
guFetchArtistInfoThread::guFetchArtistInfoThread( guLastFMPanel * lastfmpanel,
                    guDbCache * dbcache, const wxChar * artistname ) :
    guFetchLastFMInfoThread( lastfmpanel )
{
    m_DbCache = dbcache;
    //guLogMessage( wxT( "guFetchArtistInfoThread : '%s'" ), artistname );
    m_ArtistName  = wxString( artistname );
    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guFetchArtistInfoThread::~guFetchArtistInfoThread()
{
    if( !TestDestroy() )
    {
        m_LastFMPanel->m_ArtistsUpdateThreadMutex.Lock();
        if( !TestDestroy() )
        {
            m_LastFMPanel->m_ArtistsUpdateThread = NULL;
        }
        m_LastFMPanel->m_ArtistsUpdateThreadMutex.Unlock();
    }
}

// -------------------------------------------------------------------------------- //
guFetchArtistInfoThread::ExitCode guFetchArtistInfoThread::Entry()
{
    guLastFM * LastFM = new guLastFM();
    if( LastFM )
    {
        if( !TestDestroy() )
        {
            // Get the Artist Info
            //guLogMessage( wxT( "==== Getting Artists Description ====" ) );
            guArtistInfo ArtistInfo = LastFM->ArtistGetInfo( m_ArtistName );

            m_DownloadThreadsMutex.Lock();
            if( !TestDestroy() )
            {
                guLastFMArtistInfo * LastFMArtistInfo = new guLastFMArtistInfo( 0, NULL,
                    new guArtistInfo( ArtistInfo ) );
                if( LastFMArtistInfo )
                {
                    LastFMArtistInfo->m_ImageUrl = ArtistInfo.m_ImageLink;
                    guDownloadImageThread * DownloadImageThread = new guDownloadImageThread(
                        m_LastFMPanel,
                        this,
                        m_DbCache,
                        0,
                        ArtistInfo.m_ImageLink.c_str(),
                        ID_LASTFM_UPDATE_ARTISTINFO,
                        LastFMArtistInfo,
                        &LastFMArtistInfo->m_Image,
                        guDBCACHE_TYPE_IMAGE_SIZE_MID );
                    if( !DownloadImageThread )
                    {
                        guLogError( wxT( "Could not create the artist image download thread" ) );
                    }
                }
            }
            m_DownloadThreadsMutex.Unlock();

        }
        delete LastFM;
        //
        WaitDownloadThreads();
    }
    return 0;
}


// -------------------------------------------------------------------------------- //
// guFetchSimilarArtistInfoThread
// -------------------------------------------------------------------------------- //
guFetchSimilarArtistInfoThread::guFetchSimilarArtistInfoThread( guLastFMPanel * lastfmpanel,
                    guDbCache * dbcache, const wxChar * artistname, const int startpage ) :
    guFetchLastFMInfoThread( lastfmpanel )
{
    m_DbCache = dbcache;
    //guLogMessage( wxT( "guFetchSimilarArtistInfoThread : '%s'" ), artistname );
    m_ArtistName  = wxString( artistname );
    m_Start = startpage * GULASTFMINFO_MAXITEMS;
    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guFetchSimilarArtistInfoThread::~guFetchSimilarArtistInfoThread()
{
    if( !TestDestroy() )
    {
        m_LastFMPanel->m_SimArtistsUpdateThreadMutex.Lock();
        if( !TestDestroy() )
        {
            m_LastFMPanel->m_SimArtistsUpdateThread = NULL;
        }
        m_LastFMPanel->m_SimArtistsUpdateThreadMutex.Unlock();
    }
}

// -------------------------------------------------------------------------------- //
guFetchSimilarArtistInfoThread::ExitCode guFetchSimilarArtistInfoThread::Entry()
{
    int index;
    int count;
    guLastFM * LastFM = new guLastFM();
    if( LastFM )
    {
        if( !TestDestroy() )
        {
            //guLogMessage( wxT( "==== Getting Similar Artists ====" ) );
            guSimilarArtistInfoArray SimilarArtists = LastFM->ArtistGetSimilar( m_ArtistName );
            count = SimilarArtists.Count();

            wxCommandEvent CountEvent( wxEVT_MENU, ID_LASTFM_UPDATE_SIMARTIST_COUNT );
            CountEvent.SetInt( count );
            wxPostEvent( m_LastFMPanel, CountEvent );

            if( count )
            {
                //guLogMessage( wxT( "Similar Artists: %u" ), count );
                count = wxMin( count, m_Start + GULASTFMINFO_MAXITEMS );

                for( index = m_Start; index < count; index++ )
                {
                    m_DownloadThreadsMutex.Lock();
                    if( !TestDestroy() )
                    {
                        guLastFMSimilarArtistInfo * LastFMArtistInfo = new guLastFMSimilarArtistInfo( index - m_Start, NULL,
                            new guSimilarArtistInfo( SimilarArtists[ index ] ) );
                        if( LastFMArtistInfo )
                        {
                            LastFMArtistInfo->m_ImageUrl = SimilarArtists[ index ].m_ImageLink;
                            guDownloadImageThread* DownloadImageThread = new guDownloadImageThread(
                                m_LastFMPanel,
                                this,
                                m_DbCache,
                                index - m_Start,
                                SimilarArtists[ index ].m_ImageLink.c_str(),
                                ID_LASTFM_UPDATE_SIMARTIST,
                                LastFMArtistInfo,
                                &LastFMArtistInfo->m_Image,
                                guDBCACHE_TYPE_IMAGE_SIZE_TINY );
                            if( !DownloadImageThread )
                            {
                                guLogError( wxT( "Could not create the similar artist image download thread %u" ), index );
                            }
                        }
                    }
                    m_DownloadThreadsMutex.Unlock();
                    if( TestDestroy() )
                        break;
                    Sleep( GULASTFM_DOWNLOAD_IMAGE_DELAY );
                }
            }
        }
        delete LastFM;
        //
        WaitDownloadThreads();
    }
    return 0;
}


// -------------------------------------------------------------------------------- //
// guFetchEventsInfoThread
// -------------------------------------------------------------------------------- //
guFetchEventsInfoThread::guFetchEventsInfoThread( guLastFMPanel * lastfmpanel,
                    guDbCache * dbcache, const wxChar * artistname, const int startpage ) :
    guFetchLastFMInfoThread( lastfmpanel )
{
    m_DbCache = dbcache;
    //guLogMessage( wxT( "guFetchEventsInfoThread : '%s'" ), artistname );
    m_ArtistName  = wxString( artistname );
    m_Start = startpage * GULASTFMINFO_MAXITEMS;
    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guFetchEventsInfoThread::~guFetchEventsInfoThread()
{
    if( !TestDestroy() )
    {
        m_LastFMPanel->m_EventsUpdateThreadMutex.Lock();
        if( !TestDestroy() )
        {
            m_LastFMPanel->m_EventsUpdateThread = NULL;
        }
        m_LastFMPanel->m_EventsUpdateThreadMutex.Unlock();
    }
}

// -------------------------------------------------------------------------------- //
guFetchEventsInfoThread::ExitCode guFetchEventsInfoThread::Entry()
{
    int index;
    int count;
    guLastFM * LastFM = new guLastFM();
    if( LastFM )
    {
        if( !TestDestroy() )
        {
            //guLogMessage( wxT( "==== Getting Artist Events ====" ) );
            guEventInfoArray ArtistEvents = LastFM->ArtistGetEvents( m_ArtistName );
            count = ArtistEvents.Count();

            wxCommandEvent CountEvent( wxEVT_MENU, ID_LASTFM_UPDATE_EVENTS_COUNT );
            CountEvent.SetInt( count );
            wxPostEvent( m_LastFMPanel, CountEvent );

            if( count )
            {
                //guLogMessage( wxT( "Similar Artists: %u" ), count );
                count = wxMin( count, m_Start + GULASTFMINFO_MAXITEMS );

                for( index = m_Start; index < count; index++ )
                {
                    m_DownloadThreadsMutex.Lock();
                    if( !TestDestroy() )
                    {
                        guLastFMEventInfo * LastFMEventInfo = new guLastFMEventInfo( index - m_Start, NULL,
                            new guEventInfo( ArtistEvents[ index ] ) );
                        if( LastFMEventInfo )
                        {
                            LastFMEventInfo->m_ImageUrl = ArtistEvents[ index ].m_ImageLink;
                            guDownloadImageThread * DownloadImageThread = new guDownloadImageThread(
                                m_LastFMPanel,
                                this,
                                m_DbCache,
                                index - m_Start,
                                ArtistEvents[ index ].m_ImageLink.c_str(),
                                ID_LASTFM_UPDATE_EVENTINFO,
                                LastFMEventInfo,
                                &LastFMEventInfo->m_Image,
                                guDBCACHE_TYPE_IMAGE_SIZE_TINY );
                            if( !DownloadImageThread )
                            {
                                guLogError( wxT( "Could not create the event image download thread %u" ), index );
                            }
                        }
                    }
                    m_DownloadThreadsMutex.Unlock();
                    if( TestDestroy() )
                        break;
                    Sleep( GULASTFM_DOWNLOAD_IMAGE_DELAY );
                }
            }

        }
        delete LastFM;
        //
        WaitDownloadThreads();
    }
    return 0;
}


// -------------------------------------------------------------------------------- //
// guFetchSimTracksInfoThread
// -------------------------------------------------------------------------------- //
guFetchSimTracksInfoThread::guFetchSimTracksInfoThread( guLastFMPanel * lastfmpanel,
         guDbCache * dbcache, const wxChar * artistname, const wxChar * trackname,
         const int startpage ) :
    guFetchLastFMInfoThread( lastfmpanel )
{
    m_DbCache = dbcache;
    m_ArtistName = wxString( artistname );
    m_TrackName  = wxString( trackname );
    m_Start = startpage * GULASTFMINFO_MAXITEMS;

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guFetchSimTracksInfoThread::~guFetchSimTracksInfoThread()
{
    if( !TestDestroy() )
    {
        m_LastFMPanel->m_SimTracksUpdateThreadMutex.Lock();
        if( !TestDestroy() )
        {
            m_LastFMPanel->m_SimTracksUpdateThread = NULL;
        }
        m_LastFMPanel->m_SimTracksUpdateThreadMutex.Unlock();
    }
}

// -------------------------------------------------------------------------------- //
guFetchSimTracksInfoThread::ExitCode guFetchSimTracksInfoThread::Entry()
{
    int index;
    int count;
    guLastFM * LastFM = new guLastFM();
    if( LastFM )
    {
        if( !TestDestroy() )
        {
            //guLogMessage( wxT( "==== Getting Similar Tracks ====" ) );
            guSimilarTrackInfoArray SimilarTracks = LastFM->TrackGetSimilar( m_ArtistName, m_TrackName );
            count = SimilarTracks.Count();

            wxCommandEvent CountEvent( wxEVT_MENU, ID_LASTFM_UPDATE_SIMTRACK_COUNT );
            CountEvent.SetInt( count );
            wxPostEvent( m_LastFMPanel, CountEvent );

            if( count )
            {
                //guLogMessage( wxT( "Similar Tracks: %u" ), count );
                count = wxMin( count, m_Start + GULASTFMINFO_MAXITEMS );

                for( index = m_Start; index < count; index++ )
                {
                    m_DownloadThreadsMutex.Lock();
                    if( !TestDestroy() )
                    {
                        guLastFMTrackInfo * LastFMTrackInfo = new guLastFMTrackInfo( index - m_Start, NULL,
                              new guSimilarTrackInfo( SimilarTracks[ index ] ) );
                        if( LastFMTrackInfo )
                        {
                            LastFMTrackInfo->m_ImageUrl = SimilarTracks[ index ].m_ImageLink;
                            guDownloadImageThread * DownloadImageThread = new guDownloadImageThread(
                                    m_LastFMPanel,
                                    this,
                                    m_DbCache,
                                    index - m_Start,
                                    SimilarTracks[ index ].m_ImageLink.c_str(),
                                    ID_LASTFM_UPDATE_SIMTRACK,
                                    LastFMTrackInfo,
                                    &LastFMTrackInfo->m_Image,
                                    guDBCACHE_TYPE_IMAGE_SIZE_TINY );
                            if( !DownloadImageThread )
                            {
                                guLogError( wxT( "Could not create the track image download thread %u" ), index );
                            }
                        }
                    }
                    m_DownloadThreadsMutex.Unlock();
                    if( TestDestroy() )
                        break;
                    Sleep( GULASTFM_DOWNLOAD_IMAGE_DELAY );
                }
            }
        }
        delete LastFM;
        //
        WaitDownloadThreads();
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
// guLastFMPanelDropTarget
// -------------------------------------------------------------------------------- //
guLastFMPanelDropTarget::guLastFMPanelDropTarget( guLastFMPanel * lastfmpanel )
{
    m_LastFMPanel = lastfmpanel;

    wxDataObjectComposite * DataObject = new wxDataObjectComposite();
    wxCustomDataObject * TracksDataObject = new wxCustomDataObject( wxDataFormat( wxT( "x-gutracks/guayadeque-copied-tracks" ) ) );
    DataObject->Add( TracksDataObject, true );
    wxFileDataObject * FileDataObject = new wxFileDataObject();
    DataObject->Add( FileDataObject, false );
    SetDataObject( DataObject );
}

// -------------------------------------------------------------------------------- //
guLastFMPanelDropTarget::~guLastFMPanelDropTarget()
{
}

// -------------------------------------------------------------------------------- //
wxDragResult guLastFMPanelDropTarget::OnData( wxCoord x, wxCoord y, wxDragResult def )
{
    //guLogMessage( wxT( "guListViewDropTarget::OnData" ) );

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
            m_LastFMPanel->OnDropFiles( Tracks );
        }
    }
    else if( ReceivedFormat == wxDataFormat( wxDF_FILENAME ) )
    {
        wxFileDataObject * FileDataObject = ( wxFileDataObject * ) DataObject->GetDataObject( wxDataFormat( wxDF_FILENAME ) );
        if( FileDataObject )
        {
            m_LastFMPanel->OnDropFiles( FileDataObject->GetFilenames() );
        }
    }

    return def;
}

}

// -------------------------------------------------------------------------------- //
