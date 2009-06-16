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
#include "CoverFrame.h"

#include "Images.h"
#include "TagInfo.h"
#include "Utils.h"

// -------------------------------------------------------------------------------- //
guCoverFrame::guCoverFrame( wxWindow * parent, wxWindowID id, const wxString & title, const wxPoint & pos, const wxSize & size, long style ) :
     wxFrame( parent, id, title, pos, size, style | wxFRAME_NO_TASKBAR )
{
	SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer * CoverSizer;
	CoverSizer = new wxBoxSizer( wxVERTICAL );

	m_CoverBitmap = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize( -1,-1 ), 0 );
	CoverSizer->Add( m_CoverBitmap, 1, wxALL|wxEXPAND, 0 );

	SetSizer( CoverSizer );
	Layout();

	Connect( wxEVT_ACTIVATE, wxActivateEventHandler( guCoverFrame::CoverFrameActivate ) );
	m_CoverBitmap->Connect( wxEVT_LEFT_UP, wxMouseEventHandler( guCoverFrame::OnClick ), NULL, this );

    //printf( "guCoverFrame Constructed\n" );
}

// -------------------------------------------------------------------------------- //
guCoverFrame::~guCoverFrame()
{
    //printf( "guCoverFrame Destroyed\n" );
}

void guCoverFrame::OnClick( wxMouseEvent &event )
{
    Close();
}

// -------------------------------------------------------------------------------- //
void guCoverFrame::SetBitmap( const guSongCoverType CoverType, const wxString &CoverPath )
{
    wxImage CoverImage;
    if( CoverType == GU_SONGCOVER_ID3TAG )
    {
        wxImage * id3cover = ID3TagGetPicture( CoverPath );
        if( id3cover )
        {
            CoverImage = * id3cover;
            delete id3cover;
        }
        else
            guLogError( wxT( "Could not retrieve the image from '%s'" ), CoverPath.c_str() );
    }
    if( CoverType == GU_SONGCOVER_FILE )
    {
        CoverImage.LoadFile( CoverPath );
    }
    else if( CoverType == GU_SONGCOVER_NONE )
    {
        CoverImage = wxImage( guImage_no_cover );
    }
    else if( CoverType == GU_SONGCOVER_RADIO )
    {
        CoverImage = wxImage( guImage_net_radio );
    }
    //
    if( CoverImage.IsOk() )
    {
        wxBitmap * BlankCD = new wxBitmap( guImage_blank_cd_cover );
        if( BlankCD )
        {
            if( BlankCD->IsOk() )
            {
                // 38,6
                wxMemoryDC MemDC;
                MemDC.SelectObject( * BlankCD );
                CoverImage.Rescale( 250, 250, wxIMAGE_QUALITY_HIGH );
                MemDC.DrawBitmap( wxBitmap( CoverImage ), 34, 4, false );
                m_CoverBitmap->SetBitmap( * BlankCD );
                //m_CoverBitmap->SetBitmap( wxBitmap( CoverImage ) );
                m_CoverBitmap->Refresh();
            }
            delete BlankCD;
        }
    }
}

// -------------------------------------------------------------------------------- //
void guCoverFrame::CoverFrameActivate( wxActivateEvent &event )
{
    if( !event.GetActive() )
      Close();
}

// -------------------------------------------------------------------------------- //
