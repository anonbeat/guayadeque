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
#ifndef TRACKEDIT_H
#define TRACKEDIT_H

#include "DbLibrary.h"
#include "MusicBrainz.h"
#include "RatingCtrl.h"

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/statline.h>
#include <wx/splitter.h>

WX_DEFINE_ARRAY_PTR( wxImage *, guImagePtrArray );

extern const wxEventType guTrackEditEvent;
#define guTRACKEDIT_EVENT_MBRAINZ_TRACKS        1000

class guTrackEditor;

// -------------------------------------------------------------------------------- //
class guMusicBrainzMetadataThread : public wxThread
{
  protected :
    guTrackEditor * m_TrackEditor;
    guTrack *       m_Track;

  public :
    guMusicBrainzMetadataThread( guTrackEditor * trackeditor, int trackindex );
    ~guMusicBrainzMetadataThread();

    virtual ExitCode Entry();

};

// -------------------------------------------------------------------------------- //
// Class guTrackEditor
// -------------------------------------------------------------------------------- //
class guTrackEditor : public wxDialog
{
  private:
    guTrackArray *      m_Items;
    guImagePtrArray *   m_Images;
    int                 m_CurItem;
    DbLibrary *         m_Db;
    guMBTrackArray *    m_MBrainzAlbums;
    guMBReleaseArray *  m_MBrainzReleases;
    int                 m_MBrainzCurTrack;
    int                 m_MBrainzCurAlbum;
	wxColor             m_NormalColor;
	wxColor             m_ErrorColor;


  protected:
    wxSplitterWindow *  m_SongListSplitter;
    wxListBox *         m_SongListBox;
    wxBitmapButton *    m_MoveUpButton;
	wxBitmapButton *    m_MoveDownButton;
    wxBitmapButton *    m_ArCopyButton;
    wxTextCtrl *        m_ArtistTextCtrl;
    wxBitmapButton *    m_AlCopyButton;
    wxTextCtrl *        m_AlbumTextCtrl;
    wxBitmapButton *    m_TiCopyButton;
    wxTextCtrl *        m_TitleTextCtrl;
    wxBitmapButton *    m_NuCopyButton;
    wxTextCtrl *        m_NumberTextCtrl;
    wxBitmapButton *    m_GeCopyButton;
    wxTextCtrl *        m_GenreTextCtrl;
    wxBitmapButton *    m_YeCopyButton;
    wxTextCtrl *        m_YearTextCtrl;
    wxBitmapButton *    m_RaCopyButton;
    guRating *          m_Rating;
    wxStaticText *      m_DetailInfoStaticText;

    wxStaticBitmap *    m_PictureBitmap;
    wxBitmapButton *    m_AddPicButton;
    wxBitmapButton *    m_DelPicButton;
    wxBitmapButton *    m_SavePicButton;
    wxBitmapButton *    m_CopyPicButton;
    int                 m_CurrentRating;
    int                 m_RatingStartY;
    int                 m_RatingStart;
    bool                m_RatingChanged;
    //wxBitmapButton *    m_MusicBrainzButton;

    guMusicBrainzMetadataThread *   m_MBrainzThread;
    wxChoice *          m_MBrainzAlbumChoice;
    wxBitmapButton *    m_MBrainzAddButton;
    wxBitmapButton *    m_MBrainzCopyButton;


	wxTextCtrl *        m_MBQueryArtistTextCtrl;
    wxTextCtrl *        m_MBQueryTitleTextCtrl;
    wxBitmapButton *    m_MBQueryClearButton;
    bool                m_MBQuerySetArtistEnabled;

	wxStaticText *      m_MBrainzArtistStaticText;
    wxTextCtrl *        m_MBrainzArtistTextCtrl;
    wxBitmapButton *    m_MBrainzArCopyButton;
    wxStaticText *      m_MBrainzAlbumStaticText;
    wxTextCtrl *        m_MBrainzAlbumTextCtrl;
    wxBitmapButton *    m_MBrainzAlCopyButton;
    wxStaticText *      m_MBrainzDateStaticText;
    wxChoice *          m_MBrainzDateChoice;
    wxBitmapButton *    m_MBrainzDaCopyButton;
    wxStaticText *      m_MBrainzTitleStaticText;
    wxTextCtrl *        m_MBrainzTitleTextCtrl;
    wxBitmapButton *    m_MBrainzTiCopyButton;
    wxStaticText *      m_MBrainzLengthStaticText;
    wxTextCtrl *        m_MBrainzLengthTextCtrl;
    wxStaticText *      m_MBrainzNumberStaticText;
    wxTextCtrl *        m_MBrainzNumberTextCtrl;
    wxStaticText *      m_MBrainzInfoStaticText;
    wxBitmapButton *    m_MBrainzNuCopyButton;

    // Event handlers, overide them in your derived class
    void OnSongListBoxSelected( wxCommandEvent &event );
	void OnMoveUpBtnClick( wxCommandEvent& event );
	void OnMoveDownBtnClick( wxCommandEvent& event );
    void OnArCopyButtonClicked( wxCommandEvent &event );
    void OnAlCopyButtonClicked( wxCommandEvent &event );
    void OnTiCopyButtonClicked( wxCommandEvent &event );
    void OnNuCopyButtonClicked( wxCommandEvent &event );
    void OnGeCopyButtonClicked( wxCommandEvent &event );
    void OnYeCopyButtonClicked( wxCommandEvent &event );
    void OnRaCopyButtonClicked( wxCommandEvent &event );

    void OnMBrainzAddButtonClicked( wxCommandEvent &event );
    void OnMBrainzAlbumsFound( wxCommandEvent &event );
    void OnMBrainzAlbumChoiceSelected( wxCommandEvent &event );

    void OnMBrainzCopyButtonClicked( wxCommandEvent &event );

	void OnMBrainzArtistCopyButtonClicked( wxCommandEvent& event );
	void OnMBrainzAlbumCopyButtonClicked( wxCommandEvent& event );
	void OnMBrainzDateCopyButtonClicked( wxCommandEvent& event );
	void OnMBrainzTitleCopyButtonClicked( wxCommandEvent& event );
	void OnMBrainzNumberCopyButtonClicked( wxCommandEvent& event );


    void OnMBQueryClearButtonClicked( wxCommandEvent &event );
	void OnMBQueryTextCtrlChanged( wxCommandEvent& event );


    void ReadItemData( void );
    void WriteItemData( void );

    void SongListSplitterOnIdle( wxIdleEvent& );

    void RefreshImage( void );
    void OnAddImageClicked( wxCommandEvent &event );
    void OnDelImageClicked( wxCommandEvent &event );
    void OnSaveImageClicked( wxCommandEvent &event );
    void OnCopyImageClicked( wxCommandEvent &event );

    void OnRatingChanged( guRatingEvent &event );

    void FinishedMusicBrainzSearch( void );
    int  FindMBrainzReleaseId( const wxString releaseid );
    void UpdateMBrainzTrackInfo( void );
    int  CheckTracksLengths( guMBTrackArray * mbtracks, guTrackArray * tracks );

public:
    guTrackEditor( wxWindow * parent, DbLibrary * Db, guTrackArray * Songs, guImagePtrArray * m_Images );
    ~guTrackEditor();

    friend class guMusicBrainzMetadataThread;
};

#endif
// -------------------------------------------------------------------------------- //
