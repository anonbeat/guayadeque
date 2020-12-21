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
#include "CoverEdit.h"

#include "Amazon.h"
#include "Apple.h"
#include "EventCommandIds.h"
#include "Config.h"
#include "CoverFetcher.h"
#include "CoverFrame.h"
//#include "Discogs.h"
#include "Google.h"
#include "Images.h"
#include "LastFMCovers.h"
#include "MainFrame.h"
#include "Utils.h"
#include "Yahoo.h"

#include <wx/arrimpl.cpp>
#include <wx/filedlg.h>
#include <wx/statline.h>

#define MAX_COVERLINKS_ITEMS            30

namespace Guayadeque {

enum guCOVER_SEARCH_ENGINE {
//    guCOVER_SEARCH_ENGINE_GOOGLE = 0,
//    guCOVER_SEARCH_ENGINE_AMAZON,
    guCOVER_SEARCH_ENGINE_APPLE,
    guCOVER_SEARCH_ENGINE_LASTFM,
//    guCOVER_SEARCH_ENGINE_DISCOGS
//    guCOVER_SEARCH_ENGINE_YAHOO
};

WX_DEFINE_OBJARRAY( guCoverImageArray )

// -------------------------------------------------------------------------------- //
guCoverEditor::guCoverEditor( wxWindow* parent, const wxString &Artist, const wxString &Album, const wxString &albumpath ) :
               wxDialog( parent, wxID_ANY, _( "Cover Editor" ), wxDefaultPosition, wxSize( 520, 446 ), wxDEFAULT_DIALOG_STYLE )
{
    m_AlbumPath = albumpath;

    wxStaticText* ArtistStaticText;
    wxStaticText* AlbumStaticText;
    wxStaticText* FromStaticText;
    wxStaticLine * TopStaticLine;

    guConfig * Config = ( guConfig * ) guConfig::Get();

    this->SetSizeHints( wxDefaultSize, wxDefaultSize );

    wxBoxSizer* MainSizer;
    MainSizer = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer* EditsSizer;
    EditsSizer = new wxBoxSizer( wxHORIZONTAL );

    ArtistStaticText = new wxStaticText( this, wxID_ANY, _( "Artist:" ), wxDefaultPosition, wxDefaultSize, 0 );
    ArtistStaticText->Wrap( -1 );
    EditsSizer->Add( ArtistStaticText, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT, 5 );

    m_ArtistTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
    EditsSizer->Add( m_ArtistTextCtrl, 1, wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 5 );

    AlbumStaticText = new wxStaticText( this, wxID_ANY, _( "Album:" ), wxDefaultPosition, wxDefaultSize, 0 );
    AlbumStaticText->Wrap( -1 );
    EditsSizer->Add( AlbumStaticText, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT, 5 );

    m_AlbumTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
    EditsSizer->Add( m_AlbumTextCtrl, 1, wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 5 );

    FromStaticText = new wxStaticText( this, wxID_ANY, _( "From:" ), wxDefaultPosition, wxDefaultSize, 0 );
    FromStaticText->Wrap( -1 );
    EditsSizer->Add( FromStaticText, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT, 5 );

    wxString m_EngineChoiceChoices[] = {
        //wxT( "Google" ),
        //wxT( "Amazon" ),
        wxT( "Apple" ),
        wxT("Last.fm"),
        //wxT( "Discogs" )
        //wxT( "Yahoo" )
        };
    int m_EngineChoiceNChoices = sizeof( m_EngineChoiceChoices ) / sizeof( wxString );
    m_EngineChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_EngineChoiceNChoices, m_EngineChoiceChoices, 0 );
    EditsSizer->Add( m_EngineChoice, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );

    MainSizer->Add( EditsSizer, 0, wxEXPAND, 5 );

    
    wxBoxSizer * TopButtonsSizer = new wxBoxSizer( wxHORIZONTAL );

    m_EmbedToFilesChkBox = new wxCheckBox( this, wxID_ANY, _( "Embed into tracks" ), wxDefaultPosition, wxDefaultSize, 0 );
    m_EmbedToFilesChkBox->SetValue( Config->ReadBool( CONFIG_KEY_GENERAL_EMBED_TO_FILES, false, CONFIG_PATH_GENERAL ) );
    TopButtonsSizer->Add( m_EmbedToFilesChkBox, 0, wxEXPAND|wxRIGHT|wxLEFT, 5 );

