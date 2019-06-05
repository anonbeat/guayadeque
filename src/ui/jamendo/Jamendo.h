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
#ifndef __JAMENDO_H__
#define __JAMENDO_H__

#include "Config.h"
#include "LibPanel.h"
#include "DbLibrary.h"
#include "MediaViewer.h"
#include "Preferences.h"
#include "Settings.h"

#include <wx/string.h>
#include <wx/window.h>

namespace Guayadeque {

#define guJAMENDO_DATABASE_DUMP_URL         wxT( "http://img.jamendo.com/data/dbdump_artistalbumtrack.xml.gz" )
#define guJAMENDO_STREAM_FORMAT_MP3         wxT( "mp31" )
#define guJAMENDO_STREAM_FORMAT_OGG         wxT( "ogg2" )
#define guJAMENDO_FILE_STREAM_URL           wxT( "http://api.jamendo.com/get2/stream/track/redirect/?id=%u&streamencoding=" )
#define guJAMENDO_FILE_STREAM_MP3_URL       wxT( "http://api.jamendo.com/get2/stream/track/redirect/?id=%u&streamencoding=mp31" )
#define guJAMENDO_FILE_STREAM_OGG_URL       wxT( "http://api.jamendo.com/get2/stream/track/redirect/?id=%u&streamencoding=ogg2" )
#define guJAMENDO_COVER_DOWNLOAD_URL        wxT( "http://api.jamendo.com/get2/image/album/redirect/?id=%u&imagesize=%u" )
#define guJAMENDO_TORRENT_DOWNLOAD_URL      wxT( "http://api.jamendo.com/get2/bittorrent/file/plain/?album_id=%u&type=archive&class=" )
#define guJAMENDO_DOWNLOAD_FORMAT_MP3       wxT( "mp32" )
#define guJAMENDO_DOWNLOAD_FORMAT_OGG       wxT( "ogg3" )
#define guJAMENDO_DOWNLOAD_DIRECT           wxT( "http://www.jamendo.com/en/download/album/%u/do" )


#define guJAMENDO_ACTION_UPDATE             0   // Download the database and then upgrade
#define guJAMENDO_ACTION_UPGRADE            1   // Just refresh the tracks not updating the database

// -------------------------------------------------------------------------------- //
class guJamendoLibrary : public guDbLibrary
{
  public :
    guJamendoLibrary( const wxString &libpath );
    ~guJamendoLibrary();

    virtual void        UpdateArtistsLabels( const guArrayListItems &labelsets );
    virtual void        UpdateAlbumsLabels( const guArrayListItems &labelsets );
    virtual void        UpdateSongsLabels( const guArrayListItems &labelsets );

    void                CreateNewSong( guTrack * track );
};

class guMediaViewerJamendo;

// -------------------------------------------------------------------------------- //
class guJamendoUpdateThread : public wxThread
{
  private :
    guJamendoLibrary *              m_Db;
    guMediaViewerJamendo *          m_MediaViewer;
    guMainFrame *                   m_MainFrame;
    int                             m_GaugeId;
    int                             m_Action;
    wxArrayInt                      m_AllowedGenres;
    guTrack                         m_CurrentTrack;

    bool                UpgradeDatabase( void );

  protected :

  public :
    guJamendoUpdateThread( guMediaViewerJamendo * mediaviewer, const int action, int gaugeid );
    ~guJamendoUpdateThread();

    ExitCode Entry();

};

class guJamendoDownloadThread;

// -------------------------------------------------------------------------------- //
class guJamendoPanel : public guLibPanel
{
  protected :
    void                        OnDownloadAlbum( wxCommandEvent &event );
    void                        OnDownloadTrackAlbum( wxCommandEvent &event );

  public :
    guJamendoPanel( wxWindow * parent, guMediaViewer * mediaviewer );
    ~guJamendoPanel();

    guJamendoLibrary *          GetJamendoDb( void ) { return ( guJamendoLibrary * ) m_Db; }
};

// -------------------------------------------------------------------------------- //
class guJamendoAlbumBrowser : public guAlbumBrowser
{
  protected :
    void                        OnDownloadAlbum( wxCommandEvent &event );

