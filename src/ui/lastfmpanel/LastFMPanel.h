// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2023 J.Rios anonbeat@gmail.com
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
#ifndef __LASTFMPANEL_H__
#define __LASTFMPANEL_H__

#include "AuiNotebook.h"
#include "DbCache.h"
#include "LastFM.h"
#include "PlayerPanel.h"
#include "ThreadArray.h"
#include "TrackChangeInfo.h"

#include <wx/image.h>
#include <wx/panel.h>
#include <wx/html/htmlwin.h>
#include <wx/hyperlink.h>

namespace Guayadeque {

#define GULASTFMINFO_MAXITEMS  12

// -------------------------------------------------------------------------------- //
class guHtmlWindow  : public wxHtmlWindow
{
  protected :
    void    OnChangedSize( wxSizeEvent &event );
    void    OnScrollTo( wxCommandEvent &event );

  public :
    guHtmlWindow( wxWindow * parent, wxWindowID id = wxNOT_FOUND, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = wxHW_DEFAULT_STYLE );
    ~guHtmlWindow();

};

// -------------------------------------------------------------------------------- //
class guLastFMInfo
{
  public:
    int                     m_Index;
    wxImage *               m_Image;
    wxString                m_ImageUrl;

    guLastFMInfo() {}

    guLastFMInfo( int index, wxImage * image = NULL )
    {
        m_Index = index;
        m_Image = image;
    }

    ~guLastFMInfo()
    {
        if( m_Image )
            delete m_Image;
    }
};
WX_DECLARE_OBJARRAY(guLastFMInfo, guLastFMInfoArray);

// -------------------------------------------------------------------------------- //
class guLastFMArtistInfo : public guLastFMInfo
{
  public:
    guArtistInfo *  m_Artist;
    int             m_ArtistId;

    guLastFMArtistInfo() { m_Artist = NULL; m_ArtistId = wxNOT_FOUND; }
    guLastFMArtistInfo( int index, wxImage * image = NULL, guArtistInfo * artist = NULL ) :
      guLastFMInfo( index, image )
    {
        m_Artist = artist;
        m_ArtistId = wxNOT_FOUND;
    }

    ~guLastFMArtistInfo()
    {
        if( m_Artist )
            delete m_Artist;
    }
};


// -------------------------------------------------------------------------------- //
class guLastFMSimilarArtistInfo : public guLastFMInfo
{
  public:
    guSimilarArtistInfo *   m_Artist;
    int                     m_ArtistId;

    guLastFMSimilarArtistInfo() { m_Artist = NULL; m_ArtistId = wxNOT_FOUND; }

    guLastFMSimilarArtistInfo( int index, wxImage * image = NULL,
                  guSimilarArtistInfo * artist = NULL ) :
            guLastFMInfo( index, image )
    {
        m_Artist = artist;
        m_ArtistId = wxNOT_FOUND;
    };

    ~guLastFMSimilarArtistInfo()
    {
        if( m_Artist )
            delete m_Artist;
    };
};
WX_DECLARE_OBJARRAY(guLastFMSimilarArtistInfo, guLastFMSimilarArtistInfoArray);

// -------------------------------------------------------------------------------- //
class guLastFMAlbumInfo : public guLastFMInfo
{
  public:
    guAlbumInfo  *  m_Album;
    int             m_AlbumId;

    guLastFMAlbumInfo() { m_Album = NULL; m_AlbumId = wxNOT_FOUND; }

    guLastFMAlbumInfo( int index, wxImage * image = NULL, guAlbumInfo * album = NULL ) :
            guLastFMInfo( index, image )
    {
        m_Album = album;
        m_AlbumId = wxNOT_FOUND;
    };

    ~guLastFMAlbumInfo()
    {
        if( m_Album )
            delete m_Album;
    };
};
WX_DECLARE_OBJARRAY(guLastFMAlbumInfo, guLastFMAlbumInfoArray);

// -------------------------------------------------------------------------------- //
class guLastFMTrackInfo : public guLastFMInfo
{
  public:
    guSimilarTrackInfo *   m_Track;
    int                    m_TrackId;
    int                    m_ArtistId;

