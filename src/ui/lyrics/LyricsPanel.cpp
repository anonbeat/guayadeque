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
#include "LyricsPanel.h"

#include "EventCommandIds.h"
#include "Config.h"
#include "Preferences.h"
#include "Images.h"
#include "ListView.h"
#include "MainFrame.h"
#include "TagInfo.h"
#include "Utils.h"

#include <wx/arrimpl.cpp>
#include <wx/clipbrd.h>
#include <wx/html/htmprint.h>
#include <wx/html/htmlpars.h>
#include <wx/print.h>
#include <wx/printdlg.h>
#include <wx/settings.h>
#include <wx/txtstrm.h>
#include <wx/xml/xml.h>
#include <wx/regex.h>
#include <wx/sstream.h>
#include <wx/tokenzr.h>

namespace Guayadeque {

int LyricAligns[] = { wxTE_LEFT, wxTE_CENTER, wxTE_RIGHT };

WX_DEFINE_OBJARRAY( guLyricSourceReplaceArray )
WX_DEFINE_OBJARRAY( guLyricSourceExtractArray )
WX_DEFINE_OBJARRAY( guLyricSourceExcludeArray )
WX_DEFINE_OBJARRAY( guLyricSourceArray )


// -------------------------------------------------------------------------------- //
guLyricsPanel::guLyricsPanel( wxWindow * parent, guDbLibrary * db, guLyricSearchEngine * lyricsearchengine ) :
    wxPanel( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL )
{
    m_Db = db;
    m_LyricSearchEngine = lyricsearchengine;
    m_LyricSearchContext = NULL;
    m_CurrentTrack = NULL;
    m_MediaViewer = NULL;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->RegisterObject( this );

    m_UpdateEnabled = Config->ReadBool( CONFIG_KEY_LYRICS_FOLLOW_PLAYER, true, CONFIG_PATH_LYRICS );
    m_LyricAlign = LyricAligns[ Config->ReadNum( CONFIG_KEY_LYRICS_TEXT_ALIGN, 1, CONFIG_PATH_LYRICS ) ];
    wxFont CurrentFont;
    CurrentFont.SetNativeFontInfo( Config->ReadStr( CONFIG_KEY_LYRICS_FONT, wxEmptyString, CONFIG_PATH_LYRICS ) );
    if( !CurrentFont.IsOk() )
        CurrentFont = GetFont();

    wxBoxSizer * MainSizer;
    MainSizer = new wxBoxSizer( wxVERTICAL );

    m_TitleSizer = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer * EditorSizer;
    EditorSizer = new wxBoxSizer( wxHORIZONTAL );

    m_UpdateCheckBox = new wxCheckBox( this, wxID_ANY, _( "Follow player" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_UpdateCheckBox->SetToolTip( _( "Search the lyrics for the current playing track" ) );
    m_UpdateCheckBox->SetValue( m_UpdateEnabled );

    EditorSizer->Add( m_UpdateCheckBox, 0, wxEXPAND|wxTOP|wxRIGHT|wxLEFT, 5 );

    m_SetupButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_search_engine ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_SetupButton->SetToolTip( _( "Configure lyrics preferences" ) );
    EditorSizer->Add( m_SetupButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP, 5 );

    m_ReloadButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_search_again ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_ReloadButton->SetToolTip( _( "Search for lyrics" ) );
    m_ReloadButton->Enable( false );
    EditorSizer->Add( m_ReloadButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP, 5 );

    m_EditButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_edit ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_EditButton->SetToolTip( _( "Edit the lyrics" ) );
    m_EditButton->Enable( false );
    EditorSizer->Add( m_EditButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP, 5 );

    m_SaveButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_doc_save ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_SaveButton->Enable( false );
    m_SaveButton->SetToolTip( _( "Save the lyrics" ) );
    EditorSizer->Add( m_SaveButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP, 5 );

    m_WebSearchButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_search ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_WebSearchButton->Enable( false );
    m_WebSearchButton->SetToolTip( _( "Search the lyrics on the web" ) );
    EditorSizer->Add( m_WebSearchButton, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT, 5 );

    wxStaticText * ArtistStaticText = new wxStaticText( this, wxID_ANY, _( "Artist:" ), wxDefaultPosition, wxDefaultSize, 0 );
    ArtistStaticText->Wrap( -1 );

    EditorSizer->Add( ArtistStaticText, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxLEFT, 5 );

    m_ArtistTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_ArtistTextCtrl->Enable( !m_UpdateEnabled );

    EditorSizer->Add( m_ArtistTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT, 5 );

    wxStaticText * TrackStaticText = new wxStaticText( this, wxID_ANY, _( "Track:" ), wxDefaultPosition, wxDefaultSize, 0 );
    TrackStaticText->Wrap( -1 );

    EditorSizer->Add( TrackStaticText, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxLEFT, 5 );

    m_TrackTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_TrackTextCtrl->Enable( !m_UpdateEnabled );

    EditorSizer->Add( m_TrackTextCtrl, 1, wxALIGN_CENTER_VERTICAL|wxTOP|wxRIGHT, 5 );

    m_TitleSizer->Add( EditorSizer, 1, wxEXPAND, 5 );

    wxStaticLine * TopLine;
    TopLine = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    m_TitleSizer->Add( TopLine, 0, wxEXPAND|wxALL, 5 );

    m_LyricTitle = new wxStaticText( this, wxID_ANY, wxT( "/" ) );

    m_TitleSizer->Add( m_LyricTitle, 0, wxALL|m_LyricAlign, 5 );

    MainSizer->Add( m_TitleSizer, 0, wxEXPAND, 5 );

    //m_LyricText = new wxHtmlWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_AUTO );
    m_LyricText = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, m_LyricAlign|wxTE_WORDWRAP|wxTE_MULTILINE|wxNO_BORDER );
    m_EditModeFGColor = wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT );
    m_EditModeBGColor = wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW );

    m_LyricText->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWFRAME ) );
    m_LyricText->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT ) );
    m_LyricText->SetFont( CurrentFont );

    CurrentFont.SetPointSize( CurrentFont.GetPointSize() + 2 );
    CurrentFont.SetWeight( wxFONTWEIGHT_BOLD );
    m_LyricTitle->SetFont( CurrentFont );

    MainSizer->Add( m_LyricText, 1, wxALL|wxEXPAND, 5 );

    this->SetSizer( MainSizer );
    this->Layout();

    SetDropTarget( new guLyricsPanelDropTarget( this ) );
    m_LyricText->SetDropTarget( new guLyricsPanelDropTarget( this ) );
    m_LyricTextTimer.SetOwner( this );

    m_UpdateCheckBox->Bind( wxEVT_CHECKBOX, &guLyricsPanel::OnUpdateChkBoxClicked, this );
    m_SetupButton->Bind( wxEVT_BUTTON, &guLyricsPanel::OnSetupSelected, this );
    m_ReloadButton->Bind( wxEVT_BUTTON, &guLyricsPanel::OnReloadBtnClick, this );
    m_EditButton->Bind( wxEVT_BUTTON, &guLyricsPanel::OnEditBtnClick, this );
    m_SaveButton->Bind( wxEVT_BUTTON, &guLyricsPanel::OnSaveBtnClick, this );
    m_WebSearchButton->Bind( wxEVT_BUTTON, &guLyricsPanel::OnWebSearchBtnClick, this );
    m_ArtistTextCtrl->Bind( wxEVT_TEXT, &guLyricsPanel::OnTextUpdated, this );
    m_TrackTextCtrl->Bind( wxEVT_TEXT, &guLyricsPanel::OnTextUpdated, this );
    Bind( wxEVT_TIMER, &guLyricsPanel::OnTextTimer, this );

    m_LyricText->Bind( wxEVT_CONTEXT_MENU, &guLyricsPanel::OnContextMenu, this );
    Bind( wxEVT_MENU, &guLyricsPanel::OnLyricsCopy, this, ID_LYRICS_COPY );
    Bind( wxEVT_MENU, &guLyricsPanel::OnLyricsPaste, this, ID_LYRICS_PASTE );
    Bind( wxEVT_MENU, &guLyricsPanel::OnLyricsPrint, this, ID_LYRICS_PRINT );

    Bind( guConfigUpdatedEvent, &guLyricsPanel::OnConfigUpdated, this, ID_CONFIG_UPDATED );

    Bind( wxEVT_MENU, &guLyricsPanel::OnLyricFound, this, ID_LYRICS_LYRICFOUND );
}

