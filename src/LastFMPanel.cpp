// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2010 J.Rios
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
#include "LastFMPanel.h"

#include "Commands.h"
#include "curl/http.h"
#include "Images.h"
#include "MainApp.h"
#include "ShowImage.h"
#include "TagInfo.h"
#include "Utils.h"
#include "AuiNotebook.h"

#include <wx/arrimpl.cpp>
#include "wx/clipbrd.h"
#include <wx/statline.h>
#include <wx/uri.h>

#define GULASTFM_TITLE_FONT_SIZE 12

#define GULASTFM_DOWNLOAD_IMAGE_DELAY    10

WX_DEFINE_OBJARRAY(guLastFMInfoArray);
WX_DEFINE_OBJARRAY(guLastFMSimilarArtistInfoArray);
WX_DEFINE_OBJARRAY(guLastFMTrackInfoArray);
WX_DEFINE_OBJARRAY(guLastFMAlbumInfoArray);

// -------------------------------------------------------------------------------- //
// guLastFMInfoCtrl
// -------------------------------------------------------------------------------- //
guLastFMInfoCtrl::guLastFMInfoCtrl( wxWindow * parent, guDbLibrary * db, guDbCache * dbcache, guPlayerPanel * playerpanel, bool createcontrols ) :
    wxPanel( parent, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), wxTAB_TRAVERSAL )
{
    m_Db = db;
    m_DbCache = dbcache;
    m_PlayerPanel = playerpanel;
    m_NormalColor = wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT );
    m_NotFoundColor = wxSystemSettings::GetColour( wxSYS_COLOUR_GRAYTEXT );

    if( createcontrols )
        this->CreateControls( parent );


    Connect( ID_LASTFM_SEARCH_LINK, ID_LASTFM_SEARCH_LINK + 999, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLastFMInfoCtrl::OnSearchLinkClicked ) );
    Connect( ID_LASTFM_VISIT_URL, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLastFMInfoCtrl::OnSearchLinkClicked ) );

    Connect( wxEVT_CONTEXT_MENU, wxContextMenuEventHandler( guLastFMInfoCtrl::OnContextMenu ), NULL, this );
    Connect( ID_LASTFM_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLastFMInfoCtrl::OnPlayClicked ), NULL, this );
    Connect( ID_LASTFM_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLastFMInfoCtrl::OnEnqueueClicked ), NULL, this );
    Connect( ID_LASTFM_ENQUEUE_ASNEXT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLastFMInfoCtrl::OnEnqueueAsNextClicked ), NULL, this );
    Connect( ID_LASTFM_COPYTOCLIPBOARD, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLastFMInfoCtrl::OnCopyToClipboard ), NULL, this );
    Connect( ID_SONG_SELECTNAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLastFMInfoCtrl::OnSongSelectName ), NULL, this );
    Connect( ID_ARTIST_SELECTNAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLastFMInfoCtrl::OnArtistSelectName ), NULL, this );
    Connect( ID_ALBUM_SELECTNAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLastFMInfoCtrl::OnAlbumSelectName ), NULL, this );

    Connect( wxEVT_MOTION, wxMouseEventHandler( guLastFMInfoCtrl::OnMouse ), NULL, this );
    Connect( wxEVT_ENTER_WINDOW, wxMouseEventHandler( guLastFMInfoCtrl::OnMouse ), NULL, this );
    Connect( wxEVT_LEAVE_WINDOW, wxMouseEventHandler( guLastFMInfoCtrl::OnMouse ), NULL, this );
    Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( guLastFMInfoCtrl::OnMouse ), NULL, this );

    Connect( guEVT_STATICBITMAP_MOUSE_OVER, guStaticBitmapMouseOverEvent, wxCommandEventHandler( guLastFMInfoCtrl::OnBitmapMouseOver ), NULL, this );
}

