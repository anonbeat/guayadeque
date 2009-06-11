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
#include "LastFMPanel.h"

#include "Commands.h"
#include "Images.h"
#include "Utils.h"
#include "MainApp.h"

#include <wx/arrimpl.cpp>
#include <wx/curl/http.h>
#include <wx/statline.h>
#include <wx/uri.h>

#define GULASTFM_TITLE_FONT_SIZE 12

WX_DEFINE_OBJARRAY(guLastFMInfoArray);
WX_DEFINE_OBJARRAY(guLastFMSimilarArtistInfoArray);
WX_DEFINE_OBJARRAY(guLastFMTrackInfoArray);
WX_DEFINE_OBJARRAY(guLastFMAlbumInfoArray);

// -------------------------------------------------------------------------------- //
guLastFMInfoCtrl::guLastFMInfoCtrl( wxWindow * parent, DbLibrary * db, guPlayerPanel * playerpanel, bool createcontrols ) :
    wxPanel( parent, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), wxTAB_TRAVERSAL )
{
    m_Db = db;
    m_PlayerPanel = playerpanel;
    m_NormalColor = wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT );
    m_NotFoundColor = wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHT );

    if( createcontrols )
        this->CreateControls( parent );
}

// -------------------------------------------------------------------------------- //
guLastFMInfoCtrl::~guLastFMInfoCtrl()
{
    Disconnect( wxEVT_CONTEXT_MENU, wxContextMenuEventHandler( guLastFMInfoCtrl::OnContextMenu ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guLastFMInfoCtrl::CreateControls( wxWindow * parent )
{
	wxBoxSizer* MainSizer;
	MainSizer = new wxBoxSizer( wxHORIZONTAL );

	m_Bitmap = new wxStaticBitmap( this, wxID_ANY, wxBitmap( guImage_default_lastfm_image ),
	                                            wxDefaultPosition, wxSize( 50, 50 ), 0 );
    //Bitmap->SetCursor( wxCURSOR_HAND );
	MainSizer->Add( m_Bitmap, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2 );

	m_Text = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_Text->Wrap( -1 );
	//Text->SetCursor( wxCursor( wxCURSOR_HAND ) );
	m_Text->SetMaxSize( wxSize( 250, -1 ) );
	MainSizer->Add( m_Text, 1, wxALL|wxALIGN_CENTER_VERTICAL, 2 );


	SetSizer( MainSizer );
	Layout();
	MainSizer->Fit( this );

    Connect( wxEVT_CONTEXT_MENU, wxContextMenuEventHandler( guLastFMInfoCtrl::OnContextMenu ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guLastFMInfoCtrl::Clear( void )
{
    m_Bitmap->SetBitmap( guImage_default_lastfm_image );
    m_Text->SetLabel( wxEmptyString );
}

// -------------------------------------------------------------------------------- //
void guLastFMInfoCtrl::SetBitmap( const wxImage * image )
{
    if( image )
    {
        m_Bitmap->SetBitmap( wxBitmap( image->Copy() ) );
    }
    else
    {
        m_Bitmap->SetBitmap( wxBitmap( guImage_default_lastfm_image ) );
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
                    MenuItem->SetBitmap( wxBitmap( guImage_search ) );
                }
                Menu->Append( MenuItem );
            }
        }
        else
        {
            MenuItem = new wxMenuItem( Menu, -1, _( "No search link defined" ), _( "Add search links in preferences" ) );
            MenuItem->SetBitmap( wxBitmap( guImage_search ) );
            Menu->Append( MenuItem );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guLastFMInfoCtrl::OnClick( wxMouseEvent &event )
{
}

// -------------------------------------------------------------------------------- //
void guLastFMInfoCtrl::OnSearchLinkClicked( wxCommandEvent &event )
{
    int index = event.GetId();
    if( index == ID_LASTFM_VISIT_URL )
    {
        wxMouseEvent event;
        OnClick( event );
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
wxString guLastFMInfoCtrl::GetSearchText( void )
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
int guLastFMInfoCtrl::GetSelectedTracks( guTrackArray * tracks )
{
    if( m_Info->m_TrackId != wxNOT_FOUND )
    {
        wxArrayInt Selections;
        Selections.Add( m_Info->m_TrackId );
        return m_Db->GetSongs( Selections, tracks );
    }
}

// -------------------------------------------------------------------------------- //
// guArtistInfoCtrl
// -------------------------------------------------------------------------------- //
guArtistInfoCtrl::guArtistInfoCtrl( wxWindow * parent, DbLibrary * db, guPlayerPanel * playerpanel ) :
                 guLastFMInfoCtrl( parent, db, playerpanel, false )
{
    m_Info = NULL;
    m_ShowLongBioText = false;

    CreateControls( parent );

    m_Text->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( guArtistInfoCtrl::OnClick ), NULL, this );
    m_Bitmap->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( guArtistInfoCtrl::OnClick ), NULL, this );
    Connect( ID_LASTFM_SEARCH_LINK, ID_LASTFM_SEARCH_LINK + 999, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guArtistInfoCtrl::OnSearchLinkClicked ) );
    Connect( ID_LASTFM_VISIT_URL, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guArtistInfoCtrl::OnSearchLinkClicked ) );
	m_ShowMoreHyperLink->Connect( wxEVT_COMMAND_HYPERLINK, wxHyperlinkEventHandler( guArtistInfoCtrl::OnShowMoreLinkClicked ), NULL, this );
	m_ArtistDetails->Connect( wxEVT_COMMAND_HTML_LINK_CLICKED, wxHtmlLinkEventHandler( guArtistInfoCtrl::OnHtmlLinkClicked ), NULL, this );
};

// -------------------------------------------------------------------------------- //
guArtistInfoCtrl::~guArtistInfoCtrl()
{
    if( m_Info )
        delete m_Info;

    m_Text->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( guArtistInfoCtrl::OnClick ), NULL, this );
    m_Bitmap->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( guArtistInfoCtrl::OnClick ), NULL, this );
    Disconnect( ID_LASTFM_SEARCH_LINK, ID_LASTFM_SEARCH_LINK + 999, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guArtistInfoCtrl::OnSearchLinkClicked ) );
    Disconnect( ID_LASTFM_VISIT_URL, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guArtistInfoCtrl::OnSearchLinkClicked ) );
	m_ShowMoreHyperLink->Disconnect( wxEVT_COMMAND_HYPERLINK, wxHyperlinkEventHandler( guArtistInfoCtrl::OnShowMoreLinkClicked ), NULL, this );
	m_ArtistDetails->Disconnect( wxEVT_COMMAND_HTML_LINK_CLICKED, wxHtmlLinkEventHandler( guArtistInfoCtrl::OnHtmlLinkClicked ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guArtistInfoCtrl::CreateControls( wxWindow * parent )
{
	m_MainSizer = new wxBoxSizer( wxHORIZONTAL );

	m_Bitmap = new wxStaticBitmap( this, wxID_ANY, wxBitmap( guImage_nophoto ), wxDefaultPosition, wxSize( 100,100 ), 0 );
	m_MainSizer->Add( m_Bitmap, 0, wxALL, 5 );

//	wxBoxSizer * DetailSizer;
	m_DetailSizer = new wxBoxSizer( wxVERTICAL );

	m_Text = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_Text->Wrap( -1 );
	m_Text->SetFont( wxFont( 12, 74, 90, 92, false, wxT("Sans") ) );

	m_DetailSizer->Add( m_Text, 0, wxTOP|wxRIGHT|wxLEFT, 5 );

	m_ArtistDetails = new wxHtmlWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_AUTO );
	m_ArtistDetails->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT ) );
	wxColour ArtistBG = wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWFRAME );
	ArtistBG.Set( ArtistBG.Red() - 5, ArtistBG.Green() - 5, ArtistBG.Blue() - 5 );
	m_ArtistDetails->SetBackgroundColour( ArtistBG );
	m_ArtistDetails->SetBorders( 0 );
	m_DetailSizer->Add( m_ArtistDetails, 1, wxALL|wxEXPAND, 5 );

	m_ShowMoreHyperLink = new wxHyperlinkCtrl( this, wxID_ANY, _("More..."), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
	m_ShowMoreHyperLink->SetNormalColour( wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHT ) );
	m_ShowMoreHyperLink->SetVisitedColour( wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHT ) );
	m_ShowMoreHyperLink->SetHoverColour( wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHT ) );
	m_DetailSizer->Add( m_ShowMoreHyperLink, 0, wxALL|wxALIGN_RIGHT, 5 );

	m_MainSizer->Add( m_DetailSizer, 1, wxEXPAND, 5 );

	SetSizer( m_MainSizer );
	Layout();
	//MainSizer->Fit( this );

    Connect( wxEVT_CONTEXT_MENU, wxContextMenuEventHandler( guArtistInfoCtrl::OnContextMenu ), NULL, this );
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
    m_Bitmap->SetBitmap( guImage_nophoto );
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
        m_Bitmap->SetBitmap( wxBitmap( image->Copy() ) );
    }
    else
    {
        m_Bitmap->SetBitmap( wxBitmap( guImage_nophoto ) );
    }
    m_Bitmap->Refresh();
}


