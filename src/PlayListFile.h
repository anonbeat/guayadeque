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
#ifndef PLAYLISTFILE_H
#define PLAYLISTFILE_H

#include <wx/arrstr.h>
#include <wx/dynarray.h>
#include <wx/string.h>
#include <wx/xml/xml.h>

class guStationPlayListItem
{
  public:
    wxString m_Name;
    wxString m_Location;
    int      m_Length;

    guStationPlayListItem() {};

    guStationPlayListItem( const wxString &location, const wxString &title, const int length = wxNOT_FOUND )
    {
        m_Name = title;
        m_Location = location;
        m_Length = length;
    }

    wxString GetLocation( const bool relative = false, const wxString &pathbase = wxEmptyString );
};
WX_DECLARE_OBJARRAY(guStationPlayListItem, guStationPlayList);

// -------------------------------------------------------------------------------- //
class guPlayListFile
{
  private :
    void                ReadXspfPlayList( wxXmlNode * XmlNode );
    void                ReadXspfTrackList( wxXmlNode * XmlNode );
    void                ReadXspfTrack( wxXmlNode * XmlNode );

    void                ReadAsxEntry( wxXmlNode * XmlNode );
    void                ReadAsxPlayList( wxXmlNode * XmlNode );

  protected :
    wxString            m_Name;
    guStationPlayList   m_PlayList;

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
    guPlayListFile( void ) {};
    guPlayListFile( const wxString &uri );
    ~guPlayListFile();

    bool                    Load( const wxString &uri );
    bool                    Save( const wxString &filename, const bool relative = false );

    wxString                GetName( void ) { return m_Name; };
    void                    SetName( const wxString &name ) { m_Name = name; };
    guStationPlayList       GetPlayList( void ) { return m_PlayList; };
    void                    SetPlayList( const guStationPlayList &playlist ) { m_PlayList = playlist; };

    size_t                  Count( void ) const { return m_PlayList.Count(); };

    guStationPlayListItem   GetItem( const size_t index )
    {
        return m_PlayList[ index ];
    }

    void                    AddItem( const wxString &location, const wxString &title = wxEmptyString, const int length = wxNOT_FOUND )
    {
        m_PlayList.Add( new guStationPlayListItem( location, title, length ) );
    }

    void                    AddItem( const guStationPlayListItem &item )
    {
        m_PlayList.Add( new guStationPlayListItem( item ) );
    };

    static bool             IsValidPlayList( const wxString &uri );

};


#endif
// -------------------------------------------------------------------------------- //
