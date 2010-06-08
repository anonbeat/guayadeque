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
#include "PodcastsPanel.h"

#include "AuiDockArt.h"
#include "ChannelEditor.h"
#include "Commands.h"
#include "Config.h"
#include "Images.h"
#include "MainFrame.h"
#include "NewChannel.h"
#include "Utils.h"

#include <wx/curl/http.h>
#include <wx/regex.h>
#include <wx/sstream.h>
#include <wx/uri.h>
#include <wx/xml/xml.h>
#include <wx/zstream.h>

// -------------------------------------------------------------------------------- //
// guPostcastPanel
// -------------------------------------------------------------------------------- //
guPodcastPanel::guPodcastPanel( wxWindow * parent, guDbLibrary * db, guMainFrame * mainframe, guPlayerPanel * playerpanel ) :
    wxPanel( parent, wxID_ANY, wxDefaultPosition, wxSize( 672,586 ), wxTAB_TRAVERSAL )
{
    m_Db = db;
    m_MainFrame = mainframe;
    m_PlayerPanel = playerpanel;
    m_LastChannelInfoId = wxNOT_FOUND;
    m_LastPodcastInfoId = wxNOT_FOUND;

    wxPanel * ChannelsPanel;
    wxPanel * PodcastsPanel;
    wxStaticText * DetailDescLabel;
    wxStaticText * DetailAuthorLabel;
    wxStaticText * DetailOwnerLabel;
    //wxStaticText * DetailEmailLabel;
    wxStaticLine * DetailStaticLine1;
    wxStaticLine * DetailStaticLine2;
    wxStaticText * DetailItemTitleLabel;
    wxStaticText * DetailItemSumaryLabel;
    wxStaticText * DetailItemDateLabel;
    wxStaticText * DetailItemLengthLabel;


    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->RegisterObject( this );

    // Check that the directory to store podcasts are created
    m_PodcastsPath = Config->ReadStr( wxT( "Path" ), wxGetHomeDir() + wxT( "/.guayadeque/Podcasts" ), wxT( "Podcasts" ) );
    if( !wxDirExists( m_PodcastsPath ) )
    {
        wxMkdir( m_PodcastsPath, 0770 );
    }

    m_AuiManager.SetManagedWindow( this );
    m_AuiManager.SetArtProvider( new guAuiDockArt() );
    m_AuiManager.SetFlags( wxAUI_MGR_ALLOW_FLOATING |
                           wxAUI_MGR_TRANSPARENT_DRAG |
                           wxAUI_MGR_TRANSPARENT_HINT );
    wxAuiDockArt * AuiDockArt = m_AuiManager.GetArtProvider();
    AuiDockArt->SetColour( wxAUI_DOCKART_INACTIVE_CAPTION_TEXT_COLOUR,
            wxSystemSettings::GetColour( wxSYS_COLOUR_INACTIVECAPTIONTEXT ) );
    AuiDockArt->SetColour( wxAUI_DOCKART_ACTIVE_CAPTION_TEXT_COLOUR,
            wxSystemSettings::GetColour( wxSYS_COLOUR_CAPTIONTEXT ) );

    AuiDockArt->SetColour( wxAUI_DOCKART_ACTIVE_CAPTION_COLOUR,
            wxSystemSettings::GetColour( wxSYS_COLOUR_ACTIVEBORDER ) );

    AuiDockArt->SetColour( wxAUI_DOCKART_ACTIVE_CAPTION_GRADIENT_COLOUR,
            wxSystemSettings::GetColour( wxSYS_COLOUR_3DSHADOW ) );

    AuiDockArt->SetColour( wxAUI_DOCKART_INACTIVE_CAPTION_COLOUR,
            wxSystemSettings::GetColour( wxSYS_COLOUR_INACTIVEBORDER ) );

    AuiDockArt->SetColour( wxAUI_DOCKART_INACTIVE_CAPTION_GRADIENT_COLOUR,
            wxSystemSettings::GetColour( wxSYS_COLOUR_3DSHADOW ) );

    AuiDockArt->SetColour( wxAUI_DOCKART_GRADIENT_TYPE,
            wxAUI_GRADIENT_VERTICAL );

    m_VisiblePanels = Config->ReadNum( wxT( "PodVisiblePanels" ), guPANEL_PODCASTS_VISIBLE_DEFAULT, wxT( "Positions" ) );


	ChannelsPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer * ChannelsMainSizer;
	ChannelsMainSizer = new wxBoxSizer( wxVERTICAL );

	m_ChannelsListBox = new guChannelsListBox( ChannelsPanel, m_Db, _( "Channels" ) );
	ChannelsMainSizer->Add( m_ChannelsListBox, 1, wxEXPAND, 5 );

	ChannelsPanel->SetSizer( ChannelsMainSizer );
	ChannelsPanel->Layout();
	ChannelsMainSizer->Fit( ChannelsPanel );

    m_AuiManager.AddPane( ChannelsPanel,
            wxAuiPaneInfo().Name( wxT( "PodcastsChannels" ) ).Caption( _( "Channels" ) ).
            MinSize( 50, 50 ).
            CloseButton( Config->ReadBool( wxT( "ShowPaneCloseButton" ), true, wxT( "General" ) ) ).
            Dockable( true ).Left() );



	PodcastsPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer * MainPodcastsSizer;
	MainPodcastsSizer = new wxBoxSizer( wxVERTICAL );

	m_PodcastsListBox = new guPodcastListBox( PodcastsPanel, m_Db );
	MainPodcastsSizer->Add( m_PodcastsListBox, 1, wxALL|wxEXPAND, 1 );

	PodcastsPanel->SetSizer( MainPodcastsSizer );
	PodcastsPanel->Layout();
	MainPodcastsSizer->Fit( PodcastsPanel );

    m_AuiManager.AddPane( PodcastsPanel, wxAuiPaneInfo().Name( wxT( "PodcastsItems" ) ).Caption( _( "Podcasts" ) ).
            MinSize( 50, 50 ).
            CenterPane() );



	wxPanel * DetailsPanel;
	DetailsPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );

	m_DetailMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* DetailSizer;
	DetailSizer = new wxStaticBoxSizer( new wxStaticBox( DetailsPanel, wxID_ANY, _(" Details ") ), wxVERTICAL );

	m_DetailScrolledWindow = new wxScrolledWindow( DetailsPanel, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), wxHSCROLL );
	m_DetailScrolledWindow->SetScrollRate( 5, 5 );
	m_DetailScrolledWindow->SetMinSize( wxSize( -1,100 ) );

	m_DetailFlexGridSizer = new wxFlexGridSizer( 2, 2, 0, 0 );
	m_DetailFlexGridSizer->AddGrowableCol( 1 );
	m_DetailFlexGridSizer->SetFlexibleDirection( wxBOTH );
	m_DetailFlexGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_DetailImage = new wxStaticBitmap( m_DetailScrolledWindow, wxID_ANY, guImage( guIMAGE_INDEX_mid_podcast ), wxDefaultPosition, wxSize( 60,60 ), 0 );
	m_DetailFlexGridSizer->Add( m_DetailImage, 0, wxALL, 5 );

	m_DetailChannelTitle = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_DetailChannelTitle->Wrap( -1 );
	m_DetailChannelTitle->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	m_DetailFlexGridSizer->Add( m_DetailChannelTitle, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	DetailDescLabel = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, _("Description:"), wxDefaultPosition, wxDefaultSize, 0 );
	DetailDescLabel->Wrap( -1 );
	DetailDescLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );

	m_DetailFlexGridSizer->Add( DetailDescLabel, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_DetailDescText = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE );
	//m_DetailDescText->Wrap( 445 );
	m_DetailFlexGridSizer->Add( m_DetailDescText, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	DetailAuthorLabel = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, _("Author:"), wxDefaultPosition, wxDefaultSize, 0 );
	DetailAuthorLabel->Wrap( -1 );
	DetailAuthorLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );

	m_DetailFlexGridSizer->Add( DetailAuthorLabel, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_DetailAuthorText = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_DetailAuthorText->Wrap( -1 );
	m_DetailFlexGridSizer->Add( m_DetailAuthorText, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	DetailOwnerLabel = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, _("Owner:"), wxDefaultPosition, wxDefaultSize, 0 );
	DetailOwnerLabel->Wrap( -1 );
	DetailOwnerLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );

	m_DetailFlexGridSizer->Add( DetailOwnerLabel, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_DetailOwnerText = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_DetailOwnerText->Wrap( -1 );
	m_DetailFlexGridSizer->Add( m_DetailOwnerText, 0, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	wxStaticText * DetailLinkLabel;
	DetailLinkLabel = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, _("Link:"), wxDefaultPosition, wxDefaultSize, 0 );
	DetailLinkLabel->Wrap( -1 );
	DetailLinkLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );

	m_DetailFlexGridSizer->Add( DetailLinkLabel, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_DetailLinkText = new wxHyperlinkCtrl( m_DetailScrolledWindow, wxID_ANY, wxEmptyString, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
	m_DetailFlexGridSizer->Add( m_DetailLinkText, 0, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	DetailStaticLine1 = new wxStaticLine( m_DetailScrolledWindow, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	m_DetailFlexGridSizer->Add( DetailStaticLine1, 0, wxEXPAND|wxBOTTOM|wxLEFT, 5 );

	DetailStaticLine2 = new wxStaticLine( m_DetailScrolledWindow, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	m_DetailFlexGridSizer->Add( DetailStaticLine2, 0, wxEXPAND|wxBOTTOM|wxRIGHT, 5 );

	DetailItemTitleLabel = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, _("Title:"), wxDefaultPosition, wxDefaultSize, 0 );
	DetailItemTitleLabel->Wrap( -1 );
	DetailItemTitleLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );

	m_DetailFlexGridSizer->Add( DetailItemTitleLabel, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_DetailItemTitleText = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_DetailItemTitleText->Wrap( -1 );
	m_DetailFlexGridSizer->Add( m_DetailItemTitleText, 0, wxBOTTOM|wxRIGHT, 5 );

	DetailItemSumaryLabel = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, _("Sumary:"), wxDefaultPosition, wxDefaultSize, 0 );
	DetailItemSumaryLabel->Wrap( -1 );
	DetailItemSumaryLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );

	m_DetailFlexGridSizer->Add( DetailItemSumaryLabel, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_DetailItemSumaryText = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE );
	//m_DetailItemSumaryText->Wrap( 300 );
	m_DetailFlexGridSizer->Add( m_DetailItemSumaryText, 0, wxBOTTOM|wxRIGHT|wxEXPAND, 5 );

	DetailItemDateLabel = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, _("Date:"), wxDefaultPosition, wxDefaultSize, 0 );
	DetailItemDateLabel->Wrap( -1 );
	DetailItemDateLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );

	m_DetailFlexGridSizer->Add( DetailItemDateLabel, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_DetailItemDateText = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_DetailItemDateText->Wrap( -1 );
	m_DetailFlexGridSizer->Add( m_DetailItemDateText, 0, wxBOTTOM|wxRIGHT, 5 );

	DetailItemLengthLabel = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, _("Length:"), wxDefaultPosition, wxDefaultSize, 0 );
	DetailItemLengthLabel->Wrap( -1 );
	DetailItemLengthLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );

	m_DetailFlexGridSizer->Add( DetailItemLengthLabel, 0, wxBOTTOM|wxRIGHT|wxLEFT|wxALIGN_RIGHT, 5 );

	m_DetailItemLengthText = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_DetailItemLengthText->Wrap( -1 );
	m_DetailFlexGridSizer->Add( m_DetailItemLengthText, 0, wxBOTTOM|wxRIGHT, 5 );

	m_DetailScrolledWindow->SetSizer( m_DetailFlexGridSizer );
	m_DetailScrolledWindow->Layout();
	m_DetailFlexGridSizer->Fit( m_DetailScrolledWindow );
	DetailSizer->Add( m_DetailScrolledWindow, 1, wxEXPAND | wxALL, 5 );

	m_DetailMainSizer->Add( DetailSizer, 1, wxEXPAND|wxALL, 5 );

	DetailsPanel->SetSizer( m_DetailMainSizer );
	DetailsPanel->Layout();
	DetailSizer->Fit( DetailsPanel );

    m_AuiManager.AddPane( DetailsPanel, wxAuiPaneInfo().Name( wxT( "PodcastsDetails" ) ).Caption( _( "Podcast Details" ) ).
            MinSize( 100, 100 ).
            CloseButton( Config->ReadBool( wxT( "ShowPaneCloseButton" ), true, wxT( "General" ) ) ).
            Dockable( true ).Bottom() );


    wxString PodcastLayout = Config->ReadStr( wxT( "Podcasts" ), wxEmptyString, wxT( "Positions" ) );
    if( Config->GetIgnoreLayouts() || PodcastLayout.IsEmpty() )
    {
        m_AuiManager.Update();
        m_VisiblePanels = guPANEL_PODCASTS_VISIBLE_DEFAULT;
    }
    else
    {
        m_AuiManager.LoadPerspective( PodcastLayout, true );
    }

    Connect( ID_PODCASTS_CHANNEL_ADD, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPodcastPanel::AddChannel ), NULL, this );
    Connect( ID_PODCASTS_CHANNEL_DEL, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPodcastPanel::DeleteChannels ), NULL, this );
    Connect( ID_PODCASTS_CHANNEL_PROPERTIES, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPodcastPanel::ChannelProperties ), NULL, this );
    Connect( ID_PODCASTS_CHANNEL_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPodcastPanel::ChannelsCopyTo ), NULL, this );
    Connect( ID_PODCASTS_CHANNEL_UPDATE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPodcastPanel::UpdateChannels ), NULL, this );

    Connect( ID_PODCASTS_ITEM_UPDATED, guPodcastEvent, wxCommandEventHandler( guPodcastPanel::OnPodcastItemUpdated ), NULL, this );

    m_ChannelsListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED,  wxListEventHandler( guPodcastPanel::OnChannelsSelected ), NULL, this );
    m_ChannelsListBox->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxListEventHandler( guPodcastPanel::OnChannelsActivated ), NULL, this );
	m_PodcastsListBox->Connect( wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler( guPodcastPanel::OnPodcastsColClick ), NULL, this );
    m_PodcastsListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED,  wxListEventHandler( guPodcastPanel::OnPodcastItemSelected ), NULL, this );
    m_PodcastsListBox->Connect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxListEventHandler( guPodcastPanel::OnPodcastItemActivated ), NULL, this );

    Connect( ID_PODCASTS_ITEM_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPodcastPanel::OnPodcastItemPlay ), NULL, this );
    Connect( ID_PODCASTS_ITEM_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPodcastPanel::OnPodcastItemEnqueue ), NULL, this );
    Connect( ID_PODCASTS_ITEM_ENQUEUE_ASNEXT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPodcastPanel::OnPodcastItemEnqueueAsNext ), NULL, this );
    Connect( ID_PODCASTS_ITEM_DEL, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPodcastPanel::OnPodcastItemDelete ), NULL, this );
    Connect( ID_PODCASTS_ITEM_DOWNLOAD, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPodcastPanel::OnPodcastItemDownload ), NULL, this );
    Connect( ID_PODCASTS_ITEM_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPodcastPanel::OnPodcastItemCopyTo ), NULL, this );

    Connect( ID_CONFIG_UPDATED, guConfigUpdatedEvent, wxCommandEventHandler( guPodcastPanel::OnConfigUpdated ), NULL, this );

    m_AuiManager.Connect( wxEVT_AUI_PANE_CLOSE, wxAuiManagerEventHandler( guPodcastPanel::OnPaneClose ), NULL, this );
}

