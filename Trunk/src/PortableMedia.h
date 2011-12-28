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
#ifndef PORTABLEMEDIA_H
#define PORTABLEMEDIA_H

#include "AlbumBrowser.h"
#include "Config.h"
#include "DbLibrary.h"
#include "GIO_Volume.h"
#include "LibPanel.h"
#include "MediaViewer.h"
#include "PlayListPanel.h"
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

#ifdef WITH_LIBGPOD_SUPPORT

#include <gpod/itdb.h>

#endif

#define guPORTABLEMEDIA_AUDIO_FORMAT_MP3        ( 1 << 0 )
#define guPORTABLEMEDIA_AUDIO_FORMAT_OGG        ( 1 << 1 )
#define guPORTABLEMEDIA_AUDIO_FORMAT_FLAC       ( 1 << 2 )
#define guPORTABLEMEDIA_AUDIO_FORMAT_AAC        ( 1 << 3 )
#define guPORTABLEMEDIA_AUDIO_FORMAT_WMA        ( 1 << 4 )
#define guPORTABLEMEDIA_AUDIO_FORMAT_ALL        0xFFFFFFFF

#define guPORTABLEMEDIA_PLAYLIST_FORMAT_M3U     ( 1 << 0 )
#define guPORTABLEMEDIA_PLAYLIST_FORMAT_PLS     ( 1 << 1 )
#define guPORTABLEMEDIA_PLAYLIST_FORMAT_XSPF    ( 1 << 2 )
#define guPORTABLEMEDIA_PLAYLIST_FORMAT_ASX     ( 1 << 3 )

#define guPORTABLEMEDIA_COVER_FORMAT_NONE       0
#define guPORTABLEMEDIA_COVER_FORMAT_EMBEDDED   ( 1 << 0 )
#define guPORTABLEMEDIA_COVER_FORMAT_JPEG       ( 1 << 1 )
#define guPORTABLEMEDIA_COVER_FORMAT_PNG        ( 1 << 2 )
#define guPORTABLEMEDIA_COVER_FORMAT_BMP        ( 1 << 3 )
#define guPORTABLEMEDIA_COVER_FORMAT_GIF        ( 1 << 4 )


enum guPortableMediaTranscodeScope {
    guPORTABLEMEDIA_TRANSCODE_SCOPE_NOT_SUPPORTED,
    guPORTABLEMEDIA_TRANSCODE_SCOPE_ALWAYS
};

enum guPortableMediaType {
    guPORTABLE_MEDIA_TYPE_MSC,      // Regular Mass storage devices
    guPORTABLE_MEDIA_TYPE_MTP,      // Media Transfer Protocol devices
    guPORTABLE_MEDIA_TYPE_IPOD      // iPods
};


// -------------------------------------------------------------------------------- //
int GetPortableMediaType( const wxString &path );

// -------------------------------------------------------------------------------- //
int inline guGetMp3QualityBitRate( int quality )
{
    int guPortableMediaMp3QualityBitrate[] = {
        128,
        320,
        256,
        192,
        160,
        128,
        96,
        64
    };

    return guPortableMediaMp3QualityBitrate[ quality ];
}

// -------------------------------------------------------------------------------- //
int inline guGetOggQualityBitRate( int quality )
{
    int guPortableMediaOggQualityBitrate[] = {
        110,
        240,
        160,
        140,
        120,
        110,
        96,
        70
    };
    return guPortableMediaOggQualityBitrate[ quality ];
}

// -------------------------------------------------------------------------------- //
class guPortableMediaDevice
{
  protected :
    guGIO_Mount *   m_Mount;
    //
    wxString        m_Id;

    int             m_Type;
    wxString        m_Pattern;
    int             m_AudioFormats;
    int             m_TranscodeFormat;
    int             m_TranscodeScope;
    int             m_TranscodeQuality;
    wxString        m_AudioFolders;
    int             m_PlaylistFormats;
    wxString        m_PlaylistFolder;
    int             m_CoverFormats;
    wxString        m_CoverName;
    int             m_CoverSize;

    wxLongLong      m_DiskSize;
    wxLongLong      m_DiskFree;

  public :
    guPortableMediaDevice( guGIO_Mount * mount );
    ~guPortableMediaDevice();