    TopButtonsSizer->Add( 10, 10, wxEXPAND );
    
    m_CoverSelectButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_download_covers ), wxDefaultPosition, wxSize( 32, 32 ), wxBU_AUTODRAW );
    TopButtonsSizer->Add( m_CoverSelectButton, 0, wxALIGN_RIGHT | wxLEFT, 5 );

    m_CoverDownloadButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_doc_save ), wxDefaultPosition, wxSize( 32, 32 ), wxBU_AUTODRAW );
    TopButtonsSizer->Add( m_CoverDownloadButton, 0, wxALIGN_RIGHT | wxLEFT | wxRIGHT, 5 );

    MainSizer->Add( TopButtonsSizer, 0, wxEXPAND, 5 );

    
    TopStaticLine = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    MainSizer->Add( TopStaticLine, 0, wxEXPAND | wxALL, 5 );

    wxBoxSizer * CoverSizer = new wxBoxSizer( wxHORIZONTAL );


    CoverSizer->Add( 0, 0, 1, wxEXPAND, 5 );

    m_PrevButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_left ), wxDefaultPosition, wxSize( 32, 96 ), wxBU_AUTODRAW );
    CoverSizer->Add( m_PrevButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

    int CoverFrame = Config->ReadNum( CONFIG_KEY_GENERAL_COVER_FRAME, guCOVERFRAME_DEFAULT, CONFIG_PATH_GENERAL );
    wxImage DefaultCover( guImage( CoverFrame ? guIMAGE_INDEX_blank_cd_cover : guIMAGE_INDEX_no_cover ) );
    
    if( !CoverFrame ) DefaultCover.Rescale( 250, 250, wxIMAGE_QUALITY_HIGH );

    m_CoverBitmap = new wxStaticBitmap( this, wxID_ANY, DefaultCover, wxDefaultPosition, wxSize( -1,-1 ), 0 );
    CoverSizer->Add( m_CoverBitmap, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );

    m_NextButton = new wxBitmapButton( this, wxID_ANY, guImage( guIMAGE_INDEX_right ), wxDefaultPosition, wxSize( 32, 96 ), wxBU_AUTODRAW );
    CoverSizer->Add( m_NextButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );


    CoverSizer->Add( 0, 0, 1, wxEXPAND, 5 );

    MainSizer->Add( CoverSizer, 1, wxEXPAND, 5 );

    m_SizeSizer = new wxBoxSizer( wxVERTICAL );

    m_SizeStaticText = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    m_SizeStaticText->Wrap( -1 );
    m_SizeSizer->Add( m_SizeStaticText, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );

    MainSizer->Add( m_SizeSizer, 0, wxEXPAND, 5 );

    wxBoxSizer* GaugeSizer = new wxBoxSizer( wxHORIZONTAL );

    m_Gauge = new guAutoPulseGauge( this, wxID_ANY, MAX_COVERLINKS_ITEMS, wxDefaultPosition, wxSize( -1,7 ), wxGA_HORIZONTAL );
    m_Gauge->SetValue( 5 );
    GaugeSizer->Add( m_Gauge, 1, wxEXPAND|wxALL, 5 );

    m_InfoTextCtrl = new wxStaticText( this, wxID_ANY, wxT("00/00"), wxDefaultPosition, wxDefaultSize, 0 );
    m_InfoTextCtrl->Wrap( -1 );
    GaugeSizer->Add( m_InfoTextCtrl, 0, wxRIGHT, 5 );

    MainSizer->Add( GaugeSizer, 0, wxEXPAND, 5 );

    wxStdDialogButtonSizer * ButtonsSizer = new wxStdDialogButtonSizer();
    wxButton * ButtonsSizerOK = new wxButton( this, wxID_OK );
    ButtonsSizer->AddButton( ButtonsSizerOK );
    wxButton * ButtonsSizerCancel = new wxButton( this, wxID_CANCEL );
    ButtonsSizer->AddButton( ButtonsSizerCancel );
    ButtonsSizer->SetAffirmativeButton( ButtonsSizerOK );
    ButtonsSizer->SetCancelButton( ButtonsSizerCancel );
    ButtonsSizer->Realize();
    MainSizer->Add( ButtonsSizer, 0, wxEXPAND|wxALIGN_BOTTOM|wxALL, 5 );

    this->SetSizer( MainSizer );
    this->Layout();

    ButtonsSizerOK->SetDefault();

    m_ArtistTextCtrl->SetValue( Artist );
    m_AlbumTextCtrl->SetValue( Album );
    m_CurrentImage = 0;

    m_EngineIndex = Config->ReadNum( CONFIG_KEY_GENERAL_COVER_SEARCH_ENGINE, 0, CONFIG_PATH_GENERAL );
    if( m_EngineIndex >= ( int ) m_EngineChoice->GetCount() )
        m_EngineIndex = 0;
    m_EngineChoice->SetSelection( m_EngineIndex );

    // Bind Events
    m_ArtistTextCtrl->Bind( wxEVT_TEXT_ENTER, &guCoverEditor::OnTextCtrlEnter, this );
    m_AlbumTextCtrl->Bind( wxEVT_TEXT_ENTER, &guCoverEditor::OnTextCtrlEnter, this );

    m_EngineChoice->Bind( wxEVT_CHOICE, &guCoverEditor::OnEngineChanged, this );

    m_CoverBitmap->Bind( wxEVT_LEFT_DCLICK, &guCoverEditor::OnCoverLeftDClick, this );
    m_CoverBitmap->Bind( wxEVT_LEFT_DOWN, &guCoverEditor::OnCoverLeftClick, this );
    Bind( wxEVT_MOUSEWHEEL, &guCoverEditor::OnMouseWheel, this );

    m_PrevButton->Bind( wxEVT_BUTTON, &guCoverEditor::OnPrevButtonClick, this );
    m_NextButton->Bind( wxEVT_BUTTON, &guCoverEditor::OnNextButtonClick, this );

    m_CoverSelectButton->Bind( wxEVT_BUTTON, &guCoverEditor::OnCoverSelectClick, this );
    m_CoverDownloadButton->Bind( wxEVT_BUTTON, &guCoverEditor::OnCoverDownloadClick, this );

    Bind( wxEVT_MENU, &guCoverEditor::OnAddCoverImage, this, ID_COVEREDITOR_ADDCOVERIMAGE  );
    Bind( wxEVT_MENU, &guCoverEditor::OnDownloadedLinks, this, ID_COVEREDITOR_DOWNLOADEDLINKS );

    m_DownloadCoversThread = new guFetchCoverLinksThread( this, Artist.c_str(), Album.c_str(), m_EngineIndex );

    m_PrevButton->Disable();
    m_NextButton->Disable();

    m_ArtistTextCtrl->SetFocus();
}