// -------------------------------------------------------------------------------- //
void guArtistInfoCtrl::OnClick( wxMouseEvent &event )
{
    if( m_Info )
    {
        //guLogMessage( wxT( "guAlbumInfo::OnClick %s" ), Info->Album->Url.c_str() );
        guWebExecute( m_Info->m_Artist->m_Url );
    }
}

// -------------------------------------------------------------------------------- //
wxString guArtistInfoCtrl::GetSearchText( void )
{
    return m_Info->m_Artist->m_Name;
}

// -------------------------------------------------------------------------------- //
void guArtistInfoCtrl::CreateContextMenu( wxMenu * Menu )
{
    wxMenuItem * MenuItem;

    if( m_Info->m_ArtistId != wxNOT_FOUND )
    {
        MenuItem = new wxMenuItem( Menu, ID_LASTFM_PLAY, _( "Play" ), _( "Play the artist tracks" ) );
        MenuItem->SetBitmap( wxBitmap( guImage_min_media_playback_start ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_LASTFM_ENQUEUE, _( "Enqueue" ), _( "Enqueue the artist tracks to the playlist" ) );
        MenuItem->SetBitmap( wxBitmap( guImage_vol_add ) );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();
    }

    if( !m_Info->m_Artist->m_Url.IsEmpty() )
    {
        MenuItem = new wxMenuItem( Menu, ID_LASTFM_VISIT_URL, wxT( "Last.fm" ), _( "Visit last.fm page for this item" ) );
        MenuItem->SetBitmap( wxBitmap( guImage_lastfm_as_on ) );
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
          wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWFRAME ).GetAsString( wxC2S_HTML_SYNTAX ).c_str(), //wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWFRAME ).GetAsString( wxC2S_HTML_SYNTAX ).c_str(),
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
// guAlbumInfoCtrl
// -------------------------------------------------------------------------------- //
guAlbumInfoCtrl::guAlbumInfoCtrl( wxWindow * parent, DbLibrary * db, guPlayerPanel * playerpanel ) :
                 guLastFMInfoCtrl( parent, db, playerpanel )
{
    m_Info = NULL;
    m_Text->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( guAlbumInfoCtrl::OnClick ), NULL, this );
    m_Bitmap->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( guAlbumInfoCtrl::OnClick ), NULL, this );
    Connect( ID_LASTFM_SEARCH_LINK, ID_LASTFM_SEARCH_LINK + 999, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumInfoCtrl::OnSearchLinkClicked ) );
    Connect( ID_LASTFM_VISIT_URL, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumInfoCtrl::OnSearchLinkClicked ) );
};

