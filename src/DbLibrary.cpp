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
#include "DbLibrary.h"

#include "AlbumBrowser.h"
#include "EventCommandIds.h"
#include "Config.h"
#include "DynamicPlayList.h"
#include "LibUpdate.h"
#include "MainFrame.h"
#include "MediaViewer.h"
#include "MD5.h"
#include "PlayerPanel.h"
#include "TagInfo.h"
#include "Utils.h"
#include "TreePanel.h"

#include <wx/mstream.h>
#include <wx/wfstream.h>

namespace Guayadeque {

WX_DEFINE_OBJARRAY( guTrackArray );
WX_DEFINE_OBJARRAY( guListItems );
WX_DEFINE_OBJARRAY( guArrayListItems );
WX_DEFINE_OBJARRAY( guAlbumItems );
WX_DEFINE_OBJARRAY( guCoverInfos );
WX_DEFINE_OBJARRAY( guAS_SubmitInfoArray );

#define GU_CURRENT_DBVERSION    "21"

// -------------------------------------------------------------------------------- //
// Various functions
// -------------------------------------------------------------------------------- //
wxString GetSongsDBNamesSQL( const int order );
wxString GetSongsSortSQL( const int order, const bool orderdesc );

// -------------------------------------------------------------------------------- //
int guAlbumItemSearch( const guAlbumItems &items, int start, int end, int id )
{
    if( start >= end && items[ start ].m_Id != id )
    {
        return -1;
    }
    else
    {
        int MidPos = ( ( start + end ) / 2 );
        if( items[ MidPos ].m_Id == id )
            return MidPos;
        else if( id > items[ MidPos ].m_Id )
            return guAlbumItemSearch( items, MidPos + 1, end, id );
        else
            return guAlbumItemSearch( items, start, MidPos - 1, id );
    }
}

// -------------------------------------------------------------------------------- //
guAlbumItem * guAlbumItemGetItem( guAlbumItems &items, int id )
{
    int index;
    int count = items.Count();
    if( count )
    {
        index = guAlbumItemSearch( items, 0, count - 1, id );
        if( index != wxNOT_FOUND )
            return &items[ index ];
        guLogError( wxT( "Could not find in cache the albumid : %i" ), id );
    }
    return NULL;
}

// -------------------------------------------------------------------------------- //
wxString guAlbumItemsGetName( const guAlbumItems &items, int id )
{
    int index;
    int count = items.Count();
    if( count )
    {
        index = guAlbumItemSearch( items, 0, count - 1, id );
        if( index != wxNOT_FOUND )
            return items[ index ].m_Name;
        guLogError( wxT( "Could not find in cache the albumid : %i" ), id );
    }
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
int guAlbumItemsGetCoverId( const guAlbumItems &items, int id )
{
    int index;
    int count = items.Count();
    if( count )
    {
        index = guAlbumItemSearch( items, 0, count - 1, id );
        if( index != wxNOT_FOUND )
            return items[ index ].m_CoverId;
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
int guListItemSearch( const guListItems &items, int start, int end, int id )
{
    if( start >= end && items[ start ].m_Id != id )
    {
        return -1;
    }
    else
    {
        int MidPos = ( ( start + end ) / 2 );
        if( items[ MidPos ].m_Id == id )
            return MidPos;
        else if( id > items[ MidPos ].m_Id )
            return guListItemSearch( items, MidPos + 1, end, id );
        else
            return guListItemSearch( items, start, MidPos - 1, id );
    }
}

// -------------------------------------------------------------------------------- //
wxString guListItemsGetName( const guListItems &items, int id )
{
    int index;
    int count = items.Count();
    if( count )
    {
        index = guListItemSearch( items, 0, count - 1, id );
        if( index != wxNOT_FOUND )
            return items[ index ].m_Name;
    }
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
wxArrayInt GetArraySameItems( const wxArrayInt &Source, const wxArrayInt &Oper )
{
  wxArrayInt RetVal = Source;
  int index;
  int count = RetVal.Count();
  if( count )
  {
    for( index = count - 1; index >= 0; index-- )
    {
      if( Oper.Index( RetVal[ index ] ) == wxNOT_FOUND )
      {
        RetVal.RemoveAt( index );
      }
    }
  }
  return RetVal;
};

// -------------------------------------------------------------------------------- //
wxArrayInt GetArrayDiffItems( const wxArrayInt &Source, const wxArrayInt &Oper )
{
  wxArrayInt RetVal = Source;
  int index;
  int count = RetVal.Count();
  if( count )
  {
    for( index = count - 1; index >= 0; index-- )
    {
      if( Oper.Index( RetVal[ index ] ) != wxNOT_FOUND )
      {
        RetVal.RemoveAt( index );
      }
    }
  }
  return RetVal;
};

// -------------------------------------------------------------------------------- //
wxString TextFilterToSQL( const wxArrayString &TeFilters )
{
  int index;
  int count;
  wxString RetVal;
  if( ( count = TeFilters.Count() ) )
  {
    for( index = 0; index < count; index++ )
    {
        wxString Filter = escape_query_str( TeFilters[ index ] );
        RetVal += wxT( "( song_name LIKE '%" ) + Filter + wxT( "%' OR " );
        RetVal += wxT( " song_albumartist LIKE '%" ) +  Filter + wxT( "%' OR " );
        RetVal += wxT( " song_artist LIKE '%" ) +  Filter + wxT( "%' OR " );
        RetVal += wxT( " song_composer LIKE '%" ) +  Filter + wxT( "%' OR " );
        RetVal += wxT( " song_album LIKE '%" ) +  Filter + wxT( "%' ) " );
        RetVal += wxT( "AND " );
    }
    RetVal = RetVal.RemoveLast( 4 );
  }
  return RetVal;

}

// -------------------------------------------------------------------------------- //
wxString LabelFilterToSQL( const wxArrayInt &LaFilters )
{
  long count;
  long index;
  wxString subquery;
  wxString RetVal = wxEmptyString;
  if( ( count = LaFilters.Count() ) )
  {
    subquery = wxT( "(" );
    for( index = 0; index < count; index++ )
    {
        subquery += wxString::Format( wxT( "%u," ), LaFilters[ index ] );
    }
    subquery = subquery.RemoveLast( 1 ) + wxT( ")" );

    RetVal += wxT( "( (song_artistid IN ( SELECT settag_artistid FROM settags WHERE " \
                  "settag_tagid IN " ) + subquery + wxT( " and settag_artistid > 0 ) ) OR" );
    RetVal += wxT( " (song_albumid IN ( SELECT settag_albumid FROM settags WHERE " \
                  "settag_tagid IN " ) + subquery + wxT( " and settag_albumid > 0 ) ) OR" );
    RetVal += wxT( " (song_id IN ( SELECT settag_songid FROM settags WHERE " \
                  "settag_tagid IN " ) + subquery + wxT( " and settag_songid > 0 ) ) )" );
  }
  return RetVal;
}




// -------------------------------------------------------------------------------- //
// guTrack
// -------------------------------------------------------------------------------- //
bool guTrack::ReadFromFile( const wxString &filename )
{
  bool RetVal = false;

  guTagInfo * TagInfo;

  TagInfo = guGetTagInfoHandler( filename );
  if( TagInfo )
  {
      //guLogMessage( wxT( "FileName: '%s'" ), filename.c_str() );

      m_Path = wxPathOnly( filename );
      if( !m_Path.EndsWith( wxT( "/" ) ) )
        m_Path += '/';
      //guLogMessage( wxT( "PathName: %s" ), m_Path.c_str() );

      if( TagInfo->Read() )
      {
          m_Composer = TagInfo->m_Composer;

          m_ArtistName = TagInfo->m_ArtistName;

          m_AlbumArtist = TagInfo->m_AlbumArtist;

          m_AlbumName = TagInfo->m_AlbumName;

          m_GenreName = TagInfo->m_GenreName;

          m_SongName = TagInfo->m_TrackName;

          m_Number   = TagInfo->m_Track;
          m_Year     = TagInfo->m_Year;
          m_Length   = TagInfo->m_Length;
          m_Bitrate  = TagInfo->m_Bitrate;
          m_Comments = TagInfo->m_Comments;
          m_Disk     = TagInfo->m_Disk;
      }
      else
      {
          guLogError( wxT( "Cant read tags from '%s'" ), filename.c_str() );
      }

      m_PathId = wxNOT_FOUND;

      if( m_ArtistName.IsEmpty() )
        m_ArtistName = _( "Unknown" );
      m_ArtistId = wxNOT_FOUND;

      m_AlbumArtistId = wxNOT_FOUND;;

      if( m_AlbumName.IsEmpty() && !m_Path.IsEmpty() )
        m_AlbumName = m_Path.BeforeLast( wxT( '/' ) ).AfterLast( wxT( '/' ) );
      m_AlbumId = wxNOT_FOUND;;

      m_CoverId = 0;

      if( m_GenreName.IsEmpty() )
        m_GenreName = _( "Unknown" );
      m_GenreId = wxNOT_FOUND;;

      m_FileName = filename;
      m_FileSize = guGetFileSize( filename );

      if( m_SongName.IsEmpty() )
        m_SongName = m_FileName.AfterLast( wxT( '/' ) ).BeforeLast( wxT( '.' ) );

      m_SongId = wxNOT_FOUND;;

      m_Rating   = wxNOT_FOUND;

      RetVal = true;

      delete TagInfo;
  }

  return RetVal;
}




// -------------------------------------------------------------------------------- //
// guDbLibrary
// -------------------------------------------------------------------------------- //
guDbLibrary::guDbLibrary() : guDb()
{
  m_MediaViewer = NULL;
  m_NeedUpdate = false;
  m_LastAlbum = wxT( "__Gu4y4d3qu3__" );
  m_LastGenre = wxT( "__Gu4y4d3qu3__" );
  m_LastCoverId = wxNOT_FOUND;
  m_LastArtist = wxT( "__Gu4y4d3qu3__" );
  m_LastArtistId = wxNOT_FOUND;
  m_LastAlbumArtist = wxT( "__Gu4y4d3qu3__" );
  m_LastAlbumArtistId = wxNOT_FOUND;
  m_LastComposer = wxT( "__Gu4y4d3qu3__" );
  m_LastComposerId = wxNOT_FOUND;

  CheckDbVersion();

}

// -------------------------------------------------------------------------------- //
guDbLibrary::guDbLibrary( const wxString &dbname ) : guDb( dbname )
{
  m_MediaViewer = NULL;
  m_NeedUpdate = false;
  m_LastAlbum = wxT( "__Gu4y4d3qu3__" );
  m_LastGenre = wxT( "__Gu4y4d3qu3__" );
  m_LastCoverId = wxNOT_FOUND;
  m_LastArtist = wxT( "__Gu4y4d3qu3__" );
  m_LastArtistId = wxNOT_FOUND;
  m_LastAlbumArtist = wxT( "__Gu4y4d3qu3__" );
  m_LastAlbumArtistId = wxNOT_FOUND;
  m_LastComposer = wxT( "__Gu4y4d3qu3__" );
  m_LastComposerId = wxNOT_FOUND;

  CheckDbVersion();
}

// -------------------------------------------------------------------------------- //
guDbLibrary::guDbLibrary( guDb * db ) : guDb()
{
  m_MediaViewer = NULL;
  m_NeedUpdate = false;
  m_LastAlbum = wxT( "__Gu4y4d3qu3__" );
  m_LastGenre = wxT( "__Gu4y4d3qu3__" );
  m_LastCoverId = wxNOT_FOUND;
  m_LastArtist = wxT( "__Gu4y4d3qu3__" );
  m_LastArtistId = wxNOT_FOUND;
  m_LastAlbumArtist = wxT( "__Gu4y4d3qu3__" );
  m_LastAlbumArtistId = wxNOT_FOUND;
  m_LastComposer = wxT( "__Gu4y4d3qu3__" );
  m_LastComposerId = wxNOT_FOUND;

  m_Db = db->GetDb();
}

// -------------------------------------------------------------------------------- //
guDbLibrary::~guDbLibrary()
{
  Close();
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetMediaViewer( guMediaViewer * mediaviewer )
{
  m_MediaViewer = mediaviewer;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetDbVersion( void )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  unsigned long RetVal = 0;

  query = wxT( "SELECT name FROM sqlite_master WHERE type = 'table' AND name = 'Version';" );

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
    dbRes.Finalize();

    query = wxT( "SELECT version FROM Version;" );
    dbRes = ExecuteQuery( query );
    if( dbRes.NextRow() )
    {
        RetVal = dbRes.GetInt( 0 );
    }
  }
  else
  {
      query = wxT( "CREATE TABLE Version( version INTEGER );" );
      ExecuteUpdate( query );
  }
  dbRes.Finalize();

  return RetVal;
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::DoCleanUp( void )
{
  wxString query;
  wxArrayInt SongsToDel;
  wxArrayInt CoversToDel;
  wxSQLite3ResultSet dbRes;
  wxString FileName;

  guConfig * Config = ( guConfig * ) guConfig::Get();
  wxArrayString LibPaths = Config->ReadAStr( wxT( "LibPath" ), wxEmptyString, wxT( "libpaths" ) );

  CheckSymLinks( LibPaths );

  query = wxT( "SELECT DISTINCT song_id, song_filename, song_path FROM songs " );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    FileName = dbRes.GetString( 2 ) + wxT( "/" ) + dbRes.GetString( 1 );

    if( !wxFileExists( FileName ) || !CheckFileLibPath( LibPaths, FileName ) )
    {
      SongsToDel.Add( dbRes.GetInt( 0 ) );
    }
  }
  dbRes.Finalize();

  query = wxT( "SELECT DISTINCT cover_id, cover_path FROM covers;" );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    if( !wxFileExists( dbRes.GetString( 1 ) ) )
    {
      CoversToDel.Add( dbRes.GetInt( 0 ) );
    }
  }
  dbRes.Finalize();

  CleanItems( SongsToDel, CoversToDel );
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::CleanFiles( const wxArrayString &Files )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  wxArrayInt Tracks;
  wxArrayInt Covers;
  int Index;
  int Count = Files.Count();
  for( Index = 0; Index < Count; Index++ )
  {
    if( guIsValidAudioFile( Files[ Index ] ) )
    {
      int TrackId = FindTrackFile( Files[ Index ], NULL );
      if( TrackId )
        Tracks.Add( TrackId );
    }
    else if( guIsValidImageFile( Files[ Index ].Lower() ) )
    {
      query = wxString::Format( wxT( "SELECT cover_id FROM covers WHERE cover_path = '%s'" ), escape_query_str( Files[ Index ] ).c_str() );
      dbRes = ExecuteQuery( query );
      if( dbRes.NextRow() )
      {
        Covers.Add( dbRes.GetInt( 0 ) );
      }
      dbRes.Finalize();
    }
  }

  //
  CleanItems( Tracks, Covers );
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::CleanItems( const wxArrayInt &tracks, const wxArrayInt &covers )
{
  wxString query;

  if( tracks.Count() )
  {
      query = wxT( "DELETE FROM songs WHERE " ) + ArrayToFilter( tracks, wxT( "song_id" ) );
      ExecuteUpdate( query );
  }

  if( covers.Count() )
  {
      query = wxT( "DELETE FROM covers WHERE " ) + ArrayToFilter( covers, wxT( "cover_id" ) );
      ExecuteUpdate( query );
  }

  // Delete all posible orphan entries
//  query = wxT( "DELETE FROM genres WHERE genre_id NOT IN ( SELECT DISTINCT song_genreid FROM songs );" );
//  ExecuteUpdate( query );
//  query = wxT( "DELETE FROM artists WHERE artist_id NOT IN ( SELECT DISTINCT song_artistid FROM songs );" );
//  ExecuteUpdate( query );
//  query = wxT( "DELETE FROM albums WHERE album_id NOT IN ( SELECT DISTINCT song_albumid FROM songs );" );
//  ExecuteUpdate( query );
  query = wxT( "DELETE FROM covers WHERE cover_id NOT IN ( SELECT DISTINCT song_coverid FROM songs );" );
  ExecuteUpdate( query );
//  query = wxT( "DELETE FROM paths WHERE path_id NOT IN ( SELECT DISTINCT song_pathid FROM songs );" );
//  ExecuteUpdate( query );
  query = wxT( "DELETE FROM plsets WHERE plset_type = 0 AND plset_songid NOT IN ( SELECT DISTINCT song_id FROM songs );" );
  ExecuteUpdate( query );
  query = wxT( "DELETE FROM settags WHERE settag_songid NOT IN ( SELECT DISTINCT song_id FROM songs );" );
  ExecuteUpdate( query );

  //LoadCache();
}

// -------------------------------------------------------------------------------- //
bool guDbLibrary::CheckDbVersion( void )
{
  wxArrayString query;
  int Index;
  int Count;
  unsigned long dbVer;
  bool NeedVacuum = false;

  dbVer = GetDbVersion();
  guLogMessage( wxT( "Library Db Version %lu" ), dbVer );

  switch( dbVer )
  {
    case 4 :
    {
      Close();
      guLogMessage( wxT( "Update of database needed. Old database renamed to guayadeque.db.save" ) );
      wxRenameFile( m_DbName, m_DbName + wxT( ".save" ), true );
      m_NeedUpdate = true;
      Open( m_DbName );
      query.Add( wxT( "CREATE TABLE IF NOT EXISTS Version( version INTEGER );" ) );
    }

    case 0 :
    {
      // the database is going to be created... Reset the last update
      guConfig * Config = ( guConfig * ) guConfig::Get();
      if( Config )
      {
        Config->WriteStr( wxT( "LastUpdate" ), wxEmptyString, wxT( "general" ) );
      }

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS songs( song_id INTEGER PRIMARY KEY AUTOINCREMENT, " \
                      "song_name VARCHAR COLLATE NOCASE, song_genreid INTEGER, song_genre VARCHAR COLLATE NOCASE, " \
                      "song_artistid INTEGER, song_artist VARCHAR COLLATE NOCASE, song_albumartistid INTEGER, " \
                      "song_albumartist VARCHAR COLLATE NOCASE, song_composerid INTEGER, " \
                      "song_composer VARCHAR COLLATE NOCASE, song_albumid INTEGER, song_album VARCHAR COLLATE NOCASE, " \
                      "song_pathid INTEGER, song_path VARCHAR, song_filename VARCHAR, " \
                      "song_format VARCHAR(8) COLLATE NOCASE, song_disk VARCHAR COLLATE NOCASE, " \
                      "song_number INTEGER(3), song_year INTEGER(4), song_comment VARCHAR COLLATE NOCASE, " \
                      "song_coverid INTEGER, song_offset INTEGER, song_length INTEGER, song_bitrate INTEGER, " \
                      "song_rating INTEGER DEFAULT -1, song_playcount INTEGER DEFAULT 0, song_addedtime INTEGER, " \
                      "song_lastplay INTEGER, song_filesize INTEGER, song_albumsku VARCHAR, song_coverlink VARCHAR );" ) );

      query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_name on songs( song_name ASC )" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_albumid on songs( song_albumid,song_artist,song_year DESC, song_album, song_disk )" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_composerid on songs( song_composerid, song_composer )" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_artistid on songs( song_artistid, song_artist )" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_albumartistid on songs( song_albumartistid, song_albumartist )" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_genreid on songs( song_genreid, song_genre )" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_pathid on songs( song_pathid ASC )" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_length on songs( song_length ASC )" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_bitrate on songs( song_bitrate ASC )" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_rating on songs( song_rating ASC )" ) );
      //query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_rating_desc ON songs( song_rating DESC )" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_playcount on songs( song_playcount ASC )" ) );
      //query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_addedtime on songs( song_addedtime ASC )" ) );
      //query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_addedtime_desc ON songs( song_addedtime DESC )" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_lastplay on songs( song_lastplay ASC )" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_composerid on songs( song_composerid ASC )" ) );
      //query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_composerid_desc ON songs( song_composerid DESC )" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_number ON songs( song_number )" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_path ON songs( song_path ASC )" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_path_filename ON songs( song_path, song_filename )" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_format ON songs( song_format ASC )" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_genre ON songs( song_genre,song_composer,song_artist,song_album,song_disk,song_albumid,song_number )" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_genre_desc ON songs( song_genre DESC,song_composer,song_artist,song_album,song_disk,song_albumid,song_number )" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_composer ON songs( song_composer,song_artist,song_album,song_disk,song_albumid,song_number )" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_composer_desc ON songs( song_composer DESC,song_artist,song_album,song_disk,song_albumid,song_number )" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_artist ON songs( song_artist,song_album,song_disk,song_albumid,song_number )" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_artist_desc ON songs( song_artist DESC,song_album,song_disk,song_albumid,song_number )" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_albumartist ON songs( song_albumartist,song_album,song_disk,song_albumid,song_number )" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_albumartist_desc ON songs( song_albumartist DESC,song_album,song_disk,song_albumid,song_number )" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_album ON songs( song_album,song_disk,song_albumid,song_number )" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_album_desc ON songs( song_album DESC,song_disk,song_albumid,song_number )" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_disk ON songs( song_disk,song_albumid,song_number )" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_disk_desc ON songs( song_disk DESC,song_albumid,song_number )" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_year ON songs( song_year,song_album,song_disk,song_albumid,song_number )" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_year_desc ON songs( song_year DESC,song_album,song_disk,song_albumid,song_number )" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_addedtime ON songs( song_addedtime,song_album,song_disk,song_albumid,song_number )" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_addedtime_desc ON songs( song_addedtime DESC,song_album,song_disk,song_albumid,song_number )" ) );

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS tags( tag_id INTEGER PRIMARY KEY AUTOINCREMENT, tag_name VARCHAR COLLATE NOCASE );" ) );
      //query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'tag_id' on tags (tag_id ASC);" ) );
      query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'tag_name' on tags (tag_name ASC);" ) );

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS settags( settag_tagid INTEGER, settag_artistid INTEGER, settag_albumid INTEGER, settag_songid INTEGER );" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'settag_tagid' on settags (settag_tagid ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'settag_artistid' on settags (settag_artistid ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'settag_albumid' on settags (settag_albumid ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'settag_songid' on settags (settag_songid ASC);" ) );

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS playlists( playlist_id INTEGER PRIMARY KEY AUTOINCREMENT, playlist_name VARCHAR COLLATE NOCASE, " \
                      "playlist_type INTEGER(2), playlist_limited BOOLEAN, playlist_limitvalue INTEGER, playlist_limittype INTEGER(2), " \
                      "playlist_sorted BOOLEAN, playlist_sorttype INTEGER(2), playlist_sortdesc BOOLEAN, playlist_anyoption BOOLEAN, " \
                      "playlist_path VARCHAR );" ) );
      //query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'playlist_id' on playlists (playlist_id ASC);" ) );

      wxString querystr = wxT( "INSERT INTO playlists( playlist_id, playlist_name, playlist_type, " \
                      "playlist_limited, playlist_limitvalue, playlist_limittype, " \
                      "playlist_sorted, playlist_sorttype, playlist_sortdesc, playlist_anyoption ) " \
                      "VALUES( NULL, '" );
      querystr += _( "Recently added tracks" );
      querystr += wxT( "', 1, 0, 0, 0, 1, 12, 1, 0 );" );
      query.Add( querystr );
      querystr = wxT( "INSERT INTO playlists( playlist_id, playlist_name, playlist_type, " \
                      "playlist_limited, playlist_limitvalue, playlist_limittype, " \
                      "playlist_sorted, playlist_sorttype, playlist_sortdesc, playlist_anyoption ) " \
                      "VALUES( NULL, '" );

      querystr += _( "Last played tracks" );
      querystr += wxT( "', 1, 0, 0, 0, 1, 11, 1, 0 );" );
      query.Add( querystr );
      querystr = wxT( "INSERT INTO playlists( playlist_id, playlist_name, playlist_type, " \
                      "playlist_limited, playlist_limitvalue, playlist_limittype, " \
                      "playlist_sorted, playlist_sorttype, playlist_sortdesc, playlist_anyoption ) " \
                      "VALUES( NULL, '" );
      querystr += _( "Best rated tracks" );
      querystr += wxT( "', 1, 0, 0, 0, 1, 8, 1, 0 );" );
      query.Add( querystr );

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS plsets( plset_id INTEGER PRIMARY KEY AUTOINCREMENT, plset_plid INTEGER, plset_songid INTEGER, " \
                      "plset_type INTEGER(2), plset_option INTEGER(2), plset_text TEXT(255), plset_number INTEGER, plset_option2 INTEGER );" ) );
      //query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'plset_id' on plsets (plset_id ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'plset_plid' on plsets (plset_plid ASC);" ) );
      query.Add( wxT( "INSERT INTO plsets( plset_id, plset_plid, plset_songid, "
                      "plset_type, plset_option, plset_text, plset_number, plset_option2 ) " \
                      "VALUES( NULL, 1, 0, 14, 0, '', 1, 3 );" ) );
      query.Add( wxT( "INSERT INTO plsets( plset_id, plset_plid, plset_songid, " \
                      "plset_type, plset_option, plset_text, plset_number, plset_option2 ) " \
                      "VALUES( NULL, 2, 0, 13, 0, '', 1, 2 );" ) );
      query.Add( wxT( "INSERT INTO plsets( plset_id, plset_plid, plset_songid, " \
                      "plset_type, plset_option, plset_text, plset_number, plset_option2 ) " \
                      "VALUES( NULL, 3, 0, 10, 2, '', 4, 0 );" ) );

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS covers( cover_id INTEGER PRIMARY KEY AUTOINCREMENT, cover_path VARCHAR, cover_thumb BLOB, cover_midsize BLOB, cover_hash VARCHAR );" ) );
      //query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'cover_id' on covers (cover_id ASC);" ) );

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS audioscs( audiosc_id INTEGER PRIMARY KEY AUTOINCREMENT, audiosc_artist VARCHAR(255), audiosc_album varchar(255), audiosc_track varchar(255), audiosc_playedtime INTEGER, audiosc_source char(1), audiosc_ratting char(1), audiosc_len INTEGER, audiosc_tracknum INTEGER, audiosc_mbtrackid INTEGER );" ) );
      //query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'audiosc_id' on audioscs (audiosc_id ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'audiosc_playedtime' on audioscs (audiosc_playedtime ASC);" ) );

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS deleted( delete_id INTEGER PRIMARY KEY AUTOINCREMENT, " \
                      "delete_path VARCHAR, delete_date INTEGER );" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS delete_path on deleted(delete_path ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS delete_date on deleted(delete_date ASC);" ) );
    }