// -------------------------------------------------------------------------------- //
guLyricsPanel::~guLyricsPanel()
{
//    // Save the current selected server
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->WriteBool( CONFIG_KEY_LYRICS_FOLLOW_PLAYER, m_UpdateEnabled, CONFIG_PATH_LYRICS );
    Config->UnRegisterObject( this );

    m_UpdateCheckBox->Unbind( wxEVT_CHECKBOX, &guLyricsPanel::OnUpdateChkBoxClicked, this );
    m_SetupButton->Unbind( wxEVT_BUTTON, &guLyricsPanel::OnSetupSelected, this );
    m_ReloadButton->Unbind( wxEVT_BUTTON, &guLyricsPanel::OnReloadBtnClick, this );
    m_EditButton->Unbind( wxEVT_BUTTON, &guLyricsPanel::OnEditBtnClick, this );
    m_SaveButton->Unbind( wxEVT_BUTTON, &guLyricsPanel::OnSaveBtnClick, this );
    m_WebSearchButton->Unbind( wxEVT_BUTTON, &guLyricsPanel::OnWebSearchBtnClick, this );
    m_ArtistTextCtrl->Unbind( wxEVT_TEXT, &guLyricsPanel::OnTextUpdated, this );
    m_TrackTextCtrl->Unbind( wxEVT_TEXT, &guLyricsPanel::OnTextUpdated, this );
    Unbind( wxEVT_TIMER, &guLyricsPanel::OnTextTimer, this );

    m_LyricText->Unbind( wxEVT_CONTEXT_MENU, &guLyricsPanel::OnContextMenu, this );
    Unbind( wxEVT_MENU, &guLyricsPanel::OnLyricsCopy, this, ID_LYRICS_COPY );
    Unbind( wxEVT_MENU, &guLyricsPanel::OnLyricsPaste, this, ID_LYRICS_PASTE );
    Unbind( wxEVT_MENU, &guLyricsPanel::OnLyricsPrint, this, ID_LYRICS_PRINT );

    Unbind( guConfigUpdatedEvent, &guLyricsPanel::OnConfigUpdated, this, ID_CONFIG_UPDATED );

    Unbind( wxEVT_MENU, &guLyricsPanel::OnLyricFound, this, ID_LYRICS_LYRICFOUND );

    if( m_LyricSearchContext )
    {
        delete m_LyricSearchContext;
    }

    if( m_CurrentTrack )
    {
        delete m_CurrentTrack;
    }
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::OnConfigUpdated( wxCommandEvent &event )
{
    int Flags = event.GetInt();
    if( Flags & guPREFERENCE_PAGE_FLAG_LYRICS )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        if( Config )
        {
            wxFont CurrentFont;
            CurrentFont.SetNativeFontInfo( Config->ReadStr( CONFIG_KEY_LYRICS_FONT, wxEmptyString, CONFIG_PATH_LYRICS ) );
            if( !CurrentFont.IsOk() )
                CurrentFont = GetFont();

            m_LyricText->SetWindowStyle( m_LyricText->GetWindowStyle() ^ m_LyricAlign );
            m_LyricAlign = LyricAligns[ Config->ReadNum( CONFIG_KEY_LYRICS_TEXT_ALIGN, 1, CONFIG_PATH_LYRICS ) ];
            m_LyricText->SetWindowStyle( m_LyricText->GetWindowStyle() | m_LyricAlign );

            m_LyricText->SetDefaultStyle( wxTextAttr( m_LyricTitle->GetForegroundColour(),
                                                      m_LyricTitle->GetBackgroundColour(),
                                                      CurrentFont ) );

            SetText( m_LyricText->GetValue() );

            CurrentFont.SetPointSize( CurrentFont.GetPointSize() + 2 );
            CurrentFont.SetWeight( wxFONTWEIGHT_BOLD );
            m_LyricTitle->SetFont( CurrentFont );

            m_TitleSizer->Detach( m_LyricTitle );
            m_TitleSizer->Add( m_LyricTitle, 0, wxALL|m_LyricAlign, 5 );
            Layout();
        }
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
    MenuItem = new wxMenuItem( menu, ID_LYRICS_COPY, _( "Copy to Clipboard" ), _( "Copy the content of the lyric to clipboard" ) );
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
void guLyricsPanel::SetCurrentTrack( const guTrack * track )
{
    if( m_UpdateEnabled )
    {
        if( m_CurrentTrack )
        {
            delete m_CurrentTrack;
            m_CurrentTrack = NULL;
        }

        guTrackChangeInfo ChangeInfo;
        if( track )
        {
            ChangeInfo.m_ArtistName = track->m_ArtistName;
            ChangeInfo.m_TrackName = track->m_SongName;
            ChangeInfo.m_MediaViewer = track->m_MediaViewer;

            m_CurrentTrack = new guTrack( * track );
        }

        m_CurrentTrackInfo = ChangeInfo;

        m_LastSourceName = m_LastLyricText = m_CurrentLyricText = wxEmptyString;

        SetTrack( &ChangeInfo );
        m_CurrentLyricText = wxEmptyString;

        wxCommandEvent event( wxEVT_MENU, ID_MAINFRAME_UPDATE_SELINFO );
        AddPendingEvent( event );

        m_WebSearchButton->Enable( m_CurrentTrack );
    }
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::OnSetCurrentTrack( wxCommandEvent &event )
{
    SetCurrentTrack( ( guTrack * ) event.GetClientData() );
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::OnTextUpdated( wxCommandEvent& event )
{
    bool IsEnable = !m_ArtistTextCtrl->GetValue().IsEmpty() &&
                    !m_TrackTextCtrl->GetValue().IsEmpty();
    m_ReloadButton->Enable( IsEnable );
    m_WebSearchButton->Enable( IsEnable );
    if( m_LyricSearchContext )
    {
        delete m_LyricSearchContext;
        m_LyricSearchContext = NULL;
    }

//    if( !m_CurrentLyricText.IsEmpty() )
//    {
//        SetText( ( m_CurrentLyricText = wxEmptyString ) );
//    }

    SetTitle( m_TrackTextCtrl->GetValue() + wxT( " / " ) + m_ArtistTextCtrl->GetValue() );
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
        wxCommandEvent Event( wxEVT_MENU, ID_MAINFRAME_REQUEST_CURRENTTRACK );
        Event.SetClientData( this );
        wxPostEvent( guMainFrame::GetMainFrame(), Event );

        wxCommandEvent CmdEvent( wxEVT_MENU, ID_MAINFRAME_UPDATE_SELINFO );
        AddPendingEvent( CmdEvent );
    }

    m_ArtistTextCtrl->Enable( !m_UpdateEnabled );
    m_TrackTextCtrl->Enable( !m_UpdateEnabled );

    m_ReloadButton->Enable( !m_UpdateEnabled &&
                            !m_ArtistTextCtrl->IsEmpty() &&
                            !m_TrackTextCtrl->IsEmpty() );
    m_WebSearchButton->Enable( m_ReloadButton->IsEnabled() );
    m_EditButton->Enable( !m_CurrentLyricText.IsEmpty() );
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::OnSetupSelected( wxCommandEvent &event )
{
    wxCommandEvent CmdEvent( wxEVT_MENU, ID_MENU_PREFERENCES );
    CmdEvent.SetInt( guPREFERENCE_PAGE_LYRICS );
    wxPostEvent( guMainFrame::GetMainFrame(), CmdEvent );
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::OnReloadBtnClick( wxCommandEvent& event )
{
    if( m_UpdateEnabled )
    {
        wxCommandEvent CmdEvent( wxEVT_MENU, ID_MAINFRAME_LYRICSSEARCHNEXT );
        wxPostEvent( guMainFrame::GetMainFrame(), CmdEvent );

        guTrackChangeInfo TrackChangeInfo( m_ArtistTextCtrl->GetValue(), m_TrackTextCtrl->GetValue(), m_MediaViewer );
        SetTrack( &TrackChangeInfo, true );

        m_ReloadButton->Enable( false );
    }
    else
    {
        if( m_LyricSearchEngine )
        {
            if( !m_CurrentTrack )
            {
                m_CurrentTrack = new guTrack();
            }
            m_CurrentTrack->m_ArtistName = m_ArtistTextCtrl->GetValue();
            m_CurrentTrack->m_SongName = m_TrackTextCtrl->GetValue();

            SetTitle( m_TrackTextCtrl->GetValue() + wxT( " / " ) + m_ArtistTextCtrl->GetValue() );
            SetText( _( "Searching..." ) );

            if( !m_LyricSearchContext )
            {
                m_LyricSearchContext = m_LyricSearchEngine->CreateContext( this, m_CurrentTrack, false );
            }

            m_LyricSearchEngine->SearchStart( m_LyricSearchContext );
        }
    }
    wxCommandEvent CmdEvent( wxEVT_MENU, ID_MAINFRAME_UPDATE_SELINFO );
    AddPendingEvent( CmdEvent );
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::OnLyricFound( wxCommandEvent &event )
{
    wxString * LyricText = ( wxString * ) event.GetClientData();
    SetLyricText( LyricText, true );
    if( LyricText )
    {
        delete LyricText;
    }
    SetLastSource( event.GetInt() );
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::OnEditBtnClick( wxCommandEvent& event )
{
    m_ReloadButton->Enable( false );
    m_EditButton->Enable( false );
    m_SaveButton->Enable( true );
    m_LyricText->SetEditable( true );
    m_LyricText->SetBackgroundColour( m_EditModeBGColor );
    m_LyricText->SetForegroundColour( m_EditModeFGColor );

    SetText( m_CurrentLyricText );
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::OnSaveBtnClick( wxCommandEvent& event )
{
    if( m_LyricText->IsModified() )
    {
        m_CurrentLyricText = m_LyricText->GetValue();
    }

    if( m_UpdateEnabled )
    {
        wxCommandEvent CmdEvent( wxEVT_MENU, ID_MAINFRAME_LYRICSSAVECHANGES );
        CmdEvent.SetClientData( new wxString( m_CurrentLyricText ) );
        wxPostEvent( guMainFrame::GetMainFrame(), CmdEvent );
    }
    else
    {
        // Need to create a context for m_CurrentTrack and send the save message for that context
        if( m_LyricSearchEngine && m_CurrentTrack && !m_CurrentLyricText.IsEmpty() )
        {
            if( !m_LyricSearchContext )
            {
                m_LyricSearchContext = m_LyricSearchEngine->CreateContext( this, m_CurrentTrack, false );
            }
            m_LyricSearchEngine->SetLyricText( m_LyricSearchContext, m_CurrentLyricText );
        }
    }

    m_ReloadButton->Enable( true );
    m_SaveButton->Enable( false );
    m_EditButton->Enable( true );

    m_LyricText->SetEditable( false );
    m_LyricText->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWFRAME ) );
    m_LyricText->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT ) );

    SetText( m_CurrentLyricText );
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::OnWebSearchBtnClick( wxCommandEvent& event )
{
    if( !m_ArtistTextCtrl->IsEmpty() &&
        !m_TrackTextCtrl->IsEmpty() )
    {
        guWebExecute( wxT( "http://www.google.com/search?q=Lyrics+\"" ) +
                            guURLEncode( m_ArtistTextCtrl->GetValue() ) + wxT( "\"+\"" ) +
                            guURLEncode( m_TrackTextCtrl->GetValue() ) + wxT( "\"" ) );
    }
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
    m_LyricText->SetValue( wxEmptyString );
    m_LyricText->WriteText( text );
    m_LyricText->SetInsertionPoint( 0 );
    //guLogMessage( wxT( "SetText: '%s'" ), text.Mid( 0, 16 ).c_str() );
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::SetTrack( const guTrackChangeInfo * trackchangeinfo, const bool onlinesearch )
{
    //const wxString &artist, const wxString &tracK
    wxString Artist = trackchangeinfo->m_ArtistName;
    wxString Track = RemoveSearchFilters( trackchangeinfo->m_TrackName );

    m_LyricText->SetEditable( false );
    m_LyricText->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWFRAME ) );
    m_LyricText->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT ) );

    SetTitle( Track + wxT( " / " ) + Artist );
    SetText( _( "Searching..." ) );

    m_ArtistTextCtrl->SetValue( Artist );
    m_TrackTextCtrl->SetValue( Track );

    m_ReloadButton->Enable( false );
    m_EditButton->Enable( false );
    m_SaveButton->Enable( false );
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::OnTextTimer( wxTimerEvent &event )
{
    if( !m_LastLyricText.IsEmpty() )
    {
        SetText( m_LastLyricText );
        m_CurrentSourceName = m_LastSourceName;

        m_ReloadButton->Enable( true );
        m_EditButton->Enable( true );

        wxCommandEvent event( wxEVT_MENU, ID_MAINFRAME_UPDATE_SELINFO );
        AddPendingEvent( event );
    }
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
            guLogMessage( wxT( "Pasting:" ) );
            wxTextDataObject data;
            if( wxTheClipboard->GetData( data ) )
            {
                wxString PasteLyric = data.GetText();
                SetLyricText( &PasteLyric, true );

                OnSaveBtnClick( event );
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

//            Printout.SetHtmlText( wxString::Format(
//                ( ( m_LyricFormat == guLYRIC_FORMAT_NORMAL ) ? guLYRICS_TEMPLATE_DEFAULT : guLYRICS_TEMPLATE_ULTGUITAR ),
//                wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWFRAME ).GetAsString( wxC2S_HTML_SYNTAX ).c_str(),
//                wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT ).GetAsString( wxC2S_HTML_SYNTAX ).c_str(),
//                LyricText.c_str() ) );

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
//    //guLogMessage( wxT( "guLastFMPanelDropTarget::OnDropFiles" ) );
    guTrack Track;

    if( !files.Count() )
        return;

    if( !m_Db->FindTrackFile( files[ 0 ], &Track ) )
    {
        if( !Track.ReadFromFile( files[ 0 ] ) )
        {
            return;
        }
    }

    if( m_UpdateEnabled )
    {
        SetCurrentTrack( &Track );

        SetAutoUpdate( false );
    }
    else
    {
        m_ArtistTextCtrl->SetValue( Track.m_ArtistName );
        m_TrackTextCtrl->SetValue( Track.m_SongName );
        if( m_CurrentTrack )
        {
            delete m_CurrentTrack;
            m_CurrentTrack = NULL;
        }
    }
    m_CurrentLyricText = wxEmptyString;

    wxCommandEvent DummyEvent;
    OnReloadBtnClick( DummyEvent );
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::OnDropFiles( const guTrackArray * tracks )
{
    if( tracks && tracks->Count() )
    {
        const guTrack & Track= tracks->Item( 0 );
        if( m_UpdateEnabled )
        {
            SetCurrentTrack( &Track );

            SetAutoUpdate( false );
        }
        else
        {
            m_ArtistTextCtrl->SetValue( Track.m_ArtistName );
            m_TrackTextCtrl->SetValue( Track.m_SongName );
            if( m_CurrentTrack )
            {
                delete m_CurrentTrack;
                m_CurrentTrack = NULL;
            }
        }
        m_CurrentLyricText = wxEmptyString;

        wxCommandEvent DummyEvent;
        OnReloadBtnClick( DummyEvent );
    }
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::SetLyricText( const wxString * lyrictext, const bool forceupdate )
{
    if( ( m_UpdateEnabled || forceupdate ) && lyrictext )
    {
        if( lyrictext->IsEmpty() )
        {
            SetText( _( "No lyrics found" ) );
            //guLogMessage( wxT( "Current empty : %i" ), m_CurrentLyricText.IsEmpty() );

            if( !m_LastLyricText.IsEmpty() )
            {
                if( m_LyricTextTimer.IsRunning() )
                    m_LyricTextTimer.Stop();
                m_LyricTextTimer.Start( 3000, wxTIMER_ONE_SHOT );
            }
            m_EditButton->Enable( m_CurrentTrack );
        }
        else
        {
            SetText( * lyrictext );
            m_SaveButton->Enable( m_CurrentTrack && !m_UpdateEnabled );

            m_LastLyricText = m_CurrentLyricText = * lyrictext;
        }
    }
    m_ReloadButton->Enable( true );
    m_EditButton->Enable( m_CurrentTrack );
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::SetLastSource( const int sourceindex )
{
    //guLogMessage( wxT( "Setting the lyrics source index to %i" ), sourceindex );

    if( sourceindex == wxNOT_FOUND )
    {
        m_CurrentSourceName = wxEmptyString;
    }
    else
    {
        guLyricSource * LyricSource = m_LyricSearchEngine->GetSource( sourceindex );
        if( LyricSource )
        {
            m_LastSourceName = m_CurrentSourceName = LyricSource->Name();
        }
    }

    wxCommandEvent event( wxEVT_MENU, ID_MAINFRAME_UPDATE_SELINFO );
    AddPendingEvent( event );
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::UpdatedTracks( const guTrackArray * tracks )
{
    if( !m_CurrentTrack )
        return;

    int Index;
    int Count = tracks->Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guTrack * Track = &tracks->Item( Index );
        if( Track->m_FileName == m_CurrentTrack->m_FileName )
        {
            SetCurrentTrack( Track );

            wxCommandEvent Event( wxEVT_MENU, ID_MAINFRAME_LYRICSSEARCHFIRST );
            wxPostEvent( guMainFrame::GetMainFrame(), Event );

            return;
        }
    }
}

// -------------------------------------------------------------------------------- //
void guLyricsPanel::UpdatedTrack( const guTrack * track )
{
    if( track->m_FileName == m_CurrentTrack->m_FileName )
    {
        SetCurrentTrack( track );

        wxCommandEvent Event( wxEVT_MENU, ID_MAINFRAME_LYRICSSEARCHFIRST );
        wxPostEvent( guMainFrame::GetMainFrame(), Event );
    }
}

// -------------------------------------------------------------------------------- //
wxString guLyricsPanel::GetLyricSource( void )
{
    if( !m_CurrentLyricText.IsEmpty() )
    {
        return _( "Lyrics from " ) + m_CurrentSourceName;
    }
    else if( m_LyricText->GetValue() == _( "Searching..." ) )
    {
        return _( "Searching..." );
    }
    else
    {
        return _( "No lyrics found" );
    }
}


// -------------------------------------------------------------------------------- //
// guLyricsPanelDropTarget
// -------------------------------------------------------------------------------- //
guLyricsPanelDropTarget::guLyricsPanelDropTarget( guLyricsPanel * lyricspanel ) : wxDropTarget()
{
    m_LyricsPanel = lyricspanel;

    wxDataObjectComposite * DataObject = new wxDataObjectComposite();
    wxCustomDataObject * TracksDataObject = new wxCustomDataObject( wxDataFormat( wxT( "x-gutracks/guayadeque-copied-tracks" ) ) );
    DataObject->Add( TracksDataObject, true );
    wxFileDataObject * FileDataObject = new wxFileDataObject();
    DataObject->Add( FileDataObject, false );
    SetDataObject( DataObject );
}

// -------------------------------------------------------------------------------- //
guLyricsPanelDropTarget::~guLyricsPanelDropTarget()
{
}

// -------------------------------------------------------------------------------- //
wxDragResult guLyricsPanelDropTarget::OnData( wxCoord x, wxCoord y, wxDragResult def )
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
            m_LyricsPanel->OnDropFiles( Tracks );
        }
    }
    else if( ReceivedFormat == wxDataFormat( wxDF_FILENAME ) )
    {
        wxFileDataObject * FileDataObject = ( wxFileDataObject * ) DataObject->GetDataObject( wxDataFormat( wxDF_FILENAME ) );
        if( FileDataObject )
        {
            m_LyricsPanel->OnDropFiles( FileDataObject->GetFilenames() );
        }
    }

    return def;
}






// -------------------------------------------------------------------------------- //
// guLyricSearchContext
// -------------------------------------------------------------------------------- //
guLyricSearchContext::~guLyricSearchContext()
{
    m_LyricSearchEngine->RemoveContextThread( this );
}

// -------------------------------------------------------------------------------- //
bool guLyricSearchContext::GetNextSource( guLyricSource * lyricsource, const bool allowloop )
{
    return m_LyricSearchEngine->GetNextSource( this, lyricsource, allowloop );
}




// -------------------------------------------------------------------------------- //
// guLyricSourceOption
// -------------------------------------------------------------------------------- //
guLyricSourceOption::guLyricSourceOption( wxXmlNode * xmlnode, const wxString &tag1, const wxString &tag2 )
{
    if( xmlnode )
    {
        xmlnode->GetAttribute( tag1, &m_Text1 );
        if( !tag2.IsEmpty() )
        {
            xmlnode->GetAttribute( tag2, &m_Text2 );
        }
    }
}


// -------------------------------------------------------------------------------- //
guLyricSourceExtract::guLyricSourceExtract( wxXmlNode * xmlnode )
{
    if( xmlnode )
    {
        if( xmlnode->HasAttribute( wxT( "tag" ) ) )
        {
            xmlnode->GetAttribute( wxT( "tag" ), &m_Text1 );
        }
        else
        {
            xmlnode->GetAttribute( wxT( "begin" ), &m_Text1 );
            xmlnode->GetAttribute( wxT( "end" ), &m_Text2 );
        }
    }
}

// -------------------------------------------------------------------------------- //
// guLyricSource
// -------------------------------------------------------------------------------- //
guLyricSource::guLyricSource( wxXmlNode * xmlnode )
{
    if( xmlnode )
    {
        wxString TypeStr;
        xmlnode->GetAttribute( wxT( "type" ), &TypeStr );
        if( TypeStr == wxT( "download" ) )
        {
            m_Type = guLYRIC_SOURCE_TYPE_DOWNLOAD;
        }
        else if( TypeStr == wxT( "file" ) )
        {
            m_Type = guLYRIC_SOURCE_TYPE_FILE;
        }
        else if( TypeStr == wxT( "command" ) )
        {
            m_Type = guLYRIC_SOURCE_TYPE_COMMAND;
        }
        else if( TypeStr == wxT( "embedded" ) )
        {
            m_Type = guLYRIC_SOURCE_TYPE_EMBEDDED;
        }
        wxString EnabledStr;
        xmlnode->GetAttribute( wxT( "enabled" ), &EnabledStr );
        m_Enabled = ( EnabledStr == wxT( "true" ) );
        xmlnode->GetAttribute( wxT( "name" ), &m_Name );
        xmlnode->GetAttribute( wxT( "source" ), &m_Source );
        xmlnode = xmlnode->GetChildren();
        while( xmlnode )
        {
            wxString NodeName = xmlnode->GetName();
            if( NodeName == wxT( "replace" ) )
            {
                ReadReplaceItems( xmlnode->GetChildren() );
            }
            else if( NodeName == wxT( "extract" ) )
            {
                ReadExtractItems( xmlnode->GetChildren() );
            }
            else if( NodeName == wxT( "exclude" ) )
            {
                ReadExcludeItems( xmlnode->GetChildren() );
            }
            else if( NodeName == wxT( "notfound" ) )
            {
                ReadNotFoundItems( xmlnode->GetChildren() );
            }
            xmlnode = xmlnode->GetNext();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guLyricSource::ReadReplaceItems( wxXmlNode * xmlnode )
{
    while( xmlnode )
    {
        guLyricSourceReplace * LyricSourceReplace = new guLyricSourceReplace( xmlnode );
        m_ReplaceItems.Add( LyricSourceReplace );
        xmlnode = xmlnode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void guLyricSource::ReadExtractItems( wxXmlNode * xmlnode )
{
    while( xmlnode )
    {
        guLyricSourceExtract * LyricSourceExtract = new guLyricSourceExtract( xmlnode );
        m_ExtractItems.Add( LyricSourceExtract );
        xmlnode = xmlnode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void guLyricSource::ReadExcludeItems( wxXmlNode * xmlnode )
{
    while( xmlnode )
    {
        guLyricSourceExclude * LyricSourceExclude = new guLyricSourceExclude( xmlnode );
        m_ExcludeItems.Add( LyricSourceExclude );
        xmlnode = xmlnode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void guLyricSource::ReadNotFoundItems( wxXmlNode * xmlnode )
{
    while( xmlnode )
    {
        wxString NotFoundMsg;
        xmlnode->GetAttribute( wxT( "tag" ), &NotFoundMsg );
        m_NotFoundItems.Add( NotFoundMsg );
        xmlnode = xmlnode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
wxString guCharsReplace( const wxString &text, const wxString &bannedchars, const wxString &replacechar )
{
    wxString RetVal;
    int Index;
    int Count = text.Length();
    for( Index = 0; Index < Count; Index++ )
    {
        wxChar C = text[ Index ];
        if( wxStrchr( bannedchars, C ) )
            C = replacechar[ 0 ];
        RetVal.Append( C );
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
wxString guLyricSource::SourceFieldClean( const wxString &field )
{
    wxString RetVal = field;
    int Index;
    int Count = m_ReplaceItems.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guLyricSourceReplace SourceReplace = m_ReplaceItems[ Index ];
        RetVal = guCharsReplace( RetVal, SourceReplace.Search(), SourceReplace.Replace() );
    }
    return RetVal;
}


// -------------------------------------------------------------------------------- //
// guLyricSearchEngine
// -------------------------------------------------------------------------------- //
guLyricSearchEngine::guLyricSearchEngine()
{
    // Load the search engines...
    Load();
}

// -------------------------------------------------------------------------------- //
guLyricSearchEngine::~guLyricSearchEngine()
{
    m_LyricSearchThreadsMutex.Lock();
    int Index;
    int Count;
    if( ( Count = m_LyricSearchThreads.Count() ) )
    {
        for( Index = 0; Index < Count; Index++ )
        {
            guLyricSearchThread * LyrycSearchThread = m_LyricSearchThreads[ Index ];
            LyrycSearchThread->Pause();
            LyrycSearchThread->Delete();
        }
    }
    m_LyricSearchThreadsMutex.Unlock();

}

// -------------------------------------------------------------------------------- //
void guLyricSearchEngine::ReadSources( wxXmlNode * xmlnode )
{
    while( xmlnode && xmlnode->GetName() == wxT( "lyricsource" ) )
    {
        guLyricSource * LyricSource = new guLyricSource( xmlnode );
        m_LyricSources.Add( LyricSource );
        xmlnode = xmlnode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void guLyricSearchEngine::ReadTargets( wxXmlNode * xmlnode )
{
    while( xmlnode && xmlnode->GetName() == wxT( "lyrictarget" ) )
    {
        guLyricSource * LyricTarget = new guLyricSource( xmlnode );
        m_LyricTargets.Add( LyricTarget );
        if( !m_TargetsEnabled )
        {
            if( LyricTarget->Enabled() )
                m_TargetsEnabled = true;
        }
        xmlnode = xmlnode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void guLyricSearchEngine::Load( void )
{
    m_LyricSources.Empty();
    m_LyricTargets.Empty();
    m_TargetsEnabled = false;
    wxString LyricsConfFile = wxGetHomeDir() + wxT( "/.guayadeque/lyrics_sources.xml" );
    if( wxFileExists( LyricsConfFile ) )
    {
        wxFileInputStream Ins( LyricsConfFile );
        if( Ins.IsOk() )
        {
            wxXmlDocument XmlDoc( Ins );
            wxXmlNode * XmlNode = XmlDoc.GetRoot();
            //guLogMessage( wxT( "Reading: %s" ), XmlNode->GetName().c_str() );
            if( XmlNode && XmlNode->GetName() == wxT( "lyricengine" ) )
            {
                XmlNode = XmlNode->GetChildren();
                while( XmlNode )
                {
                    //guLogMessage( wxT( "Reading: %s" ), XmlNode->GetName().c_str() );
                    if( XmlNode->GetName() == wxT( "lyricsources" ) )
                    {
                        ReadSources( XmlNode->GetChildren() );
                    }
                    else if( XmlNode->GetName() == wxT( "lyrictargets" ) )
                    {
                        ReadTargets( XmlNode->GetChildren() );
                    }
                    XmlNode = XmlNode->GetNext();
                }
            }
        }
    }
    else
    {
        guLogError( wxT( "The lyrics source configuration file was not found" ) );
    }
}

// -------------------------------------------------------------------------------- //
bool guLyricSearchEngine::GetNextSource( guLyricSearchContext * context, guLyricSource * source, const bool allowloop )
{
    bool LoopedOnce = false;
    int CurrentIndex = context->m_CurrentIndex;
    while( true )
    {
        CurrentIndex++;
        if( CurrentIndex >= ( int ) m_LyricSources.Count() )
        {
            if( allowloop && !LoopedOnce )
            {
                LoopedOnce = true;
                CurrentIndex = 0;
            }
            else
            {
                context->m_CurrentIndex = wxNOT_FOUND;
                break;
            }
        }

        if( m_LyricSources[ CurrentIndex ].Enabled() )
        {
            * source = m_LyricSources[ CurrentIndex ];
            context->m_CurrentIndex = CurrentIndex;
            return true;
        }
    }
    return false;
}

// -------------------------------------------------------------------------------- //
void guLyricSearchEngine::SourceMoveUp( const int index )
{
    m_LyricSearchThreadsMutex.Lock();
    guLyricSource * LyricSource = m_LyricSources.Detach( index );
    m_LyricSources.Insert( LyricSource, index - 1 );
    m_LyricSearchThreadsMutex.Unlock();
}

// -------------------------------------------------------------------------------- //
void guLyricSearchEngine::SourceMoveDown( const int index )
{
    m_LyricSearchThreadsMutex.Lock();
    guLyricSource * LyricSource = m_LyricSources.Detach( index );
    m_LyricSources.Insert( LyricSource, index + 1 );
    m_LyricSearchThreadsMutex.Unlock();
}

// -------------------------------------------------------------------------------- //
void guLyricSearchEngine::TargetMoveUp( const int index )
{
    m_LyricSearchThreadsMutex.Lock();
    guLyricSource * LyricTarget = m_LyricTargets.Detach( index );
    m_LyricTargets.Insert( LyricTarget, index - 1 );
    m_LyricSearchThreadsMutex.Unlock();
}

// -------------------------------------------------------------------------------- //
void guLyricSearchEngine::TargetMoveDown( const int index )
{
    m_LyricSearchThreadsMutex.Lock();
    guLyricSource * LyricTarget = m_LyricTargets.Detach( index );
    m_LyricTargets.Insert( LyricTarget, index + 1 );
    m_LyricSearchThreadsMutex.Unlock();
}

// -------------------------------------------------------------------------------- //
void SaveLyricNotFound( wxXmlNode * xmlnode, guLyricSource * lyricsource )
{
    int Index;
    int Count;
    if( !( Count = lyricsource->NotFoundCount() ) )
        return;

    wxXmlNode * NotFoundNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "notfound" ) );

    for( Index = 0; Index < Count; Index++ )
    {
        wxString NotFoundTag = lyricsource->NotFoundItem( Index );

        wxXmlNode * ItemNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "item" ) );

        wxXmlAttribute * TagProperty = new wxXmlAttribute( wxT( "tag" ), NotFoundTag, NULL );
        ItemNode->SetAttributes( TagProperty );

        NotFoundNode->AddChild( ItemNode );
    }

    xmlnode->AddChild( NotFoundNode );
}

// -------------------------------------------------------------------------------- //
void SaveLyricExclude( wxXmlNode * xmlnode, guLyricSource * lyricsource )
{
    int Index;
    int Count;
    if( !( Count = lyricsource->ExcludeCount() ) )
        return;

    wxXmlNode * ExcludeNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "exclude" ) );

    for( Index = 0; Index < Count; Index++ )
    {
        guLyricSourceExtract * ExcludeItem = lyricsource->ExcludeItem( Index );

        wxXmlNode * ItemNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "item" ) );

        if( ExcludeItem->IsSingleOption() )
        {
            wxXmlAttribute * TagProperty = new wxXmlAttribute( wxT( "tag" ), ExcludeItem->Tag(), NULL );
            ItemNode->SetAttributes( TagProperty );
        }
        else
        {
            wxXmlAttribute * EndProperty = new wxXmlAttribute( wxT( "end" ), ExcludeItem->End(), NULL );
            wxXmlAttribute * BeginProperty = new wxXmlAttribute( wxT( "begin" ), ExcludeItem->Begin(), EndProperty );

            ItemNode->SetAttributes( BeginProperty );
        }

        ExcludeNode->AddChild( ItemNode );
    }

    xmlnode->AddChild( ExcludeNode );
}

// -------------------------------------------------------------------------------- //
void SaveLyricExtract( wxXmlNode * xmlnode, guLyricSource * lyricsource )
{
    int Index;
    int Count;
    if( !( Count = lyricsource->ExtractCount() ) )
        return;

    wxXmlNode * ExtractNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "extract" ) );

    for( Index = 0; Index < Count; Index++ )
    {
        guLyricSourceExtract * ExtractItem = lyricsource->ExtractItem( Index );

        wxXmlNode * ItemNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "item" ) );

        if( ExtractItem->IsSingleOption() )
        {
            wxXmlAttribute * TagProperty = new wxXmlAttribute( wxT( "tag" ), ExtractItem->Tag(), NULL );
            ItemNode->SetAttributes( TagProperty );
        }
        else
        {
            wxXmlAttribute * EndProperty = new wxXmlAttribute( wxT( "end" ), ExtractItem->End(), NULL );
            wxXmlAttribute * BeginProperty = new wxXmlAttribute( wxT( "begin" ), ExtractItem->Begin(), EndProperty );

            ItemNode->SetAttributes( BeginProperty );
        }

        ExtractNode->AddChild( ItemNode );
    }

    xmlnode->AddChild( ExtractNode );
}


// -------------------------------------------------------------------------------- //
void SaveLyricReplace( wxXmlNode * xmlnode, guLyricSource * lyricsource )
{
    int Index;
    int Count;
    if( !( Count = lyricsource->ReplaceCount() ) )
        return;

    wxXmlNode * ReplaceNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "replace" ) );

    for( Index = 0; Index < Count; Index++ )
    {
        guLyricSourceReplace * ReplaceItem = lyricsource->ReplaceItem( Index );

        wxXmlNode * ItemNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "item" ) );

        wxXmlAttribute * WithProperty = new wxXmlAttribute( wxT( "with" ), ReplaceItem->Replace(), NULL );
        wxXmlAttribute * ReplaceProperty = new wxXmlAttribute( wxT( "replace" ), ReplaceItem->Search(), WithProperty );

        ItemNode->SetAttributes( ReplaceProperty );

        ReplaceNode->AddChild( ItemNode );
    }

    xmlnode->AddChild( ReplaceNode );
}


// -------------------------------------------------------------------------------- //
void SaveLyricSource( wxXmlNode * xmlnode, guLyricSource * lyricsource, const wxString &tagname )
{
    wxXmlNode * LyricSourceNode = new wxXmlNode( wxXML_ELEMENT_NODE, tagname );

    wxXmlAttribute * SourceProperty = new wxXmlAttribute( wxT( "source" ), lyricsource->Source(), NULL );
    wxXmlAttribute * NameProperty = new wxXmlAttribute( wxT( "name" ), lyricsource->Name(), SourceProperty );
    wxXmlAttribute * EnabledProperty = new wxXmlAttribute( wxT( "enabled" ), lyricsource->Enabled() ? wxT( "true" ) : wxT( "false" ), NameProperty );

    wxString SourceType;
    if( lyricsource->Type() == guLYRIC_SOURCE_TYPE_DOWNLOAD )
        SourceType = wxT( "download" );
    else if( lyricsource->Type() == guLYRIC_SOURCE_TYPE_EMBEDDED )
        SourceType = wxT( "embedded" );
    else if( lyricsource->Type() == guLYRIC_SOURCE_TYPE_FILE )
        SourceType = wxT( "file" );
    else if( lyricsource->Type() == guLYRIC_SOURCE_TYPE_COMMAND )
        SourceType = wxT( "command" );
    else
        SourceType = wxT( "invalid" );
    wxXmlAttribute * TypeNode = new wxXmlAttribute( wxT( "type" ), SourceType, EnabledProperty );

    LyricSourceNode->SetAttributes( TypeNode );

    SaveLyricReplace( LyricSourceNode, lyricsource );
    SaveLyricExtract( LyricSourceNode, lyricsource );
    SaveLyricExclude( LyricSourceNode, lyricsource );
    SaveLyricNotFound( LyricSourceNode, lyricsource );

    xmlnode->AddChild( LyricSourceNode );

}

// -------------------------------------------------------------------------------- //
bool guLyricSearchEngine::Save( void )
{
    wxXmlDocument OutXml;
    wxXmlNode * RootNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "lyricengine" ) );

    wxXmlNode * SourcesNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "lyricsources" ) );

    int Index;
    int Count = m_LyricSources.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        SaveLyricSource( SourcesNode, &m_LyricSources[ Index ], wxT( "lyricsource" ) );
    }
    RootNode->AddChild( SourcesNode );

    wxXmlNode * TargetsNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "lyrictargets" ) );
    Count = m_LyricTargets.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        SaveLyricSource( TargetsNode, &m_LyricTargets[ Index ], wxT( "lyrictarget" ) );
    }
    RootNode->AddChild( TargetsNode );

    OutXml.SetRoot( RootNode );
    return OutXml.Save( wxGetHomeDir() + wxT( "/.guayadeque/lyrics_sources.xml" ) );
}

