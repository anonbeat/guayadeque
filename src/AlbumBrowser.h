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
#ifndef ALBUMBROWSER_H
#define ALBUMBROWSER_H

#include "DbLibrary.h"

#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/statbmp.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/panel.h>

// -------------------------------------------------------------------------------- //
class guAlbumBrowserItem
{
  public :
    wxImage *     m_Cover;
    wxString      m_ArtistName;
    wxString      m_AlbumName;
    unsigned int  m_Year;
    unsigned int  m_TrackCount;

    guAlbumBrowserItem() { m_Cover = NULL; m_Year = 0; m_TrackCount = 0; };
    ~guAlbumBrowserItem() { if( m_Cover ) delete m_Cover; };
};
WX_DECLARE_OBJARRAY( guAlbumBrowserItem, guAlbumBrowserItemArray );

// -------------------------------------------------------------------------------- //
class guAlbumBrowserItemPanel : public wxPanel
{
  protected :
    int                     m_Index;
    guAlbumBrowserItem *    m_AlbumBrowserItem;

    // GUI
    wxStaticBitmap *        m_Bitmap;
    wxStaticText *          m_ArtistLabel;
    wxStaticText *          m_AlbumLabel;
    wxStaticText *          m_TracksLabel;

  public :
    guAlbumBrowserItemPanel( wxWindow * parent, const int index, guAlbumBrowserItem * albumitem = NULL );
    ~guAlbumBrowserItemPanel();

    void SetAlbumItem( const int index, guAlbumBrowserItem * albumitem );

};
WX_DEFINE_ARRAY_PTR( guAlbumBrowserItemPanel *, guAlbumBrowserItemPanelArray );


// -------------------------------------------------------------------------------- //
class guAlbumBrowser : public wxPanel
{
  protected :
    guDbLibrary *                   m_Db;
    guAlbumBrowserItemArray         m_AlbumItems;
    guAlbumBrowserItemPanelArray    m_ItemPanels;
    unsigned int                    m_StartItem;

    // GUI
	wxGridSizer *                   m_AlbumsSizer;
	wxStaticText *                  m_NavLabel;
	wxSlider *                      m_NavSlider;

  public :
    guAlbumBrowser( wxWindow * parent, guDbLibrary * db );
    ~guAlbumBrowser();

    void ReloadItems();

};

#endif
// -------------------------------------------------------------------------------- //