    case 5 :
    {
//      query.Add( wxT( "CREATE TABLE IF NOT EXISTS podcastchs( podcastch_id INTEGER PRIMARY KEY AUTOINCREMENT, "
//                      "podcastch_url VARCHAR, podcastch_title VARCHAR COLLATE NOCASE, podcastch_description VARCHAR, "
//                      "podcastch_language VARCHAR, podcastch_time INTEGER, podcastch_sumary VARCHAR, "
//                      "podcastch_author VARCHAR, podcastch_ownername VARCHAR, podcastch_owneremail VARCHAR, "
//                      "podcastch_category VARCHAR, podcastch_image VARCHAR, podcastch_downtype INTEGER, "
//                      "podcastch_downtext VARCHAR, podcastch_allowdel BOOLEAN );" ) );
//      //query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'podcastch_id' on podcastchs(podcastch_id ASC);" ) );
//      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'podcastch_title' on podcastchs(podcastch_title ASC);" ) );
//      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'podcastch_url' on podcastchs(podcastch_url ASC);" ) );
//
//      query.Add( wxT( "CREATE TABLE IF NOT EXISTS podcastitems( podcastitem_id INTEGER PRIMARY KEY AUTOINCREMENT, "
//                      "podcastitem_chid INTEGER, podcastitem_title VARCHAR COLLATE NOCASE, podcastitem_summary VARCHAR, "
//                      "podcastitem_author VARCHAR COLLATE NOCASE, podcastitem_enclosure VARCHAR, podcastitem_time INTEGER, "
//                      "podcastitem_file VARCHAR, podcastitem_filesize INTEGER, podcastitem_length INTEGER, "
//                      "podcastitem_addeddate INTEGER, podcastitem_playcount INTEGER, "
//                      "podcastitem_lastplay INTEGER, podcastitem_status INTEGER );" ) );
//      //query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'podcastitem_id' on podcastitems(podcastitem_id ASC);" ) );
//      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'podcastitem_title' on podcastitems(podcastitem_title ASC);" ) );
//      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'podcastitem_file' on podcastitems(podcastitem_file ASC);" ) );
//      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'podcastitem_chid' on podcastitems(podcastitem_chid ASC);" ) );
//      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'podcastitem_time' on podcastitems(podcastitem_time ASC);" ) );
//      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'podcastitem_enclosure' on podcastitems(podcastitem_enclosure ASC);" ) );
//      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'podcastitem_author' on podcastitems(podcastitem_author ASC);" ) );
//      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'podcastitem_length' on podcastitems(podcastitem_length ASC);" ) );
    }

    case 6 :
    {
      if( dbVer > 4 )
      {
          query.Add( wxT( "DROP INDEX 'radiostation_id';" ) );
          query.Add( wxT( "DROP INDEX 'radiostation_genreid';" ) );
          query.Add( wxT( "DROP TABLE 'radiostations';" ) );
          query.Add( wxT( "CREATE TABLE IF NOT EXISTS radiostations( radiostation_id INTEGER PRIMARY KEY AUTOINCREMENT, " \
                          "radiostation_scid INTEGER, radiostation_source INTEGER, radiostation_genreid INTEGER, " \
                          "radiostation_name VARCHAR, radiostation_link VARCHAR, radiostation_type VARCHAR, " \
                          "radiostation_br INTEGER, radiostation_lc INTEGER, radiostation_ct VARCHAR );" ) );
          //query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'radiostation_id' on radiostations (radiostation_id ASC);" ) );
          query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'radiostation_genreid' on radiostations (radiostation_source,radiostation_genreid ASC);" ) );
          query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'radiostation_lc' on radiostations (radiostation_lc ASC);" ) );
          query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'radiostation_type' on radiostations (radiostation_type ASC);" ) );
          query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'radiostation_ct' on radiostations (radiostation_ct ASC);" ) );
      }
    }

    case 7 :
    {
      query.Add( wxT( "DROP TABLE 'covers';" ) );
      query.Add( wxT( "CREATE TABLE IF NOT EXISTS covers( cover_id INTEGER PRIMARY KEY AUTOINCREMENT, cover_path VARCHAR(1024), cover_thumb BLOB, cover_midsize BLOB, cover_hash VARCHAR( 32 ) );" ) );
      //query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'cover_id' on covers (cover_id ASC);" ) );
    }

    case 8 :
    {
      query.Add( wxT( "DELETE FROM covers;" ) );
    }

    case 9 :
    {
      if( dbVer > 4 )
      {
          query.Add( wxT( "ALTER TABLE songs ADD COLUMN song_disk VARCHAR" ) );
          query.Add( wxT( "ALTER TABLE songs ADD COLUMN song_comment VARCHAR" ) );
          //query.Add( wxT( "ALTER TABLE songs ADD COLUMN song_composer VARCHAR(512)" ) );
      }
    }

    case 10 :
    {
      if( dbVer > 4 )
      {
          query.Add( wxT( "UPDATE playlists SET playlist_sorttype = playlist_sorttype + 1 WHERE playlist_sorttype > 4;" ) );
          query.Add( wxT( "UPDATE plsets SET plset_type = plset_type + 2 WHERE plset_type > 4;" ) );
      }
    }

    case 11 :
    {
//      query.Add( wxT( "CREATE TABLE IF NOT EXISTS composers( composer_id INTEGER  PRIMARY KEY AUTOINCREMENT, composer_name varchar(512) );" ) );
//      query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'composer_id' on composers (composer_id ASC);" ) );
//      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'composer_name' on composers (composer_name ASC);" ) );

      if( dbVer > 4 )
      {
        //query.Add( wxT( "ALTER TABLE songs DELETE COLUMN song_composer" ) );
        query.Add( wxT( "ALTER TABLE songs ADD COLUMN song_composerid INTEGER" ) );
      }
      m_NeedUpdate = true;
    }

    case 12 :
    {
      if( dbVer > 4 )
      {
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'song_composerid' on songs( song_composerid ASC );" ) );
      }
    }

    case 13 :
    {
      if( dbVer > 4 )
      {
        query.Add( wxT( "ALTER TABLE radiogenres ADD COLUMN radiogenre_source INTEGER" ) );
        query.Add( wxT( "ALTER TABLE radiogenres ADD COLUMN radiogenre_flags INTEGER" ) );
        query.Add( wxT( "UPDATE radiogenres SET radiogenre_source = 0;" ) );
        query.Add( wxT( "UPDATE radiogenres SET radiogenre_flags = 0;" ) );

        query.Add( wxT( "ALTER TABLE radiostations ADD COLUMN radiostation_source INTEGER" ) );
        query.Add( wxT( "UPDATE radiostations SET radiostation_source = radiostation_isuser;" ) );
      }
    }

    case 14 :
    {
      if( dbVer > 4 )
      {
        query.Add( wxT( "ALTER TABLE radiostations ADD COLUMN radiostation_ct VARCHAR" ) );

        query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'radiostation_lc' on radiostations (radiostation_lc ASC);" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'radiostation_type' on radiostations (radiostation_type ASC);" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'radiostation_ct' on radiostations (radiostation_ct ASC);" ) );
      }

    }

    case 15 :
    {
      if( dbVer > 4 )
      {
        //query.Add( wxT( "VACUUM;" ) );
        query.Add( wxT( "CREATE TABLE IF NOT EXISTS songs1( song_id INTEGER PRIMARY KEY AUTOINCREMENT, " \
                        "song_name VARCHAR COLLATE NOCASE, song_genreid INTEGER, song_genre VARCHAR COLLATE NOCASE, " \
                        "song_artistid INTEGER, song_artist VARCHAR COLLATE NOCASE, song_albumartistid INTEGER, " \
                        "song_albumartist VARCHAR COLLATE NOCASE, song_composerid INTEGER, song_composer VARCHAR COLLATE NOCASE, " \
                        "song_albumid INTEGER, song_album VARCHAR COLLATE NOCASE, song_pathid INTEGER, song_path VARCHAR, " \
                        "song_filename VARCHAR, song_format VARCHAR(8) COLLATE NOCASE, song_disk VARCHAR COLLATE NOCASE, " \
                        "song_number INTEGER(3), song_year INTEGER(4), song_comment VARCHAR COLLATE NOCASE, " \
                        "song_coverid INTEGER, song_offset INTEGER, song_length INTEGER, song_bitrate INTEGER, " \
                        "song_rating INTEGER DEFAULT -1, song_playcount INTEGER DEFAULT 0, song_addedtime INTEGER, " \
                        "song_lastplay INTEGER, song_filesize INTEGER );" ) );
        query.Add( wxT( "INSERT INTO songs1( song_id, song_name, song_albumid, song_artistid, song_genreid, song_filename, " \
                        "song_pathid, song_number, song_disk, song_year, song_comment, song_composerid, song_length, " \
                        "song_bitrate, song_rating, song_playcount, song_addedtime, song_lastplay, song_filesize ) " \
                        "SELECT song_id, song_name, song_albumid, song_artistid, song_genreid, song_filename, song_pathid, " \
                        "song_number, song_disk, song_year, song_comment, song_composerid, song_length, song_bitrate, "
                        "song_rating, song_playcount, song_addedtime, song_lastplay, song_filesize FROM songs" ) );
        query.Add( wxT( "UPDATE songs1 SET song_genre = ( SELECT genre_name FROM genres WHERE song_genreid = genre_id )" ) );
        query.Add( wxT( "UPDATE songs1 SET song_composer = ( SELECT composer_name FROM composers WHERE song_composerid = composer_id )" ) );
        query.Add( wxT( "UPDATE songs1 SET song_artist = ( SELECT artist_name FROM artists WHERE song_artistid = artist_id )" ) );
        query.Add( wxT( "UPDATE songs1 SET song_album = ( SELECT album_name FROM albums WHERE song_albumid = album_id )" ) );
        query.Add( wxT( "UPDATE songs1 SET song_path = ( SELECT path_value FROM paths WHERE song_pathid = path_id )" ) );
        query.Add( wxT( "UPDATE songs1 SET song_coverid = ( SELECT album_coverid FROM albums WHERE song_albumid = album_id )" ) );
        query.Add( wxT( "UPDATE songs1 SET song_offset = 0" ) );
        query.Add( wxT( "DROP TABLE songs" ) );
        query.Add( wxT( "ALTER TABLE songs1 RENAME TO songs" ) );

        query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_name on songs( song_name ASC )" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_albumid on songs( song_albumid,song_artist,song_year DESC, song_album, song_disk )" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_composerid on songs( song_composerid, song_composer )" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_artistid on songs( song_artistid, song_artist )" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_albumartistid on songs( song_albumartistid, song_albumartist )" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_genreid on songs( song_genreid, song_genre )" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_pathid on songs( song_pathid ASC )" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_length on songs( song_length ASC )" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_bitrate on songs( song_bitrate ASC )" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_rating on songs( song_rating ASC )" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_playcount on songs( song_playcount ASC )" ) );
        //query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_addedtime on songs( song_addedtime ASC )" ) );
        //query.Add( wxT( "CREATE INDEX song_addedtime_desc ON songs( song_addedtime DESC )" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_lastplay on songs( song_lastplay ASC )" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_composerid on songs( song_composerid ASC )" ) );
        //query.Add( wxT( "CREATE INDEX song_composerid_desc ON songs( song_composerid DESC )" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_number ON songs( song_number )" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_path ON songs( song_path ASC )" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_format ON songs( song_format ASC )" ) );
        //query.Add( wxT( "CREATE INDEX song_rating_desc ON songs( song_rating DESC )" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_genre ON songs( song_genre,song_composer,song_artist,song_album,song_disk,song_albumid,song_number )" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_genre_desc ON songs( song_genre DESC,song_composer,song_artist,song_album,song_disk,song_albumid,song_number )" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_composer ON songs( song_composer,song_artist,song_album,song_disk,song_albumid,song_number )" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_composer_desc ON songs( song_composer DESC,song_artist,song_album,song_disk,song_albumid,song_number )" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_artist ON songs( song_artist,song_album,song_disk,song_albumid,song_number )" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_artist_desc ON songs( song_artist DESC,song_album,song_disk,song_albumid,song_number )" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_albumartist ON songs( song_albumartist,song_album,song_disk,song_albumid,song_number )" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_albumartist_desc ON songs( song_albumartist DESC,song_album,song_disk,song_albumid,song_number )" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_album ON songs( song_album,song_disk,song_albumid,song_number )" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_album_desc ON songs( song_album DESC,song_disk,song_albumid,song_number )" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_disk ON songs( song_disk,song_albumid,song_number )" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_disk_desc ON songs( song_disk DESC,song_albumid,song_number )" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_year ON songs( song_year,song_album,song_disk,song_albumid,song_number )" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_year_desc ON songs( song_year DESC,song_album,song_disk,song_albumid,song_number )" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_addedtime ON songs( song_addedtime,song_album,song_disk,song_albumid,song_number )" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_addedtime_desc ON songs( song_addedtime DESC,song_album,song_disk,song_albumid,song_number )" ) );
        NeedVacuum = ( dbVer > 4 );
      }
    }

    case 16 :   // Added Album Artist to dynamic playlist
    {
      if( dbVer > 4 )
      {
          query.Add( wxT( "UPDATE playlists SET playlist_sorttype = playlist_sorttype + 1 WHERE playlist_sorttype > 1;" ) );
          query.Add( wxT( "UPDATE plsets SET plset_type = plset_type + 1 WHERE plset_type > 1;" ) );
          query.Add( wxT( "UPDATE plsets SET plset_option = plset_option + 1 WHERE " \
                          "( plset_option > 2 AND plset_type IN ( 9 ) ) OR" \
                          "( plset_option > 0 AND plset_type IN ( 9, 10, 11 ) );" ) );
      }
    }

    case 17 :   // Changed addedtime index to sort by album, disk, track also
    {
        if( dbVer > 4 )
        {
          query.Add( wxT( "DROP INDEX song_addedtime;" ) );
          query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_addedtime ON songs( song_addedtime,song_album,song_disk,song_albumid,song_number )" ) );
          query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_addedtime_desc ON songs( song_addedtime DESC,song_album,song_disk,song_albumid,song_number )" ) );

          query.Add( wxT( "CREATE TABLE IF NOT EXISTS deleted( delete_id INTEGER PRIMARY KEY AUTOINCREMENT, " \
                          "delete_path VARCHAR, delete_date INTEGER );" ) );
          query.Add( wxT( "CREATE INDEX IF NOT EXISTS delete_path on deleted(delete_path ASC);" ) );
          query.Add( wxT( "CREATE INDEX IF NOT EXISTS delete_date on deleted(delete_date ASC);" ) );
        }

    }

    case 18 :
    {
        if( dbVer > 4 )
        {
            query.Add( wxT( "DROP INDEX song_path;" ) );
            query.Add( wxT( "CREATE INDEX IF NOT EXISTS song_path_filename ON songs( song_path, song_filename )" ) );
            query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'tag_name' on tags (tag_name ASC);" ) );
        }
    }

    case 19 :
    {
      if( dbVer > 4 )
      {
        query.Add( wxT( "ALTER TABLE songs ADD COLUMN song_albumsku VARCHAR" ) );
        query.Add( wxT( "ALTER TABLE songs ADD COLUMN song_coverlink VARCHAR" ) );
      }
    }

    case 20 :
    {
      if( dbVer > 4 )
      {
        query.Add( wxT( "ALTER TABLE playlists ADD COLUMN playlist_path VARCHAR" ) );

        query.Add( wxT( "DROP INDEX IF EXISTS 'tag_id'" ) );
        query.Add( wxT( "DROP INDEX IF EXISTS 'playlist_id'" ) );
        query.Add( wxT( "DROP INDEX IF EXISTS 'plset_id'" ) );
        query.Add( wxT( "DROP INDEX IF EXISTS 'cover_id'" ) );
        query.Add( wxT( "DROP INDEX IF EXISTS 'audiosc_id'" ) );
        query.Add( wxT( "DROP INDEX IF EXISTS 'radiogenre_id'" ) );
        query.Add( wxT( "DROP INDEX IF EXISTS 'radiostation_id'" ) );
        query.Add( wxT( "DROP INDEX IF EXISTS 'radiolabel_id'" ) );
        query.Add( wxT( "DROP INDEX IF EXISTS 'radiosetlabel_id'" ) );
        query.Add( wxT( "DROP INDEX IF EXISTS 'podcastch_id'" ) );
        query.Add( wxT( "DROP INDEX IF EXISTS 'podcastitem_id'" ) );
      }

      guLogMessage( wxT( "Updating database version to " GU_CURRENT_DBVERSION ) );
      query.Add( wxT( "DELETE FROM Version;" ) );
      query.Add( wxT( "INSERT INTO Version( version ) VALUES( " GU_CURRENT_DBVERSION " );" ) );
    }

    default:
      break;
  }

  if( NeedVacuum )
  {
    query.Add( wxT( "VACUUM;" ) );
  }

  Count = query.Count();
  for( Index = 0; Index < Count; Index++ )
  {
    try
    {
        //guLogMessage( wxT( "%s" ), query[ Index ].c_str() );
        ExecuteUpdate( query[ Index ] );
    }
    catch( wxSQLite3Exception& e )
    {
      guLogError( wxT( "%u: %s" ),  e.GetErrorCode(), e.GetMessage().c_str() );
    }
    catch( ... )
    {
      guLogError( wxT( "Error executing query %s" ),  query[ Index ].c_str() );
    }
  }

  return true;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetGenreId( wxString &genrename )
{
  if( m_LastGenre == genrename )
  {
      return m_LastGenreId;
  }

  wxString query;
  wxSQLite3ResultSet dbRes;
  int RetVal = wxNOT_FOUND;

  m_LastGenre = genrename;

  query = wxString::Format( wxT( "SELECT song_genreid FROM songs " \
                                 "WHERE song_genre = '%s' LIMIT 1;" ),
                        escape_query_str( genrename ).c_str() );

  dbRes = ExecuteQuery( query );
  if( dbRes.NextRow() )
  {
    RetVal = m_LastGenreId = dbRes.GetInt( 0 );
  }
  else
  {
    query = wxT( "SELECT MAX(song_genreid) FROM songs;" );

    dbRes = ExecuteQuery( query );
    if( dbRes.NextRow() )
    {
      RetVal = m_LastGenreId = dbRes.GetInt( 0 ) + 1;
    }
    else
    {
        RetVal = m_LastGenreId = 1;
    }
  }
  dbRes.Finalize();

  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetComposerId( wxString &composername, bool create )
{
  if( m_LastComposer == composername )
  {
      return m_LastComposerId;
  }

  wxString query;
  wxSQLite3ResultSet dbRes;
  int RetVal = wxNOT_FOUND;
//  printf( "GetArtistId\n" );

  m_LastComposer = composername;

  query = wxString::Format( wxT( "SELECT song_composerid FROM songs " \
                                 "WHERE song_composer = '%s' LIMIT 1;" ),
            escape_query_str( composername ).c_str() );

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
    RetVal = m_LastComposerId = dbRes.GetInt( 0 );
  }
  else if( create )
  {
    query = wxT( "SELECT MAX(song_composerid) FROM songs;" );

    dbRes = ExecuteQuery( query );
    if( dbRes.NextRow() )
    {
      RetVal = m_LastComposerId = dbRes.GetInt( 0 ) + 1;
    }
    else
    {
        RetVal = m_LastComposerId = 1;
    }
  }
  dbRes.Finalize();

  return RetVal;
}

