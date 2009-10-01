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
#include "PodcastsPanel.h"

#include "Commands.h"
#include "Config.h"
#include "Images.h"
#include "Utils.h"

#include <wx/curl/http.h>
#include <wx/regex.h>
#include <wx/zstream.h>
#include <wx/xml/xml.h>
#include <wx/sstream.h>

// -------------------------------------------------------------------------------- //
guPodcastPanel::guPodcastPanel( wxWindow * parent, DbLibrary * db, guPlayerPanel * playerpanel ) :
    wxPanel( parent, wxID_ANY, wxDefaultPosition, wxSize( 672,586 ), wxTAB_TRAVERSAL )
{
    m_Db = db;
    m_PlayerPanel = playerpanel;

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


    // Check that the directory to store podcasts are created
    if( !wxDirExists( wxGetHomeDir() + wxT( "/.guayadeque/Podcasts/" ) ) )
    {
        wxMkdir( wxGetHomeDir() + wxT( "/.guayadeque/Podcasts" ), 0770 );
    }
    if( !wxDirExists( wxGetHomeDir() + wxT( "/.guayadeque/Podcasts/Images" ) ) )
    {
        wxMkdir( wxGetHomeDir() + wxT( "/.guayadeque/Podcasts/Images" ), 0770 );
    }

	wxBoxSizer* MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

	m_MainSplitter = new wxSplitterWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D );
	m_MainSplitter->SetMinimumPaneSize( 150 );

    wxPanel * TopPanel;
	TopPanel = new wxPanel( m_MainSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* TopPanelSizer;
	TopPanelSizer = new wxBoxSizer( wxVERTICAL );

	m_TopSplitter = new wxSplitterWindow( TopPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D );
	m_TopSplitter->SetMinimumPaneSize( 150 );
	ChannelsPanel = new wxPanel( m_TopSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* ChannelsMainSizer;
	ChannelsMainSizer = new wxBoxSizer( wxVERTICAL );

	m_ChannelsListBox = new guChannelsListBox( ChannelsPanel, m_Db, _( "Channels" ) );
	ChannelsMainSizer->Add( m_ChannelsListBox, 1, wxEXPAND|wxALL, 1 );

	ChannelsPanel->SetSizer( ChannelsMainSizer );
	ChannelsPanel->Layout();
	ChannelsMainSizer->Fit( ChannelsPanel );
	PodcastsPanel = new wxPanel( m_TopSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* MainPodcastsSizer;
	MainPodcastsSizer = new wxBoxSizer( wxVERTICAL );

	m_PodcastsListBox = new guPodcastListBox( PodcastsPanel, m_Db );
	MainPodcastsSizer->Add( m_PodcastsListBox, 1, wxALL|wxEXPAND, 1 );

	PodcastsPanel->SetSizer( MainPodcastsSizer );
	PodcastsPanel->Layout();
	MainPodcastsSizer->Fit( PodcastsPanel );
	m_TopSplitter->SplitVertically( ChannelsPanel, PodcastsPanel, 264 );
	TopPanelSizer->Add( m_TopSplitter, 1, wxEXPAND, 5 );

	TopPanel->SetSizer( TopPanelSizer );
	TopPanel->Layout();
	TopPanelSizer->Fit( TopPanel );

	wxPanel * BottomPanel;
	BottomPanel = new wxPanel( m_MainSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );

	m_DetailMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* DetailSizer;
	DetailSizer = new wxStaticBoxSizer( new wxStaticBox( BottomPanel, wxID_ANY, wxT(" Details ") ), wxVERTICAL );

	m_DetailScrolledWindow = new wxScrolledWindow( BottomPanel, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), wxHSCROLL );
	m_DetailScrolledWindow->SetScrollRate( 5, 5 );
	m_DetailScrolledWindow->SetMinSize( wxSize( -1,100 ) );

	wxFlexGridSizer* DetailFlexGridSizer;
	DetailFlexGridSizer = new wxFlexGridSizer( 2, 2, 0, 0 );
	DetailFlexGridSizer->AddGrowableCol( 1 );
	DetailFlexGridSizer->SetFlexibleDirection( wxBOTH );
	DetailFlexGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_DetailImage = new wxStaticBitmap( m_DetailScrolledWindow, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize( 60,60 ), 0 );
	DetailFlexGridSizer->Add( m_DetailImage, 0, wxALL, 5 );

	m_DetailChannelTitle = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, wxT("My Garden Communications Impulse"), wxDefaultPosition, wxDefaultSize, 0 );
	m_DetailChannelTitle->Wrap( -1 );
	m_DetailChannelTitle->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	DetailFlexGridSizer->Add( m_DetailChannelTitle, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	DetailDescLabel = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, wxT("Description:"), wxDefaultPosition, wxDefaultSize, 0 );
	DetailDescLabel->Wrap( -1 );
	DetailDescLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );

	DetailFlexGridSizer->Add( DetailDescLabel, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_DetailDescText = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, wxT("This is the first in a serie of podcasts that treats the impulse of the communication in the collapse of the industrial economy in the my garden."), wxDefaultPosition, wxDefaultSize, 0 );
	m_DetailDescText->Wrap( 445 );
	DetailFlexGridSizer->Add( m_DetailDescText, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	DetailAuthorLabel = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, wxT("Author:"), wxDefaultPosition, wxDefaultSize, 0 );
	DetailAuthorLabel->Wrap( -1 );
	DetailAuthorLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );

	DetailFlexGridSizer->Add( DetailAuthorLabel, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_DetailAuthorText = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, wxT("Gerard Vaesley"), wxDefaultPosition, wxDefaultSize, 0 );
	m_DetailAuthorText->Wrap( -1 );
	DetailFlexGridSizer->Add( m_DetailAuthorText, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	DetailOwnerLabel = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, wxT("Owner:"), wxDefaultPosition, wxDefaultSize, 0 );
	DetailOwnerLabel->Wrap( -1 );
	DetailOwnerLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );

	DetailFlexGridSizer->Add( DetailOwnerLabel, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_DetailOwnerText = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, wxT("VRT Records S.A."), wxDefaultPosition, wxDefaultSize, 0 );
	m_DetailOwnerText->Wrap( -1 );
	DetailFlexGridSizer->Add( m_DetailOwnerText, 0, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

//	DetailEmailLabel = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, wxT("email:"), wxDefaultPosition, wxDefaultSize, 0 );
//	DetailEmailLabel->Wrap( -1 );
//	DetailEmailLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );
//
//	DetailFlexGridSizer->Add( DetailEmailLabel, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );
//
//	m_DetailEmailText = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, wxT("info@gerard.vrt.com"), wxDefaultPosition, wxDefaultSize, 0 );
//	m_DetailEmailText->Wrap( -1 );
//	DetailFlexGridSizer->Add( m_DetailEmailText, 0, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	DetailStaticLine1 = new wxStaticLine( m_DetailScrolledWindow, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	DetailFlexGridSizer->Add( DetailStaticLine1, 0, wxEXPAND|wxBOTTOM|wxLEFT, 5 );

	DetailStaticLine2 = new wxStaticLine( m_DetailScrolledWindow, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	DetailFlexGridSizer->Add( DetailStaticLine2, 0, wxEXPAND|wxBOTTOM|wxRIGHT, 5 );

	DetailItemTitleLabel = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, wxT("Title:"), wxDefaultPosition, wxDefaultSize, 0 );
	DetailItemTitleLabel->Wrap( -1 );
	DetailItemTitleLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );

	DetailFlexGridSizer->Add( DetailItemTitleLabel, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_DetailItemTitleText = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, wxT("First Podcast ever"), wxDefaultPosition, wxDefaultSize, 0 );
	m_DetailItemTitleText->Wrap( -1 );
	DetailFlexGridSizer->Add( m_DetailItemTitleText, 0, wxBOTTOM|wxRIGHT, 5 );

	DetailItemSumaryLabel = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, wxT("Sumary:"), wxDefaultPosition, wxDefaultSize, 0 );
	DetailItemSumaryLabel->Wrap( -1 );
	DetailItemSumaryLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );

	DetailFlexGridSizer->Add( DetailItemSumaryLabel, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_DetailItemSumaryText = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, wxT("This is the first podcast that have been done in this server... It will be available more soon"), wxDefaultPosition, wxDefaultSize, 0 );
	m_DetailItemSumaryText->Wrap( 300 );
	DetailFlexGridSizer->Add( m_DetailItemSumaryText, 0, wxBOTTOM|wxRIGHT|wxEXPAND, 5 );

	DetailItemDateLabel = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, wxT("Date:"), wxDefaultPosition, wxDefaultSize, 0 );
	DetailItemDateLabel->Wrap( -1 );
	DetailItemDateLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );

	DetailFlexGridSizer->Add( DetailItemDateLabel, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_DetailItemDateText = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, wxT("Mon, 11 September 2009 00:00:00 GMT"), wxDefaultPosition, wxDefaultSize, 0 );
	m_DetailItemDateText->Wrap( -1 );
	DetailFlexGridSizer->Add( m_DetailItemDateText, 0, wxBOTTOM|wxRIGHT, 5 );

	DetailItemLengthLabel = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, wxT("Length:"), wxDefaultPosition, wxDefaultSize, 0 );
	DetailItemLengthLabel->Wrap( -1 );
	DetailItemLengthLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );

	DetailFlexGridSizer->Add( DetailItemLengthLabel, 0, wxBOTTOM|wxRIGHT|wxLEFT|wxALIGN_RIGHT, 5 );

	m_DetailItemLengthText = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, wxT("04:15"), wxDefaultPosition, wxDefaultSize, 0 );
	m_DetailItemLengthText->Wrap( -1 );
	DetailFlexGridSizer->Add( m_DetailItemLengthText, 0, wxBOTTOM|wxRIGHT, 5 );

	m_DetailScrolledWindow->SetSizer( DetailFlexGridSizer );
	m_DetailScrolledWindow->Layout();
	DetailFlexGridSizer->Fit( m_DetailScrolledWindow );
	DetailSizer->Add( m_DetailScrolledWindow, 1, wxEXPAND | wxALL, 5 );

	m_DetailMainSizer->Add( DetailSizer, 1, wxEXPAND|wxALL, 5 );

	BottomPanel->SetSizer( m_DetailMainSizer );
	BottomPanel->Layout();
	DetailSizer->Fit( BottomPanel );
	m_MainSplitter->SplitHorizontally( TopPanel, BottomPanel, 300 );
	MainSizer->Add( m_MainSplitter, 1, wxEXPAND, 5 );

	this->SetSizer( MainSizer );
	this->Layout();


	Connect( wxEVT_SIZE, wxSizeEventHandler( guPodcastPanel::OnChangedSize ) );
	m_MainSplitter->Connect( wxEVT_IDLE, wxIdleEventHandler( guPodcastPanel::MainSplitterOnIdle ), NULL, this );