// -------------------------------------------------------------------------------- //
guLastFMInfoCtrl::~guLastFMInfoCtrl()
{
    Disconnect( ID_LASTFM_SEARCH_LINK, ID_LASTFM_SEARCH_LINK + 999, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLastFMInfoCtrl::OnSearchLinkClicked ) );
    Disconnect( ID_LASTFM_VISIT_URL, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLastFMInfoCtrl::OnSearchLinkClicked ) );

    Disconnect( wxEVT_CONTEXT_MENU, wxContextMenuEventHandler( guLastFMInfoCtrl::OnContextMenu ), NULL, this );
    Disconnect( ID_LASTFM_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLastFMInfoCtrl::OnPlayClicked ), NULL, this );
    Disconnect( ID_LASTFM_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLastFMInfoCtrl::OnEnqueueClicked ), NULL, this );
    Disconnect( ID_LASTFM_ENQUEUE_ASNEXT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLastFMInfoCtrl::OnEnqueueAsNextClicked ), NULL, this );
    Disconnect( ID_LASTFM_COPYTOCLIPBOARD, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLastFMInfoCtrl::OnCopyToClipboard ), NULL, this );
    Disconnect( ID_SONG_SELECTNAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLastFMInfoCtrl::OnSongSelectName ), NULL, this );
    Disconnect( ID_ARTIST_SELECTNAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLastFMInfoCtrl::OnArtistSelectName ), NULL, this );
    Disconnect( ID_ALBUM_SELECTNAME, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLastFMInfoCtrl::OnAlbumSelectName ), NULL, this );

    Disconnect( wxEVT_MOTION, wxMouseEventHandler( guLastFMInfoCtrl::OnMouse ), NULL, this );
    Disconnect( wxEVT_ENTER_WINDOW, wxMouseEventHandler( guLastFMInfoCtrl::OnMouse ), NULL, this );
    Disconnect( wxEVT_LEAVE_WINDOW, wxMouseEventHandler( guLastFMInfoCtrl::OnMouse ), NULL, this );
    Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( guLastFMInfoCtrl::OnMouse ), NULL, this );
    m_Text->Disconnect( wxEVT_MOTION, wxMouseEventHandler( guLastFMInfoCtrl::OnMouse ), NULL, this );
    m_Text->Disconnect( wxEVT_ENTER_WINDOW, wxMouseEventHandler( guLastFMInfoCtrl::OnMouse ), NULL, this );
    m_Text->Disconnect( wxEVT_LEAVE_WINDOW, wxMouseEventHandler( guLastFMInfoCtrl::OnMouse ), NULL, this );
    m_Text->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( guLastFMInfoCtrl::OnMouse ), NULL, this );

    Disconnect( guEVT_STATICBITMAP_MOUSE_OVER, guStaticBitmapMouseOverEvent, wxCommandEventHandler( guLastFMInfoCtrl::OnBitmapMouseOver ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guLastFMInfoCtrl::CreateControls( wxWindow * parent )
{
	wxBoxSizer* MainSizer;
	MainSizer = new wxBoxSizer( wxHORIZONTAL );

	m_Bitmap = new guStaticBitmap( this, wxID_ANY, guImage( guIMAGE_INDEX_default_lastfm_image ),
	                                            wxDefaultPosition, wxSize( 50, 50 ), 0 );
    //Bitmap->SetCursor( wxCURSOR_HAND );
	MainSizer->Add( m_Bitmap, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );

	m_Text = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 200, -1 ), 0 );
	m_Text->Wrap( -1 );
	//Text->SetCursor( wxCursor( wxCURSOR_HAND ) );
	//m_Text->SetMaxSize( wxSize( 250, -1 ) );
	MainSizer->Add( m_Text, 1, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 2 );


	SetSizer( MainSizer );
	Layout();
	MainSizer->Fit( this );

//    Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guLastFMInfoCtrl::OnDoubleClicked ), NULL, this );
//    m_Bitmap->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guLastFMInfoCtrl::OnDoubleClicked ), NULL, this );
    m_Text->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guLastFMInfoCtrl::OnDoubleClicked ), NULL, this );
    m_Text->Connect( wxEVT_MOTION, wxMouseEventHandler( guLastFMInfoCtrl::OnMouse ), NULL, this );
    m_Text->Connect( wxEVT_ENTER_WINDOW, wxMouseEventHandler( guLastFMInfoCtrl::OnMouse ), NULL, this );
    m_Text->Connect( wxEVT_LEAVE_WINDOW, wxMouseEventHandler( guLastFMInfoCtrl::OnMouse ), NULL, this );
    m_Text->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( guLastFMInfoCtrl::OnMouse ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guLastFMInfoCtrl::Clear( void )
{
    m_Bitmap->SetBitmap( guImage( guIMAGE_INDEX_default_lastfm_image ) );
    m_Text->SetLabel( wxEmptyString );
}

// -------------------------------------------------------------------------------- //
void guLastFMInfoCtrl::SetBitmap( const wxImage * image )
{
    if( image )
    {
        //m_Bitmap->SetBitmap( wxBitmap( image->Copy() ) );
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
    int index;
    int count;
    wxMenuItem * MenuItem;
    if( Menu )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        wxArrayString Links = Config->ReadAStr( wxT( "Link" ), wxEmptyString, wxT( "SearchLinks" ) );
        wxArrayString Names = Config->ReadAStr( wxT( "Name" ), wxEmptyString, wxT( "SearchLinks" ) );
        if( ( count = Links.Count() ) )
        {
            for( index = 0; index < count; index++ )
            {
                wxURI Uri( Links[ index ] );
                MenuItem = new wxMenuItem( Menu, ID_LASTFM_SEARCH_LINK + index, Names[ index ], Links[ index ] );
                wxString IconFile = wxGetHomeDir() + wxT( "/.guayadeque/LinkIcons/" ) + Uri.GetServer() + wxT( ".ico" );
                if( wxFileExists( IconFile ) )
                {
                    MenuItem->SetBitmap( wxBitmap( IconFile, wxBITMAP_TYPE_ICO ) );
                }
                else
                {
                    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_search ) );
                }
                Menu->Append( MenuItem );
            }
        }
        else
        {
            MenuItem = new wxMenuItem( Menu, -1, _( "No search link defined" ), _( "Add search links in preferences" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_search ) );
            Menu->Append( MenuItem );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guLastFMInfoCtrl::OnDoubleClicked( wxMouseEvent &event )
{
    guTrackArray Tracks;
    GetSelectedTracks( &Tracks );
    if( Tracks.Count() )
    {
        guConfig * Config = ( guConfig * ) Config->Get();
        if( m_PlayerPanel && Config )
        {
            if( Config->ReadBool( wxT( "DefaultActionEnqueue" ), false, wxT( "General" ) ) )
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
    int index = event.GetId();
    if( index == ID_LASTFM_VISIT_URL )
    {
        guWebExecute( GetItemUrl() );
        return;
    }

    guConfig * Config = ( guConfig * ) Config->Get();
    if( Config )
    {
        wxArrayString Links = Config->ReadAStr( wxT( "Link" ), wxEmptyString, wxT( "SearchLinks" ) );
        wxASSERT( Links.Count() > 0 );

        index -= ID_LASTFM_SEARCH_LINK;
        wxString SearchLink = Links[ index ];
        wxString Lang = Config->ReadStr( wxT( "Language" ), wxT( "en" ), wxT( "LastFM" ) );
        if( Lang.IsEmpty() )
        {
            Lang = ( ( guMainApp * ) wxTheApp )->GetLocale()->GetCanonicalName().Mid( 0, 2 );
            //guLogMessage( wxT( "Locale: %s" ), ( ( guMainApp * ) wxTheApp )->GetLocale()->GetCanonicalName().c_str() );
        }
        SearchLink.Replace( wxT( "{lang}" ), Lang );
        SearchLink.Replace( wxT( "{text}" ), guURLEncode( GetSearchText() ) );
        guWebExecute( SearchLink );
    }
    //guLogMessage( wxT( "Search Link %i Clicked" ), index );
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
wxString guLastFMInfoCtrl::GetSearchText( void )
{
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
wxString guLastFMInfoCtrl::GetItemUrl( void )
{
    return wxEmptyString;
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
        m_PlayerPanel->AddToPlayList( Tracks );
    }
}

// -------------------------------------------------------------------------------- //
void guLastFMInfoCtrl::OnEnqueueAsNextClicked( wxCommandEvent &event )
{
    guTrackArray Tracks;
    GetSelectedTracks( &Tracks );
    if( m_PlayerPanel && Tracks.Count() )
    {
        m_PlayerPanel->AddToPlayList( Tracks, true, true );
    }
}

// -------------------------------------------------------------------------------- //
int guLastFMInfoCtrl::GetSelectedTracks( guTrackArray * tracks )
{
    return 0;
}

// -------------------------------------------------------------------------------- //
void guLastFMInfoCtrl::OnSongSelectName( wxCommandEvent &event )
{
}

// -------------------------------------------------------------------------------- //
void guLastFMInfoCtrl::OnArtistSelectName( wxCommandEvent &event )
{
}

// -------------------------------------------------------------------------------- //
void guLastFMInfoCtrl::OnAlbumSelectName( wxCommandEvent &event )
{
}

// -------------------------------------------------------------------------------- //
void guLastFMInfoCtrl::OnBitmapMouseOver( wxCommandEvent &event )
{
    int ImageType;
    wxString ImageUrl = GetBitmapImageUrl();
    if( !ImageUrl.IsEmpty() )
    {
        wxImage * Image = m_DbCache->GetImage( ImageUrl, ImageType, guDBCACHE_IMAGE_SIZE_BIG );
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

    guConfig * Config = ( guConfig * ) Config->Get();
    m_ShowLongBioText = Config->ReadBool( wxT( "LFMShowLongBioText" ), false, wxT( "General" )  );

    CreateControls( parent );

	m_ShowMoreHyperLink->Connect( wxEVT_COMMAND_HYPERLINK, wxHyperlinkEventHandler( guArtistInfoCtrl::OnShowMoreLinkClicked ), NULL, this );
	m_ArtistDetails->Connect( wxEVT_COMMAND_HTML_LINK_CLICKED, wxHtmlLinkEventHandler( guArtistInfoCtrl::OnHtmlLinkClicked ), NULL, this );
};

// -------------------------------------------------------------------------------- //
guArtistInfoCtrl::~guArtistInfoCtrl()
{
    if( m_Info )
        delete m_Info;

    guConfig * Config = ( guConfig * ) Config->Get();
    Config->WriteBool( wxT( "LFMShowLongBioText" ), m_ShowLongBioText, wxT( "General" )  );

	m_ShowMoreHyperLink->Disconnect( wxEVT_COMMAND_HYPERLINK, wxHyperlinkEventHandler( guArtistInfoCtrl::OnShowMoreLinkClicked ), NULL, this );
	m_ArtistDetails->Disconnect( wxEVT_COMMAND_HTML_LINK_CLICKED, wxHtmlLinkEventHandler( guArtistInfoCtrl::OnHtmlLinkClicked ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guArtistInfoCtrl::CreateControls( wxWindow * parent )
{
	m_MainSizer = new wxBoxSizer( wxHORIZONTAL );

	m_Bitmap = new guStaticBitmap( this, wxID_ANY, guImage( guIMAGE_INDEX_no_photo ), wxDefaultPosition, wxSize( 100,100 ), 0 );
	m_MainSizer->Add( m_Bitmap, 0, wxALL, 5 );

//	wxBoxSizer * DetailSizer;
	m_DetailSizer = new wxBoxSizer( wxVERTICAL );

	wxSizer * TopSizer = new wxBoxSizer( wxHORIZONTAL );

	m_Text = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_Text->Wrap( -1 );
	wxFont CurrentFont = wxSystemSettings::GetFont( wxSYS_SYSTEM_FONT );
	CurrentFont.SetPointSize( 12 );
	CurrentFont.SetWeight( wxFONTWEIGHT_BOLD );
	m_Text->SetFont( CurrentFont );

	TopSizer->Add( m_Text, 0, wxEXPAND|wxTOP|wxRIGHT|wxLEFT, 5 );

	TopSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	m_ShowMoreHyperLink = new wxHyperlinkCtrl( this, wxID_ANY, m_ShowLongBioText ? _( "Less..." ) : _("More..."), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
	m_ShowMoreHyperLink->SetNormalColour( wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHT ) );
	m_ShowMoreHyperLink->SetVisitedColour( wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHT ) );
	m_ShowMoreHyperLink->SetHoverColour( wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHT ) );
	//m_DetailSizer->Add( m_ShowMoreHyperLink, 0, wxALL|wxALIGN_RIGHT, 5 );

	TopSizer->Add( m_ShowMoreHyperLink, 0, wxALIGN_RIGHT|wxTOP|wxRIGHT|wxLEFT, 5 );


	//m_DetailSizer->Add( m_Text, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	m_DetailSizer->Add( TopSizer, 0, wxEXPAND, 5 );

	m_ArtistDetails = new wxHtmlWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_AUTO );
	m_ArtistDetails->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT ) );
	CurrentFont.SetPointSize( 10 );
	CurrentFont.SetWeight( wxFONTWEIGHT_NORMAL );
	m_ArtistDetails->SetFonts( CurrentFont.GetFaceName(), wxEmptyString );

	m_ArtistDetails->SetBackgroundColour( m_Text->GetBackgroundColour() );
	m_ArtistDetails->SetBorders( 0 );
	m_DetailSizer->Add( m_ArtistDetails, 1, wxALL|wxEXPAND, 5 );

	m_MainSizer->Add( m_DetailSizer, 1, wxEXPAND, 5 );

	SetSizer( m_MainSizer );
	Layout();
	//MainSizer->Fit( this );

    m_Text->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guArtistInfoCtrl::OnDoubleClicked ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guArtistInfoCtrl::SetInfo( guLastFMArtistInfo * info )
{
    if( m_Info )
        delete m_Info;
    m_Info = info;


    m_Info->m_ArtistId = m_Db->FindArtist( m_Info->m_Artist->m_Name );

    m_Text->SetForegroundColour( m_Info->m_ArtistId == wxNOT_FOUND ?
                                        m_NotFoundColor : m_NormalColor );
    SetBitmap( m_Info->m_Image );
    SetLabel( m_Info->m_Artist->m_Name );

    UpdateArtistInfoText();
}

// -------------------------------------------------------------------------------- //
void guArtistInfoCtrl::Clear( void )
{
    m_Bitmap->SetBitmap( guImage( guIMAGE_INDEX_no_photo ) );
    m_Text->SetLabel( wxEmptyString );
    if( m_Info )
        delete m_Info;
    m_Info = NULL;
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
        guLogMessage( wxT( "The artist id = %i" ), m_Info->m_ArtistId );
        MenuItem = new wxMenuItem( Menu, ID_LASTFM_PLAY, _( "Play" ), _( "Play the artist tracks" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_LASTFM_ENQUEUE, _( "Enqueue" ), _( "Enqueue the artist tracks to the playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_LASTFM_ENQUEUE_ASNEXT, _( "Enqueue Next" ), _( "Enqueue the artist tracks to the playlist as Next Tracks" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_ARTIST_SELECTNAME, _( "Search Artist" ), _( "Search the artist in the library" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_search ) );
        Menu->Append( MenuItem );
        Menu->AppendSeparator();
    }

    if( !GetSearchText().IsEmpty() )
    {
        MenuItem = new wxMenuItem( Menu, ID_LASTFM_COPYTOCLIPBOARD, wxT( "Copy to clipboard" ), _( "Copy the artist name to clipboard" ) );
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

    m_ArtistDetails->SetPage( wxString::Format( wxT( "<html><body bgcolor=%s text=%s link=%s>%s</body></html>" ),
          //wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWFRAME ).GetAsString( wxC2S_HTML_SYNTAX ).c_str(), //wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWFRAME ).GetAsString( wxC2S_HTML_SYNTAX ).c_str(),
          m_Text->GetBackgroundColour().GetAsString( wxC2S_HTML_SYNTAX ).c_str(),
          wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT ).GetAsString( wxC2S_HTML_SYNTAX ).c_str(),
          wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHT ).GetAsString( wxC2S_HTML_SYNTAX ).c_str(),
          Content.c_str() ) );

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
        return m_Db->GetArtistsSongs( Selections, tracks );
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
void guArtistInfoCtrl::OnArtistSelectName( wxCommandEvent &event )
{
    wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_MAINFRAME_SELECT_ARTIST );
    evt.SetInt( m_Info->m_ArtistId );
    evt.SetExtraLong( guTRACK_TYPE_DB );
    wxPostEvent( wxTheApp->GetTopWindow(), evt );
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

    m_Info->m_AlbumId = m_Db->FindAlbum( m_Info->m_Album->m_Artist, m_Info->m_Album->m_Name );
    m_Text->SetForegroundColour( m_Info->m_AlbumId == wxNOT_FOUND ?
                                        m_NotFoundColor : m_NormalColor );

    SetBitmap( m_Info->m_Image );
    SetLabel( m_Info->m_Album->m_Name );
}

// -------------------------------------------------------------------------------- //
void guAlbumInfoCtrl::Clear( void )
{
    guLastFMInfoCtrl::Clear();
    if( m_Info )
        delete m_Info;
    m_Info = NULL;
}

//// -------------------------------------------------------------------------------- //
//void guAlbumInfoCtrl::OnClick( wxMouseEvent &event )
//{
//    if( m_Info )
//    {
//        //guLogMessage( wxT( "guAlbumInfo::OnClick %s" ), Info->Album->Url.c_str() );
//        guWebExecute( m_Info->m_Album->m_Url );
//    }
//}

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

        MenuItem = new wxMenuItem( Menu, ID_LASTFM_ENQUEUE, _( "Enqueue" ), _( "Enqueue the album tracks to the playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_LASTFM_ENQUEUE_ASNEXT, _( "Enqueue Next" ), _( "Enqueue the artist tracks to the playlist as Next Track" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_ALBUM_SELECTNAME, _( "Search Album" ), _( "Search the album in the library" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_search ) );
        Menu->Append( MenuItem );
        Menu->AppendSeparator();
    }

    if( !GetSearchText().IsEmpty() )
    {
        MenuItem = new wxMenuItem( Menu, ID_LASTFM_COPYTOCLIPBOARD, wxT( "Copy to clipboard" ), _( "Copy the album info to clipboard" ) );
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
        return m_Db->GetAlbumsSongs( Selections, tracks );
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
void guAlbumInfoCtrl::OnAlbumSelectName( wxCommandEvent &event )
{
    wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_MAINFRAME_SELECT_ALBUM );
    evt.SetInt( m_Info->m_AlbumId );
    evt.SetExtraLong( guTRACK_TYPE_DB );
    wxPostEvent( wxTheApp->GetTopWindow(), evt );
}

// -------------------------------------------------------------------------------- //
// guSimilarArtistInfoCtrl
// -------------------------------------------------------------------------------- //
guSimilarArtistInfoCtrl::guSimilarArtistInfoCtrl( wxWindow * parent, guDbLibrary * db,
        guDbCache * dbcache, guPlayerPanel * playerpanel ) :
            guLastFMInfoCtrl( parent, db, dbcache, playerpanel )
{
    m_Info = NULL;

    Connect( ID_LASTFM_SELECT_ARTIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guSimilarArtistInfoCtrl::OnSelectArtist ), NULL, this );
}

// -------------------------------------------------------------------------------- //
guSimilarArtistInfoCtrl::~guSimilarArtistInfoCtrl()
{
    if( m_Info )
        delete m_Info;

    Disconnect( ID_LASTFM_SELECT_ARTIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guSimilarArtistInfoCtrl::OnSelectArtist ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guSimilarArtistInfoCtrl::SetInfo( guLastFMSimilarArtistInfo * info )
{
    if( m_Info )
        delete m_Info;
    m_Info = info;

    m_Info->m_ArtistId = m_Db->FindArtist( m_Info->m_Artist->m_Name );
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
void guSimilarArtistInfoCtrl::Clear( void )
{
    guLastFMInfoCtrl::Clear();
    if( m_Info )
        delete m_Info;
    m_Info = NULL;
}

//// -------------------------------------------------------------------------------- //
//void guSimilarArtistInfoCtrl::OnClick( wxMouseEvent &event )
//{
//    if( m_Info )
//    {
//        //guLogMessage( wxT( "guArtistInfo::OnClick %s" ), Info->Artist->Url.c_str() );
//        guWebExecute( m_Info->m_Artist->m_Url );
//    }
//}

// -------------------------------------------------------------------------------- //
void guSimilarArtistInfoCtrl::OnSelectArtist( wxCommandEvent &event )
{
    guLastFMPanel * LastFMPanel = ( guLastFMPanel * ) GetParent();
    LastFMPanel->SetUpdateEnable( false );

    guTrackChangeInfo TrackChangeInfo( GetSearchText(), wxEmptyString );

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

        MenuItem = new wxMenuItem( Menu, ID_LASTFM_ENQUEUE, _( "Enqueue" ), _( "Enqueue the artist tracks to the playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_LASTFM_ENQUEUE_ASNEXT, _( "Enqueue Next" ), _( "Enqueue the artist tracks to the playlist as Next Track" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_ARTIST_SELECTNAME, _( "Search Artist" ), _( "Search the artist in the library" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_search ) );
        Menu->Append( MenuItem );
        Menu->AppendSeparator();
    }

    if( !GetSearchText().IsEmpty() )
    {
        MenuItem = new wxMenuItem( Menu, ID_LASTFM_SELECT_ARTIST, wxT( "Show artist info" ), _( "Update the information with the current selected artist" ) );
        Menu->Append( MenuItem );
        MenuItem = new wxMenuItem( Menu, ID_LASTFM_COPYTOCLIPBOARD, wxT( "Copy to clipboard" ), _( "Copy the artist info to clipboard" ) );
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
        return m_Db->GetArtistsSongs( Selections, tracks );
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
void guSimilarArtistInfoCtrl::OnArtistSelectName( wxCommandEvent &event )
{
    wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_MAINFRAME_SELECT_ARTIST );
    evt.SetInt( m_Info->m_ArtistId );
    evt.SetExtraLong( guTRACK_TYPE_DB );
    wxPostEvent( wxTheApp->GetTopWindow(), evt );
}

// -------------------------------------------------------------------------------- //
// guTrackInfoCtrl
// -------------------------------------------------------------------------------- //
guTrackInfoCtrl::guTrackInfoCtrl( wxWindow * parent, guDbLibrary * db, guDbCache * dbcache, guPlayerPanel * playerpanel ) :
                 guLastFMInfoCtrl( parent, db, dbcache, playerpanel )
{
    m_Info = NULL;

    Connect( ID_LASTFM_SELECT_ARTIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTrackInfoCtrl::OnSelectArtist ), NULL, this );
}

// -------------------------------------------------------------------------------- //
guTrackInfoCtrl::~guTrackInfoCtrl()
{
    if( m_Info )
        delete m_Info;

    Disconnect( ID_LASTFM_SELECT_ARTIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTrackInfoCtrl::OnSelectArtist ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guTrackInfoCtrl::SetInfo( guLastFMTrackInfo * info )
{
    if( m_Info )
        delete m_Info;
    m_Info = info;

    m_Info->m_TrackId = m_Db->FindTrack( m_Info->m_Track->m_ArtistName, m_Info->m_Track->m_TrackName );
    m_Info->m_ArtistId = m_Db->FindArtist( m_Info->m_Track->m_ArtistName );
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
void guTrackInfoCtrl::Clear( void )
{
    guLastFMInfoCtrl::Clear();
    if( m_Info )
        delete m_Info;
    m_Info = NULL;
}

// -------------------------------------------------------------------------------- //
void guTrackInfoCtrl::OnSelectArtist( wxCommandEvent &event )
{
    guLastFMPanel * LastFMPanel = ( guLastFMPanel * ) GetParent();
    LastFMPanel->SetUpdateEnable( false );

    guTrackChangeInfo TrackChangeInfo( m_Info->m_Track->m_ArtistName, wxEmptyString );

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

        MenuItem = new wxMenuItem( Menu, ID_LASTFM_ENQUEUE, _( "Enqueue" ), _( "Enqueue the artist tracks to the playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_LASTFM_ENQUEUE_ASNEXT, _( "Enqueue Next" ), _( "Enqueue the artist tracks to the playlist as Next Tracks" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_SONG_SELECTNAME, _( "Search Track" ), _( "Search the track in the library" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_search ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_ARTIST_SELECTNAME, _( "Search Artist" ), _( "Search the artist in the library" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_search ) );
        Menu->Append( MenuItem );
        Menu->AppendSeparator();
    }

    if( !GetSearchText().IsEmpty() )
    {
        MenuItem = new wxMenuItem( Menu, ID_LASTFM_SELECT_ARTIST, wxT( "Show artist info" ), _( "Update the information with the current selected artist" ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_LASTFM_COPYTOCLIPBOARD, wxT( "Copy to clipboard" ), _( "Copy the track info to clipboard" ) );
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
        return m_Db->GetSongs( Selections, tracks );
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
void guTrackInfoCtrl::OnSongSelectName( wxCommandEvent &event )
{
    wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_MAINFRAME_SELECT_TRACK );
    evt.SetInt( m_Info->m_TrackId );
    evt.SetExtraLong( guTRACK_TYPE_DB );
    wxPostEvent( wxTheApp->GetTopWindow(), evt );
}

// -------------------------------------------------------------------------------- //
void guTrackInfoCtrl::OnArtistSelectName( wxCommandEvent &event )
{
    wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_MAINFRAME_SELECT_ARTIST );
    evt.SetInt( m_Info->m_ArtistId );
    evt.SetExtraLong( guTRACK_TYPE_DB );
    wxPostEvent( wxTheApp->GetTopWindow(), evt );
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
void guEventInfoCtrl::Clear( void )
{
    guLastFMInfoCtrl::Clear();
    if( m_Info )
        delete m_Info;
    m_Info = NULL;
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
        MenuItem = new wxMenuItem( Menu, ID_LASTFM_COPYTOCLIPBOARD, wxT( "Copy to clipboard" ), _( "Copy the event info to clipboard" ) );
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
    wxScrolledWindow( Parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL )
{
    m_Db = db;
    m_DbCache = dbcache;
    m_PlayerPanel = playerpanel;
    m_UpdateEnabled = true;

    m_CurrentTrackInfo = wxNOT_FOUND;

    m_ShowArtistDetails = true;
    m_ShowAlbums = true;
    m_ShowArtists = true;
    m_ShowTracks = true;
    m_ShowEvents = true;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        m_ShowArtistDetails = Config->ReadBool( wxT( "LFMShowArtistInfo" ), true, wxT( "General" )  );
        m_ShowAlbums = Config->ReadBool( wxT( "LFMShowAlbums" ), true, wxT( "General" )  );
        m_ShowArtists = Config->ReadBool( wxT( "LFMShowArtists" ), true, wxT( "General" )  );
        m_ShowTracks = Config->ReadBool( wxT( "LFMShowTracks" ), true, wxT( "General" )  );
        m_ShowEvents = Config->ReadBool( wxT( "LFMShowEvents" ), true, wxT( "General" )  );
    }

    wxFont CurrentFont = wxSystemSettings::GetFont( wxSYS_SYSTEM_FONT );

	m_MainSizer = new wxBoxSizer( wxVERTICAL );

    // Manual Artist Editor
	wxBoxSizer * EditorSizer;
	EditorSizer = new wxBoxSizer( wxHORIZONTAL );

	m_UpdateCheckBox = new wxCheckBox( this, wxID_ANY, _( "Follow player" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_UpdateCheckBox->SetValue( Config->ReadBool( wxT( "LFMFollowPlayer" ), true, wxT( "General" ) ) );

	EditorSizer->Add( m_UpdateCheckBox, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

	m_PrevButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_left ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_PrevButton->Enable( false );
	EditorSizer->Add( m_PrevButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxLEFT, 5 );

	m_NextButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_right ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_NextButton->Enable( false );
	EditorSizer->Add( m_NextButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP, 5 );

	m_ReloadButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_reload ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_ReloadButton->Enable( false );
	EditorSizer->Add( m_ReloadButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

	EditorSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	wxStaticText * ArtistStaticText;
	ArtistStaticText = new wxStaticText( this, wxID_ANY, _( "Artist:" ), wxDefaultPosition, wxDefaultSize, 0 );
	ArtistStaticText->Wrap( -1 );
	EditorSizer->Add( ArtistStaticText, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxLEFT, 5 );

	m_ArtistTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_ArtistTextCtrl->Enable( false );

	EditorSizer->Add( m_ArtistTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT, 5 );

	wxStaticText * TrackStaticText;
	TrackStaticText = new wxStaticText( this, wxID_ANY, _( "Track:" ), wxDefaultPosition, wxDefaultSize, 0 );
	TrackStaticText->Wrap( -1 );

	EditorSizer->Add( TrackStaticText, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxLEFT, 5 );

	m_TrackTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_TrackTextCtrl->Enable( false );

	EditorSizer->Add( m_TrackTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxTOP, 5 );
	m_SearchButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_accept ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_SearchButton->Enable( false );

	EditorSizer->Add( m_SearchButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

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

	ArInfoTitleSizer->Add( m_ArtistDetailsStaticText, 0, wxALL, 5 );

	wxStaticLine * ArInfoStaticLine = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	ArInfoTitleSizer->Add( ArInfoStaticLine, 1, wxEXPAND | wxALL, 5 );

//	m_LastFMPlayBitmapBtn = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_player_normal_play ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
//	m_LastFMPlayBitmapBtn->SetToolTip( _( "Enables or Disables the update of this page when the current playing track changes" ) );
//	ArInfoTitleSizer->Add( m_LastFMPlayBitmapBtn, 0, wxRIGHT, 5 );

	m_MainSizer->Add( ArInfoTitleSizer, 0, wxEXPAND, 5 );

	m_ArtistInfoMainSizer = new wxBoxSizer( wxVERTICAL );

	m_ArtistInfoCtrl = new guArtistInfoCtrl( this, m_Db, m_DbCache, m_PlayerPanel );

	m_ArtistInfoMainSizer->Add( m_ArtistInfoCtrl, 1, wxEXPAND, 5 );

	m_MainSizer->Add( m_ArtistInfoMainSizer, 0, wxEXPAND, 5 );

    if( m_ShowArtistDetails )
        m_MainSizer->Show( m_ArtistInfoMainSizer );
    else
        m_MainSizer->Hide( m_ArtistInfoMainSizer );

    m_MainSizer->FitInside( this );
    // Top Albmus
	wxBoxSizer* AlTitleSizer;
	AlTitleSizer = new wxBoxSizer( wxHORIZONTAL );

	m_AlbumsStaticText = new wxStaticText( this, wxID_ANY, _("Top Albums"), wxDefaultPosition, wxDefaultSize, 0 );
	m_AlbumsStaticText->Wrap( -1 );
	//AlbumsStaticText->SetCursor( wxCURSOR_HAND );
	m_AlbumsStaticText->SetFont( CurrentFont ); //wxFont( GULASTFM_TITLE_FONT_SIZE, 70, 90, 92, false, wxEmptyString ) );

	AlTitleSizer->Add( m_AlbumsStaticText, 0, wxALL, 5 );

	wxStaticLine * AlStaticLine = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	AlTitleSizer->Add( AlStaticLine, 1, wxEXPAND | wxALL, 5 );

	m_MainSizer->Add( AlTitleSizer, 0, wxEXPAND, 5 );

	m_AlbumsSizer = new wxGridSizer( 4, 3, 0, 0 );
    int index;
    for( index = 0; index < GULASTFMINFO_MAXITEMS; index++ )
    {
        m_AlbumInfoCtrls.Add( new guAlbumInfoCtrl( this, m_Db, m_DbCache, m_PlayerPanel ) );
        m_AlbumsSizer->Add( m_AlbumInfoCtrls[ index ], 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
    }

    m_MainSizer->Add( m_AlbumsSizer, 0, wxEXPAND, 5 );

    if( m_ShowAlbums )
        m_MainSizer->Show( m_AlbumsSizer );
    else
        m_MainSizer->Hide( m_AlbumsSizer );

    // Similar Artists
	wxBoxSizer* SimArTitleSizer;
	SimArTitleSizer = new wxBoxSizer( wxHORIZONTAL );

	m_ArtistsStaticText = new wxStaticText( this, wxID_ANY, _("Similar Artists"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ArtistsStaticText->Wrap( -1 );
	m_ArtistsStaticText->SetFont( CurrentFont ); //wxFont( GULASTFM_TITLE_FONT_SIZE, 70, 90, 92, false, wxEmptyString ) );

	SimArTitleSizer->Add( m_ArtistsStaticText, 0, wxALL, 5 );

	wxStaticLine * SimArStaticLine = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	SimArTitleSizer->Add( SimArStaticLine, 1, wxEXPAND | wxALL, 5 );

	m_MainSizer->Add( SimArTitleSizer, 0, wxEXPAND, 5 );

	m_ArtistsSizer = new wxGridSizer( 4, 3, 0, 0 );

    for( index = 0; index < GULASTFMINFO_MAXITEMS; index++ )
    {
        m_ArtistInfoCtrls.Add( new guSimilarArtistInfoCtrl( this, m_Db, m_DbCache, m_PlayerPanel ) );
        m_ArtistsSizer->Add( m_ArtistInfoCtrls[ index ], 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
    }

	m_MainSizer->Add( m_ArtistsSizer, 0, wxEXPAND, 5 );

    if( m_ShowArtists )
        m_MainSizer->Show( m_ArtistsSizer );
    else
        m_MainSizer->Hide( m_ArtistsSizer );

    //
    // Similar Tracks
    //
	wxBoxSizer * SimTracksTitleSizer;
	SimTracksTitleSizer = new wxBoxSizer( wxHORIZONTAL );

	m_TracksStaticText = new wxStaticText( this, wxID_ANY, _("Similar Tracks"), wxDefaultPosition, wxDefaultSize, 0 );
	m_TracksStaticText->Wrap( -1 );
	m_TracksStaticText->SetFont( CurrentFont ); //wxFont( GULASTFM_TITLE_FONT_SIZE, 70, 90, 92, false, wxEmptyString ) );

	SimTracksTitleSizer->Add( m_TracksStaticText, 0, wxALL, 5 );

	wxStaticLine * SimTrStaticLine = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	SimTracksTitleSizer->Add( SimTrStaticLine, 1, wxEXPAND | wxALL, 5 );

	m_MainSizer->Add( SimTracksTitleSizer, 0, wxEXPAND, 5 );

	m_TracksSizer = new wxGridSizer( 4, 3, 0, 0 );

    for( index = 0; index < GULASTFMINFO_MAXITEMS; index++ )
    {
        m_TrackInfoCtrls.Add( new guTrackInfoCtrl( this, m_Db, m_DbCache, m_PlayerPanel ) );
        m_TracksSizer->Add( m_TrackInfoCtrls[ index ], 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
    }

	m_MainSizer->Add( m_TracksSizer, 0, wxEXPAND, 5 );
	//MainSizer->Add( new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL ), 0, wxEXPAND, 5 );
    if( m_ShowTracks )
        m_MainSizer->Show( m_TracksSizer );
    else
        m_MainSizer->Hide( m_TracksSizer );

    //
    // Artist Events
    //
	wxBoxSizer * EventsTitleSizer;
	EventsTitleSizer = new wxBoxSizer( wxHORIZONTAL );

	m_EventsStaticText = new wxStaticText( this, wxID_ANY, _("Events"), wxDefaultPosition, wxDefaultSize, 0 );
	m_EventsStaticText->Wrap( -1 );
	m_EventsStaticText->SetFont( CurrentFont ); //wxFont( GULASTFM_TITLE_FONT_SIZE, 70, 90, 92, false, wxEmptyString ) );

	EventsTitleSizer->Add( m_EventsStaticText, 0, wxALL, 5 );

	wxStaticLine * EventsStaticLine = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	EventsTitleSizer->Add( EventsStaticLine, 1, wxEXPAND | wxALL, 5 );

	m_MainSizer->Add( EventsTitleSizer, 0, wxEXPAND, 5 );

	m_EventsSizer = new wxGridSizer( 4, 3, 0, 0 );

    for( index = 0; index < GULASTFMINFO_MAXITEMS; index++ )
    {
        m_EventsInfoCtrls.Add( new guEventInfoCtrl( this, m_Db, m_DbCache, m_PlayerPanel ) );
        m_EventsSizer->Add( m_EventsInfoCtrls[ index ], 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
    }

	m_MainSizer->Add( m_EventsSizer, 0, wxEXPAND, 5 );
	//MainSizer->Add( new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL ), 0, wxEXPAND, 5 );
    if( m_ShowEvents )
        m_MainSizer->Show( m_EventsSizer );
    else
        m_MainSizer->Hide( m_EventsSizer );


	this->SetSizer( m_MainSizer );
	this->Layout();

	SetScrollRate( 21, 21 );

    m_AlbumsUpdateThread = NULL;
    m_ArtistsUpdateThread = NULL;
    m_TracksUpdateThread = NULL;


    SetDropTarget( new guLastFMPanelDropTarget( this ) );

    Connect( ID_LASTFM_UPDATE_ALBUMINFO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLastFMPanel::OnUpdateAlbumItem ) );
    Connect( ID_LASTFM_UPDATE_ARTISTINFO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLastFMPanel::OnUpdateArtistInfo ) );
    Connect( ID_LASTFM_UPDATE_SIMARTIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLastFMPanel::OnUpdateArtistItem ) );
    Connect( ID_LASTFM_UPDATE_SIMTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLastFMPanel::OnUpdateTrackItem ) );
    Connect( ID_LASTFM_UPDATE_EVENTINFO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLastFMPanel::OnUpdateEventItem ) );

	m_ArtistDetailsStaticText->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guLastFMPanel::OnArInfoTitleDClicked ), NULL, this );
	m_UpdateCheckBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guLastFMPanel::OnUpdateChkBoxClick ), NULL, this );
	m_PrevButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guLastFMPanel::OnPrevBtnClick ), NULL, this );
	m_NextButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guLastFMPanel::OnNextBtnClick ), NULL, this );
	m_ReloadButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guLastFMPanel::OnReloadBtnClick ), NULL, this );
	m_ArtistTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guLastFMPanel::OnTextUpdated ), NULL, this );
	m_TrackTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guLastFMPanel::OnTextUpdated ), NULL, this );
	m_SearchButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guLastFMPanel::OnSearchBtnClick ), NULL, this );
	m_AlbumsStaticText->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guLastFMPanel::OnTopAlbumsTitleDClick ), NULL, this );
	m_ArtistsStaticText->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guLastFMPanel::OnSimArTitleDClick ), NULL, this );
	m_TracksStaticText->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guLastFMPanel::OnSimTrTitleDClick ), NULL, this );
	m_EventsStaticText->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guLastFMPanel::OnEventsTitleDClick ), NULL, this );