    guLastFMTrackInfo() { m_Track = NULL; m_TrackId = wxNOT_FOUND; }

    guLastFMTrackInfo( int index, wxImage * image = NULL,
                  guSimilarTrackInfo * track = NULL ) :
            guLastFMInfo( index, image )
    {
        m_Track = track;
        m_TrackId = wxNOT_FOUND;
        m_ArtistId = wxNOT_FOUND;
    }

    ~guLastFMTrackInfo()
    {
        if( m_Track )
            delete m_Track;
    }
};
WX_DECLARE_OBJARRAY(guLastFMTrackInfo, guLastFMTrackInfoArray);

// -------------------------------------------------------------------------------- //
class guLastFMTopTrackInfo : public guLastFMInfo
{
  public:
    guTopTrackInfo *       m_TopTrack;
    int                    m_TrackId;
    int                    m_ArtistId;

    guLastFMTopTrackInfo() { m_TopTrack = NULL; m_TrackId = wxNOT_FOUND; }

    guLastFMTopTrackInfo( int index, wxImage * image = NULL,
                  guTopTrackInfo * track = NULL ) :
            guLastFMInfo( index, image )
    {
        m_TopTrack = track;
        m_TrackId = wxNOT_FOUND;
        m_ArtistId = wxNOT_FOUND;
    };

    ~guLastFMTopTrackInfo()
    {
        if( m_TopTrack )
            delete m_TopTrack;
    };
};
WX_DECLARE_OBJARRAY(guLastFMTopTrackInfo, guLastFMTopTrackInfoArray);

// -------------------------------------------------------------------------------- //
class guLastFMEventInfo : public guLastFMInfo
{
  public:
    guEventInfo *       m_Event;

    guLastFMEventInfo() { m_Event = NULL; }

    guLastFMEventInfo( int index, wxImage * image = NULL,
                  guEventInfo * event = NULL ) :
            guLastFMInfo( index, image )
    {
        m_Event = event;
    };

    ~guLastFMEventInfo()
    {
        if( m_Event )
            delete m_Event;
    };
};
WX_DECLARE_OBJARRAY(guLastFMEventInfo, guLastFMEventInfoArray);

class guLastFMPanel;

// -------------------------------------------------------------------------------- //
class guFetchLastFMInfoThread : public wxThread
{
  protected :
    guLastFMPanel *         m_LastFMPanel;
    guThreadArray           m_DownloadThreads;
	wxMutex                 m_DownloadThreadsMutex;

    void                    WaitDownloadThreads( void );

  public :
    guFetchLastFMInfoThread( guLastFMPanel * lastfmpanel );
    ~guFetchLastFMInfoThread();

    friend class guDownloadImageThread;
};

// -------------------------------------------------------------------------------- //
class guDownloadImageThread : public wxThread
{
  protected:
    guDbCache *                 m_DbCache;
    guLastFMPanel *             m_LastFMPanel;
    guFetchLastFMInfoThread *   m_MainThread;
    int                         m_CommandId;
    void *                      m_CommandData;
    wxImage * *                 m_pImage;
    int                         m_Index;
    wxString                    m_ImageUrl;
    int                         m_ImageSize;

  public:
    guDownloadImageThread( guLastFMPanel * lastfmpanel, guFetchLastFMInfoThread * mainthread,
            guDbCache * dbcache, int index, const wxChar * imageurl, int commandid,
            void * commanddata, wxImage ** pimage, const int imagesize = guDBCACHE_TYPE_IMAGE_SIZE_TINY );
    ~guDownloadImageThread();

    virtual ExitCode Entry();
};

// -------------------------------------------------------------------------------- //
class guFetchAlbumInfoThread : public guFetchLastFMInfoThread
{
  protected:
    guDbCache *             m_DbCache;
    int                     m_Start;
    wxString                m_ArtistName;

  public:
    guFetchAlbumInfoThread( guLastFMPanel * lastfmpanel, guDbCache * dbcache, const wxChar * artistname, const int startpage );
    ~guFetchAlbumInfoThread();

    virtual ExitCode Entry();

