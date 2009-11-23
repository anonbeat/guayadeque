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
#include "LyricsPanel.h"

#include "Base64.h"
#include "Commands.h"
#include "Config.h"
#include "Images.h"
#include "Utils.h"

#include <wx/curl/http.h>
#include <wx/settings.h>
#include <wx/xml/xml.h>

// -------------------------------------------------------------------------------- //
guLyricsPanel::guLyricsPanel( wxWindow * parent ) :
    wxPanel( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL )
{
    m_LyricThread = NULL;
    m_UpdateEnabled = true;

    wxFont CurrentFont = wxSystemSettings::GetFont( wxSYS_SYSTEM_FONT );

	wxBoxSizer * MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer * TitleSizer;
	TitleSizer = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer * EditorSizer;
	EditorSizer = new wxBoxSizer( wxHORIZONTAL );

	m_UpdateCheckBox = new wxCheckBox( this, wxID_ANY, _( "Follow player" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_UpdateCheckBox->SetValue( m_UpdateEnabled );

	EditorSizer->Add( m_UpdateCheckBox, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

	m_ReloadButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_reload ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
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

	TitleSizer->Add( EditorSizer, 1, wxEXPAND, 5 );

	wxStaticLine * TopLine;
	TopLine = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	TitleSizer->Add( TopLine, 0, wxEXPAND|wxALL, 5 );

    m_LyricTitle = new wxStaticText( this, wxID_ANY, wxT( "/" ) );
    CurrentFont.SetPointSize( 12 );
    CurrentFont.SetWeight( wxFONTWEIGHT_BOLD );
	m_LyricTitle->SetFont( CurrentFont );

	TitleSizer->Add( m_LyricTitle, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );

	MainSizer->Add( TitleSizer, 0, wxEXPAND, 5 );

	m_LyricText = new wxHtmlWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_AUTO );
	m_LyricText->SetBorders( 0 );
    m_LyricText->SetPage( wxString::Format( wxT( "<html><body bgcolor=%s></body></html>" ),
          wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWFRAME ).GetAsString( wxC2S_HTML_SYNTAX ).c_str() ) );
	m_LyricText->SetFonts( CurrentFont.GetFaceName(), wxEmptyString );

	MainSizer->Add( m_LyricText, 1, wxALL|wxEXPAND, 5 );

	this->SetSizer( MainSizer );
	this->Layout();


	m_UpdateCheckBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guLyricsPanel::OnUpdateChkBoxClicked ), NULL, this );
	m_ReloadButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guLyricsPanel::OnReloadBtnClick ), NULL, this );
	m_ArtistTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guLyricsPanel::OnTextUpdated ), NULL, this );
	m_TrackTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guLyricsPanel::OnTextUpdated ), NULL, this );
	m_SearchButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guLyricsPanel::OnSearchBtnClick ), NULL, this );
    Connect( ID_LYRICS_UPDATE_LYRICINFO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLyricsPanel::OnDownloadedLyric ) );
}