// -------------------------------------------------------------------------------- //
guAlbumInfoCtrl::~guAlbumInfoCtrl()
{
    if( m_Info )
        delete m_Info;

    m_Text->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( guAlbumInfoCtrl::OnClick ), NULL, this );
    m_Bitmap->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( guAlbumInfoCtrl::OnClick ), NULL, this );
    Disconnect( ID_LASTFM_SEARCH_LINK, ID_LASTFM_SEARCH_LINK + 999, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumInfoCtrl::OnSearchLinkClicked ) );
    Disconnect( ID_LASTFM_VISIT_URL, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guAlbumInfoCtrl::OnSearchLinkClicked ) );
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

// -------------------------------------------------------------------------------- //
void guAlbumInfoCtrl::OnClick( wxMouseEvent &event )
{
    if( m_Info )
    {
        //guLogMessage( wxT( "guAlbumInfo::OnClick %s" ), Info->Album->Url.c_str() );
        guWebExecute( m_Info->m_Album->m_Url );
    }
}

// -------------------------------------------------------------------------------- //
wxString guAlbumInfoCtrl::GetSearchText( void )
{
    return wxString::Format( wxT( "%s %s" ), m_Info->m_Album->m_Artist.c_str(), m_Info->m_Album->m_Name.c_str() );
}

// -------------------------------------------------------------------------------- //
void guAlbumInfoCtrl::CreateContextMenu( wxMenu * Menu )
{
    wxMenuItem * MenuItem;

    if( m_Info->m_AlbumId != wxNOT_FOUND )
    {
        MenuItem = new wxMenuItem( Menu, ID_LASTFM_PLAY, _( "Play" ), _( "Play the artist tracks" ) );
        MenuItem->SetBitmap( wxBitmap( guImage_min_media_playback_start ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_LASTFM_ENQUEUE, _( "Enqueue" ), _( "Enqueue the artist tracks to the playlist" ) );
        MenuItem->SetBitmap( wxBitmap( guImage_vol_add ) );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();
    }

    if( !m_Info->m_Album->m_Url.IsEmpty() )
    {
        MenuItem = new wxMenuItem( Menu, ID_LASTFM_VISIT_URL, wxT( "Last.fm" ), _( "Visit last.fm page for this item" ) );
        MenuItem->SetBitmap( wxBitmap( guImage_lastfm_as_on ) );
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
// guSimilarArtistInfoCtrl
// -------------------------------------------------------------------------------- //
guSimilarArtistInfoCtrl::guSimilarArtistInfoCtrl( wxWindow * parent, DbLibrary * db, guPlayerPanel * playerpanel ) :
                  guLastFMInfoCtrl( parent, db, playerpanel )
{
    m_Info = NULL;

    m_Text->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( guSimilarArtistInfoCtrl::OnClick ), NULL, this );
    m_Bitmap->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( guSimilarArtistInfoCtrl::OnClick ), NULL, this );
    Connect( ID_LASTFM_SEARCH_LINK, ID_LASTFM_SEARCH_LINK + 999, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guSimilarArtistInfoCtrl::OnSearchLinkClicked ) );
    Connect( ID_LASTFM_VISIT_URL, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guSimilarArtistInfoCtrl::OnSearchLinkClicked ) );
}

