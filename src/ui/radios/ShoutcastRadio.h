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
#ifndef __SHOUTCASTRADIO_H__
#define __SHOUTCASTRADIO_H__

#include "RadioProvider.h"

namespace Guayadeque {

class guDbRadios;

// -------------------------------------------------------------------------------- //
// guShoutcastUpdateThread
// -------------------------------------------------------------------------------- //
class guShoutcastUpdateThread : public wxThread
{
  private:
    guDbRadios *    m_Db;
    guRadioPanel *  m_RadioPanel;
    int             m_GaugeId;
    wxArrayInt      m_Ids;
    int             m_Source;

    void            CheckRadioStationsFilters( const int flags, const wxString &text, guRadioStations &stations );

  public:
    guShoutcastUpdateThread( guDbRadios * db, guRadioPanel * radiopanel,
                                const wxArrayInt &ids, const int source, int gaugeid = wxNOT_FOUND );

    ~guShoutcastUpdateThread(){}

    virtual ExitCode Entry();
};

// -------------------------------------------------------------------------------- //
class guShoutcastRadioProvider : public guRadioProvider
{
  protected :
    wxTreeItemId                        m_ShoutcastId;
    wxTreeItemId                        m_ShoutcastGenreId;
    wxTreeItemId                        m_ShoutcastSearchId;

    void                OnGenreAdd( wxCommandEvent &event );
    void                OnGenreEdit( wxCommandEvent &event );
    void                OnGenreDelete( wxCommandEvent &event );
    void                OnSearchAdd( wxCommandEvent &event );
    void                OnSearchEdit( wxCommandEvent &event );
    void                OnSearchDelete( wxCommandEvent &event );

  public :
    guShoutcastRadioProvider( guRadioPanel * radiopanel, guDbRadios * dbradios );
    ~guShoutcastRadioProvider();

    virtual bool        OnContextMenu( wxMenu * menu, const wxTreeItemId &itemid, const bool forstations = false, const int selcount = 0 );
//    virtual void        Activated( const int id );
    virtual void        SetSearchText( const wxArrayString &texts );
    virtual void        RegisterImages( wxImageList * imagelist );
    virtual void        RegisterItems( guRadioGenreTreeCtrl * genretreectrl, wxTreeItemId &rootitem );
    virtual bool        HasItemId( const wxTreeItemId &itemid );
    virtual int         GetStations( guRadioStations * stations, const long minbitrate );
    virtual void        SetStationsOrder( const int columnid, const bool desc ) { m_Db->SetRadioStationsOrder( columnid ); }
    virtual void        DoUpdate( void );

};

}

#endif
// -------------------------------------------------------------------------------- //
