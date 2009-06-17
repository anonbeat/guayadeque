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
#include "CoverEdit.h"

#include "Commands.h"
#include "Config.h"
#include "Images.h"
#include "MainFrame.h"
#include "Utils.h"

#include <wx/arrimpl.cpp>
#include <wx/curl/http.h>
#include <wx/statline.h>

#define GOOGLE_IMAGES_SEARCH_STR wxT( "http://images.google.com/images?imgsz=large|xlarge&q=%s&start=%u" )
#define COVERS_COUNTER_PER_PAGE 10
#define MAX_COVERLINKS_ITEMS    30

WX_DEFINE_OBJARRAY(guCoverImageArray);

// -------------------------------------------------------------------------------- //
guCoverEditor::guCoverEditor( wxWindow* parent, const wxString &Artist, const wxString &Album ) :
               wxDialog( parent, wxID_ANY, _( "Cover Editor" ), wxDefaultPosition, wxSize( -1,-1 ), wxDEFAULT_DIALOG_STYLE )
{
	wxBoxSizer *                MainSizer;
	wxBoxSizer *                SearchSizer;
	wxBoxSizer *                InputTextSizer;
	wxBoxSizer *                CoverSizer;

    wxStaticText *              SearchStaticText;
	wxPanel *                   InputTextPanel;
	wxStaticBitmap *            LogoTextBitmap;
	wxStaticLine *              TopStaticLine;
//	wxStaticLine *              BottomStaticLine;
	wxStdDialogButtonSizer *    ButtonsSizer;

	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	MainSizer = new wxBoxSizer( wxVERTICAL );

	SearchSizer = new wxBoxSizer( wxHORIZONTAL );

	SearchStaticText = new wxStaticText( this, wxID_ANY, _( "Search:" ), wxDefaultPosition, wxDefaultSize, 0 );
	SearchStaticText->Wrap( -1 );
	SearchSizer->Add( SearchStaticText, 0, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT, 5 );

	InputTextPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
	InputTextPanel->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ) );

	InputTextSizer = new wxBoxSizer( wxHORIZONTAL );

	LogoTextBitmap = new wxStaticBitmap( InputTextPanel, wxID_ANY, wxBitmap( guImage_search ), wxDefaultPosition, wxDefaultSize, 0 );
	InputTextSizer->Add( LogoTextBitmap, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0 );

	m_InputTextCtrl = new wxTextCtrl( InputTextPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER|wxNO_BORDER );
	InputTextSizer->Add( m_InputTextCtrl, 1, wxALL|wxALIGN_CENTER_VERTICAL, 2 );

	m_ClearTextBitmap = new wxStaticBitmap( InputTextPanel, wxID_ANY, wxBitmap( guImage_edit_clear ), wxDefaultPosition, wxDefaultSize, 0 );
	m_ClearTextBitmap->Enable( false );

	InputTextSizer->Add( m_ClearTextBitmap, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0 );

	InputTextPanel->SetSizer( InputTextSizer );
	InputTextPanel->Layout();
	InputTextSizer->Fit( InputTextPanel );
	SearchSizer->Add( InputTextPanel, 1, wxEXPAND|wxALL, 5 );

	MainSizer->Add( SearchSizer, 0, wxEXPAND, 5 );

	TopStaticLine = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	MainSizer->Add( TopStaticLine, 0, wxEXPAND | wxALL, 5 );

	CoverSizer = new wxBoxSizer( wxHORIZONTAL );

	m_PrevButton = new wxBitmapButton( this, wxID_ANY, wxBitmap( guImage_go_previous ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	CoverSizer->Add( m_PrevButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	m_CoverBitmap = new wxStaticBitmap( this, wxID_ANY, wxBitmap( guImage_blank_cd_cover ), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	CoverSizer->Add( m_CoverBitmap, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5 );

	m_NextButton = new wxBitmapButton( this, wxID_ANY, wxBitmap( guImage_go_next ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	CoverSizer->Add( m_NextButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );

	MainSizer->Add( CoverSizer, 1, wxEXPAND, 5 );

	m_SizeSizer = new wxBoxSizer( wxVERTICAL );
	m_SizeStaticText = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_SizeStaticText->Wrap( -1 );
	m_SizeSizer->Add( m_SizeStaticText, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5 );

	MainSizer->Add( m_SizeSizer, 0, wxEXPAND, 5 );

	wxBoxSizer * GaugeSizer;
	GaugeSizer = new wxBoxSizer( wxHORIZONTAL );

	m_Gauge = new guAutoPulseGauge( this, wxID_ANY, MAX_COVERLINKS_ITEMS, wxDefaultPosition, wxSize( -1,7 ), wxGA_HORIZONTAL );
	//m_Gauge->SetValue( 5 );
	GaugeSizer->Add( m_Gauge, 1, wxALL|wxEXPAND, 5 );

	m_InfoTextCtrl = new wxStaticText( this, wxID_ANY, wxT("00/00"), wxDefaultPosition, wxDefaultSize, 0 );
	m_InfoTextCtrl->Wrap( -1 );
	GaugeSizer->Add( m_InfoTextCtrl, 0, wxRIGHT, 5 );

	MainSizer->Add( GaugeSizer, 0, wxEXPAND, 5 );

	ButtonsSizer = new wxStdDialogButtonSizer();
	m_ButtonsSizerOK = new wxButton( this, wxID_OK );
	ButtonsSizer->AddButton( m_ButtonsSizerOK );
	m_ButtonsSizerCancel = new wxButton( this, wxID_CANCEL );
	ButtonsSizer->AddButton( m_ButtonsSizerCancel );
	ButtonsSizer->Realize();
	MainSizer->Add( ButtonsSizer, 0, wxEXPAND, 5 );

	this->SetSizer( MainSizer );
	this->Layout();
	MainSizer->Fit( this );

    m_SearchString = wxString::Format( wxT( "\"%s\" \"%s\"" ), Artist.c_str(), Album.c_str() );
    m_InputTextCtrl->SetValue( m_SearchString );
    m_CurrentImage = 0;

	// Connect Events
	m_InputTextCtrl->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( guCoverEditor::OnTextCtrlEnter ), NULL, this );
	m_ClearTextBitmap->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( guCoverEditor::OnSearchClear ), NULL, this );
	m_CoverBitmap->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guCoverEditor::OnCoverLeftDClick ), NULL, this );
	m_PrevButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guCoverEditor::OnPrevButtonClick ), NULL, this );
	m_NextButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guCoverEditor::OnNextButtonClick ), NULL, this );

    Connect( ID_COVEREDITOR_ADDCOVERIMAGE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guCoverEditor::OnAddCoverImage ) );
    Connect( ID_COVEREDITOR_DOWNLOADEDLINKS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guCoverEditor::OnDownloadedLinks ) );

	//AddCoverLinks();

    m_DownloadCoversThread = new guFetchCoverLinksThread( this, m_SearchString.c_str() );

    m_PrevButton->Disable();
    m_NextButton->Disable();
}

