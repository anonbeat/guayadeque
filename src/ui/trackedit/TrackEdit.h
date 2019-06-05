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
#ifndef __TRACKEDIT_H__
#define __TRACKEDIT_H__

#include "AcousticId.h"
#include "DbLibrary.h"
#include "LyricsPanel.h"
#include "MusicBrainz.h"
#include "RatingCtrl.h"
#include "TagInfo.h"

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/statline.h>
#include <wx/splitter.h>

namespace Guayadeque {

WX_DEFINE_ARRAY_PTR( wxImage *, guImagePtrArray );

extern const wxEventType guTrackEditEvent;

class guTrackEditor;

// -------------------------------------------------------------------------------- //
void guImagePtrArrayClean( guImagePtrArray * images );

// -------------------------------------------------------------------------------- //
class guTrackEditorGetComboDataThread;

// -------------------------------------------------------------------------------- //
// Class guTrackEditor
// -------------------------------------------------------------------------------- //
class guTrackEditor : public wxDialog, guAcousticIdClient
{
  private:
    guTrackArray *                      m_Items;
    guImagePtrArray *                   m_Images;
    wxArrayString *                     m_Lyrics;
    wxArrayInt *                        m_ChangedFlags;
    int                                 m_CurItem;
    int                                 m_NextItem;
    guDbLibrary *                       m_Db;
    wxColour                            m_NormalColor;
    wxColour                            m_ErrorColor;
    wxTimer                             m_SelectedTimer;
    wxTimer                             m_ArtistChangedTimer;
    wxTimer                             m_AlbumArtistChangedTimer;
    wxTimer                             m_AlbumChangedTimer;
    wxTimer                             m_ComposerChangedTimer;
    wxTimer                             m_GenreChangedTimer;

    void                                SetTagField( wxString &field, const wxString &newval, int &changedflags, const int flagval = guTRACK_CHANGED_DATA_TAGS );
    void                                SetTagField( int &field, const int newval, int &changedflags, const int flagval = guTRACK_CHANGED_DATA_TAGS );

  protected:
    wxSortedArrayString                 m_Artists;
    wxSortedArrayString                 m_AlbumArtists;
    wxSortedArrayString                 m_Albums;
    wxSortedArrayString                 m_Composers;
    wxSortedArrayString                 m_Genres;

    wxNotebook *                        m_MainNotebook;
    wxSplitterWindow *                  m_SongListSplitter;
    wxListBox *                         m_SongListBox;
    wxBitmapButton *                    m_MoveUpButton;
	wxBitmapButton *                    m_MoveDownButton;
    wxBitmapButton *                    m_AACopyButton;
    wxComboBox *                        m_AlbumArtistComboBox;
    bool                                m_AlbumArtistChanged;
    wxBitmapButton *                    m_ArCopyButton;
    wxComboBox *                        m_ArtistComboBox;
    bool                                m_ArtistChanged;
    wxBitmapButton *                    m_AlCopyButton;
    wxComboBox *                        m_AlbumComboBox;
    bool                                m_AlbumChanged;
    wxBitmapButton *                    m_TiCopyButton;
    wxTextCtrl *                        m_TitleTextCtrl;
    wxBitmapButton *                    m_CoCopyButton;
    wxComboBox *                        m_CompComboBox;
    bool                                m_CompChanged;
    wxBitmapButton *                    m_NuCopyButton;
    wxBitmapButton *                    m_NuOrderButton;
    wxTextCtrl *                        m_NumberTextCtrl;
    wxBitmapButton *                    m_DiCopyButton;
    wxTextCtrl *                        m_DiskTextCtrl;
    wxBitmapButton *                    m_GeCopyButton;
    wxComboBox *                        m_GenreComboBox;
    bool                                m_GenreChanged;
    wxBitmapButton *                    m_YeCopyButton;
    wxTextCtrl *                        m_YearTextCtrl;
    wxBitmapButton *                    m_RaCopyButton;
    guRating *                          m_Rating;
    wxStaticText *                      m_DetailLeftInfoStaticText;
    wxStaticText *                      m_DetailRightInfoStaticText;

    wxBitmapButton *                    m_CommentCopyButton;
    wxTextCtrl *                        m_CommentText;

