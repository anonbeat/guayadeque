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
#include "PlayListFile.h"

#include "Utils.h"

#include <wx/arrimpl.cpp>
#include <wx/fileconf.h>
#include <wx/filename.h>
#include <wx/sstream.h>
#include <wx/tokenzr.h>
#include <wx/uri.h>
#include <wx/wfstream.h>
#include <wx/xml/xml.h>

WX_DEFINE_OBJARRAY(guStationPlayList);

// -------------------------------------------------------------------------------- //
guPlayListFile::guPlayListFile( const wxString &uri )
{
    if( !uri.IsEmpty() )
        Load( uri );
}

// -------------------------------------------------------------------------------- //
guPlayListFile::~guPlayListFile()
{
}

// -------------------------------------------------------------------------------- //
bool guPlayListFile::IsValidPlayList( const wxString &uri )
{
    wxURI Uri( uri );

    wxString PLPath = Uri.GetPath().Lower();

    return  PLPath.EndsWith( wxT( ".pls" ) ) ||
            PLPath.EndsWith( wxT( ".m3u" ) ) ||
            PLPath.EndsWith( wxT( ".xspf" ) ) ||
            PLPath.EndsWith( wxT( ".asx" ) );
}

// -------------------------------------------------------------------------------- //
bool guPlayListFile::Load( const wxString &uri )
{
    wxURI Uri( uri );

    //guLogMessage( wxT( "'%s' IsReference:%i" ), uri.c_str(), Uri.IsReference() );
    if( Uri.IsReference() )
    {
        wxString PLPath = uri.Lower();
        if( PLPath.EndsWith( wxT( ".pls" ) ) )
        {
            return ReadPlsFile( uri );
        }
        else if( PLPath.EndsWith( wxT( ".m3u" ) ) )
        {
            return ReadM3uFile( uri );
        }
        else if( PLPath.EndsWith( wxT( ".xspf" ) ) )
        {
            return ReadXspfFile( uri );
        }
        else if( PLPath.EndsWith( wxT( ".asx" ) ) )
        {
            return ReadAsxFile( uri );
        }
    }
    else
    {
        if( !IsValidPlayList( uri ) )
        {
            m_PlayList.Add( new guStationPlayListItem( uri, uri ) );
            return true;
        }
        else
        {
            guLogMessage( wxT( "Trying to get the uri: %s" ), uri.c_str() );
            wxString Content = GetUrlContent( uri );
            if( !Content.IsEmpty() )
            {
                wxStringInputStream Ins( Content );

                wxString PLPath = Uri.GetPath().Lower();
                if( PLPath.EndsWith( wxT( ".pls" ) ) )
                {
                    return ReadPlsStream( Ins );
                }
                else if( PLPath.EndsWith( wxT( ".m3u" ) ) )
                {
                    return ReadM3uStream( Ins );
                }
                else if( PLPath.EndsWith( wxT( ".xspf" ) ) )
                {
                    return ReadXspfStream( Ins );
                }
                else if( PLPath.EndsWith( wxT( ".asx" ) ) )
                {
                    return ReadAsxStream( Ins );
                }
            }
            else
            {
                guLogError( wxT( "Could not get the playlist '%s'" ), uri.c_str() );
            }
        }
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
    else if( filename.Lower().EndsWith( wxT( ".xspf" ) ) )
    {
        return WriteXspfFile( filename );
    }
    else if( filename.Lower().EndsWith( wxT( ".asx" ) ) )
    {
        return WriteAsxFile( filename );
    }
    else
    {
        wxString FileName = filename;
        if( !FileName.Lower().EndsWith( wxT( ".m3u" ) ) )
            FileName += wxT( ".m3u" );
        return WriteM3uFile( FileName );
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guPlayListFile::ReadPlsStream( wxInputStream &playlist, const wxString &path )
{
    wxFileConfig * Config = new wxFileConfig( playlist );
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
                        wxString Location;
                        wxString Title;

                        if( Config->Read( wxString::Format( wxT( "File%u" ), Index ), &Location ) &&
                            !Location.IsEmpty() )
                        {
                            Config->Read( wxString::Format( wxT( "Title%u" ), Index ), &Title );

                            wxURI Uri( Location );
                            if( Location.StartsWith( wxT( "/" ) ) || Uri.HasScheme() || path.IsEmpty() )
                            {
                                m_PlayList.Add( new guStationPlayListItem( Location, Title ) );
                            }
                            else
                            {
                                wxFileName FileName( Location );
                                FileName.Normalize( wxPATH_NORM_ALL, path );

                                m_PlayList.Add( new guStationPlayListItem( FileName.GetFullPath(), Title ) );
                            }
                        }
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
        guLogError( wxT( "Could not read the playlist stream" ) );
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guPlayListFile::ReadPlsFile( const wxString &filename )
{
    wxFileInputStream Ins( filename );
    if( Ins.IsOk() )
    {
        return ReadPlsStream( Ins, wxPathOnly( filename ) );
    }
    else
    {
        guLogError( wxT( "Could not open the playlist file '%s'" ), filename.c_str() );
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guPlayListFile::ReadM3uStream( wxInputStream &playlist, const wxString &path )
{
    wxString M3uFile;
    wxStringOutputStream Outs( &M3uFile );
    playlist.Read( Outs );

    if( !M3uFile.IsEmpty() )
    {
        //guLogMessage( wxT( "Content...\n%s" ), M3uFile.c_str() );
        wxString ItemName;
        wxArrayString Lines = wxStringTokenize( M3uFile, wxT( "\n" ) );

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
                if( Lines[ Index ].Find( wxT( "," ) ) != wxNOT_FOUND )
                    ItemName = Lines[ Index ].AfterLast( wxT( ',' ) );
            }
            else
            {

                wxURI Uri( Lines[ Index ] );
                if( Lines[ Index ].StartsWith( wxT( "/" ) ) || Uri.HasScheme() || path.IsEmpty() )
                {
                    m_PlayList.Add( new guStationPlayListItem( Lines[ Index ], ItemName ) );
                }
                else
                {
                    wxFileName FileName( Lines[ Index ] );
                    FileName.Normalize( wxPATH_NORM_ALL, path );
                    m_PlayList.Add( new guStationPlayListItem( FileName.GetFullPath(), ItemName ) );
                }

                //guLogMessage( wxT( "%s - %s -> %s" ), path.c_str(), Lines[ Index ].c_str(), ( FileName.GetPathWithSep() + FileName.GetFullName() ).c_str() );
                ItemName = wxEmptyString;
            }
        }
        return true;
    }
    else
    {
        guLogError( wxT( "Empty playlist file stream" ) );
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guPlayListFile::ReadM3uFile( const wxString &filename )
{
    wxFileInputStream Ins( filename );
    if( Ins.IsOk() )
    {
        return ReadM3uStream( Ins, wxPathOnly( filename ) );
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
    wxString Title;
    wxString Location;
    while( XmlNode )
    {
        if( XmlNode->GetName().Lower() == wxT( "title" ) )
        {
            Title = XmlNode->GetNodeContent();
        }
        else if( XmlNode->GetName().Lower() == wxT( "location" ) )
        {
            Location = XmlNode->GetNodeContent();
        }
        XmlNode = XmlNode->GetNext();
    }

    if( !Location.IsEmpty() )
        m_PlayList.Add( new guStationPlayListItem( Location, Title ) );
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
bool guPlayListFile::ReadXspfStream( wxInputStream &playlist )
{
    wxXmlDocument XmlDoc( playlist );
    wxXmlNode * XmlNode = XmlDoc.GetRoot();
    if( XmlNode && XmlNode->GetName().Lower() == wxT( "playlist" ) )
    {
        ReadXspfPlayList( XmlNode->GetChildren() );
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guPlayListFile::ReadXspfFile( const wxString &filename )
{
    wxFileInputStream Ins( filename );
    if( Ins.IsOk() )
    {
        return ReadXspfStream( Ins );
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
    wxString Title;
    wxString Location;
    while( XmlNode )
    {
        if( XmlNode->GetName().Lower() == wxT( "title" ) )
        {
            Title = XmlNode->GetNodeContent();
        }
        else if( XmlNode->GetName().Lower() == wxT( "ref" ) )
        {
            XmlNode->GetPropVal( wxT( "href" ), &Location );
        }
        XmlNode = XmlNode->GetNext();
    }
    if( !Location.IsEmpty() )
        m_PlayList.Add( new guStationPlayListItem( Location, Title ) );
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
bool guPlayListFile::ReadAsxStream( wxInputStream &playlist )
{
    wxXmlDocument XmlDoc( playlist );
    wxXmlNode * XmlNode = XmlDoc.GetRoot();
    if( XmlNode && XmlNode->GetName().Lower() == wxT( "asx" ) )
    {
        ReadAsxPlayList( XmlNode->GetChildren() );
        return true;
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guPlayListFile::ReadAsxFile( const wxString &filename )
{
    wxFileInputStream Ins( filename );
    if( Ins.IsOk() )
    {
        return ReadAsxStream( Ins );
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
    wxFile PlsFile;
    if( PlsFile.Create( filename, true ) && PlsFile.IsOpened() )
    {
        PlsFile.Write( wxT( "[playlist]\n" ) );
        int Count = m_PlayList.Count();
        PlsFile.Write( wxString::Format( wxT( "numberofentries=%u\n" ), Count ) );
        for( int Index = 0; Index < Count; Index++ )
        {
            PlsFile.Write( wxString::Format( wxT( "File%u=%s\n" ), Index + 1, m_PlayList[ Index ].m_Location.c_str() ) );
            PlsFile.Write( wxString::Format( wxT( "Title%u=%s\n" ), Index + 1, m_PlayList[ Index ].m_Name.c_str() ) );
        }
        PlsFile.Close();
    }
    else
    {
        guLogError( wxT( "Could not open the m3ufile '%s'" ), filename.c_str() );
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
        int Count = m_PlayList.Count();
        for( int Index = 0; Index < Count; Index++ )
        {
            M3uFile.Write( m_PlayList[ Index ].m_Location );
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

    int Count = m_PlayList.Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        wxXmlNode * TrackNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "track" ) );

        wxXmlNode * LocationNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "location" ) );
        wxXmlNode * LocationNodeVal = new wxXmlNode( wxXML_TEXT_NODE, wxT( "location" ), m_PlayList[ Index ].m_Location );
        LocationNode->AddChild( LocationNodeVal );

        wxXmlNode * TitleNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "title" ) );
        wxXmlNode * TitleNodeVal = new wxXmlNode( wxXML_TEXT_NODE, wxT( "title" ), m_PlayList[ Index ].m_Name );
        TitleNode->AddChild( TitleNodeVal );

        TrackNode->AddChild( LocationNode );
        TrackNode->AddChild( TitleNode );

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

    int Count = m_PlayList.Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        wxXmlNode * EntryNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "entry" ) );

        wxXmlNode * RefNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "ref" ) );
        wxXmlProperty * RefNodeVal = new wxXmlProperty( wxT( "href" ), m_PlayList[ Index ].m_Location, NULL );
        RefNode->SetProperties( RefNodeVal );

        wxXmlNode * TitleNode = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "title" ) );
        wxXmlNode * TitleNodeVal = new wxXmlNode( wxXML_TEXT_NODE, wxT( "title" ), m_PlayList[ Index ].m_Name );
        TitleNode->AddChild( TitleNodeVal );

        EntryNode->AddChild( RefNode );
        EntryNode->AddChild( TitleNode );

        RootNode->AddChild( EntryNode );
    }
    OutXml.SetRoot( RootNode );
    return OutXml.Save( filename );
}

// -------------------------------------------------------------------------------- //