// -------------------------------------------------------------------------------- //
guLyricSearchContext * guLyricSearchEngine::CreateContext( wxEvtHandler * owner, guTrack * track, const bool dosaveprocess )
{
    return new guLyricSearchContext( this, owner, track, dosaveprocess );
}

// -------------------------------------------------------------------------------- //
void guLyricSearchEngine::SearchStart( guLyricSearchContext * context )
{
    RemoveContextThread( context );

    m_LyricSearchThreadsMutex.Lock();
    guLyricSearchThread * LyricSearchThread = new guLyricSearchThread( context );
    if( LyricSearchThread )
    {
        m_LyricSearchThreads.Add( LyricSearchThread );
    }
    m_LyricSearchThreadsMutex.Unlock();
}

// -------------------------------------------------------------------------------- //
void guLyricSearchEngine::SetLyricText( guLyricSearchContext * context, const wxString &lyrictext )
{
    if( !lyrictext.IsEmpty() )
    {
        RemoveContextThread( context );

        m_LyricSearchThreadsMutex.Lock();
        guLyricSearchThread * LyricSearchThread = new guLyricSearchThread( context, lyrictext, true );
        if( LyricSearchThread )
        {
            m_LyricSearchThreads.Add( LyricSearchThread );
        }
        m_LyricSearchThreadsMutex.Unlock();
    }
}