// -------------------------------------------------------------------------------- //
guPodcastPanel::~guPodcastPanel()
{
    // Save the Splitter positions into the main config
    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        Config->UnRegisterObject( this );

        Config->WriteNum( wxT( "PodVisiblePanels" ), m_VisiblePanels, wxT( "Positions" ) );
        Config->WriteStr( wxT( "Podcasts" ), m_AuiManager.SavePerspective(), wxT( "Positions" ) );
    }

    Disconnect( ID_PODCASTS_CHANNEL_ADD, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPodcastPanel::AddChannel ), NULL, this );
    Disconnect( ID_PODCASTS_CHANNEL_DEL, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPodcastPanel::DeleteChannels ), NULL, this );
    Disconnect( ID_PODCASTS_CHANNEL_PROPERTIES, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPodcastPanel::ChannelProperties ), NULL, this );
    Disconnect( ID_PODCASTS_CHANNEL_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPodcastPanel::ChannelsCopyTo ), NULL, this );
    Disconnect( ID_PODCASTS_CHANNEL_UPDATE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPodcastPanel::UpdateChannels ), NULL, this );

    Disconnect( ID_PODCASTS_ITEM_UPDATED, guPodcastEvent, wxCommandEventHandler( guPodcastPanel::OnPodcastItemUpdated ), NULL, this );

    m_ChannelsListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED,  wxListEventHandler( guPodcastPanel::OnChannelsSelected ), NULL, this );
    m_ChannelsListBox->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxListEventHandler( guPodcastPanel::OnChannelsActivated ), NULL, this );
	m_PodcastsListBox->Disconnect( wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler( guPodcastPanel::OnPodcastsColClick ), NULL, this );
    m_PodcastsListBox->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED,  wxListEventHandler( guPodcastPanel::OnPodcastItemSelected ), NULL, this );
    m_PodcastsListBox->Disconnect( wxEVT_COMMAND_LISTBOX_DOUBLECLICKED, wxListEventHandler( guPodcastPanel::OnPodcastItemActivated ), NULL, this );

    Disconnect( ID_PODCASTS_ITEM_PLAY, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPodcastPanel::OnPodcastItemPlay ), NULL, this );
    Disconnect( ID_PODCASTS_ITEM_ENQUEUE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPodcastPanel::OnPodcastItemEnqueue ), NULL, this );
    Disconnect( ID_PODCASTS_ITEM_ENQUEUE_ASNEXT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPodcastPanel::OnPodcastItemEnqueueAsNext ), NULL, this );
    Disconnect( ID_PODCASTS_ITEM_DEL, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPodcastPanel::OnPodcastItemDelete ), NULL, this );
    Disconnect( ID_PODCASTS_ITEM_DOWNLOAD, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPodcastPanel::OnPodcastItemDownload ), NULL, this );
    Disconnect( ID_PODCASTS_ITEM_COPYTO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPodcastPanel::OnPodcastItemCopyTo ), NULL, this );

    Disconnect( ID_CONFIG_UPDATED, guConfigUpdatedEvent, wxCommandEventHandler( guPodcastPanel::OnConfigUpdated ), NULL, this );

    m_AuiManager.Disconnect( wxEVT_AUI_PANE_CLOSE, wxAuiManagerEventHandler( guPodcastPanel::OnPaneClose ), NULL, this );

    m_AuiManager.UnInit();
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::OnConfigUpdated( wxCommandEvent &event )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();

    // Check that the directory to store podcasts are created
    m_PodcastsPath = Config->ReadStr( wxT( "Path" ), wxGetHomeDir() + wxT( "/.guayadeque/Podcasts" ), wxT( "Podcasts" ) );
    if( !wxDirExists( m_PodcastsPath ) )
    {
        wxMkdir( m_PodcastsPath, 0770 );
    }
}

