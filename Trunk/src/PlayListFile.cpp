// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2009 J.Rios
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
#include "PlayListFile.h"

#include "Utils.h"

#include <wx/fileconf.h>
#include <wx/sstream.h>
#include <wx/tokenzr.h>
#include <wx/wfstream.h>

// -------------------------------------------------------------------------------- //
guPlayListFile::guPlayListFile( const wxString &filename )
{
    if( !filename.IsEmpty() )
        Load( filename );
}

// -------------------------------------------------------------------------------- //
guPlayListFile::~guPlayListFile()
{
}

// -------------------------------------------------------------------------------- //
bool guPlayListFile::Load( const wxString &filename )
{
    if( filename.Lower().EndsWith( wxT( ".pls" ) ) )
    {
        return ReadPlsFile( filename );
    }
    else if( filename.Lower().EndsWith( wxT( ".m3u" ) ) )
    {
        return ReadM3uFile( filename );
    }
    else if( filename.Lower().EndsWith( wxT( ".xspf" ) ) )
    {
        return ReadXspfFile( filename );
    }
    else if( filename.Lower().EndsWith( wxT( ".asx" ) ) )
    {
        return ReadAsxFile( filename );
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guPlayListFile::Save( const wxString &filename )
{
    if( filename.Lower().EndsWith( wxT( ".pls" ) ) )
    {
        return WritePlsFile( filename );
    }
    else if( filename.Lower().EndsWith( wxT( ".m3u" ) ) )
    {
        return WriteM3uFile( filename );
    }
    else if( filename.Lower().EndsWith( wxT( ".xspf" ) ) )
    {
        return WriteXspfFile( filename );
    }
    else if( filename.Lower().EndsWith( wxT( ".asx" ) ) )
    {
        return WriteAsxFile( filename );
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guPlayListFile::ReadPlsFile( const wxString &filename )
{
    wxFileInputStream Ins( filename );
    if( Ins.IsOk() )
    {
        wxFileConfig * Config = new wxFileConfig( Ins );
        if( Config )
        {
            if( Config->HasGroup( wxT( "playlist" ) ) )
            {
                Config->SetPath( wxT( "playlist" ) );
                int Count;
                if( Config->Read( wxT( "numberofentries" ), &Count ) )
                {
                    if( !Count )
                    {
                        guLogMessage( wxT( "This station playlist is empty" ) );
                    }
                    else
                    {
                        for( int Index = 1; Index <= Count; Index++ )
                        {
                            wxString Entry;
                            Config->Read( wxString::Format( wxT( "File%u" ), Index ), &Entry );
                            if( !Entry.IsEmpty() )
                                m_Files.Add( Entry );
                        }
                        return true;
                    }
                }
            }
            else
            {
                guLogError( wxT( "ee: Station Playlist without 'playlist' group" ) );
            }
            delete Config;
        }
        else
        {
            guLogError( wxT( "Could not read the playlist file '%s'" ), filename.c_str() );
        }
    }
    else
    {
        guLogError( wxT( "Could not open the playlist file '%s'" ), filename.c_str() );
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guPlayListFile::ReadM3uFile( const wxString &filename )
{
    wxFileInputStream Ins( filename );
    if( Ins.IsOk() )
    {
        wxString M3uFile;
        wxStringOutputStream Outs( &M3uFile );
        Ins.Read( Outs );

        if( !M3uFile.IsEmpty() )
        {
            //guLogMessage( wxT( "Content...\n%s" ), M3uFile.c_str() );
            wxArrayString Lines = wxStringTokenize( M3uFile );

            int Index;
            int Count = Lines.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                Lines[ Index ].Trim( wxString::both );
                if( Lines[ Index ].IsEmpty() || ( Lines[ Index ].Find( wxT( "#EXTM3U" ) ) != wxNOT_FOUND ) )
                {
                    continue;
                }
                else if( Lines[ Index ].Find( wxT( "#EXTINF" ) ) != wxNOT_FOUND )
                {
                    if( m_Name.IsEmpty() )
                    {
                        if( Lines[ Index ].Find( wxT( "," ) ) != wxNOT_FOUND )
                            m_Name = Lines[ Index ].AfterLast( wxT( ',' ) );
                    }
                }
                else
                {
                    if( wxPathOnly( Lines[ Index ] ).IsEmpty() )
                        Lines[ Index ] = wxPathOnly( filename ) + wxT( "/" ) + Lines[ Index ];
                    m_Files.Add( Lines[ Index ] );
                }
            }
            return true;
        }
        else
        {
            guLogError( wxT( "Empty playlist file '%s'" ), filename.c_str() );
        }
    }
    else
    {
        guLogError( wxT( "Could not open the playlist file '%s'" ), filename.c_str() );
    }
    return false;
}

// -------------------------------------------------------------------------------- //
void guPlayListFile::ReadXspfTrack( wxXmlNode * XmlNode )
{
    while( XmlNode )
    {
        if( XmlNode->GetName().Lower() == wxT( "location" ) )
        {
            m_Files.Add( XmlNode->GetNodeContent() );
        }
        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void guPlayListFile::ReadXspfTrackList( wxXmlNode * XmlNode )
{
    while( XmlNode )
    {
        if( XmlNode->GetName().Lower() == wxT( "track" ) )
        {
            ReadXspfTrack( XmlNode->GetChildren() );
        }
        XmlNode = XmlNode->GetNext();
    }

}

// -------------------------------------------------------------------------------- //
void guPlayListFile::ReadXspfPlayList( wxXmlNode * XmlNode )
{
    while( XmlNode )
    {
        if( XmlNode->GetName().Lower() == wxT( "title" ) )
        {
            m_Name = XmlNode->GetNodeContent();
        }
        else if( XmlNode->GetName().Lower() == wxT( "tracklist" ) )
        {
            ReadXspfTrackList( XmlNode->GetChildren() );
        }
        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
bool guPlayListFile::ReadXspfFile( const wxString &filename )
{
    wxFileInputStream Ins( filename );
    if( Ins.IsOk() )
    {
        wxXmlDocument XmlDoc( Ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        if( XmlNode && XmlNode->GetName().Lower() == wxT( "playlist" ) )
        {
            ReadXspfPlayList( XmlNode->GetChildren() );
        }
        return true;
    }
    else
    {
        guLogError( wxT( "Could not open the playlist file '%s'" ), filename.c_str() );
    }
    return false;
}

// -------------------------------------------------------------------------------- //
void guPlayListFile::ReadAsxEntry( wxXmlNode * XmlNode )
{
    while( XmlNode )
    {
        if( XmlNode->GetName().Lower() == wxT( "ref" ) )
        {
            wxString StreamLink;
            XmlNode->GetPropVal( wxT( "href" ), &StreamLink );
            if( !StreamLink.IsEmpty() )
                m_Files.Add( StreamLink );
        }
        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
void guPlayListFile::ReadAsxPlayList( wxXmlNode * XmlNode )
{
    while( XmlNode )
    {
        if( XmlNode->GetName().Lower() == wxT( "title" ) )
        {
            m_Name = XmlNode->GetNodeContent();
        }
        else if( XmlNode->GetName().Lower() == wxT( "entry" ) )
        {
            ReadAsxEntry( XmlNode->GetChildren() );
        }
        XmlNode = XmlNode->GetNext();
    }
}

// -------------------------------------------------------------------------------- //
bool guPlayListFile::ReadAsxFile( const wxString &filename )
{
    wxFileInputStream Ins( filename );
    if( Ins.IsOk() )
    {
        wxXmlDocument XmlDoc( Ins );
        wxXmlNode * XmlNode = XmlDoc.GetRoot();
        if( XmlNode && XmlNode->GetName().Lower() == wxT( "asx" ) )
        {
            ReadAsxPlayList( XmlNode->GetChildren() );
        }
        return true;
    }
    else
    {
        guLogError( wxT( "Could not open the playlist file '%s'" ), filename.c_str() );
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guPlayListFile::WritePlsFile( const wxString &filename )
{
    wxFileInputStream Ins( filename );
    if( Ins.IsOk() )
    {
        wxFileConfig * Config = new wxFileConfig( Ins );
        if( Config )
        {
            if( Config->HasGroup( wxT( "playlist" ) ) )
            {
                Config->DeleteGroup( wxT( "playlist" ) );
            }

            Config->SetPath( wxT( "playlist" ) );
            int Count = m_Files.Count();
            Config->Write( wxT( "numberofentries" ), Count );
            for( int Index = 1; Index <= Count; Index++ )
            {
                Config->Write( wxString::Format( wxT( "File%u" ), Index ), m_Files[ Index - 1 ] );
            }

            wxFileOutputStream Outs( filename );

            Config->Save( Outs );

            delete Config;
        }
        else
        {
            guLogError( wxT( "Could not read the playlist file '%s'" ), filename.c_str() );
        }
    }
    else
    {
        guLogError( wxT( "Could not open the playlist file '%s'" ), filename.c_str() );
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guPlayListFile::WriteM3uFile( const wxString &filename )
{
    wxFile M3uFile;
    if( M3uFile.Create( filename, true ) && M3uFile.IsOpened() )
    {
        M3uFile.Write( wxT( "#EXTM3U\n" ) );
        int Count = m_Files.Count();
        for( int Index = 0; Index < Count; Index++ )
        {
            M3uFile.Write( m_Files[ Index ] );
            M3uFile.Write( wxT( "\n" ) );
        }
        M3uFile.Close();
    }
    else
    {
        guLogError( wxT( "Could not open the m3ufile '%s'" ), filename.c_str() );
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guPlayListFile::WriteXspfFile( const wxString &filename )
{
    wxXmlDocument OutXml;
    //OutXml.SetRoot(  );
    wxXmlNode * RootNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "playlist" ) );

    wxXmlNode * TitleNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "title" ) );
    wxXmlNode * TitleNodeVal = new wxXmlNode( wxXML_TEXT_NODE, wxT( "title" ), m_Name );
    TitleNode->AddChild( TitleNodeVal );
    RootNode->AddChild( TitleNode );

    wxXmlNode * TrackListNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "trackList" ) );

    int Count = m_Files.Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        wxXmlNode * TrackNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "track" ) );

        wxXmlNode * LocationNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "location" ) );
        wxXmlNode * LocationNodeVal = new wxXmlNode( wxXML_TEXT_NODE, wxT( "location" ), m_Files[ Index ] );
        LocationNode->AddChild( LocationNodeVal );
        TrackNode->AddChild( LocationNode );

        TrackListNode->AddChild( TrackNode );
    }
    RootNode->AddChild( TrackListNode );
    OutXml.SetRoot( RootNode );
    return OutXml.Save( filename );
}

// -------------------------------------------------------------------------------- //
bool guPlayListFile::WriteAsxFile( const wxString &filename )
{
    wxXmlDocument OutXml;
    //OutXml.SetRoot(  );
    wxXmlNode * RootNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "asx" ) );

    wxXmlNode * TitleNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "title" ) );
    wxXmlNode * TitleNodeVal = new wxXmlNode( wxXML_TEXT_NODE, wxT( "title" ), m_Name );
    TitleNode->AddChild( TitleNodeVal );
    RootNode->AddChild( TitleNode );

    int Count = m_Files.Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        wxXmlNode * EntryNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "entry" ) );

        wxXmlNode * RefNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "ref" ) );
        wxXmlProperty * RefNodeVal = new wxXmlProperty( wxT( "href" ), m_Files[ Index ], NULL );
        RefNode->SetProperties( RefNodeVal );
        EntryNode->AddChild( RefNode );

        RootNode->AddChild( EntryNode );
    }
    OutXml.SetRoot( RootNode );
    return OutXml.Save( filename );
}

// -------------------------------------------------------------------------------- //