    friend class guDownloadAlbumImageThread;
};

// -------------------------------------------------------------------------------- //
class guFetchTopTracksInfoThread : public guFetchLastFMInfoThread
{
  protected:
    guDbCache *             m_DbCache;
    wxString                m_ArtistName;
    int                     m_ArtistId;
    int                     m_Start;

  public:
    guFetchTopTracksInfoThread( guLastFMPanel * lastfmpanel, guDbCache * dbcache, const wxChar * artistname, const int startpage );
    ~guFetchTopTracksInfoThread();

    virtual ExitCode Entry();

    friend class guDownloadAlbumImageThread;
};

// -------------------------------------------------------------------------------- //
class guFetchArtistInfoThread : public guFetchLastFMInfoThread
{
  private:
    guDbCache *             m_DbCache;
    wxString                m_ArtistName;

  public:
    guFetchArtistInfoThread( guLastFMPanel * lastfmpanel, guDbCache * dbcache, const wxChar * artistname );
    ~guFetchArtistInfoThread();

    virtual ExitCode Entry();

    friend class guDownloadArtistImageThread;
};

// -------------------------------------------------------------------------------- //
class guFetchSimilarArtistInfoThread : public guFetchLastFMInfoThread
{
  private:
    guDbCache *             m_DbCache;
    wxString                m_ArtistName;
    int                     m_Start;

  public:
    guFetchSimilarArtistInfoThread( guLastFMPanel * lastfmpanel, guDbCache * dbcache, const wxChar * artistname, const int startpage );
    ~guFetchSimilarArtistInfoThread();

    virtual ExitCode Entry();

    friend class guDownloadArtistImageThread;
};

// -------------------------------------------------------------------------------- //
class guFetchEventsInfoThread : public guFetchLastFMInfoThread
{
  private:
    guDbCache *             m_DbCache;
    wxString                m_ArtistName;
    int                     m_Start;

  public:
    guFetchEventsInfoThread( guLastFMPanel * lastfmpanel, guDbCache * dbcache, const wxChar * artistname, const int startpage );
    ~guFetchEventsInfoThread();

    virtual ExitCode Entry();

    friend class guDownloadArtistImageThread;
};

// -------------------------------------------------------------------------------- //
class guFetchSimTracksInfoThread : public guFetchLastFMInfoThread
{
  private:
    guDbCache *             m_DbCache;
    wxString                m_ArtistName;
    wxString                m_TrackName;
    int                     m_Start;

  public:
    guFetchSimTracksInfoThread( guLastFMPanel * lastfmpanel, guDbCache * dbcache, const wxChar * artistname, const wxChar * trackname, const int stargpage );
    ~guFetchSimTracksInfoThread();

    virtual ExitCode Entry();

    friend class guDownloadTrackImageThread;
};

// -------------------------------------------------------------------------------- //
class guLastFMInfoCtrl : public wxPanel
{
  protected :
    guDbLibrary *       m_DefaultDb;
    guDbCache *         m_DbCache;
    guPlayerPanel *     m_PlayerPanel;
    wxStaticBitmap *    m_Bitmap;
	wxStaticText *      m_Text;
    wxColour             m_NormalColor;
    wxColour             m_NotFoundColor;
	guMediaViewer *     m_MediaViewer;
    guDbLibrary *       m_Db;
    wxMutex             m_DbMutex;

    virtual void        OnContextMenu( wxContextMenuEvent& event );
    virtual void        CreateContextMenu( wxMenu * Menu );
    virtual void        OnDoubleClicked( wxMouseEvent &event );
    virtual wxString    GetSearchText( void ) { return wxEmptyString; }
    virtual wxString    GetItemUrl( void ) { return wxEmptyString; }

    virtual void        OnSearchLinkClicked( wxCommandEvent &event );
    virtual void        OnCopyToClipboard( wxCommandEvent &event );
    virtual void        CreateControls( wxWindow * parent );
    virtual void        OnPlayClicked( wxCommandEvent &event );
    virtual void        OnEnqueueClicked( wxCommandEvent &event );
    virtual int         GetSelectedTracks( guTrackArray * tracks ) { return 0; }
    virtual void        OnSongSelectName( wxCommandEvent &event ) {}
    virtual void        OnArtistSelectName( wxCommandEvent &event ) {}
    virtual void        OnAlbumSelectName( wxCommandEvent &event ) {}

