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
static wxString     LastAlbumArtist = wxT( "__Gu4y4d3qu3__" );  // Make sure its not the same.
static int          LastAlbumArtistId = wxNOT_FOUND;
static wxString     LastComposer = wxT( "__Gu4y4d3qu3__" );  // Make sure its not the same.
static int          LastComposerId = wxNOT_FOUND;

WX_DEFINE_OBJARRAY(guTrackArray);
WX_DEFINE_OBJARRAY(guListItems);
WX_DEFINE_OBJARRAY(guArrayListItems);
WX_DEFINE_OBJARRAY(guAlbumItems);
WX_DEFINE_OBJARRAY(guCoverInfos);
WX_DEFINE_OBJARRAY(guAS_SubmitInfoArray);

#define GU_CURRENT_DBVERSION    "18"

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
    //m_PodcastOrder = Config->ReadNum( wxT( "Order" ), 0, wxT( "Podcasts" ) );
    //m_PodcastOrderDesc = Config->ReadBool( wxT( "OrderDesc" ), false, wxT( "Podcasts" ) );
  }

  //LoadCache();
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
    //m_PodcastOrder = Config->ReadNum( wxT( "Order" ), 0, wxT( "Podcasts" ) );
    //m_PodcastOrderDesc = Config->ReadBool( wxT( "OrderDesc" ), false, wxT( "Podcasts" ) );
  }

  //LoadCache();
}

// -------------------------------------------------------------------------------- //
guDbLibrary::guDbLibrary( guDb * db ) : guDb()
{
    m_Db = db->GetDb();
}

// -------------------------------------------------------------------------------- //
guDbLibrary::~guDbLibrary()
{
  guConfig * Config = ( guConfig * ) guConfig::Get();
  if( Config )
  {
    Config->WriteNum( wxT( "TracksOrder" ), m_TracksOrder, wxT( "General" ) );
    Config->WriteBool( wxT( "TracksOrderDesc" ), m_TracksOrderDesc, wxT( "General" ) );
    //Config->WriteNum( wxT( "Order" ), m_PodcastOrder, wxT( "Podcasts" ) );
    //Config->WriteBool( wxT( "OrderDesc" ), m_PodcastOrderDesc, wxT( "Podcasts" ) );
    Config->WriteNum( wxT( "AlbumYearOrder" ), m_AlbumsOrder, wxT( "General" ) );
  }

  Close();
}

//// -------------------------------------------------------------------------------- //
//void guDbLibrary::LoadCache( void )
//{
////    Labels.Empty();
////    GetLabels( &Labels );
////    m_GenresCache.Empty();
////    GetGenres( &m_GenresCache, true );
////    m_ArtistsCache.Empty();
////    GetArtists( &m_ArtistsCache, true );
////    m_ComposersCache.Empty();
////    GetComposers( &m_ComposersCache, true );
////    m_AlbumsCache.Empty();
////    GetAlbums( &m_AlbumsCache , true );
////    m_PathsCache.Empty();
////    GetPaths( &m_PathsCache, true );
//}

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

//      query.Add( wxT( "CREATE TABLE IF NOT EXISTS genres( genre_id INTEGER PRIMARY KEY AUTOINCREMENT,genre_name varchar(255) );" ) );
//      //query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'genre_id' on genres (genre_id ASC);" ) );
//      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'genre_name' on genres (genre_name ASC);" ) );
//
//      query.Add( wxT( "CREATE TABLE IF NOT EXISTS albums( album_id INTEGER PRIMARY KEY AUTOINCREMENT, album_artistid INTEGER, album_pathid INTEGER, album_name varchar(255), album_coverid INTEGER );" ) );
//      query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'album_id' on albums (album_id ASC);" ) );
//      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'album_artistid' on albums (album_artistid ASC);" ) );
//      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'album_pathid' on albums (album_pathid ASC);" ) );
//      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'album_name' on albums (album_name ASC);" ) );
//
//      query.Add( wxT( "CREATE TABLE IF NOT EXISTS artists( artist_id INTEGER  PRIMARY KEY AUTOINCREMENT, artist_name varchar(255) );" ) );
//      query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'artist_id' on artists (artist_id ASC);" ) );
//      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'artist_name' on artists (artist_name ASC);" ) );
//
//      query.Add( wxT( "CREATE TABLE IF NOT EXISTS composers( composer_id INTEGER  PRIMARY KEY AUTOINCREMENT, composer_name varchar(512) );" ) );
//      query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'composer_id' on composers (composer_id ASC);" ) );
//      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'composer_name' on composers (composer_name ASC);" ) );
//
//      query.Add( wxT( "CREATE TABLE IF NOT EXISTS paths( path_id INTEGER PRIMARY KEY AUTOINCREMENT,path_value varchar(1024) );" ) );
//      query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'path_id' on paths (path_id ASC);" ) );
//      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'path_value' on paths (path_value ASC);" ) );