    void                    WriteConfig( void );

    wxString                MountPath( void ) { return m_Mount->GetMountPath(); }
    wxString                Id( void ) { return m_Id; }
    void                    SetId( const wxString &id ) { m_Id = id; }
    int                     Type( void ) { return m_Type; }
    void                    SetType( const int type ) { m_Type = type; }
    wxString                DevicePath( void ) { return m_Mount->GetName() + wxT( "-" ) + m_Id; }
    wxString                DeviceName( void ) { return m_Mount->GetName(); }
    double                  DiskSize( void ) { return m_DiskSize.ToDouble(); }
    double                  DiskFree( void ) { return m_DiskFree.ToDouble(); }
    void                    UpdateDiskFree( void ) { wxGetDiskSpace( m_Mount->GetMountPath(), &m_DiskSize, &m_DiskFree ); }
    wxString                IconString( void ) { return m_Mount->IconString(); }

    wxString                Pattern( void ) { return m_Pattern; }
    void                    SetPattern( const wxString &pattern ) { m_Pattern = pattern; }

    void                    SetAudioFormats( const int formats ) { m_AudioFormats = formats; }
    int                     AudioFormats( void ) { return m_AudioFormats; }
    wxString                AudioFormatsStr( const int formats );
    wxString                AudioFormatsStr( void ) { return AudioFormatsStr( m_AudioFormats ); }

    void                    SetTranscodeFormat( const int format ) { m_TranscodeFormat = format; }
    int                     TranscodeFormat( void ) { return m_TranscodeFormat; }

    void                    SetTranscodeScope( const int scope ) { m_TranscodeScope = scope; }
    int                     TranscodeScope( void ) { return m_TranscodeScope; }

    void                    SetTranscodeQuality( const int quality ) { m_TranscodeQuality = quality; }
    int                     TranscodeQuality( void ) { return m_TranscodeQuality; }

    void                    SetAudioFolders( const wxString &folders ) { m_AudioFolders = folders; }
    wxString                AudioFolders( void ) { return m_AudioFolders; }

    void                    SetPlaylistFormats( const int format ) { m_PlaylistFormats = format; }
    int                     PlaylistFormats( void ) { return m_PlaylistFormats; }
    wxString                PlaylistFormatsStr( const int formats );
    wxString                PlaylistFormatsStr( void ) { return PlaylistFormatsStr( m_PlaylistFormats ); }

    void                    SetPlaylistFolder( const wxString &folder ) { m_PlaylistFolder = folder; }
    wxString                PlaylistFolder( void ) { return m_PlaylistFolder; }

    void                    SetCoverFormats( const int formats ) { m_CoverFormats = formats; }
    int                     CoverFormats( void ) { return m_CoverFormats; }
    wxString                CoverFormatsStr( const int formats );
    wxString                CoverFormatsStr( void ) { return CoverFormatsStr( m_CoverFormats ); }

    void                    SetCoverName( const wxString &name ) { m_CoverName = name; }
    wxString                CoverName( void ) { return m_CoverName; }

    void                    SetCoverSize( const int size ) { m_CoverSize = size; }
    int                     CoverSize( void ) { return m_CoverSize; }

//    int                     PanelActive( void ) { return m_Mount->PanelActive(); }
//    void                    SetPanelActive( const int panelactive ) { m_Mount->SetPanelActive( panelactive ); }

    bool                    IsMount( GMount * mount ) { return m_Mount->IsMount( mount ); }
    bool                    CanUnmount( void ) { return m_Mount->CanUnmount(); }
    void                    Unmount( void ) { m_Mount->Unmount(); }

};

// -------------------------------------------------------------------------------- //
class guListCheckOptionsDialog : public wxDialog
{
  protected :
    wxCheckListBox *    m_CheckListBox;

  public :
    guListCheckOptionsDialog( wxWindow * parent, const wxString &title, const wxArrayString &itemlist, const wxArrayInt &selection );
    ~guListCheckOptionsDialog();

    int       GetSelectedItems( wxArrayInt &selection );

};


// -------------------------------------------------------------------------------- //
class guPortableMediaProperties : public wxDialog
{
  protected:
    guPortableMediaDevice *     m_PortableDevice;