// -------------------------------------------------------------------------------- //
guCoverEditor::~guCoverEditor()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->WriteNum( CONFIG_KEY_GENERAL_COVER_SEARCH_ENGINE, m_EngineChoice->GetSelection(), CONFIG_PATH_GENERAL );
    Config->WriteBool( CONFIG_KEY_GENERAL_EMBED_TO_FILES, m_EmbedToFilesChkBox->GetValue(), CONFIG_PATH_GENERAL );

    m_DownloadThreadMutex.Lock();
    int index;
    int count = m_DownloadThreads.Count();
    for( index = 0; index < count; index++ )
    {
        guDownloadCoverThread * DownThread = ( guDownloadCoverThread * ) m_DownloadThreads[ index ];
        if( DownThread )
        {
            DownThread->Pause();
            DownThread->Delete();
        }
        m_DownloadThreads[ index ] = NULL;
    }
    m_DownloadThreadMutex.Unlock();

    if( m_DownloadCoversThread )
    {
        m_DownloadCoversThread->Pause();
        m_DownloadCoversThread->Delete();
    }

    // Unbind Events
    m_ArtistTextCtrl->Unbind( wxEVT_TEXT_ENTER, &guCoverEditor::OnTextCtrlEnter, this );
    m_AlbumTextCtrl->Unbind( wxEVT_TEXT_ENTER, &guCoverEditor::OnTextCtrlEnter, this );

    m_EngineChoice->Unbind( wxEVT_CHOICE, &guCoverEditor::OnEngineChanged, this );

    m_CoverBitmap->Unbind( wxEVT_LEFT_DCLICK, &guCoverEditor::OnCoverLeftDClick, this );
    m_CoverBitmap->Unbind( wxEVT_LEFT_DOWN, &guCoverEditor::OnCoverLeftClick, this );
    Unbind( wxEVT_MOUSEWHEEL, &guCoverEditor::OnMouseWheel, this );

    m_PrevButton->Unbind( wxEVT_BUTTON, &guCoverEditor::OnPrevButtonClick, this );
    m_NextButton->Unbind( wxEVT_BUTTON, &guCoverEditor::OnNextButtonClick, this );

    m_CoverSelectButton->Unbind( wxEVT_BUTTON, &guCoverEditor::OnCoverSelectClick, this );
    m_CoverDownloadButton->Unbind( wxEVT_BUTTON, &guCoverEditor::OnCoverDownloadClick, this );


    Unbind( wxEVT_MENU, &guCoverEditor::OnAddCoverImage, this, ID_COVEREDITOR_ADDCOVERIMAGE  );
    Unbind( wxEVT_MENU, &guCoverEditor::OnDownloadedLinks, this, ID_COVEREDITOR_DOWNLOADEDLINKS );

}