//CREATE TABLE IF NOT EXISTS songs1( song_id INTEGER PRIMARY KEY AUTOINCREMENT, song_name VARCHAR COLLATE NOCASE, song_genreid INTEGER, song_genre VARCHAR COLLATE NOCASE, song_artistid INTEGER, song_artist VARCHAR COLLATE NOCASE, song_albumartistid INTEGER, song_albumartist VARCHAR COLLATE NOCASE, song_composerid INTEGER, song_composer VARCHAR COLLATE NOCASE, song_albumid INTEGER, song_album VARCHAR COLLATE NOCASE, song_pathid INTEGER, song_path VARCHAR, song_filename VARCHAR, song_format VARCHAR(8) COLLATE NOCASE, song_disk VARCHAR COLLATE NOCASE, song_number INTEGER(3), song_year INTEGER(4), song_comment VARCHAR COLLATE NOCASE, song_coverid INTEGER, song_offset INTEGER, song_length INTEGER, song_bitrate INTEGER, song_rating INTEGER DEFAULT -1, song_playcount INTEGER DEFAULT 0, song_addedtime INTEGER, song_lastplay INTEGER, song_filesize INTEGER );
//INSERT INTO songs1( song_id, song_name, song_albumid, song_artistid, song_genreid, song_filename, song_pathid, song_number, song_disk, song_year, song_comment, song_composerid, song_length, song_bitrate, song_rating, song_playcount, song_addedtime, song_lastplay, song_filesize ) SELECT song_id, song_name, song_albumid, song_artistid, song_genreid, song_filename, song_pathid, song_number, song_disk, song_year, song_comment, song_composerid, song_length, song_bitrate, song_rating, song_playcount, song_addedtime, song_lastplay, song_filesize FROM songs
//UPDATE songs1 SET song_genre = ( SELECT genre_name FROM songs, genres WHERE songs1.song_id = song_id AND song_genreid = genreid )
//UPDATE songs1 SET song_genre = ( SELECT genre_name FROM genres WHERE song_genreid = genre_id )
//UPDATE songs1 SET song_artist = ( SELECT artist_name FROM artists WHERE song_artistid = artist_id )
//UPDATE songs1 SET song_album = ( SELECT album_name FROM albums WHERE song_albumid = album_id )
//UPDATE songs1 SET song_composer = ( SELECT composer_name FROM composers WHERE song_composerid = composer_id )
//UPDATE songs1 SET song_path = ( SELECT path_name FROM paths WHERE song_pathid = path_id )
//UPDATE songs1 SET song_path = ( SELECT path_value FROM paths WHERE song_pathid = path_id )
//UPDATE songs1 SET song_coverid = ( SELECT album_coverid FROM albums WHERE song_albumid = album_id )
//UPDATE songs1 SET song_offset = 0

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS songs( song_id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "song_name VARCHAR COLLATE NOCASE, song_genreid INTEGER, song_genre VARCHAR COLLATE NOCASE, "
                      "song_artistid INTEGER, song_artist VARCHAR COLLATE NOCASE, song_albumartistid INTEGER, "
                      "song_albumartist VARCHAR COLLATE NOCASE, song_composerid INTEGER, "
                      "song_composer VARCHAR COLLATE NOCASE, song_albumid INTEGER, song_album VARCHAR COLLATE NOCASE, "
                      "song_pathid INTEGER, song_path VARCHAR, song_filename VARCHAR, "
                      "song_format VARCHAR(8) COLLATE NOCASE, song_disk VARCHAR COLLATE NOCASE, "
                      "song_number INTEGER(3), song_year INTEGER(4), song_comment VARCHAR COLLATE NOCASE, "
                      "song_coverid INTEGER, song_offset INTEGER, song_length INTEGER, song_bitrate INTEGER, "
                      "song_rating INTEGER DEFAULT -1, song_playcount INTEGER DEFAULT 0, song_addedtime INTEGER, "
                      "song_lastplay INTEGER, song_filesize INTEGER );" ) );

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
      query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'tag_id' on tags (tag_id ASC);" ) );

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS settags( settag_tagid INTEGER, settag_artistid INTEGER, settag_albumid INTEGER, settag_songid INTEGER );" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'settag_tagid' on settags (settag_tagid ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'settag_artistid' on settags (settag_artistid ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'settag_albumid' on settags (settag_albumid ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'settag_songid' on settags (settag_songid ASC);" ) );

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS playlists( playlist_id INTEGER PRIMARY KEY AUTOINCREMENT, playlist_name VARCHAR COLLATE NOCASE, "
                      "playlist_type INTEGER(2), playlist_limited BOOLEAN, playlist_limitvalue INTEGER, playlist_limittype INTEGER(2), "
                      "playlist_sorted BOOLEAN, playlist_sorttype INTEGER(2), playlist_sortdesc BOOLEAN, playlist_anyoption BOOLEAN);" ) );
      query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'playlist_id' on playlists (playlist_id ASC);" ) );

      wxString querystr = wxT( "INSERT INTO playlists( playlist_id, playlist_name, playlist_type, "
                      "playlist_limited, playlist_limitvalue, playlist_limittype, "
                      "playlist_sorted, playlist_sorttype, playlist_sortdesc, playlist_anyoption ) "
                      "VALUES( NULL, '" );
      querystr += _( "Recent Added Tracks" );
      querystr += wxT( "', 1, 0, 0, 0, 1, 12, 1, 0 );" );
      query.Add( querystr );
      querystr = wxT( "INSERT INTO playlists( playlist_id, playlist_name, playlist_type, "
                      "playlist_limited, playlist_limitvalue, playlist_limittype, "
                      "playlist_sorted, playlist_sorttype, playlist_sortdesc, playlist_anyoption ) "
                      "VALUES( NULL, '" );

      querystr += _( "Last Played Tracks" );
      querystr += wxT( "', 1, 0, 0, 0, 1, 11, 1, 0 );" );
      query.Add( querystr );
      querystr = wxT( "INSERT INTO playlists( playlist_id, playlist_name, playlist_type, "
                      "playlist_limited, playlist_limitvalue, playlist_limittype, "
                      "playlist_sorted, playlist_sorttype, playlist_sortdesc, playlist_anyoption ) "
                      "VALUES( NULL, '" );
      querystr += _( "Most Rated Tracks" );
      querystr += wxT( "', 1, 0, 0, 0, 1, 8, 1, 0 );" );
      query.Add( querystr );

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS plsets( plset_id INTEGER PRIMARY KEY AUTOINCREMENT, plset_plid INTEGER, plset_songid INTEGER, "
                      "plset_type INTEGER(2), plset_option INTEGER(2), plset_text TEXT(255), plset_number INTEGER, plset_option2 INTEGER );" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'plset_id' on plsets (plset_id ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'plset_plid' on plsets (plset_plid ASC);" ) );
      query.Add( wxT( "INSERT INTO plsets( plset_id, plset_plid, plset_songid, "
                      "plset_type, plset_option, plset_text, plset_number, plset_option2 ) "
                      "VALUES( NULL, 1, 0, 14, 0, '', 1, 3 );" ) );
      query.Add( wxT( "INSERT INTO plsets( plset_id, plset_plid, plset_songid, "
                      "plset_type, plset_option, plset_text, plset_number, plset_option2 ) "
                      "VALUES( NULL, 2, 0, 13, 0, '', 1, 2 );" ) );
      query.Add( wxT( "INSERT INTO plsets( plset_id, plset_plid, plset_songid, "
                      "plset_type, plset_option, plset_text, plset_number, plset_option2 ) "
                      "VALUES( NULL, 3, 0, 10, 2, '', 4, 0 );" ) );

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS covers( cover_id INTEGER PRIMARY KEY AUTOINCREMENT, cover_path VARCHAR, cover_thumb BLOB, cover_midsize BLOB, cover_hash VARCHAR );" ) );
      query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'cover_id' on covers (cover_id ASC);" ) );

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS audioscs( audiosc_id INTEGER PRIMARY KEY AUTOINCREMENT, audiosc_artist VARCHAR(255), audiosc_album varchar(255), audiosc_track varchar(255), audiosc_playedtime INTEGER, audiosc_source char(1), audiosc_ratting char(1), audiosc_len INTEGER, audiosc_tracknum INTEGER, audiosc_mbtrackid INTEGER );" ) );
      query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'audiosc_id' on audioscs (audiosc_id ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'audiosc_playedtime' on audioscs (audiosc_playedtime ASC);" ) );

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS radiogenres( radiogenre_id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "radiogenre_name VARCHAR COLLATE NOCASE, radiogenre_source INTEGER, radiogenre_flags INTEGER );" ) );
      query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'radiogenre_id' on radiogenres (radiogenre_id ASC);" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name, radiogenre_source, radiogenre_flags ) VALUES( NULL, '60s', 0, 0 );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name, radiogenre_source, radiogenre_flags ) VALUES( NULL, '80s', 0, 0 );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name, radiogenre_source, radiogenre_flags ) VALUES( NULL, '90s', 0, 0 );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name, radiogenre_source, radiogenre_flags ) VALUES( NULL, 'Alternative', 0, 0 );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name, radiogenre_source, radiogenre_flags ) VALUES( NULL, 'Ambient', 0, 0 );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name, radiogenre_source, radiogenre_flags ) VALUES( NULL, 'Blues', 0, 0 );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name, radiogenre_source, radiogenre_flags ) VALUES( NULL, 'Chill', 0, 0 );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name, radiogenre_source, radiogenre_flags ) VALUES( NULL, 'Classical', 0, 0 );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name, radiogenre_source, radiogenre_flags ) VALUES( NULL, 'Country', 0, 0 );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name, radiogenre_source, radiogenre_flags ) VALUES( NULL, 'Dance', 0, 0 );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name, radiogenre_source, radiogenre_flags ) VALUES( NULL, 'Downtempo', 0, 0 );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name, radiogenre_source, radiogenre_flags ) VALUES( NULL, 'Easy Listening', 0, 0 );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name, radiogenre_source, radiogenre_flags ) VALUES( NULL, 'Electronic', 0, 0 );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name, radiogenre_source, radiogenre_flags ) VALUES( NULL, 'Funk', 0, 0 );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name, radiogenre_source, radiogenre_flags ) VALUES( NULL, 'Heavy Metal', 0, 0 );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name, radiogenre_source, radiogenre_flags ) VALUES( NULL, 'House', 0, 0 );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name, radiogenre_source, radiogenre_flags ) VALUES( NULL, 'Jazz', 0, 0 );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name, radiogenre_source, radiogenre_flags ) VALUES( NULL, 'New Age', 0, 0 );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name, radiogenre_source, radiogenre_flags ) VALUES( NULL, 'Oldies', 0, 0 );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name, radiogenre_source, radiogenre_flags ) VALUES( NULL, 'Pop', 0, 0 );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name, radiogenre_source, radiogenre_flags ) VALUES( NULL, 'Reggae', 0, 0 );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name, radiogenre_source, radiogenre_flags ) VALUES( NULL, 'R&B/Urban', 0, 0 );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name, radiogenre_source, radiogenre_flags ) VALUES( NULL, 'Rock', 0, 0 );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name, radiogenre_source, radiogenre_flags ) VALUES( NULL, 'Smooth Jazz', 0, 0 );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name, radiogenre_source, radiogenre_flags ) VALUES( NULL, 'Slow', 0, 0 );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name, radiogenre_source, radiogenre_flags ) VALUES( NULL, 'Techno', 0, 0 );" ) );
      query.Add( wxT( "INSERT INTO radiogenres( radiogenre_id, radiogenre_name, radiogenre_source, radiogenre_flags ) VALUES( NULL, 'Top 40', 0, 0 );" ) );

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS radiostations( radiostation_id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "radiostation_scid INTEGER, radiostation_source INTEGER, radiostation_genreid INTEGER, "
                      "radiostation_name VARCHAR COLLATE NOCASE, radiostation_link VARCHAR, radiostation_type VARCHAR, "
                      "radiostation_br INTEGER, radiostation_lc INTEGER, radiostation_ct VARCHAR );" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'radiostation_id' on radiostations (radiostation_id ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'radiostation_genreid' on radiostations (radiostation_source,radiostation_genreid ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'radiostation_lc' on radiostations (radiostation_lc ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'radiostation_type' on radiostations (radiostation_type ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'radiostation_ct' on radiostations (radiostation_ct ASC);" ) );

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS radiolabels( radiolabel_id INTEGER PRIMARY KEY AUTOINCREMENT, radiolabel_name VARCHAR COLLATE NOCASE);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'radiolabel_id' on radiolabels (radiolabel_id ASC);" ) );

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS radiosetlabels( radiosetlabel_id INTEGER PRIMARY KEY AUTOINCREMENT, radiosetlabel_labelid INTEGER, radiosetlabel_stationid INTEGER);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'radiosetlabel_id' on radiosetlabels (radiosetlabel_id ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'radiosetlabel_labelid' on radiosetlabels (radiosetlabel_labelid ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'radiosetlabel_stationidid' on radiosetlabels (radiosetlabel_stationid ASC);" ) );
    }

    case 5 :
    {
      query.Add( wxT( "CREATE TABLE IF NOT EXISTS podcastchs( podcastch_id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "podcastch_url VARCHAR, podcastch_title VARCHAR COLLATE NOCASE, podcastch_description VARCHAR, "
                      "podcastch_language VARCHAR, podcastch_time INTEGER, podcastch_sumary VARCHAR, "
                      "podcastch_author VARCHAR, podcastch_ownername VARCHAR, podcastch_owneremail VARCHAR, "
                      "podcastch_category VARCHAR, podcastch_image VARCHAR, podcastch_downtype INTEGER, "
                      "podcastch_downtext VARCHAR, podcastch_allowdel BOOLEAN );" ) );
      query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'podcastch_id' on podcastchs(podcastch_id ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'podcastch_title' on podcastchs(podcastch_title ASC);" ) );
      query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'podcastch_url' on podcastchs(podcastch_url ASC);" ) );

      query.Add( wxT( "CREATE TABLE IF NOT EXISTS podcastitems( podcastitem_id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "podcastitem_chid INTEGER, podcastitem_title VARCHAR COLLATE NOCASE, podcastitem_summary VARCHAR, "
                      "podcastitem_author VARCHAR COLLATE NOCASE, podcastitem_enclosure VARCHAR, podcastitem_time INTEGER, "
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
      if( dbVer > 4 )
      {
          query.Add( wxT( "DROP INDEX 'radiostation_id';" ) );
          query.Add( wxT( "DROP INDEX 'radiostation_genreid';" ) );
          query.Add( wxT( "DROP TABLE 'radiostations';" ) );
          query.Add( wxT( "CREATE TABLE IF NOT EXISTS radiostations( radiostation_id INTEGER PRIMARY KEY AUTOINCREMENT, "
                          "radiostation_scid INTEGER, radiostation_source INTEGER, radiostation_genreid INTEGER, "
                          "radiostation_name VARCHAR, radiostation_link VARCHAR, radiostation_type VARCHAR, "
                          "radiostation_br INTEGER, radiostation_lc INTEGER, radiostation_ct VARCHAR );" ) );
          query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'radiostation_id' on radiostations (radiostation_id ASC);" ) );
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
        query.Add( wxT( "CREATE TABLE IF NOT EXISTS songs1( song_id INTEGER PRIMARY KEY AUTOINCREMENT, "
                        "song_name VARCHAR COLLATE NOCASE, song_genreid INTEGER, song_genre VARCHAR COLLATE NOCASE, "
                        "song_artistid INTEGER, song_artist VARCHAR COLLATE NOCASE, song_albumartistid INTEGER, "
                        "song_albumartist VARCHAR COLLATE NOCASE, song_composerid INTEGER, song_composer VARCHAR COLLATE NOCASE, "
                        "song_albumid INTEGER, song_album VARCHAR COLLATE NOCASE, song_pathid INTEGER, song_path VARCHAR, "
                        "song_filename VARCHAR, song_format VARCHAR(8) COLLATE NOCASE, song_disk VARCHAR COLLATE NOCASE, "
                        "song_number INTEGER(3), song_year INTEGER(4), song_comment VARCHAR COLLATE NOCASE, "
                        "song_coverid INTEGER, song_offset INTEGER, song_length INTEGER, song_bitrate INTEGER, "
                        "song_rating INTEGER DEFAULT -1, song_playcount INTEGER DEFAULT 0, song_addedtime INTEGER, "
                        "song_lastplay INTEGER, song_filesize INTEGER );" ) );
        query.Add( wxT( "INSERT INTO songs1( song_id, song_name, song_albumid, song_artistid, song_genreid, song_filename, "
                        "song_pathid, song_number, song_disk, song_year, song_comment, song_composerid, song_length, "
                        "song_bitrate, song_rating, song_playcount, song_addedtime, song_lastplay, song_filesize ) "
                        "SELECT song_id, song_name, song_albumid, song_artistid, song_genreid, song_filename, song_pathid, "
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
          query.Add( wxT( "UPDATE plsets SET plset_option = plset_option + 1 WHERE "
                          "( plset_option > 2 AND plset_type IN ( 9 ) ) OR"
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
        }
        query.Add( wxT( "CREATE TABLE IF NOT EXISTS deleted( delete_id INTEGER PRIMARY KEY AUTOINCREMENT, "
                          "delete_path VARCHAR, delete_date INTEGER );" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS delete_path on deleted(delete_path ASC);" ) );
        query.Add( wxT( "CREATE INDEX IF NOT EXISTS delete_date on deleted(delete_date ASC);" ) );

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
  if( LastGenre == genrename )
  {
      return LastGenreId;
  }

  wxString query;
  wxSQLite3ResultSet dbRes;
  int RetVal = wxNOT_FOUND;

  LastGenre = genrename;

  query = wxString::Format( wxT( "SELECT song_genreid FROM songs "
                                 "WHERE song_genre = '%s' LIMIT 1;" ),
                        escape_query_str( genrename ).c_str() );

  dbRes = ExecuteQuery( query );
  if( dbRes.NextRow() )
  {
    RetVal = LastGenreId = dbRes.GetInt( 0 );
  }
  else
  {
    query = wxT( "SELECT MAX(song_genreid) FROM songs;" );

    dbRes = ExecuteQuery( query );
    if( dbRes.NextRow() )
    {
      RetVal = LastGenreId = dbRes.GetInt( 0 ) + 1;
    }
    else
    {
        RetVal = LastGenreId = 1;
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

  query = wxString::Format( wxT( "SELECT song_composerid FROM songs "
                                 "WHERE song_composer = '%s' LIMIT 1;" ),
            escape_query_str( composername ).c_str() );

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
    RetVal = LastComposerId = dbRes.GetInt( 0 );
  }
  else if( create )
  {
    query = wxT( "SELECT MAX(song_composerid) FROM songs;" );

    dbRes = ExecuteQuery( query );
    if( dbRes.NextRow() )
    {
      RetVal = LastComposerId = dbRes.GetInt( 0 ) + 1;
    }
    else
    {
        RetVal = LastComposerId = 1;
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
          guLogError( wxT( "Error resizing to 100" ) );
      }
    }
    else
    {
      guLogError( wxT( "Error resizing to 38" ) );
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
  //guLogMessage( wxT( "GetAlbumId : %s" ), LastAlbum.c_str() );
  if( LastAlbum == albumname )
  {
      //guLogMessage( wxT( "Album was found in cache" ) );
      return LastAlbumId;
  }

  wxString query;
  wxSQLite3ResultSet dbRes;
  int RetVal = wxNOT_FOUND;

  query = wxString::Format( wxT( "SELECT song_albumid, song_artistid "
                                 "FROM songs "
                                 "WHERE song_album = '%s' "
                                 "AND song_pathid = %u LIMIT 1;" ),
                        escape_query_str( albumname ).c_str(), pathid );

  //guLogMessage( wxT( "%s" ), query.c_str() );

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
    RetVal = LastAlbumId = dbRes.GetInt( 0 );

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
      RetVal = LastAlbumId = dbRes.GetInt( 0 ) + 1;
    }
    else
    {
        RetVal = LastAlbumId = 1;
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
  if( PathValue == LastPath )
  {
      return LastPathId;
  }

  int RetVal = 0;

//TODO Add a Lock
  wxString query;
  wxSQLite3ResultSet dbRes;

  LastPath = PathValue;

  if( !PathValue.EndsWith( wxT( "/" ) ) )
    PathValue += '/';

  query = wxString::Format( wxT( "SELECT song_pathid FROM songs "
                                 "WHERE song_path = '%s' LIMIT 1" ),
                    escape_query_str( PathValue ).c_str() );
  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
    RetVal = LastPathId = dbRes.GetInt( 0 );
  }
  else
  {
    dbRes.Finalize();
    query = wxT( "SELECT MAX(song_pathid) FROM songs;" );
    dbRes = ExecuteQuery( query );
    if( dbRes.NextRow() )
    {
        RetVal = LastPathId = dbRes.GetInt( 0 ) + 1;
    }
    else
    {
        RetVal = LastPathId = 1;
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
    query = query.Format( wxT( "INSERT INTO songs( song_id, song_pathid, song_rating, song_playcount, song_addedtime ) "
                               "VALUES( NULL, %u, -1, 0, %u )" ), pathid, wxDateTime::GetTimeNow() );
    if( ExecuteUpdate( query ) )
    {
      RetVal = GetLastRowId();
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

          m_CurSong.m_Path = wxPathOnly( FileName );
          if( !m_CurSong.m_Path.EndsWith( wxT( "/" ) ) )
            m_CurSong.m_Path += '/';

          //guLogMessage( wxT( "PathName: %s" ), PathName.c_str() );
          m_CurSong.m_PathId = GetPathId( m_CurSong.m_Path );

          m_CurSong.m_ComposerId = GetComposerId( TagInfo->m_Composer );
          m_CurSong.m_Composer = TagInfo->m_Composer;

          m_CurSong.m_ArtistId = GetArtistId( TagInfo->m_ArtistName );
          m_CurSong.m_ArtistName = TagInfo->m_ArtistName;

          m_CurSong.m_AlbumArtistId = GetAlbumArtistId( TagInfo->m_AlbumArtist );
          m_CurSong.m_AlbumArtist = TagInfo->m_AlbumArtist;

          m_CurSong.m_AlbumId = GetAlbumId( TagInfo->m_AlbumName, m_CurSong.m_ArtistId, m_CurSong.m_PathId, m_CurSong.m_Path );
          m_CurSong.m_AlbumName = TagInfo->m_AlbumName;

          m_CurSong.m_CoverId = GetAlbumCoverId( m_CurSong.m_AlbumId );

          m_CurSong.m_GenreId = GetGenreId( TagInfo->m_GenreName );
          m_CurSong.m_GenreName = TagInfo->m_GenreName;

          m_CurSong.m_FileName = FileName.AfterLast( '/' );
          m_CurSong.m_SongName = TagInfo->m_TrackName;
          m_CurSong.m_FileSize = guGetFileSize( FileName );

          m_CurSong.m_SongId = GetSongId( m_CurSong.m_FileName, m_CurSong.m_PathId );

          m_CurSong.m_Number   = TagInfo->m_Track;
          m_CurSong.m_Year     = TagInfo->m_Year;
          m_CurSong.m_Length   = TagInfo->m_Length;
          m_CurSong.m_Bitrate  = TagInfo->m_Bitrate;
          //m_CurSong.m_Rating   = -1;
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

          UpdateSong( false );

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
  wxArrayPtrVoid PanelPtrs;
  bool SendMainLibPanel = false;

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
        TagInfo->m_AlbumArtist = Song->m_AlbumArtist;
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
            if( Song->m_LibPanel )
            {
                if( PanelPtrs.Index( Song->m_LibPanel ) == wxNOT_FOUND )
                    PanelPtrs.Add( Song->m_LibPanel );
            }
            else if( !SendMainLibPanel )
            {
                SendMainLibPanel = true;
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

            m_CurSong = * Song;
            m_CurSong.m_GenreId = GenreId;
            m_CurSong.m_ArtistId = ArtistId;
            m_CurSong.m_AlbumArtistId = AlbumArtistId;
            m_CurSong.m_ComposerId = ComposerId;
            m_CurSong.m_AlbumId = AlbumId;
            m_CurSong.m_PathId = PathId;
            m_CurSong.m_FileName = Song->m_FileName.AfterLast( '/' );
            //CurSong.SongId = SongId;
            //CurSong.FileName.c_str(),
            //CurSong.Number
            //CurSong.Year,
            //CurSong.Length,
            UpdateSong( true );
        }
    }
    else
    {
        guLogMessage( wxT( "The file %s was not found for edition." ), Song->m_FileName.c_str() );
    }
    //wxSafeYield();
  }

  // We added in PanelPtr all panels we are updating tracks
  // And send the clean db event to all of them
  wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_LIBRARY_DOCLEANDB );
  event.SetEventObject( ( wxObject * ) this );

  if( SendMainLibPanel )
    wxPostEvent( wxTheApp->GetTopWindow(), event );

  count = PanelPtrs.Count();
  for( index = 0; index < count; index++ )
  {
    event.SetClientData( ( void * ) PanelPtrs[ index ] );
    wxPostEvent( wxTheApp->GetTopWindow(), event );
  }

}

// -------------------------------------------------------------------------------- //
int guDbLibrary::UpdateSong( const bool allowrating )
{
  wxString query;
//  printf( "UpdateSong\n" );

  if( allowrating )
  {
      query = wxString::Format( wxT( "UPDATE songs SET song_name = '%s', "
                                 "song_genreid = %u, song_genre = '%s', "
                                 "song_artistid = %u, song_artist = '%s', "
                                 "song_albumartistid = %u, song_albumartist = '%s', "
                                 "song_albumid = %u, song_album = '%s', "
                                 "song_pathid = %u, song_path = '%s', "
                                 "song_filename = '%s', song_format = '%s', "
                                 "song_number = %u, song_year = %u, "
                                 "song_composerid = %u, song_composer = '%s', "
                                 "song_comment = '%s', song_coverid = %i, song_disk = '%s', "
                                 "song_length = %u, song_offset = %u, song_bitrate = %u, "
                                 "song_rating = %i, "
                                 "song_filesize = %u WHERE song_id = %u;" ),
            escape_query_str( m_CurSong.m_SongName ).c_str(),
            m_CurSong.m_GenreId,
            escape_query_str( m_CurSong.m_GenreName ).c_str(),
            m_CurSong.m_ArtistId,
            escape_query_str( m_CurSong.m_ArtistName ).c_str(),
            m_CurSong.m_AlbumArtistId,
            escape_query_str( m_CurSong.m_AlbumArtist ).c_str(),
            m_CurSong.m_AlbumId,
            escape_query_str( m_CurSong.m_AlbumName ).c_str(),
            m_CurSong.m_PathId,
            escape_query_str( m_CurSong.m_Path ).c_str(),
            escape_query_str( m_CurSong.m_FileName ).c_str(),
            escape_query_str( m_CurSong.m_FileName.AfterLast( '.' ) ).c_str(),
            m_CurSong.m_Number,
            m_CurSong.m_Year,
            m_CurSong.m_ComposerId, //escape_query_str( m_CurSong.m_Composer ).c_str(),
            escape_query_str( m_CurSong.m_Composer ).c_str(),
            escape_query_str( m_CurSong.m_Comments ).c_str(),
            m_CurSong.m_CoverId,
            escape_query_str( m_CurSong.m_Disk ).c_str(),
            m_CurSong.m_Length,
            0, //m_CurSong.m_Offset,
            m_CurSong.m_Bitrate,
            m_CurSong.m_Rating,
            m_CurSong.m_FileSize,
            m_CurSong.m_SongId );
  }
  else
  {
      query = wxString::Format( wxT( "UPDATE songs SET song_name = '%s', "
                                 "song_genreid = %u, song_genre = '%s', "
                                 "song_artistid = %u, song_artist = '%s', "
                                 "song_albumartistid = %u, song_albumartist = '%s', "
                                 "song_albumid = %u, song_album = '%s', "
                                 "song_pathid = %u, song_path = '%s', "
                                 "song_filename = '%s', song_format = '%s', "
                                 "song_number = %u, song_year = %u, "
                                 "song_composerid = %u, song_composer = '%s', "
                                 "song_comment = '%s', song_coverid = %i, song_disk = '%s', "
                                 "song_length = %u, song_offset = %u, song_bitrate = %u, "
                                 "song_filesize = %u WHERE song_id = %u;" ),
            escape_query_str( m_CurSong.m_SongName ).c_str(),
            m_CurSong.m_GenreId,
            escape_query_str( m_CurSong.m_GenreName ).c_str(),
            m_CurSong.m_ArtistId,
            escape_query_str( m_CurSong.m_ArtistName ).c_str(),
            m_CurSong.m_AlbumArtistId,
            escape_query_str( m_CurSong.m_AlbumArtist ).c_str(),
            m_CurSong.m_AlbumId,
            escape_query_str( m_CurSong.m_AlbumName ).c_str(),
            m_CurSong.m_PathId,
            escape_query_str( m_CurSong.m_Path ).c_str(),
            escape_query_str( m_CurSong.m_FileName ).c_str(),
            escape_query_str( m_CurSong.m_FileName.AfterLast( '.' ) ).c_str(),
            m_CurSong.m_Number,
            m_CurSong.m_Year,
            m_CurSong.m_ComposerId, //escape_query_str( m_CurSong.m_Composer ).c_str(),
            escape_query_str( m_CurSong.m_Composer ).c_str(),
            escape_query_str( m_CurSong.m_Comments ).c_str(),
            m_CurSong.m_CoverId,
            escape_query_str( m_CurSong.m_Disk ).c_str(),
            m_CurSong.m_Length,
            0, //m_CurSong.m_Offset,
            m_CurSong.m_Bitrate,
            m_CurSong.m_FileSize,
            m_CurSong.m_SongId );
  }

  return ExecuteUpdate( query );
}

// -------------------------------------------------------------------------------- //
void guDbLibrary::UpdateImageFile( const char * filename, const char * saveto )
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
        CoverFile = wxPathOnly( FileName ) + wxT( '/' ) + SaveTo;
        if( !Image->SaveFile( CoverFile, wxBITMAP_TYPE_JPEG ) )
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
    return GetLastRowId();
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
    query = wxT( "SELECT DISTINCT song_year FROM songs "
                 "WHERE song_year > 0 AND " ) + FiltersSQL( guLIBRARY_FILTER_YEARS );
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
            wxT( "WHERE song_rating >= 0 AND " ) + FiltersSQL( guLIBRARY_FILTER_SONGS );
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
            wxT( "WHERE " ) + FiltersSQL( guLIBRARY_FILTER_SONGS );
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
    query = wxT( "SELECT song_albumartistid, song_albumartist FROM songs WHERE song_albumartist > '' "
                 "GROUP BY song_albumartistid ORDER BY song_albumartist" );
  }
  else
  {
    query = wxT( "SELECT song_albumartistid, song_albumartist FROM songs " ) \
            wxT( "WHERE song_albumartist > '' AND " ) + FiltersSQL( guLIBRARY_FILTER_ALBUMARTISTS );
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
    query = wxT( "SELECT song_composerid, song_composer FROM songs WHERE song_composer > '' "
                 "GROUP BY song_composerid ORDER BY song_composer" );
  }
  else
  {
    query = wxT( "SELECT song_composerid, song_composer FROM songs " ) \
            wxT( "WHERE song_composer > '' AND " ) + FiltersSQL( guLIBRARY_FILTER_COMPOSERS );
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
            query += wxT( "song_artist, song_album, song_disk " );
            break;

        case guALBUMS_ORDER_ARTIST_YEAR :
            query += wxT( "song_artist, song_year, song_album, song_disk" );
            break;

        case guALBUMS_ORDER_ARTIST_YEAR_REVERSE :
        default :
            query += wxT( "song_artist, song_year DESC, song_album, song_disk" );
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

const wxString DynPlayListToSQLQuery( guDynPlayList * playlist );

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

//  if( textfilters.Count() )
//  {
//      query += wxT( ", albums, artists " );
//  }

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

  //SELECT playlist_id, playlist_nameguLogMessage( wxT( "GetAlbumsCount:\n%s" ), ( query + postquery ).c_str() );

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
  wxString              subquery;

  query = wxT( "SELECT song_albumid, song_album, song_artistid, song_albumartist, song_artist, song_coverid FROM songs " );
  if( filter )
  {
    subquery = DynPlayListToSQLQuery( filter );
//    if( DynQuery.Find( wxT( "albums" ) ) == wxNOT_FOUND )
//        query += wxT( ", albums" );
//    if( DynQuery.Find( wxT( "artists" ) ) == wxNOT_FOUND )
//        query += wxT( ", artists " );
//    query += DynQuery;
//    query += DynQuery.IsEmpty() ? wxT( " WHERE " ) : wxT( " AND " );
//    query += wxT( "album_artistid = artist_id AND song_albumid = album_id " );
  }
//  else
//  {
//    query += wxT( ", albums, artists WHERE album_artistid = artist_id AND song_albumid = album_id " );
//  }

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
      query += wxT( "song_artist, song_album " );
      break;

    case guALBUMS_ORDER_ARTIST_YEAR :
      query += wxT( "song_artist, song_year" );
      break;

    case guALBUMS_ORDER_ADDEDTIME :
      query += wxT( "song_addedtime DESC,song_album,song_disk " );
      break;

    case guALBUMS_ORDER_ARTIST_YEAR_REVERSE :
    default :
      query += wxT( "song_artist, song_year DESC" );
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
  query = wxT( "SELECT song_filename, song_path FROM plsets, songs "
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
  wxString sort;
  if( playlist->m_Sorted )
  {
    sort = wxT( " ORDER BY " );
    switch( playlist->m_SortType )
    {
        case guDYNAMIC_FILTER_ORDER_TITLE :         sort += wxT( "song_name " ); break;
        case guDYNAMIC_FILTER_ORDER_ARTIST :        sort += wxT( "song_artist " ); break;
        case guDYNAMIC_FILTER_ORDER_ALBUMARTIST :   sort += wxT( "song_albumartist " ); break;
        case guDYNAMIC_FILTER_ORDER_ALBUM :         sort += wxT( "song_album " ); break;
        case guDYNAMIC_FILTER_ORDER_GENRE :         sort += wxT( "genre_name " ); break;
        case guDYNAMIC_FILTER_ORDER_COMPOSER :      sort += wxT( "song_composer " ); break;
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
        query += wxT( " ORDER BY song_number, song_filename" );
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
  if( LastArtist == artistname )
  {
      return LastArtistId;
  }

  // TODO Add a Lock
  wxString query;
  wxSQLite3ResultSet dbRes;

  LastArtist = artistname;

  query = wxString::Format( wxT( "SELECT song_artistid FROM songs "
                                 "WHERE song_artist = '%s' LIMIT 1;" ),
            escape_query_str( artistname ).c_str() );

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
    return LastArtistId = dbRes.GetInt( 0 );
  }
  else if( create )
  {
    query = wxT( "SELECT MAX(song_artistid) FROM songs" );

    dbRes = ExecuteQuery( query );
    if( dbRes.NextRow() )
    {
      return LastArtistId = dbRes.GetInt( 0 ) + 1;
    }
    else
    {
        return LastArtistId = 1;
    }
  }

  return wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
int guDbLibrary::GetAlbumArtistId( wxString &albumartist, bool create )
{
  if( LastAlbumArtist == albumartist )
  {
      return LastAlbumArtistId;
  }

  // TODO Add a Lock
  wxString query;
  wxSQLite3ResultSet dbRes;

  LastAlbumArtist = albumartist;

  query = wxString::Format( wxT( "SELECT song_albumartistid FROM songs "
                                 "WHERE song_albumartist = '%s' LIMIT 1;" ),
            escape_query_str( albumartist ).c_str() );

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
    return LastAlbumArtistId = dbRes.GetInt( 0 );
  }
  else if( create )
  {
    query = wxT( "SELECT MAX(song_albumartistid) FROM songs;" );

    dbRes = ExecuteQuery( query );
    if( dbRes.NextRow() )
    {
      return LastAlbumArtistId = dbRes.GetInt( 0 ) + 1;
    }
    else
    {
        return LastAlbumArtistId = 1;
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
    query = wxString::Format( wxT( "SELECT song_albumid FROM songs WHERE "
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
  wxString Param = Name;
  escape_query_str( &Param );

  int ArtistId = FindArtist( Artist );
  if( ArtistId != wxNOT_FOUND )
  {
    query = wxString::Format( wxT( "SELECT song_id FROM songs WHERE " )\
                wxT( "song_artistid = %d AND song_name = '%s' LIMIT 1;" ),
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
    wxASSERT( RetVal );
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
wxString GetSongsDBNamesSQL( const guTRACKS_ORDER order )
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
wxString GetSongsSortSQL( const guTRACKS_ORDER order, const bool orderdesc )
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
  //query += GetSongsDBNamesSQL( m_TracksOrder );
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
    query += wxString::Format( wxT( "VALUES( NULL, '%s', %u )" ),
              escape_query_str( file ).c_str(), wxDateTime::GetTimeNow() );
    ExecuteUpdate( query );
    return true;
  }
  return false;
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

  query = wxString::Format( wxT( "SELECT DISTINCT song_album, song_albumartist, song_artist, song_path "
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

        TagInfo->Write();

        delete TagInfo;
      }
      else
      {
        guLogError( wxT( "The file '%s' could not be found" ), Song->m_FileName.c_str() );
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

    //guLogMessage( wxT( "Artist Labels : '%s'" ), ArtistLabelStr.c_str() );
    // Update the Database
    wxArrayInt ItemIds;
    ItemIds.Add( labelsets[ ItemIndex ].GetId() );
    SetAlbumsLabels( ItemIds, ItemLabels );

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

        TagInfo->Write();

        delete TagInfo;
      }
      else
      {
        guLogError( wxT( "The file '%s' could not be found" ), Song->m_FileName.c_str() );
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

    // Get the affected tracks
    GetSongs( ItemIds, &Songs );

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
void guDbLibrary::SetTracksRating( const guTrackArray * tracks, const int rating )
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
        SetTracksRating( TrackIds, rating );
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

    query = wxT( "SELECT DISTINCT song_albumid, song_album, song_artist, song_path "
                 "FROM songs WHERE song_coverid ISNULL OR song_coverid = 0 GROUP BY song_albumid" );

    dbRes = ExecuteQuery( query );

    while( dbRes.NextRow() )
    {
        RetVal.Add( new guCoverInfo( dbRes.GetInt( 0 ), dbRes.GetString( 1 ), dbRes.GetString( 2 ), dbRes.GetString( 3 ) ) );
    }
    dbRes.Finalize();
    return RetVal;
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
    ChannelId = GetLastRowId();
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
int guDbLibrary::GetPodcastItems( guPodcastItemArray * items, const wxArrayInt &filters, const int order, const bool desc )
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

  if( filters.Count() )
  {
        query += wxT( " AND " ) + ArrayToFilter( filters, wxT( "podcastitem_chid" ) );
  }

  query += wxT( " ORDER BY " );

  switch( order )
  {
      case guPODCASTS_COLUMN_TITLE :
        query += wxT( "podcastitem_title COLLATE NOCASE" );
        break;
      case guPODCASTS_COLUMN_CHANNEL :
        query += wxT( "podcastch_title COLLATE NOCASE" );
        break;
      case guPODCASTS_COLUMN_CATEGORY :
        query += wxT( "podcastch_category COLLATE NOCASE" );
        break;
      case guPODCASTS_COLUMN_DATE :
        query += wxT( "podcastitem_time" );
        break;
      case guPODCASTS_COLUMN_LENGTH :
        query += wxT( "podcastitem_length" );
        break;
      case guPODCASTS_COLUMN_AUTHOR :
        query += wxT( "podcastitem_author COLLATE NOCASE" );
        break;
      case guPODCASTS_COLUMN_PLAYCOUNT :
        query += wxT( "podcastitem_playcount" );
        break;
      case guPODCASTS_COLUMN_LASTPLAY :
        query += wxT( "podcastitem_lastplay" );
        break;
      case guPODCASTS_COLUMN_ADDEDDATE :
        query += wxT( "podcastitem_addeddate" );
        break;
      case guPODCASTS_COLUMN_STATUS :
        query += wxT( "podcastitem_status" );
        break;
  }

  if( desc )
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
void guDbLibrary::GetPodcastCounters( const wxArrayInt &filters, wxLongLong * count, wxLongLong * len, wxLongLong * size )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxT( "SELECT COUNT(), SUM( podcastitem_length ), SUM( podcastitem_filesize ) "
            "FROM podcastitems, podcastchs "
            "WHERE podcastitem_chid = podcastch_id AND podcastitem_status != 4" ); // dont get the deleted items

  if( filters.Count() )
  {
        query += wxT( " AND " ) + ArrayToFilter( filters, wxT( "podcastitem_chid" ) );
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
int guDbLibrary::GetPodcastItems( const wxArrayInt &ids, guPodcastItemArray * items, const int order, const bool desc )
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

  switch( order )
  {
      case guPODCASTS_COLUMN_TITLE :
        query += wxT( "podcastitem_title COLLATE NOCASE" );
        break;
      case guPODCASTS_COLUMN_CHANNEL :
        query += wxT( "podcastch_title COLLATE NOCASE" );
        break;
      case guPODCASTS_COLUMN_CATEGORY :
        query += wxT( "podcastch_category COLLATE NOCASE" );
        break;
      case guPODCASTS_COLUMN_DATE :
        query += wxT( "podcastitem_time" );
        break;
      case guPODCASTS_COLUMN_LENGTH :
        query += wxT( "podcastitem_length" );
        break;
      case guPODCASTS_COLUMN_AUTHOR :
        query += wxT( "podcastitem_author COLLATE NOCASE" );
        break;
      case guPODCASTS_COLUMN_PLAYCOUNT :
        query += wxT( "podcastitem_playcount" );
        break;
      case guPODCASTS_COLUMN_LASTPLAY :
        query += wxT( "podcastitem_lastplay" );
        break;
      case guPODCASTS_COLUMN_ADDEDDATE :
        query += wxT( "podcastitem_addeddate" );
        break;
      case guPODCASTS_COLUMN_STATUS :
        query += wxT( "podcastitem_status" );
        break;
  }

  if( desc )
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
    ItemId = GetLastRowId();
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

//// -------------------------------------------------------------------------------- //
//void guDbLibrary::SetPodcastChannelFilters( const wxArrayInt &filters )
//{
//    if( filters.Index( 0 ) != wxNOT_FOUND )
//    {
//        m_PodChFilters.Empty();
//    }
//    else
//    {
//        m_PodChFilters = filters;
//    }
//}

//// -------------------------------------------------------------------------------- //
//void guDbLibrary::SetPodcastOrder( int order )
//{
//    if( m_PodcastOrder != order )
//    {
//        m_PodcastOrder = order;
//        m_PodcastOrderDesc = ( order != 0 );
//    }
//    else
//        m_PodcastOrderDesc = !m_PodcastOrderDesc;
//}

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
      wxString FileName = guFileDnDEncode( dbRes.GetString( 0 ) );
      //FileName.Replace( wxT( "#" ), wxT( "%23" ) );
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

// -------------------------------------------------------------------------------- //