// -------------------------------------------------------------------------------- //
wxBitmap * guDbLibrary::GetCoverBitmap( const int coverid, const bool thumb )
{
  if( coverid <= 0 )
    return NULL;

  wxImage *             Img = NULL;
  wxBitmap *            RetVal = NULL;
  wxString              query;
  wxSQLite3ResultSet    dbRes;
  const unsigned char * Data;
  int                   DataLen = 0;

  query = wxT( "SELECT " );
  query += thumb ? wxT( "cover_thumb" ) : wxT( "cover_midsize" );
  query += wxString::Format( wxT( " FROM covers WHERE cover_id = %u LIMIT 1;" ), coverid );

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
    Data = dbRes.GetBlob( 0, DataLen );

    if( DataLen )
    {
      //guLogMessage( wxT( "Read %i bytes for image %i" ), len, CoverId );
      unsigned char * ImgData = ( unsigned char * ) malloc( DataLen );
      memcpy( ImgData, Data, DataLen );

      int ImgSize = thumb ? GUCOVER_THUMB_SIZE : GUCOVER_IMAGE_SIZE;
      Img = new wxImage( ImgSize, ImgSize, ImgData );
      //TmpImg->SaveFile( wxString::Format( wxT( "/home/jrios/%u.jpg" ), CoverId ), wxBITMAP_TYPE_JPEG );
      RetVal = new wxBitmap( * Img );
      delete Img;
    }
  }

  dbRes.Finalize();

  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::AddCoverFile( const wxString &coverfile, const wxString &coverhash )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  int CoverId = 0;
  wxString CoverFile;

  //guLogMessage( wxT( "AddCoverFile( '%s', '%s' )" ), coverfile.c_str(), coverhash.c_str() );

  // Create the Thumb image
  wxImage TmpImg;

  TmpImg.LoadFile( coverfile );
  if( TmpImg.IsOk() )
  {
    wxString CoverHash;
    if( coverhash.IsEmpty() )
    {
      guMD5 md5;
      CoverHash = md5.MD5File( coverfile );
    }
    else
      CoverHash = coverhash;

    //guLogWarning( _T( "Scaling image %i" ), n );
    TmpImg.Rescale( GUCOVER_THUMB_SIZE, GUCOVER_THUMB_SIZE, wxIMAGE_QUALITY_HIGH );
    if( TmpImg.IsOk() )
    {
//      wxFileOutputStream FOut( wxString::Format( wxT( "/home/jrios/%s.jpg" ), coverhash.c_str() ) );
//      TmpImg.SaveFile( FOut, wxBITMAP_TYPE_JPEG );
//      FOut.Close();
      wxImage MidImg;
      MidImg.LoadFile( coverfile );
      if( MidImg.IsOk() )
      {
        MidImg.Rescale( GUCOVER_IMAGE_SIZE, GUCOVER_IMAGE_SIZE, wxIMAGE_QUALITY_HIGH );


        wxSQLite3Statement stmt = m_Db->PrepareStatement( wxString::Format( wxT( "INSERT INTO covers( cover_id, cover_path, cover_thumb, cover_midsize, cover_hash ) "
                             "VALUES( NULL, '%s', ?, ?, '%s' );" ), escape_query_str( coverfile ).c_str(), CoverHash.c_str() ) );
        try {
          stmt.Bind( 1, TmpImg.GetData(), TmpImg.GetWidth() * TmpImg.GetHeight() * 3 );
          stmt.Bind( 2, MidImg.GetData(), MidImg.GetWidth() * MidImg.GetHeight() * 3 );

          //guLogMessage( wxT( "AddCoverFile: %s" ), stmt.GetSQL().c_str() );
          stmt.ExecuteQuery();
        }
        catch( wxSQLite3Exception& e )
        {
          guLogError( wxT( "%u: %s" ),  e.GetErrorCode(), e.GetMessage().c_str() );
        }
        catch( ... )
        {
          guLogError( wxT( "Error inserting the image %s" ),  coverfile.c_str() );
        }

        CoverId = GetLastRowId();
      }
      else
      {
          guLogError( wxT( "Error resizing to %i" ), GUCOVER_IMAGE_SIZE );
      }
    }
    else
    {
      guLogError( wxT( "Error resizing to %i" ), GUCOVER_THUMB_SIZE );
    }
  }
  else
  {
    guLogError( wxT( "Invalid cover file '%s'" ), coverfile.c_str() );
  }
  return CoverId;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::AddCoverImage( const wxImage &image )
{
  //guLogMessage( wxT( "guDbLibrary::AddCoverImage" ) );
  wxString query;
  wxSQLite3ResultSet dbRes;
  int CoverId = 0;
  wxString CoverFile;

  wxImage MidImg = image;
  MidImg.Rescale( GUCOVER_IMAGE_SIZE, GUCOVER_IMAGE_SIZE, wxIMAGE_QUALITY_HIGH );
  if( MidImg.IsOk() )
  {
    wxImage TinyImg = image;
    TinyImg.Rescale( GUCOVER_THUMB_SIZE, GUCOVER_THUMB_SIZE, wxIMAGE_QUALITY_HIGH );
    if( TinyImg.IsOk() )
    {
      wxSQLite3Statement stmt = m_Db->PrepareStatement( wxT( "INSERT INTO covers( cover_id, cover_path, cover_thumb, cover_midsize, cover_hash ) "
                             "VALUES( NULL, '', ?, ?, '' );" ) );
      try {
        stmt.Bind( 1, TinyImg.GetData(), TinyImg.GetWidth() * TinyImg.GetHeight() * 3 );
        stmt.Bind( 2, MidImg.GetData(), MidImg.GetWidth() * MidImg.GetHeight() * 3 );

        //guLogMessage( wxT( "AddCoverFile: %s" ), stmt.GetSQL().c_str() );
        stmt.ExecuteQuery();
      }
      catch( wxSQLite3Exception& e )
      {
        guLogError( wxT( "%u: %s" ),  e.GetErrorCode(), e.GetMessage().c_str() );
      }
      catch( ... )
      {
        guLogError( wxT( "Error inserting the image" ) );
      }

      CoverId = GetLastRowId();
    }
    else
    {
      guLogError( wxT( "Error resizing to %i" ), GUCOVER_THUMB_SIZE );
    }
  }
  else
  {
    guLogError( wxT( "Error resizing to %i" ), GUCOVER_IMAGE_SIZE );
  }
  return CoverId;
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::UpdateCoverFile( int coverid, const wxString &coverfile, const wxString &coverhash )
{
  wxString query;
  //int CoverId = 0;
  wxString CoverFile;

  // Create the Thumb image
  wxImage TmpImg;

  TmpImg.LoadFile( coverfile );
  if( TmpImg.IsOk() )
  {
    //guLogWarning( _T( "Scaling image %i" ), n );
    TmpImg.Rescale( GUCOVER_THUMB_SIZE, GUCOVER_THUMB_SIZE, wxIMAGE_QUALITY_HIGH );

    if( TmpImg.IsOk() )
    {
      wxImage MidImg;
      MidImg.LoadFile( coverfile );
      MidImg.Rescale( GUCOVER_IMAGE_SIZE, GUCOVER_IMAGE_SIZE, wxIMAGE_QUALITY_HIGH );
      if( MidImg.IsOk() )
      {
        //guLogWarning( wxT( "Cover Image w:%u h:%u " ), TmpImg.GetWidth(), TmpImg.GetHeight() );
        wxSQLite3Statement stmt = m_Db->PrepareStatement( wxString::Format(
           wxT( "UPDATE covers SET cover_thumb = ?, cover_midsize = ?, cover_hash = '%s' WHERE cover_id = %u;" ), coverhash.c_str(), coverid ) );

        try {
          stmt.Bind( 1, TmpImg.GetData(), TmpImg.GetWidth() * TmpImg.GetHeight() * 3 );
          stmt.Bind( 2, MidImg.GetData(), MidImg.GetWidth() * MidImg.GetHeight() * 3 );
          stmt.ExecuteQuery();
        }
        catch( wxSQLite3Exception& e )
        {
          guLogMessage( wxT( "%u: %s" ),  e.GetErrorCode(), e.GetMessage().c_str() );
        }
      }
    }
  }
  else
  {
    guLogError( wxT( "Invalid cover file '%s'" ), coverfile.c_str() );
  }
}

//// -------------------------------------------------------------------------------- //
//int guDbLibrary::FindCoverFile( const wxString &dirname )
//{
//    wxString query;
//    wxSQLite3ResultSet dbRes;
//    wxDir Dir;
//    wxString FileName;
//    wxString CurFile;
//    wxString DirName = dirname;
//    int CoverId = 0;
//
//    if( !DirName.EndsWith( wxT( "/" ) ) )
//        DirName += wxT( "/" );
//
//    Dir.Open( DirName );
//    //wxSetWorkingDirectory( DirName );
//
//    if( Dir.IsOpened() )
//    {
//        if( Dir.GetFirst( &FileName, wxEmptyString, wxDIR_FILES ) )
//        {
//            do {
//                CurFile = FileName.Lower();
//                //guLogMessage( wxT( "FindCoverFile: Found file '%s'" ), FileName.c_str() );
//                if( SearchCoverWords( CurFile, m_CoverSearchWords ) )
//                {
//                    //guLogMessage( wxT( "FindCoverFile: This file have been detected as a Cover" ) );
//                    if( CurFile.EndsWith( wxT( ".jpg" ) ) ||
//                        CurFile.EndsWith( wxT( ".jpeg" ) ) ||
//                        CurFile.EndsWith( wxT( ".png" ) ) ||
//                        CurFile.EndsWith( wxT( ".bmp" ) ) ||
//                        CurFile.EndsWith( wxT( ".gif" ) ) )
//                    {
//                        //guLogMessage( wxT( "FindCoverFile: This file looks like an image file" ) );
//                        CurFile = DirName + FileName;
//                        //guLogMessage( wxT( "Found Cover: %s" ), CurFile.c_str() );
//                        guMD5 md5;
//                        wxString CoverHash = md5.MD5File( CurFile );
//
//                        escape_query_str( &CurFile );
//
//                        query = wxString::Format( wxT( "SELECT cover_id, cover_path, cover_hash FROM covers ""WHERE cover_path = '%s' LIMIT 1;" ), CurFile.c_str() );
//
//                        dbRes = ExecuteQuery( query );
//
//                        if( dbRes.NextRow() ) // The cover is found in the database
//                        {
//                            CoverId = dbRes.GetInt( 0 );
//                            // Check if the file have been changed
//                            if( dbRes.GetString( 2 ) != CoverHash )
//                            {
//                                // The cover is different. Update the thumb is needed
//                                UpdateCoverFile( CoverId, DirName + FileName, CoverHash );
//                            }
//                        }
//                        else
//                        {
//                            CoverId = AddCoverFile( DirName + FileName, CoverHash );
//                        }
//                        break;
//                    }
//                }
//            } while( Dir.GetNext( &FileName ) );
//        }
//    }
//    //wxSetWorkingDirectory( SavedDir );
//    return CoverId;
//}

// -------------------------------------------------------------------------------- //
wxString guDbLibrary::GetCoverPath( const int CoverId )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  wxString RetVal = wxEmptyString;
  int count;
  int index;
  if( m_LastCoverId != CoverId )
  {
    count = m_LastItems.Count();
    if( count )
    {
      for( index = 0; index < count; index++ )
      {
        if( m_LastItems[ index ].m_Id == CoverId )
        {
          m_LastCoverId = CoverId;
          m_LastCoverPath = m_LastItems[ index ].m_Name;
          return m_LastCoverPath;
        }
      }
    }

    if( count > 25 ) // MAX_CACHE_ITEMS
    {
        m_LastItems.RemoveAt( 0 );
    }

    query = wxString::Format( wxT( "SELECT cover_path, cover_hash FROM covers " \
                                   "WHERE cover_id = %u " \
                                   "LIMIT 1;" ), CoverId );
    dbRes = ExecuteQuery( query );
    if( dbRes.NextRow() )
    {
      RetVal = dbRes.GetString( 0 );
      if( !RetVal.IsEmpty() )
      {
          // Check if the cover have been updated
          guMD5 md5;
          wxString CoverHash = md5.MD5File( RetVal );
          if( CoverHash != dbRes.GetString( 1 ) )
          {
              UpdateCoverFile( CoverId, RetVal, CoverHash );
          }
      }
    }
    m_LastCoverId = CoverId;
    m_LastCoverPath = RetVal;
    m_LastItems.Add( new guListItem( CoverId, m_LastCoverPath ) );
    dbRes.Finalize();
  }
  else
    RetVal = m_LastCoverPath;
  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::SetAlbumCover( const int AlbumId, const wxImage &image )
{
  int CoverId = 0;
  wxSQLite3ResultSet dbRes;
  wxString query;
  wxString FileName;
  int RetVal = 0;

  CoverId = GetAlbumCoverId( AlbumId );

  if( CoverId > 0 )
  {
    query = wxString::Format( wxT( "DELETE FROM covers WHERE cover_id = %i;" ), CoverId );
    ExecuteUpdate( query );
  }

  if( image.IsOk() )
  {
    CoverId = AddCoverImage( image );
    query = wxString::Format( wxT( "UPDATE songs SET song_coverid = %i WHERE song_albumid = %i;" ), CoverId, AlbumId );
    RetVal = ExecuteUpdate( query );
  }
  else
  {
    query = wxString::Format( wxT( "UPDATE songs SET song_coverid = 0 WHERE song_albumid = %i;" ), AlbumId );
    //guLogMessage( query );
    CoverId = 0;
  }

  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::SetAlbumCover( const int AlbumId, const wxString &CoverPath, const wxString &coverhash )
{
  int CoverId = 0;
  wxSQLite3ResultSet dbRes;
  wxString query;
  wxString FileName;
  int RetVal = 0;

  //guLogMessage( wxT( "Updating album: %i path: '%s'" ), AlbumId, CoverPath.c_str() );
  // Delete the actual assigned Cover
  // Find the Cover assigned to the album
  CoverId = GetAlbumCoverId( AlbumId );

  if( CoverId > 0 )
  {
    query = wxString::Format( wxT( "DELETE FROM covers WHERE cover_id = %i;" ), CoverId );
    ExecuteUpdate( query );
  }

  if( !CoverPath.IsEmpty() )
  {
    //guLogMessage( wxT( "Setting new cover '%s'" ), CoverPath.c_str() );
    CoverId = AddCoverFile( CoverPath, coverhash );

    query = wxString::Format( wxT( "UPDATE songs SET song_coverid = %i WHERE song_albumid = %i;" ), CoverId, AlbumId );
    RetVal = ExecuteUpdate( query );
  }
  else
  {
    query = wxString::Format( wxT( "UPDATE songs SET song_coverid = 0 WHERE song_albumid = %i;" ), AlbumId );
    ExecuteUpdate( query );
    CoverId = 0;
  }

//  // Update the AlbumsCache
//  guAlbumItem * AlbumItem = guAlbumItemGetItem( m_AlbumsCache, AlbumId );
//  if( AlbumItem )
//  {
//      AlbumItem->m_CoverId = CoverId;
//      AlbumItem->m_CoverPath = CoverPath;
//      //guLogMessage( wxT( "Updated Album with Cover %i '%s'" ), CoverId, CoverPath.c_str() );
//  }
  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetAlbumId( wxString &albumname, const int artistid, const int pathid,
    const wxString &pathname, const int coverid )
{
  //guLogMessage( wxT( "GetAlbumId : %s" ), m_LastAlbum.c_str() );
  if( m_LastAlbum == albumname )
  {
      //guLogMessage( wxT( "Album was found in cache" ) );
      return m_LastAlbumId;
  }

  wxString query;
  wxSQLite3ResultSet dbRes;
  int RetVal = wxNOT_FOUND;

  query = wxString::Format( wxT( "SELECT song_albumid, song_artistid " \
                                 "FROM songs " \
                                 "WHERE song_album = '%s' " \
                                 "AND song_pathid = %u LIMIT 1;" ),
                        escape_query_str( albumname ).c_str(), pathid );

  //guLogMessage( wxT( "%s" ), query.c_str() );

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
    RetVal = m_LastAlbumId = dbRes.GetInt( 0 );

//    // Now check if the artist id changed and if so update it
//    if( dbRes.GetInt( 1 ) != artistid )
//    {
//        query = wxString::Format( wxT( "UPDATE songs SET song_artistid = %u "
//                                       "WHERE album_id = %i;" ), artistid, RetVal );
//        ExecuteUpdate( query );
//    }
  }
  else
  {
    query = wxT( "SELECT MAX(song_albumid) FROM songs;" );

    dbRes = ExecuteQuery( query );

    if( dbRes.NextRow() )
    {
      RetVal = m_LastAlbumId = dbRes.GetInt( 0 ) + 1;
    }
    else
    {
        RetVal = m_LastAlbumId = 1;
    }
  }
  dbRes.Finalize();

  return RetVal;
}

// -------------------------------------------------------------------------------- //
wxArrayInt guDbLibrary::GetLabelIds( const wxArrayString &Labels )
{
  wxArrayInt RetVal;
  wxString LabelName;
  int      LabelId;
  int index;
  int count = Labels.Count();
  for( index = 0; index < count; index++ )
  {
    LabelName = Labels[ index ];
    if( GetLabelId( &LabelId, LabelName ) )
        RetVal.Add( LabelId );
  }
  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetLabelId( int * LabelId, wxString &LabelName )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  int RetVal = 0;
//  printf( "GetLabelId\n" );

  escape_query_str( &LabelName );
  query = wxString::Format( wxT( "SELECT tag_id FROM tags " \
                                 "WHERE tag_name = '%s' LIMIT 1;" ), LabelName.c_str() );

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
    * LabelId = dbRes.GetInt( 0 );
    RetVal = 1;
  }
  else
  {
    query = wxT( "INSERT INTO tags( tag_id, tag_name ) VALUES( NULL, '" ) +
            LabelName + wxT( "');" );

    if( ExecuteUpdate( query ) == 1 )
    {
      * LabelId = GetLastRowId();
      RetVal = 1;
    }
  }
  dbRes.Finalize();
  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::PathExists( const wxString &path )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  int RetVal = wxNOT_FOUND;

  wxString PathValue = path;
  if( !PathValue.EndsWith( wxT( "/" ) ) )
    PathValue += '/';

  escape_query_str( &PathValue );

  query = wxString::Format( wxT( "SELECT song_pathid FROM songs WHERE song_path = '%s' LIMIT 1;" ),
                    PathValue.c_str() );
  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
      RetVal = dbRes.GetInt( 0 );
  }
  dbRes.Finalize();
  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetPathId( wxString &PathValue )
{
  if( PathValue == m_LastPath )
  {
      return m_LastPathId;
  }

  int RetVal = 0;

//TODO Add a Lock
  wxString query;
  wxSQLite3ResultSet dbRes;

  m_LastPath = PathValue;

  if( !PathValue.EndsWith( wxT( "/" ) ) )
    PathValue += '/';

  query = wxString::Format( wxT( "SELECT song_pathid FROM songs " \
                                 "WHERE song_path = '%s' LIMIT 1" ),
                    escape_query_str( PathValue ).c_str() );
  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
    RetVal = m_LastPathId = dbRes.GetInt( 0 );
  }
  else
  {
    dbRes.Finalize();
    query = wxT( "SELECT MAX(song_pathid) FROM songs;" );
    dbRes = ExecuteQuery( query );
    if( dbRes.NextRow() )
    {
        RetVal = m_LastPathId = dbRes.GetInt( 0 ) + 1;
    }
    else
    {
        RetVal = m_LastPathId = 1;
    }
  }
  dbRes.Finalize();
  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetSongId( wxString &filename, const int pathid, const time_t filedate, const int start, bool * created )
{
  //wxSQLite3StatementBuffer query;
  wxString query;
  wxSQLite3ResultSet dbRes;
  int RetVal = wxNOT_FOUND;

//  printf( "GetSongId0\n" );

//  printf( "%u : %s\n", FileName.Length(), TextBuf );
  query = query.Format( wxT( "SELECT song_id FROM songs WHERE song_pathid = %u " \
                             "AND song_filename = '%s' AND song_offset = %i LIMIT 1;" ),
            pathid, escape_query_str( filename ).c_str(), start );
  //printf( query.ToAscii() );
  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
    RetVal = dbRes.GetInt( 0 );
  }
  else
  {
    query = query.Format( wxT( "INSERT INTO songs( song_id, song_pathid, song_rating, song_playcount, song_addedtime ) " \
                               "VALUES( NULL, %u, -1, 0, %lu )" ), pathid, filedate );
    //guLogMessage( wxT( "Query: '%s'" ), query.c_str() );
    if( ExecuteUpdate( query ) )
    {
      RetVal = GetLastRowId();
    }
    if( created )
    {
        * created = true;
    }
  }
  dbRes.Finalize();

  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetSongId( wxString &FileName, wxString &FilePath, const time_t filedate, const int start, bool * created )
{
  return GetSongId( FileName, GetPathId( FilePath ), filedate, start, created );
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::ReadFileTags( const wxString &filename, const bool allowrating )
{
  int Index;
  int Count;
  if( guCuePlaylistFile::IsValidFile( filename ) )   // If its a cue playlist
  {
    //guLogMessage( wxT( "***** CUE FILE ***** '%s'" ), filename.c_str() );

    //
    guCuePlaylistFile CuePlaylistFile( filename );
    if( ( Count = CuePlaylistFile.Count() ) )
    {
      //guLogMessage( wxT( "With %i tracks" ), Count );
      for( Index = 0; Index < Count; Index++ )
      {
        guCuePlaylistItem &CueItem = CuePlaylistFile.GetItem( Index );
        //guLogMessage( wxT( "Loading track %i '%s'" ), Index, CueItem.m_TrackPath.c_str() );

        if( wxFileExists( CueItem.m_TrackPath ) )
        {
          wxString query = wxT( "DELETE FROM songs WHERE song_filename = '" );
          query += escape_query_str( CueItem.m_TrackPath.AfterLast( wxT( '/' ) ) );
          query += wxT( "' AND song_path = '");
          query += escape_query_str( wxPathOnly( CueItem.m_TrackPath ) + wxT( "/" ) );
          query += wxT( "' AND song_offset = 0" );
          ExecuteUpdate( query );

          guTagInfo * TagInfo = guGetTagInfoHandler( CueItem.m_TrackPath );

          if( TagInfo )
          {
            if( TagInfo->Read() )
            {
              guTrack CurTrack;
              CurTrack.m_Path = wxPathOnly( CueItem.m_TrackPath ) + wxT( "/" );

              CurTrack.m_GenreName = CueItem.m_Genre;
              CurTrack.m_SongName = CueItem.m_Name;
              CurTrack.m_ArtistName = CueItem.m_ArtistName;
              CurTrack.m_AlbumArtist = CueItem.m_AlbumArtist;
              CurTrack.m_Composer = CueItem.m_Composer;
              CurTrack.m_Comments = CueItem.m_Comment;
              CurTrack.m_AlbumName = CueItem.m_AlbumName;
              CurTrack.m_Offset = CueItem.m_Start;
              CurTrack.m_Length = CueItem.m_Length;
              CurTrack.m_FileName = CueItem.m_TrackPath.AfterLast( wxT( '/' ) );
              CurTrack.m_Number = Index + 1;
              long Year;
              if( CueItem.m_Year.ToLong( &Year ) )
                CurTrack.m_Year = Year;
              CurTrack.m_Bitrate = TagInfo->m_Bitrate;

              //
              //
              //
              CurTrack.m_PathId = GetPathId( CurTrack.m_Path );

              CurTrack.m_ComposerId = GetComposerId( CurTrack.m_Composer );

              if( CurTrack.m_ArtistName.IsEmpty() )
                CurTrack.m_ArtistName = _( "Unknown" );
              CurTrack.m_ArtistId = GetArtistId( CurTrack.m_ArtistName );

              CurTrack.m_AlbumArtistId = GetAlbumArtistId( CurTrack.m_AlbumArtist );

              if( CurTrack.m_AlbumName.IsEmpty() && !CurTrack.m_Path.IsEmpty() )
                CurTrack.m_AlbumName = CurTrack.m_Path.BeforeLast( wxT( '/' ) ).AfterLast( wxT( '/' ) );
              CurTrack.m_AlbumId = GetAlbumId( CurTrack.m_AlbumName, CurTrack.m_ArtistId, CurTrack.m_PathId, CurTrack.m_Path );

              CurTrack.m_CoverId = GetAlbumCoverId( CurTrack.m_AlbumId );

              if( CurTrack.m_GenreName.IsEmpty() )
                CurTrack.m_GenreName = _( "Unknown" );
              CurTrack.m_GenreId = GetGenreId( CurTrack.m_GenreName );

              if( TagInfo->m_Length )
              {
                CurTrack.m_FileSize = ( guGetFileSize( CueItem.m_TrackPath ) * CueItem.m_Length ) / TagInfo->m_Length;
              }
              else
              {
                CurTrack.m_FileSize = 0;
              }

              if( CurTrack.m_SongName.IsEmpty() )
                CurTrack.m_SongName = CurTrack.m_FileName.AfterLast( wxT( '/' ) ).BeforeLast( wxT( '.' ) );

              bool IsNewTrack = false;
              CurTrack.m_SongId = GetSongId( CurTrack.m_FileName, CurTrack.m_PathId,
                                             GetFileLastChangeTime( filename ), CurTrack.m_Offset, &IsNewTrack );

              UpdateSong( CurTrack, IsNewTrack || allowrating );
            }
            else
            {
              guLogError( wxT( "Cant read tags from '%s'" ), filename.c_str() );
            }

            delete TagInfo;
          }
        }
      }
      return 1;
    }
  }
  else
  {
      guTagInfo * TagInfo = guGetTagInfoHandler( filename );
      if( TagInfo )
      {
          guTrack CurTrack;

          //guLogMessage( wxT( "FileName: '%s'" ), filename.c_str() );
          CurTrack.m_Path = wxPathOnly( filename );
          if( !CurTrack.m_Path.EndsWith( wxT( "/" ) ) )
            CurTrack.m_Path += '/';
          //guLogMessage( wxT( "PathName: %s" ), m_Path.c_str() );

          if( TagInfo->Read() )
          {
              CurTrack.m_Composer       = TagInfo->m_Composer;
              CurTrack.m_ArtistName     = TagInfo->m_ArtistName;
              CurTrack.m_AlbumArtist    = TagInfo->m_AlbumArtist;
              CurTrack.m_AlbumName      = TagInfo->m_AlbumName;
              CurTrack.m_GenreName      = TagInfo->m_GenreName;
              CurTrack.m_SongName       = TagInfo->m_TrackName;
              CurTrack.m_Number         = TagInfo->m_Track;
              CurTrack.m_Year           = TagInfo->m_Year;
              CurTrack.m_Length         = TagInfo->m_Length;
              CurTrack.m_Bitrate        = TagInfo->m_Bitrate;
              CurTrack.m_Comments       = TagInfo->m_Comments;
              CurTrack.m_Disk           = TagInfo->m_Disk;
              CurTrack.m_Rating         = TagInfo->m_Rating;
              CurTrack.m_PlayCount      = TagInfo->m_PlayCount;
          }
          else
          {
              guLogError( wxT( "Cant read tags from '%s'" ), filename.c_str() );
          }

          CurTrack.m_PathId = GetPathId( CurTrack.m_Path );

          CurTrack.m_ComposerId = GetComposerId( CurTrack.m_Composer );

          if( CurTrack.m_ArtistName.IsEmpty() )
            CurTrack.m_ArtistName = _( "Unknown" );
          CurTrack.m_ArtistId = GetArtistId( CurTrack.m_ArtistName );

          CurTrack.m_AlbumArtistId = GetAlbumArtistId( CurTrack.m_AlbumArtist );

          if( CurTrack.m_AlbumName.IsEmpty() && !CurTrack.m_Path.IsEmpty() )
            CurTrack.m_AlbumName = CurTrack.m_Path.BeforeLast( wxT( '/' ) ).AfterLast( wxT( '/' ) );
          CurTrack.m_AlbumId = GetAlbumId( CurTrack.m_AlbumName, CurTrack.m_ArtistId, CurTrack.m_PathId, CurTrack.m_Path );

          CurTrack.m_CoverId = GetAlbumCoverId( CurTrack.m_AlbumId );

          if( CurTrack.m_GenreName.IsEmpty() )
            CurTrack.m_GenreName = _( "Unknown" );
          CurTrack.m_GenreId = GetGenreId( CurTrack.m_GenreName );

          CurTrack.m_FileName = filename.AfterLast( wxT( '/' ) );
          CurTrack.m_FileSize = guGetFileSize( filename );

          if( CurTrack.m_SongName.IsEmpty() )
            CurTrack.m_SongName = CurTrack.m_FileName.AfterLast( wxT( '/' ) ).BeforeLast( wxT( '.' ) );

          bool IsNewTrack = false;
          CurTrack.m_SongId = GetSongId( CurTrack.m_FileName, CurTrack.m_PathId,
                                         GetFileLastChangeTime( filename ), CurTrack.m_Offset, &IsNewTrack );

          wxArrayInt ArrayIds;

          //
          if( TagInfo->m_TrackLabels.Count() )
          {
            ArrayIds.Add( CurTrack.m_SongId );
            wxArrayInt TrackLabelIds = GetLabelIds( TagInfo->m_TrackLabels );
            SetSongsLabels( ArrayIds, TrackLabelIds );
          }

          if( TagInfo->m_ArtistLabels.Count() )
          {
            ArrayIds.Empty();
            ArrayIds.Add( CurTrack.m_ArtistId );
            wxArrayInt ArtistLabelIds = GetLabelIds( TagInfo->m_ArtistLabels );
            SetArtistsLabels( ArrayIds, ArtistLabelIds );
          }

          if( TagInfo->m_AlbumLabels.Count() )
          {
            ArrayIds.Empty();
            ArrayIds.Add( CurTrack.m_AlbumId );
            wxArrayInt AlbumLabelIds = GetLabelIds( TagInfo->m_AlbumLabels );
            SetAlbumsLabels( ArrayIds, AlbumLabelIds );
          }

          UpdateSong( CurTrack, IsNewTrack || allowrating );

          delete TagInfo;

          return 1;
      }
  }

  return 0;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::AddFiles( const wxArrayString &files )
{
    int Index;
    int Count = files.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        ReadFileTags( files[ Index ] );
    }
    return Count;
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::UpdateTrackLength( const int trackid, const int length )
{
    wxString query = wxString::Format( wxT( "UPDATE songs SET song_length = %i " \
        "WHERE song_id = %i" ), length, trackid );
    ExecuteUpdate( query );
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::UpdateTrackBitRate( const int trackid, const int bitrate )
{
    wxString query = wxString::Format( wxT( "UPDATE songs SET song_bitrate = %i " \
        "WHERE song_id = %i" ), bitrate, trackid );
    ExecuteUpdate( query );
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::UpdateSongs( const guTrackArray * Songs, const wxArrayInt &changedflags )
{
  guTrack * Song;
  int index;
  int count = Songs->Count();
  wxArrayPtrVoid MediaViewerPtrs;

  // Process each Track
  for( index = 0; index < count; index++ )
  {
    if( !( changedflags[ index ] & guTRACK_CHANGED_DATA_TAGS ) )
        continue;

    Song = &( * Songs )[ index ];

//    if( wxFileExists( Song->m_FileName ) )
//    {
//        //MainFrame->SetStatusText( wxString::Format( _( "Updating track %s" ), Song->m_FileName.c_str() ) );
//
//        //guLogMessage( wxT( "Updating FileName '%s'" ), Song->FileName.c_str() );
//
//        //
//        // Update the File Tags
//        //
//        guTagInfo * TagInfo = guGetTagInfoHandler( Song->m_FileName );
//
//        if( !TagInfo )
//        {
//            guLogError( wxT( "There is no handler for the file '%s'" ), Song->m_FileName.c_str() );
//            continue;
//        }
//
//        TagInfo->m_TrackName = Song->m_SongName;
//        TagInfo->m_AlbumArtist = Song->m_AlbumArtist;
//        TagInfo->m_ArtistName = Song->m_ArtistName;
//        TagInfo->m_AlbumName = Song->m_AlbumName;
//        TagInfo->m_GenreName = Song->m_GenreName;
//        TagInfo->m_Track = Song->m_Number;
//        TagInfo->m_Year = Song->m_Year;
//        TagInfo->m_Composer = Song->m_Composer;
//        TagInfo->m_Comments = Song->m_Comments;
//        TagInfo->m_Disk = Song->m_Disk;
//
//        TagInfo->Write();
//
//        delete TagInfo;
//        //TagInfo = NULL;


        if( Song->m_Type == guTRACK_TYPE_DB )
        {
            if( Song->m_MediaViewer )
            {
                if( MediaViewerPtrs.Index( Song->m_MediaViewer ) == wxNOT_FOUND )
                    MediaViewerPtrs.Add( Song->m_MediaViewer );
            }

            //
            // Update the Library
            //
            wxString PathName;
            int      PathId;
            int      AlbumArtistId;
            int      ArtistId;
            int      ComposerId;
            int      AlbumId;
            //int      CoverId;
            int      GenreId;
            //int      SongId;
            //wxString FileName;

            PathName = wxPathOnly( Song->m_FileName ) + wxT( '/' );

            //wxSetWorkingDirectory( PathName );

            PathId = GetPathId( PathName );

            AlbumArtistId = GetAlbumArtistId( Song->m_AlbumArtist );

            ArtistId = GetArtistId( Song->m_ArtistName );

            ComposerId = GetComposerId( Song->m_Composer );

            AlbumId = GetAlbumId( Song->m_AlbumName, ArtistId, PathId, PathName, Song->m_CoverId );

            GenreId = GetGenreId( Song->m_GenreName );

            //FileName = Song->FileName.AfterLast( '/' );
            //printf( ( wxT( "FileName: " ) + CurSong.FileName ).ToAscii() ); printf( "\n" );

            //GetSongId( &SongId, FileName, PathId );

            guTrack CurTrack = * Song;
            CurTrack.m_GenreId = GenreId;
            CurTrack.m_ArtistId = ArtistId;
            CurTrack.m_AlbumArtistId = AlbumArtistId;
            CurTrack.m_ComposerId = ComposerId;
            CurTrack.m_AlbumId = AlbumId;
            CurTrack.m_PathId = PathId;
            CurTrack.m_FileName = Song->m_FileName.AfterLast( '/' );
            //CurSong.SongId = SongId;
            //CurSong.FileName.c_str(),
            //CurSong.Number
            //CurSong.Year,
            //CurSong.Length,
            UpdateSong( CurTrack, true );
        }
//    }
//    else
//    {
//        guLogMessage( wxT( "The file %s was not found for edition." ), Song->m_FileName.c_str() );
//    }
//    //wxSafeYield();
  }

  // We added in PanelPtr all panels we are updating tracks
  // And send the clean db event to all of them
  wxCommandEvent event( wxEVT_MENU, ID_LIBRARY_DOCLEANDB );
  count = MediaViewerPtrs.Count();
  for( index = 0; index < count; index++ )
  {
//    event.SetClientData( ( void * ) MediaViewerPtrs[ index ] );
    wxPostEvent( ( guMediaViewer * ) MediaViewerPtrs[ index ], event );
  }

}

// -------------------------------------------------------------------------------- //
int guDbLibrary::UpdateSong( const guTrack &track, const bool allowrating )
{
  wxString query;

  query = wxString::Format( wxT( "UPDATE songs SET song_name = '%s', " \
                                 "song_genreid = %u, song_genre = '%s', " \
                                 "song_artistid = %u, song_artist = '%s', " \
                                 "song_albumartistid = %u, song_albumartist = '%s', " \
                                 "song_albumid = %u, song_album = '%s', " \
                                 "song_pathid = %u, song_path = '%s', " \
                                 "song_filename = '%s', song_format = '%s', " \
                                 "song_number = %u, song_year = %u, " \
                                 "song_composerid = %u, song_composer = '%s', " \
                                 "song_comment = '%s', song_coverid = %i, song_disk = '%s', " \
                                 "song_length = %u, song_offset = %u, song_bitrate = %u, " \
                                 "song_filesize = %u" ),
            escape_query_str( track.m_SongName ).c_str(),
            track.m_GenreId,
            escape_query_str( track.m_GenreName ).c_str(),
            track.m_ArtistId,
            escape_query_str( track.m_ArtistName ).c_str(),
            track.m_AlbumArtistId,
            escape_query_str( track.m_AlbumArtist ).c_str(),
            track.m_AlbumId,
            escape_query_str( track.m_AlbumName ).c_str(),
            track.m_PathId,
            escape_query_str( track.m_Path ).c_str(),
            escape_query_str( track.m_FileName ).c_str(),
            escape_query_str( track.m_FileName.AfterLast( '.' ) ).c_str(),
            track.m_Number,
            track.m_Year,
            track.m_ComposerId, //escape_query_str( track.m_Composer ).c_str(),
            escape_query_str( track.m_Composer ).c_str(),
            escape_query_str( track.m_Comments ).c_str(),
            track.m_CoverId,
            escape_query_str( track.m_Disk ).c_str(),
            track.m_Length,
            track.m_Offset,
            track.m_Bitrate,
            track.m_FileSize );

  if( allowrating )
  {
      query += wxString::Format( wxT( ", song_rating = %i, song_playcount = %i " ),
            track.m_Rating,
            track.m_PlayCount );
  }

  query += wxString::Format( wxT( " WHERE song_id = %u;" ), track.m_SongId );

  return ExecuteUpdate( query );
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::UpdateImageFile( const char * filename, const char * saveto, const wxBitmapType type, const int maxsize )
{
  wxString              query;
  wxSQLite3ResultSet    dbRes;
  wxString              FileName;
  int                   AlbumId;
  int                   CoverId;
  wxString              CoverHash;
  wxString              CoverFile = FileName = wxString( filename, wxConvUTF8 );
  wxString              SaveTo = wxString( saveto, wxConvUTF8 );


  if( FileName.IsEmpty() )
    return;

  if( guIsValidAudioFile( FileName ) )
  {
    //guLogMessage( wxT( "Trying to get image from file '%s'" ), FileName.c_str() );
    wxImage * Image = guTagGetPicture( FileName );
    if( Image )
    {
        if( !Image->IsOk() )
        {
            delete Image;
            return;
        }

        if( maxsize )
        {
            Image->Rescale( maxsize, maxsize, wxIMAGE_QUALITY_HIGH );
        }

        CoverFile = wxPathOnly( FileName ) + wxT( '/' ) + SaveTo;        
        if( !Image->SaveFile( CoverFile, type ) )
        {
            delete Image;
            return;
        }
        FileName = CoverFile;
        delete Image;
    }
    else
    {
        return;
    }
  }


  guMD5 md5;
  wxString NewCoverHash = md5.MD5File( FileName );

  escape_query_str( &CoverFile );

  query = wxT( "SELECT cover_id, cover_hash FROM covers WHERE cover_path = '" ) + CoverFile +
          wxT( "' LIMIT 1;" );
  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
    CoverId = dbRes.GetInt( 0 );
    CoverHash = dbRes.GetString( 1 );
    dbRes.Finalize();

    if( NewCoverHash != CoverHash )
    {
        UpdateCoverFile( CoverId, FileName, NewCoverHash );
    }
    return;
  }
  dbRes.Finalize();

  // No Cover was found with this PathName
  wxString Path = wxPathOnly( FileName ) + wxT( '/' );

  escape_query_str( &Path );

  // Now try to search the album in this path
  query = wxString::Format( wxT( "SELECT song_albumid FROM songs WHERE song_path = '%s' LIMIT 1" ), Path.c_str() );

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
    AlbumId = dbRes.GetInt( 0 );
    dbRes.Finalize();

    SetAlbumCover( AlbumId, FileName, NewCoverHash );

    return;
  }
  //An image which appear to not be to any album was found. We do nothing with this
//  else
//  {
//      guLogError( wxT( "The image '%s' with no album set" ), FileName.c_str() );
//  }
  dbRes.Finalize();
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetFiltersCount() const
{
    return m_TeFilters.Count() || m_GeFilters.Count() ||
           m_LaFilters.Count() || m_ArFilters.Count() ||
           m_AAFilters.Count() || m_CoFilters.Count() ||
           m_AlFilters.Count() || m_YeFilters.Count() ||
           m_RaFilters.Count() || m_PcFilters.Count();
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetTeFilters( const wxArrayString &NewTeFilters, const bool locked )
{
    //guLogMessage( wxT( "guDbLibrary::SetTeFilters %i" ), NewTeFilters.Count() );
    m_TeFilters = NewTeFilters;
    if( locked )
        return;
    m_LaFilters.Empty();
    m_GeFilters.Empty();
    m_AAFilters.Empty();
    m_ArFilters.Empty();
    m_CoFilters.Empty();
    m_AlFilters.Empty();
    m_YeFilters.Empty();
    m_RaFilters.Empty();
    m_PcFilters.Empty();
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetLaFilters( const wxArrayInt &NewLaFilters, const bool locked )
{
    //guLogMessage( wxT( "guDbLibrary::SetTaFilters %i" ), NewTaFilters.Count() );
    if( NewLaFilters.Index( 0 ) != wxNOT_FOUND )
    {
        m_LaFilters.Empty();
    }
    else
    {
        m_LaFilters = NewLaFilters;
    }
    if( locked )
        return;
    m_GeFilters.Empty();
    m_AAFilters.Empty();
    m_ArFilters.Empty();
    m_CoFilters.Empty();
    m_AlFilters.Empty();
    m_YeFilters.Empty();
    m_RaFilters.Empty();
    m_PcFilters.Empty();
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetGeFilters( const wxArrayInt &NewGeFilters, const bool locked )
{
    //guLogMessage( wxT( "guDbLibrary::SetGeFilters %i" ), NewGeFilters.Count() );
    if( NewGeFilters.Index( 0 ) != wxNOT_FOUND )
    {
        m_GeFilters.Empty();
    }
    else
    {
        m_GeFilters = NewGeFilters;
    }
    if( locked )
        return;
    m_AAFilters.Empty();
    m_ArFilters.Empty();
    m_CoFilters.Empty();
    m_AlFilters.Empty();
    m_YeFilters.Empty();
    m_RaFilters.Empty();
    m_PcFilters.Empty();
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetArFilters( const wxArrayInt &NewArFilters, const bool locked )
{
    //guLogMessage( wxT( "guDbLibrary::SetArFilters %i" ), NewArFilters.Count() );
    if( NewArFilters.Index( 0 ) != wxNOT_FOUND )
    {
        m_ArFilters.Empty();
    }
    else
    {
        m_ArFilters = NewArFilters;
    }
    if( locked )
        return;
    m_YeFilters.Empty();
    m_AlFilters.Empty();
    m_RaFilters.Empty();
    m_PcFilters.Empty();
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetCoFilters( const wxArrayInt &filter, const bool locked )
{
    //guLogMessage( wxT( "guDbLibrary::SetCoFilters %i" ), filter.Count() );
    if( filter.Index( 0 ) != wxNOT_FOUND )
    {
        m_CoFilters.Empty();
    }
    else
    {
        m_CoFilters = filter;
    }
    if( locked )
        return;
    m_AAFilters.Empty();
    m_ArFilters.Empty();
    m_YeFilters.Empty();
    m_AlFilters.Empty();
    m_RaFilters.Empty();
    m_PcFilters.Empty();
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetAAFilters( const wxArrayInt &filter, const bool locked )
{
    //guLogMessage( wxT( "guDbLibrary::SetAAFilters %i" ), filter.Count() );
    if( filter.Index( 0 ) != wxNOT_FOUND )
    {
        m_AAFilters.Empty();
    }
    else
    {
        m_AAFilters = filter;
    }
    if( locked )
        return;
    m_ArFilters.Empty();
    m_YeFilters.Empty();
    m_AlFilters.Empty();
    m_RaFilters.Empty();
    m_PcFilters.Empty();
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetYeFilters( const wxArrayInt &filter, const bool locked )
{
    //guLogMessage( wxT( "guDbLibrary::SetYeFilters %i" ), filter.Count() );
    if( filter.Index( 0 ) != wxNOT_FOUND )
    {
        m_YeFilters.Empty();
    }
    else
    {
        m_YeFilters = filter;
    }
    if( locked )
        return;
    m_AlFilters.Empty();
    m_RaFilters.Empty();
    m_PcFilters.Empty();
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetAlFilters( const wxArrayInt &NewAlFilters, const bool locked )
{
    //guLogMessage( wxT( "guDbLibrary::SetAlFilters %i" ), NewAlFilters.Count() );
//    if( locked )
//        return;
    if( NewAlFilters.Index( 0 ) != wxNOT_FOUND )
    {
        m_AlFilters.Empty();
    }
    else
    {
        m_AlFilters = NewAlFilters;
    }
    if( locked )
        return;
    m_RaFilters.Empty();
    m_PcFilters.Empty();
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetRaFilters( const wxArrayInt &filter )
{
    //guLogMessage( wxT( "guDbLibrary::SetYeFilters %i" ), filter.Count() );
    if( filter.Index( 0 ) != wxNOT_FOUND )
    {
        m_RaFilters.Empty();
    }
    else
    {
        //m_RaFilters = filter;
        m_RaFilters.Empty();
        int Index;
        int Count = filter.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            m_RaFilters.Add( filter[ Index ] - 1 );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetPcFilters( const wxArrayInt &filter )
{
    //guLogMessage( wxT( "guDbLibrary::SetYeFilters %i" ), filter.Count() );
    if( filter.Index( 0 ) != wxNOT_FOUND )
    {
        m_PcFilters.Empty();
    }
    else
    {
        m_PcFilters.Empty();
        int Index;
        int Count = filter.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            m_PcFilters.Add( filter[ Index ] - 1 );
        }
    }
}

// -------------------------------------------------------------------------------- //
wxString guDbLibrary::FiltersSQL( int Level )
{
  wxString subquery;
  wxString query;
  wxString RetVal = wxEmptyString;

  if( m_TeFilters.Count() )
  {
    RetVal += TextFilterToSQL( m_TeFilters );
  }

  if( Level > guLIBRARY_FILTER_LABELS )
  {
    if( m_LaFilters.Count() )
    {
      if( !RetVal.IsEmpty() )
        RetVal += wxT( " AND " );
      RetVal += LabelFilterToSQL( m_LaFilters );
    }

    if( Level > guLIBRARY_FILTER_GENRES )
    {
      if( m_GeFilters.Count() )
      {
        if( !RetVal.IsEmpty() )
          RetVal += wxT( " AND " );
        RetVal += ArrayToFilter( m_GeFilters, wxT( "song_genreid" ) );
      }

      if( Level > guLIBRARY_FILTER_COMPOSERS )
      {
        if( m_CoFilters.Count() )
        {
          if( !RetVal.IsEmpty() )
          {
            RetVal += wxT( " AND " );
          }
          RetVal += ArrayToFilter( m_CoFilters, wxT( "song_composerid" ) );
        }

        if( Level > guLIBRARY_FILTER_ALBUMARTISTS )
        {
          if( m_AAFilters.Count() )
          {
            if( !RetVal.IsEmpty() )
              RetVal += wxT( " AND " );
            RetVal += ArrayToFilter( m_AAFilters, wxT( "song_albumartistid" ) );
          }

            if( Level > guLIBRARY_FILTER_ARTISTS )
            {
              if( m_ArFilters.Count() )
              {
                if( !RetVal.IsEmpty() )
                  RetVal += wxT( " AND " );
                RetVal += ArrayToFilter( m_ArFilters, wxT( "song_artistid" ) );
              }
              if( Level > guLIBRARY_FILTER_YEARS )
              {
                if( m_YeFilters.Count() )
                {
                  if( !RetVal.IsEmpty() )
                  {
                    RetVal += wxT( " AND " );
                  }
                  RetVal += ArrayToFilter( m_YeFilters, wxT( "song_year" ) );
                }

                if( Level > guLIBRARY_FILTER_ALBUMS )
                {
                  if( m_AlFilters.Count() )
                  {
                    if( !RetVal.IsEmpty() )
                      RetVal += wxT( " AND " );
                    RetVal += ArrayToFilter( m_AlFilters, wxT( "song_albumid" ) );
                  }

                  if( Level >= guLIBRARY_FILTER_SONGS )
                  {
                    if( m_RaFilters.Count() )
                    {
                      if( !RetVal.IsEmpty() )
                      {
                        RetVal += wxT( " AND " );
                      }
                      RetVal += ArrayToFilter( m_RaFilters, wxT( "song_rating" ) );
                    }
                    if( m_PcFilters.Count() )
                    {
                      if( !RetVal.IsEmpty() )
                      {
                        RetVal += wxT( " AND " );
                      }
                      RetVal += ArrayToFilter( m_PcFilters, wxT( "song_playcount" ) );
                    }
                  }
                }
              }
            }
        }
      }
    }
  }
  //guLogMessage( wxT( "FilterSQL: '%s'" ), RetVal.c_str() );
  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::AddLabel( wxString LabelName )
{
    wxString query;
    escape_query_str( &LabelName );

    query = wxT( "INSERT INTO tags( tag_id, tag_name ) VALUES( NULL, '" ) +
            LabelName + wxT( "');" );

    if( ExecuteUpdate( query ) == 1 )
    {
      return GetLastRowId();
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::SetLabelName( const int labelid, const wxString &oldlabel, const wxString &newlabel )
{
  wxString query;

  if( m_MediaViewer->GetEmbeddMetadata() )
  {
      guTrackArray  Tracks;
      wxArrayInt    Labels;
      Labels.Add( labelid );

      GetLabelsSongs( Labels, &Tracks );
      UpdateSongsLabel( Tracks, oldlabel, newlabel );
  }


  wxString LabelName = newlabel;
  escape_query_str( &LabelName );

  query = query.Format( wxT( "UPDATE tags SET tag_name = '%s' WHERE tag_id = %u;" ), LabelName.c_str(), labelid );

  if( ExecuteUpdate( query ) == 1 )
  {
    return GetLastRowId();
  }
  return 0;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::DelLabel( const int LabelId )
{
  wxString      query;

  if( m_MediaViewer->GetEmbeddMetadata() )
  {
      guListItems   LaItems;
      guTrackArray  Tracks;
      wxArrayInt    Labels;
      Labels.Add( LabelId );

      GetLabels( &LaItems, true );

      GetLabelsSongs( Labels, &Tracks );
      UpdateSongsLabel( Tracks, guListItemsGetName( LaItems, LabelId ), wxEmptyString );
  }

  query = query.Format( wxT( "DELETE FROM tags WHERE tag_id = %u;" ), LabelId );
  if( ExecuteUpdate( query ) )
  {
    query = query.Format( wxT( "DELETE FROM settags WHERE settag_tagid = %u;" ), LabelId );
    if( ExecuteUpdate( query ) )
      return 1;
  }
  return 0;
}

// -------------------------------------------------------------------------------- //
wxArrayInt guDbLibrary::GetLabels( void )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  wxArrayInt RetVal;

  query = wxT( "SELECT tag_id FROM tags ORDER BY tag_id;" );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    RetVal.Add( dbRes.GetInt( 0 ) );
  }
  dbRes.Finalize();
  return RetVal;
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::GetLabels( guListItems * Labels, const bool FullList )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
//  guListItems RetVal;
  //guLogMessage( wxT( "guDbLibrary::GetLabels" ) );

  query = wxT( "SELECT tag_id, tag_name FROM tags ORDER BY " );
  query += FullList ? wxT( "tag_id;" ) : wxT( "tag_name;" );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    Labels->Add( new guListItem( dbRes.GetInt( 0 ), dbRes.GetString( 1 ) ) );
  }
  dbRes.Finalize();
//  return RetVal;
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::GetGenres( guListItems * Genres, const bool FullList )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  //guLogMessage( wxT( "guDbLibrary::GetGenres" ) );

  //if( !GetFiltersCount() )
  if( FullList )
  {
    query = wxT( "SELECT song_genreid, song_genre FROM songs GROUP BY song_genreid" );
  }
  else if( !( m_TeFilters.Count() || m_LaFilters.Count() ) )
  {
    query = wxT( "SELECT song_genreid, song_genre FROM songs GROUP BY song_genreid ORDER BY song_genre" );
  }
  else
  {
    query = wxT( "SELECT song_genreid, song_genre FROM songs " );
    query += wxT( "WHERE " ) + FiltersSQL( guLIBRARY_FILTER_GENRES );
    query += wxT( " GROUP BY song_genreid ORDER BY song_genre" );
  }

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    Genres->Add( new guListItem( dbRes.GetInt( 0 ), dbRes.GetString( 1 ) ) );
  }
  dbRes.Finalize();
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::GetArtists( guListItems * Artists, const bool FullList )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  //guLogMessage( wxT( "guDbLibrary::GetArtists" ) );
//  guListItems RetVal;

  if( FullList )
  {
    query = wxT( "SELECT song_artistid, song_artist FROM songs GROUP BY song_artistid;" );
  }
  else if( !( m_TeFilters.Count() || m_LaFilters.Count() || m_GeFilters.Count() || m_AAFilters.Count() || m_CoFilters.Count() ) )
  {
    query = wxT( "SELECT song_artistid, song_artist FROM songs GROUP BY song_artistid ORDER BY song_artist" );
  }
  else
  {
    query = wxT( "SELECT song_artistid, song_artist FROM songs " );
    query += wxT( "WHERE " ) + FiltersSQL( guLIBRARY_FILTER_ARTISTS );
    query += wxT( " GROUP BY song_artistid ORDER BY song_artist" );
  }

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    Artists->Add( new guListItem( dbRes.GetInt( 0 ), dbRes.GetString( 1 ) ) );
  }
  dbRes.Finalize();
//  return RetVal;
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::GetYears( guListItems * Years, const bool FullList )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  if( FullList || !( m_TeFilters.Count() || m_LaFilters.Count() || m_GeFilters.Count() ||
                     m_AAFilters.Count() || m_ArFilters.Count() || m_CoFilters.Count() ) )
  {
    query = wxT( "SELECT DISTINCT song_year FROM songs WHERE song_year > 0 ORDER BY song_year DESC;" );
  }
  else
  {
    query = wxT( "SELECT DISTINCT song_year FROM songs WHERE song_year > 0 AND " ) + 
                  FiltersSQL( guLIBRARY_FILTER_YEARS );
    query += wxT( " ORDER BY song_year DESC;" );
  }

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    int Year = dbRes.GetInt( 0 );
    Years->Add( new guListItem( Year, wxString::Format( wxT( "%i" ), Year ) ) );
  }
  dbRes.Finalize();
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::GetRatings( guListItems * Ratings, const bool FullList )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  if( FullList || !( m_TeFilters.Count() || m_LaFilters.Count() || m_GeFilters.Count() ||
                     m_AAFilters.Count() || m_ArFilters.Count() || m_CoFilters.Count() ||
                     m_YeFilters.Count() ) )
  {
    query = wxT( "SELECT DISTINCT song_rating FROM songs WHERE song_rating >= 0 ORDER BY song_rating DESC;" );
  }
  else
  {
    query = wxT( "SELECT DISTINCT song_rating FROM songs " ) \
            wxT( "WHERE song_rating >= 0 AND " ) + FiltersSQL( guLIBRARY_FILTER_RATINGS );
    query += wxT( " ORDER BY song_rating DESC;" );
  }

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    int Rating = dbRes.GetInt( 0 );
    // To avoid using the 0 as 0 is used for All
    Ratings->Add( new guListItem( Rating + 1, wxString::Format( wxT( "%i" ), Rating ) ) );
  }
  dbRes.Finalize();
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::GetPlayCounts( guListItems * PlayCounts, const bool FullList )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  if( FullList || !( m_TeFilters.Count() || m_LaFilters.Count() || m_GeFilters.Count() ||
                     m_AAFilters.Count() || m_ArFilters.Count() || m_CoFilters.Count() ||
                     m_YeFilters.Count() ) )
  {
    query = wxT( "SELECT DISTINCT song_playcount FROM songs ORDER BY song_playcount;" );
  }
  else
  {
    query = wxT( "SELECT DISTINCT song_playcount FROM songs " ) \
            wxT( "WHERE " ) + FiltersSQL( guLIBRARY_FILTER_PLAYCOUNTS );
    query += wxT( " ORDER BY song_playcount;" );
  }

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    int PlayCount = dbRes.GetInt( 0 );
    // To avoid using the 0 as 0 is used for All
    PlayCounts->Add( new guListItem( PlayCount + 1, wxString::Format( wxT( "%i" ), PlayCount ) ) );
  }
  dbRes.Finalize();
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::GetAlbumArtists( guListItems * Items, const bool FullList )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  if( FullList )
  {
    query = wxT( "SELECT song_albumartistid, song_albumartist FROM songs GROUP BY song_albumartistid;" );
  }
  else if( !( m_TeFilters.Count() || m_LaFilters.Count() || m_GeFilters.Count() || m_CoFilters.Count() ) )
  {
    query = wxT( "SELECT song_albumartistid, song_albumartist FROM songs WHERE song_albumartist > '' " \
                 "GROUP BY song_albumartistid ORDER BY song_albumartist" );
  }
  else
  {
    query = wxT( "SELECT song_albumartistid, song_albumartist FROM songs WHERE song_albumartist > '' AND " ) + 
                 FiltersSQL( guLIBRARY_FILTER_ALBUMARTISTS );
    query += wxT( " GROUP BY song_albumartistid ORDER BY song_albumartist" );
  }

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    Items->Add( new guListItem( dbRes.GetInt( 0 ), dbRes.GetString( 1 ) ) );
  }
  dbRes.Finalize();
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::GetComposers( guListItems * Items, const bool FullList )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  if( FullList )
  {
    query = wxT( "SELECT song_composerid, song_composer FROM songs GROUP BY song_composerid;" );
  }
  else if( !( m_TeFilters.Count() || m_LaFilters.Count() || m_GeFilters.Count() ) )
  {
    query = wxT( "SELECT song_composerid, song_composer FROM songs WHERE song_composer > '' " \
                 "GROUP BY song_composerid ORDER BY song_composer" );
  }
  else
  {
    query = wxT( "SELECT song_composerid, song_composer FROM songs WHERE song_composer > '' AND " ) + 
                 FiltersSQL( guLIBRARY_FILTER_COMPOSERS );
    query += wxT( " GROUP BY song_composerid ORDER BY song_composer" );
  }

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    Items->Add( new guListItem( dbRes.GetInt( 0 ), dbRes.GetString( 1 ) ) );
  }
  dbRes.Finalize();
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::GetPaths( guListItems * Paths, bool FullList )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  wxString CoverPath;
  //guLogMessage( wxT( "guDbLibrary::GetPaths" ) );

  query = wxT( "SELECT DISTINCT song_pathid, song_path FROM songs GROUP BY song_pathid" );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    Paths->Add( new guListItem( dbRes.GetInt( 0 ), dbRes.GetString( 1 ) ) );
  }
  dbRes.Finalize();
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetAlbumsOrder( const int order )
{
    m_AlbumsOrder = order;
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::GetAlbums( guAlbumItems * Albums, bool FullList )
{
  wxString              query;
  wxSQLite3ResultSet    dbRes;

  //guLogMessage( wxT( "guDbLibrary::GetAlbums" )
  query = wxT( "SELECT song_albumid, song_album, song_artistid, song_artist, song_albumartist, song_coverid, MAX(song_year) FROM songs " );

  if( FullList )
  {
      query += wxT( "GROUP BY song_albumid " );
  }
  else
  {
    if( m_TeFilters.Count() || m_LaFilters.Count() || m_GeFilters.Count() ||
        m_AAFilters.Count() || m_ArFilters.Count() || m_CoFilters.Count() || m_YeFilters.Count() )
    {
      query += wxT( "WHERE " ) + FiltersSQL( guLIBRARY_FILTER_ALBUMS );
    }

    query += wxT( " GROUP BY song_albumid " );
    query += wxT( " ORDER BY " );

    switch( m_AlbumsOrder )
    {
        case guALBUMS_ORDER_NAME :
            query += wxT( "song_album, song_disk" );
            break;

        case guALBUMS_ORDER_YEAR :
            query += wxT( "song_year, song_album, song_disk" );
            break;

        case guALBUMS_ORDER_YEAR_REVERSE :
            query += wxT( "song_year DESC, song_album, song_disk" );
            break;

        case guALBUMS_ORDER_ARTIST_NAME :
            query += wxT( "coalesce(nullif(song_albumartist,''),song_artist) COLLATE NOCASE, song_album, song_disk " );
            break;

        case guALBUMS_ORDER_ARTIST_YEAR :
            query += wxT( "coalesce(nullif(song_albumartist,''),song_artist) COLLATE NOCASE, song_year, song_album, song_disk" );
            break;

        case guALBUMS_ORDER_ARTIST_YEAR_REVERSE :
        default :
            query += wxT( "coalesce(nullif(song_albumartist,''),song_artist) COLLATE NOCASE, song_year DESC, song_album, song_disk" );
            break;
    }
  }

  //guLogMessage( wxT( "GetAlbums: %s" ), query.c_str() );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
  //query = wxT( "SELECT song_albumid, song_album, song_artistid, song_artist, song_albumartist, song_coverid, MAX(song_year) FROM songs " );
        guAlbumItem * AlbumItem = new guAlbumItem();
        AlbumItem->m_Id = dbRes.GetInt( 0 );
        AlbumItem->m_Name = dbRes.GetString( 1 );
        AlbumItem->m_ArtistId = dbRes.GetInt( 2 );
        AlbumItem->m_ArtistName = dbRes.GetString( 4 );
        if( AlbumItem->m_ArtistName.IsEmpty() )
            AlbumItem->m_ArtistName = dbRes.GetString( 3 );
        AlbumItem->m_CoverId = dbRes.GetInt( 5 );
        AlbumItem->m_Year = dbRes.GetInt( 6 );
        AlbumItem->m_Thumb = GetCoverBitmap( AlbumItem->m_CoverId );
        Albums->Add( AlbumItem );
  }
  dbRes.Finalize();
//  return RetVal;
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::GetAlbums( guListItems * Albums, bool FullList )
{
  wxString              query;
  wxSQLite3ResultSet    dbRes;

  //guLogMessage( wxT( "guDbLibrary::GetAlbums" )
  query = wxT( "SELECT song_albumid, song_album FROM songs " );

  if( FullList )
  {
      query += wxT( "GROUP BY song_albumid " );
  }
  else
  {
    if( m_TeFilters.Count() || m_LaFilters.Count() || m_GeFilters.Count() ||
        m_AAFilters.Count() || m_ArFilters.Count() || m_CoFilters.Count() || m_YeFilters.Count() )
    {
      query += wxT( "WHERE " ) + FiltersSQL( guLIBRARY_FILTER_ALBUMS );
    }

    query += wxT( " GROUP BY song_albumid " );
  }

  //guLogMessage( wxT( "GetAlbums: %s" ), query.c_str() );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
        guListItem * AlbumItem = new guListItem( dbRes.GetInt( 0 ), dbRes.GetString( 1 ) );
        Albums->Add( AlbumItem );
  }
  dbRes.Finalize();
}

const wxString DynPlayListToSQLQuery( guDynPlayList * playlist );

// -------------------------------------------------------------------------------- //
bool guDbLibrary::GetAlbumDetails( const int albumid, int * year, int * trackcount )
{
  wxString              query;
  wxSQLite3ResultSet    dbRes;
  bool RetVal = false;
  query = wxString::Format( wxT( "SELECT MAX(song_year), COUNT(song_id) FROM songs WHERE song_albumid = %i;" ), albumid );

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
    * year = dbRes.GetInt( 0 );
    * trackcount = dbRes.GetInt( 1 );
    RetVal = true;
  }

  dbRes.Finalize();

  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetAlbumYear( const int albumid )
{
  wxString              query;
  wxSQLite3ResultSet    dbRes;
  int RetVal = 0;
  query = wxString::Format( wxT( "SELECT MAX(song_year) FROM songs WHERE song_albumid = %i;" ), albumid );

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
    RetVal = dbRes.GetInt( 0 );

  dbRes.Finalize();

  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetAlbumTrackCount( const int albumid )
{
  wxString              query;
  wxSQLite3ResultSet    dbRes;
  int RetVal = 0;
  query = wxString::Format( wxT( "SELECT COUNT( song_id ) FROM songs WHERE song_albumid = %i" ), albumid );

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
    RetVal = dbRes.GetInt( 0 );

  dbRes.Finalize();

  return RetVal;
}

// -------------------------------------------------------------------------------- //
wxString inline AlbumBrowserTextFilterToSQL( const wxArrayString &textfilters )
{
  int index;
  int count;
  wxString RetVal;
  if( ( count = textfilters.Count() ) )
  {
    for( index = 0; index < count; index++ )
    {
        wxString Filter = escape_query_str( textfilters[ index ] );
        RetVal += wxT( "( song_album LIKE '%" ) + Filter + wxT( "%' OR " );
        RetVal += wxT( " song_albumartist LIKE '%" ) + Filter + wxT( "%' OR " );
        RetVal += wxT( " song_artist LIKE '%" ) + Filter + wxT( "%' ) " );
        RetVal += wxT( "AND " );
    }
    RetVal = RetVal.RemoveLast( 4 );
  }
  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetAlbumsCount( guDynPlayList * filter, const wxArrayString &textfilters )
{
  wxString              query;
  wxString              postquery;
  wxSQLite3ResultSet    dbRes;
  int                   RetVal = 0;

  query = wxT( "SELECT COUNT( DISTINCT song_albumid ) FROM songs " );

  if( filter )
  {
    postquery += DynPlayListToSQLQuery( filter );
  }

  if( textfilters.Count() )
  {
    if( postquery.IsEmpty() )
      postquery = wxT( "WHERE " );
    else
      postquery += wxT( " AND " );
    //postquery += wxT( "song_albumid = album_id AND song_artistid = artist_id AND " );
    postquery += AlbumBrowserTextFilterToSQL( textfilters );
  }

  dbRes = ExecuteQuery( query + postquery );

  if( dbRes.NextRow() )
  {
      RetVal = dbRes.GetInt( 0 );
      guLogMessage( wxT( "Got %i albums" ), RetVal );
  }
  dbRes.Finalize();

  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetAlbums( guAlbumBrowserItemArray * items, guDynPlayList * filter,
        const wxArrayString &textfilters, const int start, const int count, const int order )
{
  if( !count )
    return 0;

  wxString              query;
  wxSQLite3ResultSet    dbRes;
  wxString              subquery;

  query = wxT( "SELECT song_albumid, song_album, song_artistid, song_albumartist, song_artist, song_coverid FROM songs " );
  if( filter )
  {
    subquery = DynPlayListToSQLQuery( filter );
  }

  if( textfilters.Count() )
  {
    subquery += subquery.IsEmpty() ? wxT( "WHERE " ) : wxT( " AND " );
    subquery += AlbumBrowserTextFilterToSQL( textfilters );
  }

  query += subquery + wxT( " GROUP BY song_albumid ORDER BY " );

  switch( order )
  {
    case guALBUMS_ORDER_NAME :
      query += wxT( "song_album" );
      break;

    case guALBUMS_ORDER_YEAR :
      query += wxT( "song_year" );
      break;

    case guALBUMS_ORDER_YEAR_REVERSE :
      query += wxT( "song_year DESC" );
      break;

    case guALBUMS_ORDER_ARTIST_NAME :
      query += wxT( "coalesce(nullif(song_albumartist, ''),song_artist) COLLATE NOCASE, song_album " );
      break;

    case guALBUMS_ORDER_ARTIST_YEAR :
      query += wxT( "coalesce(nullif(song_albumartist,''),song_artist) COLLATE NOCASE, song_year" );
      break;

    case guALBUMS_ORDER_ADDEDTIME :
      query += wxT( "song_addedtime DESC,song_album,song_disk " );
      break;

    case guALBUMS_ORDER_ARTIST_YEAR_REVERSE :
    default :
      query += wxT( "coalesce(nullif(song_albumartist,''),song_artist) COLLATE NOCASE, song_year DESC" );
      break;

  }

  query += wxString::Format( wxT( " LIMIT %i, %i" ), start, count );

  //guLogMessage( wxT( "GetAlbums:\n%s" ), query.c_str() );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
      guAlbumBrowserItem * Item = new guAlbumBrowserItem();
      Item->m_AlbumId = dbRes.GetInt( 0 ); //AlbumId;
      Item->m_AlbumName = dbRes.GetString( 1 );
      Item->m_ArtistId = dbRes.GetInt( 2 );
      Item->m_ArtistName = dbRes.GetString( 3 );
      if( Item->m_ArtistName.IsEmpty() )
        Item->m_ArtistName = dbRes.GetString( 4 );
      Item->m_CoverId = dbRes.GetInt( 5 );
      Item->m_CoverBitmap = GetCoverBitmap( Item->m_CoverId, false );
      Item->m_Year = 0; //dbRes.GetInt( 4 );
      Item->m_TrackCount = 0; //dbRes.GetInt( 5 );
      items->Add( Item );
  }
  dbRes.Finalize();

  return items->Count();
}

// -------------------------------------------------------------------------------- //
wxArrayString guDbLibrary::GetAlbumsPaths( const wxArrayInt &AlbumIds )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  wxArrayString RetVal;

  query = wxT( "SELECT DISTINCT song_path FROM songs WHERE song_albumid IN " ) + ArrayIntToStrList( AlbumIds );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
      RetVal.Add( dbRes.GetString( 0 ) );
  }
  dbRes.Finalize();
  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetStaticPlayList( const wxString &path )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  int PLId = wxNOT_FOUND;

  query = wxString::Format( wxT( "SELECT playlist_id FROM playlists WHERE playlist_path = '%s' AND playlist_type = 0 LIMIT 1" ),
          escape_query_str( path ).c_str() );

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
      PLId = dbRes.GetInt( 0 );
  }
  dbRes.Finalize();
  return PLId;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::CreateStaticPlayList( const wxString &path, const wxArrayInt &songs )
{
  int PlayListId = 0;
  wxString query;
//  wxSQLite3ResultSet dbRes;

  wxString Name;
  wxString Path = path;
  if( guPlaylistFile::IsValidPlayList( path ) )
  {
    Name = wxFileNameFromPath( path ).BeforeLast( wxT( '.' ) );
  }
  else
  {
    Name = path;
    Path = wxEmptyString;
  }

  query = wxString::Format( wxT( "INSERT INTO playlists( playlist_id, playlist_name, playlist_type, playlist_path ) VALUES( NULL, '%s', 0, '%s' );" ),
          escape_query_str( Name ).c_str(),
          escape_query_str( Path ).c_str() );

  if( ExecuteUpdate( query ) )
  {
      PlayListId = GetLastRowId();
  }
  //dbRes.Finalize();

  if( PlayListId )
  {
    int count = songs.Count();
    int index;
    for( index = 0; index < count; index++ )
    {
//      query.Add( wxT( "CREATE TABLE IF NOT EXISTS playlists( playlist_id INTEGER PRIMARY KEY AUTOINCREMENT, playlist_name varchar(100), "
//                      "playlist_type INTEGER(2));" ) );
//      query.Add( wxT( "CREATE TABLE IF NOT EXISTS plsets( plset_id INTEGER PRIMARY KEY AUTOINCREMENT, plset_plid INTEGER, plset_songid INTEGER, "
//                      "plset_type INTEGER(2), plset_option INTEGER(2), plset_text TEXT(255), plset_option2 INTEGER );" ) );
      query = wxString::Format( wxT( "INSERT INTO plsets( plset_id, plset_plid, plset_songid, plset_type, plset_option, plset_text, plset_option2 ) " \
                                     "VALUES( NULL, %u, %u, 0, 0, '', 0 );" ),
                    PlayListId,
                    songs[ index ] );
      ExecuteUpdate( query );
      //dbRes.Finalize();
    }
  }
  return PlayListId;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::UpdateStaticPlayList( const int plid, const wxArrayInt &tracks )
{
  wxString query;

  query = wxString::Format( wxT( "DELETE FROM plsets WHERE plset_plid = %u;" ), plid );
  ExecuteUpdate( query );

  return AppendStaticPlayList( plid, tracks );
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::AppendStaticPlayList( const int plid, const wxArrayInt &tracks )
{
  wxString query;

  int index;
  int count = tracks.Count();
  for( index = 0; index < count; index++ )
  {
    query = wxString::Format( wxT( "INSERT INTO plsets( plset_id, plset_plid, plset_songid, plset_type, plset_option, plset_text, plset_option2 ) "
                                   "VALUES( NULL, %u, %u, 0, 0, '', 0 );" ),
                  plid,
                  tracks[ index ] );
    ExecuteUpdate( query );
  }
  return tracks.Count();
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::DelPlaylistSetIds( const int plid, const wxArrayInt &setids )
{
    wxString query;
    query = wxString::Format( wxT( "DELETE FROM plsets WHERE plset_plid = %u AND " ), plid );
    query += ArrayToFilter( setids, wxT( "plset_id" ) );

    //guLogMessage( wxT( "DelPlayListSetIds:\n%s" ), query.c_str() );

    int RetVal = ExecuteUpdate( query );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetPlayListFiles( const int plid, wxFileDataObject * files )
{
  int Count = 0;
  wxString query;
  wxSQLite3ResultSet dbRes;
  query = wxT( "SELECT song_filename, song_path FROM plsets, songs " \
               "WHERE plset_songid = song_id AND plset_plid = " );
  query += wxString::Format( wxT( "%u" ), plid );

  //guLogMessage( wxT( "GetPlayListFiles:\n%s" ), query.c_str() );
  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
      files->AddFile( dbRes.GetString( 1 ) + dbRes.GetString( 0 ) );
      Count++;
  }

  dbRes.Finalize();
  return Count;
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::UpdateStaticPlayListFile( const int plid )
{
    wxString PlaylistPath = GetPlayListPath( plid );
    if( !PlaylistPath.IsEmpty() )
    {
        wxFileName FileName( PlaylistPath );
        if( FileName.Normalize() )
        {
            guTrackArray Tracks;
            GetPlayListSongs( plid, guPLAYLIST_TYPE_STATIC, &Tracks, NULL, NULL );
            guPlaylistFile PlayListFile;
            PlayListFile.SetName( FileName.GetFullPath() );
            int Index;
            int Count = Tracks.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                PlayListFile.AddItem( Tracks[ Index ].m_FileName,
                    Tracks[ Index ].m_ArtistName + wxT( " - " ) + Tracks[ Index ].m_SongName, Tracks[ Index ].m_Length / 1000 );
            }

            PlayListFile.Save( FileName.GetFullPath() );
        }
    }
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::CreateDynamicPlayList( const wxString &name, const guDynPlayList * playlist )
{
  int PlayListId = 0;
  wxString query;
  wxSQLite3ResultSet dbRes;

//  playlists( playlist_id INTEGER PRIMARY KEY AUTOINCREMENT, playlist_name varchar(100), "
//            "playlist_type INTEGER(2), playlist_limited BOOLEAN, playlist_limitvalue INTEGER, playlist_limittype INTEGER(2), "
//            "playlist_sorted BOOLEAN, playlist_sorttype INTEGER(2), playlist_sortdesc BOOLEAN, playlist_anyoption BOOLEAN);" ) );

  query = wxT( "INSERT INTO playlists( playlist_id, playlist_name, playlist_type, " \
               "playlist_limited, playlist_limitvalue, playlist_limittype, " \
               "playlist_sorted, playlist_sorttype, playlist_sortdesc, " \
               "playlist_anyoption ) " );
  query += wxString::Format( wxT( "VALUES( NULL, '%s', %u, %u, %u, %u, %u, %u, %u, %u );" ),
          escape_query_str( name ).c_str(),
          guPLAYLIST_TYPE_DYNAMIC,
          playlist->m_Limited, playlist->m_LimitValue, playlist->m_LimitType,
          playlist->m_Sorted, playlist->m_SortType, playlist->m_SortDesc,
          playlist->m_AnyOption );

  if( ExecuteUpdate( query ) == 1 )
  {
      PlayListId = GetLastRowId();
  }

  if( PlayListId )
  {
      int index;
      int count = playlist->m_Filters.Count();
      for( index = 0; index < count; index++ )
      {
        guFilterItem * FilterItem = &playlist->m_Filters[ index ];

        query = wxString::Format( wxT( "INSERT INTO plsets( plset_id, plset_plid, plset_songid, plset_type, plset_option, plset_text, plset_number, plset_option2 ) "
                                       "VALUES( NULL, %u, 0, %u, %u, '%s', %u, %u );" ),
                      PlayListId,
                      FilterItem->m_Type,
                      FilterItem->m_Option,
                      escape_query_str( FilterItem->m_Text ).c_str(),
                      FilterItem->m_Number,
                      FilterItem->m_Option2
                       );

        ExecuteUpdate( query );
      }
  }

  return PlayListId;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetPlayListsCount( void )
{
  int RetVal = 0;

  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxT( "SELECT COUNT( playlist_id ) FROM playlists" );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    RetVal = dbRes.GetInt( 0 );
  }
  dbRes.Finalize();

  return RetVal;
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::GetPlayLists( guListItems &playlists )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxT( "SELECT playlist_id, playlist_name FROM playlists ORDER BY playlist_name" );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    playlists.Add( new guListItem( dbRes.GetInt( 0 ), dbRes.GetString( 1 ) ) );
  }

  dbRes.Finalize();
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::GetPlayLists( wxArrayString &playlistnames )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxT( "SELECT playlist_name FROM playlists" );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    playlistnames.Add( dbRes.GetString( 0 ) );
  }

  dbRes.Finalize();
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::GetPlayLists( wxArrayInt &playlistids )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxT( "SELECT playlist_id FROM playlists" );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    playlistids.Add( dbRes.GetInt( 0 ) );
  }

  dbRes.Finalize();
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::GetPlayLists( guListItems * PlayLists, const int type, const wxArrayString * textfilters )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxString::Format( wxT( "SELECT playlist_id, playlist_name FROM playlists " \
                                 "WHERE playlist_type = %u " ), type );

  if( textfilters && textfilters->Count() )
  {
      int Index;
      int Count = textfilters->Count();
      for( Index = 0; Index < Count; Index++ )
      {
          wxString Filter = escape_query_str( ( * textfilters )[ Index ] );
          query += wxT( "AND playlist_name LIKE '%" ) + Filter + wxT( "%' " );
      }
  }

  query += wxT( "ORDER BY playlist_name COLLATE NOCASE;" ),

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    PlayLists->Add( new guListItem( dbRes.GetInt( 0 ), dbRes.GetString( 1 ) ) );
  }
  dbRes.Finalize();
//  return RetVal;
}

// -------------------------------------------------------------------------------- //
const wxString DynPLStringOption( int option, const wxString &text )
{
  wxString TextParam = text;
  escape_query_str( &TextParam );
  wxString FmtStr;
  switch( option )
  {
    case guDYNAMIC_FILTER_OPTION_STRING_CONTAINS : // contains
      FmtStr = wxT( "LIKE '%%%s%%'" );
      break;
    case guDYNAMIC_FILTER_OPTION_STRING_NOT_CONTAINS : // not contains
      FmtStr = wxT( "NOT LIKE '%%%s%%'" );
      break;
    case guDYNAMIC_FILTER_OPTION_STRING_IS : // IS
      FmtStr = wxT( "= '%s'" );
      break;
    case guDYNAMIC_FILTER_OPTION_STRING_ISNOT : // IS NOT
      FmtStr = wxT( "!= '%s'" );
      break;
    case guDYNAMIC_FILTER_OPTION_STRING_START_WITH : // START WITH
      FmtStr = wxT( "LIKE '%s%%'" );
      break;
    case guDYNAMIC_FILTER_OPTION_STRING_ENDS_WITH : // ENDS WITH
      FmtStr = wxT( "LIKE '%%%s'" );
      break;
  }
  return wxString::Format( FmtStr, TextParam.c_str() );
}

// -------------------------------------------------------------------------------- //
const wxString DynPLLabelOption( int option, const wxString &text )
{
  wxString TextParam = text;
  escape_query_str( &TextParam );
  wxString FmtStr;
  switch( option )
  {
    case guDYNAMIC_FILTER_OPTION_STRING_NOT_CONTAINS : // not contains
    case guDYNAMIC_FILTER_OPTION_STRING_CONTAINS : // contains
      FmtStr = wxT( "LIKE '%%%s%%'" );
      break;
    case guDYNAMIC_FILTER_OPTION_STRING_IS : // IS
    case guDYNAMIC_FILTER_OPTION_STRING_ISNOT : // IS NOT
      FmtStr = wxT( "= '%s'" );
      break;
    case guDYNAMIC_FILTER_OPTION_STRING_START_WITH : // START WITH
      FmtStr = wxT( "LIKE '%s%%'" );
      break;
    case guDYNAMIC_FILTER_OPTION_STRING_ENDS_WITH : // ENDS WITH
      FmtStr = wxT( "LIKE '%%%s'" );
      break;
  }
  return wxString::Format( FmtStr, TextParam.c_str() );
}

// -------------------------------------------------------------------------------- //
const wxString DynPLYearOption( const int option, const int year )
{
  wxString FmtStr;
  switch( option )
  {
    case guDYNAMIC_FILTER_OPTION_YEAR_IS :
      FmtStr = wxT( "= %u" );
      break;
    case guDYNAMIC_FILTER_OPTION_YEAR_ISNOT :
      FmtStr = wxT( "!= %u" );
      break;
    case guDYNAMIC_FILTER_OPTION_YEAR_AFTER :
      FmtStr = wxT( "> %u" );
      break;
    case guDYNAMIC_FILTER_OPTION_YEAR_BEFORE :
      FmtStr = wxT( "< %u" );
      break;
  }
  return wxString::Format( FmtStr, year );
}

// -------------------------------------------------------------------------------- //
const wxString DynPLNumericOption( const int option, const int value )
{
  wxString FmtStr;
  switch( option )
  {
    case guDYNAMIC_FILTER_OPTION_NUMERIC_IS :
      FmtStr = wxT( "= %u" );
      break;
    case guDYNAMIC_FILTER_OPTION_NUMERIC_ISNOT :
      FmtStr = wxT( "!= %u" );
      break;
    case guDYNAMIC_FILTER_OPTION_NUMERIC_AT_LEAST :
      FmtStr = wxT( ">= %u" );
      break;
    case guDYNAMIC_FILTER_OPTION_NUMERIC_AT_MOST :
      FmtStr = wxT( "<= %u" );
      break;
  }
  return wxString::Format( FmtStr, value );
}

// -------------------------------------------------------------------------------- //
unsigned long DynPLDateOption2[] = {
    60,         // Minutes
    3600,       // Hours
    86400,      // Days
    604800,     // Weeks
    2592000     // Months
};

// -------------------------------------------------------------------------------- //
const wxString DynPLDateOption( const int option, const int value, const int option2 )
{
  unsigned long Time;
  wxString FmtStr;

  switch( option )
  {
    case guDYNAMIC_FILTER_OPTION_DATE_IN_THE_LAST : // IN_THE_LAST
    {
        FmtStr = wxT( ">= %lu" );
        break;
    }

    case guDYNAMIC_FILTER_OPTION_DATE_BEFORE_THE_LAST : // BEFORE_THE_LAST
    {
        FmtStr = wxT( "< %lu" );
        break;
    }
  }
  Time = wxDateTime::GetTimeNow() - ( value * DynPLDateOption2[ option2 ] );
  return wxString::Format( FmtStr, Time );
}

// -------------------------------------------------------------------------------- //
const wxString DynPlayListToSQLQuery( guDynPlayList * playlist )
{
  if( !playlist->m_Filters.Count() )
    return wxEmptyString;

  wxString query = wxEmptyString;
  wxString dbNames = wxEmptyString;
  wxString dbSets = wxEmptyString;
  int index;
  int count = playlist->m_Filters.Count();
  for( index = 0; index < count; index++ )
  {
    if( index )
    {
        query += playlist->m_AnyOption ? wxT( " OR  " ) : wxT( " AND " );
    }
    switch( playlist->m_Filters[ index ].m_Type )
    {
      case guDYNAMIC_FILTER_TYPE_TITLE : // TITLE
        query += wxT( "song_name " ) + DynPLStringOption( playlist->m_Filters[ index ].m_Option,
                                    playlist->m_Filters[ index ].m_Text );
        break;

      case guDYNAMIC_FILTER_TYPE_ARTIST :  // ARTIST
        query += wxT( "song_artist " ) + DynPLStringOption( playlist->m_Filters[ index ].m_Option,
                                    playlist->m_Filters[ index ].m_Text );
        break;

      case guDYNAMIC_FILTER_TYPE_ALBUMARTIST :  // ALBUMARTIST
        query += wxT( "song_albumartist " ) + DynPLStringOption( playlist->m_Filters[ index ].m_Option,
                                    playlist->m_Filters[ index ].m_Text );
        break;

      case guDYNAMIC_FILTER_TYPE_ALBUM : // ALBUM
        query += wxT( "song_album " ) + DynPLStringOption( playlist->m_Filters[ index ].m_Option,
                                    playlist->m_Filters[ index ].m_Text );
        break;

      case guDYNAMIC_FILTER_TYPE_GENRE : // GENRE
        query += wxT( "song_genre " ) + DynPLStringOption( playlist->m_Filters[ index ].m_Option,
                                    playlist->m_Filters[ index ].m_Text );
        break;

      case guDYNAMIC_FILTER_TYPE_LABEL : // LABEL
        if( playlist->m_Filters[ index ].m_Option == guDYNAMIC_FILTER_OPTION_LABELS_NOTSET )
        {
            query  += wxT( "( song_id NOT IN ( SELECT DISTINCT settag_songid FROM settags WHERE settag_songid > 0 ) ) " );
        }
        else
        {
            wxString TagNameFilter = wxT( "SELECT tag_id FROM tags WHERE tag_name " ) +
                                        DynPLLabelOption( playlist->m_Filters[ index ].m_Option,
                                            playlist->m_Filters[ index ].m_Text );
            wxString Option;
            wxString Condition;
            switch( playlist->m_Filters[ index ].m_Option )
            {
                case guDYNAMIC_FILTER_OPTION_STRING_CONTAINS : // contains
                case guDYNAMIC_FILTER_OPTION_STRING_IS : // IS
                case guDYNAMIC_FILTER_OPTION_STRING_START_WITH : // START WITH
                case guDYNAMIC_FILTER_OPTION_STRING_ENDS_WITH : // ENDS WITH
                  Option = wxT( "IN " );
                  Condition = wxT( "OR " );
                  break;

                case guDYNAMIC_FILTER_OPTION_STRING_NOT_CONTAINS : // not contains
                case guDYNAMIC_FILTER_OPTION_STRING_ISNOT : // IS NOT
                  Option = wxT( "NOT IN " );
                  Condition = wxT( "AND " );
                  break;
            }
            query += wxT( "((song_artistid " ) + Option + wxT( "(SELECT settag_artistid FROM settags WHERE settag_tagid IN (" );
            query += TagNameFilter + wxT( ") and settag_artistid > 0 )) " ) + Condition;
            query += wxT( "(song_albumid " ) + Option + wxT( "(SELECT settag_albumid FROM settags WHERE settag_tagid IN (" );
            query += TagNameFilter + wxT( ") and settag_albumid > 0 )) " ) + Condition;
            query += wxT( "(song_id ") + Option + wxT( "(SELECT settag_songid FROM settags WHERE settag_tagid IN (" );
            query += TagNameFilter + wxT( ") and settag_songid > 0 )))" );
        }
        break;

      case guDYNAMIC_FILTER_TYPE_COMPOSER : // COMPOSER
        query += wxT( "song_composer " ) + DynPLStringOption( playlist->m_Filters[ index ].m_Option,
                                    playlist->m_Filters[ index ].m_Text );
        break;

      case guDYNAMIC_FILTER_TYPE_COMMENT : // COMMENT
        query += wxT( "song_comment " ) + DynPLStringOption( playlist->m_Filters[ index ].m_Option,
                                    playlist->m_Filters[ index ].m_Text );
        break;

      case guDYNAMIC_FILTER_TYPE_PATH : // PATH
        query += wxT( "song_path " ) + DynPLStringOption( playlist->m_Filters[ index ].m_Option,
                                    playlist->m_Filters[ index ].m_Text );
        break;

      case guDYNAMIC_FILTER_TYPE_YEAR :  // YEAR
      {
        query += wxT( "( song_year " ) +
                 DynPLYearOption( playlist->m_Filters[ index ].m_Option,
                                  playlist->m_Filters[ index ].m_Number ) + wxT( ")" );
        break;
      }

      case guDYNAMIC_FILTER_TYPE_RATING :  // RATING
      {
        query += wxT( "( song_rating " ) +
                 DynPLNumericOption( playlist->m_Filters[ index ].m_Option,
                                  playlist->m_Filters[ index ].m_Number ) + wxT( ")" );
        break;
      }

      case guDYNAMIC_FILTER_TYPE_LENGTH :  // LENGTH
      {
        query += wxT( "( song_length " ) +
                 DynPLNumericOption( playlist->m_Filters[ index ].m_Option,
                                  playlist->m_Filters[ index ].m_Number * 1000 ) + wxT( ")" );
        break;
      }

      case guDYNAMIC_FILTER_TYPE_PLAYCOUNT :  // PLAYCOUNT
      {
        query += wxT( "( song_playcount " ) +
                 DynPLNumericOption( playlist->m_Filters[ index ].m_Option,
                                  playlist->m_Filters[ index ].m_Number ) + wxT( ")" );
        break;
      }

      case guDYNAMIC_FILTER_TYPE_LASTPLAY :
      {
        query += wxT( "( IFNULL( song_lastplay, 0 ) " ) +
                 DynPLDateOption( playlist->m_Filters[ index ].m_Option,
                 playlist->m_Filters[ index ].m_Number,
                 playlist->m_Filters[ index ].m_Option2 ) + wxT( ")" );
        break;
      }

      case guDYNAMIC_FILTER_TYPE_ADDEDDATE :
      {
        query += wxT( "( song_addedtime " ) +
                 DynPLDateOption( playlist->m_Filters[ index ].m_Option,
                 playlist->m_Filters[ index ].m_Number,
                 playlist->m_Filters[ index ].m_Option2 ) + wxT( ")" );
        break;
      }

      case guDYNAMIC_FILTER_TYPE_TRACKNUMBER :  // TRACK NUMBER
      {
        query += wxT( "( song_number " ) +
                 DynPLNumericOption( playlist->m_Filters[ index ].m_Option,
                                  playlist->m_Filters[ index ].m_Number ) + wxT( ")" );
        break;
      }

      case guDYNAMIC_FILTER_TYPE_BITRATE :  // BITRATE
      {
        query += wxT( "( song_bitrate " ) +
                 DynPLNumericOption( playlist->m_Filters[ index ].m_Option,
                                  playlist->m_Filters[ index ].m_Number ) + wxT( ")" );
        break;
      }

      case guDYNAMIC_FILTER_TYPE_SIZE :  // SIZE
      {
        query += wxT( "( song_filesize " ) +
                 DynPLNumericOption( playlist->m_Filters[ index ].m_Option,
                                  playlist->m_Filters[ index ].m_Number ) + wxT( ")" );
        break;
      }

      case guDYNAMIC_FILTER_TYPE_DISK :  // DISK
      {
        query += wxT( "( song_disk " ) +
                 DynPLStringOption( playlist->m_Filters[ index ].m_Option,
                                  playlist->m_Filters[ index ].m_Text ) + wxT( ")" );
        break;
      }

      case guDYNAMIC_FILTER_TYPE_HASARTWORK :  // HAS ARTWORK
      {
        query += wxT( "( song_coverid " );
        query += ( playlist->m_Filters[ index ].m_Option ? wxT( "> 0 )" ) : wxT( "< 1 )" ) );
        break;
      }

    }
  }
  // SORTING Options
  //guLogMessage( wxT( "Sort: %u %u %u" ), playlist->m_Sorted, playlist->m_SortType, playlist->m_SortDesc );
  wxString sort;
  if( playlist->m_Sorted )
  {
    sort = wxT( " ORDER BY " );
    switch( playlist->m_SortType )
    {
        case guDYNAMIC_FILTER_ORDER_TITLE :         sort += wxT( "song_name" ); break;
        case guDYNAMIC_FILTER_ORDER_ARTIST :        sort += wxT( "song_artist" ); break;
        case guDYNAMIC_FILTER_ORDER_ALBUMARTIST :   sort += wxT( "song_albumartist" ); break;
        case guDYNAMIC_FILTER_ORDER_ALBUM :         sort += wxT( "song_album" ); break;
        case guDYNAMIC_FILTER_ORDER_GENRE :         sort += wxT( "song_genre" ); break;
        case guDYNAMIC_FILTER_ORDER_COMPOSER :      sort += wxT( "song_composer" ); break;
        case guDYNAMIC_FILTER_ORDER_YEAR :          sort += wxT( "song_year" ); break;
        case guDYNAMIC_FILTER_ORDER_RATING :        sort += wxT( "song_rating" ); break;
        case guDYNAMIC_FILTER_ORDER_LENGTH :        sort += wxT( "song_length" ); break;
        case guDYNAMIC_FILTER_ORDER_PLAYCOUNT :     sort += wxT( "song_playcount" ); break;
        case guDYNAMIC_FILTER_ORDER_LASTPLAY :      sort += wxT( "song_lastplay" ); break;
        case guDYNAMIC_FILTER_ORDER_ADDEDDATE :     sort += wxT( "song_addedtime" ); break;
        case guDYNAMIC_FILTER_ORDER_RANDOM :        sort += wxT( "RANDOM()" ); break;
    }

    if( playlist->m_SortDesc )
        sort += wxT( " DESC" );

    switch( playlist->m_SortType )
    {
        case guDYNAMIC_FILTER_ORDER_ARTIST :        sort += wxT( ", song_album, song_disk, song_number" ); break;
        case guDYNAMIC_FILTER_ORDER_ALBUMARTIST :   sort += wxT( ", song_album, song_disk, song_number" ); break;
        case guDYNAMIC_FILTER_ORDER_ALBUM :         sort += wxT( ", song_disk, song_number" ); break;
        case guDYNAMIC_FILTER_ORDER_GENRE :         sort += wxT( ", song_artist, song_album, song_disk, song_number" ); break;
        case guDYNAMIC_FILTER_ORDER_COMPOSER :      sort += wxT( ", song_album, song_disk, song_number" ); break;
        case guDYNAMIC_FILTER_ORDER_YEAR :          sort += wxT( ", song_album, song_disk, song_number" ); break;
        case guDYNAMIC_FILTER_ORDER_RATING :        sort += wxT( ", song_album, song_disk, song_number" ); break;
        case guDYNAMIC_FILTER_ORDER_ADDEDDATE :     sort += wxT( ", song_album, song_disk, song_number" ); break;
    }
  }

  //guLogMessage( wxT( "..., %s%s%s" ), dbNames.c_str(), query.c_str(), sort.c_str() );
  if( !dbSets.IsEmpty() )
  {
      dbSets = wxT( "( " ) + dbSets + wxT( ") AND " );
  }

  return dbNames + wxT( " WHERE " ) + dbSets + wxT( "(" ) + query + wxT( ")" ) + sort;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetPlayListSongIds( const int plid, wxArrayInt * tracks )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  query += wxString::Format( wxT( "SELECT plset_songid FROM plsets WHERE plset_plid = %u" ), plid );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    tracks->Add( dbRes.GetInt( 0 ) );;
  }
  dbRes.Finalize();

  return tracks->Count();
}

// -------------------------------------------------------------------------------- //
wxString guDbLibrary::GetPlayListQuery( const int plid )
{
    wxString RetVal = wxEmptyString;
    if( plid )
    {
        int PlType = GetPlayListType( plid );

        if( PlType == guPLAYLIST_TYPE_STATIC )
        {
            RetVal = wxString::Format( wxT( "SELECT plset_songid FROM plsets WHERE plset_plid = %u" ), plid );
        }
        else if( PlType == guPLAYLIST_TYPE_DYNAMIC )
        {
          guDynPlayList PlayList;
          GetDynamicPlayList( plid, &PlayList );

          RetVal = wxT( "SELECT song_id FROM songs " ) + DynPlayListToSQLQuery( &PlayList );
        }
    }
    //guLogMessage( wxT( "::GetPlaylistQuery %s"), RetVal.c_str() );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetPlayListSongs( const int plid, guTrackArray * tracks )
{
    int PlType = GetPlayListType( plid );
    if( PlType != wxNOT_FOUND )
    {
        return GetPlayListSongs( plid, PlType, tracks, NULL, NULL );
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetPlayListSongs( const int plid, const int pltype, guTrackArray * tracks,
                    wxArrayInt * setids, wxLongLong * len, wxLongLong * size,
                    const int order, const bool orderdesc )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  guTrack * Track;
  wxLongLong TrackLength = 0;
  wxLongLong TrackSize = 0;

  if( plid )
  {
    if( pltype == guPLAYLIST_TYPE_STATIC )
    {
      query = wxT( "SELECT song_id, song_name, song_genreid, song_genre, song_artistid, song_artist, " \
               "song_albumartistid, song_albumartist, song_composerid, song_composer, song_albumid, song_album, " \
               "song_pathid, song_path, song_filename, song_format, song_disk, song_number, song_year, song_comment, " \
               "song_coverid, song_offset, song_length, song_bitrate, song_rating, song_playcount, " \
               "song_addedtime, song_lastplay, song_filesize, plset_id " \
               "FROM songs " );
      query += wxString::Format( wxT( ", plsets WHERE plset_songid = song_id AND plset_plid = %u" ), plid );

      query += GetSongsSortSQL( order, orderdesc );

      dbRes = ExecuteQuery( query );

      while( dbRes.NextRow() )
      {
        Track = new guTrack();
        FillTrackFromDb( Track, &dbRes );
        tracks->Add( Track );
        TrackLength += Track->m_Length;
        TrackSize += Track->m_FileSize;
        if( setids )
            setids->Add( dbRes.GetInt( 29 ) );
      }
      dbRes.Finalize();
    }
    else //guPLAYLIST_TYPE_DYNAMIC
    {
      guDynPlayList PlayList;
      GetDynamicPlayList( plid, &PlayList );
      wxLongLong Limit;
      wxLongLong Count = 0;
      if( PlayList.m_Limited )
      {
          switch( PlayList.m_LimitType )
          {
              case guDYNAMIC_FILTER_LIMIT_TRACKS : // TRACKS
                Limit = PlayList.m_LimitValue;
                break;

              case guDYNAMIC_FILTER_LIMIT_MINUTES : // Minutes -> to seconds
                Limit = PlayList.m_LimitValue * 60000;
                break;

              case guDYNAMIC_FILTER_LIMIT_MEGABYTES : // MB -> To bytes
                Limit = PlayList.m_LimitValue;
                Limit *= 1000000;
                break;

              case guDYNAMIC_FILTER_LIMIT_GIGABYTES : // GB -> to bytes
                Limit = PlayList.m_LimitValue;
                Limit *= 1000000000;
                break;
          }
      }

      query = wxT( "SELECT song_id, song_name, song_genreid, song_genre, song_artistid, song_artist, " \
               "song_albumartistid, song_albumartist, song_composerid, song_composer, song_albumid, song_album, " \
               "song_pathid, song_path, song_filename, song_format, song_disk, song_number, song_year, song_comment, " \
               "song_coverid, song_offset, song_length, song_bitrate, song_rating, song_playcount, " \
               "song_addedtime, song_lastplay, song_filesize " \
               "FROM songs " );
      query += DynPlayListToSQLQuery( &PlayList );

      if( !PlayList.m_Sorted && ( order != wxNOT_FOUND ) )
      {
        query += GetSongsSortSQL( order, orderdesc );
      }

      //guLogMessage( wxT( "GetPlayListSongs: <<%s>>" ), query.c_str() );

      dbRes = ExecuteQuery( query );

      while( dbRes.NextRow() )
      {
        guTrack * Track = new guTrack();
        FillTrackFromDb( Track, &dbRes );
        if( PlayList.m_Limited )
        {
            //guLogMessage( wxT( "Limit: %lli / %lli" ), Count.GetValue(), Limit.GetValue() );

            if( PlayList.m_LimitType == guDYNAMIC_FILTER_LIMIT_TRACKS )
            {
                Count++;
            }
            else if( PlayList.m_LimitType == guDYNAMIC_FILTER_LIMIT_MINUTES )
            {
                Count += Track->m_Length;
            }
            else
            {
                Count += Track->m_FileSize;
            }
            if( Count > Limit )
                break;
        }
        tracks->Add( Track );
        TrackLength += Track->m_Length;
        TrackSize += Track->m_FileSize;
      }
      dbRes.Finalize();
    }
  }
  if( len )
    * len = TrackLength;
  if( size )
    * size = TrackSize;
  return tracks->Count();
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetPlayListSongs( const wxArrayInt &ids, const wxArrayInt &types, guTrackArray * tracks,
                    wxArrayInt * setids, wxLongLong * len, wxLongLong * size, const int order, const bool orderdesc )
{
    int Index;
    int Count = wxMin( ids.Count(), types.Count() );

    if( Count == 1 )
    {
        return GetPlayListSongs( ids[ 0 ], types[ 0 ], tracks, setids, len, size, order, orderdesc );
    }

    //
    // Only get multiple ids for static playlists
    //
    wxString query;
    wxSQLite3ResultSet dbRes;
    guTrack * Track;
    wxLongLong TrackLength = 0;
    wxLongLong TrackSize = 0;

    query = wxT( "SELECT song_id, song_name, song_genreid, song_genre, song_artistid, song_artist, " \
               "song_albumartistid, song_albumartist, song_composerid, song_composer, song_albumid, song_album, " \
               "song_pathid, song_path, song_filename, song_format, song_disk, song_number, song_year, song_comment, " \
               "song_coverid, song_offset, song_length, song_bitrate, song_rating, song_playcount, " \
               "song_addedtime, song_lastplay, song_filesize, plset_id " \
               "FROM songs " );
    query += wxT( ", plsets WHERE plset_songid = song_id AND plset_plid IN (" );
    for( Index = 0; Index < Count; Index++ )
    {
        query += wxString::Format( wxT( "%u," ), ids[ Index ] );
    }
    query.RemoveLast( 1 );
    query += wxT( ")" ) + GetSongsSortSQL( order, orderdesc );

    dbRes = ExecuteQuery( query );

    while( dbRes.NextRow() )
    {
        Track = new guTrack();
        FillTrackFromDb( Track, &dbRes );
        tracks->Add( Track );
        TrackLength += Track->m_Length;
        TrackSize += Track->m_FileSize;
        if( setids )
            setids->Add( dbRes.GetInt( 29 ) );
    }
    dbRes.Finalize();

    if( len )
        * len = TrackLength;
    if( size )
        * size = TrackSize;

    return tracks->Count();
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetPlayListSetIds( const int plid, wxArrayInt * setids )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxT( "SELECT plset_id FROM plsets WHERE plset_plid = " );
  query += wxString::Format( wxT( "%u" ), plid );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
      setids->Add( dbRes.GetInt( 0 ) );
  }
  dbRes.Finalize();

  return setids->Count();
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetPlayListSetIds( const wxArrayInt &plids, wxArrayInt * setids )
{
    int Index;
    int Count = plids.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        GetPlayListSetIds( plids[ Index ], setids );
    }
    return setids->Count();
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::GetDynamicPlayList( const int plid, guDynPlayList * playlist )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxString::Format( wxT( "SELECT playlist_name, playlist_limited, playlist_limitvalue, playlist_limittype, " \
               "playlist_sorted, playlist_sorttype, playlist_sortdesc, playlist_anyoption " \
               "FROM playlists WHERE playlist_id = %u LIMIT 1;" ), plid );
  dbRes = ExecuteQuery( query );
  if( dbRes.NextRow() )
  {
    playlist->m_Id = plid;
    playlist->m_Name = dbRes.GetString( 0 );
    playlist->m_Limited = dbRes.GetBool( 1 );
    playlist->m_LimitValue = dbRes.GetInt( 2 );
    playlist->m_LimitType = dbRes.GetInt( 3 );
    playlist->m_Sorted = dbRes.GetBool( 4 );
    playlist->m_SortType = dbRes.GetInt( 5 );
    playlist->m_SortDesc = dbRes.GetBool( 6 );
    playlist->m_AnyOption = dbRes.GetBool( 7 );
  }
  dbRes.Finalize();

  if( playlist->m_Id )
  {
    query = wxString::Format( wxT( "SELECT plset_type, plset_option, plset_text, plset_number, plset_option2 " \
                              "FROM plsets WHERE plset_plid = %u;" ), plid );
    dbRes = ExecuteQuery( query );
    while( dbRes.NextRow() )
    {
      guFilterItem * FilterItem = new guFilterItem();
      FilterItem->m_Type = dbRes.GetInt( 0 );
      FilterItem->m_Option = dbRes.GetInt( 1 );
      FilterItem->m_Text = dbRes.GetString( 2 );
      FilterItem->m_Number = dbRes.GetInt( 3 );
      FilterItem->m_Option2 = dbRes.GetInt( 4 );
      FilterItem->SetFilterLabel();
      playlist->m_Filters.Add( FilterItem );
    }
    dbRes.Finalize();
  }
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::UpdateDynamicPlayList( const int plid, const guDynPlayList * playlist )
{
  wxString query;

  query = wxString::Format( wxT( "UPDATE playlists SET playlist_name = '%s', " \
               "playlist_limited = %u, playlist_limitvalue = %u, playlist_limittype = %u, " \
               "playlist_sorted = %u, playlist_sorttype = %u, playlist_sortdesc = %u, " \
               "playlist_anyoption = %u WHERE playlist_id = %u;" ),
                escape_query_str( playlist->m_Name ).c_str(),
               playlist->m_Limited, playlist->m_LimitValue, playlist->m_LimitType,
               playlist->m_Sorted, playlist->m_SortType, playlist->m_SortDesc,
               playlist->m_AnyOption,
               plid );

  ExecuteUpdate( query );

  query = wxString::Format( wxT( "DELETE FROM plsets WHERE plset_plid = %u;" ), plid );
  ExecuteUpdate( query );

  int index;
  int count = playlist->m_Filters.Count();
  for( index = 0; index < count; index++ )
  {
    guFilterItem * FilterItem = &playlist->m_Filters[ index ];

    query = wxString::Format( wxT( "INSERT INTO plsets( plset_id, plset_plid, plset_songid, " \
                  "plset_type, plset_option, plset_text, plset_number, plset_option2 ) " \
                  "VALUES( NULL, %u, 0, %u, %u, '%s', %u, %u );" ),
                  plid,
                  FilterItem->m_Type,
                  FilterItem->m_Option,
                  escape_query_str( FilterItem->m_Text ).c_str(),
                  FilterItem->m_Number,
                  FilterItem->m_Option2
                   );

    ExecuteUpdate( query );
  }
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::DeletePlayList( const int plid )
{
  wxString query;

  if( plid )
  {
    query = wxString::Format( wxT( "DELETE FROM plsets WHERE plset_plid = %u;" ), plid );
    ExecuteUpdate( query );

    query = wxString::Format( wxT( "DELETE FROM playlists WHERE playlist_id = %u;" ), plid );
    ExecuteUpdate( query );
  }
}

// -------------------------------------------------------------------------------- //
wxString guDbLibrary::GetPlayListName( const int plid )
{
  wxString RetVal;
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxString::Format( wxT( "SELECT playlist_name FROM playlists WHERE playlist_id = %u LIMIT 1;" ), plid );

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
      RetVal = dbRes.GetString( 0 );
  }
  dbRes.Finalize();
  return RetVal;
}

// -------------------------------------------------------------------------------- //
wxString guDbLibrary::GetPlayListPath( const int plid )
{
  wxString RetVal;
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxString::Format( wxT( "SELECT playlist_path FROM playlists WHERE playlist_id = %u LIMIT 1;" ), plid );

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
      RetVal = dbRes.GetString( 0 );
  }
  dbRes.Finalize();
  return RetVal;
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetPlayListPath( const int plid, const wxString &path )
{
  wxString query;

  if( plid )
  {
    query = wxString::Format( wxT( "UPDATE playlists SET playlist_path = '%s' WHERE playlist_id = %u;" ),
          escape_query_str( path ).c_str(), plid );
    ExecuteUpdate( query );
  }
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetPlayListName( const int plid, const wxString &plname )
{
  wxString query;

  if( plid )
  {
    query = wxString::Format( wxT( "UPDATE playlists SET playlist_name = '%s' WHERE playlist_id = %u;" ),
          escape_query_str( plname ).c_str(), plid );
    ExecuteUpdate( query );
  }
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetLabelsSongs( const wxArrayInt &Labels, guTrackArray * Songs )
{
  wxString subquery;
  wxString query;
  wxSQLite3ResultSet dbRes;
  guTrack * Song;

  if( Labels.Count() )
  {
    subquery = ArrayIntToStrList( Labels );

    query = GU_TRACKS_QUERYSTR;
    //query += GetSongsDBNamesSQL( m_TracksOrder );
    query += ( query.Find( wxT( "WHERE" ) ) == wxNOT_FOUND ) ? wxT( "WHERE " ) : wxT( "AND " );
    query += wxT( "( (song_artistid IN ( SELECT settag_artistid FROM settags WHERE " \
                  "settag_tagid IN " ) + subquery + wxT( " and settag_artistid > 0 ) ) OR" );
    query += wxT( " (song_albumid IN ( SELECT settag_albumid FROM settags WHERE " \
                  "settag_tagid IN " ) + subquery + wxT( " and settag_albumid > 0 ) ) OR" );
    query += wxT( " (song_id IN ( SELECT settag_songid FROM settags WHERE " \
                  "settag_tagid IN " ) + subquery + wxT( " and settag_songid > 0 ) ) ) " );
    query += GetSongsSortSQL( m_TracksOrder, m_TracksOrderDesc );

    dbRes = ExecuteQuery( query );

    while( dbRes.NextRow() )
    {
      Song = new guTrack();
      FillTrackFromDb( Song, &dbRes );
      Songs->Add( Song );
    }
    dbRes.Finalize();
  }
  return Songs->Count();
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetGenresSongs( const wxArrayInt &Genres, guTrackArray * Songs )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  guTrack * Song;

  if( Genres.Count() )
  {
    query = GU_TRACKS_QUERYSTR;
    //query += GetSongsDBNamesSQL( m_TracksOrder );
    query += ( query.Find( wxT( "WHERE" ) ) == wxNOT_FOUND ) ? wxT( "WHERE " ) : wxT( "AND " );
    query += wxT( "song_genreid IN " ) + ArrayIntToStrList( Genres );
    query += GetSongsSortSQL( m_TracksOrder, m_TracksOrderDesc );

    dbRes = ExecuteQuery( query );

    while( dbRes.NextRow() )
    {
      Song = new guTrack();
      FillTrackFromDb( Song, &dbRes );
      Songs->Add( Song );
    }
    dbRes.Finalize();
  }
  return Songs->Count();
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetArtistsSongs( const wxArrayInt &Artists, guTrackArray * Songs, guTrackMode trackmode )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  guTrack * Song;
  if( Artists.Count() )
  {
    query = GU_TRACKS_QUERYSTR;
    //query += GetSongsDBNamesSQL( m_TracksOrder );
    query += ( query.Find( wxT( "WHERE" ) ) == wxNOT_FOUND ) ? wxT( "WHERE " ) : wxT( "AND " );
    query += wxT( "song_artistid IN " ) + ArrayIntToStrList( Artists );
    query += GetSongsSortSQL( m_TracksOrder, m_TracksOrderDesc );

    dbRes = ExecuteQuery( query );

    while( dbRes.NextRow() )
    {
      Song = new guTrack();
      FillTrackFromDb( Song, &dbRes );
      Song->m_TrackMode = trackmode;
      Songs->Add( Song );
    }
    dbRes.Finalize();
  }
  return Songs->Count();
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetArtistsSongs( const wxArrayInt &Artists, guTrackArray * Songs,
        const int trackcount, const int filterallow, const int filterdeny )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  guTrack * Song;

  if( Artists.Count() )
  {
    wxString AllowPlQuery = GetPlayListQuery( filterallow );
    wxString DenyPlQuery = GetPlayListQuery( filterdeny );

    wxString Filters = wxEmptyString;
    if( !AllowPlQuery.IsEmpty() )
    {
      Filters += wxT( "WHERE song_id IN ( " ) + AllowPlQuery + wxT( " ) " );
    }

    if( !DenyPlQuery.IsEmpty() )
    {
      Filters += Filters.IsEmpty() ? wxT( "WHERE " ) : wxT( "AND " );
      Filters += wxT( "song_id NOT IN ( " ) + DenyPlQuery + wxT( " ) " );
    }


    query = GU_TRACKS_QUERYSTR;

    query += Filters;

    if( !Filters.IsEmpty() )
        query += wxT( " AND" );
    else
        query += wxT( " WHERE" );
    query += wxT( " song_artistid IN " ) + ArrayIntToStrList( Artists );
    query += wxString::Format( wxT( " ORDER BY RANDOM() LIMIT %u" ), trackcount );

    //guLogMessage( wxT( "GetArtistsSongs:\n%s" ), query.c_str() );

    dbRes = ExecuteQuery( query );

    while( dbRes.NextRow() )
    {
      Song = new guTrack();
      FillTrackFromDb( Song, &dbRes );
      Song->m_TrackMode = guTRACK_MODE_SMART;
      Songs->Add( Song );
    }
    dbRes.Finalize();
  }
  return Songs->Count();
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetArtistsAlbums( const wxArrayInt &Artists, wxArrayInt * Albums )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  if( Artists.Count() )
  {
    query = wxT( "SELECT DISTINCT song_albumid FROM songs WHERE song_artistid IN " );
    query += ArrayIntToStrList( Artists );

    dbRes = ExecuteQuery( query );

    while( dbRes.NextRow() )
    {
        Albums->Add( dbRes.GetInt( 0 ) );
    }
    dbRes.Finalize();
  }
  return Albums->Count();

}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetAlbumArtistsAlbums( const wxArrayInt &albumartists, wxArrayInt * Albums )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  if( albumartists.Count() )
  {
    query = wxT( "SELECT DISTINCT song_albumid FROM songs WHERE song_albumartistid IN " );
    query += ArrayIntToStrList( albumartists );

    dbRes = ExecuteQuery( query );

    while( dbRes.NextRow() )
    {
        Albums->Add( dbRes.GetInt( 0 ) );
    }
    dbRes.Finalize();
  }
  return Albums->Count();
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetAlbumsSongs( const wxArrayInt &Albums, guTrackArray * Songs, bool ordertoedit )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  guTrack * Song;
  if( Albums.Count() )
  {
    query = GU_TRACKS_QUERYSTR;
    //query += GetSongsDBNamesSQL( m_TracksOrder );
    query += ( query.Find( wxT( "WHERE" ) ) == wxNOT_FOUND ) ? wxT( "WHERE " ) : wxT( "AND " );
    query += wxT( "song_albumid IN " ) + ArrayIntToStrList( Albums );
    if( ordertoedit )
    {
        query += wxT( " ORDER BY song_disk, song_number, song_filename" );
    }
    else
    {
        query += GetSongsSortSQL( m_TracksOrder, m_TracksOrderDesc );
    }

    //guLogMessage( wxT( "GetAlbumsSongs: %s" ), query.c_str() );
    dbRes = ExecuteQuery( query );

    while( dbRes.NextRow() )
    {
      Song = new guTrack();
      FillTrackFromDb( Song, &dbRes );
      Songs->Add( Song );
    }
    dbRes.Finalize();
  }
  return Songs->Count();
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetYearsSongs( const wxArrayInt &Years, guTrackArray * Songs )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  guTrack * Song;
  if( Years.Count() )
  {
    query = GU_TRACKS_QUERYSTR;
    //query += GetSongsDBNamesSQL( m_TracksOrder );
    query += ( query.Find( wxT( "WHERE" ) ) == wxNOT_FOUND ) ? wxT( "WHERE " ) : wxT( "AND " );
    query += wxT( "song_year IN " ) + ArrayIntToStrList( Years );
    query += GetSongsSortSQL( m_TracksOrder, m_TracksOrderDesc );

    dbRes = ExecuteQuery( query );

    while( dbRes.NextRow() )
    {
      Song = new guTrack();
      FillTrackFromDb( Song, &dbRes );
      Songs->Add( Song );
    }
    dbRes.Finalize();
  }
  return Songs->Count();
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetRatingsSongs( const wxArrayInt &Ratings, guTrackArray * Songs )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  guTrack * Song;
  if( Ratings.Count() )
  {
    query = GU_TRACKS_QUERYSTR;
    //query += GetSongsDBNamesSQL( m_TracksOrder );
    query += ( query.Find( wxT( "WHERE" ) ) == wxNOT_FOUND ) ? wxT( "WHERE " ) : wxT( "AND " );
    query += wxT( "song_rating IN " ) + ArrayIntToStrList( Ratings );
    query += GetSongsSortSQL( m_TracksOrder, m_TracksOrderDesc );

    dbRes = ExecuteQuery( query );

    while( dbRes.NextRow() )
    {
      Song = new guTrack();
      FillTrackFromDb( Song, &dbRes );
      Songs->Add( Song );
    }
    dbRes.Finalize();
  }
  return Songs->Count();
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetPlayCountsSongs( const wxArrayInt &PlayCounts, guTrackArray * Songs )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  guTrack * Song;
  if( PlayCounts.Count() )
  {
    query = GU_TRACKS_QUERYSTR;
    //query += GetSongsDBNamesSQL( m_TracksOrder );
    query += ( query.Find( wxT( "WHERE" ) ) == wxNOT_FOUND ) ? wxT( "WHERE " ) : wxT( "AND " );
    query += wxT( "song_playcount IN " ) + ArrayIntToStrList( PlayCounts );
    query += GetSongsSortSQL( m_TracksOrder, m_TracksOrderDesc );

    dbRes = ExecuteQuery( query );

    while( dbRes.NextRow() )
    {
      Song = new guTrack();
      FillTrackFromDb( Song, &dbRes );
      Songs->Add( Song );
    }
    dbRes.Finalize();
  }
  return Songs->Count();
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetAlbumArtistsSongs( const wxArrayInt &albumartists, guTrackArray * tracks )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  guTrack * Track;
  if( albumartists.Count() )
  {
    query = GU_TRACKS_QUERYSTR;
    //query += GetSongsDBNamesSQL( m_TracksOrder );
    query += ( query.Find( wxT( "WHERE" ) ) == wxNOT_FOUND ) ? wxT( "WHERE " ) : wxT( "AND " );
    query += wxT( "song_albumartistid IN " ) + ArrayIntToStrList( albumartists );
    query += GetSongsSortSQL( m_TracksOrder, m_TracksOrderDesc );

    dbRes = ExecuteQuery( query );

    while( dbRes.NextRow() )
    {
      Track = new guTrack();
      FillTrackFromDb( Track, &dbRes );
      tracks->Add( Track );
    }
    dbRes.Finalize();
  }
  return tracks->Count();
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetComposersSongs( const wxArrayInt &Composers, guTrackArray * Songs )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  guTrack * Song;
  if( Composers.Count() )
  {
    query = GU_TRACKS_QUERYSTR;
    //query += GetSongsDBNamesSQL( m_TracksOrder );
    query += ( query.Find( wxT( "WHERE" ) ) == wxNOT_FOUND ) ? wxT( "WHERE " ) : wxT( "AND " );
    query += wxT( "song_composerid IN " ) + ArrayIntToStrList( Composers );
    query += GetSongsSortSQL( m_TracksOrder, m_TracksOrderDesc );

    dbRes = ExecuteQuery( query );

    while( dbRes.NextRow() )
    {
      Song = new guTrack();
      FillTrackFromDb( Song, &dbRes );
      Songs->Add( Song );
    }
    dbRes.Finalize();
  }
  return Songs->Count();
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetPlayListType( const int plid )
{
  wxString              query;
  wxSQLite3ResultSet    dbRes;
  int                   RetVal = wxNOT_FOUND;

  query = wxString::Format( wxT( "SELECT playlist_type FROM playlists WHERE playlist_id = %u LIMIT 1;" ), plid );

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
      RetVal = dbRes.GetInt( 0 );
  }
  dbRes.Finalize();

  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetRandomTracks( guTrackArray * tracks, const int count, const int rndmode,
                                     const int allowplaylist, const int denyplaylist )
{
  wxString              query;
  wxSQLite3ResultSet    dbRes;
  guTrack *             Track;
  wxString              AllowPlQuery = wxEmptyString;
  wxString              DenyPlQuery = wxEmptyString;

  //guLogMessage( wxT( "GetRandomTracks... %i %i %i" ), rndmode, allowplaylist, denyplaylist );

  AllowPlQuery = GetPlayListQuery( allowplaylist );
  DenyPlQuery = GetPlayListQuery( denyplaylist );

  wxString Filters = wxEmptyString;
  if( !AllowPlQuery.IsEmpty() )
  {
      Filters += wxT( "WHERE song_id IN ( " ) + AllowPlQuery + wxT( " ) " );
  }

  if( !DenyPlQuery.IsEmpty() )
  {
    Filters += Filters.IsEmpty() ? wxT( "WHERE " ) : wxT( "AND " );
    Filters += wxT( "song_id NOT IN ( " ) + DenyPlQuery + wxT( " ) " );
  }

  if( rndmode == guRANDOM_MODE_TRACK )
  {
      query = GU_TRACKS_QUERYSTR + Filters;
      query += wxString::Format( wxT( " ORDER BY RANDOM() LIMIT %u;" ), count );
  }
  else //if( rndmode == guRANDOM_MODE_ALBUM )
  {
      int AlbumId = 0;
      query = wxT( "SELECT song_albumid FROM songs " ) + Filters;
      query += wxT( " ORDER BY RANDOM() LIMIT 1;" );
      dbRes = ExecuteQuery( query );
      if( dbRes.NextRow() )
      {
          AlbumId = dbRes.GetInt( 0 );
      }
      dbRes.Finalize();
      query = GU_TRACKS_QUERYSTR + wxString::Format( wxT( "WHERE song_albumid = %i" ), AlbumId );
      query += wxT( " ORDER BY song_disk, song_number, song_filename" );
      //query += GetSongsSortSQL( m_TracksOrder, m_TracksOrderDesc );
  }

  //guLogMessage( wxT( "GetRandomTracks:\n%s" ), query.c_str() );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    Track = new guTrack();
    FillTrackFromDb( Track, &dbRes );
    Track->m_TrackMode = guTRACK_MODE_RANDOM;
    tracks->Add( Track );
  }
  dbRes.Finalize();

  return tracks->Count();
}

// -------------------------------------------------------------------------------- //
const wxString guDbLibrary::GetArtistName( const int ArtistId )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  wxString RetVal = wxEmptyString;

  query = wxString::Format( wxT( "SELECT song_artist FROM songs WHERE song_artistid = %u LIMIT 1;" ), ArtistId );

  dbRes = ExecuteQuery( query );
  if( dbRes.NextRow() )
  {
    RetVal = dbRes.GetString( 0 );
  }
  dbRes.Finalize();
  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetArtistId( wxString &artistname, bool create )
{
  if( m_LastArtist == artistname )
  {
      return m_LastArtistId;
  }

  // TODO Add a Lock
  wxString query;
  wxSQLite3ResultSet dbRes;

  m_LastArtist = artistname;

  query = wxString::Format( wxT( "SELECT song_artistid FROM songs " \
                                 "WHERE song_artist = '%s' LIMIT 1;" ),
            escape_query_str( artistname ).c_str() );

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
    return m_LastArtistId = dbRes.GetInt( 0 );
  }
  else if( create )
  {
    query = wxT( "SELECT MAX(song_artistid) FROM songs" );

    dbRes = ExecuteQuery( query );
    if( dbRes.NextRow() )
    {
      return m_LastArtistId = dbRes.GetInt( 0 ) + 1;
    }
    else
    {
        return m_LastArtistId = 1;
    }
  }

  return wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetAlbumArtistId( wxString &albumartist, bool create )
{
  if( m_LastAlbumArtist == albumartist )
  {
      return m_LastAlbumArtistId;
  }

  // TODO Add a Lock
  wxString query;
  wxSQLite3ResultSet dbRes;

  m_LastAlbumArtist = albumartist;

  query = wxString::Format( wxT( "SELECT song_albumartistid FROM songs " \
                                 "WHERE song_albumartist = '%s' LIMIT 1;" ),
            escape_query_str( albumartist ).c_str() );

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
    return m_LastAlbumArtistId = dbRes.GetInt( 0 );
  }
  else if( create )
  {
    query = wxT( "SELECT MAX(song_albumartistid) FROM songs;" );

    dbRes = ExecuteQuery( query );
    if( dbRes.NextRow() )
    {
      return m_LastAlbumArtistId = dbRes.GetInt( 0 ) + 1;
    }
    else
    {
        return m_LastAlbumArtistId = 1;
    }
  }

  return wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::FindArtist( const wxString &Artist )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxString::Format( wxT( "SELECT song_artistid FROM songs WHERE song_artist = '%s' LIMIT 1;" ),
                            escape_query_str( Artist ).c_str() );

  dbRes = ExecuteQuery( query );
  if( dbRes.NextRow() )
  {
      return dbRes.GetInt( 0 );
  }
  return wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::FindAlbum( const wxString &Artist, const wxString &Album )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  int ArtistId = FindArtist( Artist );
  if( ArtistId != wxNOT_FOUND )
  {
    query = wxString::Format( wxT( "SELECT song_albumid FROM songs WHERE " \
                "song_artistid = %d AND song_album = '%s' LIMIT 1;" ),
                ArtistId,
                escape_query_str( Album ).c_str() );

    dbRes = ExecuteQuery( query );
    if( dbRes.NextRow() )
    {
      return dbRes.GetInt( 0 );
    }
  }
  return wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::FindTrack( const wxString &Artist, const wxString &Name )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  int ArtistId = FindArtist( Artist );
  if( ArtistId != wxNOT_FOUND )
  {
    query = wxString::Format( wxT( "SELECT song_id FROM songs WHERE " )\
                wxT( "song_artistid = %d AND song_name = '%s' LIMIT 1;" ),
                ArtistId,
                escape_query_str( Name ).c_str() );

    dbRes = ExecuteQuery( query );

    if( dbRes.NextRow() )
    {
      return dbRes.GetInt( 0 );
    }
  }
  return wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetTrackIndex( const int trackid )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  int Index = 0;

  query = wxT( "SELECT song_id FROM songs " );
  query += GetSongsSortSQL( m_TracksOrder, m_TracksOrderDesc );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    if( dbRes.GetInt( 0 ) == trackid )
        return Index;
    Index++;
  }
  return wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
guTrack * guDbLibrary::FindSong( const wxString &artist, const wxString &trackname,
              const int filterallow, const int filterdeny )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  guTrack * RetVal = NULL;
  wxString ArtistName = artist;
  wxString TrackName = trackname;

  wxString Filters = wxEmptyString;

  wxString AllowPlQuery = GetPlayListQuery( filterallow );
  if( !AllowPlQuery.IsEmpty() )
  {
      Filters += wxT( "WHERE song_id IN ( " ) + AllowPlQuery + wxT( " ) " );
  }

  wxString DenyPlQuery = GetPlayListQuery( filterdeny );
  if( !DenyPlQuery.IsEmpty() )
  {
    Filters += Filters.IsEmpty() ? wxT( "WHERE " ) : wxT( "AND " );
    Filters += wxT( "song_id NOT IN ( " ) + DenyPlQuery + wxT( " ) " );
  }

  escape_query_str( &ArtistName );
  escape_query_str( &TrackName );

  query = GU_TRACKS_QUERYSTR;

  query += Filters;
  if( !Filters.IsEmpty() )
    query += wxT( " AND" );
  else
    query += wxT( " WHERE" );
  query += wxString::Format( wxT( " song_artist = '%s' AND song_name = '%s' LIMIT 1;" ), ArtistName.c_str(), TrackName.c_str() );

  //guLogMessage( wxT( "FindSong:\n%s" ), query.c_str() );

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
    RetVal = new guTrack();
    FillTrackFromDb( RetVal, &dbRes );
  }
  dbRes.Finalize();
  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::FindTrackFile( const wxString &filename, guTrack * track )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  int RetVal = 0;

  wxString Path = wxPathOnly( filename );
  if( !Path.EndsWith( wxT( "/" ) ) )
    Path += '/';
  escape_query_str( &Path );

//  query = wxString::Format( wxT( "SELECT path_id FROM paths WHERE path_value = '%s' LIMIT 1" ), Path.c_str() );
//
//  dbRes = ExecuteQuery( query );
//
//  if( dbRes.NextRow() )
//  {
//      PathId = dbRes.GetInt( 0 );
//  }
//  dbRes.Finalize();
//
//  if( PathId > 0 )
//  {
//    Path = filename.AfterLast( '/' );
//    escape_query_str( &Path );

    query = GU_TRACKS_QUERYSTR +
            wxString::Format( wxT( " WHERE song_path = '%s' AND song_filename = '%s' LIMIT 1;" ),
                    Path.c_str(),
                    escape_query_str( filename.AfterLast( '/' ) ).c_str() );
    dbRes = ExecuteQuery( query );
    if( dbRes.NextRow() )
    {
      RetVal = dbRes.GetInt( 0 );
      if( track )
      {
          FillTrackFromDb( track, &dbRes );
      }
    }
    dbRes.Finalize();
//  }
  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::FindTrackId( const int trackid, guTrack * track )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  int RetVal = 0;

  query = GU_TRACKS_QUERYSTR + wxString::Format( wxT( " WHERE song_id = %u LIMIT 1;" ), trackid );

  dbRes = ExecuteQuery( query );
  if( dbRes.NextRow() )
  {
    RetVal = dbRes.GetInt( 0 );
    if( track )
    {
        FillTrackFromDb( track, &dbRes );
    }
  }
  dbRes.Finalize();

  return RetVal;
}

// -------------------------------------------------------------------------------- //
wxString GetSongsDBNamesSQL( const int order )
{
  wxString query = wxEmptyString;
  //
  switch( order )
  {
    case guTRACKS_ORDER_YEAR :
    case guTRACKS_ORDER_DISK :
      //query += wxT( ",albums WHERE song_albumid = album_id " );
      break;

    case guTRACKS_ORDER_ARTIST :
      //query += wxT( ",artists,albums WHERE song_artistid = artist_id AND song_albumid = album_id " );
      break;

    case guTRACKS_ORDER_ALBUM :
      //query += wxT( ",albums WHERE song_albumid = album_id " );
      break;

    case guTRACKS_ORDER_GENRE :
      //query += wxT( ",genres WHERE song_genreid = genre_id " );
      break;

    case guTRACKS_ORDER_COMPOSER :
      //query += wxT( ",composers WHERE song_composerid = composer_id " );
      break;

//    case guTRACKS_ORDER_TITLE :
//    case guTRACKS_ORDER_NUMBER :
//    case guTRACKS_ORDER_LENGTH :
//    case guTRACKS_ORDER_RATING :
//    case guTRACKS_ORDER_BITRATE :
//    case guTRACKS_ORDER_PLAYCOUNT :
//    case guTRACKS_ORDER_LASTPLAY :
//    case guTRACKS_ORDER_ADDEDDATE :
//      break;
    default:
        break;
  }
  return query;
}

// -------------------------------------------------------------------------------- //
wxString GetSongsSortSQL( const int order, const bool orderdesc )
{
  wxString query = wxT( " ORDER BY " );
  //
  switch( order )
  {
    case guTRACKS_ORDER_TITLE :
      query += wxT( "song_name" );
      break;

    case guTRACKS_ORDER_ARTIST :
      query += wxT( "song_artist" );
      break;

    case guTRACKS_ORDER_ALBUMARTIST :
      query += wxT( "song_albumartist" );
      break;

    case guTRACKS_ORDER_ALBUM :
      query += wxT( "song_album" );
      break;

    case guTRACKS_ORDER_GENRE :
      query += wxT( "song_genre" );
      break;

    case guTRACKS_ORDER_COMPOSER :
      //query += wxT( "composer_name " );
      query += wxT( "song_composer" );
      break;

    case guTRACKS_ORDER_DISK :
      query += wxT( "song_disk" );
      break;

    case guTRACKS_ORDER_NUMBER :
      query += wxT( "song_number" );
      break;

    case guTRACKS_ORDER_LENGTH :
      query += wxT( "song_length" );
      break;

    case guTRACKS_ORDER_YEAR :
      query += wxT( "song_year" );
      break;

    case guTRACKS_ORDER_RATING :
      query += wxT( "song_rating" );
      break;

    case guTRACKS_ORDER_BITRATE :
      query += wxT( "song_bitrate" );
      break;

    case guTRACKS_ORDER_PLAYCOUNT :
      query += wxT( "song_playcount" );
      break;

    case guTRACKS_ORDER_LASTPLAY :
      query += wxT( "song_lastplay" );
      break;

    case guTRACKS_ORDER_ADDEDDATE :
      query += wxT( "song_addedtime" );
      break;

    case guTRACKS_ORDER_FORMAT :
      query += wxT( "song_format" );
      break;

    case guTRACKS_ORDER_FILEPATH :
      query += wxT( "song_path" );
      break;

    default :
        return wxEmptyString;
  }
  //
  if( orderdesc )
    query += wxT( " DESC" );

  switch( order )
  {
    case guTRACKS_ORDER_DISK :
      query += wxT( ",song_albumid,song_number " );
      break;

    case guTRACKS_ORDER_COMPOSER :
      query += wxT( ",song_artist,song_album,song_disk,song_albumid,song_number " );
      break;

    case guTRACKS_ORDER_ARTIST :
    case guTRACKS_ORDER_ALBUMARTIST :
      query += wxT( ",song_album,song_disk,song_albumid,song_number " );
      break;

    case guTRACKS_ORDER_ALBUM :
      query += wxT( ",song_disk,song_albumid,song_number " );
      break;

    case guTRACKS_ORDER_YEAR :
      query += wxT( ",song_album,song_disk,song_albumid,song_number " );
      break;

    case guTRACKS_ORDER_ADDEDDATE :
      query += wxT( ",song_album,song_disk,song_albumid,song_number " );
      break;

    default:
      break;

  }
  //
  return query;
}

// -------------------------------------------------------------------------------- //
bool guDbLibrary::GetSong( const int songid, guTrack * song )
{
  bool RetVal = false;
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = GU_TRACKS_QUERYSTR;
  query += wxString::Format( wxT( "WHERE song_id = %i LIMIT 1;" ), songid );

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
    FillTrackFromDb( song, &dbRes );
    RetVal = true;
  }
  dbRes.Finalize();

  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetSongs( const wxArrayInt &SongIds, guTrackArray * Songs )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  guTrack * Song;

  query = GU_TRACKS_QUERYSTR;
  //query += ( query.Find( wxT( "WHERE" ) ) == wxNOT_FOUND ) ? wxT( "WHERE " ) : wxT( "AND " );
  query += wxT( "WHERE song_id IN " ) + ArrayIntToStrList( SongIds );
  query += GetSongsSortSQL( m_TracksOrder, m_TracksOrderDesc );

  //guLogMessage( wxT( "%s" ), query.c_str() );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    Song = new guTrack();
    FillTrackFromDb( Song, &dbRes );
    Songs->Add( Song );
  }
  dbRes.Finalize();

  return Songs->Count();
}

// -------------------------------------------------------------------------------- //
wxString inline GetTreeViewFilterEntrySql( const guTreeViewFilterEntry &filterentry )
{
  wxString RetVal = wxT( "(" );
  int Index;
  int Count = filterentry.Count();

  for( Index = 0; Index < Count; Index++ )
  {
    switch( filterentry[ Index ].m_Type )
    {
        case guLIBRARY_ELEMENT_GENRES :
            RetVal += wxT( "song_genreid=" );
            break;

        case guLIBRARY_ELEMENT_ARTISTS :
            RetVal += wxT( "song_artistid=" );
            break;

        case guLIBRARY_ELEMENT_COMPOSERS :
            RetVal += wxT( "song_composerid=" );
            break;

        case guLIBRARY_ELEMENT_ALBUMARTISTS :
            RetVal += wxT( "song_albumartistid=" );
            break;

        case guLIBRARY_ELEMENT_ALBUMS :
            RetVal += wxT( "song_albumid=" );
            break;

        case guLIBRARY_ELEMENT_YEARS :
            RetVal += wxT( "song_year=" );
            break;

        case guLIBRARY_ELEMENT_RATINGS :
            RetVal += wxT( "song_rating=" );
            break;

        case guLIBRARY_ELEMENT_PLAYCOUNT :
            RetVal += wxT( "song_playcount=" );
            break;
    }

    RetVal += wxString::Format( wxT( "%i AND " ), filterentry[ Index ].m_Id );
  }
  RetVal.RemoveLast( 4 );
  RetVal += wxT( ")" );
  return RetVal;
}

// -------------------------------------------------------------------------------- //
wxString inline GetTreeViewFilterArraySql( const guTreeViewFilterArray &filterarray )
{
  wxString RetVal = wxT( "(" );
  int Index;
  int Count = filterarray.Count();

  for( Index = 0; Index < Count; Index++ )
  {
    RetVal += GetTreeViewFilterEntrySql( filterarray[ Index ] );
    RetVal += wxT( "OR " );
  }
  RetVal.RemoveLast( 3 );
  RetVal += wxT( ")" );
  return RetVal;
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::GetSongsCounters( const guTreeViewFilterArray &filters, const wxArrayString &textfilters, wxLongLong * count, wxLongLong * len, wxLongLong * size )
{
  if( !filters.Count() )
  {
      * count = 0;
      * len = 0;
      * size = 0;
      return;
  }
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxT( "SELECT COUNT(), SUM(song_length), SUM(song_filesize) FROM songs " );
  query += wxT( "WHERE " ) + GetTreeViewFilterArraySql( filters );
  if( textfilters.Count() )
  {
    query += wxT( " AND " ) + TextFilterToSQL( textfilters );
  }

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
      * count = dbRes.GetInt64( 0 );
      * len   = dbRes.GetInt64( 1 );
      * size  = dbRes.GetInt64( 2 );
  }
  dbRes.Finalize();
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetSongs( const guTreeViewFilterArray &filters, guTrackArray * Songs, const wxArrayString &textfilters, const int order, const bool orderdesc )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  guTrack * Song;

  query = GU_TRACKS_QUERYSTR;
  query += wxT( "WHERE " ) + GetTreeViewFilterArraySql( filters );
  if( textfilters.Count() )
  {
    query += wxT( " AND " ) + TextFilterToSQL( textfilters );
  }
  query += GetSongsSortSQL( order, orderdesc );

  //guLogMessage( wxT( "%s" ), query.c_str() );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    Song = new guTrack();
    FillTrackFromDb( Song, &dbRes );
    Songs->Add( Song );
  }
  dbRes.Finalize();

  return Songs->Count();
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetSongsCount( void )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxT( "SELECT COUNT(song_id) FROM songs " );
  //query += GetSongsDBNamesSQL( m_TracksOrder );
  if( GetFiltersCount() )
  {
    //query += ( query.Find( wxT( "WHERE" ) ) == wxNOT_FOUND ) ? wxT( "WHERE " ) : wxT( "AND " );
    query += wxT( "WHERE " ) + FiltersSQL( guLIBRARY_FILTER_SONGS );
  }
//  query += GetSongsSortSQL( m_TracksOrder, m_TracksOrderDesc );

  //guLogMessage( wxT( "%s" ), query.c_str() );
  dbRes = ExecuteQuery( query );
  if( dbRes.NextRow() )
  {
      return dbRes.GetInt( 0 );
  }
  return 0;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetSongs( guTrackArray * Songs, const int start, const int end )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  guTrack * Song;

  query = GU_TRACKS_QUERYSTR;
  //query += GetSongsDBNamesSQL( m_TracksOrder );
  if( GetFiltersCount() )
  {
    query += ( query.Find( wxT( "WHERE" ) ) == wxNOT_FOUND ) ? wxT( "WHERE " ) : wxT( "AND " );
    query += FiltersSQL( guLIBRARY_FILTER_SONGS );
  }
  query += GetSongsSortSQL( m_TracksOrder, m_TracksOrderDesc );
  query += wxString::Format( wxT( " LIMIT %i, %i " ), start, end - start + 1 );

  //guLogMessage( wxT( "%s" ), query.c_str() );
  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
      Song = new guTrack();
      FillTrackFromDb( Song, &dbRes );
      Songs->Add( Song );
  }
  dbRes.Finalize();

  return Songs->Count();
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetSongsId( const int start )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxT( "SELECT song_id FROM songs " );
  if( GetFiltersCount() )
  {
    query += ( query.Find( wxT( "WHERE" ) ) == wxNOT_FOUND ) ? wxT( "WHERE " ) : wxT( "AND " );
    query += FiltersSQL( guLIBRARY_FILTER_SONGS );
  }
  query += GetSongsSortSQL( m_TracksOrder, m_TracksOrderDesc );
  query += wxString::Format( wxT( " LIMIT %i, 1 " ), start );

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
      return dbRes.GetInt( 0 );
  }
  return 0;
}

// -------------------------------------------------------------------------------- //
wxString guDbLibrary::GetSongsName( const int start )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxT( "SELECT song_name FROM songs " );
  if( GetFiltersCount() )
  {
    query += ( query.Find( wxT( "WHERE" ) ) == wxNOT_FOUND ) ? wxT( "WHERE " ) : wxT( "AND " );
    query += FiltersSQL( guLIBRARY_FILTER_SONGS );
  }
  query += GetSongsSortSQL( m_TracksOrder, m_TracksOrderDesc );
  query += wxString::Format( wxT( " LIMIT %i, 1 " ), start );

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
      return dbRes.GetString( 0 );
  }
  return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::GetTracksCounters( wxLongLong * count, wxLongLong * len, wxLongLong * size )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxT( "SELECT COUNT(), SUM(song_length), SUM(song_filesize) FROM songs " );
  if( GetFiltersCount() )
  {
    query += wxT( "WHERE " ) + FiltersSQL( guLIBRARY_FILTER_SONGS );
  }

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
      * count = dbRes.GetInt64( 0 );
      * len   = dbRes.GetInt64( 1 );
      * size  = dbRes.GetInt64( 2 );
  }
  dbRes.Finalize();
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::DeleteLibraryTracks( const guTrackArray * tracks, const bool createdeleted )
{
    wxArrayInt Tracks;
    wxArrayInt Covers;
    int Index;
    int Count = tracks->Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guTrack &Track = tracks->Item( Index );
        Tracks.Add( Track.m_SongId );

        if( createdeleted )
        {
            FindDeletedFile( Track.m_FileName, true );
        }
    }
    CleanItems( Tracks, Covers );
}

// -------------------------------------------------------------------------------- //
bool guDbLibrary::FindDeletedFile( const wxString &file, const bool create )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  query = wxString::Format( wxT( "SELECT delete_id FROM deleted WHERE delete_path = '%s' LIMIT 1;" ),
            escape_query_str( file ).c_str() );
  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
    return true;
  }

  if( create )
  {
    query = wxT( "INSERT INTO deleted( delete_id, delete_path, delete_date ) " );
    query += wxString::Format( wxT( "VALUES( NULL, '%s', %lu )" ),
              escape_query_str( file ).c_str(), wxDateTime::GetTimeNow() );
    ExecuteUpdate( query );
    return true;
  }
  return false;
}

