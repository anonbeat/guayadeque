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
#include "DbLibrary.h"

#include "AlbumBrowser.h"
#include "Commands.h"
#include "Config.h"
#include "DynamicPlayList.h"
#include "LibUpdate.h"
#include "MainFrame.h"
#include "MD5.h"
#include "PlayerPanel.h"
#include "TagInfo.h"
#include "Utils.h"

#include <wx/mstream.h>
#include <wx/wfstream.h>

static wxString     LastAlbum       = wxT( "__Gu4y4d3qu3__" );  // Make sure its not the same.
static int          LastAlbumId;
//static int          LastAlbumCoverId;
static wxString     LastGenre       = wxT( "__Gu4y4d3qu3__" );  // Make sure its not the same.
static int          LastGenreId;
static int          LastCoverId = wxNOT_FOUND;
static wxString     LastCoverPath;
static guListItems  LastItems;
static wxString     LastPath;
static int          LastPathId;
static wxString     LastArtist = wxT( "__Gu4y4d3qu3__" );  // Make sure its not the same.
static int          LastArtistId = wxNOT_FOUND;
static wxString     LastComposer = wxT( "__Gu4y4d3qu3__" );  // Make sure its not the same.
static int          LastComposerId = wxNOT_FOUND;

//#define DBLIBRARY_SHOW_QUERIES          1

WX_DEFINE_OBJARRAY(guTrackArray);
WX_DEFINE_OBJARRAY(guListItems);
WX_DEFINE_OBJARRAY(guArrayListItems);
WX_DEFINE_OBJARRAY(guAlbumItems);
WX_DEFINE_OBJARRAY(guRadioStations);
WX_DEFINE_OBJARRAY(guCoverInfos);
WX_DEFINE_OBJARRAY(guAS_SubmitInfoArray);

#define GU_CURRENT_DBVERSION    "13"

#define GU_TRACKS_QUERYSTR   wxT( "SELECT song_id, song_name, song_genreid, song_artistid, song_albumid, song_length, "\
               "song_number, song_pathid, song_filename, song_year, "\
               "song_bitrate, song_rating, song_playcount, song_lastplay, song_addedtime, song_filesize, "\
               "song_composerid, song_comment, song_disk "\
               "FROM songs " )



// -------------------------------------------------------------------------------- //
// Various functions
// -------------------------------------------------------------------------------- //
wxString GetSongsDBNamesSQL( const guTRACKS_ORDER order );
wxString GetSongsSortSQL( const guTRACKS_ORDER order, const bool orderdesc );

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
        RetVal += wxT( "( song_name LIKE '%" ) + escape_query_str( TeFilters[ index ] ) + wxT( "%' OR " );
        RetVal += wxT( " song_artistid IN ( SELECT artist_id FROM artists WHERE artist_name LIKE '%" ) + escape_query_str( TeFilters[ index ] ) + wxT( "%' ) OR " );
        RetVal += wxT( " song_composerid IN ( SELECT composer_id FROM composers WHERE composer_name LIKE '%" ) + escape_query_str( TeFilters[ index ] ) + wxT( "%' ) OR " );
        RetVal += wxT( " song_albumid IN ( SELECT album_id FROM albums WHERE album_name LIKE '%" ) + escape_query_str( TeFilters[ index ] ) + wxT( "%' ) ) " );
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
// guDbLibrary
// -------------------------------------------------------------------------------- //
guDbLibrary::guDbLibrary() : guDb()
{
  m_NeedUpdate = false;
  CheckDbVersion();

  guConfig * Config = ( guConfig * ) guConfig::Get();
  if( Config )
  {
    m_TracksOrder = ( guTRACKS_ORDER ) Config->ReadNum( wxT( "TracksOrder" ), 0, wxT( "General" ) );
    m_TracksOrderDesc = Config->ReadBool( wxT( "TracksOrderDesc" ), false, wxT( "General" ) );
    m_AlbumsOrder = Config->ReadNum( wxT( "AlbumYearOrder" ), 0, wxT( "General" ) );
    m_StationsOrder = Config->ReadNum( wxT( "StationsOrder" ), 0, wxT( "General" ) );
    m_StationsOrderDesc = Config->ReadBool( wxT( "StationsOrderDesc" ), false, wxT( "General" ) );
    m_PodcastOrder = Config->ReadNum( wxT( "Order" ), 0, wxT( "Podcasts" ) );
    m_PodcastOrderDesc = Config->ReadBool( wxT( "OrderDesc" ), false, wxT( "Podcasts" ) );
  }
  m_RadioIsUser = 0;

  m_GeFilters.Empty();
  m_LaFilters.Empty();
  m_ArFilters.Empty();
  m_AlFilters.Empty();
  m_TeFilters.Empty();
  m_RaTeFilters.Empty();
  m_RaGeFilters.Empty();
  m_RaLaFilters.Empty();

  m_PodChFilters.Empty();

  LoadCache();
}

// -------------------------------------------------------------------------------- //
guDbLibrary::guDbLibrary( const wxString &dbname ) : guDb( dbname )
{
  m_NeedUpdate = false;
  CheckDbVersion();

  //
  guConfig * Config = ( guConfig * ) guConfig::Get();
  if( Config )
  {
    m_TracksOrder = ( guTRACKS_ORDER ) Config->ReadNum( wxT( "TracksOrder" ), 0, wxT( "General" ) );
    m_TracksOrderDesc = Config->ReadBool( wxT( "TracksOrderDesc" ), false, wxT( "General" ) );
    m_AlbumsOrder = Config->ReadNum( wxT( "AlbumYearOrder" ), 0, wxT( "General" ) );
    m_StationsOrder = Config->ReadNum( wxT( "StationsOrder" ), 0, wxT( "General" ) );
    m_StationsOrderDesc = Config->ReadBool( wxT( "StationsOrderDesc" ), false, wxT( "General" ) );
    m_PodcastOrder = Config->ReadNum( wxT( "Order" ), 0, wxT( "Podcasts" ) );
    m_PodcastOrderDesc = Config->ReadBool( wxT( "OrderDesc" ), false, wxT( "Podcasts" ) );
  }
  m_RadioIsUser = 0;

  m_GeFilters.Empty();
  m_LaFilters.Empty();
  m_ArFilters.Empty();
  m_AlFilters.Empty();
  m_TeFilters.Empty();
  m_RaTeFilters.Empty();
  m_RaGeFilters.Empty();
  m_RaLaFilters.Empty();

  m_PodChFilters.Empty();

  LoadCache();
}