// -------------------------------------------------------------------------------- //
guSimilarArtistInfoCtrl::~guSimilarArtistInfoCtrl()
{
    if( m_Info )
        delete m_Info;

    m_Text->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( guSimilarArtistInfoCtrl::OnClick ), NULL, this );
    m_Bitmap->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( guSimilarArtistInfoCtrl::OnClick ), NULL, this );
    Disconnect( ID_LASTFM_SEARCH_LINK, ID_LASTFM_SEARCH_LINK + 999, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guSimilarArtistInfoCtrl::OnSearchLinkClicked ) );
    Disconnect( ID_LASTFM_VISIT_URL, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guSimilarArtistInfoCtrl::OnSearchLinkClicked ) );
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
    SetLabel( wxString::Format( wxT( "%s\n%s%%" ), m_Info->m_Artist->m_Name.c_str(), m_Info->m_Artist->m_Match.c_str() ) );
}

// -------------------------------------------------------------------------------- //
void guSimilarArtistInfoCtrl::Clear( void )
{
    guLastFMInfoCtrl::Clear();
    if( m_Info )
        delete m_Info;
    m_Info = NULL;
}

// -------------------------------------------------------------------------------- //
void guSimilarArtistInfoCtrl::OnClick( wxMouseEvent &event )
{
    if( m_Info )
    {
        //guLogMessage( wxT( "guArtistInfo::OnClick %s" ), Info->Artist->Url.c_str() );
        guWebExecute( m_Info->m_Artist->m_Url );
    }
}

// -------------------------------------------------------------------------------- //
wxString guSimilarArtistInfoCtrl::GetSearchText( void )
{
    return m_Info->m_Artist->m_Name;
}

// -------------------------------------------------------------------------------- //
void guSimilarArtistInfoCtrl::CreateContextMenu( wxMenu * Menu )
{
    wxMenuItem * MenuItem;

    if( m_Info->m_ArtistId != wxNOT_FOUND )
    {
        MenuItem = new wxMenuItem( Menu, ID_LASTFM_PLAY, _( "Play" ), _( "Play the artist tracks" ) );
        MenuItem->SetBitmap( wxBitmap( guImage_min_media_playback_start ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_LASTFM_ENQUEUE, _( "Enqueue" ), _( "Enqueue the artist tracks to the playlist" ) );
        MenuItem->SetBitmap( wxBitmap( guImage_vol_add ) );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();
    }

    if( !m_Info->m_Artist->m_Url.IsEmpty() )
    {
        MenuItem = new wxMenuItem( Menu, ID_LASTFM_VISIT_URL, wxT( "Last.fm" ), _( "Visit last.fm page for this item" ) );
        MenuItem->SetBitmap( wxBitmap( guImage_lastfm_as_on ) );
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
    if( m_Info->m_TrackId != wxNOT_FOUND )
    {
        wxArrayInt Selections;
        Selections.Add( m_Info->m_TrackId );
        return m_Db->GetArtistsSongs( Selections, tracks );
    }
}

// -------------------------------------------------------------------------------- //
// guTrackInfoCtrl
// -------------------------------------------------------------------------------- //
guTrackInfoCtrl::guTrackInfoCtrl( wxWindow * parent, DbLibrary * db, guPlayerPanel * playerpanel ) :
                 guLastFMInfoCtrl( parent, db, playerpanel )
{
    m_Info = NULL;

    m_Text->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( guTrackInfoCtrl::OnClick ), NULL, this );
    m_Bitmap->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( guTrackInfoCtrl::OnClick ), NULL, this );
    Connect( ID_LASTFM_SEARCH_LINK, ID_LASTFM_SEARCH_LINK + 999, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTrackInfoCtrl::OnSearchLinkClicked ) );
    Connect( ID_LASTFM_VISIT_URL, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTrackInfoCtrl::OnSearchLinkClicked ) );
}

// -------------------------------------------------------------------------------- //
guTrackInfoCtrl::~guTrackInfoCtrl()
{
    if( m_Info )
        delete m_Info;

    m_Text->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( guTrackInfoCtrl::OnClick ), NULL, this );
    m_Bitmap->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( guTrackInfoCtrl::OnClick ), NULL, this );
    Disconnect( ID_LASTFM_SEARCH_LINK, ID_LASTFM_SEARCH_LINK + 999, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTrackInfoCtrl::OnSearchLinkClicked ) );
    Disconnect( ID_LASTFM_VISIT_URL, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guTrackInfoCtrl::OnSearchLinkClicked ) );
}