// -------------------------------------------------------------------------------- //
void guLyricSearchEngine::SearchFinished( guLyricSearchThread * searchthread )
{
    m_LyricSearchThreadsMutex.Lock();

    wxCommandEvent LyricEvent( wxEVT_MENU, ID_LYRICS_LYRICFOUND );
    LyricEvent.SetInt( searchthread->LyricSearchContext()->m_CurrentIndex );
    LyricEvent.SetClientData( new wxString( searchthread->LyricText() ) );
    wxPostEvent( searchthread->LyricSearchContext()->Owner(), LyricEvent );

    m_LyricSearchThreads.Remove( searchthread );

    m_LyricSearchThreadsMutex.Unlock();
}

// -------------------------------------------------------------------------------- //
void guLyricSearchEngine::RemoveContextThread( guLyricSearchContext * searchcontext )
{
    m_LyricSearchThreadsMutex.Lock();
    int Index;
    int Count = m_LyricSearchThreads.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guLyricSearchThread * LyricSearchThread = m_LyricSearchThreads[ Index ];
        if( LyricSearchThread->LyricSearchContext() == searchcontext )
        {
            LyricSearchThread->Pause();
            LyricSearchThread->Delete();
            m_LyricSearchThreads.RemoveAt( Index );
            break;
        }
    }
    m_LyricSearchThreadsMutex.Unlock();
}




