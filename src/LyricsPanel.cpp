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
#include "curl/http.h"
#include "Images.h"
#include "TagInfo.h"
#include "Utils.h"

#include <wx/clipbrd.h>
#include <wx/html/htmprint.h>
#include <wx/print.h>
#include <wx/printdlg.h>
#include <wx/settings.h>
#include <wx/xml/xml.h>

#define guLYRICS_TEMPLATE_DEFAULT       wxT( "<html><body bgcolor=%s><center><font color=%s size=\"+1\">%s</font></center></body></html>" )
#define guLYRICS_TEMPLATE_ULTGUITAR     wxT( "<html><body bgcolor=%s><font color=%s><TT>%s</TT></font></body></html>" )

// -------------------------------------------------------------------------------- //
guLyricsPanel::guLyricsPanel( wxWindow * parent, guDbLibrary * db ) :
    wxPanel( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL )
{
    m_Db = db;
    m_LyricThread = NULL;
    m_UpdateEnabled = true;
//    m_LyricsTemplate = guLYRICS_TEMPLATE_DEFAULT;
    m_LyricFormat = guLYRIC_FORMAT_NORMAL;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->RegisterObject( this );

    m_WriteToFiles   = Config->ReadBool( wxT( "SaveLyricsToFiles" ), false, wxT( "Lyrics" ) );
    m_WriteToDir     = Config->ReadBool( wxT( "SaveLyricsToDir" ), false, wxT( "Lyrics" ) );
    m_WriteToDirPath = Config->ReadStr(  wxT( "Path" ), wxGetHomeDir() + wxT( "/.guayadeque/lyrics" ), wxT( "Lyrics" ) );
    if( !m_WriteToDirPath.EndsWith( wxT( "/" ) ) )
        m_WriteToDirPath += wxT( "/" );
    // Check that the directory to store podcasts are created
    if( !wxDirExists( m_WriteToDirPath ) )
    {
        wxMkdir( m_WriteToDirPath, 0770 );
    }

    wxFont CurrentFont = wxSystemSettings::GetFont( wxSYS_SYSTEM_FONT );

	wxBoxSizer * MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer * TitleSizer;
	TitleSizer = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer * EditorSizer;
	EditorSizer = new wxBoxSizer( wxHORIZONTAL );

	m_UpdateCheckBox = new wxCheckBox( this, wxID_ANY, _( "Follow player" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_UpdateCheckBox->SetToolTip( wxT( "Search the lyrics for the current playing track" ) );
	m_UpdateCheckBox->SetValue( m_UpdateEnabled );

	EditorSizer->Add( m_UpdateCheckBox, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

//	EditorSizer->Add( 0, 0, 1, wxEXPAND, 5 );
	wxStaticText * ServerStaticText = new wxStaticText( this, wxID_ANY, wxT("Server:"), wxDefaultPosition, wxDefaultSize, 0 );
	ServerStaticText->Wrap( -1 );
	EditorSizer->Add( ServerStaticText, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxLEFT, 5 );

	wxString LyricsChoiceChoices[] = {
	    wxT( "http://lyricwiki.org" ),
	    wxT( "http://leoslyrics.com" ),
	    wxT( "http://lyrc.com.ar" ),
	    wxT( "http://cduniverse.com" ),
	    wxT( "http://ultimate-guitar.com" )
	    };
	int LyricsChoiceNChoices = sizeof( LyricsChoiceChoices ) / sizeof( wxString );
	m_ServerChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, LyricsChoiceNChoices, LyricsChoiceChoices, 0 );
	m_ServerChoice->SetSelection( Config->ReadNum( wxT( "LyricSearchEngine" ), 0, wxT( "General" ) ) );
	EditorSizer->Add( m_ServerChoice, 1, wxTOP|wxRIGHT, 5 );

	m_ReloadButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_reload ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_ReloadButton->SetToolTip( wxT( "Reload the lyric" ) );
	EditorSizer->Add( m_ReloadButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP, 5 );

	m_EditButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_EditButton->SetToolTip( wxT( "Edit the lyric for the current track" ) );
	m_EditButton->Enable( false );
	EditorSizer->Add( m_EditButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP, 5 );

	m_SaveButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_doc_save ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_SaveButton->Enable( false );
	m_SaveButton->SetToolTip( wxT( "Save the lyrics to the current file" ) );
	EditorSizer->Add( m_SaveButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT, 5 );

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

	EditorSizer->Add( m_TrackTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT, 5 );

//	m_SearchButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_search ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
//	m_SearchButton->Enable( false );

//	EditorSizer->Add( m_ReloadButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT|wxLEFT, 5 );

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

	//m_LyricText = new wxHtmlWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_AUTO );
	m_LyricText = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE|wxTE_DONTWRAP|wxTE_MULTILINE|wxNO_BORDER );
	m_EditModeColor = m_LyricText->GetBackgroundColour();
	m_LyricText->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWFRAME ) );