// -------------------------------------------------------------------------------- //
guDbLibrary::~guDbLibrary()
{
  guConfig * Config = ( guConfig * ) guConfig::Get();
  if( Config )
  {
    Config->WriteNum( wxT( "TracksOrder" ), m_TracksOrder, wxT( "General" ) );
    Config->WriteBool( wxT( "TracksOrderDesc" ), m_TracksOrderDesc, wxT( "General" ) );
    Config->WriteNum( wxT( "StationsOrder" ), m_StationsOrder, wxT( "General" ) );
    Config->WriteBool( wxT( "StationsOrderDesc" ), m_StationsOrderDesc, wxT( "General" ) );
    Config->WriteNum( wxT( "Order" ), m_PodcastOrder, wxT( "Podcasts" ) );
    Config->WriteBool( wxT( "OrderDesc" ), m_PodcastOrderDesc, wxT( "Podcasts" ) );
    Config->WriteNum( wxT( "AlbumYearOrder" ), m_AlbumsOrder, wxT( "General" ) );
  }

  Close();
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::LoadCache( void )
{
//    Labels.Empty();
//    GetLabels( &Labels );
    m_GenresCache.Empty();
    GetGenres( &m_GenresCache, true );
    m_ArtistsCache.Empty();
    GetArtists( &m_ArtistsCache, true );
    m_ComposersCache.Empty();
    GetComposers( &m_ComposersCache, true );
    m_AlbumsCache.Empty();
    GetAlbums( &m_AlbumsCache , true );
    m_PathsCache.Empty();
    GetPaths( &m_PathsCache, true );
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
  wxArrayString LibPaths = Config->ReadAStr( wxT( "LibPath" ), wxEmptyString, wxT( "LibPaths" ) );

  CheckSymLinks( LibPaths );

  query = wxT( "SELECT DISTINCT song_id, song_filename, path_value FROM songs, paths " \
               "WHERE song_pathid = path_id;" );

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
  query = wxT( "DELETE FROM genres WHERE genre_id NOT IN ( SELECT DISTINCT song_genreid FROM songs );" );
  ExecuteUpdate( query );
  query = wxT( "DELETE FROM artists WHERE artist_id NOT IN ( SELECT DISTINCT song_artistid FROM songs );" );
  ExecuteUpdate( query );
  query = wxT( "DELETE FROM albums WHERE album_id NOT IN ( SELECT DISTINCT song_albumid FROM songs );" );
  ExecuteUpdate( query );
  query = wxT( "DELETE FROM covers WHERE cover_id NOT IN ( SELECT DISTINCT album_coverid FROM albums );" );
  ExecuteUpdate( query );
  query = wxT( "DELETE FROM paths WHERE path_id NOT IN ( SELECT DISTINCT song_pathid FROM songs );" );
  ExecuteUpdate( query );
  query = wxT( "DELETE FROM plsets WHERE plset_type = 0 AND plset_songid NOT IN ( SELECT DISTINCT song_id FROM songs );" );
  ExecuteUpdate( query );
  query = wxT( "DELETE FROM settags WHERE settag_songid NOT IN ( SELECT DISTINCT song_id FROM songs );" );
  ExecuteUpdate( query );

  LoadCache();
}

// -------------------------------------------------------------------------------- //
bool guDbLibrary::CheckDbVersion( void )
{
  wxArrayString query;
  int Index;
  int Count;
  unsigned long dbVer;

  dbVer = GetDbVersion();
  guLogMessage( wxT( "Library Db Version %u" ), dbVer );

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
        Config->WriteStr( wxT( "LastUpdate" ), wxEmptyString, wxT( "General" ) );
      }

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS genres( genre_id INTEGER PRIMARY KEY AUTOINCREMENT,genre_name varchar(255) );" ) );
      query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'genre_id' on genres (genre_id ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'genre_name' on genres (genre_name ASC);" ) );

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS albums( album_id INTEGER PRIMARY KEY AUTOINCREMENT, album_artistid INTEGER, album_pathid INTEGER, album_name varchar(255), album_coverid INTEGER );" ) );
      query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'album_id' on albums (album_id ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'album_artistid' on albums (album_artistid ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'album_pathid' on albums (album_pathid ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'album_name' on albums (album_name ASC);" ) );

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS artists( artist_id INTEGER  PRIMARY KEY AUTOINCREMENT, artist_name varchar(255) );" ) );
      query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'artist_id' on artists (artist_id ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'artist_name' on artists (artist_name ASC);" ) );

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS composers( composer_id INTEGER  PRIMARY KEY AUTOINCREMENT, composer_name varchar(512) );" ) );
      query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'composer_id' on composers (composer_id ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'composer_name' on composers (composer_name ASC);" ) );

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS paths( path_id INTEGER PRIMARY KEY AUTOINCREMENT,path_value varchar(1024) );" ) );
      query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'path_id' on paths (path_id ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'path_value' on paths (path_value ASC);" ) );

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS songs( song_id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "song_name varchar(255), song_albumid INTEGER, song_artistid INTEGER, song_genreid INTEGER, "
                      "song_filename varchar(255), song_pathid INTEGER, song_number INTEGER(3), song_disk VARCHAR, "
                      "song_year INTEGER(4), song_comment VARCHAR, song_composerid INTEGER, song_length INTEGER, "
                      "song_bitrate INTEGER, song_rating INTEGER DEFAULT -1, song_playcount INTEGER, "
                      "song_addedtime INTEGER, song_lastplay INTEGER, song_filesize INTEGER );" ) );
      query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'song_id' on songs (song_id ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'song_name' on songs (song_name ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'song_albumid' on songs (song_albumid ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'song_artistid' on songs (song_artistid ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'song_genreid' on songs (song_genreid ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'song_pathid' on songs (song_pathid ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'song_length' on songs (song_length ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'song_bitrate' on songs (song_bitrate ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'song_rating' on songs (song_rating ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'song_playcount' on songs (song_playcount ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'song_addedtime' on songs (song_addedtime ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'song_lastplay' on songs (song_lastplay ASC);" ) );
      //query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'song_composer' on songs (song_composer ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'song_composerid' on songs (song_composerid ASC);" ) );

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS tags( tag_id INTEGER  PRIMARY KEY AUTOINCREMENT, tag_name varchar(100) );" ) );
      query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'tag_id' on tags (tag_id ASC);" ) );

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS settags( settag_tagid INTEGER, settag_artistid INTEGER, settag_albumid INTEGER, settag_songid INTEGER );" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'settag_tagid' on settags (settag_tagid ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'settag_artistid' on settags (settag_artistid ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'settag_albumid' on settags (settag_albumid ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'settag_songid' on settags (settag_songid ASC);" ) );

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS playlists( playlist_id INTEGER PRIMARY KEY AUTOINCREMENT, playlist_name varchar(100), "
                      "playlist_type INTEGER(2), playlist_limited BOOLEAN, playlist_limitvalue INTEGER, playlist_limittype INTEGER(2), "
                      "playlist_sorted BOOLEAN, playlist_sorttype INTEGER(2), playlist_sortdesc BOOLEAN, playlist_anyoption BOOLEAN);" ) );
      query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'playlist_id' on playlists (playlist_id ASC);" ) );
      query.Add( wxT( "INSERT INTO playlists( playlist_id, playlist_name, playlist_type, "
                      "playlist_limited, playlist_limitvalue, playlist_limittype, "
                      "playlist_sorted, playlist_sorttype, playlist_sortdesc, playlist_anyoption ) "
                      "VALUES( NULL, 'Recent Added Tracks', 1, 0, 0, 0, 1, 11, 1, 0 );" ) );
      query.Add( wxT( "INSERT INTO playlists( playlist_id, playlist_name, playlist_type, "
                      "playlist_limited, playlist_limitvalue, playlist_limittype, "
                      "playlist_sorted, playlist_sorttype, playlist_sortdesc, playlist_anyoption ) "
                      "VALUES( NULL, 'Last Played Tracks', 1, 0, 0, 0, 1, 10, 1, 0 );" ) );
      query.Add( wxT( "INSERT INTO playlists( playlist_id, playlist_name, playlist_type, "
                      "playlist_limited, playlist_limitvalue, playlist_limittype, "
                      "playlist_sorted, playlist_sorttype, playlist_sortdesc, playlist_anyoption ) "
                      "VALUES( NULL, 'Most Rated Tracks', 1, 0, 0, 0, 0, 0, 0, 0 );" ) );

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS plsets( plset_id INTEGER PRIMARY KEY AUTOINCREMENT, plset_plid INTEGER, plset_songid INTEGER, "
                      "plset_type INTEGER(2), plset_option INTEGER(2), plset_text TEXT(255), plset_number INTEGER, plset_option2 INTEGER );" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'plset_id' on plsets (plset_id ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'plset_plid' on plsets (plset_plid ASC);" ) );
      query.Add( wxT( "INSERT INTO plsets( plset_id, plset_plid, plset_songid, "
                      "plset_type, plset_option, plset_text, plset_number, plset_option2 ) "
                      "VALUES( NULL, 1, 0, 13, 0, '', 1, 3 );" ) );
      query.Add( wxT( "INSERT INTO plsets( plset_id, plset_plid, plset_songid, "
                      "plset_type, plset_option, plset_text, plset_number, plset_option2 ) "
                      "VALUES( NULL, 2, 0, 12, 0, '', 1, 2 );" ) );
      query.Add( wxT( "INSERT INTO plsets( plset_id, plset_plid, plset_songid, "
                      "plset_type, plset_option, plset_text, plset_number, plset_option2 ) "
                      "VALUES( NULL, 3, 0, 9, 1, '', 5, 0 );" ) );

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS covers( cover_id INTEGER PRIMARY KEY AUTOINCREMENT, cover_path VARCHAR(1024), cover_thumb BLOB, cover_midsize BLOB, cover_hash VARCHAR( 32 ) );" ) );
      query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'cover_id' on covers (cover_id ASC);" ) );

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS audioscs( audiosc_id INTEGER PRIMARY KEY AUTOINCREMENT, audiosc_artist VARCHAR(255), audiosc_album varchar(255), audiosc_track varchar(255), audiosc_playedtime INTEGER, audiosc_source char(1), audiosc_ratting char(1), audiosc_len INTEGER, audiosc_tracknum INTEGER, audiosc_mbtrackid INTEGER );" ) );
      query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'audiosc_id' on audioscs (audiosc_id ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'audiosc_playedtime' on audioscs (audiosc_playedtime ASC);" ) );

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS radiogenres( radiogenre_id INTEGER PRIMARY KEY AUTOINCREMENT, radiogenre_name VARCHAR(255), radio_tunerbase VARCHAR(255) );" ) );
      query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'radiogenre_id' on radiogenres (radiogenre_id ASC);" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name ) VALUES( NULL, '60s' );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name ) VALUES( NULL, '80s' );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name ) VALUES( NULL, '90s' );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name ) VALUES( NULL, 'Alternative' );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name ) VALUES( NULL, 'Ambient' );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name ) VALUES( NULL, 'Blues' );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name ) VALUES( NULL, 'Chillout' );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name ) VALUES( NULL, 'Classical' );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name ) VALUES( NULL, 'Country' );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name ) VALUES( NULL, 'Dance' );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name ) VALUES( NULL, 'Downtempo' );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name ) VALUES( NULL, 'Easy' );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name ) VALUES( NULL, 'Electronic' );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name ) VALUES( NULL, 'Funk' );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name ) VALUES( NULL, 'House' );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name ) VALUES( NULL, 'Jazz' );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name ) VALUES( NULL, 'New Age' );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name ) VALUES( NULL, 'Oldies' );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name ) VALUES( NULL, 'Pop' );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name ) VALUES( NULL, 'Reggae' );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name ) VALUES( NULL, 'RnB' );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name ) VALUES( NULL, 'Rock' );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name ) VALUES( NULL, 'Smooth Jazz' );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name ) VALUES( NULL, 'Slow' );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name ) VALUES( NULL, 'Soundtrack' );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name ) VALUES( NULL, 'Techno' );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name ) VALUES( NULL, 'Top 40' );" ) );

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS radiostations( radiostation_id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "radiostation_scid INTEGER, radiostation_isuser INTEGER, radiostation_genreid INTEGER, "
                      "radiostation_name VARCHAR(255), radiostation_link VARCHAR(255), radiostation_type VARCHAR(32), "
                      "radiostation_br INTEGER, radiostation_lc INTEGER );" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'radiostation_id' on radiostations (radiostation_id ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'radiostation_genreid' on radiostations (radiostation_isuser,radiostation_genreid ASC);" ) );

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS radiolabels( radiolabel_id INTEGER PRIMARY KEY AUTOINCREMENT, radiolabel_name VARCHAR(255));" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'radiolabel_id' on radiolabels (radiolabel_id ASC);" ) );

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS radiosetlabels( radiosetlabel_id INTEGER PRIMARY KEY AUTOINCREMENT, radiosetlabel_labelid INTEGER, radiosetlabel_stationid INTEGER);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'radiosetlabel_id' on radiosetlabels (radiosetlabel_id ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'radiosetlabel_labelid' on radiosetlabels (radiosetlabel_labelid ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'radiosetlabel_stationidid' on radiosetlabels (radiosetlabel_stationid ASC);" ) );
    }

    case 5 :
    {
      query.Add( wxT( "CREATE TABLE IF NOT EXISTS podcastchs( podcastch_id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "podcastch_url VARCHAR, podcastch_title VARCHAR, podcastch_description VARCHAR, "
                      "podcastch_language VARCHAR, podcastch_time INTEGER, podcastch_sumary VARCHAR, "
                      "podcastch_author VARCHAR, podcastch_ownername VARCHAR, podcastch_owneremail VARCHAR, "
                      "podcastch_category VARCHAR, podcastch_image VARCHAR, podcastch_downtype INTEGER, "
                      "podcastch_downtext VARCHAR, podcastch_allowdel BOOLEAN );" ) );
      query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'podcastch_id' on podcastchs(podcastch_id ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'podcastch_title' on podcastchs(podcastch_title ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'podcastch_url' on podcastchs(podcastch_url ASC);" ) );

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS podcastitems( podcastitem_id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "podcastitem_chid INTEGER, podcastitem_title VARCHAR, podcastitem_summary VARCHAR, "
                      "podcastitem_author VARCHAR, podcastitem_enclosure VARCHAR, podcastitem_time INTEGER, "
                      "podcastitem_file VARCHAR, podcastitem_filesize INTEGER, podcastitem_length INTEGER, "
                      "podcastitem_addeddate INTEGER, podcastitem_playcount INTEGER, "
                      "podcastitem_lastplay INTEGER, podcastitem_status INTEGER );" ) );
      query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'podcastitem_id' on podcastitems(podcastitem_id ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'podcastitem_title' on podcastitems(podcastitem_title ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'podcastitem_file' on podcastitems(podcastitem_file ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'podcastitem_chid' on podcastitems(podcastitem_chid ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'podcastitem_time' on podcastitems(podcastitem_time ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'podcastitem_enclosure' on podcastitems(podcastitem_enclosure ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'podcastitem_author' on podcastitems(podcastitem_author ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'podcastitem_length' on podcastitems(podcastitem_length ASC);" ) );
    }

    case 6 :
    {
      query.Add( wxT( "DROP INDEX 'radiostation_id';" ) );
      query.Add( wxT( "DROP INDEX 'radiostation_genreid';" ) );
      query.Add( wxT( "DROP TABLE 'radiostations';" ) );
      query.Add( wxT( "CREATE TABLE IF NOT EXISTS radiostations( radiostation_id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "radiostation_scid INTEGER, radiostation_isuser INTEGER, radiostation_genreid INTEGER, "
                      "radiostation_name VARCHAR(255), radiostation_link VARCHAR(255), radiostation_type VARCHAR(32), "
                      "radiostation_br INTEGER, radiostation_lc INTEGER );" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'radiostation_id' on radiostations (radiostation_id ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'radiostation_genreid' on radiostations (radiostation_isuser,radiostation_genreid ASC);" ) );

    }

    case 7 :
    {
      query.Add( wxT( "DROP INDEX 'cover_id';" ) );
      query.Add( wxT( "DROP TABLE 'covers';" ) );
      query.Add( wxT( "CREATE TABLE IF NOT EXISTS covers( cover_id INTEGER PRIMARY KEY AUTOINCREMENT, cover_path VARCHAR(1024), cover_thumb BLOB, cover_midsize BLOB, cover_hash VARCHAR( 32 ) );" ) );
      query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'cover_id' on covers (cover_id ASC);" ) );
    }

    case 8 :
    {
      query.Add( wxT( "DELETE FROM covers;" ) );
    }

    case 9 :
    {
      if( dbVer > 4 )
      {
          query.Add( wxT( "ALTER TABLE songs ADD COLUMN song_disk VARCAHR" ) );
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
      query.Add( wxT( "CREATE TABLE IF NOT EXISTS composers( composer_id INTEGER  PRIMARY KEY AUTOINCREMENT, composer_name varchar(512) );" ) );
      query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'composer_id' on composers (composer_id ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'composer_name' on composers (composer_name ASC);" ) );

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
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'song_composerid' on songs (song_composerid ASC);" ) );
      }
      guLogMessage( wxT( "Updating database version to " GU_CURRENT_DBVERSION ) );
      query.Add( wxT( "DELETE FROM Version;" ) );
      query.Add( wxT( "INSERT INTO Version( version ) VALUES( " GU_CURRENT_DBVERSION " );" ) );
    }

  }

  Count = query.Count();
  for( Index = 0; Index < Count; Index++ )
  {
      //guLogMessage( query[ Index ] );
    ExecuteUpdate( query[ Index ] );
  }

  return true;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetGenreId( wxString &genrename )
{
  if( LastGenre == genrename )
  {
      return LastGenreId;
  }

  wxString query;
  wxSQLite3ResultSet dbRes;
  int RetVal = wxNOT_FOUND;

  LastGenre = genrename;

  query = wxString::Format( wxT( "SELECT genre_id FROM genres "
                                 "WHERE genre_name = '%s' LIMIT 1;" ),
                        escape_query_str( genrename ).c_str() );

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
    RetVal = LastGenreId = dbRes.GetInt( 0 );
  }
  else
  {
    query = wxString::Format( wxT( "INSERT INTO genres( genre_id, genre_name ) "
                                   "VALUES( NULL, '%s' );" ),
                        escape_query_str( genrename ).c_str() );
    if( ExecuteUpdate( query ) )
    {
      RetVal = LastGenreId = m_Db.GetLastRowId().GetLo();
    }
  }
  dbRes.Finalize();

  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetComposerId( wxString &composername, bool create )
{
  if( LastComposer == composername )
  {
      return LastComposerId;
  }

  wxString query;
  wxSQLite3ResultSet dbRes;
  int RetVal = wxNOT_FOUND;
//  printf( "GetArtistId\n" );

  LastComposer = composername;

  query = wxString::Format( wxT( "SELECT composer_id FROM composers "
                                 "WHERE composer_name = '%s' LIMIT 1;" ),
            escape_query_str( composername ).c_str() );

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
    RetVal = LastComposerId = dbRes.GetInt( 0 );
  }
  else if( create )
  {
    query = wxString::Format( wxT( "INSERT INTO composers( composer_id, composer_name ) "
                                   "VALUES( NULL, '%s' );" ),
            escape_query_str( composername ).c_str() );
    if( ExecuteUpdate( query ) )
    {
      RetVal = LastComposerId = m_Db.GetLastRowId().GetLo();
    }
  }
  dbRes.Finalize();

  return RetVal;
}

// -------------------------------------------------------------------------------- //
wxBitmap * guDbLibrary::GetCoverBitmap( const int coverid, const bool thumb )
{
  if( !coverid )
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

      int ImgSize = thumb ? 38 : 100;
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
    TmpImg.Rescale( 38, 38, wxIMAGE_QUALITY_HIGH );
    if( TmpImg.IsOk() )
    {
//      wxFileOutputStream FOut( wxString::Format( wxT( "/home/jrios/%s.jpg" ), coverhash.c_str() ) );
//      TmpImg.SaveFile( FOut, wxBITMAP_TYPE_JPEG );
//      FOut.Close();
      wxImage MidImg;
      MidImg.LoadFile( coverfile );
      if( MidImg.IsOk() )
      {
        MidImg.Rescale( 100, 100, wxIMAGE_QUALITY_HIGH );


        wxSQLite3Statement stmt = m_Db.PrepareStatement( wxString::Format( wxT( "INSERT INTO covers( cover_id, cover_path, cover_thumb, cover_midsize, cover_hash ) "
                             "VALUES( NULL, '%s', ?, ?, '%s' );" ), escape_query_str( coverfile ).c_str(), CoverHash.c_str() ) );
        try {
          stmt.Bind( 1, TmpImg.GetData(), TmpImg.GetWidth() * TmpImg.GetHeight() * 3 );
          stmt.Bind( 2, MidImg.GetData(), MidImg.GetWidth() * MidImg.GetHeight() * 3 );
          //guLogMessage( wxT( "%s" ), stmt.GetSQL().c_str() );
          stmt.ExecuteQuery();
        }
        catch( wxSQLite3Exception& e )
        {
          guLogMessage( wxT( "%u: %s" ),  e.GetErrorCode(), e.GetMessage().c_str() );
        }

        CoverId = m_Db.GetLastRowId().GetLo();
      }
    }
  }
  else
  {
    guLogError( wxT( "Invalid cover file '%s'" ), coverfile.c_str() );
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
    TmpImg.Rescale( 38, 38, wxIMAGE_QUALITY_HIGH );

    if( TmpImg.IsOk() )
    {
      wxImage MidImg;
      MidImg.LoadFile( coverfile );
      MidImg.Rescale( 100, 100, wxIMAGE_QUALITY_HIGH );
      if( MidImg.IsOk() )
      {
        //guLogWarning( wxT( "Cover Image w:%u h:%u " ), TmpImg.GetWidth(), TmpImg.GetHeight() );
        wxSQLite3Statement stmt = m_Db.PrepareStatement( wxString::Format(
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

// -------------------------------------------------------------------------------- //
int guDbLibrary::FindCoverFile( const wxString &dirname )
{
    wxString query;
    wxSQLite3ResultSet dbRes;
    wxDir Dir;
    wxString FileName;
    wxString CurFile;
    wxString DirName = dirname;
    int CoverId = 0;

    if( !DirName.EndsWith( wxT( "/" ) ) )
        DirName += wxT( "/" );

    Dir.Open( DirName );
    //wxSetWorkingDirectory( DirName );

    if( Dir.IsOpened() )
    {
        if( Dir.GetFirst( &FileName, wxEmptyString, wxDIR_FILES ) )
        {
            do {
                CurFile = FileName.Lower();
                //guLogMessage( wxT( "FindCoverFile: Found file '%s'" ), FileName.c_str() );
                if( SearchCoverWords( CurFile, m_CoverSearchWords ) )
                {
                    //guLogMessage( wxT( "FindCoverFile: This file have been detected as a Cover" ) );
                    if( CurFile.EndsWith( wxT( ".jpg" ) ) ||
                        CurFile.EndsWith( wxT( ".jpeg" ) ) ||
                        CurFile.EndsWith( wxT( ".png" ) ) ||
                        CurFile.EndsWith( wxT( ".bmp" ) ) ||
                        CurFile.EndsWith( wxT( ".gif" ) ) )
                    {
                        //guLogMessage( wxT( "FindCoverFile: This file looks like an image file" ) );
                        CurFile = DirName + FileName;
                        //guLogMessage( wxT( "Found Cover: %s" ), CurFile.c_str() );
                        guMD5 md5;
                        wxString CoverHash = md5.MD5File( CurFile );

                        escape_query_str( &CurFile );

                        query = wxString::Format( wxT( "SELECT cover_id, cover_path, cover_hash FROM covers " \
                                    "WHERE cover_path = '%s' LIMIT 1;" ), CurFile.c_str() );

                        dbRes = ExecuteQuery( query );

                        if( dbRes.NextRow() ) // The cover is found in the database
                        {
                            CoverId = dbRes.GetInt( 0 );
                            // Check if the file have been changed
                            if( dbRes.GetString( 2 ) != CoverHash )
                            {
                                // The cover is different. Update the thumb is needed
                                UpdateCoverFile( CoverId, DirName + FileName, CoverHash );
                            }
                        }
                        else
                        {
                            CoverId = AddCoverFile( DirName + FileName, CoverHash );
                        }
                        break;
                    }
                }
            } while( Dir.GetNext( &FileName ) );
        }
    }
    //wxSetWorkingDirectory( SavedDir );
    return CoverId;
}

// -------------------------------------------------------------------------------- //
wxString guDbLibrary::GetCoverPath( const int CoverId )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  wxString RetVal = wxEmptyString;
  int count;
  int index;
  if( LastCoverId != CoverId )
  {
    count = LastItems.Count();
    if( count )
    {
      for( index = 0; index < count; index++ )
      {
        if( LastItems[ index ].m_Id == CoverId )
        {
          LastCoverId = CoverId;
          LastCoverPath = LastItems[ index ].m_Name;
          return LastCoverPath;
        }
      }
    }

    if( count > 25 ) // MAX_CACHE_ITEMS
    {
        LastItems.RemoveAt( 0 );
    }

    query = wxString::Format( wxT( "SELECT cover_path, cover_hash FROM covers "\
                                   "WHERE cover_id = %u "\
                                   "LIMIT 1;" ), CoverId );
    dbRes = ExecuteQuery( query );
    if( dbRes.NextRow() )
    {
      RetVal = dbRes.GetString( 0 );
      // Check if the cover have been updated
      guMD5 md5;
      wxString CoverHash = md5.MD5File( RetVal );
      if( CoverHash != dbRes.GetString( 1 ) )
      {
          UpdateCoverFile( CoverId, RetVal, CoverHash );
      }
    }
    LastCoverId = CoverId;
    LastCoverPath = RetVal;
    LastItems.Add( new guListItem( CoverId, LastCoverPath ) );
    dbRes.Finalize();
  }
  else
    RetVal = LastCoverPath;
  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::SetAlbumCover( const int AlbumId, const wxString &CoverPath, const wxString &coverhash )
{
  long CoverId = 0;
  wxSQLite3ResultSet dbRes;
  wxString query;
  wxString FileName;
  int RetVal = 0;

  // Delete the actual assigned Cover
  // Find the Cover assigned to the album
  query = wxString::Format( wxT( "SELECT album_coverid FROM albums WHERE album_id = %i LIMIT 1;" ), AlbumId );
  dbRes = ExecuteQuery( query );
  if( dbRes.NextRow() )
  {
    CoverId = dbRes.GetInt( 0 );
  }
  dbRes.Finalize();

  if( CoverId > 0 )
  {
    query = wxString::Format( wxT( "DELETE FROM covers WHERE cover_id = %i;" ), CoverId );
    ExecuteUpdate( query );
  }

  if( !CoverPath.IsEmpty() )
  {
    CoverId = AddCoverFile( CoverPath, coverhash );

    query = wxString::Format( wxT( "UPDATE albums SET album_coverid = %i WHERE album_id = %i;" ), CoverId, AlbumId );
    RetVal = ExecuteUpdate( query );
  }
  else
  {
    query = wxString::Format( wxT( "UPDATE albums SET album_coverid = 0 WHERE album_id = %i;" ), AlbumId );
    ExecuteUpdate( query );
    CoverId = 0;
  }
  // Update the AlbumsCache
  guAlbumItem * AlbumItem = guAlbumItemGetItem( m_AlbumsCache, AlbumId );
  if( AlbumItem )
  {
      AlbumItem->m_CoverId = CoverId;
      AlbumItem->m_CoverPath = CoverPath;
      //guLogMessage( wxT( "Updated Album with Cover %i '%s'" ), CoverId, CoverPath.c_str() );
  }
  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetAlbumId( wxString &albumname, const int artistid, const int pathid,
    const wxString &pathname, const int coverid )
{
  //guLogMessage( wxT( "GetAlbumId : %s" ), LastAlbum.c_str() );
  if( LastAlbum == albumname )
  {
      //guLogMessage( wxT( "Album was found in cache" ) );
      return LastAlbumId;
  }

  wxString query;
  wxSQLite3ResultSet dbRes;
  int RetVal = wxNOT_FOUND;

  query = wxString::Format( wxT( "SELECT album_id, album_artistid "
                                 "FROM albums "
                                 "WHERE album_name = '%s' "
                                 "AND album_pathid = %u LIMIT 1;" ),
                        escape_query_str( albumname ).c_str(), pathid );

  //guLogMessage( wxT( "%s" ), query.c_str() );

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
    RetVal = LastAlbumId = dbRes.GetInt( 0 );

    // Now check if the artist id changed and if so update it
    if( dbRes.GetInt( 1 ) != artistid )
    {
        query = wxString::Format( wxT( "UPDATE albums SET album_artistid = %u "\
                                       "WHERE album_id = %i;" ), artistid, RetVal );
        ExecuteUpdate( query );
    }
  }
  else
  {
    query = wxString::Format( wxT( "INSERT INTO albums( album_id, album_artistid, album_pathid, album_name, album_coverid ) "\
                               "VALUES( NULL, %u, %u, '%s', %u );" ),
                    artistid, pathid,  escape_query_str( albumname ).c_str(), coverid );
    if( ExecuteUpdate( query ) )
    {
      RetVal = LastAlbumId = m_Db.GetLastRowId().GetLo();
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
  query = wxString::Format( wxT( "SELECT tag_id FROM tags "\
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
      * LabelId = m_Db.GetLastRowId().GetLo();
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

  query = wxString::Format( wxT( "SELECT path_id FROM paths WHERE path_value = '%s' LIMIT 1;" ),
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
  if( PathValue == LastPath )
  {
      return LastPathId;
  }

  wxString query;
  wxSQLite3ResultSet dbRes;
  int RetVal = 0;

  LastPath = PathValue;

  if( !PathValue.EndsWith( wxT( "/" ) ) )
    PathValue += '/';

  query = wxString::Format( wxT( "SELECT path_id FROM paths "
                                 "WHERE path_value = '%s' LIMIT 1;" ),
                    escape_query_str( PathValue ).c_str() );
  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
    RetVal = LastPathId = dbRes.GetInt( 0 );
  }
  else
  {
    query = wxString::Format( wxT( "INSERT INTO paths( path_id, path_value ) "\
                                   "VALUES( NULL, '%s' );" ),
                    escape_query_str( PathValue ).c_str() );
    if( ExecuteUpdate( query ) )
    {
      RetVal = LastPathId = m_Db.GetLastRowId().GetLo();
    }
  }
  dbRes.Finalize();

  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetSongId( wxString &filename, const int pathid )
{
  //wxSQLite3StatementBuffer query;
  wxString query;
  wxSQLite3ResultSet dbRes;
  int RetVal = wxNOT_FOUND;

//  printf( "GetSongId0\n" );

//  printf( "%u : %s\n", FileName.Length(), TextBuf );
  query = query.Format( wxT( "SELECT song_id FROM songs WHERE song_pathid = %u "
                             "AND song_filename = '%s' LIMIT 1;" ),
            pathid, escape_query_str( filename ).c_str() );
  //printf( query.ToAscii() );
  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
    RetVal = dbRes.GetInt( 0 );
  }
  else
  {
    query = query.Format( wxT( "INSERT INTO songs( song_id, song_pathid, song_playcount, song_addedtime ) "
                               "VALUES( NULL, %u, %u, %u )" ), pathid, 0, wxDateTime::GetTimeNow() );
    if( ExecuteUpdate( query ) )
    {
      RetVal = m_Db.GetLastRowId().GetLo();
    }
  }
  dbRes.Finalize();

  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetSongId( wxString &FileName, wxString &FilePath )
{
  return GetSongId( FileName, GetPathId( FilePath ) );
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::ReadFileTags( const char * filename )
{
  guTagInfo * TagInfo;

  wxString FileName( filename, wxConvUTF8 );

  TagInfo = guGetTagInfoHandler( FileName );

  if( TagInfo )
  {
      //guLogMessage( wxT( "FileName: '%s'" ), FileName.c_str() );
      if( TagInfo->Read() )
      {
          //wxString PathName = wxGetCwd();
          //guLogMessage( wxT( "FileName: %s" ), FileName.c_str() );

          wxString PathName = wxPathOnly( FileName );
          //guLogMessage( wxT( "PathName: %s" ), PathName.c_str() );
          m_CurSong.m_PathId = GetPathId( PathName );

          m_CurSong.m_ArtistId = GetArtistId( TagInfo->m_ArtistName );

          m_CurSong.m_ComposerId = GetComposerId( TagInfo->m_Composer );

          m_CurSong.m_AlbumId = GetAlbumId( TagInfo->m_AlbumName, m_CurSong.m_ArtistId, m_CurSong.m_PathId, PathName );

          m_CurSong.m_GenreId = GetGenreId( TagInfo->m_GenreName );

          m_CurSong.m_FileName = FileName.AfterLast( '/' );
          m_CurSong.m_SongName = TagInfo->m_TrackName;
          m_CurSong.m_FileSize = guGetFileSize( FileName );

          m_CurSong.m_SongId = GetSongId( m_CurSong.m_FileName, m_CurSong.m_PathId );

          m_CurSong.m_Number   = TagInfo->m_Track;
          m_CurSong.m_Year     = TagInfo->m_Year;
          m_CurSong.m_Length   = TagInfo->m_Length;
          m_CurSong.m_Bitrate  = TagInfo->m_Bitrate;
          m_CurSong.m_Rating   = -1;
          m_CurSong.m_Composer = TagInfo->m_Composer;
          m_CurSong.m_Comments = TagInfo->m_Comments;
          m_CurSong.m_Disk     = TagInfo->m_Disk;

          wxArrayInt ArrayIds;
          //
          if( TagInfo->m_TrackLabels.Count() )
          {
            ArrayIds.Add( m_CurSong.m_SongId );
            wxArrayInt TrackLabelIds = GetLabelIds( TagInfo->m_TrackLabels );
            SetSongsLabels( ArrayIds, TrackLabelIds );
          }

          if( TagInfo->m_ArtistLabels.Count() )
          {
            ArrayIds.Empty();
            ArrayIds.Add( m_CurSong.m_ArtistId );
            wxArrayInt ArtistLabelIds = GetLabelIds( TagInfo->m_ArtistLabels );
            SetArtistsLabels( ArrayIds, ArtistLabelIds );
          }

          if( TagInfo->m_AlbumLabels.Count() )
          {
            ArrayIds.Empty();
            ArrayIds.Add( m_CurSong.m_AlbumId );
            wxArrayInt AlbumLabelIds = GetLabelIds( TagInfo->m_AlbumLabels );
            SetAlbumsLabels( ArrayIds, AlbumLabelIds );
          }

          UpdateSong();

          delete TagInfo;

          return 1;
      }

      delete TagInfo;
  }

  return 0;
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::UpdateTrackLength( const int trackid, const int length )
{
    wxString query = wxString::Format( wxT( "UPDATE songs SET song_length = %i "
        "WHERE song_id = %i" ), length, trackid );
    ExecuteUpdate( query );
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::UpdateTrackBitRate( const int trackid, const int bitrate )
{
    wxString query = wxString::Format( wxT( "UPDATE songs SET song_bitrate = %i "
        "WHERE song_id = %i" ), bitrate, trackid );
    ExecuteUpdate( query );
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::UpdateSongs( guTrackArray * Songs )
{
  guTrack * Song;
  int index;
  int count = Songs->Count();
  //guMainFrame * MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();

  // Process each Track
  for( index = 0; index < count; index++ )
  {
    Song = &( * Songs )[ index ];

    if( wxFileExists( Song->m_FileName ) )
    {
        //MainFrame->SetStatusText( wxString::Format( _( "Updating track %s" ), Song->m_FileName.c_str() ) );

        //guLogMessage( wxT( "Updating FileName '%s'" ), Song->FileName.c_str() );

        //
        // Update the File Tags
        //
        guTagInfo * TagInfo = guGetTagInfoHandler( Song->m_FileName );

        if( !TagInfo )
        {
            guLogError( wxT( "There is no handler for the file '%s'" ), Song->m_FileName.c_str() );
            continue;
        }

        TagInfo->m_TrackName = Song->m_SongName;
        TagInfo->m_ArtistName = Song->m_ArtistName;
        TagInfo->m_AlbumName = Song->m_AlbumName;
        TagInfo->m_GenreName = Song->m_GenreName;
        TagInfo->m_Track = Song->m_Number;
        TagInfo->m_Year = Song->m_Year;
        TagInfo->m_Composer = Song->m_Composer;
        TagInfo->m_Comments = Song->m_Comments;
        TagInfo->m_Disk = Song->m_Disk;

        TagInfo->Write();

        delete TagInfo;
        //TagInfo = NULL;


        if( Song->m_Type == guTRACK_TYPE_DB )
        {
            //
            // Update the Library
            //
            wxString PathName;
            int      PathId;
            int      ArtistId;
            int      ComposerId;
            int      AlbumId;
            //int      CoverId;
            int      GenreId;
            //int      SongId;
            //wxString FileName;

            PathName = wxPathOnly( Song->m_FileName );

            wxSetWorkingDirectory( PathName );

            PathId = GetPathId( PathName );

            ArtistId = GetArtistId( Song->m_ArtistName );

            ComposerId = GetComposerId( Song->m_Composer );

            AlbumId = GetAlbumId( Song->m_AlbumName, ArtistId, PathId, PathName, Song->m_CoverId );

            GenreId = GetGenreId( Song->m_GenreName );

            //FileName = Song->FileName.AfterLast( '/' );
            //printf( ( wxT( "FileName: " ) + CurSong.FileName ).ToAscii() ); printf( "\n" );

            //GetSongId( &SongId, FileName, PathId );

            m_CurSong = * Song;
            m_CurSong.m_GenreId = GenreId;
            m_CurSong.m_ArtistId = ArtistId;
            m_CurSong.m_ComposerId = ComposerId;
            m_CurSong.m_AlbumId = AlbumId;
            m_CurSong.m_PathId = PathId;
            m_CurSong.m_FileName = Song->m_FileName.AfterLast( '/' );
            //CurSong.SongId = SongId;
            //CurSong.FileName.c_str(),
            //CurSong.Number
            //CurSong.Year,
            //CurSong.Length,
            UpdateSong();
        }
    }
    else
    {
        guLogMessage( wxT( "The file %s was not found for edition." ), Song->m_FileName.c_str() );
    }
    //wxSafeYield();
  }
  //MainFrame->SetStatusText( _( "Done updating the tracks" ) );

  //DoCleanUp();

//////////////////////////
  wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_LIBRARY_DOCLEANDB );
  event.SetEventObject( ( wxObject * ) this );
  wxPostEvent( wxTheApp->GetTopWindow(), event );

}

// -------------------------------------------------------------------------------- //
int guDbLibrary::UpdateSong()
{
  wxString query;
//  printf( "UpdateSong\n" );

  if( m_CurSong.m_Rating >= 0 )
  {
      query = wxString::Format( wxT( "UPDATE songs SET song_name = '%s', song_genreid = %u, "
                                 "song_artistid = %u, song_albumid = %u, song_pathid = %u, "
                                 "song_filename = '%s', song_number = %u, song_year = %u, "
                                 "song_composerid = %u, song_comment = '%s', song_disk = '%s', "
                                 "song_length = %u, song_bitrate = %u, song_rating = %i, "
                                 "song_filesize = %u WHERE song_id = %u;" ),
            escape_query_str( m_CurSong.m_SongName ).c_str(),
            m_CurSong.m_GenreId,
            m_CurSong.m_ArtistId,
            m_CurSong.m_AlbumId,
            m_CurSong.m_PathId,
            escape_query_str( m_CurSong.m_FileName ).c_str(),
            m_CurSong.m_Number,
            m_CurSong.m_Year,
            m_CurSong.m_ComposerId, //escape_query_str( m_CurSong.m_Composer ).c_str(),
            escape_query_str( m_CurSong.m_Comments ).c_str(),
            escape_query_str( m_CurSong.m_Disk ).c_str(),
            m_CurSong.m_Length,
            m_CurSong.m_Bitrate,
            m_CurSong.m_Rating,
            m_CurSong.m_FileSize,
            m_CurSong.m_SongId );
  }
  else
  {
      query = wxString::Format( wxT( "UPDATE songs SET song_name = '%s', song_genreid = %u, "\
                                 "song_artistid = %u, song_albumid = %u, song_pathid = %u, "\
                                 "song_filename = '%s', song_number = %u, song_year = %u, "\
                                 "song_composerid = %u, song_comment = '%s', song_disk = '%s', "
                                 "song_length = %u, song_bitrate = %u, song_filesize = %u WHERE song_id = %u;" ),
            escape_query_str( m_CurSong.m_SongName ).c_str(),
            m_CurSong.m_GenreId,
            m_CurSong.m_ArtistId,
            m_CurSong.m_AlbumId,
            m_CurSong.m_PathId,
            escape_query_str( m_CurSong.m_FileName ).c_str(),
            m_CurSong.m_Number,
            m_CurSong.m_Year,
            m_CurSong.m_ComposerId, //escape_query_str( m_CurSong.m_Composer ).c_str(),
            escape_query_str( m_CurSong.m_Comments ).c_str(),
            escape_query_str( m_CurSong.m_Disk ).c_str(),
            m_CurSong.m_Length,
            m_CurSong.m_Bitrate,
            m_CurSong.m_FileSize,
            m_CurSong.m_SongId );
  }
  //printf( query.ToAscii() ); printf( "\n" );
  return ExecuteUpdate( query );
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::UpdateImageFile( const char * filename )
{
  wxString              query;
  wxSQLite3ResultSet    dbRes;
  wxString              FileName;
  int                   AlbumId;
  int                   CoverId;
  wxString              CoverHash;
  wxString              CoverFile = FileName = wxString( filename, wxConvUTF8 );

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
  query = wxString::Format( wxT( "SELECT album_id FROM albums WHERE album_pathid = "
               "( SELECT path_id FROM paths WHERE path_value = '%s' ) LIMIT 1;" ), Path.c_str() );

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
    AlbumId = dbRes.GetInt( 0 );
    dbRes.Finalize();

    SetAlbumCover( AlbumId, FileName, NewCoverHash );

    return;
  }
// An image which appear to not be to any album was found. We do nothing with this
//  else
//  {
//      guLogError( wxT( "The image '%s' with no album set" ), FileName.c_str() );
//  }
  dbRes.Finalize();
}


// -------------------------------------------------------------------------------- //
void guDbLibrary::ConfigChanged( void )
{
  guConfig * Config = ( guConfig * ) guConfig::Get();
  if( Config )
  {
    SetLibPath( Config->ReadAStr( wxT( "LibPath" ), wxGetHomeDir() + wxT( "/Music" ),
                                      wxT( "LibPaths" ) ) );
    //m_AlbumsOrder = Config->ReadNum( wxT( "AlbumYearOrder" ), 0, wxT( "General" ) );
  }
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetLibPath( const wxArrayString &NewPaths )
{
  m_LibPaths = NewPaths;
  //
  guLogMessage( wxT( "Library Paths: " ) );
  for( unsigned int Index = 0; Index < m_LibPaths.Count(); Index++ )
  {
    guLogMessage( m_LibPaths[ Index ] );
  }

  // Refresh the SearchCoverWords array
  guConfig * Config = ( guConfig * ) guConfig::Get();
  if( Config )
  {
      m_CoverSearchWords = Config->ReadAStr( wxT( "Word" ), wxEmptyString, wxT( "CoverSearch" ) );
  }
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetFiltersCount() const
{
    return m_TeFilters.Count() || m_GeFilters.Count() ||
           m_LaFilters.Count() || m_ArFilters.Count() ||
           m_CoFilters.Count() || m_AlFilters.Count() ||
           m_YeFilters.Count() || m_RaFilters.Count() ||
           m_PcFilters.Count();
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

  if( Level > GULIBRARY_FILTER_LABELS )
  {
    if( m_LaFilters.Count() )
    {
      if( RetVal.Len() )
        RetVal += wxT( " AND " );
      RetVal += LabelFilterToSQL( m_LaFilters );
    }

    if( Level > GULIBRARY_FILTER_GENRES )
    {
      if( m_GeFilters.Count() )
      {
        if( RetVal.Len() )
          RetVal += wxT( " AND " );
        RetVal += ArrayToFilter( m_GeFilters, wxT( "song_genreid" ) );
      }

      if( Level > GULIBRARY_FILTER_COMPOSERS )
      {
        if( m_CoFilters.Count() )
        {
          if( RetVal.Len() )
          {
            RetVal += wxT( " AND " );
          }
          RetVal += ArrayToFilter( m_CoFilters, wxT( "song_composerid" ) );
        }

        if( Level > GULIBRARY_FILTER_ARTISTS )
        {
          if( m_ArFilters.Count() )
          {
            if( RetVal.Len() )
              RetVal += wxT( " AND " );
            RetVal += ArrayToFilter( m_ArFilters, wxT( "song_artistid" ) );
          }
          if( Level > GULIBRARY_FILTER_YEARS )
          {
            if( m_YeFilters.Count() )
            {
              if( RetVal.Len() )
              {
                RetVal += wxT( " AND " );
              }
              RetVal += ArrayToFilter( m_YeFilters, wxT( "song_year" ) );
            }

            if( Level > GULIBRARY_FILTER_ALBUMS )
            {
              if( m_AlFilters.Count() )
              {
                if( RetVal.Len() )
                  RetVal += wxT( " AND " );
                RetVal += ArrayToFilter( m_AlFilters, wxT( "song_albumid" ) );
              }

              if( Level >= GULIBRARY_FILTER_SONGS )
              {
                if( m_RaFilters.Count() )
                {
                  if( RetVal.Len() )
                  {
                    RetVal += wxT( " AND " );
                  }
                  RetVal += ArrayToFilter( m_RaFilters, wxT( "song_rating" ) );
                }
                if( m_PcFilters.Count() )
                {
                  if( RetVal.Len() )
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
      return m_Db.GetLastRowId().GetLo();
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::SetLabelName( const int labelid, const wxString &oldlabel, const wxString &newlabel )
{
  wxString query;
  guListItems   LaItems;
  guTrackArray  Tracks;
  wxArrayInt    Labels;
  Labels.Add( labelid );

  GetLabelsSongs( Labels, &Tracks );
  UpdateSongsLabel( &Tracks, oldlabel, newlabel );


  wxString LabelName = newlabel;
  escape_query_str( &LabelName );

  query = query.Format( wxT( "UPDATE tags SET tag_name = '%s' WHERE tag_id = %u;" ), LabelName.c_str(), labelid );

  if( ExecuteUpdate( query ) == 1 )
  {
    return m_Db.GetLastRowId().GetLo();
  }
  return 0;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::DelLabel( const int LabelId )
{
  guListItems   LaItems;
  wxString      query;
  guTrackArray  Tracks;
  wxArrayInt    Labels;
  Labels.Add( LabelId );

  GetLabels( &LaItems, true );

  GetLabelsSongs( Labels, &Tracks );
  UpdateSongsLabel( &Tracks, guListItemsGetName( LaItems, LabelId ), wxEmptyString );

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
  query += FullList ? wxT( "tag_id;" ) : wxT( "tag_name COLLATE NOCASE;" );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    Labels->Add( new guListItem( dbRes.GetInt( 0 ), dbRes.GetString( 1 ) ) );
  }
  dbRes.Finalize();
//  return RetVal;
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::GetRadioLabels( guListItems * Labels, const bool FullList )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxT( "SELECT radiolabel_id, radiolabel_name FROM radiolabels ORDER BY " );
  query += FullList ? wxT( "radiolabel_id;" ) : wxT( "radiolabel_name COLLATE NOCASE;" );

  //guLogMessage( query );
  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    Labels->Add( new guListItem( dbRes.GetInt( 0 ), dbRes.GetString( 1 ) ) );
  }
  dbRes.Finalize();
//  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::AddRadioLabel( wxString LabelName )
{
    wxString query;
    escape_query_str( &LabelName );

    query = wxT( "INSERT INTO radiolabels( radiolabel_id, radiolabel_name ) VALUES( NULL, '" ) +
            LabelName + wxT( "');" );

    if( ExecuteUpdate( query ) == 1 )
    {
      return m_Db.GetLastRowId().GetLo();
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::SetRadioLabelName( const int LabelId, wxString LabelName )
{
    wxString query;
    escape_query_str( &LabelName );

    query = query.Format( wxT( "UPDATE radiolabels SET radiolabel_name = '%s' WHERE radiolabel_id = %u;" ), LabelName.c_str(), LabelId );

    if( ExecuteUpdate( query ) == 1 )
    {
      return m_Db.GetLastRowId().GetLo();
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::DelRadioLabel( const int labelid )
{
  wxString query;

  query = query.Format( wxT( "DELETE FROM radiolabels WHERE radiolabel_id = %u;" ), labelid );
  if( ExecuteUpdate( query ) )
  {
    query = query.Format( wxT( "DELETE FROM radiosetlabels WHERE radiosetlabel_labelid = %u;" ), labelid );
    if( ExecuteUpdate( query ) )
    {
      return 1;
    }
  }
  return 0;
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::GetGenres( guListItems * Genres, const bool FullList )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
//  guListItems RetVal;
  //guLogMessage( wxT( "guDbLibrary::GetGenres" ) );

  //if( !GetFiltersCount() )
  if( FullList )
  {
    query = wxT( "SELECT genre_id, genre_name FROM genres ORDER BY genre_id;" );
  }
  else if( !( m_TeFilters.Count() || m_LaFilters.Count() ) )
  {
    query = wxT( "SELECT genre_id, genre_name FROM genres ORDER BY genre_name COLLATE NOCASE;" );
  }
  else
  {
    query = wxT( "SELECT DISTINCT genre_id, genre_name FROM genres,songs " ) \
            wxT( "WHERE genre_id = song_genreid AND " );
    query += FiltersSQL( GULIBRARY_FILTER_GENRES );
    query += wxT( " ORDER BY genre_name COLLATE NOCASE;" );
  }

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    Genres->Add( new guListItem( dbRes.GetInt( 0 ), dbRes.GetString( 1 ) ) );
  }
  dbRes.Finalize();
//  return RetVal;
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
    query = wxT( "SELECT artist_id, artist_name FROM artists ORDER BY artist_id;" );
  }
  else if( !( m_TeFilters.Count() || m_LaFilters.Count() || m_GeFilters.Count() || m_CoFilters.Count() ) )
  {
    query = wxT( "SELECT artist_id, artist_name FROM artists ORDER BY artist_name COLLATE NOCASE;" );
  }
  else
  {
    query = wxT( "SELECT DISTINCT artist_id, artist_name FROM artists,songs " ) \
            wxT( "WHERE artist_id = song_artistid AND " );
    query += FiltersSQL( GULIBRARY_FILTER_ARTISTS );
    query += wxT( " ORDER BY artist_name COLLATE NOCASE;" );
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

  if( FullList || !( m_TeFilters.Count() || m_LaFilters.Count() || m_ArFilters.Count() || m_CoFilters.Count() ) )
  {
    query = wxT( "SELECT DISTINCT song_year FROM songs WHERE song_year > 0 ORDER BY song_year DESC;" );
  }
  else
  {
    query = wxT( "SELECT DISTINCT song_year FROM songs " ) \
            wxT( "WHERE song_year > 0 AND " ) + FiltersSQL( GULIBRARY_FILTER_YEARS );
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

  if( FullList || !GetFiltersCount() )
  {
    query = wxT( "SELECT DISTINCT song_rating FROM songs WHERE song_rating >= 0 ORDER BY song_rating DESC;" );
  }
  else
  {
    query = wxT( "SELECT DISTINCT song_rating FROM songs " ) \
            wxT( "WHERE song_rating >= 0 AND " ) + FiltersSQL( GULIBRARY_FILTER_SONGS );
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

  if( FullList || !GetFiltersCount() )
  {
    query = wxT( "SELECT DISTINCT song_playcount FROM songs ORDER BY song_playcount;" );
  }
  else
  {
    query = wxT( "SELECT DISTINCT song_playcount FROM songs " ) \
            wxT( "WHERE " ) + FiltersSQL( GULIBRARY_FILTER_SONGS );
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
void guDbLibrary::GetComposers( guListItems * Items, const bool FullList )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  if( FullList )
  {
    query = wxT( "SELECT composer_id, composer_name FROM composers ORDER BY composer_id;" );
  }
  else if( !( m_TeFilters.Count() || m_LaFilters.Count() || m_GeFilters.Count() ) )
  {
    query = wxT( "SELECT composer_id, composer_name FROM composers WHERE composer_name > '' ORDER BY composer_name COLLATE NOCASE;" );
  }
  else
  {
    query = wxT( "SELECT DISTINCT composer_id, composer_name FROM composers, songs " ) \
            wxT( "WHERE composer_id = song_composerid AND composer_name > '' AND " ) +
            FiltersSQL( GULIBRARY_FILTER_COMPOSERS );
    query += wxT( " ORDER BY composer_name COLLATE NOCASE;" );
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

  query = wxT( "SELECT path_id, path_value FROM paths ORDER BY path_id;" );

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
  wxArrayInt            AddedAlbums;
  int                   AlbumId;
  int                   AlbumFound;
  wxString              CoverPath;
  //guLogMessage( wxT( "guDbLibrary::GetAlbums" )

  query = wxT( "SELECT DISTINCT album_id, album_name, album_artistid, album_coverid, song_year "
               "FROM albums, songs, artists WHERE album_id = song_albumid AND artist_id = song_artistid " );

  if( FullList )
  {
      query += wxT( "ORDER BY album_id" );
  }
  else
  {
    if( m_TeFilters.Count() || m_LaFilters.Count() || m_GeFilters.Count() ||
        m_ArFilters.Count() || m_CoFilters.Count() || m_YeFilters.Count() )
    {
      query += wxT( "AND " ) + FiltersSQL( GULIBRARY_FILTER_ALBUMS );
    }
    query += wxT( " ORDER BY " );

    switch( m_AlbumsOrder )
    {
        case guALBUMS_ORDER_NAME :
            query += wxT( "album_name COLLATE NOCASE, song_disk" );
            break;

        case guALBUMS_ORDER_YEAR :
            query += wxT( "song_year, album_name COLLATE NOCASE, song_disk" );
            break;

        case guALBUMS_ORDER_YEAR_REVERSE :
            query += wxT( "song_year DESC, album_name COLLATE NOCASE, song_disk" );
            break;

        case guALBUMS_ORDER_ARTIST_NAME :
            query += wxT( "artist_name COLLATE NOCASE, album_name COLLATE NOCASE, song_disk " );
            break;

        case guALBUMS_ORDER_ARTIST_YEAR :
            query += wxT( "artist_name COLLATE NOCASE, song_year, album_name COLLATE NOCASE, song_disk" );
            break;

        case guALBUMS_ORDER_ARTIST_YEAR_REVERSE :
        default :
            query += wxT( "artist_name COLLATE NOCASE, song_year DESC, album_name COLLATE NOCASE, song_disk" );
            break;
    }

  }

  //guLogMessage( wxT( "GetAlbums: %s" ), query.c_str() );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
      AlbumId = dbRes.GetInt( 0 );
      if( ( AlbumFound = AddedAlbums.Index( AlbumId ) ) == wxNOT_FOUND )
      {
        guAlbumItem * AlbumItem = new guAlbumItem();
        AlbumItem->m_Id = AlbumId; //dbRes.GetInt( 0 );
        AlbumItem->m_Name = dbRes.GetString( 1 );
        AlbumItem->m_ArtistId = dbRes.GetInt( 2 );
        AlbumItem->m_CoverId = dbRes.GetInt( 3 );
        AlbumItem->m_Thumb = GetCoverBitmap( AlbumItem->m_CoverId );
        AlbumItem->m_Year = dbRes.GetInt( 4 );
        Albums->Add( AlbumItem );
        AddedAlbums.Add( AlbumId );
      }
      else
      {
          if( ( * Albums )[ AlbumFound ].m_Year < dbRes.GetInt( 4 ) )
            ( * Albums )[ AlbumFound ].m_Year = dbRes.GetInt( 4 );
      }
  }
  dbRes.Finalize();
//  return RetVal;
}

const wxString DynPlayListToSQLQuery( guDynPlayList * playlist );

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetAlbumYear( const int albumid )
{
  wxString              query;
  wxSQLite3ResultSet    dbRes;
  int RetVal = 0;
  query = wxString::Format( wxT( "SELECT song_year FROM songs WHERE song_albumid = %i ORDER by song_year DESC LIMIT 1" ), albumid );

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
        RetVal += wxT( "( album_name LIKE '%" ) + escape_query_str( textfilters[ index ] ) + wxT( "%' OR " );
        RetVal += wxT( " artist_name LIKE '%" ) + escape_query_str( textfilters[ index ] ) + wxT( "%' ) " );
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

  if( textfilters.Count() )
  {
      query += wxT( ", albums, artists " );
  }

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
    postquery += wxT( "song_albumid = album_id AND song_artistid = artist_id AND " );
    postquery += AlbumBrowserTextFilterToSQL( textfilters );
  }

  //guLogMessage( wxT( "GetAlbumsCount:\n%s" ), ( query + postquery ).c_str() );

  dbRes = ExecuteQuery( query + postquery );

  if( dbRes.NextRow() )
  {
      RetVal = dbRes.GetInt( 0 );
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

  query = wxT( "SELECT DISTINCT album_id, album_artistid, artist_name, album_name, album_coverid FROM songs " );
  if( filter )
  {
    wxString DynQuery = DynPlayListToSQLQuery( filter );
    if( DynQuery.Find( wxT( "albums" ) ) == wxNOT_FOUND )
        query += wxT( ", albums" );
    if( DynQuery.Find( wxT( "artists" ) ) == wxNOT_FOUND )
        query += wxT( ", artists " );
    query += DynQuery;
    query += DynQuery.IsEmpty() ? wxT( " WHERE " ) : wxT( " AND " );
    query += wxT( "album_artistid = artist_id AND song_albumid = album_id " );
  }
  else
  {
    query += wxT( ", albums, artists WHERE album_artistid = artist_id AND song_albumid = album_id " );
  }

  if( textfilters.Count() )
  {
      query += wxT( " AND " ) + AlbumBrowserTextFilterToSQL( textfilters );
  }

  query += wxT( " ORDER BY " );

  switch( order )
  {
    case guALBUMS_ORDER_NAME :
      query += wxT( "album_name COLLATE NOCASE " );
      break;

    case guALBUMS_ORDER_YEAR :
      query += wxT( "song_year" );
      break;

    case guALBUMS_ORDER_YEAR_REVERSE :
      query += wxT( "song_year DESC" );
      break;

    case guALBUMS_ORDER_ARTIST_NAME :
      query += wxT( "artist_name COLLATE NOCASE, album_name COLLATE NOCASE" );
      break;

    case guALBUMS_ORDER_ARTIST_YEAR :
      query += wxT( "artist_name COLLATE NOCASE, song_year" );
      break;

    case guALBUMS_ORDER_ARTIST_YEAR_REVERSE :
    default :
      query += wxT( "artist_name COLLATE NOCASE, song_year DESC" );
      break;
  }

  query += wxString::Format( wxT( " LIMIT %i, %i" ), start, count );

  //guLogMessage( wxT( "GetAlbums:\n%s" ), query.c_str() );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
      guAlbumBrowserItem * Item = new guAlbumBrowserItem();
      Item->m_AlbumId = dbRes.GetInt( 0 ); //AlbumId;
      Item->m_ArtistId = dbRes.GetInt( 1 );
      Item->m_ArtistName = dbRes.GetString( 2 );
      Item->m_AlbumName = dbRes.GetString( 3 );
      Item->m_CoverId = dbRes.GetInt( 4 );
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

  query = wxT( "SELECT DISTINCT path_value FROM albums, paths "
               "WHERE album_pathid = path_id AND album_id IN " ) +
               ArrayIntToStrList( AlbumIds );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
      RetVal.Add( dbRes.GetString( 0 ) );
  }
  dbRes.Finalize();
  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetStaticPlayList( const wxString &name )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  int PLId = wxNOT_FOUND;

  query = wxString::Format( wxT( "SELECT playlist_id FROM playlists WHERE playlist_name = '%s' AND playlist_type = %u LIMIT 1" ),
          escape_query_str( name ).c_str(),
          GUPLAYLIST_STATIC );

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
      PLId = dbRes.GetInt( 0 );
  }
  dbRes.Finalize();
  return PLId;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::CreateStaticPlayList( const wxString &name, const wxArrayInt &songs )
{
  int PlayListId = 0;
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxString::Format( wxT( "INSERT INTO playlists( playlist_id, playlist_name, playlist_type ) VALUES( NULL, '%s', %u );" ),
          escape_query_str( name ).c_str(),
          GUPLAYLIST_STATIC );

  if( ExecuteUpdate( query ) == 1 )
  {
      PlayListId = m_Db.GetLastRowId().GetLo();
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
      query = wxString::Format( wxT( "INSERT INTO plsets( plset_id, plset_plid, plset_songid, plset_type, plset_option, plset_text, plset_option2 ) "
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

    return ExecuteUpdate( query );
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetPlayListFiles( const int plid, wxFileDataObject * files )
{
  int Count = 0;
  wxString query;
  wxSQLite3ResultSet dbRes;
  query = wxT( "SELECT song_filename, path_value FROM plsets, songs, paths "
               "WHERE plset_songid = song_id AND song_pathid = path_id AND plset_plid = " );
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
int guDbLibrary::CreateDynamicPlayList( const wxString &name, guDynPlayList * playlist )
{
  wxASSERT( playlist );

  int PlayListId = 0;
  wxString query;
  wxSQLite3ResultSet dbRes;

//  playlists( playlist_id INTEGER PRIMARY KEY AUTOINCREMENT, playlist_name varchar(100), "
//            "playlist_type INTEGER(2), playlist_limited BOOLEAN, playlist_limitvalue INTEGER, playlist_limittype INTEGER(2), "
//            "playlist_sorted BOOLEAN, playlist_sorttype INTEGER(2), playlist_sortdesc BOOLEAN, playlist_anyoption BOOLEAN);" ) );

  query = wxT( "INSERT INTO playlists( playlist_id, playlist_name, playlist_type, "
               "playlist_limited, playlist_limitvalue, playlist_limittype, "
               "playlist_sorted, playlist_sorttype, playlist_sortdesc, "
               "playlist_anyoption ) " );
  query += wxString::Format( wxT( "VALUES( NULL, '%s', %u, %u, %u, %u, %u, %u, %u, %u );" ),
          escape_query_str( name ).c_str(),
          GUPLAYLIST_DYNAMIC,
          playlist->m_Limited, playlist->m_LimitValue, playlist->m_LimitType,
          playlist->m_Sorted, playlist->m_SortType, playlist->m_SortDesc,
          playlist->m_AnyOption );

  if( ExecuteUpdate( query ) == 1 )
  {
      PlayListId = m_Db.GetLastRowId().GetLo();
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
void guDbLibrary::GetPlayLists( guListItems * PlayLists, const int type, const wxArrayString * textfilters )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxString::Format( wxT( "SELECT playlist_id, playlist_name FROM playlists "
                                 "WHERE playlist_type = %u " ), type );

  if( textfilters && textfilters->Count() )
  {
      int Index;
      int Count = textfilters->Count();
      for( Index = 0; Index < Count; Index++ )
      {
          query += wxT( "AND playlist_name LIKE '%" ) + ( * textfilters )[ Index ] + wxT( "%' " );
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
void inline guDbLibrary::FillTrackFromDb( guTrack * Song, wxSQLite3ResultSet * dbRes )
{
  Song->m_SongId     = dbRes->GetInt( 0 );
  Song->m_SongName   = dbRes->GetString( 1 );
  Song->m_GenreId    = dbRes->GetInt( 2 );
  Song->m_ArtistId   = dbRes->GetInt( 3 );
  Song->m_ArtistName = guListItemsGetName( m_ArtistsCache, Song->m_ArtistId );
  Song->m_AlbumId    = dbRes->GetInt( 4 );
  Song->m_AlbumName  = guAlbumItemsGetName( m_AlbumsCache, Song->m_AlbumId );
  Song->m_Length     = dbRes->GetInt( 5 );
  Song->m_Number     = dbRes->GetInt( 6 );
  Song->m_FileName   = guListItemsGetName( m_PathsCache, dbRes->GetInt( 7 ) ) + dbRes->GetString( 8 );
  Song->m_CoverId    = guAlbumItemsGetCoverId( m_AlbumsCache, Song->m_AlbumId );
  Song->m_GenreName  = guListItemsGetName( m_GenresCache, Song->m_GenreId );
  Song->m_Year       = dbRes->GetInt( 9 );
  Song->m_Bitrate    = dbRes->GetInt( 10 );
  Song->m_Rating     = dbRes->GetInt( 11 );
  Song->m_PlayCount  = dbRes->GetInt( 12 );
  Song->m_LastPlay   = dbRes->GetInt( 13 );
  Song->m_AddedTime  = dbRes->GetInt( 14 );
  Song->m_FileSize   = dbRes->GetInt( 15 );
  Song->m_ComposerId = dbRes->GetInt( 16 );
  Song->m_Composer   = guListItemsGetName( m_ComposersCache, Song->m_ComposerId );
  Song->m_Comments   = dbRes->GetString( 17 );
  Song->m_Disk       = dbRes->GetString( 18 );
//  guLogMessage( wxT( "Rating: %i" ), Song->m_Rating );
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
    case guDYNAMIC_FILTER_OPTION_STRING_EQUAL : // EQUAL
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
    case guDYNAMIC_FILTER_OPTION_YEAR_EQUAL :
      FmtStr = wxT( "= %u" );
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
    case guDYNAMIC_FILTER_OPTION_NUMERIC_EQUALS :
      FmtStr = wxT( "= %u" );
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
        FmtStr = wxT( ">= %u" );
        break;
    }

    case guDYNAMIC_FILTER_OPTION_DATE_BEFORE_THE_LAST : // BEFORE_THE_LAST
    {
        FmtStr = wxT( "< %u" );
        break;
    }
  }
  Time = wxDateTime::GetTimeNow() - ( value * DynPLDateOption2[ option2 ] );
  return wxString::Format( FmtStr, Time );
}

// -------------------------------------------------------------------------------- //
const wxString DynPlayListToSQLQuery( guDynPlayList * playlist )
{
  wxASSERT( playlist );

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
        if( dbNames.Find( wxT( "artists" ) ) == wxNOT_FOUND )
        {
            dbNames += wxT( ", artists " );
            if( !dbSets.IsEmpty() )
                dbSets += wxT( "AND " );
            dbSets  += wxT( "song_artistid = artist_id " );
        }
        query += wxT( "( artist_name " ) +
                 DynPLStringOption( playlist->m_Filters[ index ].m_Option,
                                    playlist->m_Filters[ index ].m_Text ) + wxT( ")" );
        break;

      case guDYNAMIC_FILTER_TYPE_ALBUM : // ALBUM
        if( dbNames.Find( wxT( "albums" ) ) == wxNOT_FOUND )
        {
            dbNames += wxT( ", albums " );
            if( !dbSets.IsEmpty() )
                dbSets += wxT( "AND " );
            dbSets  += wxT( "song_albumid = album_id " );
        }
        query += wxT( "( album_name " ) +
                 DynPLStringOption( playlist->m_Filters[ index ].m_Option,
                                    playlist->m_Filters[ index ].m_Text ) + wxT( ")" );
        break;

      case guDYNAMIC_FILTER_TYPE_GENRE : // GENRE
        if( dbNames.Find( wxT( "genres" ) ) == wxNOT_FOUND )
        {
            dbNames += wxT( ", genres " );
            if( !dbSets.IsEmpty() )
                dbSets += wxT( "AND " );
            dbSets  += wxT( "song_genreid = genre_id " );
        }
        query += wxT( "( genre_name " ) +
                 DynPLStringOption( playlist->m_Filters[ index ].m_Option,
                                    playlist->m_Filters[ index ].m_Text ) + wxT( ")" );
        break;

      case guDYNAMIC_FILTER_TYPE_LABEL : // LABEL
        if( playlist->m_Filters[ index ].m_Option == guDYNAMIC_FILTER_OPTION_LABELS_NOTSET )
        {
            if( !dbSets.IsEmpty() )
            {
                dbSets += wxT( "AND " );
            }
            query  += wxT( "( song_id NOT IN ( SELECT DISTINCT settag_songid FROM settags WHERE settag_songid > 0 ) ) " );
        }
        else
        {
            if( dbNames.Find( wxT( "tags" ) ) == wxNOT_FOUND )
                dbNames += wxT( ", tags " );
            if( dbNames.Find( wxT( "settags" ) ) == wxNOT_FOUND )
                dbNames += wxT( ", settags " );
            if( !dbSets.IsEmpty() )
            {
                dbSets += wxT( "AND " );
            }
            dbSets  += wxT( "( ( song_id = settag_songid OR "
                                "song_artistid = settag_artistid OR "
                                "song_albumid = settag_albumid ) AND "
                                "settag_tagid = tag_id ) " );

            query += wxT( "( tag_name " ) +
                     DynPLStringOption( playlist->m_Filters[ index ].m_Option,
                                        playlist->m_Filters[ index ].m_Text ) + wxT( ")" );
        }
        break;

      case guDYNAMIC_FILTER_TYPE_COMPOSER : // COMPOSER
//        query += wxT( "( song_composer " ) + DynPLStringOption( playlist->m_Filters[ index ].m_Option,
//                                                   playlist->m_Filters[ index ].m_Text ) + wxT( ")" );
        if( dbNames.Find( wxT( "composers" ) ) == wxNOT_FOUND )
        {
            dbNames += wxT( ", composers " );
            if( !dbSets.IsEmpty() )
                dbSets += wxT( "AND " );
            dbSets  += wxT( "song_composerid = composer_id " );
        }
        query += wxT( "( composer_name " ) +
                 DynPLStringOption( playlist->m_Filters[ index ].m_Option,
                                    playlist->m_Filters[ index ].m_Text ) + wxT( ")" );
        break;

      case guDYNAMIC_FILTER_TYPE_COMMENT : // COMMENT
        query += wxT( "( song_comment " ) + DynPLStringOption( playlist->m_Filters[ index ].m_Option,
                                                   playlist->m_Filters[ index ].m_Text ) + wxT( ")" );
        break;

      case guDYNAMIC_FILTER_TYPE_PATH : // PATH
        if( dbNames.Find( wxT( "paths" ) ) == wxNOT_FOUND )
        {
            dbNames += wxT( ", paths " );
            if( !dbSets.IsEmpty() )
                dbSets += wxT( "AND " );
            dbSets  += wxT( "song_pathid = path_id " );
        }

        query += wxT( "( path_value " ) +
                 DynPLStringOption( playlist->m_Filters[ index ].m_Option,
                                    playlist->m_Filters[ index ].m_Text ) + wxT( ")" );
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
                                  playlist->m_Filters[ index ].m_Number ) + wxT( ")" );
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
        query += wxT( "( song_lastplay " ) +
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

    }
  }
  // SORTING Options
  //guLogMessage( wxT( "Sort: %u %u %u" ), playlist->m_Sorted, playlist->m_SortType, playlist->m_SortDesc );
  wxString sort = wxEmptyString;
//  wxString sortquery = wxEmptyString;
  if( playlist->m_Sorted )
  {
    sort = wxT( " ORDER BY " );
    switch( playlist->m_SortType )
    {
        case guDYNAMIC_FILTER_ORDER_TITLE : sort += wxT( "song_name COLLATE NOCASE" ); break;

        case guDYNAMIC_FILTER_ORDER_ARTIST :
        {
            if( !dbNames.Contains( wxT( "artists" ) ) )
            {
                dbNames += wxT( ", artists " );
                if( !dbSets.IsEmpty() )
                    dbSets += wxT( "AND " );
                dbSets  += wxT( "song_artistid = artist_id " );
            }
            sort += wxT( "artist_name COLLATE NOCASE" );
            break;
        }

        case guDYNAMIC_FILTER_ORDER_ALBUM :
        {
            if( !dbNames.Contains( wxT( "albums" ) ) )
            {
                dbNames += wxT( ", albums " );
                if( !dbSets.IsEmpty() )
                    dbSets += wxT( "AND " );
                dbSets  += wxT( "song_albumid = album_id " );
            }
            sort += wxT( "album_name COLLATE NOCASE" );
            break;
        }

        case guDYNAMIC_FILTER_ORDER_GENRE :
        {
            if( !dbNames.Contains( wxT( "genres" ) ) )
            {
                dbNames += wxT( ", genres " );
                if( !dbSets.IsEmpty() )
                    dbSets += wxT( "AND " );
                dbSets  += wxT( "song_genreid = genre_id " );
            }
            sort += wxT( "song_genrename COLLATE NOCASE" );
            break;
        }

        case guDYNAMIC_FILTER_ORDER_COMPOSER : sort += wxT( "song_composer COLLATE NOCASE" ); break;
        case guDYNAMIC_FILTER_ORDER_YEAR : sort += wxT( "song_year" ); break;
        case guDYNAMIC_FILTER_ORDER_RATING : sort += wxT( "song_rating" ); break;
        case guDYNAMIC_FILTER_ORDER_LENGTH : sort += wxT( "song_length" ); break;
        case guDYNAMIC_FILTER_ORDER_PLAYCOUNT : sort += wxT( "song_playcount" ); break;
        case guDYNAMIC_FILTER_ORDER_LASTPLAY : sort += wxT( "song_lastplay" ); break;
        case guDYNAMIC_FILTER_ORDER_ADDEDDATE : sort += wxT( "song_addedtime" ); break;
        case guDYNAMIC_FILTER_ORDER_RANDOM : sort += wxT( "RANDOM()" ); break;
    }
    if( playlist->m_SortDesc )
        sort += wxT( " DESC" );
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
    int PlType = GetPlayListType( plid );

    if( PlType == GUPLAYLIST_STATIC )
    {
        RetVal = wxString::Format( wxT( "SELECT plset_songid FROM plsets WHERE plset_plid = %u" ), plid );
    }
    else if( PlType == GUPLAYLIST_DYNAMIC )
    {
      guDynPlayList PlayList;
      GetDynamicPlayList( plid, &PlayList );

      RetVal = wxT( "SELECT song_id FROM songs " ) + DynPlayListToSQLQuery( &PlayList );
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetPlayListSongs( const int plid, const int pltype, guTrackArray * tracks,
                    wxLongLong * len, wxLongLong * size )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  guTrack * Track;
  wxLongLong TrackLength = 0;
  wxLongLong TrackSize = 0;

  if( plid )
  {
    if( pltype == GUPLAYLIST_STATIC )
    {
      query = GU_TRACKS_QUERYSTR;
      query += wxString::Format( wxT( ", plsets WHERE plset_songid = song_id AND plset_plid = %u" ), plid );

      dbRes = ExecuteQuery( query );

      while( dbRes.NextRow() )
      {
        Track = new guTrack();
        FillTrackFromDb( Track, &dbRes );
        tracks->Add( Track );
        TrackLength += Track->m_Length;
        TrackSize += Track->m_FileSize;
      }
      dbRes.Finalize();
    }
    else //GUPLAYLIST_DYNAMIC
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
                Limit = PlayList.m_LimitValue * 60;
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

      query = GU_TRACKS_QUERYSTR + DynPlayListToSQLQuery( &PlayList );

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
void guDbLibrary::GetDynamicPlayList( const int plid, guDynPlayList * playlist )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxString::Format( wxT( "SELECT playlist_limited, playlist_limitvalue, playlist_limittype, "
               "playlist_sorted, playlist_sorttype, playlist_sortdesc, playlist_anyoption "
               "FROM playlists WHERE playlist_id = %u LIMIT 1;" ), plid );
  dbRes = ExecuteQuery( query );
  if( dbRes.NextRow() )
  {
    playlist->m_Id = plid;
    playlist->m_Limited = dbRes.GetBool( 0 );
    playlist->m_LimitValue = dbRes.GetInt( 1 );
    playlist->m_LimitType = dbRes.GetInt( 2 );
    playlist->m_Sorted = dbRes.GetBool( 3 );
    playlist->m_SortType = dbRes.GetInt( 4 );
    playlist->m_SortDesc = dbRes.GetBool( 5 );
    playlist->m_AnyOption = dbRes.GetBool( 6 );
  }
  dbRes.Finalize();

  if( playlist->m_Id )
  {
    query = wxString::Format( wxT( "SELECT plset_type, plset_option, plset_text, plset_number, plset_option2 "
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
void guDbLibrary::UpdateDynPlayList( const int plid, const guDynPlayList * playlist )
{
  wxASSERT( playlist );

  wxString query;

  query = wxString::Format( wxT( "UPDATE playlists SET "
               "playlist_limited = %u, playlist_limitvalue = %u, playlist_limittype = %u, "
               "playlist_sorted = %u, playlist_sorttype = %u, playlist_sortdesc = %u, "
               "playlist_anyoption = %u WHERE playlist_id = %u;" ),
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

    query = wxString::Format( wxT( "INSERT INTO plsets( plset_id, plset_plid, plset_songid, "
                  "plset_type, plset_option, plset_text, plset_number, plset_option2 ) "
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
    query += GetSongsDBNamesSQL( m_TracksOrder );
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
    query += GetSongsDBNamesSQL( m_TracksOrder );
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
    query += GetSongsDBNamesSQL( m_TracksOrder );
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
int guDbLibrary::GetAlbumsSongs( const wxArrayInt &Albums, guTrackArray * Songs, bool ordertoedit )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  guTrack * Song;
  if( Albums.Count() )
  {
    query = GU_TRACKS_QUERYSTR;
    query += GetSongsDBNamesSQL( m_TracksOrder );
    query += ( query.Find( wxT( "WHERE" ) ) == wxNOT_FOUND ) ? wxT( "WHERE " ) : wxT( "AND " );
    query += wxT( "song_albumid IN " ) + ArrayIntToStrList( Albums );
    if( ordertoedit )
    {
        query += wxT( " ORDER BY song_number, song_filename" );
    }
    else
    {
        query += GetSongsSortSQL( m_TracksOrder, m_TracksOrderDesc );
    }

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
    query += GetSongsDBNamesSQL( m_TracksOrder );
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
    query += GetSongsDBNamesSQL( m_TracksOrder );
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
    query += GetSongsDBNamesSQL( m_TracksOrder );
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
int guDbLibrary::GetComposersSongs( const wxArrayInt &Composers, guTrackArray * Songs )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  guTrack * Song;
  if( Composers.Count() )
  {
    query = GU_TRACKS_QUERYSTR;
    query += GetSongsDBNamesSQL( m_TracksOrder );
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
      query += wxT( " ORDER BY song_number" );
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

  query = wxString::Format( wxT( "SELECT artist_name FROM artists "\
                                 "WHERE artist_id = %u LIMIT 1;" ), ArtistId );

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
  if( LastArtist == artistname )
  {
      return LastArtistId;
  }

  wxString query;
  wxSQLite3ResultSet dbRes;
  int RetVal = wxNOT_FOUND;
//  printf( "GetArtistId\n" );

  LastArtist = artistname;

  query = wxString::Format( wxT( "SELECT artist_id FROM artists "
                                 "WHERE artist_name = '%s' LIMIT 1;" ),
            escape_query_str( artistname ).c_str() );

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
    RetVal = LastArtistId = dbRes.GetInt( 0 );
  }
  else if( create )
  {
    query = wxString::Format( wxT( "INSERT INTO artists( artist_id, artist_name ) "
                                   "VALUES( NULL, '%s' );" ),
            escape_query_str( artistname ).c_str() );
    if( ExecuteUpdate( query ) )
    {
      RetVal = LastArtistId = m_Db.GetLastRowId().GetLo();
    }
  }
  dbRes.Finalize();

  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::FindArtist( const wxString &Artist )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  wxString Param = Artist.Lower();
  escape_query_str( &Param );

  query = wxString::Format( wxT( "SELECT artist_id FROM artists WHERE LOWER(artist_name) = '%s' LIMIT 1;" ), Param.c_str() );

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
  wxString Param = Album.Lower();
  escape_query_str( &Param );

  int ArtistId = FindArtist( Artist );
  if( ArtistId != wxNOT_FOUND )
  {
    query = wxString::Format( wxT( "SELECT album_id FROM albums WHERE "
                "album_artistid = %d AND LOWER(album_name) = '%s' LIMIT 1;" ),
                ArtistId, Param.c_str() );

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
  wxString Param = Name.Lower();
  escape_query_str( &Param );

  int ArtistId = FindArtist( Artist );
  if( ArtistId != wxNOT_FOUND )
  {
    query = wxString::Format( wxT( "SELECT song_id FROM songs WHERE " )\
                wxT( "song_artistid = %d AND LOWER(song_name) = '%s' LIMIT 1;" ),
                ArtistId, Param.c_str() );

    dbRes = ExecuteQuery( query );

    if( dbRes.NextRow() )
    {
      return dbRes.GetInt( 0 );
    }
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
  wxString ArtistName = artist.Upper();
  wxString TrackName = trackname.Upper();


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


  escape_query_str( &ArtistName );
  escape_query_str( &TrackName );

  query = GU_TRACKS_QUERYSTR wxT( ", artists " );

  query += Filters;
  if( !Filters.IsEmpty() )
    query += wxT( " AND" );
  else
    query += wxT( " WHERE" );
  query += wxString::Format( wxT( " artist_id = song_artistid AND UPPER(artist_name) = '%s' AND UPPER(song_name) = '%s' LIMIT 1;" ), ArtistName.c_str(), TrackName.c_str() );

  //guLogMessage( wxT( "FindSong:\n%s" ), query.c_str() );

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
    RetVal = new guTrack();
    wxASSERT( RetVal );
    FillTrackFromDb( RetVal, &dbRes );
  }
  dbRes.Finalize();
  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::FindTrackFile( const wxString &filename, guTrack * song )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  int PathId = wxNOT_FOUND;
  int RetVal = 0;

  wxString Path = wxPathOnly( filename );
  if( !Path.EndsWith( wxT( "/" ) ) )
    Path += '/';
  escape_query_str( &Path );

  query = wxString::Format( wxT( "SELECT path_id FROM paths WHERE path_value = '%s' LIMIT 1" ), Path.c_str() );

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
      PathId = dbRes.GetInt( 0 );
  }
  dbRes.Finalize();

  if( PathId > 0 )
  {
    Path = filename.AfterLast( '/' );
    escape_query_str( &Path );

    query = GU_TRACKS_QUERYSTR +
            wxString::Format( wxT( " WHERE song_pathid = %u AND song_filename = '%s' LIMIT 1;" ),
                              PathId, Path.c_str() );
    dbRes = ExecuteQuery( query );
    if( dbRes.NextRow() )
    {
      RetVal = dbRes.GetInt( 0 );
      if( song )
      {
          FillTrackFromDb( song, &dbRes );
      }
    }
    dbRes.Finalize();
  }
  return RetVal;
}

// -------------------------------------------------------------------------------- //
wxString GetSongsDBNamesSQL( const guTRACKS_ORDER order )
{
  wxString query = wxEmptyString;
  //
  switch( order )
  {
    case guTRACKS_ORDER_YEAR :
    case guTRACKS_ORDER_DISK :
      query += wxT( ",albums WHERE song_albumid = album_id " );
      break;

    case guTRACKS_ORDER_ARTIST :
    case guTRACKS_ORDER_ALBUM :
      query += wxT( ",artists,albums WHERE song_artistid = artist_id AND song_albumid = album_id " );
      break;

    case guTRACKS_ORDER_GENRE :
      query += wxT( ",genres WHERE song_genreid = genre_id " );
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
wxString GetSongsSortSQL( const guTRACKS_ORDER order, const bool orderdesc )
{
  wxString query = wxT( " ORDER BY " );
  //
  switch( order )
  {
    case guTRACKS_ORDER_TITLE :
      query += wxT( "song_name COLLATE NOCASE" );
      break;

    case guTRACKS_ORDER_ARTIST :
      query += wxT( "artist_name COLLATE NOCASE" );
      break;

    case guTRACKS_ORDER_ALBUM :
      query += wxT( "album_name COLLATE NOCASE" );
      break;

    case guTRACKS_ORDER_GENRE :
      query += wxT( "genre_name COLLATE NOCASE" );
      break;

    case guTRACKS_ORDER_COMPOSER :
      query += wxT( "song_composer COLLATE NOCASE" );
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

  }
  //
  if( orderdesc )
    query += wxT( " DESC" );

  switch( order )
  {
    case guTRACKS_ORDER_DISK :
      query += wxT( ",album_name COLLATE NOCASE,song_number;" );
      break;

    case guTRACKS_ORDER_ARTIST :
      query += wxT( ",album_name COLLATE NOCASE,song_disk,song_number;" );
      break;

    case guTRACKS_ORDER_ALBUM :
      query += wxT( ",song_disk,album_id,song_number;" );
      break;

    case guTRACKS_ORDER_YEAR :
      query += wxT( ",album_name COLLATE NOCASE,song_disk,song_number;" );
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
  query += GetSongsDBNamesSQL( m_TracksOrder );
  query += ( query.Find( wxT( "WHERE" ) ) == wxNOT_FOUND ) ? wxT( "WHERE " ) : wxT( "AND " );
  query += wxT( "song_id IN " ) + ArrayIntToStrList( SongIds );
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
int guDbLibrary::GetSongs( guTrackArray * Songs )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  guTrack * Song;
  //guLogMessage( wxT( "guDbLibrary::GetSongs" ) );

  query = GU_TRACKS_QUERYSTR;
  query += GetSongsDBNamesSQL( m_TracksOrder );
  if( GetFiltersCount() )
  {
    query += ( query.Find( wxT( "WHERE" ) ) == wxNOT_FOUND ) ? wxT( "WHERE " ) : wxT( "AND " );
    query += FiltersSQL( GULIBRARY_FILTER_SONGS );
  }
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
void guDbLibrary::GetTracksCounters( wxLongLong * count, wxLongLong * len, wxLongLong * size )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxT( "SELECT COUNT(), SUM(song_length), SUM(song_filesize) FROM songs " );
  if( GetFiltersCount() )
  {
    query += wxT( "WHERE " ) + FiltersSQL( GULIBRARY_FILTER_SONGS );
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
void guDbLibrary::SetSongsOrder( const guTRACKS_ORDER order )
{
    if( m_TracksOrder != order )
    {
        m_TracksOrder = order;
    }
    else
    {
        m_TracksOrderDesc = !m_TracksOrderDesc;
    }
}

// -------------------------------------------------------------------------------- //
guTRACKS_ORDER guDbLibrary::GetSongsOrder( void ) const
{
    return m_TracksOrder;
}

// -------------------------------------------------------------------------------- //
bool guDbLibrary::GetSongsOrderDesc( void ) const
{
    return m_TracksOrderDesc;
}

// -------------------------------------------------------------------------------- //
bool guDbLibrary::GetAlbumInfo( const int AlbumId, wxString * AlbumName, wxString * ArtistName, wxString * AlbumPath )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  bool RetVal = false;

  query = wxString::Format ( wxT( "SELECT DISTINCT album_name, artist_name, path_value " ) \
          wxT( "FROM albums, artists, paths " ) \
          wxT( "WHERE album_id = %u AND album_artistid = artist_id AND album_pathid = path_id" ), AlbumId );

  //guLogMessage( query );

  dbRes = ExecuteQuery( query );
  if( dbRes.NextRow() )
  {
    if( AlbumName )
        * AlbumName = dbRes.GetString( 0 );
    if( ArtistName )
        * ArtistName = dbRes.GetString( 1 );
    if( AlbumPath )
        * AlbumPath = dbRes.GetString( 2 );
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

  query = wxString::Format ( wxT( "SELECT album_coverid " ) \
          wxT( "FROM albums " ) \
          wxT( "WHERE album_id = %u LIMIT 1;" ), AlbumId );

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
void guDbLibrary::UpdateArtistsLabels( const wxArrayInt &Artists, const wxArrayInt &Labels )
{
  guListItems   LaItems;
  guTrackArray  Songs;
  guTrack *     Song;
  wxString      ArtistLabelStr = wxEmptyString;
  int           index;
  int           count;

  // The ArtistLabels string is the same for all songs so done out of the loop
  GetLabels( &LaItems, true );

  count = Labels.Count();
  for( index = 0; index < count; index++ )
  {
    ArtistLabelStr += guListItemsGetName( LaItems, Labels[ index ] );
    ArtistLabelStr += wxT( "|" );
  }
  if( !ArtistLabelStr.IsEmpty() )
    ArtistLabelStr.RemoveLast( 1 );

  //guLogMessage( wxT( "Artist Labels : '%s'" ), ArtistLabelStr.c_str() );
  // Update the Database
  SetArtistsLabels( Artists, Labels );

  // Get the affected tracks
  GetArtistsSongs( Artists, &Songs );
  count = Songs.Count();
  for( index = 0; index < count; index++ )
  {
    Song = &Songs[ index ];

    if( wxFileExists( Song->m_FileName ) )
    {
      guTagInfo * TagInfo;
      TagInfo = guGetTagInfoHandler( Song->m_FileName );
      if( !TagInfo )
        continue;

      TagInfo->Read();

      TagInfo->m_ArtistLabelsStr = ArtistLabelStr;

      TagInfo->Write();

      delete TagInfo;
    }
    else
    {
        guLogError( wxT( "The file '%s' could not be found" ), Song->m_FileName.c_str() );
    }
  }
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::UpdateAlbumsLabels( const wxArrayInt &Albums, const wxArrayInt &Labels )
{
  guListItems   AlItems;
  guTrackArray  Songs;
  guTrack *     Song;
  wxString      AlbumLabelStr = wxEmptyString;
  int           index;
  int           count;

  // The ArtistLabels string is the same for all songs so done out of the loop
  GetLabels( &AlItems, true );

  count = Labels.Count();
  for( index = 0; index < count; index++ )
  {
    AlbumLabelStr += guListItemsGetName( AlItems, Labels[ index ] );
    AlbumLabelStr += wxT( "|" );
  }
  if( !AlbumLabelStr.IsEmpty() )
    AlbumLabelStr.RemoveLast( 1 );

  // Update the Database
  SetAlbumsLabels( Albums, Labels );

  // Get the affected tracks
  GetAlbumsSongs( Albums, &Songs );
  count = Songs.Count();
  for( index = 0; index < count; index++ )
  {
    Song = &Songs[ index ];

    if( wxFileExists( Song->m_FileName ) )
    {
      guTagInfo * TagInfo;
      TagInfo = guGetTagInfoHandler( Song->m_FileName );
      if( !TagInfo )
        continue;

      TagInfo->Read();

      TagInfo->m_AlbumLabelsStr = AlbumLabelStr;

      TagInfo->Write();

      delete TagInfo;
    }
    else
    {
        guLogError( wxT( "The file '%s' could not be found" ), Song->m_FileName.c_str() );
    }
  }
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::UpdateSongsLabels( const wxArrayInt &SongIds, const wxArrayInt &Labels )
{
  guListItems   LaItems;
  guTrackArray  Songs;
  guTrack *     Song;
  wxString      TrackLabelStr = wxEmptyString;
  int           index;
  int           count;

  // The ArtistLabels string is the same for all songs so done out of the loop
  GetLabels( &LaItems, true );

  count = Labels.Count();
  for( index = 0; index < count; index++ )
  {
    TrackLabelStr += guListItemsGetName( LaItems, Labels[ index ] );
    TrackLabelStr += wxT( "|" );
  }
  if( !TrackLabelStr.IsEmpty() )
    TrackLabelStr.RemoveLast( 1 );

  // Update the Database
  SetSongsLabels( SongIds, Labels );

  // Get the affected tracks
  GetSongs( SongIds, &Songs );
  count = Songs.Count();
  for( index = 0; index < count; index++ )
  {
    Song = &Songs[ index ];

    //guLogMessage( wxT( "'%s' -> '%s'" ), Song->FileName.c_str(), TrackLabelStr.c_str() );
    if( wxFileExists( Song->m_FileName ) )
    {
      guTagInfo * TagInfo;
      TagInfo = guGetTagInfoHandler( Song->m_FileName );
      if( !TagInfo )
        continue;

      TagInfo->Read();

      TagInfo->m_TrackLabelsStr = TrackLabelStr;

      TagInfo->Write();

      delete TagInfo;
    }
    else
    {
        guLogError( wxT( "The file '%s' could not be found" ), Song->m_FileName.c_str() );
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
void guDbLibrary::UpdateSongsLabel( const guTrackArray * tracks, const wxString &label, const wxString &newname )
{
  wxASSERT( tracks );
  guTrack *     Song;
  int           index;
  int           count;

  count = tracks->Count();
  for( index = 0; index < count; index++ )
  {
    Song = &( * tracks )[ index ];
    if( wxFileExists( Song->m_FileName ) )
    {
      guTagInfo * TagInfo;
      TagInfo = guGetTagInfoHandler( Song->m_FileName );
      if( !TagInfo )
        continue;

      TagInfo->Read();

      RemoveLabel( &TagInfo->m_TrackLabelsStr, &label, &newname );
      RemoveLabel( &TagInfo->m_ArtistLabelsStr, &label, &newname );
      RemoveLabel( &TagInfo->m_AlbumLabelsStr, &label, &newname );

      TagInfo->Write();

      delete TagInfo;
    }
    else
    {
        guLogError( wxT( "The file '%s' could not be found" ), Song->m_FileName.c_str() );
    }
  }
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetTrackRating( const int songid, const int rating )
{
  wxString query;
  query = wxString::Format( wxT( "UPDATE songs SET song_rating = %u WHERE song_id = %u;" ),
                            rating, songid );
  ExecuteUpdate( query );
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetTracksRating( const wxArrayInt &songids, const int rating )
{
  wxString query;
  if( songids.Count() )
  {
      query = wxString::Format( wxT( "UPDATE songs SET song_rating = %u WHERE song_id IN " ),
                                rating );
      query += ArrayIntToStrList( songids );
      ExecuteUpdate( query );
  }
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetTrackPlayCount( const int songid, const int playcount )
{
  wxString query;
  query = wxString::Format( wxT( "UPDATE songs SET song_playcount = %u, song_lastplay = %u WHERE song_id = %u;" ),
                            playcount, wxDateTime::GetTimeNow(), songid );
  ExecuteUpdate( query );
}

// -------------------------------------------------------------------------------- //
guCoverInfos guDbLibrary::GetEmptyCovers( void )
{
    wxString query;
    wxSQLite3ResultSet dbRes;
    guCoverInfos RetVal;

    query = wxT( "SELECT DISTINCT album_id, album_name, artist_name, path_value " ) \
            wxT( "FROM albums, artists, paths " ) \
            wxT( "WHERE album_artistid = artist_id AND album_pathid = path_id AND album_coverid = 0" );

    dbRes = ExecuteQuery( query );

    while( dbRes.NextRow() )
    {
        RetVal.Add( new guCoverInfo( dbRes.GetInt( 0 ), dbRes.GetString( 1 ), dbRes.GetString( 2 ), dbRes.GetString( 3 ) ) );
    }
    dbRes.Finalize();
    return RetVal;
}

// -------------------------------------------------------------------------------- //
wxString RadioTextFilterToSQL( const wxArrayString &TeFilters )
{
  long count;
  long index;
  wxString RetVal = wxEmptyString;
  if( ( count = TeFilters.Count() ) )
  {
    for( index = 0; index < count; index++ )
    {
        RetVal += wxT( "( radiogenre_name LIKE '%" ) + escape_query_str( TeFilters[ index ] ) + wxT( "%' OR " );
        RetVal += wxT( " radiostation_name LIKE '%" ) + escape_query_str( TeFilters[ index ] ) + wxT( "%' ) AND " );
    }
    RetVal = RetVal.RemoveLast( 4 );
  }
  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetRadioFiltersCount( void ) const
{
    return m_RaTeFilters.Count() | m_RaGeFilters.Count() | m_RaLaFilters.Count();
}

// -------------------------------------------------------------------------------- //
wxString guDbLibrary::RadioFiltersSQL( void )
{
  wxString query;
  wxString RetVal = wxEmptyString;

  if( m_RaTeFilters.Count() )
  {
    RetVal += RadioTextFilterToSQL( m_RaTeFilters );
  }

  return RetVal;
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::GetRadioGenresList( guListItems * RadioGenres, const wxArrayInt &GenreIds )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  //if( !AllowFilter || !GetRadioFiltersCount() )
  if( GenreIds.Count() )
  {
    query = wxT( "SELECT radiogenre_id, radiogenre_name FROM radiogenres WHERE " );
    query += ArrayToFilter( GenreIds, wxT( "radiogenre_id" ) );
    query += wxT( " ORDER BY radiogenre_name COLLATE NOCASE;" );

    //guLogMessage( query );
    dbRes = ExecuteQuery( query );

    while( dbRes.NextRow() )
    {
      RadioGenres->Add( new guListItem( dbRes.GetInt( 0 ), dbRes.GetString( 1 ) ) );
    }
    dbRes.Finalize();
  }
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::GetRadioGenres( guListItems * RadioGenres, bool AllowFilter )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  //if( !AllowFilter || !GetRadioFiltersCount() )
  if( !AllowFilter || ( !m_RaTeFilters.Count() && !m_RaLaFilters.Count() ) )
  {
    query = wxT( "SELECT radiogenre_id, radiogenre_name FROM radiogenres ORDER BY radiogenre_name COLLATE NOCASE;" );
  }
  else
  {
    query = wxT( "SELECT DISTINCT radiogenre_id, radiogenre_name FROM radiogenres, radiostations " ) \
            wxT( "WHERE radiostation_isuser = 0 AND radiogenre_id = radiostation_genreid " );
    if( m_RaTeFilters.Count() )
    {
        query += wxT( " AND " );
        query += RadioFiltersSQL();
    }
    if( m_RaLaFilters.Count() )
    {
        query += wxT( " AND radiostation_id IN ( SELECT DISTINCT radiosetlabel_stationid FROM radiosetlabels WHERE " );
        query += ArrayToFilter( m_RaLaFilters, wxT( "radiosetlabel_labelid" ) );
        query += wxT( " )" );
    }
    query += wxT( " ORDER BY radiogenre_name COLLATE NOCASE;" );
  }

  //guLogMessage( query );
  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    RadioGenres->Add( new guListItem( dbRes.GetInt( 0 ), dbRes.GetString( 1 ) ) );
  }
  dbRes.Finalize();
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::AddRadioGenre( wxString GenreName )
{
    wxString query;
    escape_query_str( &GenreName );

    query = wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name ) VALUES( NULL, '" ) +
            GenreName + wxT( "');" );

    if( ExecuteUpdate( query ) == 1 )
    {
      return m_Db.GetLastRowId().GetLo();
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::SetRadioGenreName( const int GenreId, wxString GenreName )
{
    wxString query;
    escape_query_str( &GenreName );

    query = query.Format( wxT( "UPDATE radiogenres SET radiogenre_name = '%s' WHERE radiogenre_id = %u;" ), GenreName.c_str(), GenreId );

    if( ExecuteUpdate( query ) == 1 )
    {
      return m_Db.GetLastRowId().GetLo();
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::DelRadioGenre( const int GenreId )
{
  wxString query;

  query = query.Format( wxT( "DELETE FROM radiogenres WHERE radiogenre_id = %u;" ), GenreId );
  if( ExecuteUpdate( query ) )
  {
    query = query.Format( wxT( "DELETE FROM radiostations WHERE radiostation_genreid = %u;" ), GenreId );
    if( ExecuteUpdate( query ) )
      return 1;
  }
  return 0;
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetRaTeFilters( const wxArrayString &NewTeFilters )
{
    m_RaTeFilters = NewTeFilters;
    m_RaLaFilters.Empty();
    m_RaGeFilters.Empty();
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetRadioLabelsFilters( const wxArrayInt &filters )
{
    if( filters.Index( 0 ) != wxNOT_FOUND )
    {
        m_RaLaFilters.Empty();
    }
    else
    {
        m_RaLaFilters = filters;
    }
    m_RaGeFilters.Empty();
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetRadioGenresFilters( const wxArrayInt &filters )
{
    if( filters.Index( 0 ) != wxNOT_FOUND )
    {
        m_RaGeFilters.Empty();
    }
    else
    {
        m_RaGeFilters = filters;
    }
    m_RadioIsUser = false;
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetRadioIsUserFilter( bool isuserradio )
{
    m_RadioIsUser = isuserradio;
    m_RaGeFilters.Empty();
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetRadioGenres( const wxArrayString &Genres )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  int index;
  int count;
  wxString GenreName;

  query = wxT( "DELETE FROM radiogenres;" );
  ExecuteUpdate( query );

  count = Genres.Count();
  for( index = 0; index < count; index++ )
  {
    GenreName = Genres[ index ];
    escape_query_str( &GenreName );
    query = wxString::Format( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name ) "\
                                       "VALUES( NULL, '%s' );" ), GenreName.c_str() );
    ExecuteUpdate( query );
  }
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetRadioStationsOrder( int order )
{
    if( m_StationsOrder != order )
    {
        m_StationsOrder = order;
        m_StationsOrderDesc = ( order != 0 );
    }
    else
        m_StationsOrderDesc = !m_StationsOrderDesc;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetUserRadioStations( guRadioStations * stations )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  guRadioStation * Station;

  query = wxT( "SELECT DISTINCT radiostation_name, radiostation_link "
               "FROM radiostations WHERE radiostation_isuser = 1 " );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    Station = new guRadioStation();
    Station->m_Name       = dbRes.GetString( 0 );
    Station->m_Link       = dbRes.GetString( 1 );
    stations->Add( Station );
  }
  dbRes.Finalize();
  return stations->Count();
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetRadioStations( guRadioStations * Stations )
{
  wxString query;
  wxString querydb;
  wxString subquery;
  wxSQLite3ResultSet dbRes;
  guRadioStation * Station;

  if( !GetRadioFiltersCount() )
  {
    query = wxT( "SELECT DISTINCT radiostation_name, radiostation_id, radiostation_scid, radiostation_isuser, radiostation_genreid, radiostation_link, radiostation_type, radiostation_br, radiostation_lc "\
                 "FROM radiostations WHERE " );
    query += wxString::Format( wxT( "radiostation_isuser = %u " ), m_RadioIsUser );
    query += wxT( "GROUP BY radiostation_name, radiostation_br " );
  }
  else
  {
    if( m_RadioIsUser )
    {
        //SELECT * FROM radiostations, radiosetlabels WHERE radiosetlabel_stationid = radiostation_id AND radiosetlabel_labelid IN ( 1 )
        query = wxT( "SELECT DISTINCT radiostation_name, radiostation_id, radiostation_scid, radiostation_isuser, radiostation_genreid, radiostation_link, radiostation_type, radiostation_br, radiostation_lc " );
        querydb = wxT( "FROM radiostations " );

        //else
        wxString subquery = wxEmptyString;

        if( m_RaLaFilters.Count() )
        {
            querydb += wxT( ", radiosetlabels " );
            subquery += wxT( "WHERE radiostation_id = radiosetlabel_stationid AND radiostation_isuser = 1" );
            subquery += wxT( " AND " ) + ArrayToFilter( m_RaLaFilters, wxT( "radiosetlabel_labelid" ) );
        }
        else
        {
            subquery += wxT( "WHERE radiostation_isuser = 1 " );
        }

        if( m_RaTeFilters.Count() )
        {
            querydb += wxT( ", radiogenres " );
            subquery += wxT( "AND radiostation_genreid = radiogenre_id " );
            subquery += wxT( "AND " ) + RadioFiltersSQL();
        }

        subquery += wxT( "GROUP BY radiostation_name, radiostation_br " );
        query = query + querydb + subquery;
    }
    else
    {
        //SELECT * FROM radiostations, radiosetlabels WHERE radiosetlabel_stationid = radiostation_id AND radiosetlabel_labelid IN ( 1 )
        query = wxT( "SELECT DISTINCT radiostation_name, radiostation_id, radiostation_scid, radiostation_isuser, radiostation_genreid, radiostation_link, radiostation_type, radiostation_br, radiostation_lc "\
                     "FROM radiostations, radiogenres" );

        //else
        wxString subquery = wxEmptyString;
        if( m_RaLaFilters.Count() )
        {
            query += wxT( ", radiosetlabels WHERE radiostation_genreid = radiogenre_id AND radiostation_id = radiosetlabel_stationid AND " );
            subquery += ArrayToFilter( m_RaLaFilters, wxT( "radiosetlabel_labelid" ) );
        }
        else
        {
            query += wxT( " WHERE radiostation_genreid = radiogenre_id " );
        }

        subquery += wxT( " AND radiostation_isuser = 0 " );

        if( m_RaGeFilters.Count() )
        {
            subquery += wxT( " AND " ) + ArrayToFilter( m_RaGeFilters, wxT( "radiostation_genreid" ) );
        }

        if( m_RaTeFilters.Count() )
        {
            subquery += wxT( " AND " ) + RadioFiltersSQL();
        }

        if( !subquery.IsEmpty() )
        {
            query = query + subquery;
        }

        query += wxT( "GROUP BY radiostation_name, radiostation_br " );
    }
  }

  query += wxT( " ORDER BY " );

  if( m_StationsOrder == guRADIOSTATIONS_ORDER_NAME )
    query += wxT( "radiostation_name COLLATE NOCASE" );
  else if( m_StationsOrder == guRADIOSTATIONS_ORDER_BITRATE )
    query += wxT( "radiostation_br" );
  else
    query += wxT( "radiostation_lc" );

  if( m_StationsOrderDesc )
    query += wxT( " DESC;" );

  //guLogMessage( wxT( "GetRadioStations\n%s" ), query.c_str() );
  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    Station = new guRadioStation();
    Station->m_Name       = dbRes.GetString( 0 );
    Station->m_Id         = dbRes.GetInt( 1 );
    Station->m_SCId       = dbRes.GetInt( 2 );
    Station->m_IsUser     = dbRes.GetBool( 3 );
    Station->m_GenreId    = dbRes.GetInt( 4 );
    Station->m_Link       = dbRes.GetString( 5 );
    Station->m_Type       = dbRes.GetString( 6 );
    Station->m_BitRate    = dbRes.GetInt( 7 );
    Station->m_Listeners  = dbRes.GetInt( 8 );
    Stations->Add( Station );
  }
  dbRes.Finalize();
  return Stations->Count();
}

//// -------------------------------------------------------------------------------- //
//void guDbLibrary::GetRadioCounter( wxLongLong * count )
//{
//  wxString query;
//  wxSQLite3ResultSet dbRes;
//
//  if( !GetRadioFiltersCount() )
//  {
//    query = wxT( "SELECT COUNT() FROM radiostations WHERE " );
//    query += wxString::Format( wxT( "radiostation_isuser = %u " ), m_RadioIsUser );
//  }
//  else
//  {
//    //SELECT * FROM radiostations, radiosetlabels WHERE radiosetlabel_stationid = radiostation_id AND radiosetlabel_labelid IN ( 1 )
//    query = wxT( "SELECT COUNT() FROM radiostations, radiogenres" );
//
//    //else
//    wxString subquery = wxEmptyString;
//    if( m_RaLaFilters.Count() )
//    {
//        query += wxT( ", radiosetlabels WHERE radiostation_id = radiosetlabel_stationid AND " );
//        subquery += ArrayToFilter( m_RaLaFilters, wxT( "radiosetlabel_labelid" ) );
//    }
//    else
//    {
//        query += wxT( " WHERE " );
//    }
//
//    if( !subquery.IsEmpty() )
//        subquery += wxT( " AND " );
//    subquery += wxString::Format( wxT( " radiostation_isuser = %u " ), m_RadioIsUser );
//
//    if( m_RaGeFilters.Count() )
//    {
//        //if( !subquery.IsEmpty() )
//        //{
//        //    subquery += wxT( " AND " );
//        //}
//        subquery += wxT( " AND radiostation_genreid = radiogenre_id AND " );
//        subquery += ArrayToFilter( m_RaGeFilters, wxT( "radiostation_genreid" ) );
//    }
//
//    if( m_RaTeFilters.Count() )
//    {
//        //if( !subquery.IsEmpty() )
//        //{
//            subquery += wxT( " AND " );
//        //}
//        subquery += RadioFiltersSQL();
//    }
//
//    if( !subquery.IsEmpty() )
//    {
//        query = query + subquery;
//    }
//  }
//
//  //guLogMessage( wxT( "GetRadioStations\n%s" ), query.c_str() );
//  dbRes = ExecuteQuery( query );
//
//  if( dbRes.NextRow() )
//  {
//      * count = dbRes.GetInt64( 0 );
//  }
//  dbRes.Finalize();
//}

// -------------------------------------------------------------------------------- //
int guDbLibrary::DelRadioStations( const wxArrayInt &RadioGenresIds )
{
  wxString query;
  if( RadioGenresIds.Count() )
  {
    query = wxT( "DELETE FROM radiostations WHERE " );
    query += ArrayToFilter( RadioGenresIds, wxT( "radiostation_genreid" ) );
    return ExecuteUpdate( query );
  }
  return 0;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::DelRadioStation( const int radioid )
{
  wxString query;
  query = wxString::Format( wxT( "DELETE FROM radiostations WHERE radiostation_id = %u" ), radioid );
  return ExecuteUpdate( query );
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetRadioStation( const guRadioStation * radiostation )
{
  wxString query;

  if( radiostation->m_Id != wxNOT_FOUND )
  {
    query = wxString::Format( wxT( "UPDATE radiostations SET "
                                   "radiostation_scid = %u, radiostation_isuser = %u, radiostation_genreid = %u, "
                                   "radiostation_name = '%s', radiostation_link = '%s', radiostation_type = '%s', "
                                   "radiostation_br = %u, radiostation_lc = %u WHERE radiostation_id = %u;" ),
                                   radiostation->m_SCId,
                                   radiostation->m_IsUser,
                                   radiostation->m_GenreId,
                                   escape_query_str( radiostation->m_Name ).c_str(),
                                   escape_query_str( radiostation->m_Link ).c_str(),
                                   escape_query_str( radiostation->m_Type ).c_str(),
                                   radiostation->m_BitRate,
                                   radiostation->m_Listeners,
                                   radiostation->m_Id );
  }
  else
  {
    query = wxString::Format( wxT( "INSERT INTO radiostations( radiostation_id, radiostation_scid, radiostation_isuser, radiostation_genreid, "\
                                   "radiostation_name, radiostation_link, radiostation_type, radiostation_br, radiostation_lc) "\
                                   "VALUES( NULL, %u, %u, %u, '%s', '%s', '%s', %u, %u );" ),
                                   radiostation->m_SCId,
                                   radiostation->m_IsUser,
                                   radiostation->m_GenreId,
                                   escape_query_str( radiostation->m_Name ).c_str(),
                                   escape_query_str( radiostation->m_Link ).c_str(),
                                   escape_query_str( radiostation->m_Type ).c_str(),
                                   radiostation->m_BitRate,
                                   radiostation->m_Listeners );
  }
  ExecuteUpdate( query );
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetRadioStations( const guRadioStations * RadioStations )
{
  int index;
  int count;

  if( ( count = RadioStations->Count() ) )
  {
      for( index = 0; index < count; index++ )
      {
          SetRadioStation( &RadioStations->Item( index ) );
      }
  }
}

// -------------------------------------------------------------------------------- //
bool guDbLibrary::GetRadioStation( const int id, guRadioStation * radiostation )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxT( "SELECT radiostation_id, radiostation_scid, radiostation_isuser, radiostation_genreid, radiostation_name, radiostation_link, radiostation_type, radiostation_br, radiostation_lc "\
                 "FROM radiostations WHERE " );
  query += wxString::Format( wxT( "radiostation_id = %u LIMIT 1;" ), id );

  dbRes = ExecuteQuery( query );
  if( dbRes.NextRow() )
  {
    radiostation->m_Id         = dbRes.GetInt( 0 );
    radiostation->m_SCId       = dbRes.GetInt( 1 );
    radiostation->m_IsUser     = dbRes.GetBool( 2 );
    radiostation->m_GenreId    = dbRes.GetInt( 3 );
    radiostation->m_Name       = dbRes.GetString( 4 );
    radiostation->m_Link       = dbRes.GetString( 5 );
    radiostation->m_Type       = dbRes.GetString( 6 );
    radiostation->m_BitRate    = dbRes.GetInt( 7 );
    radiostation->m_Listeners  = dbRes.GetInt( 8 );
    return true;
  }
  return false;
}

// -------------------------------------------------------------------------------- //
guArrayListItems guDbLibrary::GetStationsLabels( const wxArrayInt &Stations )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  guArrayListItems RetVal;
  guArrayListItem * CurItem = NULL;
  int Id;
  int index;
  int count = Stations.Count();
  for( index = 0; index < count; index++ )
  {
      Id = Stations[ index ];
      RetVal.Add( new guArrayListItem( Id ) );
  }

  query = wxT( "SELECT radiosetlabel_stationid, radiosetlabel_labelid FROM radiosetlabels " ) \
          wxT( "WHERE radiosetlabel_stationid IN " ) + ArrayIntToStrList( Stations );
  query += wxT( " ORDER BY radiosetlabel_stationid, radiosetlabel_labelid;" );

  //guLogMessage( query );
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
void guDbLibrary::SetRadioStationsLabels( const wxArrayInt &Stations, const wxArrayInt &Labels )
{
  wxString query;
  int StaIndex;
  int StaCount;
  int LaIndex;
  int LaCount;

  query = wxT( "DELETE FROM radiosetlabels "
               "WHERE radiosetlabel_stationid IN " ) + ArrayIntToStrList( Stations );

  ExecuteUpdate( query );

  StaCount = Stations.Count();
  LaCount = Labels.Count();
  for( StaIndex = 0; StaIndex < StaCount; StaIndex++ )
  {
    for( LaIndex = 0; LaIndex < LaCount; LaIndex++ )
    {
      query = wxString::Format( wxT( "INSERT INTO radiosetlabels( radiosetlabel_labelid, radiosetlabel_stationid ) "\
                                   "VALUES( %u, %u );" ), Labels[ LaIndex ], Stations[ StaIndex ] );
      ExecuteUpdate( query );
    }
  }
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetRadioLabelsSongs( const wxArrayInt &Labels, guTrackArray * Songs )
{
  wxString subquery;
  wxString query;
  wxSQLite3ResultSet dbRes;
//  guTrack * Song;

  if( Labels.Count() )
  {
    subquery = ArrayIntToStrList( Labels );


    dbRes.Finalize();
  }
  return Songs->Count();
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

  switch( Song.m_TrackMode )
  {
    case guTRACK_MODE_USER :
      Source = 'P';
      break;
    case guTRACK_MODE_SMART :
      Source = 'L';
      break;
    case guTRACK_MODE_RANDOM :
      Source = 'S';
      break;
    default :
      Source = 'U';
      break;
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

  query = wxString::Format( wxT( "INSERT into audioscs( audiosc_id, audiosc_artist, "\
          "audiosc_album, audiosc_track, audiosc_playedtime, audiosc_source, "\
          "audiosc_ratting, audiosc_len, audiosc_tracknum, audiosc_mbtrackid) "\
          "VALUES( NULL, '%s', '%s', '%s', %u, '%c', '%c', %u, %i, %u );" ),
          Artist.c_str(),
          Album.c_str(),
          Title.c_str(),
          wxGetUTCTime() - Song.m_Length,
          Source,
          Rating,
          Song.m_Length,
          Song.m_Number,
          0 );
  guLogMessage( query );
  return ExecuteUpdate( query );
}

// -------------------------------------------------------------------------------- //
guAS_SubmitInfoArray guDbLibrary::GetCachedPlayedSongs( int MaxCount )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  guAS_SubmitInfoArray RetVal;
  guAS_SubmitInfo * PlayedSong;

  query = wxString::Format( wxT( "SELECT audiosc_id, audiosc_artist, audiosc_album, audiosc_track, "\
                      "audiosc_playedtime, audiosc_source, audiosc_ratting, audiosc_len, "\
                      "audiosc_tracknum, audiosc_mbtrackid "\
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
wxString guDbLibrary::SearchLyric( const wxString &artist, const wxString &trackname )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  wxString RetVal = wxEmptyString;
  wxString Artist;
  wxString TrackName;

  Artist = artist;
  escape_query_str( &Artist );

  TrackName = trackname;
  escape_query_str( &TrackName );

  query = wxString::Format( wxT( "SELECT lyric_text FROM lyrics "
                                 "WHERE lyric_artist = '%s' AND lyric_title = '%s' LIMIT 1;" ),
                                 Artist.c_str(), TrackName.c_str() );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
      RetVal = dbRes.GetString( 0 );
  }
  dbRes.Finalize();

  return RetVal;
}

// -------------------------------------------------------------------------------- //
bool guDbLibrary::CreateLyric( const wxString &artist, const wxString &trackname, const wxString &text )
{
  wxString query;
  wxSQLite3ResultSet dbRes;
  wxString RetVal = wxEmptyString;
  wxString Artist;
  wxString TrackName;
  wxString Text;

  Artist = artist;
  escape_query_str( &Artist );

  TrackName = trackname;
  escape_query_str( &TrackName );

  Text = text;
  escape_query_str( &Text );

  query = wxString::Format( wxT( "INSERT INTO lyrics( lyric_id, lyric_artist, lyric_title, lyric_text ) "
                                 "VALUES( NULL, '%s', '%s', '%s' );" ),
                                 Artist.c_str(), TrackName.c_str(), Text.c_str() );

  return ExecuteUpdate( query );
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetPodcastChannels( guPodcastChannelArray * channels )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxT( "SELECT podcastch_id, podcastch_url, podcastch_title, podcastch_description, "
               "podcastch_language, podcastch_sumary, "
               "podcastch_author, podcastch_ownername, podcastch_owneremail, "
               "podcastch_category, podcastch_image, "
               "podcastch_downtype, podcastch_downtext, podcastch_allowdel "
               "FROM podcastchs" );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    guPodcastChannel * Channel = new guPodcastChannel();
    Channel->m_Id = dbRes.GetInt( 0 );
    Channel->m_Url = dbRes.GetString( 1 );
    Channel->m_Title = dbRes.GetString( 2 );
    Channel->m_Description = dbRes.GetString( 3 );
    Channel->m_Lang = dbRes.GetString( 4 );
    Channel->m_Summary = dbRes.GetString( 5 );
    Channel->m_Author = dbRes.GetString( 6 );
    Channel->m_OwnerName = dbRes.GetString( 7 );
    Channel->m_OwnerEmail = dbRes.GetString( 8 );
    Channel->m_Category = dbRes.GetString( 9 );
    Channel->m_Image = dbRes.GetString( 10 );
    Channel->m_DownloadType = dbRes.GetInt( 11 );
    Channel->m_DownloadText = dbRes.GetString( 12 );
    Channel->m_AllowDelete = dbRes.GetBool( 13 );
    channels->Add( Channel );
  }
  dbRes.Finalize();
  return channels->Count();
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SavePodcastChannel( guPodcastChannel * channel, bool onlynew )
{
  wxASSERT( channel );

  wxString query;
  int ChannelId;
  if( ( ChannelId = GetPodcastChannelUrl( channel->m_Url ) ) == wxNOT_FOUND )
  {
    query = wxString::Format( wxT( "INSERT INTO podcastchs( podcastch_id, podcastch_url, podcastch_title, "
        "podcastch_description, podcastch_language, podcastch_time, podcastch_sumary, "
        "podcastch_author, podcastch_ownername, podcastch_owneremail, "
        "podcastch_category, podcastch_image, "
        "podcastch_downtype, podcastch_downtext, podcastch_allowdel ) "
        "VALUES( NULL, '%s', '%s', "
        "'%s', '%s', 0, '%s', "
        "'%s', '%s', '%s', "
        "'%s', '%s', %u, '%s', %u );" ),
        escape_query_str( channel->m_Url ).c_str(),
        escape_query_str( channel->m_Title ).c_str(),
        escape_query_str( channel->m_Description ).c_str(),
        escape_query_str( channel->m_Lang ).c_str(),
        escape_query_str( channel->m_Summary ).c_str(),
        escape_query_str( channel->m_Author ).c_str(),
        escape_query_str( channel->m_OwnerName ).c_str(),
        escape_query_str( channel->m_OwnerEmail ).c_str(),
        escape_query_str( channel->m_Category ).c_str(),
        escape_query_str( channel->m_Image ).c_str(),
        channel->m_DownloadType,
        escape_query_str( channel->m_DownloadText ).c_str(),
        channel->m_AllowDelete );

    ExecuteUpdate( query );
    ChannelId = m_Db.GetLastRowId().GetLo();
    channel->m_Id = ChannelId;
  }
  else if( !onlynew )
  {
    query = wxString::Format( wxT( "UPDATE podcastchs "
        "SET podcastch_url = '%s', podcastch_title = '%s', "
        "podcastch_description = '%s', podcastch_language = '%s', podcastch_sumary = '%s', "
        "podcastch_author = '%s', podcastch_ownername = '%s', podcastch_owneremail = '%s', "
        "podcastch_category = '%s', podcastch_image  = '%s', "
        "podcastch_downtype = %u, podcastch_downtext = '%s', podcastch_allowdel = %u "
        "WHERE podcastch_id = %u" ),
        escape_query_str( channel->m_Url ).c_str(),
        escape_query_str( channel->m_Title ).c_str(),
        escape_query_str( channel->m_Description ).c_str(),
        escape_query_str( channel->m_Lang ).c_str(),
        escape_query_str( channel->m_Summary ).c_str(),
        escape_query_str( channel->m_Author ).c_str(),
        escape_query_str( channel->m_OwnerName ).c_str(),
        escape_query_str( channel->m_OwnerEmail ).c_str(),
        escape_query_str( channel->m_Category ).c_str(),
        escape_query_str( channel->m_Image ).c_str(),
        channel->m_DownloadType,
        escape_query_str( channel->m_DownloadText ).c_str(),
        channel->m_AllowDelete,
        channel->m_Id );
    ExecuteUpdate( query );

    ChannelId = channel->m_Id;
  }

  // Save the Items
  SavePodcastItems( ChannelId, &channel->m_Items, onlynew );

}

// -------------------------------------------------------------------------------- //
int guDbLibrary::SavePodcastChannels( guPodcastChannelArray * channels, bool onlynew )
{
    wxASSERT( channels );
    int Index;
    int Count = channels->Count();
    for( Index = 0; Index < Count; Index++ )
    {
        SavePodcastChannel( &channels->Item( Index ), onlynew );
    }
    return 1;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetPodcastChannelUrl( const wxString &url, guPodcastChannel * channel )
{
  int RetVal = wxNOT_FOUND;
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxString::Format( wxT( "SELECT podcastch_id, podcastch_url, podcastch_title, podcastch_description, "
               "podcastch_language, podcastch_sumary, "
               "podcastch_author, podcastch_ownername, podcastch_owneremail, "
               "podcastch_category, podcastch_image, "
               "podcastch_downtype, podcastch_downtext, podcastch_allowdel "
               "FROM podcastchs "
               "WHERE podcastch_url = '%s' LIMIT 1;" ),
               escape_query_str( url ).c_str() );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    RetVal = dbRes.GetInt( 0 );
    if( channel )
    {
      channel->m_Id = RetVal;
      channel->m_Url = dbRes.GetString( 1 );
      channel->m_Title = dbRes.GetString( 2 );
      channel->m_Description = dbRes.GetString( 3 );
      channel->m_Lang = dbRes.GetString( 4 );
      channel->m_Summary = dbRes.GetString( 5 );
      channel->m_Author = dbRes.GetString( 6 );
      channel->m_OwnerName = dbRes.GetString( 7 );
      channel->m_OwnerEmail = dbRes.GetString( 8 );
      channel->m_Category = dbRes.GetString( 9 );
      channel->m_Image = dbRes.GetString( 10 );
      channel->m_DownloadType = dbRes.GetInt( 11 );
      channel->m_DownloadText = dbRes.GetString( 12 );
      channel->m_AllowDelete = dbRes.GetBool( 13 );
    }
  }
  dbRes.Finalize();
  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetPodcastChannelId( const int id, guPodcastChannel * channel )
{
  int RetVal = wxNOT_FOUND;
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxString::Format( wxT( "SELECT podcastch_id, podcastch_url, podcastch_title, podcastch_description, "
               "podcastch_language, podcastch_sumary, "
               "podcastch_author, podcastch_ownername, podcastch_owneremail, "
               "podcastch_category, podcastch_image, "
               "podcastch_downtype, podcastch_downtext, podcastch_allowdel "
               "FROM podcastchs "
               "WHERE podcastch_id = %u LIMIT 1;" ),
               id );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    RetVal = dbRes.GetInt( 0 );
    if( channel )
    {
      channel->m_Id = RetVal;
      channel->m_Url = dbRes.GetString( 1 );
      channel->m_Title = dbRes.GetString( 2 );
      channel->m_Description = dbRes.GetString( 3 );
      channel->m_Lang = dbRes.GetString( 4 );
      channel->m_Summary = dbRes.GetString( 5 );
      channel->m_Author = dbRes.GetString( 6 );
      channel->m_OwnerName = dbRes.GetString( 7 );
      channel->m_OwnerEmail = dbRes.GetString( 8 );
      channel->m_Category = dbRes.GetString( 9 );
      channel->m_Image = dbRes.GetString( 10 );
      channel->m_DownloadType = dbRes.GetInt( 11 );
      channel->m_DownloadText = dbRes.GetString( 12 );
      channel->m_AllowDelete = dbRes.GetBool( 13 );
    }
  }
  dbRes.Finalize();
  return RetVal;
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::DelPodcastChannel( const int id )
{
  wxString query;

  query = wxString::Format( wxT( "DELETE FROM podcastchs WHERE podcastch_id = %u;" ), id );

  ExecuteUpdate( query );

  DelPodcastItems( id );
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetPodcastItems( guPodcastItemArray * items )
{
  wxASSERT( items );
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxT( "SELECT podcastitem_id, podcastitem_chid, podcastitem_title, "
            "podcastitem_summary, podcastitem_author, podcastitem_enclosure, podcastitem_time, "
            "podcastitem_file, podcastitem_filesize, podcastitem_length, "
            "podcastitem_playcount, podcastitem_addeddate, podcastitem_lastplay, "
            "podcastitem_status, "
            "podcastch_title, podcastch_category "
            "FROM podcastitems, podcastchs "
            "WHERE podcastitem_chid = podcastch_id AND podcastitem_status != 4" ); // dont get the deleted items

  if( m_PodChFilters.Count() )
  {
        query += wxT( " AND " ) + ArrayToFilter( m_PodChFilters, wxT( "podcastitem_chid" ) );
  }

  query += wxT( " ORDER BY " );
  if( m_PodcastOrder == guPODCASTS_COLUMN_TITLE )
    query += wxT( "podcastitem_title COLLATE NOCASE" );
  else if( m_PodcastOrder == guPODCASTS_COLUMN_CHANNEL )
    query += wxT( "podcastch_title COLLATE NOCASE" );
  else if( m_PodcastOrder == guPODCASTS_COLUMN_CATEGORY )
    query += wxT( "podcastch_category COLLATE NOCASE" );
  else if( m_PodcastOrder == guPODCASTS_COLUMN_DATE )
    query += wxT( "podcastitem_time" );
  else if( m_PodcastOrder == guPODCASTS_COLUMN_LENGTH )
    query += wxT( "podcastitem_length" );
  else if( m_PodcastOrder == guPODCASTS_COLUMN_AUTHOR )
    query += wxT( "podcastitem_author COLLATE NOCASE" );
  else if( m_PodcastOrder == guPODCASTS_COLUMN_PLAYCOUNT )
    query += wxT( "podcastitem_playcount" );
  else if( m_PodcastOrder == guPODCASTS_COLUMN_LASTPLAY )
    query += wxT( "podcastitem_lastplay" );
  else if( m_PodcastOrder == guPODCASTS_COLUMN_ADDEDDATE )
    query += wxT( "podcastitem_addeddate" );
  else if( m_PodcastOrder == guPODCASTS_COLUMN_STATUS )
    query += wxT( "podcastitem_status" );

  if( m_PodcastOrderDesc )
    query += wxT( " DESC;" );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    guPodcastItem * Item = new guPodcastItem();
    Item->m_Id = dbRes.GetInt( 0 );
    Item->m_ChId = dbRes.GetInt( 1 );
    Item->m_Title = dbRes.GetString( 2 );
    Item->m_Summary = dbRes.GetString( 3 );
    Item->m_Author = dbRes.GetString( 4 );
    Item->m_Enclosure = dbRes.GetString( 5 );
    Item->m_Time = dbRes.GetInt( 6 );
    Item->m_FileName = dbRes.GetString( 7 );
    Item->m_FileSize = dbRes.GetInt( 8 );
    Item->m_Length = dbRes.GetInt( 9 );
    Item->m_PlayCount = dbRes.GetInt( 10 );
    Item->m_AddedDate = dbRes.GetInt( 11 );
    Item->m_LastPlay = dbRes.GetInt( 12 );
    Item->m_Status = dbRes.GetInt( 13 );

    Item->m_Channel = dbRes.GetString( 14 );
    Item->m_Category = dbRes.GetString( 15 );
    items->Add( Item );
  }
  dbRes.Finalize();
  return items->Count();
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::GetPodcastCounters( wxLongLong * count, wxLongLong * len, wxLongLong * size )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxT( "SELECT COUNT(), SUM( podcastitem_length ), SUM( podcastitem_filesize ) "
            "FROM podcastitems, podcastchs "
            "WHERE podcastitem_chid = podcastch_id AND podcastitem_status != 4" ); // dont get the deleted items

  if( m_PodChFilters.Count() )
  {
        query += wxT( " AND " ) + ArrayToFilter( m_PodChFilters, wxT( "podcastitem_chid" ) );
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
int guDbLibrary::GetPodcastItems( const wxArrayInt &ids, guPodcastItemArray * items )
{
  wxASSERT( items );
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxT( "SELECT podcastitem_id, podcastitem_chid, podcastitem_title, "
            "podcastitem_summary, podcastitem_author, podcastitem_enclosure, podcastitem_time, "
            "podcastitem_file, podcastitem_filesize, podcastitem_length, "
            "podcastitem_playcount, podcastitem_addeddate, podcastitem_lastplay, "
            "podcastitem_status, "
            "podcastch_title, podcastch_category "
            "FROM podcastitems, podcastchs "
            "WHERE podcastitem_chid = podcastch_id "
            "AND " ) + ArrayToFilter( ids, wxT( "podcastitem_id" ) );

  query += wxT( " ORDER BY " );
  if( m_PodcastOrder == guPODCASTS_COLUMN_TITLE )
    query += wxT( "podcastitem_title COLLATE NOCASE" );
  else if( m_PodcastOrder == guPODCASTS_COLUMN_CHANNEL )
    query += wxT( "podcastch_title COLLATE NOCASE" );
  else if( m_PodcastOrder == guPODCASTS_COLUMN_CATEGORY )
    query += wxT( "podcastch_category COLLATE NOCASE" );
  else if( m_PodcastOrder == guPODCASTS_COLUMN_DATE )
    query += wxT( "podcastitem_time" );
  else if( m_PodcastOrder == guPODCASTS_COLUMN_LENGTH )
    query += wxT( "podcastitem_length" );
  else if( m_PodcastOrder == guPODCASTS_COLUMN_AUTHOR )
    query += wxT( "podcastitem_author COLLATE NOCASE" );
  else if( m_PodcastOrder == guPODCASTS_COLUMN_PLAYCOUNT )
    query += wxT( "podcastitem_playcount" );
  else if( m_PodcastOrder == guPODCASTS_COLUMN_LASTPLAY )
    query += wxT( "podcastitem_lastplay" );
  else if( m_PodcastOrder == guPODCASTS_COLUMN_ADDEDDATE )
    query += wxT( "podcastitem_addeddate" );
  else if( m_PodcastOrder == guPODCASTS_COLUMN_STATUS )
    query += wxT( "podcastitem_status" );

  if( m_PodcastOrderDesc )
    query += wxT( " DESC;" );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    guPodcastItem * Item = new guPodcastItem();
    Item->m_Id = dbRes.GetInt( 0 );
    Item->m_ChId = dbRes.GetInt( 1 );
    Item->m_Title = dbRes.GetString( 2 );
    Item->m_Summary = dbRes.GetString( 3 );
    Item->m_Author = dbRes.GetString( 4 );
    Item->m_Enclosure = dbRes.GetString( 5 );
    Item->m_Time = dbRes.GetInt( 6 );
    Item->m_FileName = dbRes.GetString( 7 );
    Item->m_FileSize = dbRes.GetInt( 8 );
    Item->m_Length = dbRes.GetInt( 9 );
    Item->m_PlayCount = dbRes.GetInt( 10 );
    Item->m_AddedDate = dbRes.GetInt( 11 );
    Item->m_LastPlay = dbRes.GetInt( 12 );
    Item->m_Status = dbRes.GetInt( 13 );

    Item->m_Channel = dbRes.GetString( 14 );
    Item->m_Category = dbRes.GetString( 15 );
    items->Add( Item );
  }
  dbRes.Finalize();
  return items->Count();
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SavePodcastItem( const int channelid, guPodcastItem * item, bool onlynew )
{
  wxASSERT( item );

  wxString query;
  int ItemId;
  if( ( ItemId = GetPodcastItemEnclosure( item->m_Enclosure ) ) == wxNOT_FOUND )
  {
    //guLogMessage( wxT( "Inserting podcastitem '%s'" ), item->m_Title.c_str() );
    query = wxString::Format( wxT( "INSERT INTO podcastitems( "
                "podcastitem_id, podcastitem_chid, podcastitem_title, "
                "podcastitem_summary, podcastitem_author, podcastitem_enclosure, podcastitem_time, "
                "podcastitem_file, podcastitem_filesize, podcastitem_length, "
                "podcastitem_addeddate, podcastitem_playcount, podcastitem_lastplay, "
                "podcastitem_status ) "
                "VALUES( NULL, %u, '%s', '%s', '%s', '%s', %u, "
                "'%s', %u, %u, %u, %u, %u, %u );" ),
                channelid,
                escape_query_str( item->m_Title ).c_str(),
                escape_query_str( item->m_Summary ).c_str(),
                escape_query_str( item->m_Author ).c_str(),
                escape_query_str( item->m_Enclosure ).c_str(),
                item->m_Time,
                escape_query_str( item->m_FileName ).c_str(),
                item->m_FileSize,
                item->m_Length,
                wxDateTime::GetTimeNow(),
                0, 0, 0 );

    ExecuteUpdate( query );
    ItemId = m_Db.GetLastRowId().GetLo();
  }
  else if( !onlynew )
  {
    query = wxString::Format( wxT( "UPDATE podcastitems SET "
                "podcastitem_chid = %u, podcastitem_title = '%s', "
                "podcastitem_summary = '%s', podcastitem_author = '%s', "
                "podcastitem_enclosure = '%s', podcastitem_time = %u, "
                "podcastitem_file = '%s', podcastitem_filesize = %u, podcastitem_length = %u, "
                "podcastitem_status = %u "
                "WHERE podcastitem_id = %u;" ),
                channelid,
                escape_query_str( item->m_Title ).c_str(),
                escape_query_str( item->m_Summary ).c_str(),
                escape_query_str( item->m_Author ).c_str(),
                escape_query_str( item->m_Enclosure ).c_str(),
                item->m_Time,
                escape_query_str( item->m_FileName ).c_str(),
                item->m_FileSize,
                item->m_Length,
                item->m_Status,
                ItemId );

    ExecuteUpdate( query );
  }
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SavePodcastItems( const int channelid, guPodcastItemArray * items, bool onlynew )
{
    wxASSERT( items );
    int Index;
    int Count = items->Count();
    for( Index = 0; Index < Count; Index++ )
    {
        SavePodcastItem( channelid, &items->Item( Index ), onlynew );
    }
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetPodcastItemStatus( const int itemid, const int status )
{
  wxString query;
  query = wxString::Format( wxT( "UPDATE podcastitems SET "
                "podcastitem_status = %u WHERE podcastitem_id = %u;" ),
            status, itemid );

  ExecuteUpdate( query );
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetPodcastItemPlayCount( const int itemid, const int playcount )
{
  wxString query;
  query = wxString::Format( wxT( "UPDATE podcastitems SET "
                "podcastitem_playcount = %u, podcastitem_lastplay = %u WHERE podcastitem_id = %u;" ),
            playcount, wxDateTime::GetTimeNow(), itemid );

  ExecuteUpdate( query );
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::UpdatePodcastItemLength( const int itemid, const int length )
{
  wxString query;
  query = wxString::Format( wxT( "UPDATE podcastitems SET "
                "podcastitem_length = %u WHERE podcastitem_id = %u;" ),
            length, itemid );

  ExecuteUpdate( query );
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetPodcastItemEnclosure( const wxString &enclosure, guPodcastItem * item )
{
  int RetVal = wxNOT_FOUND;
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxString::Format( wxT( "SELECT podcastitem_id, podcastitem_chid, podcastitem_title, "
            "podcastitem_summary, podcastitem_author, podcastitem_enclosure, podcastitem_time, "
            "podcastitem_file, podcastitem_filesize, podcastitem_length, "
            "podcastitem_playcount, podcastitem_addeddate, podcastitem_lastplay, "
            "podcastitem_status, "
            "podcastch_title, podcastch_category "
            "FROM podcastitems, podcastchs "
            "WHERE podcastitem_chid = podcastch_id AND "
            "podcastitem_enclosure = '%s';" ),
            escape_query_str( enclosure ).c_str() );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    RetVal = dbRes.GetInt( 0 );
    if( item )
    {
      item->m_Id = RetVal;
      item->m_ChId = dbRes.GetInt( 1 );
      item->m_Title = dbRes.GetString( 2 );
      item->m_Summary = dbRes.GetString( 3 );
      item->m_Author = dbRes.GetString( 4 );
      item->m_Enclosure = dbRes.GetString( 5 );
      item->m_Time = dbRes.GetInt( 6 );
      item->m_FileName = dbRes.GetString( 7 );
      item->m_FileSize = dbRes.GetInt( 8 );
      item->m_Length = dbRes.GetInt( 9 );
      item->m_PlayCount = dbRes.GetInt( 10 );
      item->m_AddedDate = dbRes.GetInt( 11 );
      item->m_LastPlay = dbRes.GetInt( 12 );
      item->m_Status = dbRes.GetInt( 13 );

      item->m_Channel = dbRes.GetString( 14 );
      item->m_Category = dbRes.GetString( 15 );
    }
  }
  dbRes.Finalize();
  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetPodcastItemId( const int itemid, guPodcastItem * item )
{
  int RetVal = wxNOT_FOUND;
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxString::Format( wxT( "SELECT podcastitem_id, podcastitem_chid, podcastitem_title, "
            "podcastitem_summary, podcastitem_author, podcastitem_enclosure, podcastitem_time, "
            "podcastitem_file, podcastitem_filesize, podcastitem_length, "
            "podcastitem_playcount, podcastitem_addeddate, podcastitem_lastplay, "
            "podcastitem_status, "
            "podcastch_title, podcastch_category "
            "FROM podcastitems, podcastchs "
            "WHERE podcastitem_chid = podcastch_id AND "
            "podcastitem_id = %u LIMIT 1;" ),
            itemid );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    RetVal = dbRes.GetInt( 0 );
    if( item )
    {
      item->m_Id = RetVal;
      item->m_ChId = dbRes.GetInt( 1 );
      item->m_Title = dbRes.GetString( 2 );
      item->m_Summary = dbRes.GetString( 3 );
      item->m_Author = dbRes.GetString( 4 );
      item->m_Enclosure = dbRes.GetString( 5 );
      item->m_Time = dbRes.GetInt( 6 );
      item->m_FileName = dbRes.GetString( 7 );
      item->m_FileSize = dbRes.GetInt( 8 );
      item->m_Length = dbRes.GetInt( 9 );
      item->m_PlayCount = dbRes.GetInt( 10 );
      item->m_AddedDate = dbRes.GetInt( 11 );
      item->m_LastPlay = dbRes.GetInt( 12 );
      item->m_Status = dbRes.GetInt( 13 );

      item->m_Channel = dbRes.GetString( 14 );
      item->m_Category = dbRes.GetString( 15 );
    }
  }
  dbRes.Finalize();
  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetPodcastItemFile( const wxString &filename, guPodcastItem * item )
{
  int RetVal = 0;
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxString::Format( wxT( "SELECT podcastitem_id, podcastitem_chid, podcastitem_title, "
            "podcastitem_summary, podcastitem_author, podcastitem_enclosure, podcastitem_time, "
            "podcastitem_file, podcastitem_filesize, podcastitem_length, "
            "podcastitem_playcount, podcastitem_addeddate, podcastitem_lastplay, "
            "podcastitem_status, "
            "podcastch_title, podcastch_category "
            "FROM podcastitems, podcastchs "
            "WHERE podcastitem_chid = podcastch_id AND "
            "podcastitem_file = '%s' LIMIT 1;" ),
            escape_query_str( filename ).c_str() );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    RetVal = dbRes.GetInt( 0 );
    if( item )
    {
      item->m_Id = RetVal;
      item->m_ChId = dbRes.GetInt( 1 );
      item->m_Title = dbRes.GetString( 2 );
      item->m_Summary = dbRes.GetString( 3 );
      item->m_Author = dbRes.GetString( 4 );
      item->m_Enclosure = dbRes.GetString( 5 );
      item->m_Time = dbRes.GetInt( 6 );
      item->m_FileName = dbRes.GetString( 7 );
      item->m_FileSize = dbRes.GetInt( 8 );
      item->m_Length = dbRes.GetInt( 9 );
      item->m_PlayCount = dbRes.GetInt( 10 );
      item->m_AddedDate = dbRes.GetInt( 11 );
      item->m_LastPlay = dbRes.GetInt( 12 );
      item->m_Status = dbRes.GetInt( 13 );

      item->m_Channel = dbRes.GetString( 14 );
      item->m_Category = dbRes.GetString( 15 );
    }
  }
  dbRes.Finalize();
  return RetVal;
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::DelPodcastItem( const int itemid )
{
  wxString query;

  query = wxString::Format( wxT( "UPDATE podcastitems SET "
            "podcastitem_status = %u WHERE podcastitem_id = %u;" ),
            guPODCAST_STATUS_DELETED, itemid );

  ExecuteUpdate( query );
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::DelPodcastItems( const int channelid )
{
  wxString query;

  query = wxString::Format( wxT( "DELETE FROM podcastitems "
            "WHERE podcastitem_chid = %u;" ), channelid );

  ExecuteUpdate( query );
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetPodcastChannelFilters( const wxArrayInt &filters )
{
    if( filters.Index( 0 ) != wxNOT_FOUND )
    {
        m_PodChFilters.Empty();
    }
    else
    {
        m_PodChFilters = filters;
    }
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::SetPodcastOrder( int order )
{
    if( m_PodcastOrder != order )
    {
        m_PodcastOrder = order;
        m_PodcastOrderDesc = ( order != 0 );
    }
    else
        m_PodcastOrderDesc = !m_PodcastOrderDesc;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetPendingPodcasts( guPodcastItemArray * items )
{
  wxASSERT( items );
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxT( "SELECT podcastitem_id, podcastitem_chid, podcastitem_title, "
            "podcastitem_summary, podcastitem_author, podcastitem_enclosure, podcastitem_time, "
            "podcastitem_file, podcastitem_filesize, podcastitem_length, "
            "podcastitem_playcount, podcastitem_addeddate, podcastitem_lastplay, "
            "podcastitem_status, "
            "podcastch_title, podcastch_category "
            "FROM podcastitems, podcastchs "
            "WHERE podcastitem_chid = podcastch_id "
            "AND podcastitem_status IN ( 1, 2 ) "
            "ORDER BY podcastitem_status DESC;" );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    guPodcastItem * Item = new guPodcastItem();
    Item->m_Id = dbRes.GetInt( 0 );
    Item->m_ChId = dbRes.GetInt( 1 );
    Item->m_Title = dbRes.GetString( 2 );
    Item->m_Summary = dbRes.GetString( 3 );
    Item->m_Author = dbRes.GetString( 4 );
    Item->m_Enclosure = dbRes.GetString( 5 );
    Item->m_Time = dbRes.GetInt( 6 );
    Item->m_FileName = dbRes.GetString( 7 );
    Item->m_FileSize = dbRes.GetInt( 8 );
    Item->m_Length = dbRes.GetInt( 9 );
    Item->m_PlayCount = dbRes.GetInt( 10 );
    Item->m_AddedDate = dbRes.GetInt( 11 );
    Item->m_LastPlay = dbRes.GetInt( 12 );
    Item->m_Status = dbRes.GetInt( 13 );

    Item->m_Channel = dbRes.GetString( 14 );
    Item->m_Category = dbRes.GetString( 15 );
    items->Add( Item );
  }
  dbRes.Finalize();
  return items->Count();
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetPodcastFiles( const wxArrayInt &channels, wxFileDataObject * files )
{
  int Count = 0;
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxT( "SELECT podcastitem_file FROM podcastitems WHERE " ) +
          ArrayToFilter( channels, wxT( "podcastitem_chid" ) );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
      wxString FileName = dbRes.GetString( 0 );
      FileName.Replace( wxT( "#" ), wxT( "%23" ) );
      files->AddFile( FileName );
      Count++;
  }

  dbRes.Finalize();
  return Count;
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

        wxString query = wxString::Format( wxT( "UPDATE songs SET song_filename = '%s', song_pathid = %i WHERE song_id = %u;" ),
            escape_query_str( newname.AfterLast( '/' ) ).c_str(),
            PathId,
            TrackId );

        //guLogMessage( wxT( "Updating file: %s" ), query.c_str() );
        ExecuteUpdate( query );
    }
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::UpdatePaths( const wxString &oldpath, const wxString &newpath )
{
  wxString query;

  query = wxString::Format( wxT( "UPDATE paths SET path_value = replace( path_value, '%s', '%s' )" ),
            escape_query_str( oldpath ).c_str(), escape_query_str( newpath ).c_str() );

  //guLogMessage( wxT( "Updating path: %s" ), query.c_str() );
  ExecuteUpdate( query );

}

// -------------------------------------------------------------------------------- //
