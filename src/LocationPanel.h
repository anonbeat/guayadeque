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

#include "MainFrame.h"

#include <wx/aui/aui.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/statbmp.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/statline.h>
#include <wx/listctrl.h>
#include <wx/splitter.h>
#include <wx/frame.h>
#include <wx/srchctrl.h>


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
  protected :
    guMainFrame *   m_MainFrame;
    wxImageList *   m_ImageList;

    wxTreeItemId    m_RootId;
    wxTreeItemId    m_MyMusicId;
    wxTreeItemId    m_PortableDeviceId;
    wxTreeItemId    m_OnlineRadioId;
    wxTreeItemId    m_OnlineStoreId;
    wxTreeItemId    m_PodcastId;
    wxTreeItemId    m_ContextId;

    wxArrayString   m_IconNames;
    int             m_LockCount;


    void            OnContextMenu( wxTreeEvent &event );
    void            OnKeyDown( wxKeyEvent &event );

  public :
    guLocationTreeCtrl( wxWindow * parent, guMainFrame * mainframe );
    ~guLocationTreeCtrl();

    void            ReloadItems( const bool loadstate = false );

    wxTreeItemId    MyMusicId( void ) { return m_MyMusicId; }
    wxTreeItemId    PortableDeviceId( void ) { return m_PortableDeviceId; }
    wxTreeItemId    OnlineRadioId( void ) { return m_OnlineRadioId; }
    wxTreeItemId    OnlineStoreId( void ) { return m_OnlineStoreId; }
    wxTreeItemId    PodcastId( void ) { return m_PodcastId; }
    wxTreeItemId    ContextId( void ) { return m_ContextId; }

    bool            Locked( void ) { return m_LockCount; }
    void            Lock( void ) { m_LockCount++; }

    void            Unlock( void )
    {
        if( m_LockCount )
        {
            m_LockCount--;
            if( !m_LockCount )
                ReloadItems();
        }
    }
};

// -------------------------------------------------------------------------------- //
class guLocationPanel : public wxPanel
{
  protected :
    guMainFrame *           m_MainFrame;
    guLocationTreeCtrl *    m_LocationTreeCtrl;

    void                    OnLocationItemActivated( wxTreeEvent &event );
    void                    OnLocationItemChanged( wxTreeEvent &event );

  public :
    guLocationPanel( wxWindow * parent );
    ~guLocationPanel();

    void                    OnPortableDeviceChanged( void );
    void                    OnPanelVisibleChanged( void );

    bool                    Locked( void ) { return m_LocationTreeCtrl->Locked(); }
    void                    Lock( void ) { m_LocationTreeCtrl->Lock(); }
    void                    Unlock( void ) { m_LocationTreeCtrl->Unlock(); }

};


#endif
// -------------------------------------------------------------------------------- //