    //virtual void        OnBitmapMouseOver( wxCommandEvent &event );
    virtual void        OnBitmapClicked( wxMouseEvent &event );
    virtual wxString    GetBitmapImageUrl( void );

    virtual void        OnMouse( wxMouseEvent &event );

    virtual bool        ItemWasFound( void ) { return false; }

  public :
	guLastFMInfoCtrl( wxWindow * parent, guDbLibrary * db, guDbCache * dbcache, guPlayerPanel * playerpanel, bool createcontrols = true );
	~guLastFMInfoCtrl();

    virtual void        SetMediaViewer( guMediaViewer * mediaviewer );

    virtual void        Clear( guMediaViewer * mediaviewer );
    virtual void        SetBitmap( const wxImage * image );
    virtual void        SetLabel( const wxString &label );

};
WX_DEFINE_ARRAY_PTR( guLastFMInfoCtrl *, guLastFMInfoCtrlArray );

// -------------------------------------------------------------------------------- //
class guArtistInfoCtrl : public guLastFMInfoCtrl
{
  private :
    guLastFMArtistInfo *    m_Info;
    wxSizer *               m_MainSizer;
    wxSizer *               m_DetailSizer;
    guHtmlWindow *          m_ArtistDetails;
	wxHyperlinkCtrl *       m_ShowMoreHyperLink;
	bool                    m_ShowLongBioText;

    virtual void        OnContextMenu( wxContextMenuEvent& event );
    //virtual void        OnClick( wxMouseEvent &event );
    virtual wxString    GetSearchText( void );
    virtual wxString    GetItemUrl( void );
    virtual void        CreateContextMenu( wxMenu * Menu );
    virtual void        CreateControls( wxWindow * parent );
    void                UpdateArtistInfoText( void );
    void                OnShowMoreLinkClicked( wxHyperlinkEvent &event );
    void                OnHtmlLinkClicked( wxHtmlLinkEvent& event );
    virtual int         GetSelectedTracks( guTrackArray * tracks );
    virtual void        OnArtistSelectName( wxCommandEvent &event );

    virtual wxString    GetBitmapImageUrl( void ) { return m_Info ? m_Info->m_ImageUrl : wxT( "" ); }

    virtual bool        ItemWasFound( void ) { return m_Info && ( m_Info->m_ArtistId != wxNOT_FOUND ); }

  protected :
    virtual void        OnCopyToClipboard( wxCommandEvent &event );

  public :
    guArtistInfoCtrl( wxWindow * parent, guDbLibrary * db, guDbCache * dbcache, guPlayerPanel * playerpanel );
    ~guArtistInfoCtrl();

    virtual void        SetMediaViewer( guMediaViewer * mediaviewer );

    void SetInfo( guLastFMArtistInfo * info );
    virtual void Clear( guMediaViewer * mediaviewer );
    virtual void SetBitmap( const wxImage * image );

};

// -------------------------------------------------------------------------------- //
class guAlbumInfoCtrl : public guLastFMInfoCtrl
{
  private :
    guLastFMAlbumInfo * m_Info;

    virtual void        OnContextMenu( wxContextMenuEvent& event );
    virtual wxString    GetSearchText( void );
    virtual wxString    GetItemUrl( void );
    virtual void        CreateContextMenu( wxMenu * Menu );
    virtual int         GetSelectedTracks( guTrackArray * tracks );
    virtual void        OnAlbumSelectName( wxCommandEvent &event );

    virtual wxString    GetBitmapImageUrl( void ) { return m_Info ? m_Info->m_ImageUrl : wxT( "" ); }

    virtual bool        ItemWasFound( void ) { return m_Info && ( m_Info->m_AlbumId != wxNOT_FOUND ); }

