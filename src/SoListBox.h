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
#ifndef GUSOLISTBOX_H
#define GUSOLISTBOX_H

#include "DbLibrary.h"
#include "ListView.h"
#include "StatusBar.h"

#define guSONGS_COLUMN_NUMBER       0
#define guSONGS_COLUMN_TITLE        1
#define guSONGS_COLUMN_ARTIST       2
#define guSONGS_COLUMN_ALBUMARTIST  3
#define guSONGS_COLUMN_ALBUM        4
#define guSONGS_COLUMN_GENRE        5
#define guSONGS_COLUMN_COMPOSER     6
#define guSONGS_COLUMN_DISK         7
#define guSONGS_COLUMN_LENGTH       8
#define guSONGS_COLUMN_YEAR         9
#define guSONGS_COLUMN_BITRATE      10
#define guSONGS_COLUMN_RATING       11
#define guSONGS_COLUMN_PLAYCOUNT    12
#define guSONGS_COLUMN_LASTPLAY     13
#define guSONGS_COLUMN_ADDEDDATE    14
#define guSONGS_COLUMN_FORMAT       15
#define guSONGS_COLUMN_FILEPATH     16

#define guSONGS_COLUMN_COUNT        17

class guLibPanel;

// -------------------------------------------------------------------------------- //
class guSoListBox : public guListView
{
  protected :
    guMediaViewer *             m_MediaViewer;

    guDbLibrary *               m_Db;
    guTrackArray                m_Items;
    wxMutex                     m_ItemsMutex;
    int                         m_ItemsFirst;
    int                         m_ItemsLast;
    wxString                    m_ConfName;
    int                         m_LastColumnRightClicked;
    int                         m_LastRowRightClicked;
    int                         m_TracksOrder;
    bool                        m_TracksOrderDesc;

    wxBitmap *                  m_NormalStar;
    wxBitmap *                  m_SelectStar;

    virtual void                DrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const;
    virtual void                CreateContextMenu( wxMenu * Menu ) const;
    virtual wxString            OnGetItemText( const int row, const int column ) const;
    virtual void                GetItemsList( void );

    virtual int                 GetDragFiles( wxFileDataObject * files );

    void                        OnSearchLinkClicked( wxCommandEvent &event );
    void                        OnCommandClicked( wxCommandEvent &event );
    virtual wxString            GetSearchText( int item ) const;

    void                        OnItemColumnClicked( wxListEvent &event );
    void                        OnItemColumnRClicked( wxListEvent &event );

    virtual void                ItemsLock() { m_ItemsMutex.Lock(); };
    virtual void                ItemsUnlock() { m_ItemsMutex.Unlock(); };
    virtual void                ItemsCheckRange( const int start, const int end );

    virtual void                AppendFastEditMenu( wxMenu * menu, const int selcount ) const;

    virtual void                OnConfigUpdated( wxCommandEvent &event );
    virtual void                CreateAcceleratorTable();

  public :
    guSoListBox( wxWindow * parent, guMediaViewer * mediaviewer, wxString confname, long style = 0 );
    ~guSoListBox();

    virtual void                ReloadItems( bool reset = true );

    virtual int                 GetSelectedSongs( guTrackArray * Songs );
    virtual void                GetAllSongs( guTrackArray * Songs );

    virtual int                 GetItemId( const int row ) const;
    virtual wxString            GetItemName( const int row ) const;

    virtual wxArrayString       GetColumnNames( void ) const;

    void                        UpdatedTracks( const guTrackArray * tracks );
    void                        UpdatedTrack( const guTrack * track );
    int                         FindItem( const int trackid );

    int                         GetLastColumnClicked( void ) { return m_LastColumnRightClicked; }
    int                         GetLastRowClicked( void ) { return m_LastRowRightClicked; }
    wxVariant                   GetLastDataClicked( void );

    virtual int                 GetTracksOrder( void ) { return m_TracksOrder; }
    virtual void                SetTracksOrder( const int order );
    virtual bool                GetTracksOrderDesc( void ) { return m_TracksOrderDesc; }
    virtual void                SetTracksOrderDesc( const bool orderdesc ) { m_TracksOrderDesc = orderdesc; }

};

#endif
// -------------------------------------------------------------------------------- //
