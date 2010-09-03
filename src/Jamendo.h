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
#ifndef JAMENDO_H
#define JAMENDO_H

#include "Config.h"
#include "LibPanel.h"
#include "DbLibrary.h"
#include "Preferences.h"

#include <wx/string.h>
#include <wx/window.h>

#define guJAMENDO_DATABASE_DUMP_URL         wxT( "http://img.jamendo.com/data/dbdump_artistalbumtrack.xml.gz" )
#define guJAMENDO_FILE_FORMAT_MP3           wxT( "mp31" )
#define guJAMENDO_FILE_FORMAT_OGG           wxT( "ogg2" )
#define guJAMENDO_FILE_STREAM_URL           wxT( "http://api.jamendo.com/get2/stream/track/redirect/?id=%u&streamencoding=" )
#define guJAMENDO_FILE_STREAM_MP3_URL       wxT( "http://api.jamendo.com/get2/stream/track/redirect/?id=%u&streamencoding=mp31" )
#define guJAMENDO_FILE_STREAM_OGG_URL       wxT( "http://api.jamendo.com/get2/stream/track/redirect/?id=%u&streamencoding=ogg2" )
#define guJAMENDO_COVER_DOWNLOAD_URL        wxT( "http://api.jamendo.com/get2/image/album/redirect/?id=%u&imagesize=%u" )

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

// -------------------------------------------------------------------------------- //
class guJamendoUpdateThread : public wxThread
{
  private :
    guJamendoLibrary *              m_Db;
    guMainFrame *                   m_MainFrame;
    int                             m_GaugeId;
    int                             m_Action;
    wxArrayInt                      m_AllowedGenres;
    guTrack                         m_CurrentTrack;

    bool                UpdateDatabase( void );

  protected :

  public :
    guJamendoUpdateThread( guJamendoLibrary * db, const int action, int gaugeid );
    ~guJamendoUpdateThread();

    ExitCode Entry();

};

// -------------------------------------------------------------------------------- //
class guJamendoPanel : public guLibPanel
{
  protected :
    virtual void        NormalizeTracks( guTrackArray * tracks );
    virtual void        CreateContextMenu( wxMenu * menu );
    void                OnEditSetup( wxCommandEvent &event );
    void                OnUpdate( wxCommandEvent &event );
    void                OnUpgrade( wxCommandEvent &event );

    void                OnConfigUpdated( wxCommandEvent &event );
    void                OnCoverDownloaded( wxCommandEvent &event );

    void                OnAlbumDownloadCoverClicked( wxCommandEvent &event );
    void                OnAlbumSelectCoverClicked( wxCommandEvent &event );

  public :
    guJamendoPanel( wxWindow * parent, guJamendoLibrary * db, guPlayerPanel * playerpanel, const wxString &prefix = wxT( "Jam" ) );
    ~guJamendoPanel();

    guJamendoLibrary *  GetJamendoDb( void ) { return ( guJamendoLibrary * ) m_Db; }
    wxImage *           GetAlbumCover( const int albumid, wxString &coverpath );
    void                DownloadCover( const int albumid );

};

// -------------------------------------------------------------------------------- //
class guJamendoDownloadCoverThread : public wxThread
{
  private :
    guJamendoLibrary *  m_Db;
    guJamendoPanel *    m_JamendoPanel;
    wxArrayInt          m_Albums;
    wxMutex             m_AlbumsMutex;

  protected :

  public :
    guJamendoDownloadCoverThread( guJamendoPanel * jamendopanel );
    ~guJamendoDownloadCoverThread();

    void AddAlbum( const int albumid );

    ExitCode Entry();

};




#endif
// -------------------------------------------------------------------------------- //