// -------------------------------------------------------------------------------- //
bool guDbLibrary::GetAlbumInfo( const int AlbumId, wxString * AlbumName, wxString * ArtistName, wxString * AlbumPath )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  bool RetVal = false;

  query = wxString::Format( wxT( "SELECT DISTINCT song_album, song_albumartist, song_artist, song_path " \
          "FROM songs WHERE song_albumid = %u LIMIT 1" ), AlbumId );

  //guLogMessage( query );

  dbRes = ExecuteQuery( query );
  if( dbRes.NextRow() )
  {
    if( AlbumName )
        * AlbumName = dbRes.GetString( 0 );
    if( ArtistName )
    {
        * ArtistName = dbRes.GetString( 1 );
        if( ArtistName->IsEmpty() )
            * ArtistName = dbRes.GetString( 2 );
    }
    if( AlbumPath )
        * AlbumPath = dbRes.GetString( 3 );
    RetVal = true;
  }
  dbRes.Finalize();
  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetAlbumCoverId( const int AlbumId )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  int RetVal = wxNOT_FOUND;

  query = wxString::Format ( wxT( "SELECT song_coverid " ) \
          wxT( "FROM songs " ) \
          wxT( "WHERE song_albumid = %u LIMIT 1;" ), AlbumId );

  dbRes = ExecuteQuery( query );
  if( dbRes.NextRow() )
  {
    RetVal = dbRes.GetInt( 0 );
  }
  dbRes.Finalize();
  return RetVal;
}

