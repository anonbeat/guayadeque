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
#ifndef LASTFMPANEL_H
#define LASTFMPANEL_H

#include "LastFM.h"
#include "PlayerPanel.h"
#include "ThreadArray.h"

#include <wx/image.h>
#include <wx/panel.h>
#include <wx/html/htmlwin.h>
#include <wx/hyperlink.h>

#define GULASTFMINFO_MAXITEMS  12

// -------------------------------------------------------------------------------- //
class guLastFMInfo
{
  public:
    int                     m_Index;
    wxImage *               m_Image;

    guLastFMInfo() {};

    guLastFMInfo( int index, wxImage * image = NULL )
    {
        m_Index = index;
        m_Image = image;
    };

    ~guLastFMInfo()
    {
        if( m_Image )
            delete m_Image;
    };
};
WX_DECLARE_OBJARRAY(guLastFMInfo, guLastFMInfoArray);

// -------------------------------------------------------------------------------- //
class guLastFMArtistInfo : public guLastFMInfo
{
  public:
    guArtistInfo *  m_Artist;
    int             m_ArtistId;

    guLastFMArtistInfo() { m_Artist = NULL; m_ArtistId = wxNOT_FOUND; };
    guLastFMArtistInfo( int index, wxImage * image = NULL, guArtistInfo * artist = NULL ) :
      guLastFMInfo( index, image )
    {
        m_Artist = artist;
        m_ArtistId = wxNOT_FOUND;
    };

    ~guLastFMArtistInfo()
    {
        if( m_Artist )
            delete m_Artist;
    };
};


// -------------------------------------------------------------------------------- //
class guLastFMSimilarArtistInfo : public guLastFMInfo
{
  public:
    guSimilarArtistInfo *   m_Artist;
    int                     m_ArtistId;

    guLastFMSimilarArtistInfo() { m_Artist = NULL; m_ArtistId = wxNOT_FOUND; };

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

    guLastFMAlbumInfo() { m_Album = NULL; m_AlbumId = wxNOT_FOUND; };

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

    guLastFMTrackInfo() { m_Track = NULL; m_TrackId = wxNOT_FOUND; };

    guLastFMTrackInfo( int index, wxImage * image = NULL,
                  guSimilarTrackInfo * track = NULL ) :
            guLastFMInfo( index, image )
    {
        m_Track = track;
        m_TrackId = wxNOT_FOUND;
    };

    ~guLastFMTrackInfo()
    {
        if( m_Track )
            delete m_Track;
    };
};
WX_DECLARE_OBJARRAY(guLastFMTrackInfo, guLastFMTrackInfoArray);

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
    guLastFMPanel *             m_LastFMPanel;
    guFetchLastFMInfoThread *   m_MainThread;
    int                         m_CommandId;
    void *                      m_CommandData;
    wxImage * *                 m_pImage;
    int                         m_Index;
    wxString                    m_ImageUrl;
    wxSize                      m_ScaleSize;

  public:
    guDownloadImageThread( guLastFMPanel * lastfmpanel, guFetchLastFMInfoThread * mainthread,
            int index, const wxChar * imageurl, int commandid, void * commanddata, wxImage ** pimage, const wxSize &scalesize = wxDefaultSize );
    ~guDownloadImageThread();

    virtual ExitCode Entry();
};

// -------------------------------------------------------------------------------- //
class guFetchAlbumInfoThread : public guFetchLastFMInfoThread
{
  protected:
    wxString                m_ArtistName;

  public:
    guFetchAlbumInfoThread( guLastFMPanel * lastfmpanel, const wxChar * artistname );
    ~guFetchAlbumInfoThread();

    virtual ExitCode Entry();

    friend class guDownloadAlbumImageThread;
};

// -------------------------------------------------------------------------------- //
class guFetchSimilarArtistInfoThread : public guFetchLastFMInfoThread
{
  private:
    wxString                m_ArtistName;

  public:
    guFetchSimilarArtistInfoThread( guLastFMPanel * lastfmpanel, const wxChar * artistname );
    ~guFetchSimilarArtistInfoThread();

    virtual ExitCode Entry();

    friend class guDownloadArtistImageThread;
};

// -------------------------------------------------------------------------------- //
class guFetchTrackInfoThread : public guFetchLastFMInfoThread
{
  private:
    wxString                m_ArtistName;
    wxString                m_TrackName;