    wxStaticText *              m_NameText;
    wxStaticText *              m_MountPathText;
    wxGauge *                   m_UsedGauge;
    wxStaticText *              m_UsedLabel;

    wxTextCtrl *                m_NamePatternText;

    wxTextCtrl *                m_AudioFolderText;
    wxButton *                  m_AudioFolderBtn;
    wxTextCtrl *                m_AudioFormatText;
    wxButton *                  m_AudioFormatBtn;

    wxChoice *                  m_TransFormatChoice;
    wxChoice *                  m_TransScopeChoice;

    wxChoice *                  m_TransQualityChoice;

    wxTextCtrl *                m_PlaylistFormatText;
    wxButton *                  m_PlaylistFormatBtn;
    wxTextCtrl *                m_PlaylistFolderText;
    wxButton *                  m_PlaylistFolderBtn;
    wxTextCtrl *                m_CoverFormatText;
    wxButton *                  m_CoverFormatBtn;
    wxTextCtrl *                m_CoverNameText;

    wxTextCtrl *                m_CoverSizeText;

    int                         m_AudioFormats;
    int                         m_PlaylistFormats;
    int                         m_CoverFormats;

    bool                        m_IsIpod;

    void                        OnAudioFolderBtnClick( wxCommandEvent& event );
    void                        OnAudioFormatBtnClick( wxCommandEvent& event );
    void                        OnTransFormatChanged( wxCommandEvent& event );
    void                        OnPlaylistFormatBtnClick( wxCommandEvent& event );
    void                        OnPlaylistFolderBtnClick( wxCommandEvent& event );
    void                        OnCoverFormatBtnClick( wxCommandEvent& event );

  public:

	guPortableMediaProperties( wxWindow * parent, guPortableMediaDevice * mediadevice );
	~guPortableMediaProperties();

    void                        WriteConfig( void );
};

// -------------------------------------------------------------------------------- //
class guPortableMediaLibrary : public guDbLibrary
{
  protected :
    guPortableMediaDevice * m_PortableDevice;

  public :
    guPortableMediaLibrary( const wxString &libpath, guPortableMediaDevice * portablemediadevice );
    ~guPortableMediaLibrary();

    virtual void            DeletePlayList( const int plid );

    virtual void            UpdateStaticPlayListFile( const int plid );

};
WX_DEFINE_ARRAY_PTR( guPortableMediaLibrary *, guPortableMediaLibraryArray );

class guMediaViewerPortableDevice;

// -------------------------------------------------------------------------------- //
class guPortableMediaLibPanel : public guLibPanel
{
  protected :
    guPortableMediaDevice *     m_PortableDevice;

//    virtual void                CreateContextMenu( wxMenu * menu, const int windowid = 0 );

//    virtual void                OnPortableLibraryUpdate( wxCommandEvent &event );
//    virtual void                OnPortableUnmount( wxCommandEvent &event );
//    virtual void                OnPortableProperties( wxCommandEvent &event );
//
//    virtual bool                OnDropFiles( const wxArrayString &filenames );
//
//    virtual void                UpdatePlaylists( void );

  public :
    guPortableMediaLibPanel( wxWindow * parent, guMediaViewerPortableDevice * mediaviewer );
    ~guPortableMediaLibPanel();

    wxString                    IconString( void ) { return m_PortableDevice->IconString(); }
    virtual wxString            GetName( void );

//    virtual wxArrayString       GetLibraryPaths( void );
//    virtual wxString            GetPlaylistPath( void );

    guPortableMediaDevice *     PortableMediaDevice( void ) { return m_PortableDevice; }
    virtual void                SetPortableMediaDevice( guPortableMediaDevice * portablemediadevice ) { m_PortableDevice = portablemediadevice; }

//    virtual int                 LastUpdate( void );
//    virtual void                SetLastUpdate( int lastupdate = wxNOT_FOUND );

//    virtual wxArrayString       GetCoverSearchWords( void );

//    virtual wxString            GetCoverName( void );
//    virtual int                 GetCoverMaxSize( void );

//    virtual void                DoUpdate( const bool forced = false );

//    virtual int                 CopyTo( const guTrack * track, wxString &filename ) { return wxNOT_FOUND; }

};
WX_DEFINE_ARRAY_PTR( guPortableMediaLibPanel *, guPortableMediaPanelArray );