// -------------------------------------------------------------------------------- //
// guLyricSearchThread
// -------------------------------------------------------------------------------- //
guLyricSearchThread::guLyricSearchThread( guLyricSearchContext * context, const wxString &lyrictext, const bool forcesaveprocess ) : wxThread()
{
    m_LyricSearchContext = context;
    m_LyricText = lyrictext;
    m_ForceSaveProcess = forcesaveprocess;
    m_CommandIsExecuting = false;
    m_NotifyHere = NULL;

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guLyricSearchThread::~guLyricSearchThread()
{
    guLogDebug( "guLyricSearchThread::~guLyricSearchThread" );
    if( !TestDestroy() )
    {
        m_LyricSearchContext->m_LyricSearchEngine->SearchFinished( this );
    }
    if( m_NotifyHere != NULL )
        *m_NotifyHere = false;
}

// -------------------------------------------------------------------------------- //
bool guLyricSearchThread::CheckNotFound( guLyricSource &lyricsource )
{
    int Index;
    int Count = lyricsource.NotFoundCount();
    for( Index = 0; Index < Count; Index++ )
    {
        wxString NotFoundLabel = lyricsource.NotFoundItem( Index );
        if( m_LyricText.Find( NotFoundLabel ) != wxNOT_FOUND )
        {
            return true;
        }
        if( TestDestroy() )
            break;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
wxString DoExtractTags( const wxString &content, const wxString &begin, const wxString &end )
{
    //guLogMessage( wxT( "CheckExtract: '%s' -> '%s'" ), begin.c_str(), end.c_str() );
    int FoundPos = content.Find( begin );
    if( FoundPos != wxNOT_FOUND )
    {
        wxString Content = content.Mid( FoundPos + begin.Length() );
        FoundPos = Content.Find( end );
        if( FoundPos != wxNOT_FOUND )
        {
            //guLogMessage( wxT( "Found:\n%s" ), Content.Mid( 0, FoundPos ).c_str() );
            return Content.Mid( 0, FoundPos );
        }
    }
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
wxString DoExtractTag( const wxString &content, const wxString &tag )
{
    if( content.Find( tag ) != wxNOT_FOUND )
    {
        wxString TagEnd = ( tag.Find( wxT( " " ) ) != wxNOT_FOUND ) ? tag.BeforeFirst( wxT( ' ' ) ) + wxT( ">" ) : tag;
        TagEnd.Replace( wxT( "<" ), wxT( "</" ) );
        return DoExtractTags( content, tag, TagEnd );
    }
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
wxString guLyricSearchThread::CheckExtract( const wxString &content, guLyricSource &lyricsource )
{
    wxString RetVal = wxEmptyString;
    int Index;
    int Count = lyricsource.ExtractCount();
    if( !Count )
        return content;
    for( Index = 0; Index < Count; Index++ )
    {
        guLyricSourceExtract * LyricSourceExtract = lyricsource.ExtractItem( Index );
        if( LyricSourceExtract->IsSingleOption() )
        {
            RetVal = DoExtractTag( content, LyricSourceExtract->Tag() );
        }
        else
        {
            RetVal = DoExtractTags( content, LyricSourceExtract->Begin(), LyricSourceExtract->End() );
        }
        if( TestDestroy() )
            break;
        if( !RetVal.IsEmpty() )
            break;
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
wxString DoExcludeTags( const wxString &content, const wxString &begin, const wxString &end )
{
    int BeginPos = content.Find( begin );
    if( BeginPos != wxNOT_FOUND )
    {
        int EndPos = content.Find( end );
        if( EndPos != wxNOT_FOUND )
        {
            EndPos += end.Length();
            return content.Mid( 0, BeginPos ) + content.Mid( EndPos );
        }
    }
    return content;
}

// -------------------------------------------------------------------------------- //
wxString DoExcludeTag( const wxString &content, const wxString &tag )
{
    if( content.Find( tag ) != wxNOT_FOUND )
    {
        wxString TagEnd;
        if( tag.Find( wxT( " " ) ) != wxNOT_FOUND )
            TagEnd = tag.BeforeFirst( wxT( ' ' ) ) + wxT( ">" );
        else
            TagEnd = tag;
        TagEnd.Replace( wxT( "<" ), wxT( "</" ) );
        return DoExcludeTags( content, tag, TagEnd );
    }
    return content;
}

// -------------------------------------------------------------------------------- //
wxString guLyricSearchThread::CheckExclude( const wxString &content, guLyricSource &lyricsource )
{
    wxString RetVal = content;
    int Index;
    int Count = lyricsource.ExcludeCount();
    for( Index = 0; Index < Count; Index++ )
    {
        guLyricSourceExclude * LyricSourceExclude = lyricsource.ExcludeItem( Index );
        if( LyricSourceExclude->IsSingleOption() )
        {
            RetVal = DoExcludeTag( RetVal, LyricSourceExclude->Tag() );
        }
        else
        {
            RetVal = DoExcludeTags( RetVal, LyricSourceExclude->Begin(), LyricSourceExclude->End() );
        }
        if( TestDestroy() || RetVal.IsEmpty() )
            break;
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
wxString guLyricSearchThread::DoReplace( const wxString &text, const wxString &search, const wxString &replace )
{
    wxString RetVal = text;
    wxRegEx RegEx( wxT( "[" ) + search + wxT( "]" ) );
    RegEx.Replace( &RetVal, replace );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
wxString guLyricSearchThread::DoReplace( const wxString &text, guLyricSource &lyricsource )
{
    wxString RetVal = text;
    int Index;
    int Count = lyricsource.ReplaceCount();
    for( Index = 0; Index < Count; Index++ )
    {
        guLyricSourceReplace * LyricSourceReplace = lyricsource.ReplaceItem( Index );
        RetVal = DoReplace( RetVal, LyricSourceReplace->Search(), LyricSourceReplace->Replace() );
    }
    return guURLEncode( RetVal );
}

// -------------------------------------------------------------------------------- //
wxString SpecialCase( const wxString &text )
{
    wxString RetVal;
    wxArrayString Words = wxStringTokenize( text, wxT( " " ) );
    int Index;
    int Count = Words.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        Words[ Index ][ 0 ] = wxToupper( Words[ Index ][ 0 ] );
        RetVal += Words[ Index ] + wxT( " " );
    }
    RetVal.Trim( 1 );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guLyricSearchThread::ProcessTags( wxString * text, guLyricSource &lyricsource )
{
    guTrack * Track = &m_LyricSearchContext->m_Track;

    if( text->Find( guLYRICS_ARTIST ) != wxNOT_FOUND )
        text->Replace( guLYRICS_ARTIST, DoReplace( Track->m_ArtistName, lyricsource ) );
    if( text->Find( guLYRICS_ARTIST_LOWER ) != wxNOT_FOUND )
        text->Replace( guLYRICS_ARTIST_LOWER, DoReplace( Track->m_ArtistName.Lower(), lyricsource ) );
    if( text->Find( guLYRICS_ARTIST_UPPER ) != wxNOT_FOUND )
        text->Replace( guLYRICS_ARTIST_UPPER, DoReplace( Track->m_ArtistName.Upper(), lyricsource ) );
    if( text->Find( guLYRICS_ARTIST_FIRST ) != wxNOT_FOUND )
        text->Replace( guLYRICS_ARTIST_FIRST, DoReplace( Track->m_ArtistName.Trim( false )[ 0 ], lyricsource ) );
    if( text->Find( guLYRICS_ARTIST_LOWER_FIRST ) != wxNOT_FOUND )
        text->Replace( guLYRICS_ARTIST_LOWER_FIRST, DoReplace( Track->m_ArtistName.Lower().Trim( false )[ 0 ], lyricsource ) );
    if( text->Find( guLYRICS_ARTIST_UPPER_FIRST ) != wxNOT_FOUND )
        text->Replace( guLYRICS_ARTIST_UPPER_FIRST, DoReplace( Track->m_ArtistName.Upper().Trim( false )[ 0 ], lyricsource ) );
    if( text->Find( guLYRICS_ARTIST_CAPITALIZE ) )
        text->Replace( guLYRICS_ARTIST_CAPITALIZE, DoReplace( SpecialCase( Track->m_ArtistName.Lower() ), lyricsource ) );

    if( text->Find( guLYRICS_ALBUM ) != wxNOT_FOUND )
        text->Replace( guLYRICS_ALBUM, DoReplace( Track->m_AlbumName, lyricsource ) );
    if( text->Find( guLYRICS_ALBUM_LOWER ) != wxNOT_FOUND )
        text->Replace( guLYRICS_ALBUM_LOWER, DoReplace( Track->m_AlbumName.Lower(), lyricsource ) );
    if( text->Find( guLYRICS_ALBUM_UPPER ) != wxNOT_FOUND )
        text->Replace( guLYRICS_ALBUM_UPPER, DoReplace( Track->m_AlbumName.Upper(), lyricsource ) );
    if( text->Find( guLYRICS_ALBUM_FIRST ) != wxNOT_FOUND )
        text->Replace( guLYRICS_ALBUM_FIRST, DoReplace( Track->m_AlbumName.Trim( false )[ 0 ], lyricsource ) );
    if( text->Find( guLYRICS_ALBUM_LOWER_FIRST ) != wxNOT_FOUND )
        text->Replace( guLYRICS_ALBUM_LOWER_FIRST, DoReplace( Track->m_AlbumName.Lower().Trim( false )[ 0 ], lyricsource ) );
    if( text->Find( guLYRICS_ALBUM_UPPER_FIRST ) != wxNOT_FOUND )
        text->Replace( guLYRICS_ALBUM_UPPER_FIRST, DoReplace( Track->m_AlbumName.Upper().Trim( false )[ 0 ], lyricsource ) );
    if( text->Find( guLYRICS_ALBUM_CAPITALIZE ) )
        text->Replace( guLYRICS_ALBUM_CAPITALIZE, DoReplace( SpecialCase( Track->m_AlbumName.Lower() ), lyricsource ) );
    if( text->Find( guLYRICS_ALBUM_PATH ) != wxNOT_FOUND )
        text->Replace( guLYRICS_ALBUM_PATH, Track->m_FileName.BeforeLast( wxT( '/' ) ) );

    if( text->Find( guLYRICS_TITLE ) != wxNOT_FOUND )
        text->Replace( guLYRICS_TITLE, DoReplace( Track->m_SongName, lyricsource ) );
    if( text->Find( guLYRICS_TITLE_LOWER ) != wxNOT_FOUND )
        text->Replace( guLYRICS_TITLE_LOWER, DoReplace( Track->m_SongName.Lower(), lyricsource ) );
    if( text->Find( guLYRICS_TITLE_UPPER ) != wxNOT_FOUND )
        text->Replace( guLYRICS_TITLE_UPPER, DoReplace( Track->m_SongName.Upper(), lyricsource ) );
    if( text->Find( guLYRICS_TITLE_FIRST ) != wxNOT_FOUND )
        text->Replace( guLYRICS_TITLE_FIRST, DoReplace( Track->m_SongName.Trim( false )[ 0 ], lyricsource ) );
    if( text->Find( guLYRICS_TITLE_LOWER_FIRST ) != wxNOT_FOUND )
        text->Replace( guLYRICS_TITLE_LOWER_FIRST, DoReplace( Track->m_SongName.Lower().Trim( false )[ 0 ], lyricsource ) );
    if( text->Find( guLYRICS_TITLE_UPPER_FIRST ) != wxNOT_FOUND )
        text->Replace( guLYRICS_TITLE_UPPER_FIRST, DoReplace( Track->m_SongName.Upper().Trim( false )[ 0 ], lyricsource ) );
    if( text->Find( guLYRICS_TITLE_CAPITALIZE ) )
        text->Replace( guLYRICS_TITLE_CAPITALIZE, DoReplace( SpecialCase( Track->m_SongName.Lower() ), lyricsource ) );

    if( text->Find( guLYRICS_FILENAME ) != wxNOT_FOUND )
        text->Replace( guLYRICS_FILENAME, Track->m_FileName.BeforeLast( wxT( '.' ) ) );
}

// -------------------------------------------------------------------------------- //
wxString guLyricSearchThread::GetSource( guLyricSource &lyricsource )
{
    wxString RetVal = lyricsource.Source();
    ProcessTags( &RetVal, lyricsource );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guLyricSearchThread::LyricDownload( guLyricSource &lyricsource )
{
    wxString Url = GetSource( lyricsource );
    guLogMessage( wxT( "Trying to get url: %s" ), Url.c_str() );

    m_LyricText = GetUrlContent( Url, wxEmptyString, true );
    //guLogMessage( wxT( "Content: '%s'" ), m_LyricText.c_str() );
}

// -------------------------------------------------------------------------------- //
void guLyricSearchThread::LyricFile( guLyricSource &lyricsource )
{
    wxString FilePath = GetSource( lyricsource );
    wxFileName FileName( FilePath );
    if( FileName.Normalize( wxPATH_NORM_ALL|wxPATH_NORM_CASE ) )
    {
        if( wxFileExists( FileName.GetFullPath() ) )
        {
            wxFileInputStream Ins( FileName.GetFullPath() );
            if( Ins.IsOk() )
            {
                wxStringOutputStream Outs( &m_LyricText );
                Ins.Read( Outs );
            }
        }
//        else
//        {
//            guLogMessage( wxT( "File not found: '%s'" ), FileName.GetFullPath().c_str() );
//        }
    }
}

// -------------------------------------------------------------------------------- //
void guLyricSearchThread::LyricCommand( guLyricSource &lyricsource )
{
    wxString * CommandText = new wxString( GetSource( lyricsource ) );
    guLogMessage( wxT( "Trying to execute command: %s" ), CommandText->c_str() );

    wxCommandEvent CommandEvent( wxEVT_MENU, ID_MAINFRAME_LYRICSEXECCOMMAND );
    CommandEvent.SetClientData( CommandText );
    CommandEvent.SetClientObject( ( wxClientData * ) this );
    CommandEvent.SetInt( false );
    wxPostEvent( guMainFrame::GetMainFrame(), CommandEvent );

    m_CommandIsExecuting = true;

    long EndLocalTime = wxGetLocalTime() + 90;

    while( !TestDestroy() && m_CommandIsExecuting )
    {
        Sleep( 20 );

        if( wxGetLocalTime() > EndLocalTime )
            break;
    }
    m_CommandIsExecuting = false;
}

// -------------------------------------------------------------------------------- //
void guLyricSearchThread::FinishExecCommand( const wxString &lyrictext )
{
    if( m_CommandIsExecuting )
    {
        //guLogMessage( wxT( "Finish Executing the Command..." ) );
        m_LyricText = lyrictext;
        m_CommandIsExecuting = false;
    }
}

// -------------------------------------------------------------------------------- //
void guLyricSearchThread::ProcessSave( guLyricSource &lyricsource )
{
    //guLogMessage( wxT( "guLyricSearchThread::ProcessSave" ) );
    if( !TestDestroy() && ( m_LyricSearchContext->DoSaveProcess() || m_ForceSaveProcess ) )
    {
        guLyricSearchEngine * LyricSearchEngine = m_LyricSearchContext->m_LyricSearchEngine;
        int Index;
        int Count = LyricSearchEngine->TargetsCount();
        for( Index = 0; Index < Count; Index++ )
        {
            guLyricSource * LyricTarget = LyricSearchEngine->GetTarget( Index );
            if( LyricTarget->Enabled() )
            {
                if( LyricTarget->Type() == guLYRIC_SOURCE_TYPE_EMBEDDED )
                {
                    if( !m_LyricSearchContext->m_Track.m_Offset &&  // If its not from a cue file
                        ( lyricsource.Type() != guLYRIC_SOURCE_TYPE_EMBEDDED ) &&
                        wxFileExists( m_LyricSearchContext->m_Track.m_FileName ) )
                    {
                        if( !guTagSetLyrics( m_LyricSearchContext->m_Track.m_FileName, m_LyricText ) )
                        {
                            guLogMessage( wxT( "Could not save the lyrics to the file '%s'" ), m_LyricSearchContext->m_Track.m_FileName.c_str() );
                        }
                    }
                }
                else if( LyricTarget->Type() == guLYRIC_SOURCE_TYPE_FILE )
                {
                    wxString TargetFileName = GetSource( * LyricTarget );
                    //guLogMessage( wxT( "Lyrics Save to File %i : '%s'" ), Index, TargetFileName.c_str() );
                    if( lyricsource.Type() == guLYRIC_SOURCE_TYPE_FILE )
                    {
                        wxString SourceFileName = GetSource( lyricsource );
                        if( TargetFileName == SourceFileName )
                        {
                            continue;
                        }
                    }
                    else if( lyricsource.Type() == guLYRIC_SOURCE_TYPE_EMBEDDED )
                    {
                        if( wxFileExists( TargetFileName ) )
                        {
                            continue;
                        }
                    }

                    wxFileName FileName( TargetFileName );
                    if( FileName.Normalize( wxPATH_NORM_ALL|wxPATH_NORM_CASE ) )
                    {
                        FileName.Mkdir( 0770, wxPATH_MKDIR_FULL );

                        if( !m_LyricText.IsEmpty() )
                        {
                            wxFile LyricFile( FileName.GetFullPath(), wxFile::write );
                            if( LyricFile.IsOpened() )
                            {
                                if( !LyricFile.Write( m_LyricText ) )
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
                        else
                        {
                            wxRemoveFile( FileName.GetFullPath() );
                        }
                    }
                }
                else if( LyricTarget->Type() == guLYRIC_SOURCE_TYPE_COMMAND )
                {
                    wxString * CommandText = new wxString( GetSource( * LyricTarget ) );
                    guLogMessage( wxT( "Trying to execute save command: %s" ), CommandText->c_str() );

                    wxCommandEvent CommandEvent( wxEVT_MENU, ID_MAINFRAME_LYRICSEXECCOMMAND );
                    CommandEvent.SetClientObject( ( wxClientData * ) this );
                    CommandEvent.SetClientData( CommandText );
                    CommandEvent.SetInt( true );
                    wxPostEvent( guMainFrame::GetMainFrame(), CommandEvent );
                }
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
guLyricSearchThread::ExitCode guLyricSearchThread::Entry()
{
    if( TestDestroy() )
        return 0;

    guLyricSource LyricSource;

    // When we are called to only save the lyrics as they have been changed
    if( !m_LyricText.IsEmpty() )
    {
        ProcessSave( LyricSource );
        return 0;
    }

    while( m_LyricSearchContext->GetNextSource( &LyricSource, false ) )
    {
        if( LyricSource.Type() == guLYRIC_SOURCE_TYPE_DOWNLOAD )
        {
            LyricDownload( LyricSource );
        }
        else if( LyricSource.Type() == guLYRIC_SOURCE_TYPE_EMBEDDED )
        {
            if( wxFileExists( m_LyricSearchContext->m_Track.m_FileName ) )
            {
                m_LyricText = guTagGetLyrics( m_LyricSearchContext->m_Track.m_FileName );
                if( !m_LyricText.IsEmpty() )
                    break;
            }
        }
        else if( LyricSource.Type() == guLYRIC_SOURCE_TYPE_FILE )
        {
            LyricFile( LyricSource );
        }
        else if( LyricSource.Type() == guLYRIC_SOURCE_TYPE_COMMAND )
        {
            LyricCommand( LyricSource );
        }

        if( !TestDestroy() && !m_LyricText.IsEmpty() )
        {
            if( !CheckNotFound( LyricSource ) )
            {
                m_LyricText = CheckExtract( m_LyricText, LyricSource );
                //guLogMessage( wxT( "Content:\n%s" ), m_LyricText.c_str() );
                if( !m_LyricText.IsEmpty() )
                {
                    wxHtmlEntitiesParser EntitiesParser;
                    m_LyricText = EntitiesParser.Parse( CheckExclude( m_LyricText, LyricSource ) );

                    wxRegEx RegExNewLine( wxT( " ?\r? ?\n? ?< ?br ?/? ?> ?\r? ?\n? ?" ), wxRE_EXTENDED );
                    RegExNewLine.ReplaceAll( &m_LyricText, wxT( "\n" ) );

                    wxRegEx RegExHtml( wxT( " ?</?[^>]*> ?" ), wxRE_EXTENDED );
                    RegExHtml.ReplaceAll( &m_LyricText, wxT( "" ) );

                    if( !CheckNotFound( LyricSource ) )
                    {
                        m_LyricText = m_LyricText.Trim( true ).Trim( false );
                        //guLogMessage( wxT( "LyricText:\n%s" ), m_LyricText.c_str() );
                        guLogMessage( wxT( "Found the lyrics from source: %s" ), LyricSource.Name().c_str() );

                        // Do the save of the lyrics
                        ProcessSave( LyricSource );
                    }
                    else
                    {
                        m_LyricText = wxEmptyString;
                    }
                }
            }
            else
            {
                m_LyricText = wxEmptyString;
            }
        }

        if( TestDestroy() || !m_LyricText.IsEmpty() )
            break;
    }

    return 0;
}

// -------------------------------------------------------------------------------- //
//
// -------------------------------------------------------------------------------- //
guLyricExecCommandTerminate::guLyricExecCommandTerminate( guLyricSearchThread * searchthread, const bool issavecommand ) :
    wxProcess( wxPROCESS_REDIRECT )
{
    guLogMessage( wxT( "guLyricExecCommandTerminate::guLyricExecCommandTerminate" ) );
    m_LyricSearchThread = searchthread;
    m_IsSaveCommand = issavecommand;
    m_ThreadActive = true;
}

// -------------------------------------------------------------------------------- //
void guLyricExecCommandTerminate::OnTerminate( int pid, int status )
{
    guLogMessage( wxT( "Command with pid '%d' finished with status '%d'" ), pid, status );

    wxString CommandLyric;
    if( !m_IsSaveCommand && IsInputAvailable() )
    {
        wxInputStream * CmdOutStream = GetInputStream();
        if( CmdOutStream )
        {
            guLogDebug( "guLyricExecCommandTerminate::OnTerminate<%i> while not eof start", pid );
            wxTextInputStream TextStream( * CmdOutStream );
            while( !CmdOutStream->Eof() )
            {
                wxString Line = TextStream.ReadLine() + wxT( "\n" );
                guLogDebug( "guLyricExecCommandTerminate::OnTerminate<%i> line: %s", pid, Line );
                CommandLyric += Line;
            }
        }
        else
        {
            guLogMessage( wxT( "Error getting the exec command input stream" ) );
        }
    }
    else
        guLogDebug( "guLyricExecCommandTerminate::OnTerminate<%i> skip it", pid );

    guLogDebug( "guLyricExecCommandTerminate::OnTerminate<%i> thread active = %i", pid, m_ThreadActive );
    if( m_ThreadActive )
        m_LyricSearchThread->FinishExecCommand( CommandLyric );

    delete this;
}

// -------------------------------------------------------------------------------- //
// guLyricSourceEditor
// -------------------------------------------------------------------------------- //
guLyricSourceEditor::guLyricSourceEditor( wxWindow * parent, guLyricSource * lyricsource, const bool istarget ) :
    //wxDialog( parent, wxID_ANY, _( "Lyric Source Editor" ), wxDefaultPosition, wxSize( 500, 425 ), wxDEFAULT_DIALOG_STYLE )
    wxDialog( parent, wxID_ANY, istarget ? _( "Lyrics Target Editor" ) : _( "Lyrics Source Editor" ), wxDefaultPosition, wxSize( 500, 450 - ( istarget * 235 ) ), wxDEFAULT_DIALOG_STYLE )
{
    m_LyricSource = lyricsource;
    m_IsTarget = istarget;
    m_ReplaceItems = lyricsource->ReplaceItems();
    m_ExtractItems = lyricsource->ExtractItems();
    m_ExcludeItems = lyricsource->ExcludeItems();
    m_NotFoundItems = lyricsource->NotFoundItems();

    SetSizeHints( wxDefaultSize, wxDefaultSize );

    wxBoxSizer * MainSizer = new wxBoxSizer( wxVERTICAL );

    MainSizer->Add( 0, 10, 0, wxEXPAND, 5 );

    wxFlexGridSizer * OptionsSizer = new wxFlexGridSizer( 2, 0, 0 );
    OptionsSizer->AddGrowableCol( 1 );
//    OptionsSizer->AddGrowableRow( 2 );
//    OptionsSizer->AddGrowableRow( 3 );
//    OptionsSizer->AddGrowableRow( 4 );
//    OptionsSizer->AddGrowableRow( 5 );
    OptionsSizer->SetFlexibleDirection( wxBOTH );
    OptionsSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    wxStaticText * NameLabel = new wxStaticText( this, wxID_ANY, _( "Name:" ), wxDefaultPosition, wxDefaultSize, 0 );
    NameLabel->Wrap( -1 );
    OptionsSizer->Add( NameLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );

    wxBoxSizer * NameSizer = new wxBoxSizer( wxHORIZONTAL );

    m_NameTextCtrl = new wxTextCtrl( this, wxID_ANY, lyricsource->Name(), wxDefaultPosition, wxDefaultSize, 0 );
    NameSizer->Add( m_NameTextCtrl, 1, wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 5 );

    wxStaticText * TypeLabel = new wxStaticText( this, wxID_ANY, _( "Type:" ), wxDefaultPosition, wxDefaultSize, 0 );
    TypeLabel->Wrap( -1 );
    NameSizer->Add( TypeLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

    wxArrayString LyricSourceTypes;
    LyricSourceTypes.Add( _( "Embedded" ) );
    LyricSourceTypes.Add( _( "File" ) );
    LyricSourceTypes.Add( _( "Command" ) );
    if( !istarget )
        LyricSourceTypes.Add( _( "Download" ) );
    m_TypeChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, LyricSourceTypes, 0 );
    m_TypeChoice->SetSelection( lyricsource->Type() );
    NameSizer->Add( m_TypeChoice, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );

    OptionsSizer->Add( NameSizer, 1, wxEXPAND, 5 );

    wxStaticText * SourceLabel = new wxStaticText( this, wxID_ANY, _( "Source:" ), wxDefaultPosition, wxDefaultSize, 0 );
    SourceLabel->Wrap( -1 );
    OptionsSizer->Add( SourceLabel, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );

    m_SourceTextCtrl = new wxTextCtrl( this, wxID_ANY, lyricsource->Source(), wxDefaultPosition, wxDefaultSize, 0 );
    m_SourceTextCtrl->Enable( lyricsource->Type() != guLYRIC_SOURCE_TYPE_EMBEDDED );
    OptionsSizer->Add( m_SourceTextCtrl, 1, wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 5 );

    wxStaticText * ReplaceLabel = new wxStaticText( this, wxID_ANY, _( "Replace:" ), wxDefaultPosition, wxDefaultSize, 0 );
    ReplaceLabel->Wrap( -1 );
    OptionsSizer->Add( ReplaceLabel, 0, wxALL|wxALIGN_RIGHT, 5 );

    wxBoxSizer * ReplaceListBoxSizer = new wxBoxSizer( wxHORIZONTAL );

    wxArrayString ListBoxOptions;
    int Index;
    int Count = m_ReplaceItems.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guLyricSourceReplace * ReplaceItem = &m_ReplaceItems[ Index ];
        ListBoxOptions.Add( ReplaceItem->ToStr() );
    }
    m_ReplaceListBox = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxSize( -1, 70 ), 0, NULL, wxLB_SINGLE );
    m_ReplaceListBox->Append( ListBoxOptions );
    m_ReplaceListBox->Enable( lyricsource->Type() != guLYRIC_SOURCE_TYPE_EMBEDDED );
    ReplaceListBoxSizer->Add( m_ReplaceListBox, 1, wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 5 );

    wxBoxSizer * ReplaceBtnSizer = new wxBoxSizer( wxVERTICAL );

    m_ReplaceAdd = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_ReplaceAdd->Enable( lyricsource->Type() != guLYRIC_SOURCE_TYPE_EMBEDDED );
    ReplaceBtnSizer->Add( m_ReplaceAdd, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );

    m_ReplaceDel = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_del ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_ReplaceDel->Enable( false );
    ReplaceBtnSizer->Add( m_ReplaceDel, 0, wxBOTTOM|wxRIGHT, 5 );

    ReplaceListBoxSizer->Add( ReplaceBtnSizer, 0, wxEXPAND, 5 );

    OptionsSizer->Add( ReplaceListBoxSizer, 1, wxEXPAND, 5 );

    if( !istarget )
    {
        wxStaticText * ExtractLabel = new wxStaticText( this, wxID_ANY, _( "Extract:" ), wxDefaultPosition, wxDefaultSize, 0 );
        ExtractLabel->Wrap( -1 );
        OptionsSizer->Add( ExtractLabel, 0, wxALL|wxALIGN_RIGHT, 5 );

        wxBoxSizer * ExtractListBoxSizer = new wxBoxSizer( wxHORIZONTAL );

        ListBoxOptions.Empty();
        Count = m_ExtractItems.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            guLyricSourceExtract * ExtractItem = &m_ExtractItems[ Index ];
            ListBoxOptions.Add( ExtractItem->ToStr() );
        }
        m_ExtractListBox = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxSize( -1, 70 ), 0, NULL, wxLB_SINGLE );
        m_ExtractListBox->Append( ListBoxOptions );
        ExtractListBoxSizer->Add( m_ExtractListBox, 1, wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 5 );

        wxBoxSizer * ExtractBtnSizer = new wxBoxSizer( wxVERTICAL );

        m_ExtractAdd = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
        ExtractBtnSizer->Add( m_ExtractAdd, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );

        m_ExtractDel = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_del ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
        m_ExtractDel->Enable( false );
        ExtractBtnSizer->Add( m_ExtractDel, 0, wxBOTTOM|wxRIGHT, 5 );

        ExtractListBoxSizer->Add( ExtractBtnSizer, 0, wxEXPAND, 5 );

        OptionsSizer->Add( ExtractListBoxSizer, 1, wxEXPAND, 5 );

        wxStaticText * ExcludeLabel = new wxStaticText( this, wxID_ANY, _( "Exclude:" ), wxDefaultPosition, wxDefaultSize, 0 );
        ExcludeLabel->Wrap( -1 );
        OptionsSizer->Add( ExcludeLabel, 0, wxALL|wxALIGN_RIGHT, 5 );

        wxBoxSizer * ExcludeListBoxSizer = new wxBoxSizer( wxHORIZONTAL );

        ListBoxOptions.Empty();
        Count = m_ExcludeItems.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            guLyricSourceExclude * ExcludeItem = &m_ExcludeItems[ Index ];
            ListBoxOptions.Add( ExcludeItem->ToStr() );
        }
        m_ExcludeListBox = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxSize( -1, 70 ), 0, NULL, wxLB_SINGLE );
        m_ExcludeListBox->Append( ListBoxOptions );
        ExcludeListBoxSizer->Add( m_ExcludeListBox, 1, wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 5 );

        wxBoxSizer * ExcludeBtnSizer = new wxBoxSizer( wxVERTICAL );

        m_ExcludeAdd = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
        ExcludeBtnSizer->Add( m_ExcludeAdd, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );

        m_ExcludeDel = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_del ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
        m_ExcludeDel->Enable( false );
        ExcludeBtnSizer->Add( m_ExcludeDel, 0, wxBOTTOM|wxRIGHT, 5 );

        ExcludeListBoxSizer->Add( ExcludeBtnSizer, 0, wxEXPAND, 5 );

        OptionsSizer->Add( ExcludeListBoxSizer, 1, wxEXPAND, 5 );

        wxStaticText * NotFoundLabel = new wxStaticText( this, wxID_ANY, _( "Not Found:" ), wxDefaultPosition, wxDefaultSize, 0 );
        NotFoundLabel->Wrap( -1 );
        OptionsSizer->Add( NotFoundLabel, 0, wxALL|wxALIGN_RIGHT, 5 );

        wxBoxSizer * NotFoundListBoxSizer = new wxBoxSizer( wxHORIZONTAL );

        m_NotFoundListBox = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxSize( -1, 70 ), 0, NULL, wxLB_SINGLE );
        m_NotFoundListBox->Append( m_NotFoundItems );
        NotFoundListBoxSizer->Add( m_NotFoundListBox, 1, wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 5 );

        wxBoxSizer* NotFoundBtnSizer;
        NotFoundBtnSizer = new wxBoxSizer( wxVERTICAL );

        m_NotFoundAdd = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_add ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
        NotFoundBtnSizer->Add( m_NotFoundAdd, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );

        m_NotFoundDel = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_tiny_del ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
        m_NotFoundDel->Enable( false );
        NotFoundBtnSizer->Add( m_NotFoundDel, 0, wxBOTTOM|wxRIGHT, 5 );

        NotFoundListBoxSizer->Add( NotFoundBtnSizer, 0, wxEXPAND, 5 );

        OptionsSizer->Add( NotFoundListBoxSizer, 1, wxEXPAND, 5 );
    }

    MainSizer->Add( OptionsSizer, 1, wxEXPAND, 5 );

    wxStdDialogButtonSizer * ButtonsSizer = new wxStdDialogButtonSizer();
    wxButton * ButtonsSizerOK = new wxButton( this, wxID_OK );
    ButtonsSizer->AddButton( ButtonsSizerOK );
    wxButton * ButtonsSizerCancel = new wxButton( this, wxID_CANCEL );
    ButtonsSizer->AddButton( ButtonsSizerCancel );
    ButtonsSizer->SetAffirmativeButton( ButtonsSizerOK );
    ButtonsSizer->SetCancelButton( ButtonsSizerCancel );
    ButtonsSizer->Realize();
    MainSizer->Add( ButtonsSizer, 0, wxEXPAND|wxALL, 5 );

    SetSizer( MainSizer );
    Layout();

    ButtonsSizerOK->SetDefault();

    // Bind Events
    m_TypeChoice->Bind( wxEVT_CHOICE, &guLyricSourceEditor::OnTypeChanged, this );
    m_ReplaceListBox->Bind( wxEVT_LISTBOX, &guLyricSourceEditor::OnReplaceSelected, this );
    m_ReplaceListBox->Bind( wxEVT_LISTBOX_DCLICK, &guLyricSourceEditor::OnReplaceDClicked, this );
    m_ReplaceAdd->Bind( wxEVT_BUTTON, &guLyricSourceEditor::OnReplaceAddClicked, this );
    m_ReplaceDel->Bind( wxEVT_BUTTON, &guLyricSourceEditor::OnReplaceDelClicked, this );
    if( !istarget )
    {
        m_ExtractListBox->Bind( wxEVT_LISTBOX, &guLyricSourceEditor::OnExtractSelected, this );
        m_ExtractListBox->Bind( wxEVT_LISTBOX_DCLICK, &guLyricSourceEditor::OnExtractDClicked, this );
        m_ExtractAdd->Bind( wxEVT_BUTTON, &guLyricSourceEditor::OnExtractAddClicked, this );
        m_ExtractDel->Bind( wxEVT_BUTTON, &guLyricSourceEditor::OnExtractDelClicked, this );
        m_ExcludeListBox->Bind( wxEVT_LISTBOX, &guLyricSourceEditor::OnExcludeSelected, this );
        m_ExcludeListBox->Bind( wxEVT_LISTBOX_DCLICK, &guLyricSourceEditor::OnExcludeDClicked, this );
        m_ExcludeAdd->Bind( wxEVT_BUTTON, &guLyricSourceEditor::OnExcludeAddClicked, this );
        m_ExcludeDel->Bind( wxEVT_BUTTON, &guLyricSourceEditor::OnExcludeDelClicked, this );
        m_NotFoundListBox->Bind( wxEVT_LISTBOX, &guLyricSourceEditor::OnNotFoundSelected, this );
        m_NotFoundListBox->Bind( wxEVT_LISTBOX_DCLICK, &guLyricSourceEditor::OnNotFoundDClicked, this );
        m_NotFoundAdd->Bind( wxEVT_BUTTON, &guLyricSourceEditor::OnNotFoundAddClicked, this );
        m_NotFoundDel->Bind( wxEVT_BUTTON, &guLyricSourceEditor::OnNotFoundDelClicked, this );
    }

    m_NameTextCtrl->SetFocus();
}

// -------------------------------------------------------------------------------- //
guLyricSourceEditor::~guLyricSourceEditor()
{
    m_TypeChoice->Unbind( wxEVT_CHOICE, &guLyricSourceEditor::OnTypeChanged, this );
    m_ReplaceListBox->Unbind( wxEVT_LISTBOX, &guLyricSourceEditor::OnReplaceSelected, this );
    m_ReplaceListBox->Unbind( wxEVT_LISTBOX_DCLICK, &guLyricSourceEditor::OnReplaceDClicked, this );
    m_ReplaceAdd->Unbind( wxEVT_BUTTON, &guLyricSourceEditor::OnReplaceAddClicked, this );
    m_ReplaceDel->Unbind( wxEVT_BUTTON, &guLyricSourceEditor::OnReplaceDelClicked, this );
    if( !m_IsTarget )
    {
        m_ExtractListBox->Unbind( wxEVT_LISTBOX, &guLyricSourceEditor::OnExtractSelected, this );
        m_ExtractListBox->Unbind( wxEVT_LISTBOX_DCLICK, &guLyricSourceEditor::OnExtractDClicked, this );
        m_ExtractAdd->Unbind( wxEVT_BUTTON, &guLyricSourceEditor::OnExtractAddClicked, this );
        m_ExtractDel->Unbind( wxEVT_BUTTON, &guLyricSourceEditor::OnExtractDelClicked, this );
        m_ExcludeListBox->Unbind( wxEVT_LISTBOX, &guLyricSourceEditor::OnExcludeSelected, this );
        m_ExcludeListBox->Unbind( wxEVT_LISTBOX_DCLICK, &guLyricSourceEditor::OnExcludeDClicked, this );
        m_ExcludeAdd->Unbind( wxEVT_BUTTON, &guLyricSourceEditor::OnExcludeAddClicked, this );
        m_ExcludeDel->Unbind( wxEVT_BUTTON, &guLyricSourceEditor::OnExcludeDelClicked, this );
        m_NotFoundListBox->Unbind( wxEVT_LISTBOX, &guLyricSourceEditor::OnNotFoundSelected, this );
        m_NotFoundListBox->Unbind( wxEVT_LISTBOX_DCLICK, &guLyricSourceEditor::OnNotFoundDClicked, this );
        m_NotFoundAdd->Unbind( wxEVT_BUTTON, &guLyricSourceEditor::OnNotFoundAddClicked, this );
        m_NotFoundDel->Unbind( wxEVT_BUTTON, &guLyricSourceEditor::OnNotFoundDelClicked, this );
    }
}

// -------------------------------------------------------------------------------- //
void guLyricSourceEditor::OnTypeChanged( wxCommandEvent &event )
{
    bool Enabled = ( m_TypeChoice->GetSelection() != guLYRIC_SOURCE_TYPE_EMBEDDED );
    m_SourceTextCtrl->Enable( Enabled );
    m_ReplaceListBox->Enable( Enabled );
    m_ReplaceAdd->Enable( Enabled );
}

// -------------------------------------------------------------------------------- //
void guLyricSourceEditor::OnReplaceSelected( wxCommandEvent &event )
{
    m_ReplaceDel->Enable( event.GetInt() != wxNOT_FOUND );
}

// -------------------------------------------------------------------------------- //
void guLyricSourceEditor::OnReplaceAddClicked( wxCommandEvent &event )
{
    guLyricSourceReplace LyricSourceReplace;
    guLyricSourceOptionEditor * SourceOptionEditor = new guLyricSourceOptionEditor( this, &LyricSourceReplace, guLYRIC_SOURCE_OPTION_TYPE_REPLACE );
    if( SourceOptionEditor )
    {
        if( SourceOptionEditor->ShowModal() == wxID_OK )
        {
            SourceOptionEditor->UpdateSourceOption();
            m_ReplaceItems.Add( LyricSourceReplace );
            m_ReplaceListBox->Append( LyricSourceReplace.ToStr() );
        }
        SourceOptionEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guLyricSourceEditor::OnReplaceDelClicked( wxCommandEvent &event )
{
    int Selected = m_ReplaceListBox->GetSelection();
    m_ReplaceListBox->Delete( Selected );
    m_ReplaceItems.RemoveAt( Selected );
}

// -------------------------------------------------------------------------------- //
void guLyricSourceEditor::OnReplaceDClicked( wxCommandEvent &event )
{
    int Selected = m_ReplaceListBox->GetSelection();
    if( Selected != wxNOT_FOUND )
    {
        guLyricSourceReplace * LyricSourceReplace = &m_ReplaceItems[ Selected ];
        guLyricSourceOptionEditor * SourceOptionEditor = new guLyricSourceOptionEditor( this, LyricSourceReplace, guLYRIC_SOURCE_OPTION_TYPE_REPLACE );
        if( SourceOptionEditor )
        {
            if( SourceOptionEditor->ShowModal() == wxID_OK )
            {
                SourceOptionEditor->UpdateSourceOption();
                m_ReplaceListBox->SetString( Selected, LyricSourceReplace->ToStr() );
            }
            SourceOptionEditor->Destroy();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guLyricSourceEditor::OnExtractSelected( wxCommandEvent &event )
{
    m_ExtractDel->Enable( event.GetInt() != wxNOT_FOUND );
}

// -------------------------------------------------------------------------------- //
void guLyricSourceEditor::OnExtractAddClicked( wxCommandEvent &event )
{
    guLyricSourceExtract LyricSourceExtract;
    guLyricSourceOptionEditor * SourceOptionEditor = new guLyricSourceOptionEditor( this, &LyricSourceExtract, guLYRIC_SOURCE_OPTION_TYPE_EXTRACT );
    if( SourceOptionEditor )
    {
        if( SourceOptionEditor->ShowModal() == wxID_OK )
        {
            SourceOptionEditor->UpdateSourceOption();
            m_ExtractItems.Add( LyricSourceExtract );
            m_ExtractListBox->Append( LyricSourceExtract.ToStr() );
        }
        SourceOptionEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guLyricSourceEditor::OnExtractDelClicked( wxCommandEvent &event )
{
    int Selected = m_ExtractListBox->GetSelection();
    m_ExtractListBox->Delete( Selected );
    m_ExtractItems.RemoveAt( Selected );
}

// -------------------------------------------------------------------------------- //
void guLyricSourceEditor::OnExtractDClicked( wxCommandEvent &event )
{
    int Selected = m_ExtractListBox->GetSelection();
    if( Selected != wxNOT_FOUND )
    {
        guLyricSourceExtract * LyricSourceExtract = &m_ExtractItems[ Selected ];
        guLyricSourceOptionEditor * SourceOptionEditor = new guLyricSourceOptionEditor( this, LyricSourceExtract, guLYRIC_SOURCE_OPTION_TYPE_EXTRACT );
        if( SourceOptionEditor )
        {
            if( SourceOptionEditor->ShowModal() == wxID_OK )
            {
                SourceOptionEditor->UpdateSourceOption();
                m_ExtractListBox->SetString( Selected, LyricSourceExtract->ToStr() );
            }
            SourceOptionEditor->Destroy();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guLyricSourceEditor::OnExcludeSelected( wxCommandEvent &event )
{
    m_ExcludeDel->Enable( event.GetInt() != wxNOT_FOUND );
}

// -------------------------------------------------------------------------------- //
void guLyricSourceEditor::OnExcludeAddClicked( wxCommandEvent &event )
{
    guLyricSourceExclude LyricSourceExclude;
    guLyricSourceOptionEditor * SourceOptionEditor = new guLyricSourceOptionEditor( this, &LyricSourceExclude, guLYRIC_SOURCE_OPTION_TYPE_EXCLUDE );
    if( SourceOptionEditor )
    {
        if( SourceOptionEditor->ShowModal() == wxID_OK )
        {
            SourceOptionEditor->UpdateSourceOption();
            m_ExcludeItems.Add( LyricSourceExclude );
            m_ExcludeListBox->Append( LyricSourceExclude.ToStr() );
        }
        SourceOptionEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guLyricSourceEditor::OnExcludeDelClicked( wxCommandEvent &event )
{
    int Selected = m_ExcludeListBox->GetSelection();
    m_ExcludeListBox->Delete( Selected );
    m_ExcludeItems.RemoveAt( Selected );
}

// -------------------------------------------------------------------------------- //
void guLyricSourceEditor::OnExcludeDClicked( wxCommandEvent &event )
{
    int Selected = m_ExcludeListBox->GetSelection();
    if( Selected != wxNOT_FOUND )
    {
        guLyricSourceExclude * LyricSourceExclude = &m_ExcludeItems[ Selected ];
        guLyricSourceOptionEditor * SourceOptionEditor = new guLyricSourceOptionEditor( this, LyricSourceExclude, guLYRIC_SOURCE_OPTION_TYPE_EXCLUDE );
        if( SourceOptionEditor )
        {
            if( SourceOptionEditor->ShowModal() == wxID_OK )
            {
                SourceOptionEditor->UpdateSourceOption();
                m_ExcludeListBox->SetString( Selected, LyricSourceExclude->ToStr() );
            }
            SourceOptionEditor->Destroy();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guLyricSourceEditor::OnNotFoundSelected( wxCommandEvent &event )
{
    m_NotFoundDel->Enable( event.GetInt() != wxNOT_FOUND );
}

// -------------------------------------------------------------------------------- //
void guLyricSourceEditor::OnNotFoundAddClicked( wxCommandEvent &event )
{
    guLyricSourceOption LyricSourceOption;
    guLyricSourceOptionEditor * SourceOptionEditor = new guLyricSourceOptionEditor( this, &LyricSourceOption, guLYRIC_SOURCE_OPTION_TYPE_NOTFOUND );
    if( SourceOptionEditor )
    {
        if( SourceOptionEditor->ShowModal() == wxID_OK )
        {
            SourceOptionEditor->UpdateSourceOption();
            m_NotFoundItems.Add( LyricSourceOption.Text1() );
            m_NotFoundListBox->Append( LyricSourceOption.Text1() );
        }
        SourceOptionEditor->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guLyricSourceEditor::OnNotFoundDelClicked( wxCommandEvent &event )
{
    int Selected = m_NotFoundListBox->GetSelection();
    m_NotFoundListBox->Delete( Selected );
    m_NotFoundItems.RemoveAt( Selected );
}

// -------------------------------------------------------------------------------- //
void guLyricSourceEditor::OnNotFoundDClicked( wxCommandEvent &event )
{
    int Selected = m_NotFoundListBox->GetSelection();
    if( Selected != wxNOT_FOUND )
    {
        guLyricSourceOption * LyricSourceNotFound = new guLyricSourceOption( m_NotFoundItems[ Selected ] );
        guLyricSourceOptionEditor * SourceOptionEditor = new guLyricSourceOptionEditor( this, LyricSourceNotFound, guLYRIC_SOURCE_OPTION_TYPE_NOTFOUND );
        if( SourceOptionEditor )
        {
            if( SourceOptionEditor->ShowModal() == wxID_OK )
            {
                SourceOptionEditor->UpdateSourceOption();
                m_NotFoundItems[ Selected ] = LyricSourceNotFound->Text1();
                m_NotFoundListBox->SetString( Selected, LyricSourceNotFound->Text1() );
                delete LyricSourceNotFound;
            }
            SourceOptionEditor->Destroy();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guLyricSourceEditor::UpdateLyricSource( void )
{
    m_LyricSource->Name( m_NameTextCtrl->GetValue() );
    m_LyricSource->Type( m_TypeChoice->GetSelection() );
    m_LyricSource->Source( m_SourceTextCtrl->GetValue() );
    m_LyricSource->ReplaceItems( m_ReplaceItems );
    m_LyricSource->ExtractItems( m_ExtractItems );
    m_LyricSource->ExcludeItems( m_ExcludeItems );
    m_LyricSource->NotFoundItems( m_NotFoundItems );
}

// -------------------------------------------------------------------------------- //
guLyricSourceOptionEditor::guLyricSourceOptionEditor( wxWindow * parent, guLyricSourceOption * sourceoption, const int optiontype )
{
    m_SourceOption = sourceoption;

    wxString Title;
    wxString Label1;
    wxString Label2;
    if( optiontype == guLYRIC_SOURCE_OPTION_TYPE_REPLACE )
    {
        Title = _( "Replace option editor" );
        Label1 = _( "Search:" );
        Label2 = _( "Replace:" );
    }
    else if( optiontype == guLYRIC_SOURCE_OPTION_TYPE_EXTRACT )
    {
        Title = _( "Extract option editor" );
        Label1 = _( "Begin:" );
        Label2 = _( "End:" );
    }
    else if( optiontype == guLYRIC_SOURCE_OPTION_TYPE_EXCLUDE )
    {
        Title = _( "Exclude option editor" );
        Label1 = _( "Begin:" );
        Label2 = _( "End:" );
    }
    else if( optiontype == guLYRIC_SOURCE_OPTION_TYPE_NOTFOUND )
    {
        Title = _( "Not Found option editor" );
        Label1 = _( "Tag:" );
        Label2 = _( "Tag:" );
    }

    Create( parent, wxID_ANY, Title, wxDefaultPosition, wxSize( 400, 150 ), wxDEFAULT_DIALOG_STYLE );

    SetSizeHints( wxDefaultSize, wxDefaultSize );

    wxBoxSizer * MainSizer = new wxBoxSizer( wxVERTICAL );

    //MainSizer->Add( 0, 10, 1, wxEXPAND, 5 );

    wxFlexGridSizer * OptionsSizer;
    OptionsSizer = new wxFlexGridSizer( 2, 0, 0 );
    OptionsSizer->AddGrowableCol( 1 );
    OptionsSizer->SetFlexibleDirection( wxBOTH );
    OptionsSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_SearchLabel = new wxStaticText( this, wxID_ANY, Label1, wxDefaultPosition, wxDefaultSize, 0 );
    m_SearchLabel->Wrap( -1 );
    OptionsSizer->Add( m_SearchLabel, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );

    m_SearchTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_SearchTextCtrl->SetValue( sourceoption->Text1() );
    OptionsSizer->Add( m_SearchTextCtrl, 1, wxTOP|wxBOTTOM|wxRIGHT|wxEXPAND, 5 );

    m_ReplaceLabel = new wxStaticText( this, wxID_ANY, Label2, wxDefaultPosition, wxDefaultSize, 0 );
    m_ReplaceLabel->Wrap( -1 );
    OptionsSizer->Add( m_ReplaceLabel, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );

    m_ReplaceTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_ReplaceTextCtrl->SetValue( sourceoption->Text2() );
    m_ReplaceTextCtrl->Enable( optiontype != guLYRIC_SOURCE_OPTION_TYPE_NOTFOUND );
    OptionsSizer->Add( m_ReplaceTextCtrl, 0, wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 5 );

    MainSizer->Add( OptionsSizer, 1, wxEXPAND, 5 );

    wxStdDialogButtonSizer * ButtonsSizer = new wxStdDialogButtonSizer();
    wxButton * ButtonsSizerOK = new wxButton( this, wxID_OK );
    ButtonsSizer->AddButton( ButtonsSizerOK );
    wxButton * ButtonsSizerCancel = new wxButton( this, wxID_CANCEL );
    ButtonsSizer->AddButton( ButtonsSizerCancel );
    ButtonsSizer->SetAffirmativeButton( ButtonsSizerOK );
    ButtonsSizer->SetCancelButton( ButtonsSizerCancel );
    ButtonsSizer->Realize();
    MainSizer->Add( ButtonsSizer, 1, wxEXPAND|wxALL, 5 );

    SetSizer( MainSizer );
    Layout();

    ButtonsSizerOK->SetDefault();

    m_SearchTextCtrl->SetFocus();
}

// -------------------------------------------------------------------------------- //
guLyricSourceOptionEditor::~guLyricSourceOptionEditor()
{
}

// -------------------------------------------------------------------------------- //
void guLyricSourceOptionEditor::UpdateSourceOption( void )
{
    m_SourceOption->Text1( m_SearchTextCtrl->GetValue() );
    m_SourceOption->Text2( m_ReplaceTextCtrl->GetValue() );
}

}

// -------------------------------------------------------------------------------- //