// -------------------------------------------------------------------------------- //
guArrayListItems guDbLibrary::GetArtistsLabels( const wxArrayInt &Artists )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  guArrayListItems RetVal;
  guArrayListItem * CurItem = NULL;
  int Id;
  int index;
  int count = Artists.Count();
  if( count )
  {
      for( index = 0; index < count; index++ )
      {
          Id = Artists[ index ];
          RetVal.Add( new guArrayListItem( Id ) );
      }

      query = wxT( "SELECT settag_artistid, settag_tagid FROM settags " ) \
              wxT( "WHERE settag_artistid IN " ) + ArrayIntToStrList( Artists );
      query += wxT( " ORDER BY settag_artistid,settag_tagid;" );

      dbRes = ExecuteQuery( query );

      index = 0;
      CurItem = &RetVal[ index ];
      while( dbRes.NextRow() )
      {
        Id = dbRes.GetInt( 0 );
        if( CurItem->GetId() != Id )
        {
            index = 0;
            while( CurItem->GetId() != Id )
            {
                CurItem = &RetVal[ index ];
                index++;
            }
        }
        CurItem->AddData( dbRes.GetInt( 1 ) );
      }
      dbRes.Finalize();
  }
  return RetVal;
}

// -------------------------------------------------------------------------------- //
guArrayListItems guDbLibrary::GetAlbumsLabels( const wxArrayInt &Albums )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  guArrayListItems RetVal;
  guArrayListItem * CurItem = NULL;
  int Id;
  int index;
  int count = Albums.Count();
  for( index = 0; index < count; index++ )
  {
      Id = Albums[ index ];
      RetVal.Add( new guArrayListItem( Id ) );
  }

  query = wxT( "SELECT settag_albumid, settag_tagid FROM settags " ) \
          wxT( "WHERE settag_albumid IN " ) + ArrayIntToStrList( Albums );
  query += wxT( " ORDER BY settag_albumid,settag_tagid;" );

  dbRes = ExecuteQuery( query );

  index = 0;
  CurItem = &RetVal[ index ];
  while( dbRes.NextRow() )
  {
    Id = dbRes.GetInt( 0 );
    if( CurItem->GetId() != Id )
    {
        index = 0;
        while( CurItem->GetId() != Id )
        {
            CurItem = &RetVal[ index ];
            index++;
        }
    }
    CurItem->AddData( dbRes.GetInt( 1 ) );
  }
  dbRes.Finalize();
  return RetVal;
}