  public:
    guFetchTrackInfoThread( guLastFMPanel * lastfmpanel, const wxChar * artistname, const wxChar * trackname );
    ~guFetchTrackInfoThread();

    virtual ExitCode Entry();

    friend class guDownloadTrackImageThread;
};


// -------------------------------------------------------------------------------- //
class guLastFMInfoCtrl : public wxPanel
{
  protected :
    DbLibrary *         m_Db;
    guPlayerPanel *     m_PlayerPanel;
    wxStaticBitmap *    m_Bitmap;
	wxStaticText *      m_Text;
	wxColor             m_NormalColor;
	wxColor             m_NotFoundColor;

    virtual void        OnContextMenu( wxContextMenuEvent& event );
    virtual void        CreateContextMenu( wxMenu * Menu );
    virtual void        OnClick( wxMouseEvent &event );
    virtual wxString    GetSearchText( void );
    virtual void        OnSearchLinkClicked( wxCommandEvent &event );
    virtual void        OnCopyToClipboard( wxCommandEvent &event );
    virtual void        CreateControls( wxWindow * parent );
    virtual void        OnPlayClicked( wxCommandEvent &event );
    virtual void        OnEnqueueClicked( wxCommandEvent &event );
    virtual int         GetSelectedTracks( guTrackArray * tracks );
    virtual void        OnArtistSelectName( wxCommandEvent &event );
    virtual void        OnAlbumSelectName( wxCommandEvent &event );

  public :
	guLastFMInfoCtrl( wxWindow * parent, DbLibrary * db, guPlayerPanel * playerpanel, bool createcontrols = true );
	~guLastFMInfoCtrl();

    virtual void Clear( void );
    virtual void SetBitmap( const wxImage * image );
    virtual void SetLabel( const wxString &label );
};
WX_DEFINE_ARRAY_PTR( guLastFMInfoCtrl *, guLastFMInfoCtrlArray );

// -------------------------------------------------------------------------------- //
class guArtistInfoCtrl : public guLastFMInfoCtrl
{
  private :
    guLastFMArtistInfo *    m_Info;
    wxSizer *               m_MainSizer;
    wxSizer *               m_DetailSizer;
    wxHtmlWindow *          m_ArtistDetails;
	wxHyperlinkCtrl *       m_ShowMoreHyperLink;
	bool                    m_ShowLongBioText;

    virtual void        OnContextMenu( wxContextMenuEvent& event );
    virtual void        OnClick( wxMouseEvent &event );
    virtual wxString    GetSearchText( void );
    virtual void        CreateContextMenu( wxMenu * Menu );
    virtual void        CreateControls( wxWindow * parent );
    void                UpdateArtistInfoText( void );
    void                OnShowMoreLinkClicked( wxHyperlinkEvent &event );
    void                OnHtmlLinkClicked( wxHtmlLinkEvent& event );
    virtual int         GetSelectedTracks( guTrackArray * tracks );
    virtual void        OnArtistSelectName( wxCommandEvent &event );

  public :
    guArtistInfoCtrl( wxWindow * parent, DbLibrary * db, guPlayerPanel * playerpanel );
    ~guArtistInfoCtrl();

    void SetInfo( guLastFMArtistInfo * info );
    virtual void Clear( void );
    virtual void SetBitmap( const wxImage * image );
};

// -------------------------------------------------------------------------------- //
class guAlbumInfoCtrl : public guLastFMInfoCtrl
{
  private :
    guLastFMAlbumInfo * m_Info;

    virtual void        OnContextMenu( wxContextMenuEvent& event );
//    void                OnSearchLinkClicked( wxCommandEvent &event );
    virtual void        OnClick( wxMouseEvent &event );
    virtual wxString    GetSearchText( void );
    virtual void        CreateContextMenu( wxMenu * Menu );
    virtual int         GetSelectedTracks( guTrackArray * tracks );
    virtual void        OnAlbumSelectName( wxCommandEvent &event );

  public :
    guAlbumInfoCtrl( wxWindow * parent, DbLibrary * db, guPlayerPanel * playerpanel );
    ~guAlbumInfoCtrl();

    void SetInfo( guLastFMAlbumInfo * info );
    virtual void Clear( void );

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
    void                OnClick( wxMouseEvent &event );
    virtual int         GetSelectedTracks( guTrackArray * tracks );
    virtual void        OnArtistSelectName( wxCommandEvent &event );

  public :
    guSimilarArtistInfoCtrl( wxWindow * parent, DbLibrary * db, guPlayerPanel * playerpanel );
    ~guSimilarArtistInfoCtrl();

