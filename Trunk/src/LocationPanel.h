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
#ifndef LOCATIONPANEL_H
#define LOCATIONPANEL_H

#define     guLOCATION_ID_LIBRARY               ( 1 << 0 )
#define     guLOCATION_ID_LIBRARY_TREE          ( 1 << 1 )
#define     guLOCATION_ID_ALBUM_BROWSER         ( 1 << 2 )
#define     guLOCATION_ID_PLAYLISTS             ( 1 << 3 )
#define     guLOCATION_ID_FILE_BROWSER          ( 1 << 4 )
#define     guLOCATION_ID_JAMENDO               ( 1 << 5 )
#define     guLOCATION_ID_MAGNATUNE             ( 1 << 6 )

#define     guLOCATION_ID_MY_MUSIC              ( 1 << 15 )
#define     guLOCATION_ID_PORTABLE_DEVICE       ( 1 << 16 )
#define     guLOCATION_ID_ONLINE_RADIO          ( 1 << 17 )
#define     guLOCATION_ID_ONLINE_SHOPS          ( 1 << 18 )
#define     guLOCATION_ID_PODCASTS              ( 1 << 19 )


enum guLocationOpenMode {
    guLOCATION_OPENMODE_AUTOMATIC,
    guLOCATION_OPENMODE_FORCED
};

// -------------------------------------------------------------------------------- //
class guLocationTreeCtrl : public wxTreeCtrl
{
};

// -------------------------------------------------------------------------------- //
class guLocationPanel : public wxPanel
{
  protected :
    int         m_OpenMode;
  public :
    guLocationPanel( wxWindow * parent );
    ~guLocationPanel();

}


#endif
// -------------------------------------------------------------------------------- //
