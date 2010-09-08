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
#ifndef MAGNATUNE_H
#define MAGNATUNE_H

#include "Config.h"
#include "LibPanel.h"
#include "DbLibrary.h"
#include "Preferences.h"

#include <wx/string.h>
#include <wx/window.h>

#define guMAGNATUNE_DATABASE_DUMP_URL         wxT( "http://he3.magnatune.com/info/album_info_xml.gz" )
//#define guMAGNATUNE_FILE_STREAM_URL
#define guMAGNATUNE_STREAM_FORMAT_MP3         wxT( ".mp3" )
#define guMAGNATUNE_STREAM_FORMAT_OGG         wxT( ".ogg" )

#define guMAGNATUNE_ACTION_UPDATE             0   // Download the database and then upgrade
#define guMAGNATUNE_ACTION_UPGRADE            1   // Just refresh the tracks not updating the database

// -------------------------------------------------------------------------------- //
class guMagnatuneLibrary : public guDbLibrary
{
  public :
    guMagnatuneLibrary( const wxString &libpath );
    ~guMagnatuneLibrary();

    virtual void        UpdateArtistsLabels( const guArrayListItems &labelsets );
    virtual void        UpdateAlbumsLabels( const guArrayListItems &labelsets );
    virtual void        UpdateSongsLabels( const guArrayListItems &labelsets );

    void                CreateNewSong( guTrack * track );
    int                 GetTrackId( const wxString &url, guTrack * track = NULL );
};

// -------------------------------------------------------------------------------- //
class guMagnatuneUpdateThread : public wxThread
{
  private :
    guMagnatuneLibrary *              m_Db;
    guMainFrame *                   m_MainFrame;
    int                             m_GaugeId;
    int                             m_Action;
    wxArrayString                   m_AllowedGenres;
    guTrack                         m_CurrentTrack;

    bool                UpdateDatabase( void );

  protected :

  public :
    guMagnatuneUpdateThread( guMagnatuneLibrary * db, const int action, int gaugeid );
    ~guMagnatuneUpdateThread();

    ExitCode Entry();

};

class guMagnatuneDownloadThread;

// -------------------------------------------------------------------------------- //
class guMagnatunePanel : public guLibPanel
{
  protected :
    guMagnatuneUpdateThread *     m_UpdateThread;
    wxMutex                     m_UpdateThreadMutex;

    guMagnatuneDownloadThread *   m_DownloadThread;
    wxMutex                     m_DownloadThreadMutex;

    virtual void                NormalizeTracks( guTrackArray * tracks, const bool isdrag = false );
    virtual void                CreateContextMenu( wxMenu * menu, const int windowid = 0 );
    void                        OnEditSetup( wxCommandEvent &event );

    void                        OnUpdate( wxCommandEvent &event );
    void                        OnUpgrade( wxCommandEvent &event );
    void                        StartUpdateTracks( const int action );

    void                        OnConfigUpdated( wxCommandEvent &event );
    void                        OnCoverDownloaded( wxCommandEvent &event );

    void                        OnAlbumDownloadCoverClicked( wxCommandEvent &event );
    void                        OnAlbumSelectCoverClicked( wxCommandEvent &event );

    void                        OnDownloadAlbum( wxCommandEvent &event );
    void                        OnDownloadTrackAlbum( wxCommandEvent &event );

  public :
    guMagnatunePanel( wxWindow * parent, guMagnatuneLibrary * db, guPlayerPanel * playerpanel, const wxString &prefix = wxT( "Jam" ) );
    ~guMagnatunePanel();

    guMagnatuneLibrary *          GetMagnatuneDb( void ) { return ( guMagnatuneLibrary * ) m_Db; }
    wxImage *                   GetAlbumCover( const int albumid, wxString &coverpath );
    void                        AddDownload( const int albumid, const bool iscover = true );
    void                        AddDownloads( wxArrayInt &albumids, const bool iscover = true );

    void                        EndUpdateThread( void );
    void                        EndDownloadThread( void );


};

// -------------------------------------------------------------------------------- //
class guMagnatuneDownloadThread : public wxThread
{
  private :
    guMagnatuneLibrary *  m_Db;
    guMagnatunePanel *    m_MagnatunePanel;
    wxArrayInt          m_Covers;
    wxMutex             m_CoversMutex;
    wxArrayInt          m_Albums;
    wxMutex             m_AlbumsMutex;

  protected :

  public :
    guMagnatuneDownloadThread( guMagnatunePanel * jamendopanel );
    ~guMagnatuneDownloadThread();

    void AddAlbum( const int albumid, const bool iscover = true );
    void AddAlbums( wxArrayInt &albums, const bool iscover = true );

    ExitCode Entry();

};




#endif
// -------------------------------------------------------------------------------- //
