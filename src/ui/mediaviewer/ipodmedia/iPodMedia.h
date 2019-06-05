// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2019 J.Rios anonbeat@gmail.com
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
#ifndef __IPODMEDIA_H__
#define __IPODMEDIA_H__

#ifdef WITH_LIBGPOD_SUPPORT

#include "AlbumBrowser.h"
#include "Config.h"
#include "DbLibrary.h"
#include "GIO_Volume.h"
#include "LibPanel.h"
#include "MediaViewer.h"
#include "PlayListPanel.h"
#include "PortableMedia.h"
#include "Preferences.h"

#include <wx/window.h>
#include <wx/dynarray.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/gauge.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/panel.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/notebook.h>
#include <wx/statbox.h>
#include <wx/dialog.h>


#include <gpod/itdb.h>

namespace Guayadeque {

class guMediaVieweriPodDevice;

// -------------------------------------------------------------------------------- //
class guIpodLibrary : public guPortableMediaLibrary
{
  protected :
    Itdb_iTunesDB * m_iPodDb;

  public :
    guIpodLibrary( const wxString &libpath, guPortableMediaDevice * portablemediadevice, Itdb_iTunesDB * ipoddb );
    ~guIpodLibrary();

    Itdb_iTunesDB *     GetiPodDb( void ) { return m_iPodDb; }

    virtual int         UpdateSong( const guTrack &track, const bool allowrating );

    virtual int         CreateStaticPlayList( const wxString &name, const wxArrayInt &tracks );
    int                 CreateStaticPlayList( const wxString &name, const wxArrayInt &tracks, const bool indbonly ) { return guDbLibrary::CreateStaticPlayList( name, tracks ); }
    virtual int         UpdateStaticPlayList( const int plid, const wxArrayInt &tracks );
    virtual int         AppendStaticPlayList( const int plid, const wxArrayInt &tracks );
    virtual int         DelPlaylistSetIds( const int plid, const wxArrayInt &tracks );
    virtual void        SetPlayListName( const int plid, const wxString &newname );
    virtual void        DeletePlayList( const int plid );

    virtual void        UpdateStaticPlayListFile( const int plid );

    virtual int         CreateDynamicPlayList( const wxString &name, const guDynPlayList * playlist );
    int                 CreateDynamicPlayList( const wxString &name, const guDynPlayList * playlist, const bool indbonly ) { return guDbLibrary::CreateDynamicPlayList( name, playlist ); }
    virtual void        UpdateDynamicPlayList( const int plid, const guDynPlayList * playlist );

    int GetAlbumId( const wxString &albumname, const wxString &artist, const wxString &albumartist, const wxString &disk );

    Itdb_Playlist *     CreateiPodPlayList( const wxString &path, const wxArrayString &filenames );
    Itdb_Track *        iPodFindTrack( const wxString &filename );
    Itdb_Track *        iPodFindTrack( const wxString &artist, const wxString &albumartist, const wxString &album, const wxString &title );


    void                iPodRemoveTrack( const wxString &filename );
    void                iPodRemoveTrack( Itdb_Track * track );
    bool                iPodFlush( void ) { return itdb_write( m_iPodDb, NULL ); }

};

// -------------------------------------------------------------------------------- //
class guIpodLibraryUpdate : public wxThread
{
  protected :
    guMediaVieweriPodDevice *   m_MediaViewer;
    int                         m_GaugeId;

  public :
    guIpodLibraryUpdate( guMediaVieweriPodDevice * mediaviewer, const int gaugeid );
    ~guIpodLibraryUpdate();

