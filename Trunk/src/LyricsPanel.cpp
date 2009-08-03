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

#include "Commands.h"
#include "Images.h"
#include "Utils.h"

#include <wx/curl/http.h>
#include <wx/settings.h>

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
        SetTrack( ( * Params )[ 0 ], ( * Params )[ 1 ] );
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
    SetText( _( "No lyrics found for this song." ) );
    m_LyricThread = new guFetchLyricThread( this, artist.c_str(), track.c_str() );
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
}

// -------------------------------------------------------------------------------- //
//
// -------------------------------------------------------------------------------- //
guFetchLyricThread::guFetchLyricThread( guLyricsPanel * lyricpanel, const wxChar * artistname, const wxChar * trackname ) :
    wxThread()
{
    wxASSERT( lyricpanel );
    wxASSERT( artistname );
    wxASSERT( trackname );

    m_LyricsPanel = lyricpanel;
    m_ArtistName = artistname;
    m_TrackName = trackname;
    if( Create() == wxTHREAD_NO_ERROR )
    {
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guFetchLyricThread::~guFetchLyricThread()
{
    if( !TestDestroy() )
    {
        m_LyricsPanel->ClearLyricThread();
    }
}

// -------------------------------------------------------------------------------- //
wxString GetUrlContent( const wxString &url )
{
    wxCurlHTTP  http;
    char *      Buffer;
    wxString RetVal = wxEmptyString;

    http.AddHeader( wxT( "User-Agent: Mozilla/5.0 (X11; U; Linux i686; es-ES; rv:1.9.0.5) Gecko/2008121622 Ubuntu/8.10 (intrepid) Firefox/3.0.5" ) );
    http.AddHeader( wxT( "Accept: text/html" ) );
    http.AddHeader( wxT( "Accept-Charset: utf-8" ) );
    Buffer = NULL;
    http.Get( Buffer, url );
    if( Buffer )
    {
        RetVal = wxString( Buffer, wxConvUTF8 );
        free( Buffer );
    }
    else
    {
        guLogError( wxT( "Could not get the lyrics" ) );
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
guFetchLyricThread::ExitCode guFetchLyricThread::Entry()
{
    int         StartPos;
    int         EndPos;
    wxString    Content;
    wxString    UrlStr = wxString::Format( wxT( "http://lyricwiki.org/api.php?func=getSong&artist=%s&song=%s" ),
                        guURLEncode( m_ArtistName ).c_str(), guURLEncode( m_TrackName ).c_str() );
    do {
        Content = GetUrlContent( UrlStr );
        //
        if( !Content.IsEmpty() )
        {
            guLogMessage( wxT( "Content:\n%s" ), Content.c_str() );
            StartPos = Content.Find( wxT( "<div class='lyricbox' >" ) );
            if( StartPos != wxNOT_FOUND )
            {
                Content = Content.Mid( StartPos + 23 );
                EndPos = Content.Find( wxT( "</div>" ) );
                Content = Content.Mid( 0, EndPos );

                Content.Replace( wxT( "\n" ), wxT( "<br>" ) );

                if( !TestDestroy() )
                {
                    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_LYRICS_UPDATE_LYRICINFO );
                    event.SetClientData( new wxString( Content.c_str() ) );
                    wxPostEvent( m_LyricsPanel, event );
                }
            }
            else if( Content.Find( wxT( "#REDIRECT" ) ) != wxNOT_FOUND )
            {
                //guLogMessage( wxT( "Redirection found..." ) );
                StartPos = Content.Find( wxT( "url: " ) );
                if( StartPos != wxNOT_FOUND )
                {
                    Content = Content.Mid( StartPos );
                    StartPos = Content.Find( wxT( "href='" ) );
                    if( StartPos != wxNOT_FOUND )
                    {
                        EndPos = Content.Find( wxT( "' title" ) );
                        StartPos += 6;
                        Content = Content.Mid( StartPos, EndPos - StartPos );
                        UrlStr = Content;
                        continue;
                    }
                }
            }
            else
            {
//                StartPos = Content.Find( wxT( "<pre>" ) );
//                EndPos = Content.Find( wxT( "</pre>" ) );
//                if( StartPos == wxNOT_FOUND || EndPos == wxNOT_FOUND )
//                {
//                    Content = _( "No lyrics found for this song." );
//                }
//                else
//                {
//                    StartPos += 5;
//                    Content = Content.Mid( StartPos, EndPos - StartPos );
//                    if( Content.Find( wxT( "Not found" ) ) != wxNOT_FOUND )
//                    {
//                        Content = _( "No lyrics found for this song." );
//                    }
//                }
//
//                Content.Replace( wxT( "\n" ), wxT( "<br>" ) );
//
//                if( !TestDestroy() )
//                {
//                    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_LYRICS_UPDATE_LYRICINFO );
//                    event.SetClientData( new wxString( Content.c_str() ) );
//                    wxPostEvent( m_LyricsPanel, event );
//                }
                StartPos = Content.Find( wxT( "<a href='" ) );
                if( StartPos != wxNOT_FOUND )
                {
                    EndPos = Content.Find( wxT( "'>" ) );
                    StartPos += 9;
                    Content = Content.Mid( StartPos, EndPos - StartPos );
                    UrlStr = Content;
                    continue;
                }
            }
        }
        else
        {
            guLogError( wxT( "Lyrics error converting the buffer" ) );
        }
        break;
    } while( true );
    return 0;
}

// -------------------------------------------------------------------------------- //
