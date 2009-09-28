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
    wxStaticText * DetailEmailLabel;
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


	SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* MainSizer;
	MainSizer = new wxBoxSizer( wxVERTICAL );

	m_MainSplitter = new wxSplitterWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D );
	m_MainSplitter->SetMinimumPaneSize( 100 );
	ChannelsPanel = new wxPanel( m_MainSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* ChannelsMainSizer;
	ChannelsMainSizer = new wxBoxSizer( wxVERTICAL );

	m_ChannelsListBox = new guChannelsListBox( ChannelsPanel, m_Db, _( "Channels" ) );
	ChannelsMainSizer->Add( m_ChannelsListBox, 1, wxEXPAND|wxALL, 5 );

	ChannelsPanel->SetSizer( ChannelsMainSizer );
	ChannelsPanel->Layout();
	ChannelsMainSizer->Fit( ChannelsPanel );
	PodcastsPanel = new wxPanel( m_MainSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* MainPodcastsSizer;
	MainPodcastsSizer = new wxBoxSizer( wxVERTICAL );

	m_Podcasts = new guPodcastListBox( PodcastsPanel, m_Db, _( "Podcasts" ) );
	MainPodcastsSizer->Add( m_Podcasts, 1, wxALL|wxEXPAND, 5 );

	PodcastsPanel->SetSizer( MainPodcastsSizer );
	PodcastsPanel->Layout();
	MainPodcastsSizer->Fit( PodcastsPanel );
	m_MainSplitter->SplitVertically( ChannelsPanel, PodcastsPanel, 231 );
	MainSizer->Add( m_MainSplitter, 1, wxEXPAND, 5 );

	wxStaticBoxSizer* DetailSizer;
	DetailSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT(" Details ") ), wxVERTICAL );

	wxFlexGridSizer* DetailFlexGridSizer;
	DetailFlexGridSizer = new wxFlexGridSizer( 2, 2, 0, 0 );
	DetailFlexGridSizer->AddGrowableCol( 1 );
	DetailFlexGridSizer->SetFlexibleDirection( wxBOTH );
	DetailFlexGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_DetailImage = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize( 60,60 ), 0 );
	DetailFlexGridSizer->Add( m_DetailImage, 0, wxALL, 5 );

	m_DetailChannelTitle = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_DetailChannelTitle->Wrap( -1 );
	m_DetailChannelTitle->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	DetailFlexGridSizer->Add( m_DetailChannelTitle, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	DetailDescLabel = new wxStaticText( this, wxID_ANY, wxT("Description:"), wxDefaultPosition, wxDefaultSize, 0 );
	DetailDescLabel->Wrap( -1 );
	//DetailDescLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	DetailFlexGridSizer->Add( DetailDescLabel, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_DetailDescText = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	DetailFlexGridSizer->Add( m_DetailDescText, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	DetailAuthorLabel = new wxStaticText( this, wxID_ANY, wxT("Author:"), wxDefaultPosition, wxDefaultSize, 0 );
	DetailAuthorLabel->Wrap( -1 );
	//DetailAuthorLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	DetailFlexGridSizer->Add( DetailAuthorLabel, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_DetailAuthorText = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_DetailAuthorText->Wrap( -1 );
	DetailFlexGridSizer->Add( m_DetailAuthorText, 0, wxEXPAND|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	DetailOwnerLabel = new wxStaticText( this, wxID_ANY, wxT("Owner:"), wxDefaultPosition, wxDefaultSize, 0 );
	DetailOwnerLabel->Wrap( -1 );
	//DetailOwnerLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	DetailFlexGridSizer->Add( DetailOwnerLabel, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_DetailOwnerText = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_DetailOwnerText->Wrap( -1 );
	DetailFlexGridSizer->Add( m_DetailOwnerText, 0, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	DetailEmailLabel = new wxStaticText( this, wxID_ANY, wxT("email:"), wxDefaultPosition, wxDefaultSize, 0 );
	DetailEmailLabel->Wrap( -1 );
	//DetailEmailLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	DetailFlexGridSizer->Add( DetailEmailLabel, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_DetailEmailText = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_DetailEmailText->Wrap( -1 );
	DetailFlexGridSizer->Add( m_DetailEmailText, 0, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	DetailStaticLine1 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	DetailFlexGridSizer->Add( DetailStaticLine1, 0, wxEXPAND|wxBOTTOM|wxLEFT, 5 );

	DetailStaticLine2 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	DetailFlexGridSizer->Add( DetailStaticLine2, 0, wxEXPAND|wxBOTTOM|wxRIGHT, 5 );

	DetailItemTitleLabel = new wxStaticText( this, wxID_ANY, wxT("Title:"), wxDefaultPosition, wxDefaultSize, 0 );
	DetailItemTitleLabel->Wrap( -1 );
	//DetailItemTitleLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	DetailFlexGridSizer->Add( DetailItemTitleLabel, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_DetailItemTitleText = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_DetailItemTitleText->Wrap( -1 );
	DetailFlexGridSizer->Add( m_DetailItemTitleText, 0, wxBOTTOM|wxRIGHT, 5 );

	DetailItemSumaryLabel = new wxStaticText( this, wxID_ANY, wxT("Sumary:"), wxDefaultPosition, wxDefaultSize, 0 );
	DetailItemSumaryLabel->Wrap( -1 );
	//DetailItemSumaryLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	DetailFlexGridSizer->Add( DetailItemSumaryLabel, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_DetailItemSumaryText = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	DetailFlexGridSizer->Add( m_DetailItemSumaryText, 0, wxBOTTOM|wxRIGHT|wxEXPAND, 5 );

	DetailItemDateLabel = new wxStaticText( this, wxID_ANY, wxT("Date:"), wxDefaultPosition, wxDefaultSize, 0 );
	DetailItemDateLabel->Wrap( -1 );
	//DetailItemDateLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	DetailFlexGridSizer->Add( DetailItemDateLabel, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_DetailItemDateText = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_DetailItemDateText->Wrap( -1 );
	DetailFlexGridSizer->Add( m_DetailItemDateText, 0, wxBOTTOM|wxRIGHT, 5 );

	DetailItemLengthLabel = new wxStaticText( this, wxID_ANY, wxT("Length:"), wxDefaultPosition, wxDefaultSize, 0 );
	DetailItemLengthLabel->Wrap( -1 );
	//DetailItemLengthLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	DetailFlexGridSizer->Add( DetailItemLengthLabel, 0, wxBOTTOM|wxRIGHT|wxLEFT|wxALIGN_RIGHT, 5 );

	m_DetailItemLengthText = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_DetailItemLengthText->Wrap( -1 );
	DetailFlexGridSizer->Add( m_DetailItemLengthText, 0, wxBOTTOM|wxRIGHT, 5 );

	DetailSizer->Add( DetailFlexGridSizer, 0, wxEXPAND, 5 );

	MainSizer->Add( DetailSizer, 0, wxEXPAND|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	this->SetSizer( MainSizer );
	this->Layout();

	Connect( wxEVT_SIZE, wxSizeEventHandler( guPodcastPanel::OnChangedSize ) );
	m_MainSplitter->Connect( wxEVT_IDLE, wxIdleEventHandler( guPodcastPanel::MainSplitterOnIdle ), NULL, this );
    Connect( ID_PODCASTS_ADD, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guPodcastPanel::AddChannel ) );
}

guPodcastPanel::~guPodcastPanel()
{
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::OnChangedSize( wxSizeEvent& event )
{
	event.Skip();
    wxSize Size = event.GetSize();
	m_DetailDescText->Wrap( Size.GetWidth() );
	m_DetailItemSumaryText->Wrap( Size.GetWidth() );
	Layout();
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
            item->m_Length = XmlNode->GetNodeContent();
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
            if( http.GetResponseHeader().Find( wxT( "iso-8859-1" ) ) )
            {
                Content = wxString( Buffer, wxConvISO8859_1 );
            }
            else
            {
                Content = wxString( Buffer, wxConvUTF8 );
            }
            free( Buffer );
        }

        if( !Content.IsEmpty() )
        {
            wxStringInputStream ins( Content );
            wxXmlDocument XmlDoc( ins );
            wxXmlNode * XmlNode = XmlDoc.GetRoot();
            if( XmlNode && XmlNode->GetName() == wxT( "rss" ) )
            {
                guPodcastChannel PodcastChannel;
                PodcastChannel.m_Url = EntryDialog->GetValue();
                ReadXmlPodcastChannel( XmlNode->GetChildren(), &PodcastChannel );

                guLogMessage( wxT( "Podcast '%s'" ), PodcastChannel.m_Title.c_str() );
            }
        }
        else
        {
            guLogError( wxT( "Could not get podcast content for %s" ), EntryDialog->GetValue().c_str() );
        }
    }
    EntryDialog->Destroy();
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::MainSplitterOnIdle( wxIdleEvent &event )
{
    m_MainSplitter->SetSashPosition( 230 );
    m_MainSplitter->Disconnect( wxEVT_IDLE, wxIdleEventHandler( guPodcastPanel::MainSplitterOnIdle ), NULL, this );
}

// -------------------------------------------------------------------------------- //
// guChannelsListBox
// -------------------------------------------------------------------------------- //
void guChannelsListBox::GetItemsList( void )
{
    m_Channels.Empty();
    int Index;
    int Count;
    Count = m_Db->GetPodcastChannels( &m_Channels );
    for( Index = 0; Index < Count; Index++ )
    {
        m_Items->Add( new guListItem( m_Channels[ Index ].m_Id, m_Channels[ Index ].m_Title ) );
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
void guPodcastListBox::GetItemsList( void )
{
    //m_Db->GetGenres( m_Items );
}

// -------------------------------------------------------------------------------- //
int guPodcastListBox::GetSelectedSongs( guTrackArray * Songs ) const
{
    //return m_Db->GetGenresSongs( GetSelectedItems(), Songs );
}

// -------------------------------------------------------------------------------- //
void guPodcastListBox::CreateContextMenu( wxMenu * Menu ) const
{
//    wxMenuItem * MenuItem;
//    int SelCount = GetSelectedItems().Count();
//
//    MenuItem = new wxMenuItem( Menu, ID_GENRE_PLAY, _( "Play" ), _( "Play current selected genres" ) );
//    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_playback_start ) );
//    Menu->Append( MenuItem );
//
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
