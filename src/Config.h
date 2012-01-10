// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2012 J.Rios
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
#ifndef GUCONFIG_H
#define GUCONFIG_H

#include "Collections.h"
#include "Settings.h"

#include <wx/wx.h>
#include <wx/fileconf.h>
#include <wx/tokenzr.h>
#include <wx/xml/xml.h>

WX_DEFINE_ARRAY_PTR( wxEvtHandler *, guEvtHandlerArray );
extern const wxEventType guConfigUpdatedEvent;

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

    void                LoadOldConfig( const wxString &conffile );
    void                LoadOldAccelerators( wxFileConfig * fileconfig );
    void                LoadOldAlbumBrowser( wxFileConfig * fileconfig, const wxString &uniqueid );
    void                LoadOldAlbumBrowserFilters( wxFileConfig * fileconfig, const wxString &uniqueid );
    void                LoadOldCommands( wxFileConfig * fileconfig );
    void                LoadOldCopyTo( wxFileConfig * fileconfig );
    void                LoadOldCoverSearch( wxFileConfig * fileconfig, guMediaCollection * collection );
    void                LoadOldCrossfader( wxFileConfig * fileconfig );
    void                LoadOldEqualizer( wxFileConfig * fileconfig );
    void                LoadOldFileBrowser( wxFileConfig * fileconfig );
    void                LoadOldFileRenamer( wxFileConfig * fileconfig );
    void                LoadOldGeneral( wxFileConfig * fileconfig, guMediaCollection * collection, const wxString &uniqueid );
    void                LoadOldJamendo( wxFileConfig * fileconfig );
    void                LoadOldJamendoGenres( wxFileConfig * fileconfig );
    void                LoadOldLastFM( wxFileConfig * fileconfig );
    void                LoadOldLibPaths( wxFileConfig * fileconfig, guMediaCollection * collection );
    void                LoadOldLibreFM( wxFileConfig * fileconfig );
    void                LoadOldLyrics( wxFileConfig * fileconfig );
    void                LoadOldMagnatune( wxFileConfig * fileconfig );
    void                LoadOldMagnatuneGenreList( wxFileConfig * fileconfig );
    void                LoadOldMagnatuneGenres( wxFileConfig * fileconfig );
    void                LoadOldMainSources( wxFileConfig * fileconfig );
    void                LoadOldPlayback( wxFileConfig * fileconfig );
    void                LoadOldPlayList( wxFileConfig * fileconfig );
    void                LoadOldPodcasts( wxFileConfig * fileconfig );
    void                LoadOldPositions( wxFileConfig * fileconfig );
    void                LoadOldRadios( wxFileConfig * fileconfig );
    void                LoadOldRecord( wxFileConfig * fileconfig );
    void                LoadOldSearchFilters( wxFileConfig * fileconfig );
    void                LoadOldSearchLinks( wxFileConfig * fileconfig );
    void                LoadOldTreeView( wxFileConfig * fileconfig, const wxString &uniqueid );
    void                LoadOldTreeViewFilters( wxFileConfig * fileconfig, const wxString &uniqueid );

  public :
    guConfig( const wxString &conffile = guPATH_CONFIG_FILENAME );
    ~guConfig();

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
    void                SetIgnoreLayouts( const bool ignorelayouts ) { m_IgnoreLayouts = ignorelayouts; };

};

// -------------------------------------------------------------------------------- //
wxString inline escape_configlist_str( const wxString &val )
{
    wxString RetVal = val;
    RetVal.Replace( wxT( ":" ), wxT( "_$&" ) );
    RetVal.Replace( wxT( ";" ), wxT( "_&$" ) );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
wxString inline unescape_configlist_str( const wxString &val )
{
    wxString RetVal = val;
    RetVal.Replace( wxT( "_$&" ), wxT( ":" ) );
    RetVal.Replace( wxT( "_&$" ), wxT( ";" ) );
    return RetVal;
}

#endif
// -------------------------------------------------------------------------------- //