    void SetInfo( guLastFMSimilarArtistInfo * info );
    virtual void Clear( void );

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
    void                OnClick( wxMouseEvent &event );
    virtual int         GetSelectedTracks( guTrackArray * tracks );
    virtual void        OnArtistSelectName( wxCommandEvent &event );

  public :
    guTrackInfoCtrl( wxWindow * parent, DbLibrary * db, guPlayerPanel * playerpanel );
    ~guTrackInfoCtrl();

    void SetInfo( guLastFMTrackInfo * info );
    virtual void Clear( void );

};
WX_DEFINE_ARRAY_PTR( guTrackInfoCtrl *, guTrackInfoCtrlArray );

// -------------------------------------------------------------------------------- //
class guLastFMPanel : public wxScrolledWindow
{
  private :
    DbLibrary *         m_Db;
    guPlayerPanel *     m_PlayerPanel;
	wxString            m_ArtistName;
	wxString            m_LastArtistName;
	wxString            m_TrackName;
	wxString            m_LastTrackName;
	wxString            m_ShortBio;
	wxString            m_LongBio;
	bool                m_UpdateTracks;

	guFetchAlbumInfoThread *            m_AlbumsUpdateThread;
	wxMutex                             m_AlbumsUpdateThreadMutex;

	guFetchSimilarArtistInfoThread *    m_ArtistsUpdateThread;
	wxMutex                             m_ArtistsUpdateThreadMutex;

	guFetchTrackInfoThread *            m_TracksUpdateThread;
	wxMutex                             m_TracksUpdateThreadMutex;

    // TODO : Check if its really necesary
	wxMutex                             m_UpdateEventsMutex;

	// GUI Elements
	wxBoxSizer *                        m_MainSizer;

    wxCheckBox *                        m_UpdateCheckBox;
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
    guAlbumInfoCtrlArray                m_AlbumInfoCtrls;

	bool                                m_ShowArtists;
    wxStaticText *                      m_ArtistsStaticText;
	wxGridSizer *                       m_ArtistsSizer;
	guSimilarArtistInfoCtrlArray        m_ArtistInfoCtrls;

	bool                                m_ShowTracks;
    wxStaticText *                      m_TracksStaticText;
	wxGridSizer *                       m_TracksSizer;
	guTrackInfoCtrlArray                m_TrackInfoCtrls;


	// TODO : Check if its really necesary
	wxMutex                             m_UpdateInfoMutex;

    void    OnUpdateArtistInfo( wxCommandEvent &event );
    void    OnUpdateAlbumItem( wxCommandEvent &event );
    void    OnUpdateArtistItem( wxCommandEvent &event );
    void    OnUpdateTrackItem( wxCommandEvent &event );

//    void    OnShowMoreLinkClicked( wxHyperlinkEvent &event );
//	  void    OnHtmlLinkClicked( wxHtmlLinkEvent& event );
	void    OnArInfoTitleDClicked( wxMouseEvent &event );
    void    OnTopAlbumsTitleDClick( wxMouseEvent &event );
	void    OnSimArTitleDClick( wxMouseEvent &event );
	void    OnSimTrTitleDClick( wxMouseEvent &event );

	void    OnUpdateChkBoxClick( wxCommandEvent &event );
    void    OnTextUpdated( wxCommandEvent& event );
    void    OnSearchBtnClick( wxCommandEvent &event );

	void    OnAlbumTextClicked( wxMouseEvent &event );
	void    OnArtistTextClicked( wxMouseEvent &event );
	void    OnTrackTextClicked( wxMouseEvent &event );
	void    OnAlbumTextRightClicked( wxMouseEvent &event );
	void    OnArtistTextRightClicked( wxMouseEvent &event );
	void    OnTrackTextRightClicked( wxMouseEvent &event );


  public :
            guLastFMPanel( wxWindow * parent, DbLibrary * db, guPlayerPanel * playerpanel );
            ~guLastFMPanel();

    void    OnUpdatedTrack( wxCommandEvent &event );
	void    SetTrack( const wxString &artistname, const wxString &trackname );
	void    UpdateLayout( void );

    friend class guFetchLastFMInfoThread;
    friend class guFetchAlbumInfoThread;
    friend class guFetchSimilarArtistInfoThread;
    friend class guFetchTrackInfoThread;
    friend class guDownloadImageThread;
};

#endif
// -------------------------------------------------------------------------------- //
