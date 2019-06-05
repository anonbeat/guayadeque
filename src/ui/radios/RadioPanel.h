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
#ifndef __RADIOPANEL_H__
#define __RADIOPANEL_H__

#include "AuiManagerPanel.h"
#include "AuiNotebook.h"
#include "DbRadios.h"
#include "PlayerPanel.h"
#include "ItemListBox.h"
#include "Shoutcast.h"
#include "RadioProvider.h"

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
#include <wx/treectrl.h>

namespace Guayadeque {

#define     guPANEL_RADIO_TEXTSEARCH        ( 1 << 0 )
#define     guPANEL_RADIO_GENRES            ( 1 << 1 )
//#define     guPANEL_RADIO_LABELS            ( 1 << 2 )
//#define     guPANEL_RADIO_STATIONS          ( 1 << 3 )

#define     guPANEL_RADIO_VISIBLE_DEFAULT   ( guPANEL_RADIO_TEXTSEARCH | guPANEL_RADIO_GENRES )

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


// -------------------------------------------------------------------------------- //
// guRadioItemData
// -------------------------------------------------------------------------------- //
class guRadioItemData : public wxTreeItemData
{
  private :
    int         m_Id;
    int         m_Source;
    wxString    m_Name;
    wxString    m_Url;
    int         m_Flags;    // Search flags

  public :
    guRadioItemData( const int id, const int source, const wxString &name, int flags )
    {
        m_Id = id;
        m_Source = source;
        m_Name = name;
        m_Flags = flags;
    }

    guRadioItemData( const int id, const int source, const wxString &name, const wxString &url, int flags )
    {
        m_Id = id;
        m_Source = source;
        m_Name = name;
        m_Url = url;
        m_Flags = flags;
    }

    int         GetId( void ) { return m_Id; }
    void        SetId( int id ) { m_Id = id; }
    int         GetSource( void ) { return m_Source; }
    void        SetSource( int source ) { m_Source = source; }
    wxString    GetName( void ) { return m_Name; }
    void        SetName( const wxString &name ) { m_Name = name; }
    wxString    GetUrl( void ) { return m_Url; }
    void        SetUrl( const wxString &url ) { m_Url = url; }
    int         GetFlags( void ) { return m_Flags; }
    void        SetFlags( int flags ) { m_Flags = flags; }

};

// -------------------------------------------------------------------------------- //
class guShoutcastSearch : public wxDialog
{
  protected:
    guRadioItemData *   m_ItemData;
	wxTextCtrl *            m_SearchTextCtrl;
    wxCheckBox *            m_SearchPlayChkBox;
	wxCheckBox *            m_SearchGenreChkBox;
	wxCheckBox *            m_SearchNameChkBox;
	wxCheckBox *            m_AllGenresChkBox;

    void                    OnOkButton( wxCommandEvent &event );

  public:
    guShoutcastSearch( wxWindow * parent, guRadioItemData * itemdata );
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
    int             m_AfterCurrent;

  public :
    guRadioPlayListLoadThread( guRadioPanel * radiopanel, const wxChar * stationurl, guTrackArray * tracks, const bool enqueue, const int aftercurrent );
    ~guRadioPlayListLoadThread();

    virtual ExitCode Entry();

};

// -------------------------------------------------------------------------------- //
// guRadioGenreTreeCtrl
// -------------------------------------------------------------------------------- //
class guRadioGenreTreeCtrl : public wxTreeCtrl
{
  private :
    guRadioPanel *  m_RadioPanel;
    wxImageList *   m_ImageList;
    wxTreeItemId    m_RootId;

    void            OnContextMenu( wxTreeEvent &event );
    void            OnKeyDown( wxKeyEvent &event );

    void            OnConfigUpdated( wxCommandEvent &event );
    void            CreateAcceleratorTable( void );

  public :
    guRadioGenreTreeCtrl( wxWindow * parent, guRadioPanel * radiopanel );
    ~guRadioGenreTreeCtrl();

    void            ReloadProviders( guRadioProviderArray * radioproviders );
//    void            ReloadItems( void );
//    wxTreeItemId *  GetShoutcastId( void ) { return &m_ShoutcastId; }
//    wxTreeItemId *  GetShoutcastGenreId( void ) { return &m_ShoutcastGenreId; }
//    wxTreeItemId *  GetShoutcastSearchId( void ) { return &m_ShoutcastSearchsId; }
//    wxTreeItemId *  GetManualId( void ) { return &m_ManualId; }
//    wxTreeItemId *  GetTuneInId( void ) { return &m_TuneInId; }
    wxTreeItemId    GetItemId( wxTreeItemId * itemid, const int id );

};

// -------------------------------------------------------------------------------- //
// Class guRadioPanel
// -------------------------------------------------------------------------------- //
class guRadioPanel : public guAuiManagerPanel
{
  private:
    guDbRadios *                    m_Db;
	guPlayerPanel *                 m_PlayerPanel;
	guTrackArray                    m_StationPlayListTracks;
    guRadioProviderArray *          m_RadioProviders;
    long                            m_MinBitRate;
    wxTimer                         m_GenreSelectTimer;