    ExitCode Entry();

};


//////// -------------------------------------------------------------------------------- //
//////class guIpodMediaLibPanel : public guPortableMediaLibPanel
//////{
//////  protected :
//////     guIpodLibraryUpdate *      m_UpdateThread;
//////
//////    virtual void                CreateContextMenu( wxMenu * menu, const int windowid = 0 );
//////
//////    virtual void                NormalizeTracks( guTrackArray * tracks, const bool isdrag = false );
//////
//////    virtual void                UpdateTracks( const guTrackArray &tracks, const guImagePtrArray &images,
//////                                const wxArrayString &lyrics, const wxArrayInt &changedflags );
//////
//////    virtual void                DoDeleteAlbumCover( const int albumid );
//////
//////    virtual bool                SetAlbumCover( const int albumid, const wxString &albumpath, wxImage * coverimg );
//////    virtual bool                SetAlbumCover( const int albumid, const wxString &albumpath, wxString &coverpath );
//////
//////    virtual void                DeleteTracks( guTrackArray * tracks );
//////
//////    void                        OnGaugeCreated( wxCommandEvent &event );
//////
//////  public :
//////    guIpodMediaLibPanel( wxWindow * parent, guIpodLibrary * db, guPlayerPanel * playerpanel );
//////    ~guIpodMediaLibPanel();
//////
//////    virtual void                DoUpdate( const bool forced = false );
//////    virtual void                UpdateFinished( void );
//////
//////    virtual void                SetPortableMediaDevice( guPortableMediaDevice * portablemediadevice );
//////
//////    virtual wxArrayString       GetLibraryPaths( void );
//////
//////    virtual int                 CopyTo( const guTrack * track, wxString &filename );
//////
//////};
//////
//////// -------------------------------------------------------------------------------- //
//////class guIpodPlayListPanel : public guPortableMediaPlayListPanel
//////{
//////  protected :
//////    //guIpodMediaLibPanel * m_LibPanel;
//////
//////    virtual void        NormalizeTracks( guTrackArray * tracks, const bool isdrag = false );
//////
//////  public :
//////    guIpodPlayListPanel( wxWindow * parent, guIpodLibrary * db, guPlayerPanel * playerpanel, guIpodMediaLibPanel * libpanel );
//////    ~guIpodPlayListPanel();
//////
//////};
//////
//////// -------------------------------------------------------------------------------- //
//////class guIpodAlbumBrowser : public guPortableMediaAlbumBrowser
//////{
//////  protected :
//////
//////    virtual void OnBitmapMouseOver( const int coverid, const wxPoint &position );
//////
//////  public :
//////    guIpodAlbumBrowser( wxWindow * parent, guIpodLibrary * db, guPlayerPanel * playerpanel, guIpodMediaLibPanel * libpanel );
//////    ~guIpodAlbumBrowser();
//////
//////};
//////
//////
//////#endif

//// -------------------------------------------------------------------------------- //
//class guPortableMediaViewCtrl
//{
//  protected :
//    guMainFrame *                   m_MainFrame;
//
//    guPortableMediaDevice *         m_PortableDevice;
//    guPortableMediaLibrary *        m_Db;
//    guPortableMediaLibPanel *       m_LibPanel;
//    guPortableMediaPlayListPanel *  m_PlayListPanel;
//    guPortableMediaAlbumBrowser *   m_AlbumBrowserPanel;
//    int                             m_BaseCommand;
//    int                             m_VisiblePanels;
//
//  public :
//    guPortableMediaViewCtrl( guMainFrame * mainframe, guGIO_Mount * mount, int basecommand );
//    ~guPortableMediaViewCtrl();
//
//    int                              BaseCommand( void ) { return m_BaseCommand; }
//    int                              VisiblePanels( void ) { return m_VisiblePanels; }
//
//    guPortableMediaDevice *          MediaDevice( void ) { return m_PortableDevice; }
//    guPortableMediaLibrary *         Db( void ) { return m_Db; }
//    guPortableMediaLibPanel *        LibPanel( void ) { return m_LibPanel; }
//    guPortableMediaPlayListPanel *   PlayListPanel( void ) { return m_PlayListPanel; }
//    guPortableMediaAlbumBrowser *    AlbumBrowserPanel( void ) { return m_AlbumBrowserPanel; }
//
//    guPortableMediaLibPanel *        CreateLibPanel( wxWindow * parent, guPlayerPanel * playerpanel );
//    void                             DestroyLibPanel( void );
//    guPortableMediaAlbumBrowser *    CreateAlbumBrowser( wxWindow * parent, guPlayerPanel * playerpanel );
//    void                             DestroyAlbumBrowser( void );
//    guPortableMediaPlayListPanel *   CreatePlayListPanel( wxWindow * parent, guPlayerPanel * playerpanel );
//    void                             DestroyPlayListPanel( void );
//
//    wxString                         DeviceName( void ) { return m_PortableDevice->DeviceName(); }
//    bool                             IsMount( GMount * mount ) { return m_PortableDevice->IsMount( mount ); }
//    wxString                         IconString( void ) { return m_PortableDevice->IconString(); }
//
////    int                              CopyTo( const guTrack * track, wxString &filename ) { return m_LibPanel ? m_LibPanel->CopyTo( track, filename ) : wxNOT_FOUND; }
//
//};
//WX_DEFINE_ARRAY_PTR( guPortableMediaViewCtrl *, guPortableMediaViewCtrlArray );

// -------------------------------------------------------------------------------- //
class guMediaVieweriPodDevice : public guMediaViewerPortableDeviceBase
{
  protected :
    bool                    m_PendingSaving;

    virtual void            LoadMediaDb( void );

    virtual void            NormalizeTracks( guTrackArray * tracks, const bool isdrag = false );
//    void                    UpdateCollectionProperties( void );

  public :
    guMediaVieweriPodDevice( wxWindow * parent, guMediaCollection &mediacollection,
                          const int basecmd, guMainFrame * mainframe, const int mode,
                          guPlayerPanel * playerpanel, guGIO_Mount * mount );

    ~guMediaVieweriPodDevice();

    virtual void            HandleCommand( const int command );

    virtual void            UpdateLibrary( void );
    virtual void            CleanLibrary( void );

//    virtual wxArrayString   GetPaths( void );

    virtual void            CreateContextMenu( wxMenu * menu, const int windowid = wxNOT_FOUND );

    virtual bool            SetAlbumCover( const int albumid, const wxString &albumpath, wxImage * coverimg );
    virtual bool            SetAlbumCover( const int albumid, const wxString &albumpath, wxString &coverpath );
    virtual void            DeleteAlbumCover( const int albumid );
//    virtual void            DeleteAlbumCover( const wxArrayInt &albumids );

    virtual void            DeleteTracks( const guTrackArray * tracks );

//    virtual bool            CreateLibraryView( void );
////    virtual bool            CreateAlbumBrowserView( void );
////    virtual bool            CreateTreeView( void );
////    virtual bool            CreatePlayListView( void );

    virtual void            EditProperties( void );

    virtual int             CopyTo( const guTrack * track, wxString &filename );

    virtual wxImage *       GetAlbumCover( const int albumid, int &coverid, wxString &coverpath,
                                           const wxString &artistname = wxEmptyString, const wxString &albumname = wxEmptyString );

    virtual void            UpdateTracks( const guTrackArray &tracks, const guImagePtrArray &images,
                                const wxArrayString &lyrics, const wxArrayInt &changedflags );

};

}

#endif

#endif
// -------------------------------------------------------------------------------- //