  public :
    guAlbumInfoCtrl( wxWindow * parent, guDbLibrary * db, guDbCache * dbcache, guPlayerPanel * playerpanel );
    ~guAlbumInfoCtrl();

    virtual void        SetMediaViewer( guMediaViewer * mediaviewer );

    void SetInfo( guLastFMAlbumInfo * info );
    virtual void Clear( guMediaViewer * mediaviewer );

};
WX_DEFINE_ARRAY_PTR( guAlbumInfoCtrl *, guAlbumInfoCtrlArray );

// -------------------------------------------------------------------------------- //
class guSimilarArtistInfoCtrl : public guLastFMInfoCtrl
{
  private :
    guLastFMSimilarArtistInfo * m_Info;

    virtual void        OnContextMenu( wxContextMenuEvent& event );
    virtual void        CreateContextMenu( wxMenu * Menu );
    virtual wxString    GetSearchText( void );
    virtual wxString    GetItemUrl( void );
    //void                OnClick( wxMouseEvent &event );
    virtual int         GetSelectedTracks( guTrackArray * tracks );
    virtual void        OnArtistSelectName( wxCommandEvent &event );
    void                OnSelectArtist( wxCommandEvent &event );

    virtual wxString    GetBitmapImageUrl( void ) { return m_Info ? m_Info->m_ImageUrl : wxT( "" ); }

    virtual bool        ItemWasFound( void ) { return m_Info && ( m_Info->m_ArtistId != wxNOT_FOUND ); }

  public :
    guSimilarArtistInfoCtrl( wxWindow * parent, guDbLibrary * db, guDbCache * dbcache, guPlayerPanel * playerpanel );
    ~guSimilarArtistInfoCtrl();

    virtual void        SetMediaViewer( guMediaViewer * mediaviewer );

    void SetInfo( guLastFMSimilarArtistInfo * info );
    virtual void Clear( guMediaViewer * mediaviewer );

};
WX_DEFINE_ARRAY_PTR( guSimilarArtistInfoCtrl *, guSimilarArtistInfoCtrlArray );

// -------------------------------------------------------------------------------- //
class guTrackInfoCtrl : public guLastFMInfoCtrl
{
  private :
    guLastFMTrackInfo * m_Info;

    virtual void        OnContextMenu( wxContextMenuEvent& event );
    virtual void        CreateContextMenu( wxMenu * Menu );
    virtual wxString    GetSearchText( void );
    virtual wxString    GetItemUrl( void );
    virtual int         GetSelectedTracks( guTrackArray * tracks );
    virtual void        OnSongSelectName( wxCommandEvent &event );
    virtual void        OnArtistSelectName( wxCommandEvent &event );

    void                OnSelectArtist( wxCommandEvent &event );

    virtual wxString    GetBitmapImageUrl( void ) { return m_Info ? m_Info->m_ImageUrl : wxT( "" ); }

    virtual bool        ItemWasFound( void ) { return m_Info && ( m_Info->m_TrackId != wxNOT_FOUND ); }

  public :
    guTrackInfoCtrl( wxWindow * parent, guDbLibrary * db, guDbCache * dbcache, guPlayerPanel * playerpanel );
    ~guTrackInfoCtrl();

    virtual void        SetMediaViewer( guMediaViewer * mediaviewer );

    void SetInfo( guLastFMTrackInfo * info );
    virtual void Clear( guMediaViewer * mediaviewer );

};
WX_DEFINE_ARRAY_PTR( guTrackInfoCtrl *, guTrackInfoCtrlArray );

// -------------------------------------------------------------------------------- //
class guTopTrackInfoCtrl : public guLastFMInfoCtrl
{
  private :
    guLastFMTopTrackInfo * m_Info;

    virtual void        OnContextMenu( wxContextMenuEvent& event );
    virtual void        CreateContextMenu( wxMenu * Menu );
    virtual wxString    GetSearchText( void );
    virtual wxString    GetItemUrl( void );
    virtual int         GetSelectedTracks( guTrackArray * tracks );
    virtual void        OnSongSelectName( wxCommandEvent &event );
    virtual void        OnArtistSelectName( wxCommandEvent &event );

    void                OnSelectArtist( wxCommandEvent &event );

