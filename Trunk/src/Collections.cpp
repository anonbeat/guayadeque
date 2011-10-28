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
#include "Collections.h"

#include "Settings.h"

#include <wx/arrimpl.cpp>
#include <wx/timer.h>

WX_DEFINE_OBJARRAY( guMediaCollectionArray );
WX_DEFINE_OBJARRAY( guManagedCollectionArray );

// -------------------------------------------------------------------------------- //
guMediaCollection::guMediaCollection( const int type )
{
    m_UniqueId = wxString::Format( wxT( "%08X" ), wxGetLocalTime() );
    m_Type = type;
    m_UpdateOnStart = false;
    m_ScanPlaylists = true;
    m_ScanFollowSymLinks = false;
    m_ScanEmbeddedCovers = true;
    m_EmbeddMetadata = false;
    m_LastUpdate = wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
guMediaCollection::~guMediaCollection()
{
}

// -------------------------------------------------------------------------------- //
bool guMediaCollection::CheckPaths( void )
{
    int Index;
    int Count = m_Paths.Count();
    bool RetVal = Count;
    for( Index = 0; Index < Count; Index++ )
    {
        wxString CurPath = m_Paths[ Index ];
        if( !CurPath.EndsWith( wxT( "/" ) ) )
            CurPath.Append( wxT( "/" ) );
        if( !wxFileExists( CurPath + guCOLLECTIONS_ID_FILENAME ) )
        {
            RetVal = false;
            break;
        }
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
// guManagedCollection
// -------------------------------------------------------------------------------- //
guManagedCollection::guManagedCollection( void )
{
    m_Enabled = false;
    m_MenuItem = NULL;
}

// -------------------------------------------------------------------------------- //
guManagedCollection::~guManagedCollection()
{
}

// -------------------------------------------------------------------------------- //
