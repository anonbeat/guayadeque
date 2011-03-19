// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2011 J.Rios
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
#ifndef RADIOPANEL_H
#define RADIOPANEL_H

#include "AuiManagedPanel.h"
#include "DbRadios.h"
#include "PlayerPanel.h"
#include "ItemListBox.h"
#include "Shoutcast.h"

#include <wx/aui/aui.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/statbmp.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/statline.h>
#include <wx/listctrl.h>
#include <wx/splitter.h>
#include <wx/frame.h>
#include <wx/srchctrl.h>

#define     guPANEL_RADIO_TEXTSEARCH        ( 1 << 0 )
#define     guPANEL_RADIO_GENRES            ( 1 << 1 )
#define     guPANEL_RADIO_LABELS            ( 1 << 2 )
//#define     guPANEL_RADIO_STATIONS          ( 1 << 3 )

#define     guPANEL_RADIO_VISIBLE_DEFAULT   ( guPANEL_RADIO_TEXTSEARCH | guPANEL_RADIO_GENRES |\
                                              guPANEL_RADIO_LABELS )

class guRadioGenreTreeCtrl;
class guRadioStationListBox;
class guRadioLabelListBox;

#define     guRADIO_SEARCH_FLAG_NONE            ( 0 )
#define     guRADIO_SEARCH_FLAG_NOWPLAYING      ( 1 << 0 )
#define     guRADIO_SEARCH_FLAG_STATION         ( 1 << 1 )
#define     guRADIO_SEARCH_FLAG_GENRE           ( 1 << 2 )
#define     guRADIO_SEARCH_FLAG_ALLGENRES       ( 1 << 3 )
#define     guRADIO_SEARCH_FLAG_DEFAULT         ( guRADIO_SEARCH_FLAG_NOWPLAYING | guRADIO_SEARCH_FLAG_ALLGENRES )

#define guRADIOSTATIONS_COLUMN_NAME         0
#define guRADIOSTATIONS_COLUMN_BITRATE      1
#define guRADIOSTATIONS_COLUMN_LISTENERS    2
#define guRADIOSTATIONS_COLUMN_TYPE         3
#define guRADIOSTATIONS_COLUMN_NOWPLAYING   4

#define guRADIOSTATIONS_COLUMN_COUNT        5


class guShoutcastItemData;

// -------------------------------------------------------------------------------- //
class guShoutcastSearch : public wxDialog
{
  protected:
    guShoutcastItemData *   m_ItemData;
	wxTextCtrl *            m_SearchTextCtrl;
    wxCheckBox *            m_SearchPlayChkBox;
	wxCheckBox *            m_SearchGenreChkBox;
	wxCheckBox *            m_SearchNameChkBox;
	wxCheckBox *            m_AllGenresChkBox;

    void                    OnOkButton( wxCommandEvent &event );

  public:
    guShoutcastSearch( wxWindow * parent, guShoutcastItemData * itemdata );
	~guShoutcastSearch();

};

class guRadioPanel;

// -------------------------------------------------------------------------------- //
// Class guRadioPlayListLoadThread
// -------------------------------------------------------------------------------- //
class guRadioPlayListLoadThread : public wxThread
{
  protected :
    guRadioPanel *  m_RadioPanel;
    wxString        m_StationUrl;
    guTrackArray *  m_Tracks;
    bool            m_Enqueue;
    bool            m_AsNext;

  public :
    guRadioPlayListLoadThread( guRadioPanel * radiopanel, const wxChar * stationurl, guTrackArray * tracks, const bool enqueue, const bool asnext );
    ~guRadioPlayListLoadThread();

    virtual ExitCode Entry();

};

// -------------------------------------------------------------------------------- //
// Class guRadioPanel
// -------------------------------------------------------------------------------- //
class guRadioPanel : public guAuiManagedPanel
{
  private:
	guDbRadios *                    m_Db;
	guPlayerPanel *                 m_PlayerPanel;
	guTrackArray                    m_StationPlayListTracks;

    wxTimer                         m_TextChangedTimer;

    guRadioPlayListLoadThread *     m_RadioPlayListLoadThread;
    wxMutex                         m_RadioPlayListLoadThreadMutex;

    void OnRadioUpdateEnd( wxCommandEvent &event );
	void OnRadioUpdate( wxCommandEvent &Event );
	void OnRadioUpdated( wxCommandEvent &Event );
	void OnRadioGenreListSelected( wxTreeEvent &Event );
    void OnStationListActivated( wxListEvent &event );
    void OnStationListBoxColClick( wxListEvent &event );
    void OnStationsEditLabelsClicked( wxCommandEvent &event );
    void OnSearchActivated( wxCommandEvent &event );
    void OnSearchSelected( wxCommandEvent &event );
    void OnSearchCancelled( wxCommandEvent &event );
	void OnRadioGenreListActivated( wxTreeEvent &Event );
	void OnRadioLabelListSelected( wxListEvent &Event );
	void OnRadioStationsPlay( wxCommandEvent &event );
	void OnRadioStationsEnqueue( wxCommandEvent &event );
	void OnRadioStationsEnqueueAsNext( wxCommandEvent &event );
	void OnSelectStations( bool enqueue = false, const bool asnext = false );

	void OnRadioUserAdd( wxCommandEvent &event );
	void OnRadioUserEdit( wxCommandEvent &event );
	void OnRadioUserDel( wxCommandEvent &event );

	void OnRadioSearchAdd( wxCommandEvent &event );
	void OnRadioSearchEdit( wxCommandEvent &event );
	void OnRadioSearchDel( wxCommandEvent &event );

	void OnRadioUserExport( wxCommandEvent &event );
	void OnRadioUserImport( wxCommandEvent &event );

    void OnTextChangedTimer( wxTimerEvent &event );

    void LoadStationUrl( const wxString &stationurl, const bool enqueue, const bool asnext );
    void OnStationPlayListLoaded( wxCommandEvent &event );

    void OnGoToSearch( wxCommandEvent &event );

  protected:
    wxSearchCtrl *          m_InputTextCtrl;
	guRadioGenreTreeCtrl *  m_GenresTreeCtrl;
	guRadioLabelListBox *   m_LabelsListBox;
	guRadioStationListBox * m_StationsListBox;

  public:
	guRadioPanel( wxWindow * parent, guDbLibrary * Db, guPlayerPanel * NewPlayerPanel );
	~guRadioPanel();

    virtual void        InitPanelData( void );

    void                GetRadioCounter( wxLongLong * count );

    void                EndStationPlayListLoaded( void ) { m_RadioPlayListLoadThreadMutex.Lock();
                                            m_RadioPlayListLoadThread = NULL;
                                            m_RadioPlayListLoadThreadMutex.Unlock(); }

    virtual int         GetListViewColumnCount( void ) { return guRADIOSTATIONS_COLUMN_COUNT; }
    virtual bool        GetListViewColumnData( const int id, int * index, int * width, bool * enabled );
    virtual bool        SetListViewColumnData( const int id, const int index, const int width, const bool enabled, const bool refresh = false );

};

#endif
// -------------------------------------------------------------------------------- //