    wxStaticBitmap *                    m_PictureBitmap;
    wxBitmapButton *                    m_AddPicButton;
    wxBitmapButton *                    m_DelPicButton;
    wxBitmapButton *                    m_SavePicButton;
    wxBitmapButton *                    m_SearchPicButton;
    wxBitmapButton *                    m_CopyPicButton;
    int                                 m_CurrentRating;
    int                                 m_RatingStartY;
    int                                 m_RatingStart;
    bool                                m_RatingChanged;
    wxBitmapButton *                    m_LyricReloadButton;
    wxTextCtrl *                        m_LyricArtistTextCtrl;
    wxTextCtrl *                        m_LyricTrackTextCtrl;
    wxTextCtrl *                        m_LyricsTextCtrl;

    guAcousticId *                      m_AcousticId;

    wxChoice *                          m_MBAlbumChoice;
    wxBitmapButton *                    m_MBAddButton;
    wxBitmapButton *                    m_MBCopyButton;

    wxTextCtrl *                        m_MBQueryArtistTextCtrl;
    wxTextCtrl *                        m_MBQueryTitleTextCtrl;
    wxBitmapButton *                    m_MBQueryClearButton;
    bool                                m_MBQuerySetArtistEnabled;

    wxStaticText *                      m_MBArtistStaticText;
    wxTextCtrl *                        m_MBArtistTextCtrl;
    wxBitmapButton *                    m_MBArCopyButton;
    wxStaticText *                      m_MBAlbumArtistStaticText;
    wxTextCtrl *                        m_MBAlbumArtistTextCtrl;
    wxBitmapButton *                    m_MBAlArCopyButton;
    wxStaticText *                      m_MBAlbumStaticText;
    wxTextCtrl *                        m_MBAlbumTextCtrl;
    wxBitmapButton *                    m_MBAlCopyButton;
    wxStaticText *                      m_MBYearStaticText;
    wxTextCtrl *                        m_MBYearTextCtrl;
    wxBitmapButton *                    m_MBDaCopyButton;
    wxStaticText *                      m_MBTitleStaticText;
    wxTextCtrl *                        m_MBTitleTextCtrl;
    wxBitmapButton *                    m_MBTiCopyButton;
    wxStaticText *                      m_MBLengthStaticText;
    wxTextCtrl *                        m_MBLengthTextCtrl;
    wxStaticText *                      m_MBNumberStaticText;
    wxTextCtrl *                        m_MBNumberTextCtrl;
    wxStaticText *                      m_MBInfoStaticText;
    wxBitmapButton *                    m_MBNuCopyButton;

    guMBReleaseArray *                  m_MBAlbums;
    int                                 m_MBCurTrack;
    int                                 m_MBCurAlbum;
    wxString                            m_MBRecordingId;


    guTrackEditorGetComboDataThread *   m_GetComboDataThread;

    guLyricSearchEngine *               m_LyricSearchEngine;
    guLyricSearchContext *              m_LyricSearchContext;

    wxString                            m_LastArtist;
    wxString                            m_LastAlbumArtist;
    wxString                            m_LastAlbum;
    wxString                            m_LastComposer;
    wxString                            m_LastGenre;

    // Event handlers
	void                                OnSongListBoxSelected( wxCommandEvent &event );
	void                                OnMoveUpBtnClick( wxCommandEvent& event );
	void                                OnMoveDownBtnClick( wxCommandEvent& event );
	void                                OnAACopyButtonClicked( wxCommandEvent &event );
	void                                OnArCopyButtonClicked( wxCommandEvent &event );
	void                                OnAlCopyButtonClicked( wxCommandEvent &event );
	void                                OnTiCopyButtonClicked( wxCommandEvent &event );
	void                                OnCoCopyButtonClicked( wxCommandEvent &event );
	void                                OnNuCopyButtonClicked( wxCommandEvent &event );
	void                                OnNuOrderButtonClicked( wxCommandEvent &event );
	void                                OnDiCopyButtonClicked( wxCommandEvent &event );
	void                                OnGeCopyButtonClicked( wxCommandEvent &event );
	void                                OnYeCopyButtonClicked( wxCommandEvent &event );
	void                                OnRaCopyButtonClicked( wxCommandEvent &event );

	void                                OnArtistTextChanged( wxCommandEvent& event );
	void                                OnAlbumArtistTextChanged( wxCommandEvent& event );
	void                                OnAlbumTextChanged( wxCommandEvent& event );
	void                                OnComposerTextChanged( wxCommandEvent& event );
	void                                OnGenreTextChanged( wxCommandEvent& event );
    void                                OnArtistChangedTimeout( wxTimerEvent &event );
    void                                OnAlbumArtistChangedTimeout( wxTimerEvent &event );
    void                                OnAlbumChangedTimeout( wxTimerEvent &event );
    void                                OnComposerChangedTimeout( wxTimerEvent &event );
    void                                OnGenreChangedTimeout( wxTimerEvent &event );

