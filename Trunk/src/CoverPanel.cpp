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
#include "CoverPanel.h"

#include "Images.h"
#include "TagInfo.h"

#define guCOVERPANEL_RESIZE_TIMER_TIME  250
#define guCOVERPANEL_RESIZE_TIMER_ID    10

// -------------------------------------------------------------------------------- //
guCoverPanel::guCoverPanel( wxWindow * parent, guPlayerPanel * playerpanel ) :
    wxPanel( parent, wxID_ANY, wxDefaultPosition, wxSize( 100, 100 ), wxTAB_TRAVERSAL ),
    m_ResizeTimer( this, guCOVERPANEL_RESIZE_TIMER_ID )
{
    m_PlayerPanel = playerpanel;
    m_LastSize = 100;
    m_CoverImage = wxBitmap( guImage( guIMAGE_INDEX_no_cover ) );
    m_CoverType = GU_SONGCOVER_NONE;

	Connect( wxEVT_SIZE, wxSizeEventHandler( guCoverPanel::OnSize ) );
	Connect( wxEVT_PAINT, wxPaintEventHandler( guCoverPanel::OnPaint ) );
	Connect( guCOVERPANEL_RESIZE_TIMER_ID, wxEVT_TIMER, wxTimerEventHandler( guCoverPanel::OnResizeTimer ), NULL, this );
}

// -------------------------------------------------------------------------------- //
guCoverPanel::~guCoverPanel()
{
	Disconnect( wxEVT_SIZE, wxSizeEventHandler( guCoverPanel::OnSize ) );
	Disconnect( wxEVT_PAINT, wxPaintEventHandler( guCoverPanel::OnPaint ) );
	Disconnect( guCOVERPANEL_RESIZE_TIMER_ID, wxEVT_TIMER, wxTimerEventHandler( guCoverPanel::OnResizeTimer ), NULL, this );
}

// -------------------------------------------------------------------------------- //
void guCoverPanel::OnPaint( wxPaintEvent &event )
{
	wxCoord Width;
	wxCoord Height;
	GetClientSize( &Width, &Height );

    wxMutexLocker Lock( m_CoverImageMutex );
    wxPaintDC dc( this );
    dc.DrawBitmap( m_CoverImage, ( Width - m_LastSize ) / 2, ( Height - m_LastSize ) / 2, false );
}

// -------------------------------------------------------------------------------- //
void guCoverPanel::OnResizeTimer( wxTimerEvent &event )
{
    UpdateImage();
}

// -------------------------------------------------------------------------------- //
void guCoverPanel::OnSize( wxSizeEvent &event )
{
    wxSize Size = event.GetSize();
    int MinSize = wxMin( Size.GetWidth(), Size.GetHeight() );
    guLogMessage( wxT( "NewSize: %u" ), MinSize );
    if( MinSize != m_LastSize )
    {
        m_LastSize = MinSize;
        guLogMessage( wxT( "Updating Size: %u" ), MinSize );
//        UpdateImage();
        if( m_ResizeTimer.IsRunning() )
        {
            m_ResizeTimer.Stop();
        }
        m_ResizeTimer.Start( guCOVERPANEL_RESIZE_TIMER_TIME, wxTIMER_ONE_SHOT );
    }
    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guCoverPanel::UpdateImage( void )
{
    wxImage * CoverImage = NULL;

    switch( m_CoverType )
    {
        case GU_SONGCOVER_FILE :
            CoverImage = new wxImage( m_CoverPath );
            break;

        case GU_SONGCOVER_ID3TAG :
            CoverImage = guTagGetPicture( m_CoverPath );
            break;

        case GU_SONGCOVER_RADIO :
            CoverImage = new wxImage( guImage( guIMAGE_INDEX_net_radio ) );
            break;

        case GU_SONGCOVER_PODCAST :
            CoverImage = new wxImage( guImage( guIMAGE_INDEX_podcast ) );
            break;

        default :
            break;
    }

    if( !CoverImage )
    {
        CoverImage = new wxImage( guImage( guIMAGE_INDEX_no_cover ) );
    }

    if( CoverImage )
    {
        CoverImage->Rescale( m_LastSize, m_LastSize, wxIMAGE_QUALITY_HIGH );
        wxMutexLocker Lock( m_CoverImageMutex );
        m_CoverImage = wxBitmap( * CoverImage );
        //Update();
        Refresh();

        delete CoverImage;
    }
}

// -------------------------------------------------------------------------------- //
void guCoverPanel::OnUpdatedTrack( wxCommandEvent &event )
{
    const guCurrentTrack * CurrentTrack = m_PlayerPanel->GetCurrentTrack();

    if( CurrentTrack )
    {
        m_CoverType = CurrentTrack->m_CoverType;
        m_CoverPath = CurrentTrack->m_CoverPath;
        guLogMessage( wxT( "Changed image to %i '%s'" ), m_CoverType, m_CoverPath.c_str() );
    }
    else
    {
        m_CoverType = GU_SONGCOVER_NONE;
        m_CoverPath = wxEmptyString;
    }
    UpdateImage();
}

// -------------------------------------------------------------------------------- //