// -------------------------------------------------------------------------------- //
void guCoverEditor::EndDownloadLinksThread( void )
{
    m_DownloadThreadMutex.Lock();
    if( !m_DownloadThreads.Count() )
    {
        if( m_Gauge->IsPulsing() )
        {
            m_Gauge->StopPulse( MAX_COVERLINKS_ITEMS, MAX_COVERLINKS_ITEMS );
        }
        else
        {
            m_Gauge->SetValue( MAX_COVERLINKS_ITEMS );
        }
    }
    m_DownloadThreadMutex.Unlock();
    //m_Gauge->SetValue( 0 );
    //guLogMessage( wxT( "EndDownloadThread called" ) );
    m_DownloadCoversThread = NULL;
}

// -------------------------------------------------------------------------------- //
void guCoverEditor::OnDownloadedLinks( wxCommandEvent &event )
{
    EndDownloadLinksThread();
}

// -------------------------------------------------------------------------------- //
void guCoverEditor::EndDownloadCoverThread( guDownloadCoverThread * DownloadCoverThread )
{
    if( m_Gauge->IsPulsing() )
    {
        m_Gauge->StopPulse( MAX_COVERLINKS_ITEMS, 1 );
    }
    else
    {
        m_Gauge->SetValue( wxMin( m_Gauge->GetValue() + 1, MAX_COVERLINKS_ITEMS ) );
    }
    m_DownloadThreadMutex.Lock();
    m_DownloadThreads.Remove( DownloadCoverThread );
    m_DownloadThreadMutex.Unlock();
}

// -------------------------------------------------------------------------------- //
void guCoverEditor::OnPrevButtonClick( wxCommandEvent& event )
{
    if( m_CurrentImage )
    {
        m_CurrentImage--;
        if( !m_NextButton->IsEnabled() )
            m_NextButton->Enable();
        if( !m_CurrentImage )
            m_PrevButton->Disable();

        UpdateCoverBitmap();
    }
}

// -------------------------------------------------------------------------------- //
void guCoverEditor::OnNextButtonClick( wxCommandEvent& event )
{
    if( m_CurrentImage < ( int ) ( m_AlbumCovers.Count() - 1 ) )
    {
        m_CurrentImage++;
        if( m_CurrentImage == ( int ) m_AlbumCovers.Count() - 1 )
            m_NextButton->Disable();
        if( !m_PrevButton->IsEnabled() )
            m_PrevButton->Enable();

        UpdateCoverBitmap();
    }
}

