// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2016 J.Rios
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
#include "Config.h"

#include "Commands.h"
#include "Utils.h"
#include "Preferences.h"
#include "DbLibrary.h"
#include "DbRadios.h"
#include "PodcastsPanel.h"
#include "MediaViewer.h"

#include <wx/wfstream.h>

static guConfig * m_Config = NULL;

const wxEventType guConfigUpdatedEvent = wxNewEventType();

#define guCONFIG_DEFAULT_VERSION    1

// -------------------------------------------------------------------------------- //
guConfig::guConfig( const wxString &conffile )
{
    m_IgnoreLayouts = false;
    m_Version = guCONFIG_DEFAULT_VERSION;
    m_FileName = conffile;
    m_XmlDocument = NULL;
    m_RootNode = NULL;

    if( LoadWithBackup( m_FileName ) )
        return;

    // The file could not be read so create it
    guLogMessage( wxT( "Could not read the conf file '%s'" ), conffile.c_str() );

    if( wxFileExists( conffile ) )
    {
        wxRenameFile( conffile, conffile + wxT( ".old" ) );
    }

    m_XmlDocument = new wxXmlDocument();

    m_RootNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "config" ) );

    wxXmlAttribute * Version = new wxXmlAttribute( wxT( "version" ), wxString::Format( wxT( "%i" ), m_Version ),
                               NULL );

    m_RootNode->SetAttributes( Version );

    m_XmlDocument->SetRoot( m_RootNode );

    LoadOldConfig( conffile + wxT( ".old" ) );

    m_XmlDocument->Save( conffile );
}

// -------------------------------------------------------------------------------- //
guConfig::~guConfig()
{
    Flush();

    if( m_XmlDocument )
    {
        delete m_XmlDocument;
    }
}

