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
#ifndef PLAYLISTFILE_H
#define PLAYLISTFILE_H

#include <wx/arrstr.h>
#include <wx/dynarray.h>
#include <wx/uri.h>
#include <wx/string.h>
#include <wx/xml/xml.h>

class guPlaylistItem
{
  public:
    wxString m_Name;
    wxString m_Location;
    int      m_Length;

    guPlaylistItem() { m_Length = 0; };

    guPlaylistItem( const wxString &location, const wxString &title, const int length = wxNOT_FOUND )
    {
        m_Name = title;
        m_Location = location;
        m_Length = length;
    }

    wxString GetLocation( const bool relative = false, const wxString &pathbase = wxEmptyString );
};
WX_DECLARE_OBJARRAY(guPlaylistItem, guPlaylistItemArray);

// -------------------------------------------------------------------------------- //
class guPlaylistFile
{
  private :
    void                ReadXspfPlayList( wxXmlNode * XmlNode );
    void                ReadXspfTrackList( wxXmlNode * XmlNode );
    void                ReadXspfTrack( wxXmlNode * XmlNode );

    void                ReadAsxEntry( wxXmlNode * XmlNode );
    void                ReadAsxPlayList( wxXmlNode * XmlNode );

  protected :
    wxString            m_Name;
    guPlaylistItemArray m_Playlist;

    bool                ReadPlsStream( wxInputStream &playlist, const wxString &path = wxEmptyString );
    bool                ReadM3uStream( wxInputStream &playlist, const wxString &path = wxEmptyString );
    bool                ReadXspfStream( wxInputStream &playlist );
    bool                ReadAsxStream( wxInputStream &playlist );

    bool                ReadPlsFile( const wxString &filename );
    bool                ReadM3uFile( const wxString &filename );
    bool                ReadXspfFile( const wxString &filename );
    bool                ReadAsxFile( const wxString &filename );

    bool                WritePlsFile( const wxString &filename, const bool relative = false );
    bool                WriteM3uFile( const wxString &filename, const bool relative = false );
    bool                WriteXspfFile( const wxString &filename, const bool relative = false );
    bool                WriteAsxFile( const wxString &filename, const bool relative = false );

  public :
    guPlaylistFile( void ) {};
    guPlaylistFile( const wxString &uri );
    ~guPlaylistFile();

    bool                    Load( const wxString &uri );
    bool                    Save( const wxString &filename, const bool relative = false );

    wxString                GetName( void ) { return m_Name; };
    void                    SetName( const wxString &name ) { m_Name = name; };
    guPlaylistItemArray     GetPlayList( void ) { return m_Playlist; };
    void                    SetPlayList( const guPlaylistItemArray &playlist ) { m_Playlist = playlist; };

    size_t                  Count( void ) const { return m_Playlist.Count(); };

    guPlaylistItem   GetItem( const size_t index )
    {
        return m_Playlist[ index ];
    }

    void                    AddItem( const wxString &location, const wxString &title = wxEmptyString, const int length = wxNOT_FOUND )
    {
        m_Playlist.Add( new guPlaylistItem( location, title, length ) );
    }

    void                    AddItem( const guPlaylistItem &item )
    {
        m_Playlist.Add( new guPlaylistItem( item ) );
    };

    static bool             IsValidPlayList( const wxString &uri );

};

// -------------------------------------------------------------------------------- //
class guCuePlaylistItem : public guPlaylistItem
{
  public :
    int         m_Start;
    //int       m_Length;
    //wxString  m_Name;
    //wxString  m_Location;

    wxString    m_TrackPath;
    wxString    m_ArtistName;
    wxString    m_AlbumArtist;
    wxString    m_AlbumName;
    wxString    m_Composer;
    wxString    m_Genre;
    wxString    m_Year;
    wxString    m_Comment;

    guCuePlaylistItem() : guPlaylistItem() { m_Start = 0; }

    guCuePlaylistItem( const wxString &location, const wxString &title, const int start = 0, const int length = wxNOT_FOUND ) :
        guPlaylistItem( location, title, length )
    {
        m_Start = start;
    }

};
WX_DECLARE_OBJARRAY(guCuePlaylistItem, guCuePlaylistItemArray);

// -------------------------------------------------------------------------------- //
class guCuePlaylistFile
{
  public :
    wxString                m_Location;
    guCuePlaylistItemArray  m_PlaylistItems;

    wxString                m_TrackPath;
    int                     m_TrackLength;
    wxString                m_ArtistName;
    wxString                m_AlbumArtist;
    wxString                m_AlbumName;
    wxString                m_Composer;
    wxString                m_Genre;
    wxString                m_Year;
    wxString                m_Comment;

    guCuePlaylistFile() {}

    guCuePlaylistFile( const wxString &uri );
    ~guCuePlaylistFile() {}

    void                    SetLocation( const wxString &location ) { m_Location = location; }

    bool                    Load( const wxString &location );
    bool                    LoadFromText( const wxString &playlist );
    //bool                  Save( const wxString &filename );

    size_t                  Count( void ) const { return m_PlaylistItems.Count(); };

    guCuePlaylistItem &     GetItem( const size_t index ) { return m_PlaylistItems[ index ]; }

    static bool             IsValidFile( const wxString &uri ) {
            wxURI Uri( uri );
            wxString CuePath = Uri.GetPath().Lower();
            return  CuePath.EndsWith( wxT( ".cue" ) );
    }

};

#endif
// -------------------------------------------------------------------------------- //