//    m_LyricText->SetPage( wxString::Format( wxT( "<html><body bgcolor=%s></body></html>" ),
//          wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWFRAME ).GetAsString( wxC2S_HTML_SYNTAX ).c_str() ) );
//	m_LyricText->SetFonts( CurrentFont.GetFaceName(), wxEmptyString );

	MainSizer->Add( m_LyricText, 1, wxALL|wxEXPAND, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

    SetDropTarget( new guLyricsPanelDropTarget( this ) );
    m_LyricText->SetDropTarget( new guLyricsPanelDropTarget( this ) );

	m_UpdateCheckBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guLyricsPanel::OnUpdateChkBoxClicked ), NULL, this );
	m_ReloadButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guLyricsPanel::OnReloadBtnClick ), NULL, this );
	m_EditButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guLyricsPanel::OnEditBtnClick ), NULL, this );
	m_SaveButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guLyricsPanel::OnSaveBtnClick ), NULL, this );
	m_ArtistTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guLyricsPanel::OnTextUpdated ), NULL, this );
	m_TrackTextCtrl->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guLyricsPanel::OnTextUpdated ), NULL, this );
    Connect( ID_LYRICS_UPDATE_LYRICINFO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLyricsPanel::OnDownloadedLyric ), NULL, this );

    m_LyricText->Connect( wxEVT_CONTEXT_MENU, wxContextMenuEventHandler( guLyricsPanel::OnContextMenu ), NULL, this );
    Connect( ID_LYRICS_COPY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLyricsPanel::OnLyricsCopy ), NULL, this );
    Connect( ID_LYRICS_PASTE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLyricsPanel::OnLyricsPaste ), NULL, this );
    Connect( ID_LYRICS_PRINT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLyricsPanel::OnLyricsPrint ), NULL, this );

    Connect( ID_CONFIG_UPDATED, guConfigUpdatedEvent, wxCommandEventHandler( guLyricsPanel::OnConfigUpdated ), NULL, this );
	m_ServerChoice->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( guLyricsPanel::OnServerSelected ), NULL, this );
}