    virtual wxString    GetBitmapImageUrl( void ) { return m_Info ? m_Info->m_ImageUrl : wxT( "" ); }

    virtual bool        ItemWasFound( void ) { return m_Info && ( m_Info->m_TrackId != wxNOT_FOUND ); }

  public :
    guTopTrackInfoCtrl( wxWindow * parent, guDbLibrary * db, guDbCache * dbcache, guPlayerPanel * playerpanel );
    ~guTopTrackInfoCtrl();

    virtual void        SetMediaViewer( guMediaViewer * mediaviewer );

    void SetInfo( guLastFMTopTrackInfo * info );
    virtual void Clear( guMediaViewer * mediaviewer );

};
WX_DEFINE_ARRAY_PTR( guTopTrackInfoCtrl *, guTopTrackInfoCtrlArray );

// -------------------------------------------------------------------------------- //
class guEventInfoCtrl : public guLastFMInfoCtrl
{
  private :
    guLastFMEventInfo * m_Info;

    virtual void        OnContextMenu( wxContextMenuEvent& event );
    virtual void        CreateContextMenu( wxMenu * Menu );
    virtual wxString    GetSearchText( void );
    virtual wxString    GetItemUrl( void );
    virtual int         GetSelectedTracks( guTrackArray * tracks );

    virtual wxString    GetBitmapImageUrl( void ) { return m_Info ? m_Info->m_ImageUrl : wxT( "" ); }

    virtual bool        ItemWasFound( void ) { return true; }

  public :
    guEventInfoCtrl( wxWindow * parent, guDbLibrary * db, guDbCache * dbcache, guPlayerPanel * playerpanel );
    ~guEventInfoCtrl();

    void SetInfo( guLastFMEventInfo * info );
    virtual void Clear( guMediaViewer * mediaviewer );

};
WX_DEFINE_ARRAY_PTR( guEventInfoCtrl *, guEventInfoCtrlArray );

// -------------------------------------------------------------------------------- //
class guLastFMPanel : public wxScrolledWindow
{
  private :
    guDbLibrary *           m_DefaultDb;
    guDbLibrary *           m_Db;
    guDbCache *             m_DbCache;
    guPlayerPanel *         m_PlayerPanel;
    guTrackChangeInfoArray  m_TrackChangeItems;
    int                     m_CurrentTrackInfo;
	wxString                m_ArtistName;
	wxString                m_LastArtistName;
	wxString                m_TrackName;
	wxString                m_LastTrackName;
	guMediaViewer *         m_MediaViewer;
	wxString                m_ShortBio;
	wxString                m_LongBio;
	bool                    m_UpdateEnabled;

	guFetchArtistInfoThread *           m_ArtistsUpdateThread;
	wxMutex                             m_ArtistsUpdateThreadMutex;

	guFetchAlbumInfoThread *            m_AlbumsUpdateThread;
	wxMutex                             m_AlbumsUpdateThreadMutex;

	guFetchTopTracksInfoThread *        m_TopTracksUpdateThread;
	wxMutex                             m_TopTracksUpdateThreadMutex;

	guFetchSimilarArtistInfoThread *    m_SimArtistsUpdateThread;
	wxMutex                             m_SimArtistsUpdateThreadMutex;

	guFetchSimTracksInfoThread *        m_SimTracksUpdateThread;
	wxMutex                             m_SimTracksUpdateThreadMutex;

	guFetchEventsInfoThread *           m_EventsUpdateThread;
	wxMutex                             m_EventsUpdateThreadMutex;

    // TODO : Check if its really necesary
	wxMutex                             m_UpdateEventsMutex;

	// GUI Elements
	wxBoxSizer *                        m_MainSizer;

    wxCheckBox *                        m_UpdateCheckBox;
	wxBitmapButton *                    m_PrevButton;
	wxBitmapButton *                    m_NextButton;
	wxBitmapButton *                    m_ReloadButton;
    wxTextCtrl *                        m_ArtistTextCtrl;
    wxTextCtrl *                        m_TrackTextCtrl;
    wxBitmapButton *                    m_SearchButton;