	void                                OnCommentCopyButtonClicked( wxCommandEvent &event );

    void                                OnMBrainzAddButtonClicked( wxCommandEvent &event );
    void                                OnMBrainzAlbumChoiceSelected( wxCommandEvent &event );

    void                                OnMBrainzCopyButtonClicked( wxCommandEvent &event );

    void                                OnMBrainzArtistCopyButtonClicked( wxCommandEvent& event );
    void                                OnMBrainzAlbumArtistCopyButtonClicked( wxCommandEvent& event );
    void                                OnMBrainzAlbumCopyButtonClicked( wxCommandEvent& event );
    void                                OnMBrainzYearCopyButtonClicked( wxCommandEvent& event );
    void                                OnMBrainzTitleCopyButtonClicked( wxCommandEvent& event );
    void                                OnMBrainzNumberCopyButtonClicked( wxCommandEvent& event );
    void                                OnMBQueryClearButtonClicked( wxCommandEvent &event );
    void                                OnMBQueryTextCtrlChanged( wxCommandEvent& event );
    void                                MBCheckCountAndLengths( void );

    void                                OnOKButton( wxCommandEvent& event );
	void                                ReadItemData( void );
	void                                WriteItemData( void );

	void                                SongListSplitterOnIdle( wxIdleEvent& );

	void                                RefreshImage( void );
	void                                OnAddImageClicked( wxCommandEvent &event );
	void                                OnDelImageClicked( wxCommandEvent &event );
	void                                OnSaveImageClicked( wxCommandEvent &event );
	void                                OnSearchImageClicked( wxCommandEvent &event );
	void                                OnCopyImageClicked( wxCommandEvent &event );

	void                                OnRatingChanged( guRatingEvent &event );

    void                                UpdateMBrainzTrackInfo( void );
    int                                 CheckTracksLengths( guMBRecordingArray * mbtracks, guTrackArray * tracks );

    void                                OnTextUpdated( wxCommandEvent& event );
	void                                OnSearchLyrics( wxCommandEvent &event );
	void                                OnDownloadedLyric( wxCommandEvent &event );

    void                                OnPageChanged( wxNotebookEvent &event );

    void                                UpdateArtists( void ) { wxCommandEvent Event( wxEVT_TEXT ); wxPostEvent( m_ArtistComboBox, Event ); }
    void                                UpdateAlbumArtists( void ) { wxCommandEvent Event( wxEVT_TEXT ); wxPostEvent( m_AlbumArtistComboBox, Event ); }
    void                                UpdateAlbums( void ) { wxCommandEvent Event( wxEVT_TEXT ); wxPostEvent( m_AlbumComboBox, Event ); }
    void                                UpdateComposers( void ) { wxCommandEvent Event( wxEVT_TEXT ); wxPostEvent( m_CompComboBox, Event ); }
    void                                UpdateGenres( void ) { wxCommandEvent Event( wxEVT_TEXT ); wxPostEvent( m_GenreComboBox, Event ); }

	void                                OnSelectTimeout( wxTimerEvent &event );

    guTrack *                           GetTrack( const int index );

    void ReloadMBAlbums();

public:
    guTrackEditor( wxWindow * parent, guDbLibrary * db,
            guTrackArray * songs, guImagePtrArray * images, wxArrayString * lyrics, wxArrayInt * changedflags );
    ~guTrackEditor();

    // AcousticIdClient functions
    virtual void                        OnAcousticIdMBIdFound( const wxString &mbid );
    virtual void                        OnAcousticIdError( const int errorcode );


    friend class guMusicBrainzMetadataThread;
    friend class guTrackEditorGetComboDataThread;
};

// -------------------------------------------------------------------------------- //
// Class guTrackEditorGetComboDataThread
// -------------------------------------------------------------------------------- //
class guTrackEditorGetComboDataThread : public wxThread
{
  protected :
    guTrackEditor *     m_TrackEditor;
    guDbLibrary *       m_Db;

    void                FillArrayStrings( wxSortedArrayString &array, const guListItems &items );

  public :
    guTrackEditorGetComboDataThread( guTrackEditor * trackedit, guDbLibrary * db );
    ~guTrackEditorGetComboDataThread();

    virtual ExitCode Entry();

};

}

#endif
// -------------------------------------------------------------------------------- //
