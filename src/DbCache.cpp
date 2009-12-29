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
#include "DbCache.h"

#include "Utils.h"

#include <wx/mstream.h>

// -------------------------------------------------------------------------------- //
guDbCache::guDbCache( const wxString &dbname ) : guDb( dbname )
{
  wxArrayString query;

  query.Add( wxT( "CREATE TABLE IF NOT EXISTS cache( cache_id INTEGER PRIMARY KEY AUTOINCREMENT, "
                  "cache_key varchar, cache_data BLOB, cache_time INTEGER, "
                  "cache_type INTEGER  );" ) );

  query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'cache_id' on cache( cache_id ASC );" ) );
  query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'cache_key' on cache( cache_key ASC );" ) );
  query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'cache_time' on cache( cache_time ASC );" ) );

  int Index;
  int Count = query.Count();
  for( Index = 0; Index < Count; Index++ )
  {
      //guLogMessage( query[ Index ] );
    ExecuteUpdate( query[ Index ] );
  }

}

// -------------------------------------------------------------------------------- //
guDbCache::~guDbCache()
{
}

// -------------------------------------------------------------------------------- //
wxImage * guDbCache::GetImage( const wxString &url, int &imgtype )
{
  wxImage *             Img = NULL;
  wxString              query;
  wxSQLite3ResultSet    dbRes;
  const unsigned char * Data;
  int                   DataLen = 0;

  query = wxString::Format( wxT( "SELECT cache_data, cache_type FROM cache WHERE cache_key = '%s' LIMIT 1;" ),
      escape_query_str( url ).c_str() );


  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
    Data = dbRes.GetBlob( 0, DataLen );
    imgtype = dbRes.GetInt( 1 );


    if( DataLen )
    {
      wxMemoryInputStream Ins( Data, DataLen );
      wxImage * Img = new wxImage( Ins, imgtype );
      if( Img )
      {
        if( !Img->IsOk() )
        {
          guLogMessage( wxT( "Image is not OK" ) );
          delete Img;
          Img = NULL;
        }
      }
    }
  }
  else
  {
      guLogMessage( wxT( "DbCache failed '%s'" ), url.c_str() );
  }

  dbRes.Finalize();

  return Img;
}

// -------------------------------------------------------------------------------- //
bool guDbCache::SetImage( const wxString &url, wxImage * img, int imgtype )
{
  wxMemoryOutputStream Outs;
  if( img->SaveFile( Outs, imgtype ) )
  {
      wxSQLite3Statement stmt = m_Db.PrepareStatement( wxString::Format( wxT(
              "INSERT INTO cache( cache_id, cache_key, cache_data, cache_type, cache_time ) "
              "VALUES( NULL, '%s', ?, %u, %u );" ),
              escape_query_str( url ).c_str(), imgtype, wxDateTime::Now().GetTicks() ) );
      try {
        stmt.Bind( 1, ( const unsigned char * ) Outs.GetOutputStreamBuffer()->GetBufferStart(), Outs.GetSize() );
        //guLogMessage( wxT( "%s" ), stmt.GetSQL().c_str() );
        stmt.ExecuteQuery();
        return true;
      }
      catch( wxSQLite3Exception& e )
      {
        guLogError( wxT( "%u: %s" ),  e.GetErrorCode(), e.GetMessage().c_str() );
      }
  }
  return false;
}

// -------------------------------------------------------------------------------- //
wxString guDbCache::GetContent( const wxString &url )
{
  wxString              query;
  wxSQLite3ResultSet    dbRes;
  const char *          Data = NULL;
  int                   DataLen = 0;

  query = wxString::Format( wxT( "SELECT cache_data FROM cache WHERE cache_key = '%s' LIMIT 1;" ),
             escape_query_str( url ).c_str() );

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
    Data = ( const char * ) dbRes.GetBlob( 0, DataLen );
  }
  dbRes.Finalize();

  return wxString( Data, wxConvUTF8 );
}

// -------------------------------------------------------------------------------- //
bool guDbCache::SetContent( const wxString &url, const wxString &content )
{
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