//	m_MainSplitter->Connect( wxEVT_IDLE, wxIdleEventHandler( guPodcastPanel::m_MainSplitterOnIdle ), NULL, this );
//	m_TopSplitter->Connect( wxEVT_IDLE, wxIdleEventHandler( guPodcastPanel::m_TopSplitterOnIdle ), NULL, this );
    Connect( ID_PODCASTS_ADD, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPodcastPanel::AddChannel ) );

    m_ChannelsListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED,  wxListEventHandler( guPodcastPanel::OnChannelsSelected ), NULL, this );
	m_PodcastsListBox->Connect( wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler( guPodcastPanel::OnPodcastsColClick ), NULL, this );
    m_PodcastsListBox->Connect( wxEVT_COMMAND_LISTBOX_SELECTED,  wxListEventHandler( guPodcastPanel::OnPodcastItemSelected ), NULL, this );
}

// -------------------------------------------------------------------------------- //
guPodcastPanel::~guPodcastPanel()
{
    // Save the Splitter positions into the main config
    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        Config->WriteNum( wxT( "PodcastsMainSashPos" ), m_MainSplitter->GetSashPosition(), wxT( "Positions" ) );
        Config->WriteNum( wxT( "PodcastsTopSashPos" ), m_TopSplitter->GetSashPosition(), wxT( "Positions" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::OnChangedSize( wxSizeEvent& event )
{
	event.Skip();
//    wxSize Size = event.GetSize();
//	m_DetailDescText->Wrap( Size.GetWidth() - 100 );
//	m_DetailItemSumaryText->Wrap( Size.GetWidth() - 100 );
//	Layout();
}

// -------------------------------------------------------------------------------- //
int StrLengthToInt( const wxString &length )
{
    int RetVal = 0;
    int Factor[] = { 1, 60, 3600, 86400 };
    int FactorIndex = 0;
    // 1:02:03:04
    wxString Rest = length.Strip( wxString::both );
    int element;
    do {
        Rest.AfterLast( wxT( ':' ) ).ToLong( ( long * ) &element );
        if( !element )
            break;
        RetVal += Factor[ FactorIndex ] * element;
        FactorIndex++;
        if( ( FactorIndex > 3 ) )
            break;
        Rest = Rest.BeforeLast( wxT( ':' ) );
    } while( !Rest.IsEmpty() );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void ReadXmlPodcastOwner( wxXmlNode * XmlNode, guPodcastChannel * channel )
{
    while( XmlNode )
    {
        if( XmlNode->GetName() == wxT( "itunes:name" ) )
        {
            channel->m_OwnerName = XmlNode->GetNodeContent();
        }
        else if( XmlNode->GetName() == wxT( "itunes:email" ) )
        {
            channel->m_OwnerEmail = XmlNode->GetNodeContent();
        }
        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void ReadXmlPodcastItem( wxXmlNode * XmlNode, guPodcastItem * item )
{
    while( XmlNode )
    {
        if( XmlNode->GetName() == wxT( "title" ) )
        {
            item->m_Title = XmlNode->GetNodeContent();
            guLogMessage( wxT( "Item: '%s'" ), item->m_Title.c_str() );
        }
        else if( XmlNode->GetName() == wxT( "enclosure" ) )
        {
            XmlNode->GetPropVal( wxT( "url" ), &item->m_Enclosure );
        }
        else if( XmlNode->GetName() == wxT( "itunes:summary" ) )
        {
            item->m_Summary= XmlNode->GetNodeContent();
        }
        else if( XmlNode->GetName() == wxT( "pubDate" ) )
        {
            wxDateTime DateTime;
            DateTime.ParseRfc822Date( XmlNode->GetNodeContent() );
            item->m_Time = DateTime.GetTicks();
        }
        else if( XmlNode->GetName() == wxT( "itunes:duration" ) )
        {
            item->m_Length = StrLengthToInt( XmlNode->GetNodeContent() );
        }
        else if( XmlNode->GetName() == wxT( "itunes:author" ) )
        {
            item->m_Author = XmlNode->GetNodeContent();
        }
        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void ReadXmlPodcastChannel( wxXmlNode * XmlNode, guPodcastChannel * channel )
{
    if( XmlNode && XmlNode->GetName() == wxT( "channel" ) )
    {
        XmlNode = XmlNode->GetChildren();
        while( XmlNode )
        {
            if( XmlNode->GetName() == wxT( "title" ) )
            {
                channel->m_Title = XmlNode->GetNodeContent();
            }
            else if( XmlNode->GetName() == wxT( "link" ) )
            {
                channel->m_Link = XmlNode->GetNodeContent();
            }
            else if( XmlNode->GetName() == wxT( "language" ) )
            {
                channel->m_Lang = XmlNode->GetNodeContent();
            }
            else if( XmlNode->GetName() == wxT( "description" ) )
            {
                channel->m_Description = XmlNode->GetNodeContent();
            }
            else if( XmlNode->GetName() == wxT( "itunes:author" ) )
            {
                channel->m_Author = XmlNode->GetNodeContent();
            }
            else if( XmlNode->GetName() == wxT( "itunes:owner" ) )
            {
                ReadXmlPodcastOwner( XmlNode->GetChildren(), channel );
            }
            else if( XmlNode->GetName() == wxT( "itunes:image" ) )
            {
                XmlNode->GetPropVal( wxT( "href" ), &channel->m_Image );
            }
            else if( XmlNode->GetName() == wxT( "itunes:category" ) )
            {
                XmlNode->GetPropVal( wxT( "text" ), &channel->m_Category );
            }
            else if( XmlNode->GetName() == wxT( "itunes:summary" ) )
            {
                channel->m_Summary = XmlNode->GetNodeContent();
            }
            else if( XmlNode->GetName() == wxT( "item" ) )
            {
                guPodcastItem * PodcastItem = new guPodcastItem();
                ReadXmlPodcastItem( XmlNode->GetChildren(), PodcastItem );
                channel->m_Items.Add( PodcastItem );
            }
            XmlNode = XmlNode->GetNext();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::AddChannel( wxCommandEvent &event )
{
    wxTextEntryDialog * EntryDialog = new wxTextEntryDialog( this, _( "Channel Url: " ), _( "Please enter the channel url" ) );
    if( EntryDialog->ShowModal() == wxID_OK )
    {
        wxCurlHTTP  http;

        guLogMessage( wxT( "The address is %s" ), EntryDialog->GetValue().c_str() );

        wxString Content;

        http.AddHeader( wxT( "User-Agent: Mozilla/5.0 (X11; U; Linux i686; es-ES; rv:1.9.0.5) Gecko/2008121622 Ubuntu/8.10 (intrepid) Firefox/3.0.5" ) );
        http.AddHeader( wxT( "Accept: */*" ) );
        http.AddHeader( wxT( "Accept-Charset: utf-8;iso-8859-1" ) );
        char * Buffer = NULL;
        http.Get( Buffer, EntryDialog->GetValue() );
        if( Buffer )
        {
            wxMemoryInputStream ins( Buffer, Strlen( Buffer ) );
            wxXmlDocument XmlDoc( ins );
            //wxSt
            wxXmlNode * XmlNode = XmlDoc.GetRoot();
            if( XmlNode && XmlNode->GetName() == wxT( "rss" ) )
            {
                guPodcastChannel PodcastChannel;
                PodcastChannel.m_Url = EntryDialog->GetValue();
                ReadXmlPodcastChannel( XmlNode->GetChildren(), &PodcastChannel );

                m_Db->SavePodcastChannel( &PodcastChannel );
                m_ChannelsListBox->ReloadItems();
            }

            free( Buffer );
        }
        else
        {
            guLogError( wxT( "Could not get podcast content for %s" ), EntryDialog->GetValue().c_str() );
        }
    }
    EntryDialog->Destroy();
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::OnChannelsSelected( wxListEvent &event )
{
    m_Db->SetPodcastChannelFilters( m_ChannelsListBox->GetSelectedItems() );
    m_PodcastsListBox->ReloadItems();
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::OnPodcastsColClick( wxListEvent &event )
{
    int col = event.GetColumn();
    if( col < 0 )
        return;
    m_PodcastsListBox->SetOrder( col );
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::OnPodcastItemSelected( wxListEvent &event )
{
    wxArrayInt Selection = m_PodcastsListBox->GetSelectedItems();
    UpdateChannelInfo( Selection.Count() ? Selection[ 0 ] : -1 );
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::UpdateChannelInfo( int itemid )
{
    guPodcastChannel    PodcastChannel;
    guPodcastItem       PodcastItem;
    //
    guLogMessage( wxT( "Updating the info of the item %u" ), itemid );
    if( itemid > 0 )
    {
//    int                 m_Id;
//    wxString            m_Url;
//    wxString            m_Title;
//    wxString            m_Link;
//    wxString            m_Description;
//    wxString            m_Lang;
//    wxString            m_Summary;
//    wxString            m_Category;
//    wxString            m_Image;
//    wxString            m_Author;
//    wxString            m_OwnerName;
//    wxString            m_OwnerEmail;
        m_Db->GetPodcastItemId( itemid, &PodcastItem );
        m_Db->GetPodcastChannelId( PodcastItem.m_ChId, &PodcastChannel );
        guLogMessage( wxT( "PodcastChannel:\n"
            "Id             : %u\n"
            "Url            : %s\n"
            "Title          : %s\n"
            "Link           : %s\n"
            "Description    : %s\n"
            "Lang           : %s\n"
            "Summary        : %s\n"
            "Category       : %s\n"
            "Image          : %s\n"
            "Author         : %s\n"
            "OwnerName      : %s\n"
            "OwnerEmail     : %s\n" ),
            PodcastChannel.m_Id,
            PodcastChannel.m_Url.c_str(),
            PodcastChannel.m_Title.c_str(),
            PodcastChannel.m_Link.c_str(),
            PodcastChannel.m_Description.c_str(),
            PodcastChannel.m_Lang.c_str(),
            PodcastChannel.m_Summary.c_str(),
            PodcastChannel.m_Category.c_str(),
            PodcastChannel.m_Image.c_str(),
            PodcastChannel.m_Author.c_str(),
            PodcastChannel.m_OwnerName.c_str(),
            PodcastChannel.m_OwnerEmail.c_str() );


        //m_DetailImage->
        m_DetailChannelTitle->SetLabel( PodcastChannel.m_Title );
        m_DetailDescText->SetLabel( PodcastChannel.m_Description );
        m_DetailAuthorText->SetLabel( PodcastChannel.m_Author );
        m_DetailOwnerText->SetLabel( PodcastChannel.m_OwnerName +
                                     wxT( " (" + PodcastChannel.m_OwnerEmail + wxT( ")" ) ) );
        //m_DetailEmailText->SetLabel( PodcastChannel.m_OwnerEmail );
        m_DetailItemTitleText->SetLabel( PodcastItem.m_Title );
        m_DetailItemSumaryText->SetLabel( PodcastItem.m_Summary );
        wxDateTime AddedDate;
        AddedDate.Set( ( time_t ) PodcastItem.m_Time );
        m_DetailItemDateText->SetLabel( AddedDate.FormatDate() );
        m_DetailItemLengthText->SetLabel( LenToString( PodcastItem.m_Length ) );
    }
    else
    {
        m_DetailImage->SetBitmap( guBitmap( guIMAGE_INDEX_podcast_icon ) );
//        wxStaticBitmap *    m_DetailImage;
        m_DetailChannelTitle->SetLabel( wxEmptyString );
        m_DetailDescText->SetLabel( wxEmptyString );
        m_DetailAuthorText->SetLabel( wxEmptyString );
        m_DetailOwnerText->SetLabel( wxEmptyString );
        //m_DetailEmailText->SetLabel( wxEmptyString );
        m_DetailItemTitleText->SetLabel( wxEmptyString );
        m_DetailItemSumaryText->SetLabel( wxEmptyString );
        m_DetailItemDateText->SetLabel( wxEmptyString );
        m_DetailItemLengthText->SetLabel( wxEmptyString );
    }
//    wxSize Size = GetSize();
//	m_DetailDescText->Wrap( Size.GetWidth() - 70 );
//	//m_DetailItemTitleText->Wrap( Size.GetWidth() - 70 );
//	m_DetailItemSumaryText->Wrap( Size.GetWidth() - 70 );
//
    m_DetailMainSizer->FitInside( m_DetailScrolledWindow );
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::MainSplitterOnIdle( wxIdleEvent &event )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    m_MainSplitter->SetSashPosition( Config->ReadNum( wxT( "PodcastsMainSashPos" ), 150, wxT( "Positions" ) ) );
    m_MainSplitter->Disconnect( wxEVT_IDLE, wxIdleEventHandler( guPodcastPanel::MainSplitterOnIdle ), NULL, this );
    m_TopSplitter->SetSashPosition( Config->ReadNum( wxT( "PodcastsTopSashPos" ), 150, wxT( "Positions" ) ) );
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
    //return m_Db->GetGenresSongs( GetSelectedItems(), Songs );
}

// -------------------------------------------------------------------------------- //
void guChannelsListBox::CreateContextMenu( wxMenu * Menu ) const
{
    wxMenuItem * MenuItem;
    int SelCount = GetSelectedItems().Count();

    MenuItem = new wxMenuItem( Menu, ID_PODCASTS_ADD, _( "New Channel" ), _( "Add a new podcast channel" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_add ) );
    Menu->Append( MenuItem );

//    MenuItem = new wxMenuItem( Menu, ID_GENRE_ENQUEUE, _( "Enqueue" ), _( "Add current selected genres to playlist" ) );
//    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_add ) );
//    Menu->Append( MenuItem );
//
//    if( SelCount )
//    {
//        Menu->AppendSeparator();
//
//        MenuItem = new wxMenuItem( Menu, ID_GENRE_COPYTO, _( "Copy to..." ), _( "Copy the current selected songs to a directory or device" ) );
//        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit_copy ) );
//        Menu->Append( MenuItem );
//    }
}

// -------------------------------------------------------------------------------- //
// guPodcastListBox
// -------------------------------------------------------------------------------- //
wxString guPODCASTS_COLUMN_NAMES[] = {
    _( "Title" ),
    _( "Channel" ),
    _( "Category" ),
    _( "Date" ),
    _( "Length" ),
    _( "Author" ),
    _( "PlayCount" ),
    _( "LastPlay" ),
    _( "Status" )
};

// -------------------------------------------------------------------------------- //
guPodcastListBox::guPodcastListBox( wxWindow * parent, DbLibrary * db ) :
    guListView( parent, wxLB_SINGLE )
{
    m_Db = db;

    guConfig * Config = ( guConfig * ) guConfig::Get();

    m_Order = Config->ReadNum( wxT( "Order" ), 0, wxT( "Podcasts" ) );
    m_OrderDesc = Config->ReadNum( wxT( "OrderDesc" ), false, wxT( "Podcasts" ) );

    // Create the Columns
    int ColId;
    int index;
    int count = sizeof( guPODCASTS_COLUMN_NAMES ) / sizeof( wxString );
    for( index = 0; index < count; index++ )
    {
        guListViewColumn * Column = new guListViewColumn(
            guPODCASTS_COLUMN_NAMES[ index ] + ( ( index == m_Order ) ? ( m_OrderDesc ? wxT( " ▼" ) : wxT( " ▲" ) ) : wxEmptyString ),
            index,
            Config->ReadNum( wxString::Format( wxT( "PodcastsColSize%u" ), index ), 80, wxT( "Positions" ) )
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
    int count = sizeof( guPODCASTS_COLUMN_NAMES ) / sizeof( wxString );
    for( index = 0; index < count; index++ )
    {
        Config->WriteNum( wxString::Format( wxT( "PodcastsColSize%u" ), index ), GetColumnWidth( index ), wxT( "Positions" ) );
    }

    Config->WriteNum( wxT( "Order" ), m_Order, wxT( "Podcasts" ) );
    Config->WriteBool( wxT( "OrderDesc" ), m_OrderDesc, wxT( "Podcasts" ) );
}

// -------------------------------------------------------------------------------- //
wxString guPodcastListBox::OnGetItemText( const int row, const int col ) const
{
    guPodcastItem * Podcast;
    Podcast = &m_PodItems[ row ];
    switch( col )
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

        case guPODCASTS_COLUMN_STATUS :
            return wxEmptyString;
    }
    return wxEmptyString;
}


// -------------------------------------------------------------------------------- //
void guPodcastListBox::GetItemsList( void )
{
    m_Db->GetPodcastItems( &m_PodItems );
}

// -------------------------------------------------------------------------------- //
void guPodcastListBox::ReloadItems( bool reset )
{
    wxASSERT( m_Db );

    //
    wxArrayInt Selection;
    int FirstVisible = GetFirstVisibleLine();

    if( reset )
        SetSelection( -1 );
    else
        Selection = GetSelectedItems( false );

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
    wxMenuItem * MenuItem;
    MenuItem = new wxMenuItem( Menu, ID_RADIO_PLAY, _( "Play" ), _( "Play current selected songs" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_playback_start ) );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, ID_RADIO_ENQUEUE, _( "Enqueue" ), _( "Add current selected songs to playlist" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_add ) );
    Menu->Append( MenuItem );

    Menu->AppendSeparator();

    MenuItem = new wxMenuItem( Menu, ID_RADIO_EDIT_LABELS, _( "Edit Labels" ), _( "Edit the labels assigned to the selected stations" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_edit ) );
    Menu->Append( MenuItem );
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
void guPodcastListBox::SetOrder( int order )
{
    if( m_Order != order )
    {
        m_Order = order;
        m_OrderDesc = ( order != 0 );
    }
    else
        m_OrderDesc = !m_OrderDesc;

    m_Db->SetPodcastOrder( m_Order );

    int index;
    int count = sizeof( guPODCASTS_COLUMN_NAMES ) / sizeof( wxString );
    for( index = 0; index < count; index++ )
    {
        SetColumnLabel( index,
            guPODCASTS_COLUMN_NAMES[ index ]  + ( ( index == m_Order ) ?
                ( m_OrderDesc ? wxT( " ▼" ) : wxT( " ▲" ) ) : wxEmptyString ) );
    }

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