// -------------------------------------------------------------------------------- //
guArrayListItems guDbLibrary::GetSongsLabels( const wxArrayInt &Songs )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  guArrayListItems RetVal;
  guArrayListItem * CurItem = NULL;
  int Id;
  int index;
  int count = Songs.Count();
  for( index = 0; index < count; index++ )
  {
      Id = Songs[ index ];
      RetVal.Add( new guArrayListItem( Id ) );
  }

  query = wxT( "SELECT settag_songid, settag_tagid FROM settags " ) \
          wxT( "WHERE settag_songid IN " ) + ArrayIntToStrList( Songs );
  query += wxT( " ORDER BY settag_songid,settag_tagid;" );

  dbRes = ExecuteQuery( query );
  //guLogMessage( query );

  index = 0;
  CurItem = &RetVal[ index ];
  while( dbRes.NextRow() )
  {
    Id = dbRes.GetInt( 0 );
    if( CurItem->GetId() != Id )
    {
        index = 0;
        while( CurItem->GetId() != Id )
        {
            CurItem = &RetVal[ index ];
            index++;
        }
    }
    CurItem->AddData( dbRes.GetInt( 1 ) );
  }
  dbRes.Finalize();
  return RetVal;
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetArtistsLabels( const wxArrayInt &Artists, const wxArrayInt &Labels )
{
  wxString query;
  int ArIndex;
  int ArCount;
  int LaIndex;
  int LaCount;

  query = wxT( "DELETE FROM settags " ) \
          wxT( "WHERE settag_artistid IN " ) + ArrayIntToStrList( Artists );

  ExecuteUpdate( query );

  ArCount = Artists.Count();
  LaCount = Labels.Count();
  for( ArIndex = 0; ArIndex < ArCount; ArIndex++ )
  {
    for( LaIndex = 0; LaIndex < LaCount; LaIndex++ )
    {
      query = wxString::Format( wxT( "INSERT INTO settags( settag_tagid, settag_artistid ) "\
                                   "VALUES( %u, %u );" ), Labels[ LaIndex ], Artists[ ArIndex ] );
      ExecuteUpdate( query );
    }
  }

}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetAlbumsLabels( const wxArrayInt &Albums, const wxArrayInt &Labels )
{
  wxString query;
  int AlIndex;
  int AlCount;
  int TaIndex;
  int TaCount;

  query = wxT( "DELETE FROM settags " ) \
          wxT( "WHERE settag_albumid IN " ) + ArrayIntToStrList( Albums );

  ExecuteUpdate( query );

  AlCount = Albums.Count();
  for( AlIndex = 0; AlIndex < AlCount; AlIndex++ )
  {
    TaCount = Labels.Count();
    for( TaIndex = 0; TaIndex < TaCount; TaIndex++ )
    {
      query = wxString::Format( wxT( "INSERT INTO settags( settag_tagid, settag_albumid ) "\
                                   "VALUES( %u, %u );" ), Labels[ TaIndex ], Albums[ AlIndex ] );
      ExecuteUpdate( query );
    }
  }
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetSongsLabels( const wxArrayInt &Songs, const wxArrayInt &Labels )
{
  wxString query;
  int SoIndex;
  int SoCount;
  int LaIndex;
  int LaCount;

  query = wxT( "DELETE FROM settags " ) \
          wxT( "WHERE settag_songid IN " ) + ArrayIntToStrList( Songs );

  ExecuteUpdate( query );

  SoCount = Songs.Count();
  LaCount = Labels.Count();
  for( SoIndex = 0; SoIndex < SoCount; SoIndex++ )
  {
    for( LaIndex = 0; LaIndex < LaCount; LaIndex++ )
    {
      query = wxString::Format( wxT( "INSERT INTO settags( settag_tagid, settag_songid ) "\
                                   "VALUES( %u, %u );" ), Labels[ LaIndex ], Songs[ SoIndex ] );
      ExecuteUpdate( query );
    }
  }
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::UpdateArtistsLabels( const guArrayListItems &labelsets )
{
  guListItems   LaItems;
  guTrackArray  Songs;
  guTrack *     Song;
  wxString      ArtistLabelStr;

  // The ArtistLabels string is the same for all songs so done out of the loop
  GetLabels( &LaItems, true );

  int           ArIndex;
  int           ArCount = labelsets.Count();
  for( ArIndex = 0; ArIndex < ArCount; ArIndex++ )
  {
    wxArrayInt ArLabels = labelsets[ ArIndex ].GetData();
    int Index;
    int Count = ArLabels.Count();

    ArtistLabelStr.Empty();
    for( Index = 0; Index < Count; Index++ )
    {
      ArtistLabelStr += guListItemsGetName( LaItems, ArLabels[ Index ] );
      ArtistLabelStr += wxT( "|" );
    }
    if( !ArtistLabelStr.IsEmpty() )
      ArtistLabelStr.RemoveLast( 1 );

    //guLogMessage( wxT( "Artist Labels : '%s'" ), ArtistLabelStr.c_str() );
    // Update the Database
    wxArrayInt Artists;
    Artists.Add( labelsets[ ArIndex ].GetId() );
    SetArtistsLabels( Artists, ArLabels );

    if( m_MediaViewer->GetEmbeddMetadata() )
    {
      // Get the affected tracks
      GetArtistsSongs( Artists, &Songs );
      Count = Songs.Count();
      for( Index = 0; Index < Count; Index++ )
      {
        Song = &Songs[ Index ];

        if( wxFileExists( Song->m_FileName ) )
        {
          guTagInfo * TagInfo;
          TagInfo = guGetTagInfoHandler( Song->m_FileName );
          if( !TagInfo )
            continue;

          TagInfo->Read();

          TagInfo->m_ArtistLabelsStr = ArtistLabelStr;

          TagInfo->Write( guTRACK_CHANGED_DATA_LABELS );

          delete TagInfo;
        }
        else
        {
          guLogError( wxT( "The file '%s' could not be found" ), Song->m_FileName.c_str() );
        }
      }
    }
  }
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::UpdateAlbumsLabels( const guArrayListItems &labelsets )
{
  guListItems   LaItems;
  guTrackArray  Songs;
  guTrack *     Song;
  wxString      AlbumLabelStr;

  // The ArtistLabels string is the same for all songs so done out of the loop
  GetLabels( &LaItems, true );

  int           ItemIndex;
  int           ItemCount = labelsets.Count();
  for( ItemIndex = 0; ItemIndex < ItemCount; ItemIndex++ )
  {
    wxArrayInt ItemLabels = labelsets[ ItemIndex ].GetData();
    int Index;
    int Count = ItemLabels.Count();

    AlbumLabelStr.Empty();
    for( Index = 0; Index < Count; Index++ )
    {
      AlbumLabelStr += guListItemsGetName( LaItems, ItemLabels[ Index ] ) + wxT( "|" );
    }
    if( !AlbumLabelStr.IsEmpty() )
      AlbumLabelStr.RemoveLast( 1 );

    guLogMessage( wxT( "Album Labels : '%s'" ), AlbumLabelStr.c_str() );
    // Update the Database
    wxArrayInt ItemIds;
    ItemIds.Add( labelsets[ ItemIndex ].GetId() );
    SetAlbumsLabels( ItemIds, ItemLabels );

    if( m_MediaViewer->GetEmbeddMetadata() )
    {
        // Get the affected tracks
        GetAlbumsSongs( ItemIds, &Songs );

        Count = Songs.Count();
        for( Index = 0; Index < Count; Index++ )
        {
          Song = &Songs[ Index ];

          if( wxFileExists( Song->m_FileName ) )
          {
            guTagInfo * TagInfo;
            TagInfo = guGetTagInfoHandler( Song->m_FileName );
            if( !TagInfo )
              continue;

            TagInfo->Read();

            TagInfo->m_AlbumLabelsStr = AlbumLabelStr;

            TagInfo->Write( guTRACK_CHANGED_DATA_LABELS );

            delete TagInfo;
          }
          else
          {
            guLogError( wxT( "The file '%s' could not be found" ), Song->m_FileName.c_str() );
          }
        }
    }
  }
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::UpdateSongsLabels( const guArrayListItems &labelsets )
{
  guListItems   LaItems;
  guTrackArray  Songs;
  guTrack *     Song;
  wxString      TrackLabelStr;

  // The ArtistLabels string is the same for all songs so done out of the loop
  GetLabels( &LaItems, true );

  int           ItemIndex;
  int           ItemCount = labelsets.Count();
  for( ItemIndex = 0; ItemIndex < ItemCount; ItemIndex++ )
  {
    wxArrayInt ItemLabels = labelsets[ ItemIndex ].GetData();
    int Index;
    int Count = ItemLabels.Count();

    TrackLabelStr.Empty();
    for( Index = 0; Index < Count; Index++ )
    {
      TrackLabelStr += guListItemsGetName( LaItems, ItemLabels[ Index ] ) + wxT( "|" );
    }
    if( !TrackLabelStr.IsEmpty() )
      TrackLabelStr.RemoveLast( 1 );

    //guLogMessage( wxT( "Artist Labels : '%s'" ), ArtistLabelStr.c_str() );
    // Update the Database
    wxArrayInt ItemIds;
    ItemIds.Add( labelsets[ ItemIndex ].GetId() );
    SetSongsLabels( ItemIds, ItemLabels );

    if( m_MediaViewer->GetEmbeddMetadata() )
    {
        // Get the affected tracks
        GetSongs( ItemIds, &Songs );

        Count = Songs.Count();
        for( Index = 0; Index < Count; Index++ )
        {
          Song = &Songs[ Index ];

          if( Song->m_Offset )
            continue;

          if( wxFileExists( Song->m_FileName ) )
          {
            guTagInfo * TagInfo;
            TagInfo = guGetTagInfoHandler( Song->m_FileName );
            if( !TagInfo )
              continue;

            TagInfo->Read();

            TagInfo->m_TrackLabelsStr = TrackLabelStr;

            TagInfo->Write( guTRACK_CHANGED_DATA_LABELS );

            delete TagInfo;
          }
          else
          {
            guLogError( wxT( "The file '%s' could not be found" ), Song->m_FileName.c_str() );
          }
        }
    }
  }
}

// -------------------------------------------------------------------------------- //
void inline RemoveLabel( wxString * labelstr, const wxString * labelname, const wxString * newname )
{
    if( labelstr->IsEmpty() )
        return;
    if( labelstr->Replace( wxT( "|" + ( * labelname ) ), * newname ) )
        return;
    if( labelstr->Replace( ( * labelname ) + wxT( "|" ), * newname ) )
        return;
    labelstr->Replace( ( * labelname ), * newname );
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::UpdateSongLabel( const guTrack &track, const wxString &label, const wxString &newname )
{
    if( wxFileExists( track.m_FileName ) )
    {
      guTagInfo * TagInfo = guGetTagInfoHandler( track.m_FileName );
      if( !TagInfo )
        return;

      TagInfo->Read();

      RemoveLabel( &TagInfo->m_TrackLabelsStr, &label, &newname );
      RemoveLabel( &TagInfo->m_ArtistLabelsStr, &label, &newname );
      RemoveLabel( &TagInfo->m_AlbumLabelsStr, &label, &newname );

      TagInfo->Write( guTRACK_CHANGED_DATA_LABELS );

      delete TagInfo;
    }
    else
    {
        guLogError( wxT( "The file '%s' could not be found" ), track.m_FileName.c_str() );
    }
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::UpdateSongsLabel( const guTrackArray &tracks, const wxString &label, const wxString &newname )
{
    int Index;
    int Count = tracks.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        UpdateSongLabel( tracks[ Index ], label, newname );
    }
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetTrackRating( const int songid, const int rating, const bool writetags )
{
  wxString query;
  query = wxString::Format( wxT( "UPDATE songs SET song_rating = %u WHERE song_id = %u;" ),
                            rating, songid );

  ExecuteUpdate( query );

  if( writetags )
  {
  }
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetTracksRating( const wxArrayInt &songids, const int rating, const bool writetags )
{
  wxString query;
  if( songids.Count() )
  {
      query = wxString::Format( wxT( "UPDATE songs SET song_rating = %u WHERE song_id IN " ),
                                rating );
      query += ArrayIntToStrList( songids );
      ExecuteUpdate( query );

      if( writetags )
      {

      }
  }
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetTracksRating( const guTrackArray * tracks, const int rating, const bool writetags )
{
    wxArrayInt TrackIds;
    int Index;
    int Count = tracks->Count();
    for( Index = 0; Index < Count; Index++ )
    {
        TrackIds.Add( tracks->Item( Index ).m_SongId );
    }
    if( Count )
    {
        SetTracksRating( TrackIds, rating, writetags );
    }
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetTrackPlayCount( const int songid, const int playcount, const bool writetags )
{
  wxString query;
  query = wxString::Format( wxT( "UPDATE songs SET song_playcount = %u, song_lastplay = %lu WHERE song_id = %u;" ),
                            playcount, wxDateTime::GetTimeNow(), songid );
  ExecuteUpdate( query );

  if( writetags )
  {
  }
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetEmptyCovers( guCoverInfos &coverinfos )
{
    wxString query;
    wxSQLite3ResultSet dbRes;

    query = wxT( "SELECT DISTINCT song_albumid, song_album, song_artist, song_path " \
                 "FROM songs WHERE song_coverid ISNULL OR song_coverid < 1 GROUP BY song_albumid" );

    dbRes = ExecuteQuery( query );

    while( dbRes.NextRow() )
    {
        coverinfos.Add( new guCoverInfo( dbRes.GetInt( 0 ), dbRes.GetString( 1 ), dbRes.GetString( 2 ), dbRes.GetString( 3 ) ) );
    }
    dbRes.Finalize();

    return coverinfos.Count();
}

// -------------------------------------------------------------------------------- //
// AudioScrobbler Functions
// -------------------------------------------------------------------------------- //
//audioscs(
//    audiosc_id INTEGER PRIMARY KEY AUTOINCREMENT,
//    audiosc_artist VARCHAR(255),
//    audiosc_album varchar(255),
//    audiosc_track varchar(255),
//    audiosc_playedtime INTEGER,
//    audiosc_source char(1),
//    audiosc_ratting char(1),
//    audiosc_len INTEGER,
//    audiosc_tracknum INTEGER,
//    audiosc_mbtrackid INTEGER
// -------------------------------------------------------------------------------- //
bool guDbLibrary::AddCachedPlayedSong( const guCurrentTrack &Song )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  wxString Title = Song.m_SongName;
  wxString Artist = Song.m_ArtistName;
  wxString Album = Song.m_AlbumName;
  char Source;

  if( Song.m_Type == guTRACK_TYPE_RADIOSTATION )
  {
    Source = 'R';
  }
  else
  {
    switch( Song.m_TrackMode )
    {
      case guTRACK_MODE_SMART :
        Source = 'L';
        break;
      case guTRACK_MODE_USER :
        Source = 'P';
        break;
      case guTRACK_MODE_RANDOM :
        Source = 'S';
        break;
      default :
        Source = 'U';
        break;
    }
  }

  char Rating;
  switch( Song.m_ASRating )
  {
      case guAS_RATING_LOVE :
        Rating = 'L';
        break;
      case guAS_RATING_BAN :
        Rating = 'B';
        break;
      case guAS_RATING_SKIP :
        Rating = 'S';
        break;
      //case guAS_RATING_NONE :
      default :
        Rating = ' ';
        break;
  }

  escape_query_str( &Title );
  escape_query_str( &Artist );
  escape_query_str( &Album );

  int PlayTime = Song.m_PlayTime / 1000;
  int TrackLength = Song.m_Length / 1000;
  if( Song.m_Type == guTRACK_TYPE_RADIOSTATION )
  {
      PlayTime = 180;
      TrackLength = 180;
  }

  guLogMessage( wxT( "Adding track at %lu played %u mseconds" ), wxGetUTCTime(), PlayTime );

  query = wxString::Format( wxT( "INSERT into audioscs( audiosc_id, audiosc_artist, " \
          "audiosc_album, audiosc_track, audiosc_playedtime, audiosc_source, " \
          "audiosc_ratting, audiosc_len, audiosc_tracknum, audiosc_mbtrackid) " \
          "VALUES( NULL, '%s', '%s', '%s', %lu, '%c', '%c', %u, %i, %u );" ),
          Artist.c_str(),
          Album.c_str(),
          Title.c_str(),
          wxGetUTCTime() - PlayTime,
          Source,
          Rating,
          TrackLength,
          Song.m_Number,
          0 );
  //guLogMessage( query );
  return ExecuteUpdate( query );
}

// -------------------------------------------------------------------------------- //
guAS_SubmitInfoArray guDbLibrary::GetCachedPlayedSongs( int MaxCount )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  guAS_SubmitInfoArray RetVal;
  guAS_SubmitInfo * PlayedSong;

  query = wxString::Format( wxT( "SELECT audiosc_id, audiosc_artist, audiosc_album, audiosc_track, " \
                      "audiosc_playedtime, audiosc_source, audiosc_ratting, audiosc_len, " \
                      "audiosc_tracknum, audiosc_mbtrackid " \
                      "FROM audioscs ORDER BY audiosc_playedtime LIMIT %u" ), MaxCount );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    PlayedSong = new guAS_SubmitInfo();
    PlayedSong->m_Id = dbRes.GetInt( 0 );
    PlayedSong->m_ArtistName = dbRes.GetString( 1 );
    PlayedSong->m_AlbumName = dbRes.GetString( 2 );
    PlayedSong->m_TrackName = dbRes.GetString( 3 );
    PlayedSong->m_PlayedTime = dbRes.GetInt( 4 );
    PlayedSong->m_Source = dbRes.GetString( 5 )[ 0 ];
    PlayedSong->m_Rating = dbRes.GetString( 6 )[ 0 ];
    PlayedSong->m_TrackLen = dbRes.GetInt( 7 );
    PlayedSong->m_TrackNum = dbRes.GetInt( 8 );
    PlayedSong->m_MBTrackId = dbRes.GetInt( 9 );
    RetVal.Add( PlayedSong );
  }
  dbRes.Finalize();

  return RetVal;
}

// -------------------------------------------------------------------------------- //
bool guDbLibrary::DeleteCachedPlayedSongs( const guAS_SubmitInfoArray &SubmitInfo )
{
  wxString query;
//  wxSQLite3ResultSet dbRes;
  int index;
  int count;

  if( ( count = SubmitInfo.Count() ) )
  {
    query = wxT( "DELETE from audioscs WHERE audiosc_id IN (" );
    for( index = 0; index < count; index++ )
    {
        query += wxString::Format( wxT( "%u," ), SubmitInfo[ index ].m_Id );
    }
    query.RemoveLast( 1 );
    query += wxT( ");" );

    if( ExecuteUpdate( query ) )
        return true;
  }
  return false;
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::UpdateTrackFileName( const wxString &oldname, const wxString &newname )
{
    int TrackId = FindTrackFile( oldname, NULL );
    //guLogMessage( wxT( "The track %s was found with id %i" ), oldname.c_str(), TrackId );
    if( TrackId )
    {
        wxString NewPath = wxPathOnly( newname );
        if( !NewPath.EndsWith( wxT( "/" ) ) )
            NewPath += wxT( "/" );

        int PathId = GetPathId( NewPath );

        wxString query = wxString::Format( wxT( "UPDATE songs SET song_filename = '%s', song_pathid = %i, song_path = '%s' WHERE song_id = %u;" ),
            escape_query_str( newname.AfterLast( '/' ) ).c_str(),
            PathId,
            escape_query_str( NewPath ).c_str(),
            TrackId );

        //guLogMessage( wxT( "Updating file: %s" ), query.c_str() );
        ExecuteUpdate( query );
    }
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::UpdatePaths( const wxString &oldpath, const wxString &newpath )
{
  wxString query;

  query = wxString::Format( wxT( "UPDATE songs SET song_path = replace( song_path, '%s', '%s' )" ),
            escape_query_str( oldpath ).c_str(), escape_query_str( newpath ).c_str() );

  //guLogMessage( wxT( "Updating path: %s" ), query.c_str() );
  ExecuteUpdate( query );

}

}

// -------------------------------------------------------------------------------- //