// -------------------------------------------------------------------------------- //
guLyricsPanel::~guLyricsPanel()
{
    if( m_LyricThread )
    {
        m_LyricThread->Pause();
        m_LyricThread->Delete();
    }
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::OnUpdatedTrack( wxCommandEvent &event )
{
    if( m_UpdateEnabled )
    {
        const wxArrayString * Params = ( wxArrayString * ) event.GetClientData();
        if( !Params )
        {
            SetTrack( wxEmptyString, wxEmptyString );
        }
        else
        {
            SetTrack( ( * Params )[ 0 ], ( * Params )[ 1 ] );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::OnTextUpdated( wxCommandEvent& event )
{
    m_SearchButton->Enable( !m_UpdateEnabled &&
                            !m_ArtistTextCtrl->GetValue().IsEmpty() &&
                            !m_TrackTextCtrl->GetValue().IsEmpty() );
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::OnUpdateChkBoxClicked( wxCommandEvent& event )
{
    m_UpdateEnabled = m_UpdateCheckBox->IsChecked();
    m_ArtistTextCtrl->Enable( !m_UpdateEnabled );
    m_TrackTextCtrl->Enable( !m_UpdateEnabled );
    m_SearchButton->Enable( !m_UpdateEnabled &&
                            !m_ArtistTextCtrl->GetValue().IsEmpty() &&
                            !m_TrackTextCtrl->GetValue().IsEmpty() );
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::OnReloadBtnClick( wxCommandEvent& event )
{
    SetTrack( m_ArtistTextCtrl->GetValue(), m_TrackTextCtrl->GetValue() );
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::OnSearchBtnClick( wxCommandEvent& event )
{
    SetTrack( m_ArtistTextCtrl->GetValue(), m_TrackTextCtrl->GetValue() );
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::SetTitle( const wxString &title )
{
    wxString Label = title;
    Label.Replace( wxT( "&" ), wxT( "&&" ) );
    m_LyricTitle->SetLabel( Label );
    Layout();
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::SetText( const wxString &text )
{
    m_LyricText->SetPage( wxString::Format( wxT( "<html><body bgcolor=%s><center><font color=%s size=\"+1\">%s</font></center></body></html>" ),
          wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWFRAME ).GetAsString( wxC2S_HTML_SYNTAX ).c_str(),
          wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT ).GetAsString( wxC2S_HTML_SYNTAX ).c_str(),
          text.c_str() ) );
    Layout();
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::SetTrack( const wxString &artist, const wxString &track )
{
    SetTitle( track + wxT( " / " ) + artist );
    //SetText( _( "No lyrics found for this song." ) );

    m_ArtistTextCtrl->SetValue( artist );
    m_TrackTextCtrl->SetValue( track );
    if( !artist.IsEmpty() && !track.IsEmpty() )
    {
        SetText( _( "Searching the lyrics for this track" ) );

        guConfig * Config = ( guConfig * ) Config->Get();

        int Engine = Config->ReadNum( wxT( "LyricSearchEngine" ), 0, wxT( "General" ) );
        if( Engine == guLYRIC_ENGINE_LYRICWIKI )
        {
            m_LyricThread = new guLyricWikiEngine( this, artist.c_str(), track.c_str() );
        }
        else if( Engine == guLYRIC_ENGINE_LEOSLYRICS )
        {
            m_LyricThread = new guLeosLyricsEngine( this, artist.c_str(), track.c_str() );
        }
//        else if( Engine == guLYRIC_ENGINE_LYRC_COM_AR )
//        {
//            m_LyricThread = new guLyrcComArEngine( this, artist.c_str(), track.c_str() );
//        }
        else //if( Engine == guLYRIC_ENGINE_CDUNIVERSE )
        {
            m_LyricThread = new guCDUEngine( this, artist.c_str(), track.c_str() );
        }
    }
    else
    {
        SetText( wxEmptyString );
    }
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::ClearLyricThread( void )
{
    m_LyricThread = NULL;
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::OnDownloadedLyric( wxCommandEvent &event )
{
    wxString * Content = ( wxString * ) event.GetClientData();
    if( Content )
    {
        SetText( * Content );
        delete Content;
    }
    else
    {
        SetText( _( "No lyrics found for this song." ) );
    }
}

// -------------------------------------------------------------------------------- //
//
// -------------------------------------------------------------------------------- //
guSearchLyricEngine::guSearchLyricEngine( guLyricsPanel * lyricpanel, const wxChar * artistname, const wxChar * trackname ) :
    wxThread()
{
    wxASSERT( lyricpanel );
    wxASSERT( artistname );
    wxASSERT( trackname );

    m_LyricsPanel = lyricpanel;
    m_ArtistName = artistname;
    m_TrackName = trackname;
}

// -------------------------------------------------------------------------------- //
guSearchLyricEngine::~guSearchLyricEngine()
{
    if( !TestDestroy() )
    {
        m_LyricsPanel->ClearLyricThread();
    }
}

// -------------------------------------------------------------------------------- //
guSearchLyricEngine::ExitCode guSearchLyricEngine::Entry()
{
    if( !TestDestroy() )
    {
        SearchLyric();
    }
    return 0;
}

//// -------------------------------------------------------------------------------- //
//void guSearchLyricEngine::SearchLyric( void )
//{
//}

// -------------------------------------------------------------------------------- //
void guSearchLyricEngine::SetLyric( wxString * lyrictext )
{
    if( !TestDestroy() )
    {
        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_LYRICS_UPDATE_LYRICINFO );
        event.SetClientData( ( void * ) lyrictext );
        wxPostEvent( m_LyricsPanel, event );
    }
}


// -------------------------------------------------------------------------------- //
// guLyricWikiEngine
// -------------------------------------------------------------------------------- //
guLyricWikiEngine::guLyricWikiEngine( guLyricsPanel * lyricpanel, const wxChar * artistname, const wxChar * trackname ) :
    guSearchLyricEngine( lyricpanel, artistname, trackname )
{
    if( Create() == wxTHREAD_NO_ERROR )
    {
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guLyricWikiEngine::~guLyricWikiEngine()
{
}

//// -------------------------------------------------------------------------------- //
//wxString ProcessHexData( const wxString hexstr )
//{
//    int index;
//    wxString RetVal = hexstr;
//    for( index = 0; index < 256; index++ )
//    {
//        wxString Old = wxString::Format( wxT( "&#%u;" ), index );
//        wxString New = wxString::Format( wxT( "%c" ), index );
//        RetVal.Replace( Old, New );
//    }
//    return RetVal;
//}

// -------------------------------------------------------------------------------- //
void guLyricWikiEngine::SearchLyric( void )
{
    int         StartPos;
    int         EndPos;
    wxString    Content;
     //http://lyrics.wikia.com/index.php?title=U2:Where_The_Streets_Has_No_Name&action=edit
    wxString ArtistName = m_ArtistName;
    ArtistName.Replace( wxT( " " ), wxT( "_" ) );
    wxString TrackName = m_TrackName;
    TrackName.Replace( wxT( " " ), wxT( "_" ) );

    wxString    UrlStr = wxString::Format( wxT( "http://lyricwiki.org/index.php?title=%s:%s" ),
                        guURLEncode( ArtistName ).c_str(), guURLEncode( TrackName ).c_str() );

    //guLogMessage( wxT( "LyricWiky searching... %s" ), UrlStr.c_str() );

    //while( !TestDestroy() )
    {
        Content = GetUrlContent( UrlStr, wxEmptyString, true );
        //
        if( !Content.IsEmpty() )
        {
            //guLogMessage( wxT( "Content:\n%s" ), Content.c_str() );
            StartPos = Content.Find( wxT( "<div class='lyricbox'" ) );
            if( StartPos != wxNOT_FOUND )
            {
                Content = Content.Mid( StartPos );
                EndPos = Content.Find( wxT( "<!--" ) );
                Content = Content.Mid( 0, EndPos );
                StartPos = Content.Find( wxT( "</div>" ) );

                if( StartPos != wxNOT_FOUND )
                {
                    Content = Content.Mid( StartPos + 6 );

                    //Content = ProcessHexData( Content );
                    Content.Replace( wxT( "\n" ), wxT( "<br>" ) );
                    SetLyric( new wxString( Content.c_str() ) );
                }

//                Content.Replace( wxT( "\n" ), wxT( "<br>" ) );
//                SetLyric( new wxString( Content.c_str() ) );
                //break;





            }
////            else if( Content.Find( wxT( "#REDIRECT" ) ) != wxNOT_FOUND )
////            {
////                guLogMessage( wxT( "Redirection found..." ) );
////                StartPos = Content.Find( wxT( "url: " ) );
////                if( StartPos != wxNOT_FOUND )
////                {
////                    Content = Content.Mid( StartPos );
////                    StartPos = Content.Find( wxT( "href='" ) );
////                    if( StartPos != wxNOT_FOUND )
////                    {
////                        EndPos = Content.Find( wxT( "' title" ) );
////                        StartPos += 6;
////                        Content = Content.Mid( StartPos, EndPos - StartPos );
////                        UrlStr = Content;
////                    }
////                }
////            }
////            else
////            {
//////                StartPos = Content.Find( wxT( "<pre>" ) );
//////                EndPos = Content.Find( wxT( "</pre>" ) );
//////                if( StartPos == wxNOT_FOUND || EndPos == wxNOT_FOUND )
//////                {
//////                    Content = _( "No lyrics found for this song." );
//////                }
//////                else
//////                {
//////                    StartPos += 5;
//////                    Content = Content.Mid( StartPos, EndPos - StartPos );
//////                    if( Content.Find( wxT( "Not found" ) ) != wxNOT_FOUND )
//////                    {
//////                        Content = _( "No lyrics found for this song." );
//////                    }
//////                }
//////
//////                Content.Replace( wxT( "\n" ), wxT( "<br>" ) );
//////
//////                if( !TestDestroy() )
//////                {
//////                    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_LYRICS_UPDATE_LYRICINFO );
//////                    event.SetClientData( new wxString( Content.c_str() ) );
//////                    wxPostEvent( m_LyricsPanel, event );
//////                }
////                StartPos = Content.Find( wxT( "<a href='" ) );
////                if( StartPos != wxNOT_FOUND )
////                {
////                    EndPos = Content.Find( wxT( "'>" ) );
////                    StartPos += 9;
////                    Content = Content.Mid( StartPos, EndPos - StartPos );
////                    UrlStr = Content;
////                }
        }
        else
        {
            guLogError( wxT( "Could not get the content of the lyrics." ) );
            if( !TestDestroy() )
            {
                SetLyric( NULL );
                //break;
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
// guLyricWikiEngine
// -------------------------------------------------------------------------------- //
guLeosLyricsEngine::guLeosLyricsEngine( guLyricsPanel * lyricpanel, const wxChar * artistname, const wxChar * trackname ) :
    guSearchLyricEngine( lyricpanel, artistname, trackname )
{
    if( Create() == wxTHREAD_NO_ERROR )
    {
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guLeosLyricsEngine::~guLeosLyricsEngine()
{
}

// -------------------------------------------------------------------------------- //
wxString guLeosLyricsEngine::GetLyricId( void )
{
    wxString RetVal = wxEmptyString;

    wxString LyricIdUrl = wxString::Format( wxT( "http://api.leoslyrics.com/api_search.php?auth=Guayadeque&artist=%s&songtitle=%s" ),
      guURLEncode( m_ArtistName ).c_str(), guURLEncode( m_TrackName ).c_str() );

    wxString Content = GetUrlContent( LyricIdUrl );
    if( !Content.IsEmpty() )
    {
        //guLogMessage( wxT( "%s" ), LyricIdText.c_str() );
//        <?xml version="1.0" encoding="UTF-8"?>
//        <leoslyrics>
//          <response code="0">SUCCESS</response>
//          <searchResults>
//            <result id="68822" hid="RHNi62g2zmQ=" exactMatch="true">
//              <title>Home</title>
//              <feat />
//              <artist>
//                <name>Sarah McLachlan</name>
//              </artist>
//            </result>
//          </searchResults>
//        </leoslyrics>

        wxStringInputStream ins( Content );
        wxXmlDocument XmlDoc( ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        wxString Status;
        if( XmlNode && XmlNode->GetName() == wxT( "leoslyrics" ) )
        {
            XmlNode = XmlNode->GetChildren();
            while( XmlNode )
            {
                //guLogMessage( wxT( "Name: %s" ), XmlNode->GetName().c_str() );
                if( XmlNode->GetName() == wxT( "response" ) )
                {
                    //guLogMessage( wxT( "Result: %s" ), XmlNode->GetNodeContent().c_str() );
                    if( XmlNode->GetNodeContent() != wxT( "SUCCESS" ) )
                    {
                        break;
                    }
                }
                else if( XmlNode->GetName() == wxT( "searchResults" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    while( XmlNode )
                    {
                        //guLogMessage( wxT( "SubName: %s" ), XmlNode->GetName().c_str() );
                        if( XmlNode->GetName() == wxT( "result" ) )
                        {
                            XmlNode->GetPropVal( wxT( "exactMatch" ), &Status );
                            if( Status == wxT( "true" ) )
                            {
                                XmlNode->GetPropVal( wxT( "hid" ), &Status );
                                //guLogMessage( wxT( "Found it with id '%s'" ), Status.c_str() );
                                if( !Status.IsEmpty() )
                                    RetVal = Status;
                                break;
                            }
                        }
                        XmlNode = XmlNode->GetNext();
                    }
                    break;
                }
                XmlNode = XmlNode->GetNext();
            }
        }
    }
    return RetVal;
}


// -------------------------------------------------------------------------------- //
wxString guLeosLyricsEngine::GetLyricText( const wxString &lyricid )
{
    wxString RetVal = wxEmptyString;

    wxString LyricIdUrl = wxString::Format( wxT( "http://api.leoslyrics.com/api_lyrics.php?auth=Guayadeque&hid=%s" ),
        lyricid.c_str() );

    wxString Content = GetUrlContent( LyricIdUrl );
    if( !Content.IsEmpty() )
    {
//        <?xml version="1.0" encoding="UTF-8"?>
//        <leoslyrics>
//          <response code="0">SUCCESS</response>
//          <lyric hid="VxwOBYpM3iY=" id="120741">
//            <title>Comfort Eagle</title>
//            <feat />
//            <artist>
//              <name>Cake</name>
//
//            </artist>
//            <albums>
//              <album>
//                <name>Comfort Eagle</name>
//                <imageUrl>http://images.amazon.com/images/P/B00005MCW5.01.MZZZZZZZ.jpg</imageUrl>
//              </album>
//            </albums>
//            <writer />
//
//            <text>LyricText&#xD;
//            </text>
//          </lyric>
//        </leoslyrics>
//
        wxStringInputStream ins( Content );
        wxXmlDocument XmlDoc( ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        wxString Status;
        if( XmlNode && XmlNode->GetName() == wxT( "leoslyrics" ) )
        {
            XmlNode = XmlNode->GetChildren();
            while( XmlNode )
            {
                //guLogMessage( wxT( "Name: %s" ), XmlNode->GetName().c_str() );
                if( XmlNode->GetName() == wxT( "response" ) )
                {
                    //guLogMessage( wxT( "Result: %s" ), XmlNode->GetNodeContent().c_str() );
                    if( XmlNode->GetNodeContent() != wxT( "SUCCESS" ) )
                    {
                        break;
                    }
                }
                else if( XmlNode->GetName() == wxT( "lyric" ) )
                {
                    XmlNode = XmlNode->GetChildren();
                    continue;
                }
                else if( XmlNode->GetName() == wxT( "text" ) )
                {
                    RetVal = XmlNode->GetNodeContent();
                    //RetVal.Replace( wxT( "&#xD" ), wxT( "<br>" ) );
                    RetVal.Replace( wxT( "\n" ), wxT( "<br>" ) );
                    //guLogMessage( wxT( "Result: %s" ), RetVal.c_str() );
                    break;
                }
                XmlNode = XmlNode->GetNext();
            }
        }
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guLeosLyricsEngine::SearchLyric( void )
{
    // 1
    //http://api.leoslyrics.com/api_search.php?auth=Guayadeque&artist=%s&songtitle=%s
    // 2
    //http://api.leoslyrics.com/api_lyrics.php?auth=Guayadeque&hid=%s
    //
    wxString LyricId = GetLyricId();
    if( !LyricId.IsEmpty() )
    {
        wxString LyricText = GetLyricText( LyricId );
        if( LyricText.IsEmpty() )
        {
            SetLyric( NULL );
        }
        else
        {
            SetLyric( new wxString( LyricText ) );
        }
    }
    else
    {
        SetLyric( NULL );
    }
}

//// -------------------------------------------------------------------------------- //
//// guLyrcComArEngine
//// -------------------------------------------------------------------------------- //
//guLyrcComArEngine::guLyrcComArEngine( guLyricsPanel * lyricpanel, const wxChar * artistname, const wxChar * trackname ) :
//    guSearchLyricEngine( lyricpanel, artistname, trackname )
//{
//    if( Create() == wxTHREAD_NO_ERROR )
//    {
//        Run();
//    }
//}
//
//// -------------------------------------------------------------------------------- //
//guLyrcComArEngine::~guLyrcComArEngine()
//{
//}
//
//// -------------------------------------------------------------------------------- //
//void guLyrcComArEngine::SearchLyric( void )
//{
//    int         StartPos;
//    int         EndPos;
//    wxString    Content;
//    wxString    UrlStr = wxString::Format( wxT( "http://lyrc.com.ar/en/tema1en.php?artist=%s&songname=%s" ),
//                        guURLEncode( m_ArtistName ).c_str(), guURLEncode( m_TrackName ).c_str() );
//
//    Content = GetUrlContent( UrlStr );
//    //
//    if( !Content.IsEmpty() )
//    {
//        StartPos = Content.Find( wxT( "</script></td></tr></table>" ) );
//        if( StartPos != wxNOT_FOUND )
//        {
//            Content = Content.Mid( StartPos + 27 );
//            EndPos = Content.Find( wxT( "<a href=\"#\"" ) );
//            Content = Content.Mid( 0, EndPos );
//            //Content.Replace( wxT( "\n" ), wxT( "<br>" ) );
//            SetLyric( new wxString( Content.c_str() ) );
//            return;
//        }
//    }
//    if( !TestDestroy() )
//    {
//        SetLyric( NULL );
//    }
//}

// -------------------------------------------------------------------------------- //
// guCDUEngine
// -------------------------------------------------------------------------------- //
guCDUEngine::guCDUEngine( guLyricsPanel * lyricpanel, const wxChar * artistname, const wxChar * trackname ) :
    guSearchLyricEngine( lyricpanel, artistname, trackname )
{
    if( Create() == wxTHREAD_NO_ERROR )
    {
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guCDUEngine::~guCDUEngine()
{
}

// -------------------------------------------------------------------------------- //
void guCDUEngine::SearchLyric( void )
{
    int         StartPos;
    int         EndPos;
    wxString    Content;
    wxString    UrlStr = wxString::Format( wxT( "http://www.cduniverse.com/lyricsearch.asp?artist=%s&song=%s" ),
                        guURLEncode( m_ArtistName ).c_str(), guURLEncode( m_TrackName ).c_str() );

    Content = GetUrlContent( UrlStr, wxT( "http://www.cduniverse.com/lyrics.asp?id=&style=music&pid=" ) );
    //
    if( !Content.IsEmpty() )
    {
        StartPos = Content.Find( wxT( "<div id=\"divEncodedLyrics\" style=\"display:none\">" ) );
        if( StartPos != wxNOT_FOUND )
        {
            Content = Content.Mid( StartPos + 48 );
            EndPos = Content.Find( wxT( "</div></body>" ) );
            Content = Content.Mid( 0, EndPos );
            if( !Content.IsEmpty() )
            {
                //guLogMessage( wxT( "Content = '%s'" ), Content.c_str() );
                wxMemoryBuffer LyricBuffer = guBase64Decode( Content );

                Content = wxString::FromUTF8( ( char * ) LyricBuffer.GetData(), LyricBuffer.GetDataLen() );
                Content.Replace( wxT( "\n" ), wxT( "<br>" ) );

                //printf( "Lyrics : %s\n", ( char * ) LyricBuffer.GetData() );

                SetLyric( new wxString( Content ) );
            }
            return;
        }
    }
    if( !TestDestroy() )
    {
        SetLyric( NULL );
    }
}

// -------------------------------------------------------------------------------- //