    wxTimer                         m_TextChangedTimer;

    guRadioPlayListLoadThread *     m_RadioPlayListLoadThread;
    wxMutex                         m_RadioPlayListLoadThreadMutex;

    bool                            m_InstantSearchEnabled;
    bool                            m_EnterSelectSearchEnabled;

    int                             m_StationsOrder;
    bool                            m_StationsOrderDesc;


    void OnRadioUpdateEnd( wxCommandEvent &event );
	void OnRadioUpdate( wxCommandEvent &Event );
	void OnRadioUpdated( wxCommandEvent &Event );
	void OnRadioGenreListSelected( wxTreeEvent &Event );
    void OnStationListActivated( wxCommandEvent &event );
    void OnStationListBoxColClick( wxListEvent &event );
    void OnSearchActivated( wxCommandEvent &event );
    void OnSearchSelected( wxCommandEvent &event );
    void OnSearchCancelled( wxCommandEvent &event );
	void OnRadioStationsPlay( wxCommandEvent &event );
	void OnRadioStationsEnqueue( wxCommandEvent &event );
	void OnSelectStations( bool enqueue = false, const int aftercurrent = 0 );
    void OnLoadStationsFinished( wxCommandEvent &event );
    void OnRadioStationsAddToUser( wxCommandEvent &event );

    void OnTextChangedTimer( wxTimerEvent &event );

    void LoadStationUrl( const wxString &stationurl, const bool enqueue, const int aftercurrent );
    void OnStationPlayListLoaded( wxCommandEvent &event );

    void OnGoToSearch( wxCommandEvent &event );
    bool DoTextSearch( void );

    void OnRadioCreateItems( wxCommandEvent &event );

    void OnGenreSelectTimeout( wxTimerEvent &event );

  protected:
    wxSearchCtrl *          m_InputTextCtrl;
	guRadioGenreTreeCtrl *  m_GenresTreeCtrl;
	guRadioLabelListBox *   m_LabelsListBox;
	guRadioStationListBox * m_StationsListBox;

    void                    OnConfigUpdated( wxCommandEvent &event );

  public:
	guRadioPanel( wxWindow * parent, guDbLibrary * Db, guPlayerPanel * NewPlayerPanel );
	~guRadioPanel();

    guDbRadios *                 GetDbRadios( void ) { return m_Db; }
    virtual void               InitPanelData( void );
    void                        RegisterRadioProvider( guRadioProvider * provider, const bool reload = false );
    void                        UnRegisterRadioProvider( guRadioProvider * provider, const bool reload = false );
    void                        ReloadProviders( void );
    void                        ReloadStations( void );

    int                         GetProviderCount( void ) { return m_RadioProviders->Count(); }
    guRadioProvider *           GetProvider( const int index ) { return ( * m_RadioProviders )[ index ]; }
    guRadioProvider *           GetProvider( const wxTreeItemId &itemid );

    void                        GetRadioCounter( wxLongLong * count );
    int                         GetProviderStations( guRadioStations * stations );
    void                        RefreshStations( void );
    bool                        GetSelectedStation( guRadioStation * station );
    wxTreeItemId                 GetSelectedGenre( void );
    guRadioItemData *           GetSelectedData( const wxTreeItemId &itemid );
    guRadioItemData *           GetSelectedData( void );
    wxTreeItemId                GetItemParent( const wxTreeItemId &item ) const;
    guRadioGenreTreeCtrl *      GetTreeCtrl( void ) { return m_GenresTreeCtrl; }


    bool                        OnContextMenu( wxMenu * menu, const bool forstations = false, const int selcount = 0 );

    void                        EndStationPlayListLoaded( void );

    virtual int                 GetListViewColumnCount( void ) { return guRADIOSTATIONS_COLUMN_COUNT; }
    virtual bool                GetListViewColumnData( const int id, int * index, int * width, bool * enabled );
    virtual bool                SetListViewColumnData( const int id, const int index, const int width, const bool enabled, const bool refresh = false );

    void                        SetStationsOrder( const int columnid, const bool desc );
    int                         GetStationsOrder( void ) { return m_StationsOrder; }
    bool                        GetStationsOrderDesc( void ) { return m_StationsOrderDesc; }


    void StartLoadingStations( void );
    void EndLoadingStations( void );

};

}

#endif
// -------------------------------------------------------------------------------- //