// -------------------------------------------------------------------------------- //
bool guConfig::LoadFile( const wxString &filename )
{
    wxFileInputStream Ins( filename );
    if( Ins.IsOk() )
    {
        m_XmlDocument = new wxXmlDocument( Ins );
        if( m_XmlDocument )
        {
            if( m_XmlDocument->IsOk() )
            {
                m_RootNode = m_XmlDocument->GetRoot();

                if( m_RootNode && m_RootNode->GetName() == wxT( "config" ) )
                {
                    wxString VersionStr;
                    m_RootNode->GetAttribute( wxT( "version" ), &VersionStr );
                    long Version;
                    if( VersionStr.ToLong( &Version ) )
                    {
                        m_Version = Version;
                    }
                    return true;
                }
            }
            delete m_XmlDocument;
            m_XmlDocument = NULL;
        }
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guConfig::AddBackupFile( const wxString &filename )
{
    if( wxFileExists( filename + wxT( ".00" ) ) )
    {
        if( !wxCopyFile( filename + wxT( ".00" ), filename + wxT( ".01" ), true ) )
        {
            guLogMessage( wxT( "Could not create the 01 backup conf file" ) );
            return false;
        }
    }

    if( !wxCopyFile( filename, filename + wxT( ".00" ), true ) )
    {
        guLogMessage( wxT( "Could not create the 00 backup conf file" ) );
        return false;
    }

    return true;
}

// -------------------------------------------------------------------------------- //
bool guConfig::LoadWithBackup( const wxString &conffile )
{
    if( LoadFile( m_FileName ) )
    {
        AddBackupFile( m_FileName );
        return true;
    }
    else
    {
        if( LoadFile( conffile + wxT( ".00" ) ) )
        {
            m_FileName = conffile;
            Flush();
            AddBackupFile( m_FileName );
            return true;
        }
        else
        {
            if( LoadFile( conffile + wxT( ".01" ) ) )
            {
                m_FileName = conffile;
                Flush();
                AddBackupFile( m_FileName );
                return true;
            }
        }
    }
    return false;
}

// -------------------------------------------------------------------------------- //
void guConfig::Set( guConfig * config )
{
    m_Config = config;
}

// -------------------------------------------------------------------------------- //
guConfig * guConfig::Get( void )
{
    return m_Config;
}

// -------------------------------------------------------------------------------- //
void guConfig::Flush( void )
{
    if( m_XmlDocument )
    {
        if( !m_XmlDocument->Save( m_FileName ) )
        {
            guLogMessage( wxT( "Error saving the configuration file '%s'" ), m_FileName.c_str() );
            return;
        }
    }
}

// -------------------------------------------------------------------------------- //
long guConfig::ReadNum( const wxString &keyname, const long defval, const wxString &category )
{
    wxString KeyValue = ReadStr( keyname, wxEmptyString, category );
    if( !KeyValue.IsEmpty() )
    {
        long RetVal;
        if( KeyValue.ToLong( &RetVal ) )
        {
            return RetVal;
        }
    }
    return defval;
}

// -------------------------------------------------------------------------------- //
bool guConfig::WriteNum( const wxString &keyname, long value, const wxString &category )
{
    return WriteStr( keyname, wxString::Format( wxT( "%ld" ), value ), category );
}

// -------------------------------------------------------------------------------- //
bool guConfig::ReadBool( const wxString &keyname, bool defval, const wxString &category )
{
    return ReadNum( keyname, defval, category );
}

// -------------------------------------------------------------------------------- //
bool guConfig::WriteBool( const wxString &keyname, bool value, const wxString &category )
{
    return WriteStr( keyname, wxString::Format( wxT( "%i" ), value ), category );
}

// -------------------------------------------------------------------------------- //
inline wxXmlNode * FindNodeByName( wxXmlNode * xmlnode, const wxString &category )
{
    wxXmlNode * XmlNode = xmlnode;
    while( XmlNode )
    {
        if( XmlNode->GetName() == category )
        {
            return XmlNode;
        }
        XmlNode = XmlNode->GetNext();
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
inline wxXmlNode * guConfig::FindNode( const wxString &category )
{
    wxArrayString Keys;
    Keys = wxStringTokenize( category, wxT( "/" ) );
    int Index;
    int Count = Keys.Count();
    if( Count && m_RootNode )
    {
        Index = 0;
        wxXmlNode * XmlNode = FindNodeByName( m_RootNode->GetChildren(), Keys[ Index ] );
        while( XmlNode )
        {
            Index++;
            if( Index >= Count )
                return XmlNode;
            XmlNode = FindNodeByName( XmlNode->GetChildren(), Keys[ Index ] );
        }
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
inline wxXmlAttribute * FindPropertyByName( wxXmlAttribute * property, const wxString &category )
{
    wxXmlAttribute * Property = property;
    while( Property )
    {
        if( Property->GetName() == category )
        {
            return Property;
        }
        Property = Property->GetNext();
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
wxString guConfig::ReadStr( const wxString &keyname, const wxString &defval, const wxString &category  )
{
    //guLogMessage( wxT( "ReadStr( '%s', '%s', '%s' ) " ), keyname.c_str(), defval.c_str(), category.c_str() );
    wxMutexLocker Locker( m_ConfigMutex );

    wxXmlNode * XmlNode = category.IsEmpty() ? m_RootNode : FindNode( category );

    if( XmlNode )
    {
        XmlNode = FindNodeByName( XmlNode->GetChildren(), keyname );
        if( XmlNode )
        {
            wxString RetVal;
            XmlNode->GetAttribute( wxT( "value" ), &RetVal );
            //guLogMessage( wxT( "ReadStr( '%s/%s' (%s) => '%s'" ), category.c_str(), keyname.c_str(), defval.c_str(), RetVal.c_str() );
            return RetVal;
        }
    }
    //guLogMessage( wxT( "******************** FAILED!!!!!!!!!!!!" ) );
    return defval;
}

// -------------------------------------------------------------------------------- //
inline wxXmlNode * CreateCategoryNode( wxXmlNode * xmlnode, const wxString &category )
{
    //guLogMessage( wxT( "CreateCategoryNode( '%s' )" ), category.c_str() );
    wxArrayString Keys;
    Keys = wxStringTokenize( category, wxT( "/" ) );
    int Index;
    int Count = Keys.Count();
    if( Count && xmlnode )
    {
        Index = 0;
        wxXmlNode * ParentNode = xmlnode;
        wxXmlNode * XmlNode;
        do {
            XmlNode = FindNodeByName( ParentNode->GetChildren(), Keys[ Index ] );
            if( !XmlNode )
            {
                XmlNode = new wxXmlNode( wxXML_ELEMENT_NODE, Keys[ Index ] );
                ParentNode->AddChild( XmlNode );
            }

            Index++;
            if( Index >= Count )
                return XmlNode;

            ParentNode = XmlNode;
        } while( true );
    }
    return xmlnode;
}

// -------------------------------------------------------------------------------- //
bool guConfig::WriteStr( const wxString &keyname, const wxString &value, const wxString &category )
{
    wxMutexLocker Locker( m_ConfigMutex );

    wxXmlNode * CatNode = category.IsEmpty() ? m_RootNode : CreateCategoryNode( m_RootNode, category );

    wxXmlNode * XmlNode = FindNodeByName( CatNode->GetChildren(), keyname );
    if( !XmlNode )
    {
        XmlNode = new wxXmlNode( wxXML_ELEMENT_NODE, keyname );

        wxXmlAttribute * Properties = new wxXmlAttribute( wxT( "value" ), value, NULL );

        XmlNode->SetAttributes( Properties );

        CatNode->AddChild( XmlNode );
    }
    else
    {
        wxXmlAttribute * Property = FindPropertyByName( XmlNode->GetProperties(), wxT( "value" ) );
        if( !Property )
        {
            Property = new wxXmlAttribute( wxT( "value" ), value, NULL );

            XmlNode->SetAttributes( Property );
        }
        else
        {
            Property->SetValue( value );
        }
    }
    return true;
}

// -------------------------------------------------------------------------------- //
wxArrayString guConfig::ReadAStr( const wxString &keyname, const wxString &defval, const wxString &category )
{
    wxMutexLocker Locker( m_ConfigMutex );

    wxArrayString RetVal;
    wxXmlNode * XmlNode = FindNode( category );
    if( XmlNode )
    {
        int Index = 0;
        do {
            wxXmlNode * EntryNode = FindNodeByName( XmlNode->GetChildren(), keyname + wxString::Format( wxT( "%i" ), Index++ ) );
            if( !EntryNode )
                break;
            wxString Entry;
            EntryNode->GetAttribute( wxT( "value" ), &Entry );
            RetVal.Add( Entry );
        } while( true );
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void guConfig::DeleteCategory( const wxString &category )
{
    wxXmlNode * CatNode = FindNode( category );
    if( CatNode )
    {
        wxXmlNode * XmlNode;
        do {
            XmlNode = CatNode->GetChildren();
            if( !XmlNode )
                break;
            CatNode->RemoveChild( XmlNode );
            delete XmlNode;
        } while( XmlNode );
    }
}

// -------------------------------------------------------------------------------- //
void LoadCollectionWordList( wxXmlNode * xmlnode, wxArrayString * wordlist )
{
    while( xmlnode )
    {
        wxString Value;
        xmlnode->GetAttribute( wxT( "value" ), &Value );
        if( !Value.IsEmpty() )
            wordlist->Add( Value );
        xmlnode = xmlnode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
int LoadCollectionInt( wxXmlNode * xmlnode )
{
    wxString ValueStr;
    long ValueNum;
    xmlnode->GetAttribute( wxT( "value" ), &ValueStr );
    if( !ValueStr.IsEmpty() && ValueStr.ToLong( &ValueNum ) )
        return ValueNum;
    return 0;
}

// -------------------------------------------------------------------------------- //
void LoadCollection( wxXmlNode * xmlnode, guMediaCollection * collection )
{
    while( xmlnode )
    {
        wxString Name = xmlnode->GetName();
        if( Name == wxT( "paths" ) )
        {
            LoadCollectionWordList( xmlnode->GetChildren(), &collection->m_Paths );
        }
        else if( Name == wxT( "covers" ) )
        {
            LoadCollectionWordList( xmlnode->GetChildren(), &collection->m_CoverWords );
        }
        else if( Name == wxT( "UniqueId" ) )
        {
            xmlnode->GetAttribute( wxT( "value" ), &collection->m_UniqueId );
        }
        else if( Name == wxT( "Type" ) )
        {
            collection->m_Type = LoadCollectionInt( xmlnode );
        }
        else if( Name == wxT( "Name" ) )
        {
            xmlnode->GetAttribute( wxT( "value" ), &collection->m_Name );
        }
        else if( Name == wxT( "LastUpdate" ) )
        {
            collection->m_LastUpdate = LoadCollectionInt( xmlnode );
        }
        else if( Name == wxT( "UpdateOnStart" ) )
        {
            collection->m_UpdateOnStart = LoadCollectionInt( xmlnode );
        }
        else if( Name == wxT( "ScanPlaylists" ) )
        {
            collection->m_ScanPlaylists = LoadCollectionInt( xmlnode );
        }
        else if( Name == wxT( "ScanFollowSymLinks" ) )
        {
            collection->m_ScanFollowSymLinks = LoadCollectionInt( xmlnode );
        }
        else if( Name == wxT( "ScanEmbeddedCovers" ) )
        {
            collection->m_ScanEmbeddedCovers = LoadCollectionInt( xmlnode );
        }
        else if( Name == wxT( "EmbeddMetadata" ) )
        {
            collection->m_EmbeddMetadata = LoadCollectionInt( xmlnode );
        }
        else if( Name == wxT( "DefaultCopyAction" ) )
        {
            xmlnode->GetAttribute( wxT( "value" ), &collection->m_DefaultCopyAction );
        }

        xmlnode = xmlnode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
int guConfig::LoadCollections( guMediaCollectionArray * collections, const int type )
{
    int LoadCount = 0;
    wxXmlNode * XmlNode = FindNode( wxT( "collections" ) );
    if( XmlNode )
    {
        XmlNode = XmlNode->GetChildren();
        while( XmlNode )
        {
            guMediaCollection * Collection = new guMediaCollection();
            LoadCollection( XmlNode->GetChildren(), Collection );
            if( ( type == wxNOT_FOUND ) || ( Collection->m_Type == type ) )
            {
                collections->Add( Collection );
                LoadCount++;
            }
            else
            {
                delete Collection;
            }
            XmlNode = XmlNode->GetNext();
        }
    }
    return LoadCount;
}

// -------------------------------------------------------------------------------- //
void WriteStr( wxXmlNode * xmlnode, const wxString &name, const wxString &value )
{
    wxXmlNode * XmlNode = new wxXmlNode( wxXML_ELEMENT_NODE, name );
    wxXmlAttribute * Properties = new wxXmlAttribute( wxT( "value" ), value, NULL );
    XmlNode->SetAttributes( Properties );
    xmlnode->AddChild( XmlNode );
}

// -------------------------------------------------------------------------------- //
void SaveCollection( wxXmlNode * xmlnode, guMediaCollection * collection )
{
    wxXmlNode * XmlNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "collection" ) );

    WriteStr( XmlNode, wxT( "UniqueId" ), collection->m_UniqueId );
    WriteStr( XmlNode, wxT( "Type" ), wxString::Format( wxT( "%i" ), collection->m_Type ) );
    WriteStr( XmlNode, wxT( "Name" ), collection->m_Name );
    WriteStr( XmlNode, wxT( "UpdateOnStart" ), wxString::Format( wxT( "%i" ), collection->m_UpdateOnStart ) );
    WriteStr( XmlNode, wxT( "ScanPlaylists" ), wxString::Format( wxT( "%i" ), collection->m_ScanPlaylists ) );
    WriteStr( XmlNode, wxT( "ScanFollowSymLinks" ), wxString::Format( wxT( "%i" ), collection->m_ScanFollowSymLinks ) );
    WriteStr( XmlNode, wxT( "ScanEmbeddedCovers" ), wxString::Format( wxT( "%i" ), collection->m_ScanEmbeddedCovers ) );
    WriteStr( XmlNode, wxT( "EmbeddMetadata" ), wxString::Format( wxT( "%i" ), collection->m_EmbeddMetadata ) );
    WriteStr( XmlNode, wxT( "DefaultCopyAction" ), collection->m_DefaultCopyAction );
    WriteStr( XmlNode, wxT( "LastUpdate" ), wxString::Format( wxT( "%i" ), collection->m_LastUpdate ) );

    wxXmlNode * ParentNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "paths" ) );
    int Index;
    int Count = collection->m_Paths.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        WriteStr( ParentNode, wxString::Format( wxT( "Path%i" ), Index ), collection->m_Paths[ Index ] );
    }

    XmlNode->AddChild( ParentNode );

    ParentNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "covers" ) );
    Count = collection->m_CoverWords.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        WriteStr( ParentNode, wxString::Format( wxT( "Cover%i" ), Index ), collection->m_CoverWords[ Index ] );
    }

    XmlNode->AddChild( ParentNode );


    xmlnode->AddChild( XmlNode );
}

// -------------------------------------------------------------------------------- //
void guConfig::SaveCollections( guMediaCollectionArray * collections, const bool resetgroup )
{
    if( resetgroup )
        DeleteCategory( wxT( "collections" ) );

    wxXmlNode * XmlNode = FindNode( wxT( "collections" ) );
    if( !XmlNode )
    {
        XmlNode = CreateCategoryNode( m_RootNode, wxT( "collections" ) );
    }

    int Index;
    int Count = collections->Count();
    for( Index = 0; Index < Count; Index++ )
    {
        SaveCollection( XmlNode, &collections->Item( Index ) );
    }
}

// -------------------------------------------------------------------------------- //
bool guConfig::WriteAStr( const wxString &keyname, const wxArrayString &value, const wxString &category, bool resetgroup )
{
    int Index;
    int Count = value.Count();

    if( resetgroup )
        DeleteCategory( category );

    for( Index = 0; Index < Count; Index++ )
    {
        if( !WriteStr( keyname + wxString::Format( wxT( "%i" ), Index ), value[ Index ], category ) )
            break;
    }
    return ( Index = Count );
}

#if wxUSE_STL
// -------------------------------------------------------------------------------- //
bool guConfig::WriteAStr( const wxString &Key, const wxSortedArrayString &Value, const wxString &Category, bool ResetGroup )
{
    wxArrayString AStrings;
    int Index;
    int Count = Value.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        AStrings.Add( Value[ Index ] );
    }
    return WriteAStr( Key, AStrings, Category, ResetGroup );
}
#endif

// -------------------------------------------------------------------------------- //
wxArrayInt guConfig::ReadANum( const wxString &keyname, const int defval, const wxString &category )
{
    //guLogMessage( wxT( "ReadANum( '%s', %i, '%s'" ), keyname.c_str(), defval, category.c_str() );
    wxMutexLocker Locker( m_ConfigMutex );
    wxArrayInt RetVal;
    wxXmlNode * XmlNode = FindNode( category );
    if( XmlNode )
    {
        int Index = 0;
        do {
            wxXmlNode * EntryNode = FindNodeByName( XmlNode->GetChildren(), keyname + wxString::Format( wxT( "%i" ), Index++ ) );
            if( !EntryNode )
                break;
            wxString EntryStr;
            long Entry;
            EntryNode->GetAttribute( wxT( "value" ), &EntryStr );
            EntryStr.ToLong( &Entry );
            RetVal.Add( Entry );
        } while( true );
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
bool guConfig::WriteANum( const wxString &keyname, const wxArrayInt &value, const wxString &category, bool resetgroup )
{
    int Index;
    int Count = value.Count();

    if( resetgroup )
        DeleteCategory( category );

    for( Index = 0; Index < Count; Index++ )
    {
        if( !WriteNum( keyname + wxString::Format( wxT( "%i" ), Index ), value[ Index ], category ) )
            break;
    }
    return ( Index = Count );
}

// -------------------------------------------------------------------------------- //
void guConfig::RegisterObject( wxEvtHandler * object )
{
    wxMutexLocker Lock( m_ObjectsMutex );
    if( m_Objects.Index( object ) == wxNOT_FOUND )
    {
        m_Objects.Add( object );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::UnRegisterObject( wxEvtHandler * object )
{
    wxMutexLocker Lock( m_ObjectsMutex );
    int Index = m_Objects.Index( object );
    if( Index != wxNOT_FOUND )
    {
        m_Objects.RemoveAt( Index );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::SendConfigChangedEvent( const int flags )
{
    wxMutexLocker Lock( m_ObjectsMutex );
    wxCommandEvent event( guConfigUpdatedEvent, ID_CONFIG_UPDATED );
    event.SetInt( flags );

    int Index;
    int Count = m_Objects.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        m_Objects[ Index ]->AddPendingEvent( event );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::LoadOldAccelerators( wxFileConfig * fileconfig )
{
    wxString EntryName;
    wxArrayString Accelerators;
    for( int Index = 0; Index < 61; Index++ )
    {
        Accelerators.Add( fileconfig->Read( wxString::Format( wxT( "AccelKey%i" ), Index ), wxT( "0" ) ) );
    }

    if( Accelerators.Count() == 61 )
    {
        Accelerators.RemoveAt( 37 );
        Accelerators.RemoveAt( 41 );
        WriteAStr( wxT( "AccelKey" ), Accelerators, wxT( "accelerators" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::LoadOldAlbumBrowser( wxFileConfig * fileconfig, const wxString &uniqueid )
{
    wxString EntryName;
    long EntryIndex = wxNOT_FOUND;
    if( fileconfig->GetFirstEntry( EntryName, EntryIndex ) )
    {
        do {
            //guLogMessage( wxT( "Reading Entry '%s'" ), EntryName.c_str() );
            WriteStr( EntryName, fileconfig->Read( EntryName, wxT( "0" ) ), wxT( "mediaviewers/mediaviewer_" ) + uniqueid + wxT( "/albumbrowser" ) );
        } while( fileconfig->GetNextEntry( EntryName, EntryIndex ) );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::LoadOldAlbumBrowserFilters( wxFileConfig * fileconfig, const wxString &uniqueid )
{
    wxString EntryName;
    long EntryIndex = wxNOT_FOUND;
    if( fileconfig->GetFirstEntry( EntryName, EntryIndex ) )
    {
        do {
            WriteStr( EntryName, fileconfig->Read( EntryName, wxEmptyString ), wxT( "mediaviewers/mediaviewer_" ) + uniqueid + wxT( "/albumbrowser/filters" ) );
        } while( fileconfig->GetNextEntry( EntryName, EntryIndex ) );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::LoadOldCommands( wxFileConfig * fileconfig )
{
    wxString EntryName;
    long EntryIndex = wxNOT_FOUND;
    if( fileconfig->GetFirstEntry( EntryName, EntryIndex ) )
    {
        do {
            wxString Value = fileconfig->Read( EntryName, wxEmptyString );
            if( EntryName.StartsWith( wxT( "Cmd" ) ) )
            {
                EntryName.Replace( wxT( "Cmd" ), wxT( "Exec" ) );
                WriteStr( EntryName, Value, wxT( "commands/execs" ) );
            }
            else
            {
                WriteStr( EntryName, Value, wxT( "commands/names" ) );
            }
        } while( fileconfig->GetNextEntry( EntryName, EntryIndex ) );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::LoadOldCopyTo( wxFileConfig * fileconfig )
{
    wxString EntryName;
    long EntryIndex = wxNOT_FOUND;
    if( fileconfig->GetFirstEntry( EntryName, EntryIndex ) )
    {
        do {
            WriteStr( EntryName, fileconfig->Read( EntryName, wxEmptyString ), wxT( "copyto/options" ) );
        } while( fileconfig->GetNextEntry( EntryName, EntryIndex ) );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::LoadOldCoverSearch( wxFileConfig * fileconfig, guMediaCollection * mediacollection )
{
    wxString EntryName;
    long EntryIndex = wxNOT_FOUND;
    if( fileconfig->GetFirstEntry( EntryName, EntryIndex ) )
    {
        do {
            mediacollection->m_CoverWords.Add( fileconfig->Read( EntryName, wxEmptyString ) );
        } while( fileconfig->GetNextEntry( EntryName, EntryIndex ) );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::LoadOldCrossfader( wxFileConfig * fileconfig )
{
    wxString EntryName;
    long EntryIndex = wxNOT_FOUND;
    if( fileconfig->GetFirstEntry( EntryName, EntryIndex ) )
    {
        do {
            WriteStr( EntryName, fileconfig->Read( EntryName, wxEmptyString ), wxT( "crossfader" ) );
        } while( fileconfig->GetNextEntry( EntryName, EntryIndex ) );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::LoadOldEqualizer( wxFileConfig * fileconfig )
{
    wxString EntryName;
    long EntryIndex = wxNOT_FOUND;
    if( fileconfig->GetFirstEntry( EntryName, EntryIndex ) )
    {
        do {
            WriteStr( EntryName, fileconfig->Read( EntryName, wxEmptyString ), wxT( "equalizer" ) );
        } while( fileconfig->GetNextEntry( EntryName, EntryIndex ) );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::LoadOldFileBrowser( wxFileConfig * fileconfig )
{
    wxString EntryName;
    long EntryIndex = wxNOT_FOUND;
    if( fileconfig->GetFirstEntry( EntryName, EntryIndex ) )
    {
        do {
            WriteStr( EntryName, fileconfig->Read( EntryName, wxEmptyString ), wxT( "filebrowser" ) );
        } while( fileconfig->GetNextEntry( EntryName, EntryIndex ) );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::LoadOldFileRenamer( wxFileConfig * fileconfig )
{
    wxString EntryName;
    long EntryIndex = wxNOT_FOUND;
    if( fileconfig->GetFirstEntry( EntryName, EntryIndex ) )
    {
        do {
            WriteStr( EntryName, fileconfig->Read( EntryName, wxEmptyString ), wxT( "filebrowser/filerenamer" ) );
        } while( fileconfig->GetNextEntry( EntryName, EntryIndex ) );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::LoadOldGeneral( wxFileConfig * fileconfig, guMediaCollection * mediacollection, const wxString &uniqueid )
{
    wxString EntryName;
    long EntryIndex = wxNOT_FOUND;
    if( fileconfig->GetFirstEntry( EntryName, EntryIndex ) )
    {
        do {
            if( EntryName == wxT( "AlbumYearOrder" ) )
            {
                WriteStr( wxT( "AlbumsOrder" ), fileconfig->Read( EntryName, wxEmptyString ), wxT( "mediaviewers/mediaviewer_" ) + uniqueid );
            }
            else if( EntryName == wxT( "BrowserCommand" ) ||
                    EntryName == wxT( "BufferSize" ) ||
                    EntryName == wxT( "CloseToTaskBar" ) ||
                    EntryName == wxT( "CoverSearchEngine" ) ||
                    EntryName == wxT( "DefaultActionEnqueue" ) ||
                    EntryName == wxT( "DropFilesClearPlaylist" ) ||
                    EntryName == wxT( "InstantTextSearchEnabled" ) ||
                    EntryName == wxT( "LoadDefaultLayouts" ) ||
                    EntryName == wxT( "MinSavePlayPosLength" ) ||
                    EntryName == wxT( "PlayerCurVol" ) ||
                    EntryName == wxT( "PlayerLoop" ) ||
                    EntryName == wxT( "PlayerSmart" ) ||
                    EntryName == wxT( "ReplayGainMode" ) ||
                    EntryName == wxT( "ReplayGainPreAmp" ) ||
                    EntryName == wxT( "RndModeOnEmptyPlayList" ) ||
                    EntryName == wxT( "RndPlayOnEmptyPlayList" ) ||
                    EntryName == wxT( "SaveCurrentTrackPos" ) ||
                    EntryName == wxT( "ShowCloseConfirm" ) ||
                    EntryName == wxT( "ShowNotifications" ) ||
                    EntryName == wxT( "ShowRevTime" ) ||
                    EntryName == wxT( "ShowSplashScreen" ) ||
                    EntryName == wxT( "ShowTaskBarIcon" ) ||
                    EntryName == wxT( "SoundMenuIntegration" ) ||
                    EntryName == wxT( "StartMinimized" ) ||
                    EntryName == wxT( "TextSearchEnterRelax" ) )
            {
                WriteStr( EntryName, fileconfig->Read( EntryName, wxEmptyString ), wxT( "general" ) );
            }
            else if( EntryName == wxT( "EmbedToFiles" ) )
            {
                mediacollection->m_EmbeddMetadata = ( fileconfig->Read( EntryName, wxEmptyString ) == wxT( "1" ) );
            }
            else if( EntryName == wxT( "JamendoLastUpdate" ) )
            {
                WriteStr( wxT( "LastUpdate" ), fileconfig->Read( EntryName, wxEmptyString ), wxT( "jamendo" ) );
            }
            else if( EntryName == wxT( "LasPreferencePage" ) ) // * Yes there was a typo and the name is missing a 't'
            {
                WriteStr( wxT( "LastPage" ), fileconfig->Read( EntryName, wxEmptyString ), wxT( "preferences" ) );
            }
            else if( EntryName == wxT( "LastUpdate" ) )
            {
                long LastUpdate = 0;
                fileconfig->Read( EntryName, wxEmptyString ).ToLong( &LastUpdate );
                mediacollection->m_LastUpdate = LastUpdate;
            }
            else if( EntryName.StartsWith( wxT( "LFM" ) ) )
            {
                wxString Value = fileconfig->Read( EntryName, wxEmptyString );
                EntryName.Replace( wxT( "LFM" ), wxEmptyString );
                WriteStr( EntryName, Value, wxT( "lastfm" ) );
            }
            else if( EntryName == wxT( "MagnatuneLastUpdate" ) )
            {
                WriteStr( wxT( "LastUpdate" ), fileconfig->Read( EntryName, wxEmptyString ), wxT( "magnatune" ) );
            }
            else if( EntryName == wxT( "PlayerCurItem" ) )
            {
                WriteStr( wxT( "CurItem" ), fileconfig->Read( EntryName, wxEmptyString ), wxT( "playlist/nowplaying" ) );
            }
            else if( EntryName == wxT( "SavePlayListOnClose" ) )
            {
                WriteStr( wxT( "SaveOnClose" ), fileconfig->Read( EntryName, wxEmptyString ), wxT( "playlist" ) );
            }
            else if( EntryName == wxT( "EmbedToFiles" ) )
            {
                mediacollection->m_EmbeddMetadata = ( fileconfig->Read( EntryName, wxEmptyString ) == wxT( "1" ) );
            }
            else if( EntryName == wxT( "ScanAddPlayLists" ) )
            {
                mediacollection->m_ScanPlaylists = ( fileconfig->Read( EntryName, wxEmptyString ) == wxT( "1" ) );
            }
            else if( EntryName == wxT( "ScanEmbeddedCovers" ) )
            {
                mediacollection->m_ScanEmbeddedCovers = ( fileconfig->Read( EntryName, wxEmptyString ) == wxT( "1" ) );
            }
            else if( EntryName == wxT( "ScanSymlinks" ) )
            {
                mediacollection->m_ScanFollowSymLinks = ( fileconfig->Read( EntryName, wxEmptyString ) == wxT( "1" ) );
            }
            else if( EntryName == wxT( "ShowFullScreen" ) )
            {
                WriteStr( EntryName, fileconfig->Read( EntryName, wxEmptyString ), wxT( "mainwindow" ) );
            }
            else if( EntryName == wxT( "ShowStatusBar" ) )
            {
                WriteStr( EntryName, fileconfig->Read( EntryName, wxEmptyString ), wxT( "mainwindow" ) );
            }
            else if( EntryName == wxT( "StationsOrder" ) || EntryName == wxT( "StationsOrderDesc" ) )
            {
                WriteStr( EntryName, fileconfig->Read( EntryName, wxEmptyString ), wxT( "radios" ) );
            }
            else if( EntryName == wxT( "UpdateLibOnStart" ) )
            {
                mediacollection->m_UpdateOnStart = ( fileconfig->Read( EntryName, wxEmptyString ) == wxT( "1" ) );
            }

            //WriteStr( EntryName, fileconfig->Read( EntryName, wxEmptyString ), wxT( "filebrowser/filerenamer" ) );
        } while( fileconfig->GetNextEntry( EntryName, EntryIndex ) );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::LoadOldJamendo( wxFileConfig * fileconfig )
{
    wxString EntryName;
    long EntryIndex = wxNOT_FOUND;
    if( fileconfig->GetFirstEntry( EntryName, EntryIndex ) )
    {
        do {
            WriteStr( EntryName, fileconfig->Read( EntryName, wxEmptyString ), wxT( "jamendo" ) );
        } while( fileconfig->GetNextEntry( EntryName, EntryIndex ) );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::LoadOldJamendoGenres( wxFileConfig * fileconfig )
{
    wxString EntryName;
    long EntryIndex = wxNOT_FOUND;
    if( fileconfig->GetFirstEntry( EntryName, EntryIndex ) )
    {
        do {
            WriteStr( EntryName, fileconfig->Read( EntryName, wxEmptyString ), wxT( "jamendo/genres" ) );
        } while( fileconfig->GetNextEntry( EntryName, EntryIndex ) );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::LoadOldLastFM( wxFileConfig * fileconfig )
{
    wxString EntryName;
    long EntryIndex = wxNOT_FOUND;
    if( fileconfig->GetFirstEntry( EntryName, EntryIndex ) )
    {
        do {
            WriteStr( EntryName, fileconfig->Read( EntryName, wxEmptyString ), wxT( "lastfm" ) );
        } while( fileconfig->GetNextEntry( EntryName, EntryIndex ) );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::LoadOldLibPaths( wxFileConfig * fileconfig, guMediaCollection * collection )
{
    wxString EntryName;
    long EntryIndex = wxNOT_FOUND;
    if( fileconfig->GetFirstEntry( EntryName, EntryIndex ) )
    {
        do {
            wxString Value = fileconfig->Read( EntryName, wxEmptyString );
            //guLogMessage( wxT( "Reading '%s' : '%s'" ), EntryName.c_str(), Value.c_str() );
            collection->m_Paths.Add( Value );
        } while( fileconfig->GetNextEntry( EntryName, EntryIndex ) );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::LoadOldLibreFM( wxFileConfig * fileconfig )
{
    wxString EntryName;
    long EntryIndex = wxNOT_FOUND;
    if( fileconfig->GetFirstEntry( EntryName, EntryIndex ) )
    {
        do {
            WriteStr( EntryName, fileconfig->Read( EntryName, wxEmptyString ), wxT( "librefm" ) );
        } while( fileconfig->GetNextEntry( EntryName, EntryIndex ) );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::LoadOldLyrics( wxFileConfig * fileconfig )
{
    wxString EntryName;
    long EntryIndex = wxNOT_FOUND;
    if( fileconfig->GetFirstEntry( EntryName, EntryIndex ) )
    {
        do {
            WriteStr( EntryName, fileconfig->Read( EntryName, wxEmptyString ), wxT( "lyrics" ) );
        } while( fileconfig->GetNextEntry( EntryName, EntryIndex ) );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::LoadOldMagnatune( wxFileConfig * fileconfig )
{
    wxString EntryName;
    long EntryIndex = wxNOT_FOUND;
    if( fileconfig->GetFirstEntry( EntryName, EntryIndex ) )
    {
        do {
            WriteStr( EntryName, fileconfig->Read( EntryName, wxEmptyString ), wxT( "magnatune" ) );
        } while( fileconfig->GetNextEntry( EntryName, EntryIndex ) );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::LoadOldMagnatuneGenres( wxFileConfig * fileconfig )
{
    wxString EntryName;
    long EntryIndex = wxNOT_FOUND;
    if( fileconfig->GetFirstEntry( EntryName, EntryIndex ) )
    {
        do {
            WriteStr( EntryName, fileconfig->Read( EntryName, wxEmptyString ), wxT( "magnatune/genres" ) );
        } while( fileconfig->GetNextEntry( EntryName, EntryIndex ) );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::LoadOldMagnatuneGenreList( wxFileConfig * fileconfig )
{
    wxString EntryName;
    long EntryIndex = wxNOT_FOUND;
    if( fileconfig->GetFirstEntry( EntryName, EntryIndex ) )
    {
        do {
            WriteStr( EntryName, fileconfig->Read( EntryName, wxEmptyString ), wxT( "magnatune/genrelist" ) );
        } while( fileconfig->GetNextEntry( EntryName, EntryIndex ) );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::LoadOldMainSources( wxFileConfig * fileconfig )
{
    wxString EntryName;
    long EntryIndex = wxNOT_FOUND;
    if( fileconfig->GetFirstEntry( EntryName, EntryIndex ) )
    {
        do {
            WriteStr( EntryName, fileconfig->Read( EntryName, wxEmptyString ), wxT( "mainsources" ) );
        } while( fileconfig->GetNextEntry( EntryName, EntryIndex ) );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::LoadOldPlayback( wxFileConfig * fileconfig )
{
    wxString EntryName;
    long EntryIndex = wxNOT_FOUND;
    if( fileconfig->GetFirstEntry( EntryName, EntryIndex ) )
    {
        do {
            WriteStr( EntryName, fileconfig->Read( EntryName, wxEmptyString ), wxT( "playback" ) );
        } while( fileconfig->GetNextEntry( EntryName, EntryIndex ) );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::LoadOldPlayList( wxFileConfig * fileconfig )
{
    wxString EntryName;
    long EntryIndex = wxNOT_FOUND;
    if( fileconfig->GetFirstEntry( EntryName, EntryIndex ) )
    {
        do {
            WriteStr( EntryName, fileconfig->Read( EntryName, wxEmptyString ), wxT( "playlist/nowplaying" ) );
        } while( fileconfig->GetNextEntry( EntryName, EntryIndex ) );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::LoadOldPodcasts( wxFileConfig * fileconfig )
{
    wxString EntryName;
    long EntryIndex = wxNOT_FOUND;
    if( fileconfig->GetFirstEntry( EntryName, EntryIndex ) )
    {
        do {
            WriteStr( EntryName, fileconfig->Read( EntryName, wxEmptyString ), wxT( "podcasts" ) );
        } while( fileconfig->GetNextEntry( EntryName, EntryIndex ) );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::LoadOldPositions( wxFileConfig * fileconfig )
{
    wxString EntryName;
    long EntryIndex = wxNOT_FOUND;
    if( fileconfig->GetFirstEntry( EntryName, EntryIndex ) )
    {
        do {
            if( EntryName.StartsWith( wxT( "MainWindow" ) ) )
            {
                wxString Value = fileconfig->Read( EntryName, wxEmptyString );
                EntryName.Replace( wxT( "MainWindowSize" ), wxEmptyString );
                EntryName.Replace( wxT( "MainWindow" ), wxEmptyString );
                WriteStr( EntryName, Value, wxT( "mainwindow/positions" ) );
            }
            else if( EntryName.StartsWith( wxT( "Preferences" ) ) )
            {
                wxString Value = fileconfig->Read( EntryName, wxEmptyString );
                EntryName.Replace( wxT( "PreferencesSize" ), wxEmptyString );
                EntryName.Replace( wxT( "Preferences" ), wxEmptyString );
                WriteStr( EntryName, Value, wxT( "preferences" ) );
            }
            else if( EntryName.StartsWith( wxT( "TrackEdit" ) ) ||
                     EntryName.StartsWith( wxT( "PMProperties" ) ) ||
                     EntryName.StartsWith( wxT( "LabelEdit" ) ) )
            {
                WriteStr( EntryName, fileconfig->Read( EntryName, wxEmptyString ), wxT( "positions" ) );
            }
            //WriteStr( EntryName, , wxT( "podcasts" ) );
        } while( fileconfig->GetNextEntry( EntryName, EntryIndex ) );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::LoadOldRadios( wxFileConfig * fileconfig )
{
    wxString EntryName;
    long EntryIndex = wxNOT_FOUND;
    if( fileconfig->GetFirstEntry( EntryName, EntryIndex ) )
    {
        do {
            WriteStr( EntryName, fileconfig->Read( EntryName, wxEmptyString ), wxT( "radios" ) );
        } while( fileconfig->GetNextEntry( EntryName, EntryIndex ) );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::LoadOldRecord( wxFileConfig * fileconfig )
{
    wxString EntryName;
    long EntryIndex = wxNOT_FOUND;
    if( fileconfig->GetFirstEntry( EntryName, EntryIndex ) )
    {
        do {
            WriteStr( EntryName, fileconfig->Read( EntryName, wxEmptyString ), wxT( "record" ) );
        } while( fileconfig->GetNextEntry( EntryName, EntryIndex ) );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::LoadOldSearchFilters( wxFileConfig * fileconfig )
{
    wxString EntryName;
    long EntryIndex = wxNOT_FOUND;
    if( fileconfig->GetFirstEntry( EntryName, EntryIndex ) )
    {
        do {
            WriteStr( EntryName, fileconfig->Read( EntryName, wxEmptyString ), wxT( "searchfilters" ) );
        } while( fileconfig->GetNextEntry( EntryName, EntryIndex ) );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::LoadOldSearchLinks( wxFileConfig * fileconfig )
{
    wxString EntryName;
    long EntryIndex = wxNOT_FOUND;
    if( fileconfig->GetFirstEntry( EntryName, EntryIndex ) )
    {
        do {
            wxString Value = fileconfig->Read( EntryName, wxEmptyString );
            if( EntryName.StartsWith( wxT( "Link" ) ) )
            {
                WriteStr( EntryName, Value, wxT( "searchlinks/links" ) );
            }
            else
            {
                WriteStr( EntryName, Value, wxT( "searchlinks/names" ) );
            }
        } while( fileconfig->GetNextEntry( EntryName, EntryIndex ) );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::LoadOldTreeView( wxFileConfig * fileconfig, const wxString &uniqueid )
{
    wxString EntryName;
    long EntryIndex = wxNOT_FOUND;
    if( fileconfig->GetFirstEntry( EntryName, EntryIndex ) )
    {
        do {
            WriteStr( EntryName, fileconfig->Read( EntryName, wxEmptyString ), wxT( "mediaviewers/mediaviewer_" ) + uniqueid + wxT( "/treeview" ) );
        } while( fileconfig->GetNextEntry( EntryName, EntryIndex ) );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::LoadOldTreeViewFilters( wxFileConfig * fileconfig, const wxString &uniqueid )
{
    wxString EntryName;
    long EntryIndex = wxNOT_FOUND;
    if( fileconfig->GetFirstEntry( EntryName, EntryIndex ) )
    {
        do {
            WriteStr( EntryName, fileconfig->Read( EntryName, wxEmptyString ), wxT( "mediaviewers/mediaviewer_" ) + uniqueid + wxT( "/treeview/sortings" ) );
        } while( fileconfig->GetNextEntry( EntryName, EntryIndex ) );
    }
}

// -------------------------------------------------------------------------------- //
void GetGroupNames( wxFileConfig * fileconfig, wxArrayString &groupnames )
{
    wxString    GroupName;
    long GroupIndex = wxNOT_FOUND;
    if( fileconfig->GetFirstGroup( GroupName, GroupIndex ) )
    {
        do {
            //guLogMessage( wxT( "Reading entries from [%s]" ), GroupName.c_str() );
            groupnames.Add( GroupName );
        } while( fileconfig->GetNextGroup( GroupName, GroupIndex ) );
    }
}

// -------------------------------------------------------------------------------- //
void TransferRadioDb( const wxString &olddb, const wxString &newdb )
{
    guDbRadios * OldDb = new guDbRadios( olddb );
    if( OldDb )
    {
        // Create the database with the initial structure
        guDbRadios * NewDb = new guDbRadios( newdb );
        if( NewDb )
        {
            delete NewDb;

            wxArrayString query;

            query.Add( wxString::Format( wxT( "ATTACH DATABASE '%s' AS NewDb" ), escape_query_str( newdb ).c_str() ) );

            query.Add( wxT( "DELETE FROM NewDb.radiogenres;" ) );
            query.Add( wxT( "DELETE FROM NewDb.radiostations;" ) );
            query.Add( wxT( "DELETE FROM NewDb.radiolabels;" ) );
            query.Add( wxT( "DELETE FROM NewDb.radiosetlabels;" ) );

            query.Add( wxT( "INSERT INTO NewDb.radiogenres SELECT * FROM radiogenres;" ) );
            query.Add( wxT( "INSERT INTO NewDb.radiostations SELECT * FROM radiostations;" ) );
            query.Add( wxT( "INSERT INTO NewDb.radiolabels SELECT * FROM radiolabels;" ) );
            query.Add( wxT( "INSERT INTO NewDb.radiosetlabels SELECT * FROM radiosetlabels;" ) );

            int Index;
            int Count = query.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                OldDb->ExecuteUpdate( query[ Index ] );
            }
        }
        else
        {
            guLogMessage( wxT( "Could not open the db '%s'" ), newdb.c_str() );
        }

        delete OldDb;
    }
    else
    {
        guLogMessage( wxT( "Could not open the old db '%s'" ), olddb.c_str() );
        return;
    }
}

// -------------------------------------------------------------------------------- //
void TransferPodcastDb( const wxString &olddb, const wxString &newdb )
{
    guDbPodcasts * OldDb = new guDbPodcasts( olddb );
    if( OldDb )
    {
        // Create the database with the initial structure
        guDbPodcasts * NewDb = new guDbPodcasts( newdb );
        if( NewDb )
        {
            delete NewDb;

            wxArrayString query;

            query.Add( wxString::Format( wxT( "ATTACH DATABASE '%s' AS NewDb" ), escape_query_str( newdb ).c_str() ) );

            query.Add( wxT( "DELETE FROM NewDb.podcastchs;" ) );
            query.Add( wxT( "DELETE FROM NewDb.podcastitems;" ) );

            query.Add( wxT( "INSERT INTO NewDb.podcastchs SELECT * FROM podcastchs;" ) );
            query.Add( wxT( "INSERT INTO NewDb.podcastitems SELECT * FROM podcastitems;" ) );

            int Index;
            int Count = query.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                OldDb->ExecuteUpdate( query[ Index ] );
            }
        }
        else
        {
            guLogMessage( wxT( "Could not open the db '%s'" ), newdb.c_str() );
        }

        delete OldDb;
    }
    else
    {
        guLogMessage( wxT( "Could not open the old db '%s'" ), olddb.c_str() );
        return;
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::LoadOldConfig( const wxString &conffile )
{
    guLogMessage( wxT( "Reading old configuration from file '%s'" ), conffile.c_str() );

    wxFileConfig * FileConfig = new wxFileConfig( wxEmptyString, wxEmptyString, conffile );
    if( FileConfig )
    {
        wxArrayString GroupNames;
        GetGroupNames( FileConfig, GroupNames );

        wxString UniqueId = wxString::Format( wxT( "%08lX" ), wxGetLocalTime() );

        guMediaCollectionArray MediaCollections;

        guMediaCollection * MediaCollection = new guMediaCollection( guMEDIA_COLLECTION_TYPE_NORMAL );
        MediaCollection->m_Name = _( "My Music" );
        MediaCollection->m_UniqueId = UniqueId;
        MediaCollections.Add( MediaCollection );

        if( !wxDirExists( guPATH_COLLECTIONS ) )
        {
            wxMkdir( guPATH_COLLECTIONS, 0770 );
            guLogMessage( wxT( "Created the collections folder" ) );
        }

        if( !wxDirExists( guPATH_COLLECTIONS + UniqueId ) )
        {
            wxMkdir( guPATH_COLLECTIONS + UniqueId, 0770 );
        }

        TransferRadioDb( guPATH_OLD_DBNAME, guPATH_RADIOS_DBNAME );

        TransferPodcastDb( guPATH_OLD_DBNAME, guPATH_PODCASTS_DBNAME );

        if( wxFileExists( guPATH_OLD_DBNAME ) )
        {
            wxCopyFile( guPATH_OLD_DBNAME, guPATH_COLLECTIONS + UniqueId + wxT( "/" ) guPATH_DBNAME );
            wxRenameFile( guPATH_OLD_DBNAME, guPATH_OLD_DBNAME wxT( ".old" ) );
        }

        int Index;
        int Count = GroupNames.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            wxString GroupName = GroupNames[ Index ];

            //guLogMessage( wxT( "Reading entries from [%s]" ), GroupName.c_str() );

            FileConfig->SetPath( wxT( "/" ) + GroupName );

            if(      GroupName == wxT( "Accelerators" ) )           { LoadOldAccelerators( FileConfig ); }
            else if( GroupName == wxT( "AlbumBrowser" ) )           { LoadOldAlbumBrowser( FileConfig, UniqueId ); }
            else if( GroupName == wxT( "AlbumBrowserFilters" ) )    { LoadOldAlbumBrowserFilters( FileConfig, UniqueId ); }
            else if( GroupName == wxT( "Commands" ) )               { LoadOldCommands( FileConfig ); }
            else if( GroupName == wxT( "CopyTo" ) )                 { LoadOldCopyTo( FileConfig ); }
            else if( GroupName == wxT( "CoverSearch" ) )            { LoadOldCoverSearch( FileConfig, MediaCollection ); }
            else if( GroupName == wxT( "Crossfader" ) )             { LoadOldCrossfader( FileConfig ); }
            else if( GroupName == wxT( "Equalizer" ) )              { LoadOldEqualizer( FileConfig ); }
            else if( GroupName == wxT( "FileBrowser" ) )            { LoadOldFileBrowser( FileConfig ); }
            else if( GroupName == wxT( "FileRenamer" ) )            { LoadOldFileRenamer( FileConfig ); }
            else if( GroupName == wxT( "General" ) )                { LoadOldGeneral( FileConfig, MediaCollection, UniqueId ); }
            else if( GroupName == wxT( "Jamendo" ) )                { LoadOldJamendo( FileConfig ); }
            else if( GroupName == wxT( "JamendoGenres" ) )          { LoadOldJamendoGenres( FileConfig ); }
            else if( GroupName == wxT( "LastFM" ) )                 { LoadOldLastFM( FileConfig ); }
            else if( GroupName == wxT( "LibPaths" ) )               { LoadOldLibPaths( FileConfig, MediaCollection ); }
            else if( GroupName == wxT( "LibreFM" ) )                { LoadOldLibreFM( FileConfig ); }
            else if( GroupName == wxT( "Lyrics" ) )                 { LoadOldLyrics( FileConfig ); }
            else if( GroupName == wxT( "Magnatune" ) )              { LoadOldMagnatune( FileConfig ); }
            else if( GroupName == wxT( "MagnatuneGenreList" ) )     { LoadOldMagnatuneGenreList( FileConfig ); }
            else if( GroupName == wxT( "MagnatuneGenres" ) )        { LoadOldMagnatuneGenres( FileConfig ); }
            else if( GroupName == wxT( "MainSources" ) )            { LoadOldMainSources( FileConfig ); }
            else if( GroupName == wxT( "Playback" ) )               { LoadOldPlayback( FileConfig ); }
            else if( GroupName == wxT( "PlayList" ) )               { LoadOldPlayList( FileConfig ); }
            else if( GroupName == wxT( "Podcasts" ) )               { LoadOldPodcasts( FileConfig ); }
            else if( GroupName == wxT( "Positions" ) )              { LoadOldPositions( FileConfig ); }
            else if( GroupName == wxT( "Radios" ) )                 { LoadOldRadios( FileConfig ); }
            else if( GroupName == wxT( "Record" ) )                 { LoadOldRecord( FileConfig ); }
            else if( GroupName == wxT( "SearchFilters" ) )          { LoadOldSearchFilters( FileConfig ); }
            else if( GroupName == wxT( "SearchLinks" ) )            { LoadOldSearchLinks( FileConfig ); }
            else if( GroupName == wxT( "TreeView" ) )               { LoadOldTreeView( FileConfig, UniqueId ); }
            else if( GroupName == wxT( "TreeViewFilters" ) )        { LoadOldTreeViewFilters( FileConfig, UniqueId ); }

        }

        MediaCollection = new guMediaCollection( guMEDIA_COLLECTION_TYPE_JAMENDO );
        MediaCollection->m_Name = wxT( "Jamendo" );
        MediaCollection->m_UniqueId = wxT( "Jamendo" );
        MediaCollections.Add( MediaCollection );

        MediaCollection = new guMediaCollection( guMEDIA_COLLECTION_TYPE_MAGNATUNE );
        MediaCollection->m_Name = wxT( "Magnatune" );
        MediaCollection->m_UniqueId = wxT( "Magnatune" );
        MediaCollections.Add( MediaCollection );

        SaveCollections( &MediaCollections );

        delete FileConfig;
    }
}

// -------------------------------------------------------------------------------- //
void WriteTrack( wxXmlNode * xmlnode, const guTrack &track )
{
    wxXmlNode * XmlNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "track" ) );

    WriteStr( XmlNode, wxT( "Type" ), wxString::Format( wxT( "%i" ), track.m_Type ) );
    if( track.m_MediaViewer )
    {
        //WriteStr( XmlNode, wxT( "MediaViewer" ), track.m_MediaViewer->GetUniqueId() );
    }
    WriteStr( XmlNode, wxT( "Name" ), track.m_SongName );
    WriteStr( XmlNode, wxT( "Artist" ), track.m_ArtistName );
    WriteStr( XmlNode, wxT( "AlbumArtist" ), track.m_AlbumArtist );
    WriteStr( XmlNode, wxT( "Composer" ), track.m_Composer );
    WriteStr( XmlNode, wxT( "Album" ), track.m_AlbumName );
    WriteStr( XmlNode, wxT( "Path" ), track.m_Path );
    WriteStr( XmlNode, wxT( "FileName" ), track.m_FileName );
    WriteStr( XmlNode, wxT( "Number" ), wxString::Format( wxT( "%i" ), track.m_Number ) );
    WriteStr( XmlNode, wxT( "Rating" ), wxString::Format( wxT( "%i" ), track.m_Rating ) );
    WriteStr( XmlNode, wxT( "Offset" ), wxString::Format( wxT( "%u" ), track.m_Offset ) );
    WriteStr( XmlNode, wxT( "Length" ), wxString::Format( wxT( "%u" ), track.m_Length ) );

    xmlnode->AddChild( XmlNode );
}

// -------------------------------------------------------------------------------- //
int inline StrToInt( const wxString &value )
{
    long RetVal = 0;
    value.ToLong( &RetVal );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
void ReadTrack( wxXmlNode * xmlnode, guTrack &track )
{
    while( xmlnode )
    {
        wxString Name = xmlnode->GetName();
        wxString Value;
        if( Name == wxT( "Type" ) )
        {
            xmlnode->GetAttribute( wxT( "value" ), &Value );
            track.m_Type = ( guTrackType ) StrToInt( Value );
        }
        else if( Name == wxT( "MediaViewer" ) )
        {
        }
        else if( Name == wxT( "Name" ) )
        {
            xmlnode->GetAttribute( wxT( "value" ), &track.m_SongName );
        }
        else if( Name == wxT( "Artist" ) )
        {
            xmlnode->GetAttribute( wxT( "value" ), &track.m_ArtistName );
        }
        else if( Name == wxT( "AlbumArtist" ) )
        {
            xmlnode->GetAttribute( wxT( "value" ), &track.m_AlbumArtist );
        }
        else if( Name == wxT( "Composer" ) )
        {
            xmlnode->GetAttribute( wxT( "value" ), &track.m_Composer );
        }
        else if( Name == wxT( "Album" ) )
        {
            xmlnode->GetAttribute( wxT( "value" ), &track.m_AlbumName );
        }
        else if( Name == wxT( "Path" ) )
        {
            xmlnode->GetAttribute( wxT( "value" ), &track.m_Path );
        }
        else if( Name == wxT( "FileName" ) )
        {
            xmlnode->GetAttribute( wxT( "value" ), &track.m_FileName );
        }
        else if( Name == wxT( "Number" ) )
        {
            xmlnode->GetAttribute( wxT( "value" ), &Value );
            track.m_Number = StrToInt( Value );
        }
        else if( Name == wxT( "Rating" ) )
        {
            xmlnode->GetAttribute( wxT( "value" ), &Value );
            track.m_Rating = StrToInt( Value );
        }
        else if( Name == wxT( "Offset" ) )
        {
            xmlnode->GetAttribute( wxT( "value" ), &Value );
            track.m_Offset = StrToInt( Value );
        }
        else if( Name == wxT( "Length" ) )
        {
            xmlnode->GetAttribute( wxT( "value" ), &Value );
            track.m_Length = StrToInt( Value );
        }

        xmlnode = xmlnode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
bool guConfig::SavePlaylistTracks( const guTrackArray &tracks, const int currenttrack )
{
    DeleteCategory( wxT( "playlist/nowplaying") );

    WriteNum( wxT( "Count" ), tracks.Count(), wxT( "playlist/nowplaying" ) );
    WriteNum( wxT( "CurItem" ), currenttrack, wxT( "playlist/nowplaying" ) );

    wxXmlNode * XmlNode = CreateCategoryNode( m_RootNode, wxT( "playlist/nowplaying/tracks" ) );

    int Index;
    int Count = tracks.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        WriteTrack( XmlNode, tracks[ Index ] );
    }
    return true;
}

// -------------------------------------------------------------------------------- //
int guConfig::LoadPlaylistTracks( guTrackArray &tracks )
{
    int CurItem = ReadNum( wxT( "CurItem" ), wxNOT_FOUND, wxT( "playlist/nowplaying" ) );
    wxXmlNode * XmlNode = FindNode( wxT( "playlist/nowplaying/tracks" ) );
    if( XmlNode )
    {
        XmlNode = XmlNode->GetChildren();
        while( XmlNode )
        {
            if( XmlNode->GetName() == wxT( "track" ) )
            {
                guTrack Track;
                ReadTrack( XmlNode->GetChildren(), Track );
                tracks.Add( new guTrack( Track ) );
            }
            XmlNode = XmlNode->GetNext();
        }
    }

    return CurItem;
}

// -------------------------------------------------------------------------------- //
