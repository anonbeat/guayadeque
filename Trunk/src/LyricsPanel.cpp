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
#include "Utils.h"

#include <wx/curl/http.h>
#include <wx/settings.h>

// -------------------------------------------------------------------------------- //
guLyricsPanel::guLyricsPanel( wxWindow * parent ) :
    wxScrolledWindow( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL )
{
    m_LyricThread = NULL;

    wxFont CurrentFont = wxSystemSettings::GetFont( wxSYS_SYSTEM_FONT );

	wxBoxSizer * MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

	MainSizer->Add( 0, 15, 0, wxEXPAND, 5 );

    m_LyricTitle = new wxStaticText( this, wxID_ANY, wxEmptyString );
    CurrentFont.SetPointSize( 12 );
    CurrentFont.SetWeight( wxFONTWEIGHT_BOLD );
	m_LyricTitle->SetFont( CurrentFont ); //wxFont( 12, 70, 90, 92, false, wxEmptyString ) );
    MainSizer->Add( m_LyricTitle, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );

	m_LyricText = new wxHtmlWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_AUTO );
	m_LyricText->SetBorders( 0 );

	wxColour BGColor = wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWFRAME );
	BGColor.Set( BGColor.Red() - 5, BGColor.Green() - 5, BGColor.Blue() - 5 );
	m_LyricText->SetBackgroundColour( BGColor );
	m_LyricText->SetFonts( CurrentFont.GetFaceName(), wxEmptyString );

	MainSizer->Add( m_LyricText, 1, wxALL|wxEXPAND, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

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
    // Player informs there is a new track playing
    //guLogMessage( wxT( "Received guLyricsPanel::UpdateTrack event" ) );
    const wxArrayString * Params = ( wxArrayString * ) event.GetClientData();
    SetTrack( ( * Params )[ 0 ], ( * Params )[ 1 ] );
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
guFetchLyricThread::ExitCode guFetchLyricThread::Entry()
{
    wxCurlHTTP http;
    char * Buffer = NULL;
    wxString UrlStr = wxString::Format( wxT( "http://lyricwiki.org/api.php?func=getSong&artist=%s&song=%s" ),
                        guURLEncode( m_ArtistName ).c_str(), guURLEncode( m_TrackName ).c_str() );
    http.AddHeader( wxT( "User-Agent: Mozilla/5.0 (X11; U; Linux i686; es-ES; rv:1.9.0.5) Gecko/2008121622 Ubuntu/8.10 (intrepid) Firefox/3.0.5" ) );
    http.AddHeader( wxT( "Accept: text/html" ) );
    http.AddHeader( wxT( "Accept-Charset: utf-8" ) );
    http.Get( Buffer, UrlStr );
    if( Buffer )
    {
        if( !TestDestroy() )
        {
            wxString Content = wxString( Buffer, wxConvUTF8 );
            free( Buffer );
            //
            if( !Content.IsEmpty() )
            {
                int StartPos = Content.Find( wxT( "<pre>" ) ) + 5;
                int EndPos = Content.Find( wxT( "</pre>" ) );
                if( StartPos == wxNOT_FOUND || EndPos == wxNOT_FOUND )
                {
                    Content = _( "No lyrics found for this song." );
                }
                else
                {
                    Content = Content.Mid( StartPos, EndPos - StartPos );
                    if( Content.Find( wxT( "Not found" ) ) != wxNOT_FOUND )
                    {
                        Content = _( "No lyrics found for this song." );
                    }
                }
                //Content = Content.Mid( 0,  ) );
                //guLogMessage( wxT( "Content: %i \n%s" ), Content.Find( wxT( "</pre>" ) ), Content.c_str() );
                Content.Replace( wxT( "\n" ), wxT( "<br>" ) );

                if( !TestDestroy() )
                {
                    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_LYRICS_UPDATE_LYRICINFO );
                    event.SetClientData( new wxString( Content.c_str() ) );
                    wxPostEvent( m_LyricsPanel, event );
                }
            }
            else
            {
                guLogError( wxT( "Lyrics error converting the buffer" ) );
            }
        }
    }
    else
    {
        guLogError( wxT( "Could not get the lyrics" ) );
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