// -------------------------------------------------------------------------------- //
guCoverEditor::~guCoverEditor()
{
    if( m_DownloadCoversThread )
    {
        m_DownloadCoversThread->Pause();
        m_DownloadCoversThread->Delete();
    }

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
    m_DownloadThreadMutex.Unlock();

	// Connect Events
	m_InputTextCtrl->Disconnect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( guCoverEditor::OnTextCtrlEnter ), NULL, this );
	m_ClearTextBitmap->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( guCoverEditor::OnSearchClear ), NULL, this );
	m_CoverBitmap->Disconnect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( guCoverEditor::OnCoverLeftDClick ), NULL, this );
	m_PrevButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guCoverEditor::OnPrevButtonClick ), NULL, this );
	m_NextButton->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( guCoverEditor::OnNextButtonClick ), NULL, this );

    Disconnect( ID_COVEREDITOR_ADDCOVERIMAGE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( guCoverEditor::OnAddCoverImage ) );
}

// -------------------------------------------------------------------------------- //
void guCoverEditor::EndDownloadLinksThread( void )
{
    m_DownloadThreadMutex.Lock();
    if( !m_DownloadThreads.Count() )
    {
        if( m_Gauge->IsPulsing() )
            m_Gauge->StopPulse( MAX_COVERLINKS_ITEMS, MAX_COVERLINKS_ITEMS );
        else
            m_Gauge->SetValue( MAX_COVERLINKS_ITEMS );
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
        m_Gauge->SetValue( m_Gauge->GetValue() + 1 );
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
void guCoverEditor::OnAddCoverImage( wxCommandEvent &event )
{
    m_DownloadEventsMutex.Lock();
    guCoverImage * Image = ( guCoverImage * ) event.GetClientData();
    if( Image )
        m_AlbumCovers.Add( Image );
    // When it is the first show the image.
    if( m_AlbumCovers.Count() == 1 )
        UpdateCoverBitmap();
    if( ( m_CurrentImage < ( int ) ( m_AlbumCovers.Count() - 1 ) ) && !m_NextButton->IsEnabled() )
        m_NextButton->Enable();
    m_InfoTextCtrl->SetLabel( wxString::Format( wxT( "%02u/%02u" ), m_CurrentImage + 1, m_AlbumCovers.Count() ) );
    //guLogMessage( wxT( "CurImg: %u  Total:%u" ), m_CurrentImage + 1, m_AlbumCovers.Count() );
    EndDownloadCoverThread( ( guDownloadCoverThread * ) event.GetClientObject() );
    m_DownloadEventsMutex.Unlock();
}

// -------------------------------------------------------------------------------- //
void guCoverEditor::UpdateCoverBitmap( void )
{
    m_InfoTextCtrl->SetLabel( wxString::Format( wxT( "%02u/%02u" ),  m_AlbumCovers.Count() ? m_CurrentImage + 1 : 0, m_AlbumCovers.Count() ) );

    wxBitmap * BlankCD = new wxBitmap( guImage_blank_cd_cover );
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

// -------------------------------------------------------------------------------- //
void guCoverEditor::OnCoverLeftDClick( wxMouseEvent &event )
{
    EndModal( wxID_OK );
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
    // Get the new Search String
    m_SearchString = guURLEncode( m_InputTextCtrl->GetValue().Trim().Trim( false ) );
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
    m_DownloadCoversThread = new guFetchCoverLinksThread( this, m_SearchString.c_str() );

    // Disable buttons till one cover is downloaded
    m_PrevButton->Disable();
    m_NextButton->Disable();
}

// -------------------------------------------------------------------------------- //
void guCoverEditor::OnSearchClear( wxMouseEvent& event )
{
    m_InputTextCtrl->SetValue( wxEmptyString );
}

// -------------------------------------------------------------------------------- //
// guFetchCoverLinksThread
// -------------------------------------------------------------------------------- //
guFetchCoverLinksThread::guFetchCoverLinksThread( guCoverEditor * Owner, const wxChar * SearchStr ) :
    wxThread()
{
    m_CoverEditor   = Owner;
    m_SearchString  = wxString( SearchStr );
    m_LastDownload  = 0;
    m_CurrentPage   = 0;

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guFetchCoverLinksThread::~guFetchCoverLinksThread()
{
    // Indicate the main object that the child thread has been deleted
//    if( !TestDestroy() )
//        m_CoverEditor->EndDownloadLinksThread();
};

// -------------------------------------------------------------------------------- //
wxArrayString guFetchCoverLinksThread::ExtractImageInfo( const wxString &Content )
{
    wxArrayString RetVal;
    wxString CurParam;
    CurParam = wxEmptyString;
    wxChar CurChar;
    int index;
    int count = Content.Length();
    for( index = 0; index < count; index++ )
    {
        CurChar = Content[ index ];
        if( CurChar == wxT( '\"' ) )
        {
            index++;
            while( ( CurChar = Content[ index ] ) != wxT( '\"' ) && ( index < count ) )
            {
                CurParam.Append( CurChar );
                index++;
            }
        }
        else if( CurChar == wxT( ',' ) /*|| CurChar == wxT( ')' )*/ )
        {
            //guLogMessage( wxT( "%s" ), CurParam.c_str() );
            RetVal.Add( CurParam );
            CurParam = wxEmptyString;
        }
        else
        {
            CurParam.Append( CurChar );
        }
    }
    if( !CurParam.IsEmpty() )
        RetVal.Add( CurParam );
    //guLogMessage( wxT( "ImageLink: %s" ), RetVal[ 3 ].c_str() );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
int guFetchCoverLinksThread::ExtractImagesInfo( wxString &Content, int Count )
{
    wxArrayString CurImage;
    int ImageIndex = 0;

    int StrPos = Content.Find( wxT( "dyn.setResults([[" ) );

    if( StrPos != wxNOT_FOUND )
        StrPos += 14;
    //guLogMessage( wxT( "Content:\n%s" ), Content.c_str() );
    while( ( StrPos != wxNOT_FOUND ) && !TestDestroy() )
    {
        Content = Content.Mid( StrPos + 3 );
        StrPos = Content.Find( wxT( "],[" ) );
        if( StrPos == wxNOT_FOUND )
          return 0; //break;
        //guLogMessage( wxT( "%s" ), Content.Mid( 0, StrPos ).c_str() );
        CurImage = ExtractImageInfo( Content.Mid( 0, StrPos ) );
        //RetVal.Add( CurImage );
        m_CoverLinks.Add( CurImage );
        ImageIndex++;
        if( ImageIndex == Count )
            break;

        //guLogMessage( wxT( "Pos: %u" ), StrPos );
    }
    return ImageIndex;
}

#define GUCOVERINFO_LINK    3           // 3 -> Link
#define GUCOVERINFO_COMMENT 6           // 6 -> Comment
#define GUCOVERINFO_SIZE    9           // 9 -> Size >> 425 x 283 - 130 KB

// -------------------------------------------------------------------------------- //
bool guFetchCoverLinksThread::AddCoverLinks( void )
{
    //guLogMessage( wxT( "URL: %u %s" ), m_CurrentPage, m_SearchString.c_str() );
    wxString SearchUrl = wxString::Format( GOOGLE_IMAGES_SEARCH_STR, guURLEncode( m_SearchString ).c_str(), ( m_CurrentPage * COVERS_COUNTER_PER_PAGE ) );
    //guLogMessage( wxT( "URL: %u %s" ), m_CurrentPage, SearchUrl.c_str() );
    //guHTTP http;
    //http.SetHeader( wxT( "User-Agent" ), wxT( "Mozilla/5.0 (X11; U; Linux x86_64; en-US; rv:1.9.0.2) Gecko/2008092313 Ubuntu/8.04 (hardy) Firefox/3.1" ) );
    char * Buffer = NULL;
    wxCurlHTTP http;
    http.AddHeader( wxT( "User-Agent: Mozilla/5.0 (X11; U; Linux i686; es-ES; rv:1.9.0.5) Gecko/2008121622 Ubuntu/8.10 (intrepid) Firefox/3.0.5" ) );
    http.AddHeader( wxT( "Accept: text/html" ) );
    http.AddHeader( wxT( "Accept-Charset: utf-8" ) );
    http.SetVerbose( true );
    http.Get( Buffer, SearchUrl );
    if( Buffer )
    {
        //printf( "Buffer:\n%s\n", Buffer );
        if( !TestDestroy() )
        {
            wxString Content = wxString( Buffer, wxConvUTF8 );
            //Content = http.GetContent( SearchUrl, 60 );
            if( Content.Length() )
            {
                if( !TestDestroy() )
                {
                    //guLogMessage( Content );
                    return ExtractImagesInfo( Content, COVERS_COUNTER_PER_PAGE );
                }
            }
            else
            {
                guLogError( wxT( "Could not get the remote data from connection" ) );
            }
        }
        free( Buffer );
    }
    else
    {
        guLogWarning( wxT( "No data received when searching for images" ) );
    }
    return false;
}

// -------------------------------------------------------------------------------- //
guFetchCoverLinksThread::ExitCode guFetchCoverLinksThread::Entry()
{
    bool NoMorePics = false;
    while( !TestDestroy() )
    {
        if( m_LastDownload < ( int ) m_CoverLinks.Count() )
        {
            m_CoverEditor->m_DownloadThreadMutex.Lock();
            if( !TestDestroy() )
            {
                guDownloadCoverThread * DownloadThread = new guDownloadCoverThread( m_CoverEditor,
                                          &m_CoverLinks[ m_LastDownload ] );
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
            if( !AddCoverLinks() )
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

    if( !TestDestroy() )
    {
        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_COVEREDITOR_DOWNLOADEDLINKS );
        event.SetClientObject( ( wxClientData * ) this );
        wxPostEvent( m_CoverEditor, event );
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
//    if( !TestDestroy() )
//    {
//        m_CoverEditor->EndDownloadCoverThread( this );
//    }
}

// -------------------------------------------------------------------------------- //
guDownloadCoverThread::ExitCode guDownloadCoverThread::Entry()
{
    //wxURL       url;
    long        ImageType;
    wxImage *   Image = NULL;
    guCoverImage * CoverImage = NULL;

    if( m_UrlStr.Lower().EndsWith( wxT( ".jpg" ) ) )
      ImageType = wxBITMAP_TYPE_JPEG;
    else if( m_UrlStr.Lower().EndsWith( wxT( ".png" ) ) )
      ImageType = wxBITMAP_TYPE_PNG;
//    else if( UrlStr.Lower().EndsWith( wxT( ".gif" ) ) ) // Removed because of some random segfaults
//      ImageType = wxBITMAP_TYPE_GIF;                    // in gifs handler functions
    else if( m_UrlStr.Lower().EndsWith( wxT( ".bmp" ) ) )
      ImageType = wxBITMAP_TYPE_BMP;
    else
      ImageType = wxBITMAP_TYPE_INVALID;

    if( !TestDestroy() && ImageType > wxBITMAP_TYPE_INVALID )
    {
        wxMemoryOutputStream Buffer;
        wxCurlHTTP http;
        //guLogMessage( wxT( "Init: '%s'" ), m_UrlStr.c_str() );
        if( http.Get( Buffer, m_UrlStr ) )
        {
            if( Buffer.IsOk() && !TestDestroy() )
            {
                wxMemoryInputStream Ins( Buffer );
                if( Ins.IsOk() && !TestDestroy() )
                {
                    Image = new wxImage( Ins, ImageType );
                    if( Image )
                    {
                        if( Image->IsOk() && !TestDestroy() )
                        {
                            CoverImage = new guCoverImage( m_UrlStr, m_SizeStr, Image );
                        }
                        else
                        {
                          //guLogWarning( wxT( "Could not load image from the net index %u." ), LastDownload );
                          delete Image;
                        }
                    }
                }
            }
        }
    }
    if( !TestDestroy() )
    {
        //guLogMessage( wxT( "Done: '%s'" ), m_UrlStr.c_str() );
        //m_CoverEditor->m_DownloadEventsMutex.Lock();
        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_COVEREDITOR_ADDCOVERIMAGE );
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

// -------------------------------------------------------------------------------- //