// -------------------------------------------------------------------------------- //
void NormalizePodcastChannel( guPodcastChannel * PodcastChannel )
{
    int ChId = PodcastChannel->m_Id;
    wxString ChName = PodcastChannel->m_Title;
    wxString Category = PodcastChannel->m_Category;
    int Index;
    int Count = PodcastChannel->m_Items.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guPodcastItem * PodcastItem = &PodcastChannel->m_Items[ Index ];
        PodcastItem->m_ChId = ChId;
        PodcastItem->m_Channel = ChName;
        PodcastItem->m_Category = Category;
    }
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::AddChannel( wxCommandEvent &event )
{
    guNewPodcastChannelSelector * NewPodcastChannel = new guNewPodcastChannelSelector( this );
    if( NewPodcastChannel->ShowModal() == wxID_OK )
    {
        wxSetCursor( * wxHOURGLASS_CURSOR );
        wxTheApp->Yield();

        wxString PodcastUrl = NewPodcastChannel->GetValue().Trim( wxString::both );
        if( !PodcastUrl.IsEmpty() )
        {
            // If we find an itunes link we replace the itpc to the standard http
            if( PodcastUrl.StartsWith( wxT( "itpc://" ) ) )
            {
                PodcastUrl.Replace( wxT( "itpc://" ), wxT( "http://" ) );
            }

            guPodcastChannel PodcastChannel( PodcastUrl );

            wxSetCursor( wxNullCursor );
                    //
            guChannelEditor * ChannelEditor = new guChannelEditor( this, &PodcastChannel );
            if( ChannelEditor->ShowModal() == wxID_OK )
            {
                wxSetCursor( * wxHOURGLASS_CURSOR );

                ChannelEditor->GetEditData();

                // Create the channel dir
                wxFileName ChannelDir = wxFileName( m_PodcastsPath + wxT( "/" ) +
                                          PodcastChannel.m_Title );
                if( ChannelDir.Normalize( wxPATH_NORM_ALL | wxPATH_NORM_CASE ) )
                {
                    if( !wxDirExists( ChannelDir.GetFullPath() ) )
                    {
                        wxMkdir( ChannelDir.GetFullPath(), 0770 );
                    }
                }

                PodcastChannel.CheckLogo();

                //
                //guLogMessage( wxT( "The Channel have DownloadType : %u" ), PodcastChannel.m_DownloadType );
                m_Db->SavePodcastChannel( &PodcastChannel );

                PodcastChannel.CheckDownloadItems( m_Db, m_MainFrame );

                m_ChannelsListBox->ReloadItems();
            }
            ChannelEditor->Destroy();
        }
        wxSetCursor( wxNullCursor );
    }
    NewPodcastChannel->Destroy();
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::DeleteChannels( wxCommandEvent &event )
{
    if( wxMessageBox( _( "Are you sure to delete the selected podcast channel?" ),
                      _( "Confirm" ),
                      wxICON_QUESTION | wxYES_NO | wxCANCEL, this ) == wxYES )
    {
        wxArrayInt SelectedItems = m_ChannelsListBox->GetSelectedItems();
        int Index;
        int Count;
        if( ( Count = SelectedItems.Count() ) )
        {
            wxSetCursor( * wxHOURGLASS_CURSOR );
            for( Index = 0; Index < Count; Index++ )
            {
                m_Db->DelPodcastChannel( SelectedItems[ Index ] );
            }
            m_ChannelsListBox->ReloadItems();
            wxSetCursor( wxNullCursor );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::ChannelProperties( wxCommandEvent &event )
{
    guPodcastChannel PodcastChannel;
    wxArrayInt SelectedItems = m_ChannelsListBox->GetSelectedItems();
    m_Db->GetPodcastChannelId( SelectedItems[ 0 ], &PodcastChannel );

    guChannelEditor * ChannelEditor = new guChannelEditor( this, &PodcastChannel );
    if( ChannelEditor->ShowModal() == wxID_OK )
    {
        wxSetCursor( * wxHOURGLASS_CURSOR );

        ChannelEditor->GetEditData();

        // Create the channel dir
        wxFileName ChannelDir = wxFileName( m_PodcastsPath + wxT( "/" ) +
                                  PodcastChannel.m_Title );
        if( ChannelDir.Normalize( wxPATH_NORM_ALL | wxPATH_NORM_CASE ) )
        {
            if( !wxDirExists( ChannelDir.GetFullPath() ) )
            {
                wxMkdir( ChannelDir.GetFullPath(), 0770 );
            }
        }

        PodcastChannel.CheckLogo();

        //
        //guLogMessage( wxT( "The Channel have DownloadType : %u" ), PodcastChannel.m_DownloadType );

        m_Db->SavePodcastChannel( &PodcastChannel );

        PodcastChannel.CheckDownloadItems( m_Db, m_MainFrame );

        m_ChannelsListBox->ReloadItems();

        wxSetCursor( wxNullCursor );
    }
    ChannelEditor->Destroy();
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::ChannelsCopyTo( wxCommandEvent &event )
{
    guPodcastItemArray PodcastItems;
    guTrackArray * Tracks = new guTrackArray();

    m_Db->GetPodcastItems( &PodcastItems );

    int Index;
    int Count = PodcastItems.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guTrack * Track = new guTrack();
        if( Track )
        {
            Track->m_Type = guTRACK_TYPE_PODCAST;
            Track->m_SongId = PodcastItems[ Index ].m_Id;
            Track->m_FileName = PodcastItems[ Index ].m_FileName;
            Track->m_SongName = PodcastItems[ Index ].m_Title;
            Track->m_ArtistName = PodcastItems[ Index ].m_Author;
            Track->m_AlbumId = PodcastItems[ Index ].m_ChId;
            Track->m_AlbumName = PodcastItems[ Index ].m_Channel;
            Track->m_Length = PodcastItems[ Index ].m_Length;
            Track->m_PlayCount = PodcastItems[ Index ].m_PlayCount;
            Track->m_GenreName = wxT( "Podcasts" );
            Track->m_Number = Index;
            Track->m_Rating = -1;
            Track->m_CoverId = 0;
            Track->m_Year = 0; // Get year from item date
            Tracks->Add( Track );
        }
    }

    event.SetId( ID_MAINFRAME_COPYTO );
    event.SetClientData( ( void * ) Tracks );
    wxPostEvent( m_MainFrame, event );
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::UpdateChannels( wxCommandEvent &event )
{
    wxArrayInt Selected = m_ChannelsListBox->GetSelectedItems();
    int Count;
    if( ( Count = Selected.Count() ) )
    {
        wxSetCursor( * wxHOURGLASS_CURSOR );
        guPodcastChannel PodcastChannel;
        int Index;
        for( Index = 0; Index < Count; Index++ )
        {
            if( m_Db->GetPodcastChannelId( Selected[ Index ], &PodcastChannel ) != wxNOT_FOUND )
            {
                ProcessChannel( &PodcastChannel );
            }
        }
        wxSetCursor( wxNullCursor );
    }
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::OnChannelsSelected( wxListEvent &event )
{
    wxArrayInt SelectedItems = m_ChannelsListBox->GetSelectedItems();
    m_Db->SetPodcastChannelFilters( SelectedItems );
    m_PodcastsListBox->ReloadItems();

    if( SelectedItems.Count() == 1 && SelectedItems[ 0 ] != 0 )
        UpdateChannelInfo( SelectedItems[ 0 ] );
    else
        UpdateChannelInfo( -1 );
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::ProcessChannel( guPodcastChannel * channel )
{
    wxSetCursor( * wxHOURGLASS_CURSOR );
    wxTheApp->Yield();

    channel->Update( m_Db, m_MainFrame );

    m_ChannelsListBox->ReloadItems( false );
    m_PodcastsListBox->ReloadItems( false );

    wxSetCursor( wxNullCursor );
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::OnChannelsActivated( wxListEvent &event )
{
    wxArrayInt Selected = m_ChannelsListBox->GetSelectedItems();

    if( Selected.Count() )
    {
        guPodcastChannel PodcastChannel;
        if( m_Db->GetPodcastChannelId( Selected[ 0 ], &PodcastChannel ) != wxNOT_FOUND )
        {
            ProcessChannel( &PodcastChannel );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::OnPodcastsColClick( wxListEvent &event )
{
    int ColId = m_PodcastsListBox->GetColumnId( event.m_col );

    m_PodcastsListBox->SetOrder( ColId );
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::OnPodcastItemSelected( wxListEvent &event )
{
    wxArrayInt Selection = m_PodcastsListBox->GetSelectedItems();
    UpdatePodcastInfo( Selection.Count() ? Selection[ 0 ] : -1 );
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::UpdatePodcastInfo( int itemid )
{
    if( m_LastPodcastInfoId == itemid )
        return;

    m_LastPodcastInfoId = itemid;

    guPodcastItem       PodcastItem;
    //
    //guLogMessage( wxT( "Updating the podcast info of the item %u" ), itemid );
    if( itemid > 0 )
    {
        m_Db->GetPodcastItemId( itemid, &PodcastItem );

        UpdateChannelInfo( PodcastItem.m_ChId );

        m_DetailItemTitleText->SetLabel( PodcastItem.m_Title );
        m_DetailItemSumaryText->SetLabel( PodcastItem.m_Summary );
        wxDateTime AddedDate;
        AddedDate.Set( ( time_t ) PodcastItem.m_Time );
        m_DetailItemDateText->SetLabel( AddedDate.Format( wxT( "%a, %d %b %Y %T %z" ) ) );
        m_DetailItemLengthText->SetLabel( LenToString( PodcastItem.m_Length ) );
    }
    else
    {
        m_DetailItemTitleText->SetLabel( wxEmptyString );
        m_DetailItemSumaryText->SetLabel( wxEmptyString );
        m_DetailItemDateText->SetLabel( wxEmptyString );
        m_DetailItemLengthText->SetLabel( wxEmptyString );
    }
    m_DetailMainSizer->Layout();
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::UpdateChannelInfo( int itemid )
{
    if( m_LastChannelInfoId == itemid )
        return;

    m_LastChannelInfoId = itemid;

    guPodcastChannel    PodcastChannel;
    //
    //guLogMessage( wxT( "Updating the channel info of the item %u" ), itemid );
    if( itemid > 0 )
    {
        m_Db->GetPodcastChannelId( itemid, &PodcastChannel );

        // Set Image...
        wxFileName ImageFile = wxFileName( m_PodcastsPath + wxT( "/" ) +
                                           PodcastChannel.m_Title + wxT( "/" ) +
                                           PodcastChannel.m_Title + wxT( ".jpg" ) );
        if( ImageFile.Normalize( wxPATH_NORM_ALL|wxPATH_NORM_CASE ) )
        {
            wxImage PodcastImage;
            if( wxFileExists( ImageFile.GetFullPath() ) &&
                PodcastImage.LoadFile( ImageFile.GetFullPath() ) &&
                PodcastImage.IsOk() )
            {
                m_DetailImage->SetBitmap( PodcastImage );
            }
            else
                m_DetailImage->SetBitmap( guBitmap( guIMAGE_INDEX_mid_podcast ) );
        }

        m_DetailChannelTitle->SetLabel( PodcastChannel.m_Title );
        m_DetailDescText->SetLabel( PodcastChannel.m_Description );
        m_DetailAuthorText->SetLabel( PodcastChannel.m_Author );
        m_DetailOwnerText->SetLabel( PodcastChannel.m_OwnerName +
                                     wxT( " (" ) + PodcastChannel.m_OwnerEmail + wxT( ")" ) );
        m_DetailLinkText->SetLabel( PodcastChannel.m_Url );
        m_DetailLinkText->SetURL( PodcastChannel.m_Url );
    }
    else
    {
        m_DetailImage->SetBitmap( guBitmap( guIMAGE_INDEX_mid_podcast ) );
        m_DetailChannelTitle->SetLabel( wxEmptyString );
        m_DetailDescText->SetLabel( wxEmptyString );
        m_DetailAuthorText->SetLabel( wxEmptyString );
        m_DetailOwnerText->SetLabel( wxEmptyString );
        m_DetailLinkText->SetURL( wxEmptyString );
        m_DetailLinkText->SetLabel( wxEmptyString );
    }
    m_DetailMainSizer->FitInside( m_DetailScrolledWindow );
    m_DetailScrolledWindow->SetVirtualSize( m_DetailMainSizer->GetSize() );
    //m_DetailFlexGridSizer->FitInside( m_DetailScrolledWindow );
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::OnPodcastItemUpdated( wxCommandEvent &event )
{
    guPodcastItem * PodcastItem = ( guPodcastItem * ) event.GetClientData();

    //guLogMessage( wxT( "PodcastItem Updated... Item: %s Status: %u" ), PodcastItem->m_Title.c_str(), PodcastItem->m_Status );
    m_Db->SavePodcastItem( PodcastItem->m_ChId, PodcastItem );
    m_PodcastsListBox->ReloadItems( false );

    delete PodcastItem;
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::OnSelectPodcasts( bool enqueue, const bool asnext )
{
    int Index;
    int Count;
    wxArrayInt Selected = m_PodcastsListBox->GetSelectedItems();
    if( ( Count = Selected.Count() ) )
    {
        guTrackArray Tracks;
        for( Index = 0; Index < Count; Index++ )
        {
            guPodcastItem PodcastItem;
            if( m_Db->GetPodcastItemId( Selected[ Index ], &PodcastItem ) != wxNOT_FOUND )
            {
                if( PodcastItem.m_Status == guPODCAST_STATUS_READY )
                {
                    if( wxFileExists( PodcastItem.m_FileName ) )
                    {
                        guTrack * Track = new guTrack();
                        if( Track )
                        {
                            Track->m_Type = guTRACK_TYPE_PODCAST;
                            Track->m_SongId = PodcastItem.m_Id;
                            Track->m_FileName = PodcastItem.m_FileName;
                            Track->m_SongName = PodcastItem.m_Title;
                            Track->m_ArtistName = PodcastItem.m_Author;
                            Track->m_AlbumId = PodcastItem.m_ChId;
                            Track->m_AlbumName = PodcastItem.m_Channel;
                            Track->m_Length = PodcastItem.m_Length;
                            Track->m_PlayCount = PodcastItem.m_PlayCount;
                            Track->m_Rating = -1;
                            Track->m_CoverId = 0;
                            Track->m_Year = 0; // Get year from item date
                            Tracks.Add( Track );
                        }
                    }
                    else
                    {
                        PodcastItem.m_Status = guPODCAST_STATUS_ERROR;
                        wxCommandEvent event( guPodcastEvent, ID_PODCASTS_ITEM_UPDATED );
                        event.SetClientData( new guPodcastItem( PodcastItem ) );
                        wxPostEvent( this, event );
                    }
                }
                else if( ( PodcastItem.m_Status == guPODCAST_STATUS_NORMAL ) ||
                         ( PodcastItem.m_Status == guPODCAST_STATUS_ERROR ) )
                {
                    // Download the item
                    guPodcastItemArray AddList;
                    AddList.Add( PodcastItem );
                    m_MainFrame->AddPodcastsDownloadItems( &AddList );

                    PodcastItem.m_Status = guPODCAST_STATUS_PENDING;
                    wxCommandEvent event( guPodcastEvent, ID_PODCASTS_ITEM_UPDATED );
                    event.SetClientData( new guPodcastItem( PodcastItem ) );
                    wxPostEvent( this, event );
                }
            }
        }

        if( Tracks.Count() )
        {
            if( enqueue )
            {
                m_PlayerPanel->AddToPlayList( Tracks, true, asnext );
            }
            else
            {
                m_PlayerPanel->SetPlayList( Tracks );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::OnPodcastItemActivated( wxListEvent &event )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    OnSelectPodcasts( Config->ReadBool( wxT( "DefaultActionEnqueue" ), false, wxT( "General" ) ) );
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::OnPodcastItemPlay( wxCommandEvent &event )
{
    OnSelectPodcasts( false );
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::OnPodcastItemEnqueue( wxCommandEvent &event )
{
    OnSelectPodcasts( true );
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::OnPodcastItemEnqueueAsNext( wxCommandEvent &event )
{
    OnSelectPodcasts( true, true );
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::OnPodcastItemDelete( wxCommandEvent &event )
{
    if( wxMessageBox( _( "Are you sure to delete the selected podcast item?" ),
                      _( "Confirm" ),
                      wxICON_QUESTION | wxYES_NO | wxCANCEL, this ) == wxYES )
    {
        wxArrayInt Selection = m_PodcastsListBox->GetSelectedItems();
        int Index;
        int Count = Selection.Count();

        // Check if in the download thread this items are included and delete them
        guPodcastItemArray Podcasts;
        m_Db->GetPodcastItems( Selection, &Podcasts );
        m_MainFrame->RemovePodcastDownloadItems( &Podcasts );

        for( Index = 0; Index < Count; Index++ )
        {
            m_Db->SetPodcastItemStatus( Selection[ Index ], guPODCAST_STATUS_DELETED );
        }

        m_PodcastsListBox->ReloadItems();
    }
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::OnPodcastItemDownload( wxCommandEvent &event )
{
    wxArrayInt Selection = m_PodcastsListBox->GetSelectedItems();
    guPodcastItemArray DownloadList;
    int Index;
    int Count = Selection.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guPodcastItem PodcastItem;
        m_Db->GetPodcastItemId( Selection[ Index ], &PodcastItem );
        DownloadList.Add( new guPodcastItem( PodcastItem ) );
    }
    m_MainFrame->AddPodcastsDownloadItems( &DownloadList );
    m_PodcastsListBox->ReloadItems( false );
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::OnPodcastItemCopyTo( wxCommandEvent &event )
{
    guTrackArray * Tracks = new guTrackArray();
    m_PodcastsListBox->GetSelectedSongs( Tracks );

    event.SetId( ID_MAINFRAME_COPYTO );
    event.SetClientData( ( void * ) Tracks );
    wxPostEvent( wxTheApp->GetTopWindow(), event );
}

// -------------------------------------------------------------------------------- //
bool guPodcastPanel::IsPanelShown( const int panelid ) const
{
    return ( m_VisiblePanels & panelid );
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::ShowPanel( const int panelid, bool show )
{
    wxString PaneName;

    switch( panelid )
    {
        case guPANEL_PODCASTS_CHANNELS :
            PaneName = wxT( "PodcastsChannels" );
            break;

        case guPANEL_PODCASTS_DETAILS :
            PaneName = wxT( "PodcastsDetails" );
            break;

        default :
            return;

    }

    wxAuiPaneInfo &PaneInfo = m_AuiManager.GetPane( PaneName );
    if( PaneInfo.IsOk() )
    {
        if( show )
            PaneInfo.Show();
        else
            PaneInfo.Hide();

        m_AuiManager.Update();
    }

    if( show )
        m_VisiblePanels |= panelid;
    else
        m_VisiblePanels ^= panelid;

    guLogMessage( wxT( "Id: %i Pane: %s Show:%i  Flags:%08X" ), panelid, PaneName.c_str(), show, m_VisiblePanels );
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::OnPaneClose( wxAuiManagerEvent &event )
{
    wxAuiPaneInfo * PaneInfo = event.GetPane();
    wxString PaneName = PaneInfo->name;
    int CmdId = 0;

    if( PaneName == wxT( "PodcastsChannels" ) )
    {
        CmdId = ID_MENU_VIEW_POD_CHANNELS;
    }
    else if( PaneName == wxT( "PodcastsDetails" ) )
    {
        CmdId = ID_MENU_VIEW_POD_DETAILS;
    }

    guLogMessage( wxT( "OnPaneClose: %s  %i" ), PaneName.c_str(), CmdId );
    if( CmdId )
    {
        wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, CmdId );
        AddPendingEvent( evt );
    }

    event.Veto();
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::SelectPodcast( const int podcastid )
{
    m_ChannelsListBox->SetSelection( wxNOT_FOUND );
    m_PodcastsListBox->SetSelection( m_PodcastsListBox->FindItem( podcastid ) );
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::SelectChannel( const int channelid )
{
    m_ChannelsListBox->SetSelection( wxNOT_FOUND );
    m_ChannelsListBox->SetSelection( m_ChannelsListBox->FindItem( channelid ) );
}

// -------------------------------------------------------------------------------- //
// guChannelsListBox
// -------------------------------------------------------------------------------- //
void guChannelsListBox::GetItemsList( void )
{
    m_PodChannels.Empty();
    int Index;
    int Count;
    Count = m_Db->GetPodcastChannels( &m_PodChannels );
    for( Index = 0; Index < Count; Index++ )
    {
        m_Items->Add( new guListItem( m_PodChannels[ Index ].m_Id, m_PodChannels[ Index ].m_Title ) );
    }
}

// -------------------------------------------------------------------------------- //
int guChannelsListBox::GetSelectedSongs( guTrackArray * Songs ) const
{
    return 0;
}

// -------------------------------------------------------------------------------- //
int guChannelsListBox::GetDragFiles( wxFileDataObject * files )
{
    return m_Db->GetPodcastFiles( GetSelectedItems(), files );
}

// -------------------------------------------------------------------------------- //
void guChannelsListBox::CreateContextMenu( wxMenu * Menu ) const
{
    wxMenuItem * MenuItem;
    int SelCount = GetSelectedItems().Count();

    MenuItem = new wxMenuItem( Menu, ID_PODCASTS_CHANNEL_ADD, _( "New Channel" ), _( "Add a new podcast channel" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_add ) );
    Menu->Append( MenuItem );

    if( SelCount )
    {
        MenuItem = new wxMenuItem( Menu, ID_PODCASTS_CHANNEL_DEL, _( "Delete" ), _( "delete this podcast channels and all its items" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_del ) );
        Menu->Append( MenuItem );

//        MenuItem = new wxMenuItem( Menu, ID_PODCASTS_CHANNEL_UNDELETE, _( "Undelete" ), _( "Show all deleted podcasts of the selected channels" ) );
//        //MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_copy ) );
//        Menu->Append( MenuItem );

        Menu->AppendSeparator();
        MenuItem = new wxMenuItem( Menu, ID_PODCASTS_CHANNEL_UPDATE, _( "Update" ), _( "Update the podcast items of the selected channels" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_doc_save ) );
        Menu->Append( MenuItem );

        if( SelCount == 1 )
        {
            MenuItem = new wxMenuItem( Menu, ID_PODCASTS_CHANNEL_PROPERTIES, _( "Properties" ), _( "Edit the podcast channel" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit ) );
            Menu->Append( MenuItem );
        }

        Menu->AppendSeparator();
        MenuItem = new wxMenuItem( Menu, ID_PODCASTS_CHANNEL_COPYTO, _( "Copy to..." ), _( "Copy the current selected podcasts to a directory or device" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_copy ) );
        Menu->Append( MenuItem );
    }
}

// -------------------------------------------------------------------------------- //
int guChannelsListBox::FindItem( const int channelid )
{
    int Index;
    int Count = m_Items->Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( m_Items->Item( Index ).m_Id == channelid )
        {
            return Index;
        }
    }
    return wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
// guPodcastListBox
// -------------------------------------------------------------------------------- //
guPodcastListBox::guPodcastListBox( wxWindow * parent, guDbLibrary * db ) :
    guListView( parent, wxLB_MULTIPLE | guLISTVIEW_COLUMN_SELECT | guLISTVIEW_COLUMN_SORTING | guLISTVIEW_ALLOWDRAG )
{
    m_Db = db;

    guConfig * Config = ( guConfig * ) guConfig::Get();

    m_ColumnNames.Add( _( "Status" ) );
    m_ColumnNames.Add( _( "Title" ) );
    m_ColumnNames.Add( _( "Channel" ) );
    m_ColumnNames.Add( _( "Category" ) );
    m_ColumnNames.Add( _( "Date" ) );
    m_ColumnNames.Add( _( "Length" ) );
    m_ColumnNames.Add( _( "Author" ) );
    m_ColumnNames.Add( _( "PlayCount" ) );
    m_ColumnNames.Add( _( "LastPlay" ) );
    m_ColumnNames.Add( _( "Added" ) );

    m_Order = Config->ReadNum( wxT( "Order" ), 0, wxT( "Podcasts" ) );
    m_OrderDesc = Config->ReadNum( wxT( "OrderDesc" ), false, wxT( "Podcasts" ) );

    // Construct the images for the status
    m_Images[ guPODCAST_STATUS_NORMAL ] = NULL;
    m_Images[ guPODCAST_STATUS_PENDING ] = new wxImage( guImage( guIMAGE_INDEX_tiny_status_pending ) );
    m_Images[ guPODCAST_STATUS_DOWNLOADING ] = new wxImage( guImage( guIMAGE_INDEX_tiny_doc_save ) );
    m_Images[ guPODCAST_STATUS_READY ] = new wxImage( guImage( guIMAGE_INDEX_tiny_accept ) );
    m_Images[ guPODCAST_STATUS_DELETED ] = new wxImage( guImage( guIMAGE_INDEX_tiny_status_error ) );
    m_Images[ guPODCAST_STATUS_ERROR ] = new wxImage( guImage( guIMAGE_INDEX_tiny_status_error ) );

    int ColId;
    wxString ColName;
    int index;
    int count = m_ColumnNames.Count();
    for( index = 0; index < count; index++ )
    {
        ColId = Config->ReadNum( wxString::Format( wxT( "PodcastsCol%u" ), index ), index, wxT( "PodcastsColumns" ) );

        ColName = m_ColumnNames[ ColId ];

        ColName += ( ( ColId == m_Order ) ? ( m_OrderDesc ? wxT( " ▼" ) : wxT( " ▲" ) ) : wxEmptyString );

        guListViewColumn * Column = new guListViewColumn(
            ColName,
            ColId,
            Config->ReadNum( wxString::Format( wxT( "PodcastsColWidth%u" ), index ), 80, wxT( "PodcastsColumns" ) ),
            Config->ReadBool( wxString::Format( wxT( "PodcastsColShow%u" ), index ), true, wxT( "PodcastsColumns" ) )
            );
        InsertColumn( Column );
    }

    ReloadItems();
}


// -------------------------------------------------------------------------------- //
guPodcastListBox::~guPodcastListBox()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    int index;
    int count = m_ColumnNames.Count();
    for( index = 0; index < count; index++ )
    {
        Config->WriteNum( wxString::Format( wxT( "PodcastsCol%u" ), index ),
                          ( * m_Columns )[ index ].m_Id, wxT( "PodcastsColumns" ) );
        Config->WriteNum( wxString::Format( wxT( "PodcastsColWidth%u" ), index ),
                          ( * m_Columns )[ index ].m_Width, wxT( "PodcastsColumns" ) );
        Config->WriteBool( wxString::Format( wxT( "PodcastsColShow%u" ), index ),
                           ( * m_Columns )[ index ].m_Enabled, wxT( "PodcastsColumns" ) );
    }

    Config->WriteNum( wxT( "Order" ), m_Order, wxT( "Podcasts" ) );
    Config->WriteBool( wxT( "OrderDesc" ), m_OrderDesc, wxT( "Podcasts" ) );


    for( index = 0; index < guPODCAST_STATUS_ERROR + 1; index++ )
    {
        delete m_Images[ index ];
    }
}

// -------------------------------------------------------------------------------- //
void guPodcastListBox::DrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const
{
    if( ( * m_Columns )[ col ].m_Id == guPODCASTS_COLUMN_STATUS )
    {
        guPodcastItem * Podcast;
        Podcast = &m_PodItems[ row ];

        if( Podcast->m_Status )
        {
            dc.SetBackgroundMode( wxTRANSPARENT );
            dc.DrawBitmap( * m_Images[ Podcast->m_Status ], rect.x + 3, rect.y + 3, true );
        }
    }
    else
    {
        guListView::DrawItem( dc, rect, row, col );
    }
}

// -------------------------------------------------------------------------------- //
wxString guPodcastListBox::OnGetItemText( const int row, const int col ) const
{
    guPodcastItem * Podcast;
    Podcast = &m_PodItems[ row ];
    switch( ( * m_Columns )[ col ].m_Id )
    {
        case guPODCASTS_COLUMN_TITLE :
          return Podcast->m_Title;

        case guPODCASTS_COLUMN_CHANNEL :
          return Podcast->m_Channel;

        case guPODCASTS_COLUMN_CATEGORY :
          return Podcast->m_Category;

        case guPODCASTS_COLUMN_DATE :
        {
          wxDateTime AddedDate;
          AddedDate.Set( ( time_t ) Podcast->m_Time );
          return AddedDate.FormatDate();
          break;
        }

        case guPODCASTS_COLUMN_LENGTH :
          return LenToString( Podcast->m_Length );

        case guPODCASTS_COLUMN_AUTHOR :
          return Podcast->m_Author;

        case guPODCASTS_COLUMN_PLAYCOUNT :
          return wxString::Format( wxT( "%u" ), Podcast->m_PlayCount );

        case guPODCASTS_COLUMN_LASTPLAY :
          if( Podcast->m_LastPlay )
          {
            wxDateTime LastPlay;
            LastPlay.Set( ( time_t ) Podcast->m_LastPlay );
            return LastPlay.FormatDate();
          }
          else
            return _( "Never" );

        case guPODCASTS_COLUMN_ADDEDDATE :
          wxDateTime AddedDate;
          AddedDate.Set( ( time_t ) Podcast->m_AddedDate );
          return AddedDate.FormatDate();

    }
    return wxEmptyString;
}


// -------------------------------------------------------------------------------- //
void guPodcastListBox::GetItemsList( void )
{
    m_Db->GetPodcastItems( &m_PodItems );

    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_MAINFRAME_UPDATE_SELINFO );
    AddPendingEvent( event );
}

// -------------------------------------------------------------------------------- //
void guPodcastListBox::ReloadItems( bool reset )
{
    wxASSERT( m_Db );

    //
    wxArrayInt Selection;
    int FirstVisible = 0;

    if( reset )
    {
        SetSelection( -1 );
    }
    else
    {
        Selection = GetSelectedItems( false );
        FirstVisible = GetFirstVisibleLine();
    }

    m_PodItems.Empty();

    GetItemsList();

    SetItemCount( m_PodItems.Count() );

    if( !reset )
    {
      SetSelectedItems( Selection );
      ScrollToLine( FirstVisible );
    }
    RefreshAll();
}

// -------------------------------------------------------------------------------- //
void guPodcastListBox::CreateContextMenu( wxMenu * Menu ) const
{
    wxArrayInt Selection = GetSelectedItems();
    if( Selection.Count() )
    {
        wxMenuItem * MenuItem;
        MenuItem = new wxMenuItem( Menu, ID_PODCASTS_ITEM_PLAY, _( "Play" ), _( "Play current selected songs" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_PODCASTS_ITEM_ENQUEUE, _( "Enqueue" ), _( "Add current selected songs to playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_add ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_PODCASTS_ITEM_ENQUEUE_ASNEXT, _( "Enqueue Next" ), _( "Add current selected songs to playlist as Next Tracks" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_add ) );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, ID_PODCASTS_ITEM_DEL, _( "Delete" ), _( "Delete the current selected podcasts" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_del ) );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, ID_PODCASTS_ITEM_DOWNLOAD, _( "Download" ), _( "Download the current selected podcasts" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_doc_save ) );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();
        MenuItem = new wxMenuItem( Menu, ID_PODCASTS_ITEM_COPYTO, _( "Copy to..." ), _( "Copy the current selected podcasts to a directory or device" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_copy ) );
        Menu->Append( MenuItem );
    }
    else
    {
        wxMenuItem * MenuItem;
        MenuItem = new wxMenuItem( Menu, wxID_ANY, _( "No selected items..." ), _( "Copy the current selected podcasts to a directory or device" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_status_error ) );
        Menu->Append( MenuItem );
    }
}

// -------------------------------------------------------------------------------- //
int inline guPodcastListBox::GetItemId( const int row ) const
{
    return m_PodItems[ row ].m_Id;
}

// -------------------------------------------------------------------------------- //
wxString inline guPodcastListBox::GetItemName( const int row ) const
{
    return m_PodItems[ row ].m_Title;
}

// -------------------------------------------------------------------------------- //
void guPodcastListBox::SetOrder( int columnid )
{
    if( m_Order != columnid )
    {
        m_Order = columnid;
        m_OrderDesc = ( columnid != 0 );
    }
    else
        m_OrderDesc = !m_OrderDesc;

    m_Db->SetPodcastOrder( m_Order );

    int CurColId;
    int index;
    int count = m_ColumnNames.Count();
    for( index = 0; index < count; index++ )
    {
        CurColId = GetColumnId( index );
        SetColumnLabel( index,
            m_ColumnNames[ CurColId ]  + ( ( CurColId == m_Order ) ?
                ( m_OrderDesc ? wxT( " ▼" ) : wxT( " ▲" ) ) : wxEmptyString ) );
    }

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
int guPodcastListBox::GetSelectedSongs( guTrackArray * tracks ) const
{
    wxArrayInt Selection = GetSelectedItems();
    int Index;
    int Count = Selection.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guPodcastItem PodcastItem;
        if( ( m_Db->GetPodcastItemId( Selection[ Index ], &PodcastItem ) != wxNOT_FOUND ) &&
            ( PodcastItem.m_Status == guPODCAST_STATUS_READY ) &&
            ( wxFileExists( PodcastItem.m_FileName ) ) )
        {
            guTrack * Track = new guTrack();
            if( Track )
            {
                Track->m_Type = guTRACK_TYPE_PODCAST;
                Track->m_SongId = PodcastItem.m_Id;
                Track->m_FileName = PodcastItem.m_FileName;
                Track->m_SongName = PodcastItem.m_Title;
                Track->m_ArtistName = PodcastItem.m_Author;
                Track->m_AlbumId = PodcastItem.m_ChId;
                Track->m_AlbumName = PodcastItem.m_Channel;
                Track->m_Length = PodcastItem.m_Length;
                Track->m_PlayCount = PodcastItem.m_PlayCount;
                Track->m_GenreName = wxT( "Podcasts" );
                Track->m_Number = Index;
                Track->m_Rating = -1;
                Track->m_CoverId = 0;
                Track->m_Year = 0; // Get year from item date
                tracks->Add( Track );
            }
        }
    }
    return tracks->Count();
}

// -------------------------------------------------------------------------------- //
int guPodcastListBox::GetDragFiles( wxFileDataObject * files )
{
    guTrackArray Songs;
    int index;
    int count = GetSelectedSongs( &Songs );
    for( index = 0; index < count; index++ )
    {
       files->AddFile( Songs[ index ].m_FileName );
    }
    return count;
}

// -------------------------------------------------------------------------------- //
int guPodcastListBox::FindItem( const int podcastid )
{
    int Index;
    int Count = m_PodItems.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( m_PodItems[ Index ].m_Id == podcastid )
        {
            return Index;
        }
    }
    return wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