//	ShowMoreHyperLink->Connect( wxEVT_COMMAND_HYPERLINK, wxHyperlinkEventHandler( guLastFMPanel::OnShowMoreLinkClicked ), NULL, this );
//	ArtistDetails->Connect( wxEVT_COMMAND_HTML_LINK_CLICKED, wxHtmlLinkEventHandler( guLastFMPanel::OnHtmlLinkClicked ), NULL, this );

}

// -------------------------------------------------------------------------------- //
guLastFMPanel::~guLastFMPanel()
{
    m_ArtistsUpdateThreadMutex.Lock();
    if( m_ArtistsUpdateThread )
    {
        m_ArtistsUpdateThread->Pause();
        m_ArtistsUpdateThread->Delete();
    }
    m_ArtistsUpdateThreadMutex.Unlock();

    m_TracksUpdateThreadMutex.Lock();
    if( m_TracksUpdateThread )
    {
        m_TracksUpdateThread->Pause();
        m_TracksUpdateThread->Delete();
    }
    m_TracksUpdateThreadMutex.Unlock();

    //
    Disconnect( ID_LASTFM_UPDATE_ALBUMINFO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLastFMPanel::OnUpdateAlbumItem ) );
    Disconnect( ID_LASTFM_UPDATE_ARTISTINFO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLastFMPanel::OnUpdateArtistInfo ) );
    Disconnect( ID_LASTFM_UPDATE_SIMARTIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLastFMPanel::OnUpdateArtistItem ) );
    Disconnect( ID_LASTFM_UPDATE_SIMTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLastFMPanel::OnUpdateTrackItem ) );
    Disconnect( ID_LASTFM_UPDATE_EVENTINFO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLastFMPanel::OnUpdateEventItem ) );

	m_ArtistDetailsStaticText->Disconnect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guLastFMPanel::OnArInfoTitleDClicked ), NULL, this );
	m_UpdateCheckBox->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guLastFMPanel::OnUpdateChkBoxClick ), NULL, this );
	m_ArtistTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guLastFMPanel::OnTextUpdated ), NULL, this );
	m_TrackTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guLastFMPanel::OnTextUpdated ), NULL, this );
	m_SearchButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guLastFMPanel::OnSearchBtnClick ), NULL, this );
	m_AlbumsStaticText->Disconnect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guLastFMPanel::OnTopAlbumsTitleDClick ), NULL, this );
	m_ArtistsStaticText->Disconnect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guLastFMPanel::OnSimArTitleDClick ), NULL, this );
	m_TracksStaticText->Disconnect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guLastFMPanel::OnSimTrTitleDClick ), NULL, this );
	m_EventsStaticText->Disconnect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guLastFMPanel::OnEventsTitleDClick ), NULL, this );

    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        Config->WriteBool( wxT( "LFMShowArtistInfo" ), m_ShowArtistDetails, wxT( "General" ) );
        Config->WriteBool( wxT( "LFMShowAlbums" ), m_ShowAlbums, wxT( "General" ) );
        Config->WriteBool( wxT( "LFMShowArtists" ), m_ShowArtists, wxT( "General" ) );
        Config->WriteBool( wxT( "LFMShowTracks" ), m_ShowTracks, wxT( "General" ) );
        Config->WriteBool( wxT( "LFMShowEvents" ), m_ShowEvents, wxT( "General" ) );
        Config->WriteBool( wxT( "LFMFollowPlayer" ), m_UpdateCheckBox->GetValue(), wxT( "General" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::ShowCurrentTrack( void )
{
    int index;

    m_ArtistName = m_TrackChangeItems[ m_CurrentTrackInfo ].m_ArtistName;
    m_TrackName  = m_TrackChangeItems[ m_CurrentTrackInfo ].m_TrackName;
    //guLogMessage( wxT( "LastFMPanel:ShowCurrentTrack( '%s', '%s' )" ), artistname.c_str(), trackname.c_str() );

    if( m_LastArtistName != m_ArtistName )
    {
        m_ArtistsUpdateThreadMutex.Lock();
        if( m_ArtistsUpdateThread )
        {
            m_ArtistsUpdateThread->Pause();
            m_ArtistsUpdateThread->Delete();
        }

        m_AlbumsUpdateThreadMutex.Lock();
        if( m_AlbumsUpdateThread )
        {
            m_AlbumsUpdateThread->Pause();
            m_AlbumsUpdateThread->Delete();
        }

        // Clear the LastFM controls to default values
        m_ArtistInfoCtrl->Clear();

        for( index = 0; index < GULASTFMINFO_MAXITEMS; index++ )
        {
            m_ArtistInfoCtrls[ index ]->Clear();
            m_AlbumInfoCtrls[ index ]->Clear();
            m_EventsInfoCtrls[ index ]->Clear();
        }

        m_LastArtistName = m_ArtistName;
        if( !m_ArtistName.IsEmpty() )
        {
            m_ArtistsUpdateThread = new guFetchSimilarArtistInfoThread( this, m_DbCache, m_ArtistName.c_str() );

            m_AlbumsUpdateThread = new guFetchAlbumInfoThread( this, m_DbCache, m_ArtistName.c_str() );
        }
        m_ArtistTextCtrl->SetValue( m_ArtistName );

        m_ArtistsUpdateThreadMutex.Unlock();
        m_AlbumsUpdateThreadMutex.Unlock();
    }

    if( m_LastTrackName != m_TrackName )
    {
        m_TracksUpdateThreadMutex.Lock();
        if( m_TracksUpdateThread )
        {
            m_TracksUpdateThread->Pause();
            m_TracksUpdateThread->Delete();
        }

        for( index = 0; index < GULASTFMINFO_MAXITEMS; index++ )
        {
            m_TrackInfoCtrls[ index ]->Clear();
        }

        m_LastTrackName = m_TrackName;
        if( !m_ArtistName.IsEmpty() && !m_TrackName.IsEmpty() )
        {
            m_TracksUpdateThread = new guFetchTrackInfoThread( this, m_DbCache, m_ArtistName.c_str(), m_TrackName.c_str() );
        }
        m_TrackTextCtrl->SetValue( m_TrackName );

        m_TracksUpdateThreadMutex.Unlock();
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
        wxCommandEvent UpdateEvent( wxEVT_COMMAND_MENU_SELECTED, ID_MAINFRAME_REQUEST_CURRENTTRACK );
        UpdateEvent.SetClientData( this );
        wxPostEvent( wxTheApp->GetTopWindow(), UpdateEvent );
    }
    //
    m_ArtistTextCtrl->Enable( !m_UpdateEnabled );
    m_TrackTextCtrl->Enable( !m_UpdateEnabled );
    m_SearchButton->Enable( !m_UpdateEnabled &&
                !m_ArtistTextCtrl->GetValue().IsEmpty() );
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnTextUpdated( wxCommandEvent& event )
{
    m_SearchButton->Enable( !m_UpdateEnabled &&
        !m_ArtistTextCtrl->GetValue().IsEmpty() );
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

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnSearchBtnClick( wxCommandEvent& event )
{
    guTrackChangeInfo TrackChangeInfo( m_ArtistTextCtrl->GetValue(), m_TrackTextCtrl->GetValue() );
    AppendTrackChangeInfo( &TrackChangeInfo );
    ShowCurrentTrack();
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
        m_AlbumInfoCtrls[ index ]->SetInfo( AlbumInfo );
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
        m_ArtistInfoCtrls[ index ]->SetInfo( ArtistInfo );
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
        m_TrackInfoCtrls[ index ]->SetInfo( TrackInfo );
        //Layout();
        m_MainSizer->FitInside( this );
    }
    else
    {
        guLogError( wxT( "Received a LastFMInfo track event with NULL Data" ) );
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
void guLastFMPanel::UpdateLayout( void )
{
    m_MainSizer->FitInside( this );
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnTopAlbumsTitleDClick( wxMouseEvent &event )
{
    m_ShowAlbums = !m_ShowAlbums;
    if( m_ShowAlbums )
        m_MainSizer->Show( m_AlbumsSizer );
    else
        m_MainSizer->Hide( m_AlbumsSizer );
    UpdateLayout();
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnSimArTitleDClick( wxMouseEvent &event )
{
    m_ShowArtists = !m_ShowArtists;
    if( m_ShowArtists )
        m_MainSizer->Show( m_ArtistsSizer );
    else
        m_MainSizer->Hide( m_ArtistsSizer );
    UpdateLayout();
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnSimTrTitleDClick( wxMouseEvent &event )
{
    m_ShowTracks = !m_ShowTracks;
    if( m_ShowTracks )
        m_MainSizer->Show( m_TracksSizer );
    else
        m_MainSizer->Hide( m_TracksSizer );
    UpdateLayout();
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnEventsTitleDClick( wxMouseEvent &event )
{
    m_ShowEvents = !m_ShowEvents;
    if( m_ShowEvents )
        m_MainSizer->Show( m_EventsSizer );
    else
        m_MainSizer->Hide( m_EventsSizer );
    UpdateLayout();
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnDropFiles( const wxArrayString &files )
{
    //guLogMessage( wxT( "guLastFMPanelDropTarget::OnDropFiles" ) );
    if( !files.Count() )
        return;

    guTrack Track;
    if( m_Db->FindTrackFile( files[ 0 ], &Track ) )
    {
        guTrackChangeInfo ChangeInfo;

        ChangeInfo.m_ArtistName = Track.m_ArtistName;
        ChangeInfo.m_TrackName = Track.m_SongName;

        SetUpdateEnable( false );
        AppendTrackChangeInfo( &ChangeInfo );
        ShowCurrentTrack();
    }
    else
    {
        guTagInfo * TagInfo;
        TagInfo = guGetTagInfoHandler( files[ 0 ] );

        if( TagInfo )
        {
            //guLogMessage( wxT( "Reading tags from the file..." ) );
            if( TagInfo->Read() )
            {
                Track.m_FileName = files[ 0 ];
                Track.m_Type = guTRACK_TYPE_NOTDB;
                Track.m_ArtistName = TagInfo->m_ArtistName;
                Track.m_AlbumName = TagInfo->m_AlbumName;
                Track.m_SongName = TagInfo->m_TrackName;
                Track.m_Number = TagInfo->m_Track;
                Track.m_GenreName = TagInfo->m_GenreName;
                Track.m_Length = TagInfo->m_Length;
                Track.m_Year = TagInfo->m_Year;
                Track.m_Rating = wxNOT_FOUND;

                guTrackChangeInfo ChangeInfo;

                ChangeInfo.m_ArtistName = Track.m_ArtistName;
                ChangeInfo.m_TrackName = Track.m_SongName;

                SetUpdateEnable( false );
                AppendTrackChangeInfo( &ChangeInfo );
                ShowCurrentTrack();
            }

            delete TagInfo;
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
        m_MainThread->m_DownloadThreads.Remove( this );
        m_MainThread->m_DownloadThreadsMutex.Unlock();
    }
}

// -------------------------------------------------------------------------------- //
guDownloadImageThread::ExitCode guDownloadImageThread::Entry()
{
    int             ImageType;
    wxImage *       Image = NULL;

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
                        case guDBCACHE_IMAGE_SIZE_TINY  :
                        {
                            Size = 50;
                            break;
                        }

                        case guDBCACHE_IMAGE_SIZE_MID   :
                        {
                            Size = 100;
                            break;
                        }

                        default : //case guDBCACHE_IMAGE_SIZE_BIG    :
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
        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, m_CommandId );
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
           guDbCache * dbcache, const wxChar * artistname ) :
    guFetchLastFMInfoThread( lastfmpanel )
{
    m_DbCache = dbcache;
    m_ArtistName = wxString( artistname );
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
        m_LastFMPanel->m_AlbumsUpdateThread = NULL;
        m_LastFMPanel->m_AlbumsUpdateThreadMutex.Unlock();
    }
}

// -------------------------------------------------------------------------------- //
guFetchAlbumInfoThread::ExitCode guFetchAlbumInfoThread::Entry()
{
    int index;
    int count;
    guLastFM * LastFM = new guLastFM();
    if( LastFM )
    {
        if( !TestDestroy() )
        {
            //guLogMessage( wxT( "==== Getting Top Albums ====" ) );
            guAlbumInfoArray TopAlbums = LastFM->ArtistGetTopAlbums( m_ArtistName );
            if( ( count = TopAlbums.Count() ) )
            {
                //guLogMessage( wxT( "Top Albums: %u" ), count );
                if( count > GULASTFMINFO_MAXITEMS )
                    count = GULASTFMINFO_MAXITEMS;

                for( index = 0; index < count; index++ )
                {
                    m_DownloadThreadsMutex.Lock();
                    if( !TestDestroy() )
                    {
                        guLastFMAlbumInfo * LastFMAlbumInfo = new guLastFMAlbumInfo( index, NULL,
                              new guAlbumInfo( TopAlbums[ index ] ) );
                        if( LastFMAlbumInfo )
                        {
                            LastFMAlbumInfo->m_ImageUrl = TopAlbums[ index ].m_ImageLink;
                            guDownloadImageThread * DownloadImageThread = new guDownloadImageThread(
                                    m_LastFMPanel,
                                    this,
                                    m_DbCache,
                                    index,
                                    TopAlbums[ index ].m_ImageLink.c_str(),
                                    ID_LASTFM_UPDATE_ALBUMINFO,
                                    LastFMAlbumInfo,
                                    &LastFMAlbumInfo->m_Image,
                                    guDBCACHE_IMAGE_SIZE_TINY );
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
// guFetchSimilarArtistInfoThread
// -------------------------------------------------------------------------------- //
guFetchSimilarArtistInfoThread::guFetchSimilarArtistInfoThread( guLastFMPanel * lastfmpanel,
                    guDbCache * dbcache, const wxChar * artistname ) :
    guFetchLastFMInfoThread( lastfmpanel )
{
    m_DbCache = dbcache;
    //guLogMessage( wxT( "guFetchSimilarArtistInfoThread : '%s'" ), artistname );
    m_ArtistName  = wxString( artistname );
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
        m_LastFMPanel->m_ArtistsUpdateThreadMutex.Lock();
        m_LastFMPanel->m_ArtistsUpdateThread = NULL;
        m_LastFMPanel->m_ArtistsUpdateThreadMutex.Unlock();
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
                        guDBCACHE_IMAGE_SIZE_MID );
                    if( !DownloadImageThread )
                    {
                        guLogError( wxT( "Could not create the artist image download thread" ) );
                    }
                }
            }
            m_DownloadThreadsMutex.Unlock();

            //guLogMessage( wxT( "==== Getting Similar Artists ====" ) );
            guSimilarArtistInfoArray SimilarArtists = LastFM->ArtistGetSimilar( m_ArtistName );
            if( ( count = SimilarArtists.Count() ) )
            {
                //guLogMessage( wxT( "Similar Artists: %u" ), count );
                if( count > GULASTFMINFO_MAXITEMS )
                    count = GULASTFMINFO_MAXITEMS;
                for( index = 0; index < count; index++ )
                {
                    m_DownloadThreadsMutex.Lock();
                    if( !TestDestroy() )
                    {
                        guLastFMSimilarArtistInfo * LastFMArtistInfo = new guLastFMSimilarArtistInfo( index, NULL,
                            new guSimilarArtistInfo( SimilarArtists[ index ] ) );
                        if( LastFMArtistInfo )
                        {
                            LastFMArtistInfo->m_ImageUrl = SimilarArtists[ index ].m_ImageLink;
                            guDownloadImageThread* DownloadImageThread = new guDownloadImageThread(
                                m_LastFMPanel,
                                this,
                                m_DbCache,
                                index,
                                SimilarArtists[ index ].m_ImageLink.c_str(),
                                ID_LASTFM_UPDATE_SIMARTIST,
                                LastFMArtistInfo,
                                &LastFMArtistInfo->m_Image,
                                guDBCACHE_IMAGE_SIZE_TINY );
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

            //guLogMessage( wxT( "==== Getting Artist Events ====" ) );
            guEventInfoArray ArtistEvents = LastFM->ArtistGetEvents( m_ArtistName );
            if( ( count = ArtistEvents.Count() ) )
            {
                //guLogMessage( wxT( "Similar Artists: %u" ), count );
                if( count > GULASTFMINFO_MAXITEMS )
                    count = GULASTFMINFO_MAXITEMS;
                for( index = 0; index < count; index++ )
                {
                    m_DownloadThreadsMutex.Lock();
                    if( !TestDestroy() )
                    {
                        guLastFMEventInfo * LastFMEventInfo = new guLastFMEventInfo( index, NULL,
                            new guEventInfo( ArtistEvents[ index ] ) );
                        if( LastFMEventInfo )
                        {
                            LastFMEventInfo->m_ImageUrl = ArtistEvents[ index ].m_ImageLink;
                            guDownloadImageThread * DownloadImageThread = new guDownloadImageThread(
                                m_LastFMPanel,
                                this,
                                m_DbCache,
                                index,
                                ArtistEvents[ index ].m_ImageLink.c_str(),
                                ID_LASTFM_UPDATE_EVENTINFO,
                                LastFMEventInfo,
                                &LastFMEventInfo->m_Image,
                                guDBCACHE_IMAGE_SIZE_TINY );
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
// guFetchTrackInfoThread
// -------------------------------------------------------------------------------- //
guFetchTrackInfoThread::guFetchTrackInfoThread( guLastFMPanel * lastfmpanel,
         guDbCache * dbcache, const wxChar * artistname, const wxChar * trackname ) :
    guFetchLastFMInfoThread( lastfmpanel )
{
    m_DbCache = dbcache;
    m_ArtistName = wxString( artistname );
    m_TrackName  = wxString( trackname );

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guFetchTrackInfoThread::~guFetchTrackInfoThread()
{
    if( !TestDestroy() )
    {
        m_LastFMPanel->m_TracksUpdateThreadMutex.Lock();
        m_LastFMPanel->m_TracksUpdateThread = NULL;
        m_LastFMPanel->m_TracksUpdateThreadMutex.Unlock();
    }
}

// -------------------------------------------------------------------------------- //
guFetchTrackInfoThread::ExitCode guFetchTrackInfoThread::Entry()
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
            if( ( count = SimilarTracks.Count() ) )
            {
                //guLogMessage( wxT( "Similar Tracks: %u" ), count );
                if( count > GULASTFMINFO_MAXITEMS )
                    count = GULASTFMINFO_MAXITEMS;
                for( index = 0; index < count; index++ )
                {
                    m_DownloadThreadsMutex.Lock();
                    if( !TestDestroy() )
                    {
                        guLastFMTrackInfo * LastFMTrackInfo = new guLastFMTrackInfo( index, NULL,
                              new guSimilarTrackInfo( SimilarTracks[ index ] ) );
                        if( LastFMTrackInfo )
                        {
                            LastFMTrackInfo->m_ImageUrl = SimilarTracks[ index ].m_ImageLink;
                            guDownloadImageThread * DownloadImageThread = new guDownloadImageThread(
                                    m_LastFMPanel,
                                    this,
                                    m_DbCache,
                                    index, SimilarTracks[ index ].m_ImageLink.c_str(),
                                    ID_LASTFM_UPDATE_SIMTRACK,
                                    LastFMTrackInfo,
                                    &LastFMTrackInfo->m_Image,
                                    guDBCACHE_IMAGE_SIZE_TINY );
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
}

// -------------------------------------------------------------------------------- //
guLastFMPanelDropTarget::~guLastFMPanelDropTarget()
{
}

// -------------------------------------------------------------------------------- //
bool guLastFMPanelDropTarget::OnDropFiles( wxCoord x, wxCoord y, const wxArrayString &files )
{
    m_LastFMPanel->OnDropFiles( files );
    return true;
}

// -------------------------------------------------------------------------------- //
wxDragResult guLastFMPanelDropTarget::OnDragOver( wxCoord x, wxCoord y, wxDragResult def )
{
    //printf( "guLastFMPanelDropTarget::OnDragOver... %d - %d\n", x, y );
    return wxDragCopy;
}

// -------------------------------------------------------------------------------- //
