// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2023 J.Rios anonbeat@gmail.com
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
#include "CoverWindow.h"

#include "CoverPanel.h"
#include "Images.h"
#include "TagInfo.h"

#define guCOVERWINDOW_RESIZE_TIMER_TIME  250
#define guCOVERWINDOW_RESIZE_TIMER_ID    10

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
guCoverWindow::guCoverWindow( guCoverPanel * parent, wxWindowID id, const wxString & title, const wxPoint & pos, const wxSize & size, long style ) :
     wxFrame( parent, id, title, pos, size, style ),
    m_ResizeTimer( this, guCOVERWINDOW_RESIZE_TIMER_ID )
{
    m_LastSize = 100;
    m_CoverPanel = parent;

    m_Panel = new wxPanel(this);

    Bind( wxEVT_SIZE, &guCoverWindow::OnSize, this );
    Bind( wxEVT_PAINT, &guCoverWindow::OnPaint, this );
    Bind( wxEVT_TIMER, &guCoverWindow::OnResizeTimer, this, guCOVERWINDOW_RESIZE_TIMER_ID );
    Bind( wxEVT_LEFT_UP, &guCoverWindow::OnClick, this);
    Bind( wxEVT_RIGHT_UP, &guCoverWindow::OnRightClick, this);
    Bind( wxEVT_MOUSEWHEEL, &guCoverWindow::OnClick, this);
    m_Panel->Bind( wxEVT_KEY_UP, &guCoverWindow::OnKey, this);
    m_Panel->SetFocus();

    wxCommandEvent Event;
    OnUpdatedTrack( Event );
}

// -------------------------------------------------------------------------------- //
guCoverWindow::~guCoverWindow()
{
    guLogDebug("guCoverWindow::~guCoverWindow");

    m_CoverPanel->m_CoverWindow = NULL;

    m_Panel->Unbind( wxEVT_KEY_UP, &guCoverWindow::OnKey, this);

    Unbind( wxEVT_SIZE, &guCoverWindow::OnSize, this );
    Unbind( wxEVT_PAINT, &guCoverWindow::OnPaint, this );
    Unbind( wxEVT_TIMER, &guCoverWindow::OnResizeTimer, this, guCOVERWINDOW_RESIZE_TIMER_ID );
    Unbind( wxEVT_LEFT_UP, &guCoverWindow::OnClick, this);
    Unbind( wxEVT_RIGHT_UP, &guCoverWindow::OnRightClick, this);
    Unbind( wxEVT_MOUSEWHEEL, &guCoverWindow::OnClick, this);
}

// -------------------------------------------------------------------------------- //
void guCoverWindow::OnPaint( wxPaintEvent &event )
{
	wxCoord Width;
	wxCoord Height;
	GetClientSize( &Width, &Height );
    if( Width && Height )
    {
        wxMutexLocker Lock( m_CoverImageMutex );
        wxPaintDC dc( this );
        dc.DrawBitmap( m_CoverImage, ( Width - m_LastSize ) / 2, ( Height - m_LastSize ) / 2, false );
    }
    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guCoverWindow::OnResizeTimer( wxTimerEvent &event )
{
    UpdateImage();
}

// -------------------------------------------------------------------------------- //
void guCoverWindow::OnClick( wxMouseEvent &event )
{
    guLogDebug("guCoverWindow::OnClick");
    Close();
}

// -------------------------------------------------------------------------------- //
void guCoverWindow::OnRightClick( wxMouseEvent &event )
{
    guLogDebug("guCoverWindow::OnRightClick");
    if ( IsFullScreen() ) {
        ShowFullScreen( false );
    }
    else
    {
        ShowFullScreen( true );
    }
}

void guCoverWindow::OnKey( wxKeyEvent &event )
{
    guLogDebug("guCoverWindow::OnKey");
    Close();
}

// -------------------------------------------------------------------------------- //
void guCoverWindow::OnSize( wxSizeEvent &event )
{
    guLogDebug("guCoverWindow::OnSize");
    wxSize Size = event.GetSize();
    int MinSize = wxMin( Size.GetWidth(), Size.GetHeight() );
    if( MinSize != m_LastSize )
    {
        m_LastSize = MinSize;
        if( m_ResizeTimer.IsRunning() )
        {
            m_ResizeTimer.Stop();
        }
        m_ResizeTimer.Start( guCOVERWINDOW_RESIZE_TIMER_TIME, wxTIMER_ONE_SHOT );
    }
    SetFocus();
}

// -------------------------------------------------------------------------------- //
void guCoverWindow::UpdateImage( void )
{
    guLogDebug("guCoverWindow::UpdateImage");
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

    if( !CoverImage || !CoverImage->IsOk() )
    {
        if( CoverImage )
            delete CoverImage;
        CoverImage = new wxImage( guImage( guIMAGE_INDEX_no_cover ) );
    }

    if( CoverImage )
    {
        if( m_LastSize > 0 )
            CoverImage->Rescale( m_LastSize, m_LastSize, wxIMAGE_QUALITY_HIGH );
        wxMutexLocker Lock( m_CoverImageMutex );
        m_CoverImage = wxBitmap( * CoverImage );
        //Update();
        Refresh();

        delete CoverImage;
    }
}

// -------------------------------------------------------------------------------- //
void guCoverWindow::OnUpdatedTrack( wxCommandEvent &event )
{
    UpdateImage();
}

// -------------------------------------------------------------------------------- //
void guCoverWindow::SetBitmap( const guSongCoverType CoverType, const wxString &CoverPath )
{
    guLogDebug("guCoverWindow::SetBitmap ( %d , %s )", CoverType, CoverPath);
    m_CoverType = CoverType;
    m_CoverPath = CoverPath;
    UpdateImage();
}

} // Guayadeque namespace

// -------------------------------------------------------------------------------- //