    bool                                m_ShowArtistDetails;
    wxStaticText *                      m_ArtistDetailsStaticText;
	wxBoxSizer *                        m_ArtistInfoMainSizer;
	wxBoxSizer *                        m_ArtistDetailsSizer;

    guArtistInfoCtrl *                  m_ArtistInfoCtrl;

	bool                                m_ShowAlbums;
    wxStaticText *                      m_AlbumsStaticText;
	wxGridSizer *                       m_AlbumsSizer;
    guAlbumInfoCtrlArray                m_AlbumsInfoCtrls;
    wxStaticText *                      m_AlbumsRangeLabel;
    wxBitmapButton *                    m_AlbumsPrevBtn;
    wxBitmapButton *                    m_AlbumsNextBtn;
    int                                 m_AlbumsCount;
    int                                 m_AlbumsPageStart;

	bool                                m_ShowTopTracks;
    wxStaticText *                      m_TopTracksStaticText;
	wxGridSizer *                       m_TopTracksSizer;
	guTopTrackInfoCtrlArray             m_TopTrackInfoCtrls;
    wxStaticText *                      m_TopTracksRangeLabel;
    wxBitmapButton *                    m_TopTracksPrevBtn;
    wxBitmapButton *                    m_TopTracksNextBtn;
    int                                 m_TopTracksCount;
    int                                 m_TopTracksPageStart;

	bool                                m_ShowSimArtists;
    wxStaticText *                      m_SimArtistsStaticText;
	wxGridSizer *                       m_SimArtistsSizer;
	guSimilarArtistInfoCtrlArray        m_SimArtistsInfoCtrls;
    wxStaticText *                      m_SimArtistsRangeLabel;
    wxBitmapButton *                    m_SimArtistsPrevBtn;
    wxBitmapButton *                    m_SimArtistsNextBtn;
    int                                 m_SimArtistsCount;
    int                                 m_SimArtistsPageStart;

	bool                                m_ShowSimTracks;
    wxStaticText *                      m_SimTracksStaticText;
	wxGridSizer *                       m_SimTracksSizer;
	guTrackInfoCtrlArray                m_SimTracksInfoCtrls;
    wxStaticText *                      m_SimTracksRangeLabel;
    wxBitmapButton *                    m_SimTracksPrevBtn;
    wxBitmapButton *                    m_SimTracksNextBtn;
    int                                 m_SimTracksCount;
    int                                 m_SimTracksPageStart;

	bool                                m_ShowEvents;
    wxStaticText *                      m_EventsStaticText;
	wxGridSizer *                       m_EventsSizer;
	guEventInfoCtrlArray                m_EventsInfoCtrls;
    wxStaticText *                      m_EventsRangeLabel;
    wxBitmapButton *                    m_EventsPrevBtn;
    wxBitmapButton *                    m_EventsNextBtn;
    int                                 m_EventsCount;
    int                                 m_EventsPageStart;

    wxStaticText *                      m_ContextMenuObject;


	// TODO : Check if its really necesary
	wxMutex                             m_UpdateInfoMutex;

    void    OnUpdateArtistInfo( wxCommandEvent &event );
    void    OnUpdateAlbumItem( wxCommandEvent &event );
    void    OnUpdateTopTrackItem( wxCommandEvent &event );
    void    OnUpdateArtistItem( wxCommandEvent &event );
    void    OnUpdateTrackItem( wxCommandEvent &event );
    void    OnUpdateEventItem( wxCommandEvent &event );

	void    OnArInfoTitleDClicked( wxMouseEvent &event );
    void    OnTopAlbumsTitleDClick( wxMouseEvent &event );
    void    OnTopTracksTitleDClick( wxMouseEvent &event );
	void    OnSimArTitleDClick( wxMouseEvent &event );
	void    OnSimTrTitleDClick( wxMouseEvent &event );
	void    OnEventsTitleDClick( wxMouseEvent &event );

    void    SetTopAlbumsVisible( const bool dolayout = false );
    void    SetTopTracksVisible( const bool dolayout = false );
    void    SetSimArtistsVisible( const bool dolayout = false );
    void    SetSimTracksVisible( const bool dolayout = false );
    void    SetEventsVisible( const bool dolayout = false );


