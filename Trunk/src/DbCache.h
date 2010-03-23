// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2010 J.Rios
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
#ifndef guDBCACHE_H
#define guDBCACHE_H

#include "Db.h"

#include <wx/image.h>

enum guDBCacheImageSize {
    guDBCACHE_IMAGE_SIZE_TINY      = 0,
    guDBCACHE_IMAGE_SIZE_MID,
    guDBCACHE_IMAGE_SIZE_BIG
};

#define guDBCACHE_TYPE_TEXT     0x45545458

// -------------------------------------------------------------------------------- //
class guDbCache : public guDb
{
  private :
    static guDbCache * m_DbCache;

  protected :
    bool        DoSetImage( const wxString &url, wxImage * img, const int imgtype, int imagesize );

  public :
    guDbCache( const wxString &dbname );
    ~guDbCache();

    wxImage *           GetImage( const wxString &url, int &imagetype, const int imagesize );
    bool                SetImage( const wxString &url, wxImage * img, const int imgtype );
    wxString            GetContent( const wxString &url );
    bool                SetContent( const wxString &url, const char * str, const int len );
    bool                SetContent( const wxString &url, const wxString &content );

    static guDbCache *  GetDbCache( void ) { return m_DbCache; }
    void                SetDbCache( void ) { m_DbCache = this; }
    void                ClearExpired( void );

};

#endif
// -------------------------------------------------------------------------------- //
