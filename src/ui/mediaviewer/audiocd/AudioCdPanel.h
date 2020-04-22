// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2020 J.Rios anonbeat@gmail.com
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
#ifndef __AUDIOCDPANEL_H__
#define __AUDIOCDPANEL_H__

#include "AuiManagerPanel.h"
#include "DbLibrary.h"
#include "ListView.h"
#include "PlayerPanel.h"

namespace Guayadeque {

#include <wx/wx.h>

typedef enum {
    guAUDIOCD_COLUMN_NUMBER,
    guAUDIOCD_COLUMN_TITLE,
    guAUDIOCD_COLUMN_ARTIST,
    guAUDIOCD_COLUMN_ALBUMARTIST,
    guAUDIOCD_COLUMN_ALBUM,
    guAUDIOCD_COLUMN_GENRE,
    guAUDIOCD_COLUMN_COMPOSER,
    guAUDIOCD_COLUMN_DISK,
    guAUDIOCD_COLUMN_LENGTH,
    guAUDIOCD_COLUMN_YEAR,
    guAUDIOCD_COLUMN_COUNT
} guAudioCd_Column;


// -------------------------------------------------------------------------------- //
class guCdTracksListBox : public guListView
{
  protected :
    guTrackArray                m_AudioCdTracks;

    int                         m_LastColumnRightClicked;
    int                         m_LastRowRightClicked;

    int                         m_Order;
    bool                        m_OrderDesc;
    wxArrayString               m_ColumnNames;

    void                        AppendFastEditMenu( wxMenu * menu, const int selcount ) const;
    virtual void                CreateContextMenu( wxMenu * Menu ) const;
    virtual wxString            OnGetItemText( const int row, const int column ) const;
    virtual void                GetItemsList( void );
    virtual int                 GetSelectedSongs( guTrackArray * tracks, const bool isdrag = false ) const;
    void                        GetAllSongs( guTrackArray * tracks );

    void                        OnConfigUpdated( wxCommandEvent &event );
    void                        CreateAcceleratorTable();

    void                        OnItemColumnRClicked( wxListEvent &event );

  public :
    guCdTracksListBox( wxWindow * parent );
    ~guCdTracksListBox();

    void                        SetTrackList( guTrackArray &audiocdtracks );
    void                        ClearTracks( void );

    virtual void                ReloadItems( bool reset = true );

    virtual wxString inline     GetItemName( const int item ) const;
    virtual int inline          GetItemId( const int item ) const;

    int                         GetLastColumnClicked( void ) { return m_LastColumnRightClicked; }
    int                         GetLastRowClicked( void ) { return m_LastRowRightClicked; }
    wxVariant                   GetLastDataClicked( void );

    void                        SetOrder( int order );
    int                         GetOrder( void ) { return m_Order; }
    bool                        GetOrderDesc( void ) { return m_OrderDesc; }

    const wxArrayString &       GetColumnNames( void ) const { return m_ColumnNames; }

    void                        UpdatedTracks( guTrackArray * tracks );

    void                        GetCounters( wxLongLong * count, wxLongLong * len, wxLongLong * size );

    friend class guAudioCdPanel;
};




// -------------------------------------------------------------------------------- //
class guAudioCdPanel : public wxPanel
{
  protected:
    guCdTracksListBox *         m_CdTracksListBox;
    guPlayerPanel *             m_PlayerPanel;

    void                        OnAudioCdTrackActivated( wxCommandEvent &event );

    void                        OnSelectAudioCdTracks( const bool enqueue, const int aftercurrent = 0 );
    void                        OnAudioCdTrackPlay( wxCommandEvent &event );
    void                        OnAudioCdTrackEnqueue( wxCommandEvent &event );
    void                        OnAudioCdTrackCopyTo( wxCommandEvent &event );

    void                        OnAudioCdRefresh( wxCommandEvent &event );

    void                        OnSongSetField( wxCommandEvent &event );
    void                        OnSongEditField( wxCommandEvent &event );

  public:
    guAudioCdPanel( wxWindow * parent, guPlayerPanel * playerpanel );
    ~guAudioCdPanel();

    void GetCounters( wxLongLong * count, wxLongLong * len, wxLongLong * size );
    void UpdateVolume( const bool added );

    void SetAudioCdTracks( guTrackArray &audiocdtracks );

};

}

#endif
// -------------------------------------------------------------------------------- //
