// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2016 J.Rios anonbeat@gmail.com
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
#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "Collections.h"
#include "ConfigConsts.h"
#include "Settings.h"

#include <wx/wx.h>
#include <wx/fileconf.h>
#include <wx/tokenzr.h>
#include <wx/xml/xml.h>

namespace Guayadeque {

WX_DEFINE_ARRAY_PTR( wxEvtHandler *, guEvtHandlerArray );

wxDECLARE_EVENT( guConfigUpdatedEvent, wxCommandEvent );

class guMediaCollectionArray;
class guTrack;
class guTrackArray;

// -------------------------------------------------------------------------------- //
class guConfig
{
  protected :
    wxXmlDocument *     m_XmlDocument;
    wxXmlNode *         m_RootNode;
    wxString            m_FileName;
    wxMutex             m_ConfigMutex;
    guEvtHandlerArray   m_Objects;
    wxMutex             m_ObjectsMutex;

    bool                m_IgnoreLayouts;
    int                 m_Version;

    inline wxXmlNode *  FindNode( const wxString &category );
    bool                LoadFile( const wxString &filename );
    bool                LoadWithBackup( const wxString &filename );
    bool                AddBackupFile( const wxString &filename );

  public :
    guConfig( const wxString &conffile = guPATH_CONFIG_FILENAME );
    virtual ~guConfig();

    static guConfig *   Get( void );
    void                Set( guConfig * config );


    int                 Version( void ) { return m_Version; }
    void                Flush( void );

    long                ReadNum( const wxString &keyname, long defval = 0, const wxString &category = wxEmptyString );
    bool                WriteNum( const wxString &keyname, long value = 0, const wxString &category = wxEmptyString );

    bool                ReadBool( const wxString &keyname, bool defval = true, const wxString &category = wxEmptyString );
    bool                WriteBool( const wxString &keyname, bool value, const wxString &category = wxEmptyString );

    wxString            ReadStr( const wxString &keyname, const wxString &defval, const wxString &category = wxEmptyString );
    bool                WriteStr( const wxString &keyname, const wxString &value, const wxString &category = wxEmptyString );

    wxArrayString       ReadAStr( const wxString &Key, const wxString &defval, const wxString &category = wxEmptyString );
    bool                WriteAStr( const wxString &Key, const wxArrayString &value, const wxString &category = wxEmptyString, bool ResetGroup = true );
#if wxUSE_STL
    bool                WriteAStr( const wxString &Key, const wxSortedArrayString &value, const wxString &category = wxEmptyString, bool ResetGroup = true );
#endif
    wxArrayInt          ReadANum( const wxString &Key, const int defval, const wxString &category = wxEmptyString );
    bool                WriteANum( const wxString &Key, const wxArrayInt &value, const wxString &category = wxEmptyString, bool ResetGroup = true );

    void                DeleteCategory( const wxString &category );

    int                 LoadCollections( guMediaCollectionArray * collections, const int type = wxNOT_FOUND );
    void                SaveCollections( guMediaCollectionArray * collections, const bool resetgroup = true );

    bool                SavePlaylistTracks( const guTrackArray &tracks, const int currenttrack );
    int                 LoadPlaylistTracks( guTrackArray &tracks );

    void                RegisterObject( wxEvtHandler * object );
    void                UnRegisterObject( wxEvtHandler * object );
    void                SendConfigChangedEvent( const int flags = 0 );

    bool                GetIgnoreLayouts( void ) { return m_IgnoreLayouts; }
    void                SetIgnoreLayouts( const bool ignorelayouts ) { m_IgnoreLayouts = ignorelayouts; }

};

// -------------------------------------------------------------------------------- //
wxString inline escape_configlist_str( const wxString &val )
{
    wxString RetVal = val;
    RetVal.Replace( wxT( ":" ), wxT( "_$&" ) );
    RetVal.Replace( wxT( ";" ), wxT( "_&$" ) );
    RetVal.Replace( wxT( "," ), wxT( "$_&" ) );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
wxString inline unescape_configlist_str( const wxString &val )
{
    wxString RetVal = val;
    RetVal.Replace( wxT( "_$&" ), wxT( ":" ) );
    RetVal.Replace( wxT( "_&$" ), wxT( ";" ) );
    RetVal.Replace( wxT( "$_&" ), wxT( "," ) );
    return RetVal;
}

}

#endif
// -------------------------------------------------------------------------------- //
