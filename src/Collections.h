// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2016 J.Rios anonbeat@gmail.com
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
//    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#ifndef guCOLLECTIONS_H
#define guCOLLECTIONS_H

#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/dynarray.h>
#include <wx/menu.h>

enum guMediaCollectionType {
    guMEDIA_COLLECTION_TYPE_NORMAL,
    guMEDIA_COLLECTION_TYPE_JAMENDO,
    guMEDIA_COLLECTION_TYPE_MAGNATUNE,
    guMEDIA_COLLECTION_TYPE_PORTABLE_DEVICE,
    guMEDIA_COLLECTION_TYPE_IPOD
};

// -------------------------------------------------------------------------------- //
class guMediaCollection
{
  public :
    wxString        m_UniqueId;
    int             m_Type;
    wxString        m_Name;
    wxArrayString   m_Paths;
    wxArrayString   m_CoverWords;
    bool            m_UpdateOnStart;
    bool            m_ScanPlaylists;
    bool            m_ScanFollowSymLinks;
    bool            m_ScanEmbeddedCovers;
    bool            m_EmbeddMetadata;
    wxString        m_DefaultCopyAction;
    int             m_LastUpdate;

    guMediaCollection( const int type = guMEDIA_COLLECTION_TYPE_NORMAL );
    ~guMediaCollection();

    bool            CheckPaths( void );
};
WX_DECLARE_OBJARRAY( guMediaCollection, guMediaCollectionArray );

// -------------------------------------------------------------------------------- //
class guManagedCollection : public guMediaCollection
{
  protected :
    bool            m_Enabled;
    wxMenu *        m_MenuItem;

  public :
    guManagedCollection( void );
    ~guManagedCollection();

};
WX_DECLARE_OBJARRAY( guManagedCollection, guManagedCollectionArray );

#endif
// -------------------------------------------------------------------------------- //