	void    OnUpdateChkBoxClick( wxCommandEvent &event );
    void    OnPrevBtnClick( wxCommandEvent &event );
    void    OnNextBtnClick( wxCommandEvent &event );
    void    OnReloadBtnClick( wxCommandEvent &event );
    void    OnTextUpdated( wxCommandEvent& event );
    void    OnTextCtrlKeyDown( wxKeyEvent &event );
    void    OnSearchSelected( wxCommandEvent &event );

    void    UpdateTrackChangeButtons( void );

    void    UpdateAlbumsRangeLabel( void );
    void    OnAlbumsCountUpdated( wxCommandEvent &event );
    void    OnAlbumsPrevClicked( wxCommandEvent &event );
    void    OnAlbumsNextClicked( wxCommandEvent &event );

    void    UpdateTopTracksRangeLabel( void );
    void    OnTopTracksCountUpdated( wxCommandEvent &event );
    void    OnTopTracksPrevClicked( wxCommandEvent &event );
    void    OnTopTracksNextClicked( wxCommandEvent &event );

    void    UpdateSimArtistsRangeLabel( void );
    void    OnSimArtistsCountUpdated( wxCommandEvent &event );
    void    OnSimArtistsPrevClicked( wxCommandEvent &event );
    void    OnSimArtistsNextClicked( wxCommandEvent &event );

    void    UpdateSimTracksRangeLabel( void );
    void    OnSimTracksCountUpdated( wxCommandEvent &event );
    void    OnSimTracksPrevClicked( wxCommandEvent &event );
    void    OnSimTracksNextClicked( wxCommandEvent &event );

    void    UpdateEventsRangeLabel( void );
    void    OnEventsCountUpdated( wxCommandEvent &event );
    void    OnEventsPrevClicked( wxCommandEvent &event );
    void    OnEventsNextClicked( wxCommandEvent &event );

    void    OnContextMenu( wxContextMenuEvent& event );

    void    OnPlayClicked( wxCommandEvent &event );
    void    OnEnqueueClicked( wxCommandEvent &event );
    void    OnSaveClicked( wxCommandEvent &event );
    void    OnCopyToClicked( wxCommandEvent &event );

    void    GetContextMenuTracks( guTrackArray * tracks );

  public :
            guLastFMPanel( wxWindow * parent, guDbLibrary * db,
                guDbCache * dbcache, guPlayerPanel * playerpanel );
            ~guLastFMPanel();

    void                OnUpdatedTrack( wxCommandEvent &event );
    void                AppendTrackChangeInfo( const guTrackChangeInfo * trackchangeinfo );
	void                ShowCurrentTrack( void );
	void                SetUpdateEnable( bool value );
	void                UpdateLayout( void ) { m_MainSizer->FitInside( this ); }
	void                OnDropFiles( const wxArrayString &files );
	void                OnDropFiles( const guTrackArray * tracks );

	guMediaViewer *     GetMediaViewer( void ) { return m_MediaViewer; }
	void                SetMediaViewer( guMediaViewer * mediaviewer );

	void                MediaViewerClosed( guMediaViewer * mediaviewer );

    friend class guFetchLastFMInfoThread;
    friend class guFetchArtistInfoThread;
    friend class guFetchAlbumInfoThread;
    friend class guFetchTopTracksInfoThread;
    friend class guFetchSimilarArtistInfoThread;
    friend class guFetchSimTracksInfoThread;
    friend class guFetchEventsInfoThread;
    friend class guDownloadImageThread;
};

// -------------------------------------------------------------------------------- //
class guLastFMPanelDropTarget : public wxDropTarget
{
  private:
    guLastFMPanel *     m_LastFMPanel;

  public:
    guLastFMPanelDropTarget( guLastFMPanel * lastfmpanel );
    ~guLastFMPanelDropTarget();

    virtual wxDragResult OnData( wxCoord x, wxCoord y, wxDragResult def );
};

}

#endif
// -------------------------------------------------------------------------------- //