// -------------------------------------------------------------------------------- //
void guTrackInfoCtrl::SetInfo( guLastFMTrackInfo * info )
{
    if( m_Info )
        delete m_Info;
    m_Info = info;

    m_Info->m_TrackId = m_Db->FindTrack( m_Info->m_Track->m_ArtistName, m_Info->m_Track->m_TrackName );
    m_Text->SetForegroundColour( m_Info->m_TrackId == wxNOT_FOUND ?
                                       m_NotFoundColor : m_NormalColor );

    SetBitmap( m_Info->m_Image );
    SetLabel( wxString::Format( wxT( "%s\n%s\n%s%%" ),
        m_Info->m_Track->m_TrackName.c_str(),
        m_Info->m_Track->m_ArtistName.c_str(),
        m_Info->m_Track->m_Match.c_str() ) );
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
void guTrackInfoCtrl::OnClick( wxMouseEvent &event )
{
    if( m_Info )
    {
        //guLogMessage( wxT( "guArtistInfo::OnClick %s" ), Info->Artist->Url.c_str() );
        guWebExecute( m_Info->m_Track->m_Url );
    }
}

// -------------------------------------------------------------------------------- //
wxString guTrackInfoCtrl::GetSearchText( void )
{
    return wxString::Format( wxT( "%s %s" ), m_Info->m_Track->m_ArtistName.c_str(), m_Info->m_Track->m_TrackName.c_str() );
}

// -------------------------------------------------------------------------------- //
void guTrackInfoCtrl::CreateContextMenu( wxMenu * Menu )
{
    wxMenuItem * MenuItem;

    if( m_Info->m_TrackId != wxNOT_FOUND )
    {
        MenuItem = new wxMenuItem( Menu, ID_LASTFM_PLAY, _( "Play" ), _( "Play the artist tracks" ) );
        MenuItem->SetBitmap( wxBitmap( guImage_min_media_playback_start ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_LASTFM_ENQUEUE, _( "Enqueue" ), _( "Enqueue the artist tracks to the playlist" ) );
        MenuItem->SetBitmap( wxBitmap( guImage_vol_add ) );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();
    }

    if( !m_Info->m_Track->m_Url.IsEmpty() )
    {
        MenuItem = new wxMenuItem( Menu, ID_LASTFM_VISIT_URL, wxT( "Last.fm" ), _( "Visit last.fm page for this item" ) );
        MenuItem->SetBitmap( wxBitmap( guImage_lastfm_as_on ) );
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
void guTrackInfoCtrl::GetSelectedTracks( guTrackArray * tracks )
{
    if( m_Info->m_TrackId != wxNOT_FOUND )
    {
        wxArrayInt Selections;
        Selections.Add( m_Info->m_TrackId );
        return m_Db->GetSongs( Selections, tracks );
    }
}

// -------------------------------------------------------------------------------- //
// guLastFMPanel
// -------------------------------------------------------------------------------- //
guLastFMPanel::guLastFMPanel( wxWindow * Parent, DbLibrary * db, guPlayerPanel * playerpanel ) :
    wxScrolledWindow( Parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL )
{
    m_Db = db;
    m_PlayerPanel = playerpanel;
    m_UpdateTracks = true;

    m_ShowArtistDetails = true;
    m_ShowAlbums = true;
    m_ShowArtists = true;
    m_ShowTracks = true;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        m_ShowArtistDetails = Config->ReadBool( wxT( "LFMShowArtistInfo" ), true, wxT( "General" )  );
        m_ShowAlbums = Config->ReadBool( wxT( "LFMShowAlbums" ), true, wxT( "General" )  );
        m_ShowArtists = Config->ReadBool( wxT( "LFMShowArtists" ), true, wxT( "General" )  );
        m_ShowTracks = Config->ReadBool( wxT( "LFMShowTracks" ), true, wxT( "General" )  );
    }

	m_MainSizer = new wxBoxSizer( wxVERTICAL );

    // Artist Info
	wxBoxSizer * ArInfoTitleSizer;
	ArInfoTitleSizer = new wxBoxSizer( wxHORIZONTAL );

	m_ArtistDetailsStaticText = new wxStaticText( this, wxID_ANY, _("Artist Info"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ArtistDetailsStaticText->Wrap( -1 );
	m_ArtistDetailsStaticText->SetFont( wxFont( GULASTFM_TITLE_FONT_SIZE, 70, 90, 92, false, wxEmptyString ) );

	ArInfoTitleSizer->Add( m_ArtistDetailsStaticText, 0, wxALL, 5 );

	wxStaticLine * ArInfoStaticLine = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	ArInfoTitleSizer->Add( ArInfoStaticLine, 1, wxEXPAND | wxALL, 5 );

	m_LastFMPlayBitmapBtn = new wxBitmapButton( this, wxID_ANY, wxBitmap( guImage_min_media_playback_start ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_LastFMPlayBitmapBtn->SetToolTip( _( "Enables or Disables the update of this page when the current playing track changes" ) );
	ArInfoTitleSizer->Add( m_LastFMPlayBitmapBtn, 0, wxRIGHT, 5 );

	m_MainSizer->Add( ArInfoTitleSizer, 0, wxEXPAND, 5 );

	m_ArtistInfoMainSizer = new wxBoxSizer( wxVERTICAL );

	m_ArtistInfoCtrl = new guArtistInfoCtrl( this, m_Db, m_PlayerPanel );

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
	m_AlbumsStaticText->SetFont( wxFont( GULASTFM_TITLE_FONT_SIZE, 70, 90, 92, false, wxEmptyString ) );

	AlTitleSizer->Add( m_AlbumsStaticText, 0, wxALL, 5 );

	wxStaticLine * AlStaticLine = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	AlTitleSizer->Add( AlStaticLine, 1, wxEXPAND | wxALL, 5 );

	m_MainSizer->Add( AlTitleSizer, 0, wxEXPAND, 5 );

	m_AlbumsSizer = new wxGridSizer( 4, 3, 0, 0 );
    int index;
    for( index = 0; index < GULASTFMINFO_MAXITEMS; index++ )
    {
        m_AlbumInfoCtrls.Add( new guAlbumInfoCtrl( this, m_Db, m_PlayerPanel ) );
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
	m_ArtistsStaticText->SetFont( wxFont( GULASTFM_TITLE_FONT_SIZE, 70, 90, 92, false, wxEmptyString ) );

	SimArTitleSizer->Add( m_ArtistsStaticText, 0, wxALL, 5 );

	wxStaticLine * SimArStaticLine = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	SimArTitleSizer->Add( SimArStaticLine, 1, wxEXPAND | wxALL, 5 );

	m_MainSizer->Add( SimArTitleSizer, 0, wxEXPAND, 5 );

	m_ArtistsSizer = new wxGridSizer( 4, 3, 0, 0 );

    for( index = 0; index < GULASTFMINFO_MAXITEMS; index++ )
    {
        m_ArtistInfoCtrls.Add( new guSimilarArtistInfoCtrl( this, m_Db, m_PlayerPanel ) );
        m_ArtistsSizer->Add( m_ArtistInfoCtrls[ index ], 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
    }

	m_MainSizer->Add( m_ArtistsSizer, 0, wxEXPAND, 5 );

    if( m_ShowArtists )
        m_MainSizer->Show( m_ArtistsSizer );
    else
        m_MainSizer->Hide( m_ArtistsSizer );

    // Similar Tracks
	wxBoxSizer* SimTrTItleSizer;
	SimTrTItleSizer = new wxBoxSizer( wxHORIZONTAL );

	m_TracksStaticText = new wxStaticText( this, wxID_ANY, _("Similar Tracks"), wxDefaultPosition, wxDefaultSize, 0 );
	m_TracksStaticText->Wrap( -1 );
	m_TracksStaticText->SetFont( wxFont( GULASTFM_TITLE_FONT_SIZE, 70, 90, 92, false, wxEmptyString ) );

	SimTrTItleSizer->Add( m_TracksStaticText, 0, wxALL, 5 );

	wxStaticLine * SimTrStaticLine = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	SimTrTItleSizer->Add( SimTrStaticLine, 1, wxEXPAND | wxALL, 5 );

	m_MainSizer->Add( SimTrTItleSizer, 0, wxEXPAND, 5 );

	m_TracksSizer = new wxGridSizer( 4, 3, 0, 0 );

    for( index = 0; index < GULASTFMINFO_MAXITEMS; index++ )
    {
        m_TrackInfoCtrls.Add( new guTrackInfoCtrl( this, m_Db, m_PlayerPanel ) );
        m_TracksSizer->Add( m_TrackInfoCtrls[ index ], 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
    }

	m_MainSizer->Add( m_TracksSizer, 0, wxEXPAND, 5 );
	//MainSizer->Add( new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL ), 0, wxEXPAND, 5 );
    if( m_ShowTracks )
        m_MainSizer->Show( m_TracksSizer );
    else
        m_MainSizer->Hide( m_TracksSizer );

	this->SetSizer( m_MainSizer );
	this->Layout();

	SetScrollRate( 7, 7 );

    m_AlbumsUpdateThread = NULL;
    m_ArtistsUpdateThread = NULL;
    m_TracksUpdateThread = NULL;

    Connect( ID_LASTFM_UPDATE_ALBUMINFO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLastFMPanel::OnUpdateAlbumItem ) );
    Connect( ID_LASTFM_UPDATE_ARTISTINFO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLastFMPanel::OnUpdateArtistInfo ) );
    Connect( ID_LASTFM_UPDATE_SIMARTIST, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLastFMPanel::OnUpdateArtistItem ) );
    Connect( ID_LASTFM_UPDATE_SIMTRACK, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLastFMPanel::OnUpdateTrackItem ) );

	m_ArtistDetailsStaticText->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guLastFMPanel::OnArInfoTitleDClicked ), NULL, this );
	m_LastFMPlayBitmapBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guLastFMPanel::OnLastFMPlayBitmapBtnClick ), NULL, this );
	m_AlbumsStaticText->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guLastFMPanel::OnTopAlbumsTitleDClick ), NULL, this );
	m_ArtistsStaticText->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guLastFMPanel::OnSimArTitleDClick ), NULL, this );
	m_TracksStaticText->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guLastFMPanel::OnSimTrTitleDClick ), NULL, this );

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

	m_AlbumsStaticText->Disconnect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guLastFMPanel::OnTopAlbumsTitleDClick ), NULL, this );

    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        Config->WriteBool( wxT( "LFMShowArtistInfo" ), m_ShowArtistDetails, wxT( "General" ) );
        Config->WriteBool( wxT( "LFMShowAlbums" ), m_ShowAlbums, wxT( "General" ) );
        Config->WriteBool( wxT( "LFMShowArtists" ), m_ShowArtists, wxT( "General" ) );
        Config->WriteBool( wxT( "LFMShowTracks" ), m_ShowTracks, wxT( "General" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::SetTrack( const wxString &artistname, const wxString &trackname )
{
    if( !m_UpdateTracks )
        return;

    int index;

    m_ArtistName = artistname;
    m_TrackName = trackname;
    //guLogMessage( wxT( "LastFMPanel:SetTrack( '%s', '%s' )" ), artistname.c_str(), trackname.c_str() );

    if( m_LastArtistName != m_ArtistName )
    {
        m_ArtistsUpdateThreadMutex.Lock();
        if( m_ArtistsUpdateThread )
        {
            m_ArtistsUpdateThread->Pause();
            m_ArtistsUpdateThread->Delete();
        }
        m_ArtistsUpdateThreadMutex.Unlock();

        m_AlbumsUpdateThreadMutex.Lock();
        if( m_AlbumsUpdateThread )
        {
            m_AlbumsUpdateThread->Pause();
            m_AlbumsUpdateThread->Delete();
        }
        m_AlbumsUpdateThreadMutex.Unlock();

        // Clear the LastFM controls to default values
        m_ArtistInfoCtrl->Clear();

        for( index = 0; index < GULASTFMINFO_MAXITEMS; index++ )
        {
            m_ArtistInfoCtrls[ index ]->Clear();
            m_AlbumInfoCtrls[ index ]->Clear();
        }

        m_ArtistsUpdateThread = new guFetchSimilarArtistInfoThread( this, artistname.c_str() );

        m_AlbumsUpdateThread = new guFetchAlbumInfoThread( this, artistname.c_str() );

        m_LastArtistName = m_ArtistName;

    }

    if( m_LastTrackName != m_TrackName )
    {
        m_TracksUpdateThreadMutex.Lock();
        if( m_TracksUpdateThread )
        {
            m_TracksUpdateThread->Pause();
            m_TracksUpdateThread->Delete();
        }
        m_TracksUpdateThreadMutex.Unlock();

        for( index = 0; index < GULASTFMINFO_MAXITEMS; index++ )
        {
            m_TrackInfoCtrls[ index ]->Clear();
        }

        m_TracksUpdateThread = new guFetchTrackInfoThread( this, artistname.c_str(), trackname.c_str() );
        m_LastTrackName = m_TrackName;

    }
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnUpdatedTrack( wxCommandEvent &event )
{
    // Player informs there is a new track playing
    //guLogMessage( wxT( "Received LastFMPanel::UpdateTrack event" ) );
    const wxArrayString * Params = ( wxArrayString * ) event.GetClientData();
    SetTrack( ( * Params )[ 0 ], ( * Params )[ 1 ] );
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
        Layout();
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
        Layout();
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
        Layout();
    }
    else
    {
        guLogError( wxT( "Received a LastFMInfo event with NULL Data" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guLastFMPanel::OnLastFMPlayBitmapBtnClick( wxCommandEvent &event )
{
    m_UpdateTracks = !m_UpdateTracks;
    m_LastFMPlayBitmapBtn->SetLabel( m_UpdateTracks ? guImage_min_media_playback_start :
                                                  guImage_min_media_playback_pause );
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
    //AlbumsSizer->Show( ShowAlbums );
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
    //ArtistsSizer->Show( ShowArtists );
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
    //TracksSizer->Show( ShowTracks );
    if( m_ShowTracks )
        m_MainSizer->Show( m_TracksSizer );
    else
        m_MainSizer->Hide( m_TracksSizer );
    UpdateLayout();
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
        Sleep( 10 );
    }
}


// -------------------------------------------------------------------------------- //
// guDownloadImageThread
// -------------------------------------------------------------------------------- //
guDownloadImageThread::guDownloadImageThread( guLastFMPanel * lastfmpanel, guFetchLastFMInfoThread * mainthread,
    const int index, const wxChar * imageurl, int commandid, void * commanddata, wxImage ** pimage, const wxSize &scalesize ) :
    wxThread( wxTHREAD_DETACHED )
{
    m_LastFMPanel = lastfmpanel;
    m_MainThread  = mainthread;
    m_CommandId   = commandid;
    m_CommandData = commanddata;
    m_pImage      = pimage;
    m_Index       = index;
    m_ImageUrl    = wxString( imageurl );
    m_ScaleSize   = scalesize;
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
    wxImage *       Image = NULL;
    long            ImageType;

    //wxASSERT( pImage );

    if( !m_ImageUrl.IsEmpty() )
    {
        if( m_ImageUrl.Lower().EndsWith( wxT( ".jpg" ) ) )
          ImageType = wxBITMAP_TYPE_JPEG;
        else if( m_ImageUrl.Lower().EndsWith( wxT( ".png" ) ) )
          ImageType = wxBITMAP_TYPE_PNG;
        else if( m_ImageUrl.Lower().EndsWith( wxT( ".gif" ) ) )    // Removed because of some random segfaults
          ImageType = wxBITMAP_TYPE_GIF;                                  // in gifs handler functions
        else if( m_ImageUrl.Lower().EndsWith( wxT( ".bmp" ) ) )
          ImageType = wxBITMAP_TYPE_BMP;
        else
          ImageType = wxBITMAP_TYPE_INVALID;

        if( ImageType > wxBITMAP_TYPE_INVALID )
        {
            wxMemoryOutputStream Buffer;
            wxCurlHTTP http;
            if( http.Get( Buffer, m_ImageUrl ) )
            {
                if( Buffer.IsOk() && !TestDestroy() )
                {
                    wxMemoryInputStream Ins( Buffer );
                    if( Ins.IsOk() && !TestDestroy() )
                    {
                        Image = new wxImage( Ins, ImageType );
                        if( Image )
                        {
                            //guLogMessage( wxT( "Image loaded ok %u" ), Index );
                            if( Image->IsOk() && ( m_ScaleSize.x > 0 ) && !TestDestroy() )
                            {
                                Image->Rescale( m_ScaleSize.x, m_ScaleSize.y, wxIMAGE_QUALITY_HIGH );
                            }
                            else
                            {
                              delete Image;
                              Image = NULL;
                            }
                        }
                    }
                }
            }
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
    else if( Image )
    {
        delete Image;
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
// guFetchAlbumInfoThread
// -------------------------------------------------------------------------------- //
guFetchAlbumInfoThread::guFetchAlbumInfoThread( guLastFMPanel * lastfmpanel,
                            const wxChar * artistname ) :
    guFetchLastFMInfoThread( lastfmpanel )
{
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
                            guDownloadImageThread * DownloadImageThread = new guDownloadImageThread(
                                    m_LastFMPanel,
                                    this,
                                    index,
                                    TopAlbums[ index ].m_ImageLink.c_str(),
                                    ID_LASTFM_UPDATE_ALBUMINFO,
                                    LastFMAlbumInfo,
                                    &LastFMAlbumInfo->m_Image, wxSize( 50, 50 ) );
                            if( !DownloadImageThread )
                            {
                                guLogError( wxT( "Could not create the album image download thread %u" ), index );
                            }
                        }
                    }
                    m_DownloadThreadsMutex.Unlock();
                    if( TestDestroy() )
                        break;
                    Sleep( 100 );
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
                                                      const wxChar * artistname ) :
    guFetchLastFMInfoThread( lastfmpanel )
{
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
                    guDownloadImageThread * DownloadImageThread = new guDownloadImageThread(
                        m_LastFMPanel,
                        this,
                        0,
                        ArtistInfo.m_ImageLink.c_str(),
                        ID_LASTFM_UPDATE_ARTISTINFO,
                        LastFMArtistInfo,
                        &LastFMArtistInfo->m_Image, wxSize( 100, 100 ) );
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
                            guDownloadImageThread* DownloadImageThread = new guDownloadImageThread(
                                m_LastFMPanel,
                                this,
                                index,
                                SimilarArtists[ index ].m_ImageLink.c_str(),
                                ID_LASTFM_UPDATE_SIMARTIST,
                                LastFMArtistInfo,
                                &LastFMArtistInfo->m_Image, wxSize( 50, 50 ) );
                            if( !DownloadImageThread )
                            {
                                guLogError( wxT( "Could not create the similar artist image download thread %u" ), index );
                            }
                        }
                    }
                    m_DownloadThreadsMutex.Unlock();
                    if( TestDestroy() )
                        break;
                    Sleep( 100 );
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
                            const wxChar * artistname, const wxChar * trackname ) :
    guFetchLastFMInfoThread( lastfmpanel )
{
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
                            guDownloadImageThread * DownloadImageThread = new guDownloadImageThread(
                                    m_LastFMPanel,
                                    this, index, SimilarTracks[ index ].m_ImageLink.c_str(),
                                    ID_LASTFM_UPDATE_SIMTRACK,
                                    LastFMTrackInfo,
                                    &LastFMTrackInfo->m_Image, wxSize( 50, 50 ) );
                            if( !DownloadImageThread )
                            {
                                guLogError( wxT( "Could not create the track image download thread %u" ), index );
                            }
                        }
                    }
                    m_DownloadThreadsMutex.Unlock();
                    if( TestDestroy() )
                        break;
                    Sleep( 100 );
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
