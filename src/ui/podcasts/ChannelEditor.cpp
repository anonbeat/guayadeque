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
#include "ChannelEditor.h"

#include "Config.h"
#include "EventCommandIds.h"
#include "Images.h"
#include "Utils.h"

#include <wx/filename.h>

namespace Guayadeque {

wxDECLARE_EVENT( guChannelEditorEvent, wxCommandEvent );

#define guPODCASTS_IMAGE_SIZE   60

// -------------------------------------------------------------------------------- //
guChannelEditor::guChannelEditor( wxWindow * parent, guPodcastChannel * channel ) :
    wxDialog( parent, wxID_ANY, _( "Podcast Channel Editor" ), wxDefaultPosition, wxSize( 564,329 ), wxDEFAULT_DIALOG_STYLE )
{
    wxStaticText* DescLabel;
    wxStaticText* AuthorLabel;
    wxStaticText* OwnerLabel;
    wxStaticText* DownloadLabel;
    wxStaticText* DeleteLabel;
    wxStdDialogButtonSizer* ButtonsSizer;
    wxButton* ButtonsSizerOK;
    wxButton* ButtonsSizerCancel;

    m_PodcastChannel = channel;

    guConfig * Config = ( guConfig * ) guConfig::Get();

	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* ChannelSizer;
	ChannelSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _( " Podcast Channel " ) ), wxVERTICAL );

	wxFlexGridSizer* FlexGridSizer;
    FlexGridSizer = new wxFlexGridSizer( 2, 0, 0 );
	FlexGridSizer->AddGrowableCol( 1 );
	FlexGridSizer->SetFlexibleDirection( wxBOTH );
	FlexGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_Image = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize( guPODCASTS_IMAGE_SIZE,guPODCASTS_IMAGE_SIZE ), 0 );
	FlexGridSizer->Add( m_Image, 0, wxALL, 5 );

    // Check that the directory to store podcasts are created
    wxString PodcastsPath = Config->ReadStr( CONFIG_KEY_PODCASTS_PATH, guPATH_PODCASTS, CONFIG_PATH_PODCASTS );
    wxFileName ImageFile = wxFileName( PodcastsPath + wxT( "/" ) +
                                       channel->m_Title + wxT( "/" ) +
                                       channel->m_Title + wxT( ".jpg" ) );
    if( ImageFile.Normalize( wxPATH_NORM_ALL | wxPATH_NORM_CASE ) )
    {
        wxImage PodcastImage;
        if( wxFileExists( ImageFile.GetFullPath() ) &&
            PodcastImage.LoadFile( ImageFile.GetFullPath() ) &&
            PodcastImage.IsOk() )
        {
            m_Image->SetBitmap( PodcastImage );
        }
        else
        {
            m_Image->SetBitmap( guBitmap( guIMAGE_INDEX_mid_podcast ) );
            if( !channel->m_Image.IsEmpty() )
            {
                guChannelUpdateImageThread * UpdateImageThread = new guChannelUpdateImageThread( this, channel->m_Image.c_str() );
                if( !UpdateImageThread )
                {
                    guLogError( wxT( "Could not create the Channel Image Thread" ) );
                }
            }
        }
    }

	m_Title = new wxStaticText( this, wxID_ANY, channel->m_Title, wxDefaultPosition, wxDefaultSize, 0 );
	FlexGridSizer->Add( m_Title, 1, wxEXPAND, 5 );

	DescLabel = new wxStaticText( this, wxID_ANY, _( "Description:" ), wxDefaultPosition, wxDefaultSize, 0 );
	DescLabel->Wrap( -1 );
	DescLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );

	FlexGridSizer->Add( DescLabel, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_DescText = new wxStaticText( this, wxID_ANY, ( channel->m_Description.Length() > 200 ?
                        channel->m_Description.Mid( 0, 200 ) + wxT( " ..." ) :
                        channel->m_Description ), wxDefaultPosition, wxDefaultSize, 0 );
	m_DescText->Wrap( 450 );
	FlexGridSizer->Add( m_DescText, 0, wxEXPAND|wxBOTTOM|wxRIGHT, 5 );

	AuthorLabel = new wxStaticText( this, wxID_ANY, _("Author:"), wxDefaultPosition, wxDefaultSize, 0 );
	AuthorLabel->Wrap( -1 );
	AuthorLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );

	FlexGridSizer->Add( AuthorLabel, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_AuthorText = new wxStaticText( this, wxID_ANY, channel->m_Author, wxDefaultPosition, wxDefaultSize, 0 );
	m_AuthorText->Wrap( -1 );
	FlexGridSizer->Add( m_AuthorText, 0, wxEXPAND|wxBOTTOM|wxRIGHT, 5 );

	OwnerLabel = new wxStaticText( this, wxID_ANY, _("Owner:"), wxDefaultPosition, wxDefaultSize, 0 );
	OwnerLabel->Wrap( -1 );
	OwnerLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );

	FlexGridSizer->Add( OwnerLabel, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_OwnerText = new wxStaticText( this, wxID_ANY, channel->m_OwnerName +
                  wxT( " ( " ) + channel->m_OwnerEmail + wxT( " )" ) , wxDefaultPosition, wxDefaultSize, 0 );
	m_OwnerText->Wrap( -1 );
	FlexGridSizer->Add( m_OwnerText, 0, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	DownloadLabel = new wxStaticText( this, wxID_ANY, _("Download:"), wxDefaultPosition, wxDefaultSize, 0 );
	DownloadLabel->Wrap( -1 );
	FlexGridSizer->Add( DownloadLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	wxBoxSizer* DownloadSizer;
	DownloadSizer = new wxBoxSizer( wxHORIZONTAL );

	wxString m_DownloadChoiceChoices[] = { _( "Manually" ), _( "Only if contains" ), _( "Everything" ) };
	int m_DownloadChoiceNChoices = sizeof( m_DownloadChoiceChoices ) / sizeof( wxString );
	m_DownloadChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_DownloadChoiceNChoices, m_DownloadChoiceChoices, 0 );
	m_DownloadChoice->SetSelection( channel->m_DownloadType );
	DownloadSizer->Add( m_DownloadChoice, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );

	m_DownloadText = new wxTextCtrl( this, wxID_ANY, channel->m_DownloadText, wxDefaultPosition, wxDefaultSize, 0 );
	m_DownloadText->Enable( ( channel->m_DownloadType == guPODCAST_DOWNLOAD_FILTER ) );

	DownloadSizer->Add( m_DownloadText, 1, wxEXPAND|wxALL, 5 );

	FlexGridSizer->Add( DownloadSizer, 1, wxEXPAND, 5 );

	DeleteLabel = new wxStaticText( this, wxID_ANY, _("Delete:"), wxDefaultPosition, wxDefaultSize, 0 );
	DeleteLabel->Wrap( -1 );
	FlexGridSizer->Add( DeleteLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_DeleteCheckBox = new wxCheckBox( this, wxID_ANY, _( "Allow delete old items" ), wxDefaultPosition, wxDefaultSize, 0 );
	m_DeleteCheckBox->SetValue( channel->m_AllowDelete );

	FlexGridSizer->Add( m_DeleteCheckBox, 0, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	ChannelSizer->Add( FlexGridSizer, 1, wxEXPAND, 5 );

	MainSizer->Add( ChannelSizer, 1, wxEXPAND|wxALL, 5 );

	ButtonsSizer = new wxStdDialogButtonSizer();
	ButtonsSizerOK = new wxButton( this, wxID_OK );
	ButtonsSizer->AddButton( ButtonsSizerOK );
	ButtonsSizerCancel = new wxButton( this, wxID_CANCEL );
	ButtonsSizer->AddButton( ButtonsSizerCancel );
	ButtonsSizer->SetAffirmativeButton( ButtonsSizerOK );
	ButtonsSizer->SetCancelButton( ButtonsSizerCancel );
	ButtonsSizer->Realize();
	MainSizer->Add( ButtonsSizer, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

	ButtonsSizerOK->SetDefault();

    // Bind Events
    m_DownloadChoice->Bind( wxEVT_CHOICE, &guChannelEditor::OnDownloadChoice, this );
    Bind( guChannelEditorEvent, &guChannelEditor::OnChannelImageUpdated, this, guCHANNELEDITOR_EVENT_UPDATE_IMAGE );

    m_DescText->SetFocus();
}

// -------------------------------------------------------------------------------- //
guChannelEditor::~guChannelEditor()
{
    // Unbind Events
    m_DownloadChoice->Unbind( wxEVT_CHOICE, &guChannelEditor::OnDownloadChoice, this );
    Unbind( guChannelEditorEvent, &guChannelEditor::OnChannelImageUpdated, this, guCHANNELEDITOR_EVENT_UPDATE_IMAGE );
}

// -------------------------------------------------------------------------------- //
void guChannelEditor::OnDownloadChoice( wxCommandEvent &event )
{
    m_DownloadText->Enable( m_DownloadChoice->GetSelection() == 1 );
}

// -------------------------------------------------------------------------------- //
void guChannelEditor::GetEditData( void )
{
    //m_PodcastChannel->m_Title = m_Title->GetValue();
    m_PodcastChannel->m_DownloadType = m_DownloadChoice->GetSelection();
    m_PodcastChannel->m_DownloadText = ( m_PodcastChannel->m_DownloadType == 1 ) ?
                                        m_DownloadText->GetValue() : wxT( "" );
    m_PodcastChannel->m_AllowDelete = m_DeleteCheckBox->IsChecked();
}

// -------------------------------------------------------------------------------- //
void guChannelEditor::OnChannelImageUpdated( wxCommandEvent &event )
{
    wxImage * Image = ( wxImage * ) event.GetClientData();
    if( Image )
    {
        m_Image->SetBitmap( * Image );
        delete Image;
    }
}

// -------------------------------------------------------------------------------- //
// guChannelUpdateImageThread
// -------------------------------------------------------------------------------- //
guChannelUpdateImageThread::guChannelUpdateImageThread( guChannelEditor * channeleditor, const wxChar * imageurl ) :
    wxThread( wxTHREAD_DETACHED )
{
    m_ChannelEditor = channeleditor;
    m_ImageUrl      = wxString( imageurl );

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guChannelUpdateImageThread::~guChannelUpdateImageThread()
{
}

// -------------------------------------------------------------------------------- //
guChannelUpdateImageThread::ExitCode guChannelUpdateImageThread::Entry()
{
    wxImage *       Image = NULL;

    if( !m_ImageUrl.IsEmpty() )
    {
        if( !TestDestroy() )
        {
            wxBitmapType    ImageType;
            Image = guGetRemoteImage( m_ImageUrl, ImageType );
            if( Image )
            {
                //guLogMessage( wxT( "Image loaded ok %u" ), Index );
                if( Image->IsOk() && !TestDestroy() )
                {
                    Image->Rescale( guPODCASTS_IMAGE_SIZE, guPODCASTS_IMAGE_SIZE, wxIMAGE_QUALITY_HIGH );
                }
                else
                {
                  delete Image;
                  Image = NULL;
                }
            }
        }
    }
    //
    if( !TestDestroy() )
    {
        wxCommandEvent event( guChannelEditorEvent, guCHANNELEDITOR_EVENT_UPDATE_IMAGE );
        event.SetClientData( Image );
        wxPostEvent( m_ChannelEditor, event );
    }
    else if( Image )
    {
        delete Image;
    }
    return 0;
}

}

// -------------------------------------------------------------------------------- //