// -------------------------------------------------------------------------------- //
guLyricsPanel::~guLyricsPanel()
{
	m_UpdateCheckBox->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( guLyricsPanel::OnUpdateChkBoxClicked ), NULL, this );
	m_ReloadButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guLyricsPanel::OnReloadBtnClick ), NULL, this );
	m_EditButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guLyricsPanel::OnEditBtnClick ), NULL, this );
	m_SaveButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guLyricsPanel::OnSaveBtnClick ), NULL, this );
	m_ArtistTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guLyricsPanel::OnTextUpdated ), NULL, this );
	m_TrackTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( guLyricsPanel::OnTextUpdated ), NULL, this );
	//m_SearchButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guLyricsPanel::OnSearchBtnClick ), NULL, this );
    Disconnect( ID_LYRICS_UPDATE_LYRICINFO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLyricsPanel::OnDownloadedLyric ), NULL, this );

    m_LyricText->Disconnect( wxEVT_CONTEXT_MENU, wxContextMenuEventHandler( guLyricsPanel::OnContextMenu ), NULL, this );
    Disconnect( ID_LYRICS_COPY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLyricsPanel::OnLyricsCopy ), NULL, this );
    Disconnect( ID_LYRICS_PASTE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLyricsPanel::OnLyricsPaste ), NULL, this );
    Disconnect( ID_LYRICS_PRINT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guLyricsPanel::OnLyricsPrint ), NULL, this );

    Disconnect( ID_CONFIG_UPDATED, guConfigUpdatedEvent, wxCommandEventHandler( guLyricsPanel::OnConfigUpdated ), NULL, this );
	m_ServerChoice->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( guLyricsPanel::OnServerSelected ), NULL, this );

    if( m_LyricThread )
    {
        m_LyricThread->Pause();
        m_LyricThread->Delete();
    }

    // Save the current selected server
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->WriteNum( wxT( "LyricSearchEngine" ), m_ServerChoice->GetSelection(), wxT( "General" ) );
    Config->UnRegisterObject( this );
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::OnConfigUpdated( wxCommandEvent &event )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        m_WriteToFiles   = Config->ReadBool( wxT( "SaveLyricsToFiles" ), false, wxT( "Lyrics" ) );
        m_WriteToDir     = Config->ReadBool( wxT( "SaveLyricsToDir" ), false, wxT( "Lyrics" ) );
        m_WriteToDirPath = Config->ReadStr(  wxT( "Path" ), wxGetHomeDir() + wxT( "/.guayadeque/lyrics" ), wxT( "Lyrics" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::OnContextMenu( wxContextMenuEvent &event )
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
void guLyricsPanel::CreateContextMenu( wxMenu * menu )
{
    wxMenuItem * MenuItem;
    MenuItem = new wxMenuItem( menu, ID_LYRICS_COPY, _( "Copy to clipboard" ), _( "Copy the content of the lyric to clipboard" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_copy ) );
    menu->Append( MenuItem );

    wxTheClipboard->UsePrimarySelection( false );
    if( wxTheClipboard->Open() )
    {
        if( wxTheClipboard->IsSupported( wxDF_TEXT ) )
        {
            wxTextDataObject data;
            if( wxTheClipboard->GetData( data ) )
            {
                MenuItem = new wxMenuItem( menu, ID_LYRICS_PASTE, _( "Paste" ), _( "Paste the content of the lyric from clipboard" ) );
                menu->Append( MenuItem );
            }
        }
        wxTheClipboard->Close();
    }

    menu->AppendSeparator();

    MenuItem = new wxMenuItem( menu, ID_LYRICS_PRINT, _( "Print" ), _( "Print the content of the lyrics" ) );
    //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_ ) );
    menu->Append( MenuItem );
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::OnUpdatedTrack( wxCommandEvent &event )
{
    if( m_UpdateEnabled )
    {
        // If have been edited and not saved
        if( m_LyricText->IsModified() )
        {
            SaveLyrics();
        }

        m_CurrentFileName = wxEmptyString;

        guTrack * Track = ( guTrack * ) event.GetClientData();
        guTrackChangeInfo ChangeInfo;
        if( Track )
        {
            ChangeInfo.m_ArtistName = Track->m_ArtistName;
            ChangeInfo.m_TrackName = Track->m_SongName;
            if( Track->m_Type < guTRACK_TYPE_RADIOSTATION )
            {
                m_CurrentFileName = Track->m_FileName;
            }
        }
        m_CurrentTrackInfo = ChangeInfo;
        SetTrack( &ChangeInfo );
    }
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::OnTextUpdated( wxCommandEvent& event )
{
    m_ReloadButton->Enable( !m_ArtistTextCtrl->GetValue().IsEmpty() &&
                            !m_TrackTextCtrl->GetValue().IsEmpty() );
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::OnUpdateChkBoxClicked( wxCommandEvent& event )
{
    SetAutoUpdate( m_UpdateCheckBox->IsChecked() );
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::SetAutoUpdate( const bool autoupdate )
{
    m_UpdateEnabled = autoupdate;
    if( m_UpdateEnabled != m_UpdateCheckBox->IsChecked() )
    {
        m_UpdateCheckBox->SetValue( autoupdate );
    }
    if( m_UpdateEnabled )
    {
        m_ArtistTextCtrl->SetValue( m_CurrentTrackInfo.m_ArtistName );
        m_TrackTextCtrl->SetValue( m_CurrentTrackInfo.m_TrackName );
    }
    m_ArtistTextCtrl->Enable( !m_UpdateEnabled );
    m_TrackTextCtrl->Enable( !m_UpdateEnabled );
    m_ReloadButton->Enable( !m_ArtistTextCtrl->GetValue().IsEmpty() &&
                            !m_TrackTextCtrl->GetValue().IsEmpty() );
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::OnServerSelected( wxCommandEvent &event )
{
    OnReloadBtnClick( event );
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::OnReloadBtnClick( wxCommandEvent& event )
{
    guTrackChangeInfo TrackChangeInfo( m_ArtistTextCtrl->GetValue(), m_TrackTextCtrl->GetValue() );
    SetTrack( &TrackChangeInfo, true );
    m_ReloadButton->Enable( false );
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::SaveLyrics( void )
{
    //guLogMessage( wxT( "Saving to File: %i  Dir: %i" ), m_WriteToFiles, m_WriteToDir );
    if( m_WriteToFiles )
    {
        if( !m_CurrentFileName.IsEmpty() && guIsValidAudioFile( m_CurrentFileName ) )
        {
            guTagInfo * TagInfo;

            TagInfo = guGetTagInfoHandler( m_CurrentFileName );

            if( TagInfo )
            {
                if( TagInfo->CanHandleLyrics() )
                {
                    if( m_LyricText->IsModified() )
                        m_CurrentLyricText = m_LyricText->GetValue();

                    TagInfo->SetLyrics( m_CurrentLyricText );
                }
                delete TagInfo;
            }
        }
    }

    if( m_WriteToDir )
    {
        wxFileName FileName( m_WriteToDirPath +
                    m_ArtistTextCtrl->GetValue() + wxT( "/" ) +
                    m_TrackTextCtrl->GetValue() + wxT( ".lyric" ) );
        if( FileName.Normalize( wxPATH_NORM_ALL|wxPATH_NORM_CASE ) )
        {
            FileName.Mkdir( 0770, wxPATH_MKDIR_FULL );
            wxFile LyricFile( FileName.GetFullPath(), wxFile::write );
            if( LyricFile.IsOpened() )
            {
                if( !LyricFile.Write( m_CurrentLyricText ) )
                {
                    guLogError( wxT( "Error writing to lyric file '%s'" ), FileName.GetFullPath().c_str() );
                }
                LyricFile.Flush();
                LyricFile.Close();
            }
            else
            {
                guLogError( wxT( "Error saving lyric file '%s'" ), FileName.GetFullPath().c_str() );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::OnEditBtnClick( wxCommandEvent& event )
{
    m_EditButton->Enable( false );
    m_SaveButton->Enable( true );
    m_LyricText->SetEditable( true );
    m_LyricText->SetBackgroundColour( m_EditModeColor );
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::OnSaveBtnClick( wxCommandEvent& event )
{
    SaveLyrics();
    m_SaveButton->Enable( false );
    m_EditButton->Enable( true );
    m_LyricText->SetEditable( false );
    m_LyricText->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWFRAME ) );
}

//// -------------------------------------------------------------------------------- //
//void guLyricsPanel::OnSearchBtnClick( wxCommandEvent& event )
//{
//    guTrackChangeInfo TrackChangeInfo( m_ArtistTextCtrl->GetValue(), m_TrackTextCtrl->GetValue() );
//    SetTrack( &TrackChangeInfo );
//}

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
    wxString LyricText = text;
    LyricText.Replace( wxT( "\n" ), wxT( "<br>" ) );
    LyricText.Replace( wxT( " " ), wxT( "&nbsp;" ) );

	wxFont CurrentFont = wxSystemSettings::GetFont( wxSYS_SYSTEM_FONT );

    if( m_LyricFormat == guLYRIC_FORMAT_NORMAL )
    {
        m_LyricText->SetWindowStyle( ( m_LyricText->GetWindowStyle() ^ wxTE_LEFT ) | wxTE_CENTER );
        m_LyricText->SetFont( CurrentFont );
    }
    else
    {
        m_LyricText->SetWindowStyle( ( m_LyricText->GetWindowStyle() ^ wxTE_CENTER ) | wxTE_LEFT );
        CurrentFont.SetFamily( wxTELETYPE );
        m_LyricText->SetFont( CurrentFont );
    }
//    m_LyricText->SetPage( wxString::Format( m_LyricsTemplate,
//          wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWFRAME ).GetAsString( wxC2S_HTML_SYNTAX ).c_str(),
//          wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT ).GetAsString( wxC2S_HTML_SYNTAX ).c_str(),
//          LyricText.c_str() ) );
    m_LyricText->SetValue( text );
    Layout();
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::SetTrack( const guTrackChangeInfo * trackchangeinfo, const bool onlinesearch )
{
    //const wxString &artist, const wxString &tracK
    wxString Artist = trackchangeinfo->m_ArtistName;
    wxString Track = trackchangeinfo->m_TrackName;
    wxString LyricText;

    m_EditButton->Enable( !Artist.IsEmpty() && !Track.IsEmpty() &&
                          !m_CurrentFileName.IsEmpty() && wxFileExists( m_CurrentFileName ) );
    m_LyricText->SetEditable( false );
    m_LyricText->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWFRAME ) );

    SetTitle( Track + wxT( " / " ) + Artist );
    //SetText( _( "No lyrics found for this song." ) );

    m_ArtistTextCtrl->SetValue( Artist );
    m_TrackTextCtrl->SetValue( Track );

    m_SaveButton->Enable( false );

    // If the search was done manually ( m_UpdateEnable == false
    // Then we dont save it automatically
    if( !onlinesearch && m_UpdateEnabled && !m_CurrentFileName.IsEmpty() && guIsValidAudioFile( m_CurrentFileName ) )
    {
        guTagInfo * TagInfo;

        TagInfo = guGetTagInfoHandler( m_CurrentFileName );

        if( TagInfo )
        {
            if( TagInfo->CanHandleLyrics() )
            {
                LyricText = TagInfo->GetLyrics();
            }
            delete TagInfo;
        }
    }

    // If was not found as a tag in the file try to read it from the lyric directory.
    if( LyricText.IsEmpty() )
    {
        wxFileName FileName( m_WriteToDirPath +
                    m_ArtistTextCtrl->GetValue() + wxT( "/" ) +
                    m_TrackTextCtrl->GetValue() + wxT( ".lyric" ) );
        if( FileName.Normalize( wxPATH_NORM_ALL|wxPATH_NORM_CASE ) )
        {
            if( wxFileExists( FileName.GetFullPath() ) )
            {
                wxFile LyricFile( FileName.GetFullPath(), wxFile::read );
                if( LyricFile.IsOpened() )
                {
                    int FileSize = LyricFile.SeekEnd();
                    char * Buffer = ( char * ) malloc( FileSize + 1 );
                    if( Buffer )
                    {
                        LyricFile.Seek( 0 );
                        if( LyricFile.Read( Buffer, FileSize ) != FileSize )
                        {
                            guLogError( wxT( "Error reading lyric file '%s'" ), FileName.GetFullPath().c_str() );
                        }
                        Buffer[ FileSize ] = 0;
                        LyricText = wxString::FromUTF8( Buffer );
                        free( Buffer );
                    }
                    LyricFile.Close();
                }
            }
            else
            {
                guLogMessage( wxT( "The file dont exists %s" ), FileName.GetFullPath().c_str() );
            }
        }
    }

    if( !LyricText.IsEmpty() )
    {
        m_CurrentLyricText = LyricText;

        // When reading lyrics from an mp3 file I have no way to know if its a
        // Tab or a regular lyric so I search for what its usually used
        // for tab representation to decide the template to use
        // I know this is an ugly hack :/
        if( LyricText.Find( wxT( "-----" ) ) != wxNOT_FOUND )
            //m_LyricsTemplate = guLYRICS_TEMPLATE_ULTGUITAR;
            m_LyricFormat = guLYRIC_FORMAT_GUITAR;
        else
            //m_LyricsTemplate = guLYRICS_TEMPLATE_DEFAULT;
            m_LyricFormat = guLYRIC_FORMAT_NORMAL;

        SetText( LyricText );
    }
    else if( !Artist.IsEmpty() && !Track.IsEmpty() )
    {
        SetText( _( "Searching the lyrics for this track" ) );

        if( m_LyricThread )
        {
            m_LyricThread->Pause();
            m_LyricThread->Delete();
        }

        //guConfig * Config = ( guConfig * ) Config->Get();
        //int Engine = Config->ReadNum( wxT( "LyricSearchEngine" ), 0, wxT( "General" ) );
        int Engine = m_ServerChoice->GetSelection();
        if( Engine == guLYRIC_ENGINE_LYRICWIKI )
        {
            m_LyricThread = new guLyricWikiEngine( ( wxEvtHandler * ) this, &m_LyricThread, Artist.c_str(), Track.c_str() );
        }
        else if( Engine == guLYRIC_ENGINE_LEOSLYRICS )
        {
            m_LyricThread = new guLeosLyricsEngine( ( wxEvtHandler * ) this, &m_LyricThread, Artist.c_str(), Track.c_str() );
        }
        else if( Engine == guLYRIC_ENGINE_LYRC_COM_AR )
        {
            m_LyricThread = new guLyrcComArEngine( ( wxEvtHandler * ) this, &m_LyricThread, Artist.c_str(), Track.c_str() );
        }
        else if( Engine == guLYRIC_ENGINE_CDUNIVERSE )
        {
            m_LyricThread = new guCDUEngine( ( wxEvtHandler * ) this, &m_LyricThread, Artist.c_str(), Track.c_str() );
        }
        else //if( Engine == guLYRIC_ENGINE_ULTGUITAR )
        {
            m_LyricThread = new guUltGuitarEngine( ( wxEvtHandler * ) this, &m_LyricThread, Artist.c_str(), Track.c_str() );
        }
        //m_LyricsTemplate = m_LyricThread->GetTemplate();
        m_LyricFormat = m_LyricThread->GetFormat();
    }
    else
    {
        SetText( wxEmptyString );
    }
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::OnDownloadedLyric( wxCommandEvent &event )
{
    wxString * Content = ( wxString * ) event.GetClientData();
    if( Content )
    {
        m_CurrentLyricText = * Content;

        SetText( * Content );

        if( m_UpdateEnabled )
        {
            SaveLyrics();
        }
        else
        {
            m_SaveButton->Enable( !Content->IsEmpty() && !m_CurrentFileName.IsEmpty() && guIsValidAudioFile( m_CurrentFileName ) );
        }

        delete Content;
    }
    else
    {
        SetText( _( "No lyrics found for this song." ) );
    }
    m_ReloadButton->Enable( true );
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::OnLyricsCopy( wxCommandEvent &event )
{
    wxTheClipboard->UsePrimarySelection( false );
    if( wxTheClipboard->Open() )
    {
        wxTheClipboard->Clear();
        wxString CopyText = m_LyricText->GetStringSelection();
        if( CopyText.IsEmpty() )
        {
            CopyText = m_TrackTextCtrl->GetValue() + wxT( " / " ) +
                       m_ArtistTextCtrl->GetValue() + wxT( "\n\n" ) +
                       m_CurrentLyricText;
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
void guLyricsPanel::OnLyricsPaste( wxCommandEvent &event )
{
    wxTheClipboard->UsePrimarySelection( false );
    if( wxTheClipboard->Open() )
    {
        if( wxTheClipboard->IsSupported( wxDF_TEXT ) )
        {
            wxTextDataObject data;
            if( wxTheClipboard->GetData( data ) )
            {
                m_CurrentLyricText = data.GetText();

                SetText( m_CurrentLyricText );

                if( m_UpdateEnabled )
                {
                    SaveLyrics();
                }
                else
                {
                    m_SaveButton->Enable( !m_CurrentLyricText.IsEmpty() && !m_CurrentFileName.IsEmpty() && guIsValidAudioFile( m_CurrentFileName ) );
                }
            }
        }
        wxTheClipboard->Close();
    }
    else
    {
        guLogError( wxT( "Could not open the clipboard object" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::OnLyricsPrint( wxCommandEvent &event )
{
    wxPrintData * PrintData = new wxPrintData;

    if( PrintData )
    {
        PrintData->SetPaperId( wxPAPER_A4 );

        wxPrintDialogData * PrintDialogData = new wxPrintDialogData( * PrintData );
        if( PrintDialogData )
        {
            wxString TrackName = m_TrackTextCtrl->GetValue() + wxT( " / " ) + m_ArtistTextCtrl->GetValue();
            wxHtmlPrintout Printout( TrackName );

            wxString LyricText = wxString::Format( wxT( "<b>%s</b><br><br>" ), TrackName.c_str() ) +
                                 m_CurrentLyricText;
            LyricText.Replace( wxT( "\n" ), wxT( "<br>" ) );

            Printout.SetHtmlText( wxString::Format(
                ( ( m_LyricFormat == guLYRIC_FORMAT_NORMAL ) ? guLYRICS_TEMPLATE_DEFAULT : guLYRICS_TEMPLATE_ULTGUITAR ),
                wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWFRAME ).GetAsString( wxC2S_HTML_SYNTAX ).c_str(),
                wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT ).GetAsString( wxC2S_HTML_SYNTAX ).c_str(),
                LyricText.c_str() ) );

            wxPrinter Printer( PrintDialogData );

            Printout.SetFooter( wxT( "<center>Guayadeque Music Player   @DATE@ @TIME@   @PAGENUM@ - @PAGESCNT@</center>" ) );

            if( Printer.Print( this, &Printout, true ) )
            {
                if( wxPrinter::GetLastError() == wxPRINTER_ERROR )
                    guLogMessage( wxT( "There was a problem printing the lyric" ) );
            }

            delete PrintDialogData;
        }

        delete PrintData;
    }
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::OnDropFiles( const wxArrayString &files )
{
    //guLogMessage( wxT( "guLastFMPanelDropTarget::OnDropFiles" ) );
    guTrack Track;

    // If have been edited and not saved
    if( m_LyricText->IsModified() )
    {
        SaveLyrics();
    }

    if( m_Db->FindTrackFile( files[ 0 ], &Track ) )
    {
        guTrackChangeInfo ChangeInfo;

        ChangeInfo.m_ArtistName = Track.m_ArtistName;
        ChangeInfo.m_TrackName = Track.m_SongName;

        m_CurrentFileName = files[ 0 ];

        SetAutoUpdate( false );

        m_CurrentTrackInfo = ChangeInfo;
        SetTrack( &ChangeInfo );
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
                //Track.m_Type = guTRACK_TYPE_NOTDB;
                Track.m_ArtistName = TagInfo->m_ArtistName;
                //Track.m_AlbumName = TagInfo->m_AlbumName;
                Track.m_SongName = TagInfo->m_TrackName;
                //Track.m_Number = TagInfo->m_Track;
                //Track.m_GenreName = TagInfo->m_GenreName;
                //Track.m_Length = TagInfo->m_Length;
                //Track.m_Year = TagInfo->m_Year;
                //Track.m_Rating = wxNOT_FOUND;

                guTrackChangeInfo ChangeInfo;

                ChangeInfo.m_ArtistName = Track.m_ArtistName;
                ChangeInfo.m_TrackName = Track.m_SongName;

                m_CurrentFileName = files[ 0 ];

                SetAutoUpdate( false );

                m_CurrentTrackInfo = ChangeInfo;
                SetTrack( &ChangeInfo );
            }

            delete TagInfo;
        }
    }
}

// -------------------------------------------------------------------------------- //
// guLyricsPanelDropTarget
// -------------------------------------------------------------------------------- //
guLyricsPanelDropTarget::guLyricsPanelDropTarget( guLyricsPanel * lyricspanel )
{
    m_LyricsPanel = lyricspanel;
}

// -------------------------------------------------------------------------------- //
guLyricsPanelDropTarget::~guLyricsPanelDropTarget()
{
}

// -------------------------------------------------------------------------------- //
bool guLyricsPanelDropTarget::OnDropFiles( wxCoord x, wxCoord y, const wxArrayString &files )
{
    m_LyricsPanel->OnDropFiles( files );
    return true;
}

// -------------------------------------------------------------------------------- //
wxDragResult guLyricsPanelDropTarget::OnDragOver( wxCoord x, wxCoord y, wxDragResult def )
{
    //printf( "guLyricsPanelDropTarget::OnDragOver... %d - %d\n", x, y );
    return wxDragCopy;
}

// -------------------------------------------------------------------------------- //
// guSearchLyricEngine
// -------------------------------------------------------------------------------- //
guSearchLyricEngine::guSearchLyricEngine( wxEvtHandler * owner,
        guSearchLyricEngine ** threadpointer, const wxChar * artistname, const wxChar * trackname ) :
    wxThread()
{
    wxASSERT( owner );
    wxASSERT( artistname );
    wxASSERT( trackname );

    m_Owner = owner;
    m_ThreadPointer = threadpointer;
    m_ArtistName = artistname;
    m_TrackName = trackname;
}

// -------------------------------------------------------------------------------- //
guSearchLyricEngine::~guSearchLyricEngine()
{
    if( !TestDestroy() && m_ThreadPointer )
    {
        * m_ThreadPointer = NULL;
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

// -------------------------------------------------------------------------------- //
void guSearchLyricEngine::SetLyric( wxString * lyrictext )
{
    if( !TestDestroy() )
    {
        //guLogMessage( wxT( "==== Lyrics ====\n%s" ), lyrictext->c_str() );
        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_LYRICS_UPDATE_LYRICINFO );
        event.SetClientData( ( void * ) lyrictext );
        wxPostEvent( m_Owner, event );
    }
}

// -------------------------------------------------------------------------------- //
wxString guSearchLyricEngine::GetTemplate( void )
{
    return guLYRICS_TEMPLATE_DEFAULT;
}

// -------------------------------------------------------------------------------- //
int guSearchLyricEngine::GetFormat( void )
{
    return guLYRIC_FORMAT_NORMAL;
}

// -------------------------------------------------------------------------------- //
// guLyricWikiEngine
// -------------------------------------------------------------------------------- //
guLyricWikiEngine::guLyricWikiEngine( wxEvtHandler * owner, guSearchLyricEngine ** psearchengine,
         const wxChar * artistname, const wxChar * trackname ) :
    guSearchLyricEngine( owner, psearchengine, artistname, trackname )
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

// -------------------------------------------------------------------------------- //
wxString ProcessHexData( const wxString &hexstr )
{
    int index;
    wxString RetVal = hexstr;
    for( index = 0; index < 256; index++ )
    {
        wxString Old = wxString::Format( wxT( "&#%u;" ), index );
        wxString New = wxString::Format( wxT( "%c" ), index );
        RetVal.Replace( Old, New );
    }
    RetVal.Replace( wxT( "<br />" ), wxT( "\n" ) );
    return RetVal;
}

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

                Content = ProcessHexData( Content );
                SetLyric( new wxString( Content.c_str() ) );
                return;
            }
        }
    }

//    else
//    {
    guLogError( wxT( "Could not get the content of the lyrics." ) );
    if( !TestDestroy() )
    {
        SetLyric( NULL );
        //break;
    }
//    }
}

// -------------------------------------------------------------------------------- //
// guLyricWikiEngine
// -------------------------------------------------------------------------------- //
guLeosLyricsEngine::guLeosLyricsEngine( wxEvtHandler * owner, guSearchLyricEngine ** psearchengine,
         const wxChar * artistname, const wxChar * trackname ) :
    guSearchLyricEngine( owner, psearchengine, artistname, trackname )
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
        if( !LyricText.IsEmpty() )
        {
            SetLyric( new wxString( LyricText ) );
            return;
        }
    }

    if( !TestDestroy() )
    {
        SetLyric( NULL );
    }
}

// -------------------------------------------------------------------------------- //
// guLyrcComArEngine
// -------------------------------------------------------------------------------- //
guLyrcComArEngine::guLyrcComArEngine( wxEvtHandler * owner, guSearchLyricEngine ** psearchengine,
         const wxChar * artistname, const wxChar * trackname ) :
    guSearchLyricEngine( owner, psearchengine, artistname, trackname )
{
    if( Create() == wxTHREAD_NO_ERROR )
    {
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guLyrcComArEngine::~guLyrcComArEngine()
{
}

// -------------------------------------------------------------------------------- //
bool guLyrcComArEngine::DoSearchLyric( const wxString &content )
{
    int         StartPos;
    int         EndPos;
    wxString Content = content;

    if( !TestDestroy() && !Content.IsEmpty() )
    {
        StartPos = Content.Find( wxT( "</td></tr></table>" ) );

        if( StartPos != wxNOT_FOUND )
        {
            Content = Content.Mid( StartPos + 18 );
            if( ( EndPos = Content.Find( wxT( "<p><hr size" ) ) ) == wxNOT_FOUND )
            {
                EndPos = Content.Find( wxT( "<a href=\"#\"" ) );
            }
            Content = Content.Mid( 0, EndPos );

            Content.Replace( wxT( "<br />" ), wxT( "" ) );
            Content.Replace( wxT( "<br>" ), wxT( "\n" ) );
            SetLyric( new wxString( Content.c_str() ) );
            return true;
        }
        else
        {
            StartPos = Content.Lower().Find( wxString::Format( wxT( "%s - %s" ),
                m_ArtistName.Lower().c_str(),
                m_TrackName.Lower().c_str() ) );

            if( StartPos != wxNOT_FOUND )
            {
                Content = Content.Mid( 0, StartPos ).BeforeLast( wxT( '"' ) );
                Content = Content.AfterLast( wxT( '"' ) );
                return DoSearchLyric( GetUrlContent( wxT( "http://lyrc.com.ar/en/" ) + Content ) );
            }
        }
    }
    return false;
}

// -------------------------------------------------------------------------------- //
void guLyrcComArEngine::SearchLyric( void )
{
    wxString    UrlStr = wxString::Format( wxT( "http://lyrc.com.ar/en/tema1en.php?artist=%s&songname=%s" ),
                        guURLEncode( m_ArtistName ).c_str(), guURLEncode( m_TrackName ).c_str() );

    if( !DoSearchLyric( GetUrlContent( UrlStr ) ) && !TestDestroy() )
    {
        SetLyric( NULL );
    }
}

// -------------------------------------------------------------------------------- //
// guCDUEngine
// -------------------------------------------------------------------------------- //
guCDUEngine::guCDUEngine( wxEvtHandler * owner, guSearchLyricEngine ** psearchengine,
         const wxChar * artistname, const wxChar * trackname ) :
    guSearchLyricEngine( owner, psearchengine, artistname, trackname )
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

    //guLogMessage( wxT( "Url: %s" ), UrlStr.c_str() );
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

                SetLyric( new wxString( Content ) );
                return;
            }
        }
    }
    if( !TestDestroy() )
    {
        SetLyric( NULL );
    }
}

// -------------------------------------------------------------------------------- //
// guUltGuitarEngine
// -------------------------------------------------------------------------------- //

#define guULTIMATE_GUITAR_BASE_URL      wxT( "http://www.ultimate-guitar.com" )
#define guULTIMAGE_GUITAR_SEARCH_URL    guULTIMATE_GUITAR_BASE_URL wxT( "/search.php?view_state=advanced&band_name=%s&song_name=%s&type[]=200&type2[]=40000&version_la=" )


// -------------------------------------------------------------------------------- //
guUltGuitarEngine::guUltGuitarEngine( wxEvtHandler * owner, guSearchLyricEngine ** psearchengine,
         const wxChar * artistname, const wxChar * trackname ) :
    guSearchLyricEngine( owner, psearchengine, artistname, trackname )
{
    if( Create() == wxTHREAD_NO_ERROR )
    {
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guUltGuitarEngine::~guUltGuitarEngine()
{
}

// -------------------------------------------------------------------------------- //
void guUltGuitarEngine::SearchLyric( void )
{
    wxString    Content;
    wxString    UrlStr = wxString::Format( guULTIMAGE_GUITAR_SEARCH_URL,
                        guURLEncode( m_ArtistName ).c_str(), guURLEncode( m_TrackName ).c_str() );

    //guLogMessage( wxT( "Url: %s" ), UrlStr.c_str() );
    Content = GetUrlContent( UrlStr );
    //guLogMessage( wxT( "Content:\n%s" ), Content.c_str() );
    //
    if( !Content.IsEmpty() )
    {
        wxString ArtistName = m_ArtistName.Lower();
        ArtistName.Replace( wxT( " " ), wxT( "_" ) );
        wxString TrackName = m_TrackName.Lower();
        TrackName.Replace( wxT( " " ), wxT( "_" ) );

        int Pos = Content.Find( wxString::Format( wxT( "/tabs/%c/%s/%s" ),
                    ArtistName[ 0 ],
                    ArtistName.c_str(),
                    TrackName.c_str() ) );

        if( Pos != wxNOT_FOUND )
        {
            Content = Content.Mid( Pos );
            Pos = Content.Find( wxT( ".htm" ) );
            if( Pos != wxNOT_FOUND )
            {
                Content = Content.Mid( 0, Pos + 4 );
                Content = guULTIMATE_GUITAR_BASE_URL + Content;
                Content = GetUrlContent( Content );
                if( !Content.IsEmpty() )
                {
                    Pos = Content.Find( wxT( "<pre>" ) );
                    if( Pos != wxNOT_FOUND )
                    {
                        Content = Content.Mid( Pos + 5 );
                        Pos = Content.Find( wxT( "</pre>" ) );
                        if( Pos != wxNOT_FOUND )
                        {
                            Content = Content.Mid( 0, Pos );
                            SetLyric( new wxString( Content ) );
                            return;
                        }
                    }
                }
            }
        }
    }
    if( !TestDestroy() )
    {
        SetLyric( NULL );
    }
}

// -------------------------------------------------------------------------------- //
wxString guUltGuitarEngine::GetTemplate( void )
{
    return guLYRICS_TEMPLATE_ULTGUITAR;
}

// -------------------------------------------------------------------------------- //
int guUltGuitarEngine::GetFormat( void )
{
    return guLYRIC_FORMAT_GUITAR;
}

// -------------------------------------------------------------------------------- //