  public :
    guJamendoAlbumBrowser( wxWindow * parent, guMediaViewer * mediaviewer );
    ~guJamendoAlbumBrowser();
};

// -------------------------------------------------------------------------------- //
class guJamendoTreePanel : public guTreeViewPanel
{
  protected :
    void                        OnDownloadAlbum( wxCommandEvent &event );
    void                        OnDownloadTrackAlbum( wxCommandEvent &event );

  public :
    guJamendoTreePanel( wxWindow * parent, guMediaViewer * mediaviewer );
    ~guJamendoTreePanel();

};

// -------------------------------------------------------------------------------- //
class guJamendoPlayListPanel : public guPlayListPanel
{
  protected :
    void                        OnDownloadTrackAlbum( wxCommandEvent &event );

  public :
    guJamendoPlayListPanel( wxWindow * parent, guMediaViewer * mediaviewer );
    ~guJamendoPlayListPanel();

};

// -------------------------------------------------------------------------------- //
class guJamendoDownloadThread : public wxThread
{
  private :
    guJamendoLibrary *      m_Db;
    guMediaViewerJamendo *  m_MediaViewer;
    wxArrayInt              m_Covers;
    wxMutex                 m_CoversMutex;
    wxArrayInt              m_Albums;
    wxMutex                 m_AlbumsMutex;

  protected :

  public :
    guJamendoDownloadThread( guMediaViewerJamendo * jamendopanel );
    ~guJamendoDownloadThread();

    void AddAlbum( const int albumid, const bool iscover = true );
    void AddAlbums( const wxArrayInt &albums, const bool iscover = true );

    ExitCode Entry();

};

// -------------------------------------------------------------------------------- //
class guMediaViewerJamendo : public guMediaViewer
{
  protected :
    guJamendoDownloadThread *   m_DownloadThread;
    wxMutex                     m_DownloadThreadMutex;


    virtual void            LoadMediaDb( void );
    virtual void            OnConfigUpdated( wxCommandEvent &event );

    void                    OnCoverDownloaded( wxCommandEvent &event );
    void                    OnUpdateFinished( wxCommandEvent &event );

    void                    EndUpdateThread( void );
    void                    EndDownloadThread( void );

  public :
    guMediaViewerJamendo( wxWindow * parent, guMediaCollection &mediacollection,
                          const int basecmd, guMainFrame * mainframe, const int mode,
                          guPlayerPanel * playerpanel );
    ~guMediaViewerJamendo();

    virtual wxImage *       GetAlbumCover( const int albumid, int &coverid, wxString &coverpath,
                                const wxString &artistname = wxEmptyString, const wxString &albumname = wxEmptyString );

    virtual void            UpdateLibrary( void );
    virtual void            UpgradeLibrary( void );
    virtual void            NormalizeTracks( guTrackArray * tracks, const bool isdrag = false );

    void                    AddDownload( const int albumid, const bool iscover = true );
    void                    AddDownloads( const wxArrayInt &albumids, const bool iscover = true );

    virtual void            DownloadAlbumCover( const int albumid );

    virtual void            DownloadAlbums( const wxArrayInt &albums, const bool istorrent );

    virtual void            CreateContextMenu( wxMenu * menu, const int windowid = wxNOT_FOUND );

    virtual bool            CreateLibraryView( void );
    virtual bool            CreateAlbumBrowserView( void );
    virtual bool            CreateTreeView( void );
    virtual bool            CreatePlayListView( void );

    virtual wxString        GetCoverName( const int albumid );
    virtual void            SelectAlbumCover( const int albumid );

    virtual bool            FindMissingCover( const int albumid, const wxString &artistname,
                                              const wxString &albumname, const wxString &albumpath );

    virtual void            EditProperties( void );

    friend class guJamendoDownloadThread;
    friend class guJamendoUpdateThread;
};

}

#endif
// -------------------------------------------------------------------------------- //