//// -------------------------------------------------------------------------------- //
//class guPortableMediaAlbumBrowser : public guAlbumBrowser
//{
//  protected :
//    guPortableMediaLibPanel * m_LibPanel;
//
//    virtual void            NormalizeTracks( guTrackArray * tracks, const bool isdrag = false );
//
//    virtual void            OnAlbumSelectName( const int albumid );
//    virtual void            OnArtistSelectName( const int artistid );
//
//  public :
//    guPortableMediaAlbumBrowser( wxWindow * parent, guPortableMediaLibrary * db, guPlayerPanel * playerpanel, guPortableMediaLibPanel * libpanel );
//    ~guPortableMediaAlbumBrowser();
//
//
//    virtual void            OnAlbumDownloadCoverClicked( const int albumid );
//    virtual void            OnAlbumSelectCoverClicked( const int albumid );
//
//    virtual int             GetContextMenuFlags( void ) { return m_LibPanel->GetContextMenuFlags(); }
//
//};

//// -------------------------------------------------------------------------------- //
//class guPortableMediaPlayListPanel : public guPlayListPanel
//{
//  protected :
//    guPortableMediaLibPanel * m_LibPanel;
//
//    virtual void        NormalizeTracks( guTrackArray * tracks, const bool isdrag = false );
//
//    virtual void        SendPlayListUpdatedEvent( void );
//
//  public :
//    guPortableMediaPlayListPanel( wxWindow * parent, guPortableMediaLibrary * db, guPlayerPanel * playerpanel, guPortableMediaLibPanel * libpanel );
//    ~guPortableMediaPlayListPanel();
//
//};

// -------------------------------------------------------------------------------- //
class guMediaViewerPortableDeviceBase : public guMediaViewer
{
  protected :
    guPortableMediaDevice * m_PortableDevice;

  public :
    guMediaViewerPortableDeviceBase( wxWindow * parent, guMediaCollection &mediacollection,
                          const int basecmd, guMainFrame * mainframe, const int mode,
                          guPlayerPanel * playerpanel, guGIO_Mount * mount );

    ~guMediaViewerPortableDeviceBase();

    guPortableMediaDevice * GetPortableMediaDevice( void ) { return m_PortableDevice; }

    virtual wxString        GetCoverName( const int albumid );
    virtual int             GetCoverType( void ) { return m_PortableDevice->CoverFormats(); }
    virtual int             GetCoverMaxSize( void ) { return m_PortableDevice->CoverSize(); }

    virtual bool            IsMount( GMount * mount ) { return m_PortableDevice->IsMount( mount ); }

    virtual wxString        AudioPath( void );
    virtual wxString        Pattern( void ) { return m_PortableDevice->Pattern(); }
    virtual int             AudioFormats( void ) { return m_PortableDevice->AudioFormats(); }
    virtual int             TranscodeFormat( void ) { return m_PortableDevice->TranscodeFormat(); }
    virtual int             TranscodeScope( void ) { return m_PortableDevice->TranscodeScope(); }
    virtual int             TranscodeQuality( void ) { return m_PortableDevice->TranscodeQuality(); }
    virtual int             PlaylistFormats( void ) { return m_PortableDevice->PlaylistFormats(); }
    virtual wxString        PlaylistPath( void ) { return m_PortableDevice->MountPath() + m_PortableDevice->PlaylistFolder(); }
};

// -------------------------------------------------------------------------------- //
class guMediaViewerPortableDevice : public guMediaViewerPortableDeviceBase
{
  protected :
    virtual void            LoadMediaDb( void );

    void                    UpdateCollectionProperties( void );

  public :
    guMediaViewerPortableDevice( wxWindow * parent, guMediaCollection &mediacollection,
                          const int basecmd, guMainFrame * mainframe, const int mode,
                          guPlayerPanel * playerpanel, guGIO_Mount * mount );

    ~guMediaViewerPortableDevice();

    virtual void            HandleCommand( const int command );

    virtual wxArrayString   GetPaths( void );

    virtual void            CreateContextMenu( wxMenu * menu, const int windowid = wxNOT_FOUND );

    virtual bool            CreateLibraryView( void );

    virtual void            EditProperties( void );

};

#endif
// -------------------------------------------------------------------------------- //