// -------------------------------------------------------------------------------- //
void guCoverEditor::OnMouseWheel( wxMouseEvent &event )
{
    int Rotation = event.GetWheelRotation() / event.GetWheelDelta() * -1;
    int NewPos = m_CurrentImage + Rotation;

    //guLogMessage( wxT( "MouseWheel %i => %i" ), m_CurrentImage, NewPos );

    NewPos = wxMax( 0, wxMin( NewPos, ( int ) m_AlbumCovers.Count() - 1 ) );

    if( NewPos != m_CurrentImage )
    {
        m_CurrentImage = NewPos;

        m_NextButton->Enable( m_CurrentImage < ( int ) m_AlbumCovers.Count() - 1 );
        m_PrevButton->Enable( m_CurrentImage );

        UpdateCoverBitmap();
    }
}

// -------------------------------------------------------------------------------- //
void guCoverEditor::OnAddCoverImage( wxCommandEvent &event )
{
    m_DownloadEventsMutex.Lock();
    guCoverImage * Image = ( guCoverImage * ) event.GetClientData();
    if( Image )
    {
        if( Image->m_SizeStr.IsEmpty() )
            Image->m_SizeStr = wxString::Format( wxT( "%ux%u" ), Image->m_Image->GetWidth(), Image->m_Image->GetHeight() );
        m_AlbumCovers.Add( Image );
        // When it is the first show the image.
        if( m_AlbumCovers.Count() == 1 )
            UpdateCoverBitmap();
        if( ( m_CurrentImage < ( int ) ( m_AlbumCovers.Count() - 1 ) ) && !m_NextButton->IsEnabled() )
            m_NextButton->Enable();
        m_InfoTextCtrl->SetLabel( wxString::Format( wxT( "%02u/%02lu" ), m_CurrentImage + 1, m_AlbumCovers.Count() ) );
        //guLogMessage( wxT( "CurImg: %u  Total:%lu" ), m_CurrentImage + 1, m_AlbumCovers.Count() );
    }
        if( event.GetClientObject() )
            EndDownloadCoverThread( ( guDownloadCoverThread * ) event.GetClientObject() );
    m_DownloadEventsMutex.Unlock();
}

// -------------------------------------------------------------------------------- //
void guCoverEditor::UpdateCoverBitmap( void )
{
    m_InfoTextCtrl->SetLabel( wxString::Format( wxT( "%02u/%02lu" ),  m_AlbumCovers.Count() ? m_CurrentImage + 1 : 0, m_AlbumCovers.Count() ) );

    guConfig * Config = ( guConfig * ) guConfig::Get();
    int CoverFrame = Config->ReadNum( CONFIG_KEY_GENERAL_COVER_FRAME, guCOVERFRAME_DEFAULT, CONFIG_PATH_GENERAL );
    if( CoverFrame == guCOVERFRAME_DEFAULT )
    {
        wxBitmap * BlankCD = new wxBitmap( guImage( guIMAGE_INDEX_blank_cd_cover ) );
        if( BlankCD )
        {
            if( BlankCD->IsOk() )
            {
                if( m_AlbumCovers.Count() && m_AlbumCovers[ m_CurrentImage ].m_Image )
                {
                    wxImage CoverImage = m_AlbumCovers[ m_CurrentImage ].m_Image->Copy();
                    // 38,6
                    wxMemoryDC MemDC;
                    MemDC.SelectObject( * BlankCD );
                    CoverImage.Rescale( 250, 250, wxIMAGE_QUALITY_HIGH );
                    MemDC.DrawBitmap( wxBitmap( CoverImage ), 34, 4, false );
                    // Update the Size label
                    m_SizeStaticText->SetLabel( m_AlbumCovers[ m_CurrentImage ].m_SizeStr );
                }
                else
                {
                    m_SizeStaticText->SetLabel( wxEmptyString );
                }
                m_SizeSizer->Layout();
                m_CoverBitmap->SetBitmap( * BlankCD );
                m_CoverBitmap->Refresh();
            }
            delete BlankCD;
        }
    }
    else
    {
        wxImage CoverImage;
        if( m_AlbumCovers.Count() && m_AlbumCovers[ m_CurrentImage ].m_Image )
        {
            CoverImage = m_AlbumCovers[ m_CurrentImage ].m_Image->Copy();
            // Update the Size label
            m_SizeStaticText->SetLabel( m_AlbumCovers[ m_CurrentImage ].m_SizeStr );
        }
        else
        {
            CoverImage = guImage( guIMAGE_INDEX_no_cover );
            m_SizeStaticText->SetLabel( wxEmptyString );
        }
        CoverImage.Rescale( 250, 250, wxIMAGE_QUALITY_HIGH );
        m_SizeSizer->Layout();
        m_CoverBitmap->SetBitmap( CoverImage );
        m_CoverBitmap->Refresh();
    }
}

