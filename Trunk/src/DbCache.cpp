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

// -------------------------------------------------------------------------------- //
guDbCache::guDbCache( const wxString &dbname ) : guDb( dbname )
{
  wxArrayString query;

  query.Add( wxT( "CREATE TABLE IF NOT EXISTS cache( cache_id INTEGER PRIMARY KEY AUTOINCREMENT, "
                  "cache_key varchar, cache_data BLOB, cache_width INTEGER, cache_height INTEGER );" ) );

  query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'cache_id' on cache (cache_id ASC);" ) );
  query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'cache_key' on cache (cache_key ASC);" ) );

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
