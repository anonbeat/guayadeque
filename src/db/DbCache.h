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
#ifndef __DBCACHE_H__
#define __DBCACHE_H__

#include "Db.h"

#include <wx/image.h>

namespace Guayadeque {

enum guDBCacheTypes {
    guDBCACHE_TYPE_TEXT = 0x45545458,
    guDBCACHE_TYPE_IMAGE_SIZE_TINY      = 0,
    guDBCACHE_TYPE_IMAGE_SIZE_MID,
    guDBCACHE_TYPE_IMAGE_SIZE_BIG,
    guDBCACHE_TYPE_LASTFM,
    guDBCACHE_TYPE_SHOUTCAST,
    guDBCACHE_TYPE_TUNEIN
};

// -------------------------------------------------------------------------------- //
class guDbCache : public guDb
{
  private :
    static guDbCache * m_DbCache;

  protected :
    bool        DoSetImage( const wxString &url, wxImage * img, const wxBitmapType imgtype, int imagesize );

  public :
    guDbCache( const wxString &dbname );
    ~guDbCache();

    wxImage *           GetImage( const wxString &url, wxBitmapType &imagetype, const int imagesize );
    bool                SetImage( const wxString &url, wxImage * img, const wxBitmapType imgtype );
    wxString            GetContent( const wxString &url );
    bool                SetContent( const wxString &url, const char * str, const int len );
    bool                SetContent( const wxString &url, const wxString &content, const int type = guDBCACHE_TYPE_TEXT );

    static guDbCache *  GetDbCache( void ) { return m_DbCache; }
    void                SetDbCache( void ) { m_DbCache = this; }
    void                ClearExpired( void );

};

}

#endif
// -------------------------------------------------------------------------------- //