// -------------------------------------------------------------------------------- //
void guCoverEditor::OnCoverLeftDClick( wxMouseEvent &event )
{
    if( event.m_x >= 75 && event.m_x <= 225 )
        EndModal( wxID_OK );
}

// -------------------------------------------------------------------------------- //
void guCoverEditor::OnCoverLeftClick( wxMouseEvent &event )
{
    wxCommandEvent CmdEvent;
    if( event.m_x < 75 )
    {
        OnPrevButtonClick( CmdEvent );
    }
    else if( event.m_x > 225 )
    {
        OnNextButtonClick( CmdEvent );
    }
    //guLogMessage( wxT( "%i %i" ), event.m_x, event.m_y );
    event.Skip();
}

// -------------------------------------------------------------------------------- //
wxString guCoverEditor::GetSelectedCoverUrl( void )
{
    if( m_AlbumCovers.Count() )
        return m_AlbumCovers[ m_CurrentImage ].m_Link;
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
wxImage * guCoverEditor::GetSelectedCoverImage( void )
{
    if( m_AlbumCovers.Count() )
        return m_AlbumCovers[ m_CurrentImage ].m_Image;
    return NULL;
}

// -------------------------------------------------------------------------------- //
void guCoverEditor::OnTextCtrlEnter( wxCommandEvent& event )
{
    m_DownloadThreadMutex.Lock();
    int index;
    int count = m_DownloadThreads.Count();
    for( index = 0; index < count; index++ )
    {
        guDownloadCoverThread * DownThread = ( guDownloadCoverThread * ) m_DownloadThreads[ index ];
        if( DownThread )
        {
            DownThread->Pause();
            DownThread->Delete();
        }
    }
    m_DownloadThreads.Empty();
    m_DownloadThreadMutex.Unlock();

    // If Thread still running delete it
    if( m_DownloadCoversThread )
    {
        m_DownloadCoversThread->Pause();
        m_DownloadCoversThread->Delete();
    }
    // Empty already downloaded covers
    m_AlbumCovers.Empty();
    // Reset to the 1st Image
	m_CurrentImage = 0;
	// Set blank Cover Bitmap
	UpdateCoverBitmap();

    m_Gauge->StartPulse();

    // Start again the cover fetcher thread
    m_DownloadCoversThread = new guFetchCoverLinksThread( this,
                 m_ArtistTextCtrl->GetValue().c_str(),
                 m_AlbumTextCtrl->GetValue().c_str(), m_EngineIndex );

    // Disable buttons till one cover is downloaded
    m_PrevButton->Disable();
    m_NextButton->Disable();
}

// -------------------------------------------------------------------------------- //
void guCoverEditor::OnEngineChanged( wxCommandEvent& event )
{
    if( m_EngineIndex != m_EngineChoice->GetSelection() )
    {
        m_DownloadThreadMutex.Lock();
        int index;
        int count = m_DownloadThreads.Count();
        for( index = 0; index < count; index++ )
        {
            guDownloadCoverThread * DownThread = ( guDownloadCoverThread * ) m_DownloadThreads[ index ];
            if( DownThread )
            {
                DownThread->Pause();
                DownThread->Delete();
            }
        }
        m_DownloadThreads.Empty();
        m_DownloadThreadMutex.Unlock();

        // If Thread still running delete it
        if( m_DownloadCoversThread )
        {
            m_DownloadCoversThread->Pause();
            m_DownloadCoversThread->Delete();
        }

        // Empty already downloaded covers
        m_AlbumCovers.Empty();
        // Reset to the 1st Image
        m_CurrentImage = 0;
        // Set blank Cover Bitmap
        UpdateCoverBitmap();

        m_Gauge->StartPulse();

        m_EngineIndex = m_EngineChoice->GetSelection();

        // Start again the cover fetcher thread
        m_DownloadCoversThread = new guFetchCoverLinksThread( this,
                     m_ArtistTextCtrl->GetValue().c_str(),
                     m_AlbumTextCtrl->GetValue().c_str(), m_EngineIndex );

        // Disable buttons till one cover is downloaded
        m_PrevButton->Disable();
        m_NextButton->Disable();
    }
}

// -------------------------------------------------------------------------------- //
void guCoverEditor::OnCoverSelectClick( wxCommandEvent & )
{
    wxFileDialog * FileDialog = new wxFileDialog( this, _( "Select the cover filename" ),
        m_AlbumPath, "", wxT( "*.jpg;*.JPG;*.jpeg;*.JPEG;*.png;*,PNG" ),
        wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_PREVIEW );
    if( FileDialog )
    {
        if( FileDialog->ShowModal() == wxID_OK )
        {
            wxString CoverFile = FileDialog->GetPath();
            if( !CoverFile.IsEmpty() )
            {
                wxImage * CoverImage = new wxImage( CoverFile );
                if( CoverImage->IsOk() )
                {
                    wxCommandEvent event( wxEVT_MENU, ID_COVEREDITOR_ADDCOVERIMAGE );
                    event.SetClientData( new guCoverImage( CoverFile, "", CoverImage ) );
                    OnAddCoverImage( event );
                }
            }
        }
        FileDialog->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guCoverEditor::OnCoverDownloadClick( wxCommandEvent & )
{
    wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( this, "Url:",
                                            _( "Cover Url" ), wxEmptyString );
    if( EntryDialog )
    {
        if( EntryDialog->ShowModal() == wxID_OK )
        {
            wxString CoverUrl = EntryDialog->GetValue();
            if( !CoverUrl.IsEmpty() )
            {
                wxBitmapType CoverType;
                wxImage * CoverImage = guGetRemoteImage( CoverUrl, CoverType );
                if( CoverImage )
                {
                    wxCommandEvent event( wxEVT_MENU, ID_COVEREDITOR_ADDCOVERIMAGE );
                    event.SetClientData( new guCoverImage( CoverUrl, "", CoverImage ) );
                    OnAddCoverImage( event );                
                }
            }

        }
        EntryDialog->Destroy();
    }
}


// -------------------------------------------------------------------------------- //
// guFetchCoverLinksThread
// -------------------------------------------------------------------------------- //
guFetchCoverLinksThread::guFetchCoverLinksThread( guCoverEditor * owner,
                    const wxChar * artist, const wxChar * album, int engineindex ) :
    wxThread()
{
    m_CoverEditor   = owner;
    m_CoverFetcher = NULL;
//    m_Artist = wxString( artist );
//    m_Album = wxString( album );
    m_EngineIndex = engineindex;
    m_LastDownload  = 0;
    m_CurrentPage   = 0;

//    if( m_EngineIndex == guCOVER_SEARCH_ENGINE_GOOGLE )
//    {
//        m_CoverFetcher = ( guCoverFetcher * ) new guGoogleCoverFetcher( this, &m_CoverLinks, artist, album );
//    }
//    else if( m_EngineIndex == guCOVER_SEARCH_ENGINE_AMAZON )
    // if( m_EngineIndex == guCOVER_SEARCH_ENGINE_AMAZON )
    // {
    //     m_CoverFetcher = ( guCoverFetcher * ) new guAmazonCoverFetcher( this, &m_CoverLinks, artist, album );
    // }
    //    else if( m_EngineIndex == guCOVER_SEARCH_ENGINE_APPLE )
    if( m_EngineIndex == guCOVER_SEARCH_ENGINE_APPLE )
    {
        m_CoverFetcher = ( guCoverFetcher * ) new guAppleCoverFetcher( this, &m_CoverLinks, artist, album );
    }
    else if( m_EngineIndex == guCOVER_SEARCH_ENGINE_LASTFM )
    {
        m_CoverFetcher = ( guCoverFetcher * ) new guLastFMCoverFetcher( this, &m_CoverLinks, artist, album );
    }
//    else if( m_EngineIndex == guCOVER_SEARCH_ENGINE_DISCOGS )
//    {
//        m_CoverFetcher = ( guCoverFetcher * ) new guDiscogsCoverFetcher( this, &m_CoverLinks, artist, album );
//    }
    // else if( m_EngineIndex == guCOVER_SEARCH_ENGINE_YAHOO )
    // {
    //     m_CoverFetcher = ( guCoverFetcher * ) new guYahooCoverFetcher( this, &m_CoverLinks, artist, album );
    // }

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guFetchCoverLinksThread::~guFetchCoverLinksThread()
{
    if( !TestDestroy() )
    {
        wxCommandEvent event( wxEVT_MENU, ID_COVEREDITOR_DOWNLOADEDLINKS );
        event.SetClientObject( ( wxClientData * ) this );
        wxPostEvent( m_CoverEditor, event );
    }
}

// -------------------------------------------------------------------------------- //
guFetchCoverLinksThread::ExitCode guFetchCoverLinksThread::Entry()
{
    bool NoMorePics = false;

    if( m_CoverFetcher )
    {
        while( !TestDestroy() )
        {
            if( m_LastDownload < ( int ) m_CoverLinks.Count() )
            {
                m_CoverEditor->m_DownloadThreadMutex.Lock();
                if( !TestDestroy() )
                {
                    guDownloadCoverThread * DownloadThread = new guDownloadCoverThread( m_CoverEditor,
                                              &m_CoverLinks[ m_LastDownload ] );
                    if( !DownloadThread )
                    {
                        guLogError( wxT( "Could not create the download covers thread" ) );
                    }
                }
                m_CoverEditor->m_DownloadThreadMutex.Unlock();
                m_LastDownload++;
                Sleep( 20 );
            }
            else
            {
                if( NoMorePics )
                    break;
                if( m_CoverLinks.Count() > MAX_COVERLINKS_ITEMS )
                    break;
                if( !m_CoverFetcher->AddCoverLinks( m_CurrentPage ) )
                {
                    NoMorePics = true;
                    if( m_LastDownload < ( int ) m_CoverLinks.Count() )
                    {
                        continue;
                    }
                    break;
                }
                m_CurrentPage++;
            }
        }
    }

    return 0;
}

// -------------------------------------------------------------------------------- //
// guDownloadCoverThread
// -------------------------------------------------------------------------------- //
guDownloadCoverThread::guDownloadCoverThread( guCoverEditor * Owner, const wxArrayString * CoverInfo ) :
    wxThread()
{
    m_CoverEditor = Owner;
    m_UrlStr      = wxString( ( * CoverInfo )[ GUCOVERINFO_LINK ].c_str() );
    m_SizeStr     = wxString( ( * CoverInfo )[ GUCOVERINFO_SIZE ].c_str() );
    //guLogMessage( wxT( "Link : %s (%s)" ), UrlStr.c_str(), SizeStr.c_str() );
    m_CoverEditor->m_DownloadThreads.Add( this );

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guDownloadCoverThread::~guDownloadCoverThread()
{
}

// -------------------------------------------------------------------------------- //
guDownloadCoverThread::ExitCode guDownloadCoverThread::Entry()
{
    wxBitmapType ImageType;
    guCoverImage *  CoverImage = NULL;
    wxImage *   Image = guGetRemoteImage( m_UrlStr, ImageType );

    if( Image )
    {
        if( !TestDestroy() )
        {
            CoverImage = new guCoverImage( m_UrlStr, m_SizeStr, Image );
        }
        else
        {
          //guLogWarning( wxT( "Could not load image from the net index %u." ), LastDownload );
          delete Image;
        }
    }

    if( !TestDestroy() )
    {
        //guLogMessage( wxT( "Done: '%s'" ), m_UrlStr.c_str() );
        //m_CoverEditor->m_DownloadEventsMutex.Lock();
        wxCommandEvent event( wxEVT_MENU, ID_COVEREDITOR_ADDCOVERIMAGE );
        event.SetClientObject( ( wxClientData * ) this );
        event.SetClientData( CoverImage );
        wxPostEvent( m_CoverEditor, event );
        //m_CoverEditor->m_DownloadEventsMutex.Unlock();
    }
    else
    {
        if( CoverImage )
            delete CoverImage;
    }
    return 0;
}

}

// -------------------------------------------------------------------------------- //
