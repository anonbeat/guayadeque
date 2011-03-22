// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2011 J.Rios
//	anonbeat@gmail.com
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

#define guMAGNATUNE_PARTNER_ID                wxT( "guayadeque" )
#define guMAGNATUNE_DOWNLOAD_URL              wxT( "http://%s:%s@download.magnatune.com/buy/membership_free_dl_xml.php?sku=%s&id=guayadeque" )

enum guMagnatune_Membership {
    guMAGNATUNE_MEMBERSHIP_FREE,
    guMAGNATUNE_MEMBERSHIP_STREAM,
    guMAGNATUNE_MEMBERSHIP_DOWNLOAD
};

// -------------------------------------------------------------------------------- //
class guMagnatuneLibrary : public guDbLibrary
{
  public :
    guMagnatuneLibrary( const wxString &libpath );
    ~guMagnatuneLibrary();

    virtual void        UpdateArtistsLabels( const guArrayListItems &labelsets );
    virtual void        UpdateAlbumsLabels( const guArrayListItems &labelsets );
    virtual void        UpdateSongsLabels( const guArrayListItems &labelsets );

    void                CreateNewSong( guTrack * track, const wxString &albumsku, const wxString &coverlink );
    int                 GetTrackId( const wxString &url, guTrack * track = NULL );

    wxString            GetAlbumSku( const int trackid );
};

// -------------------------------------------------------------------------------- //
class guMagnatuneUpdateThread : public wxThread
{
  private :
    guMagnatuneLibrary *    m_Db;
    guMainFrame *           m_MainFrame;
    int                     m_GaugeId;
    int                     m_Action;
    wxSortedArrayString     m_GenreList;
    wxArrayString           m_AllowedGenres;
    guTrack                 m_CurrentTrack;
    wxString                m_AlbumSku;
    wxString                m_CoverLink;

    bool                UpdateDatabase( void );
    void                ReadMagnatuneXmlTrack( wxXmlNode * xmlnode );
    void                ReadMagnatuneXmlAlbum( wxXmlNode * xmlnode );
    void                AddGenres( const wxString &genre );


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
    int                         m_Membership;
    wxString                    m_UserName;
    wxString                    m_Password;
    guMagnatuneUpdateThread *   m_UpdateThread;
    wxMutex                     m_UpdateThreadMutex;

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

    void                        DownloadAlbums( const wxArrayInt &albumids );

  public :
    guMagnatunePanel( wxWindow * parent, guMagnatuneLibrary * db, guPlayerPanel * playerpanel, const wxString &prefix = wxT( "Jam" ) );
    ~guMagnatunePanel();

    virtual void                InitPanelData();

    guMagnatuneLibrary *        GetMagnatuneDb( void ) { return ( guMagnatuneLibrary * ) m_Db; }
    wxImage *                   GetAlbumCover( const int albumid, const wxString &artist, const wxString &album, wxString &coverpath );
    void                        AddDownload( const int albumid, const wxString &artist, const wxString &album );

    void                        EndUpdateThread( void );


};

// -------------------------------------------------------------------------------- //
class guMagnatuneDownloadThread : public wxThread
{
  protected :
    guMagnatuneLibrary *    m_Db;
    guMagnatunePanel *      m_MagnatunePanel;
    wxString                m_ArtistName;
    wxString                m_AlbumName;
    int                     m_AlbumId;

  public :
    guMagnatuneDownloadThread( guMagnatunePanel * jamendopanel, const int albumid,
                                const wxString &artist, const wxString &album );
    ~guMagnatuneDownloadThread();

    ExitCode Entry();

};

#endif
// -------------------------------------------------------------------------------- //
