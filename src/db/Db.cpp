// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2019 J.Rios anonbeat@gmail.com
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
//    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
//    Boston, MA 02110-1301 USA.
//
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#include "Db.h"

#include "Utils.h"

//#define DBLIBRARY_SHOW_QUERIES      1
//#define DBLIBRARY_SHOW_TIMES        1
//#define DBLIBRARY_MIN_TIME          50

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
guDb::guDb()
{
    m_Db = NULL;
}

// -------------------------------------------------------------------------------- //
guDb::guDb( const wxString &dbname )
{
    Open( dbname );
}

// -------------------------------------------------------------------------------- //
guDb::~guDb()
{
    if( m_Db )
    {
        Close();

        delete m_Db;
    }
}

// -------------------------------------------------------------------------------- //
int guDb::Open( const wxString &dbname )
{
  m_DbName = dbname;
  m_Db = new wxSQLite3Database();
  if( m_Db )
  {
    m_Db->Open( dbname );

    if( m_Db->IsOpen() )
    {
      SetInitParams();
      return true;
    }
  }
  return false;
}

// -------------------------------------------------------------------------------- //
int guDb::Close()
{
  if( m_Db->IsOpen() )
    m_Db->Close();
  return 1;
}

// -------------------------------------------------------------------------------- //
wxSQLite3ResultSet guDb::ExecuteQuery( const wxString &query )
{
#ifdef  DBLIBRARY_SHOW_QUERIES
  guLogMessage( wxT( "ExecQuery:\n%s" ), query.c_str() );
#endif
#ifdef DBLIBRARY_SHOW_TIMES
  wxLongLong time = wxGetLocalTimeMillis();
#endif
  wxSQLite3ResultSet RetVal;
  try {
    RetVal = m_Db->ExecuteQuery( query );
  }
  catch( wxSQLite3Exception &e )
  {
    guLogError( wxT( "guDbLibrary::ExecuteQuery exception '%s'\n%u: %s" ),
        query.c_str(), e.GetErrorCode(), e.GetMessage().c_str() );
  }
  catch(...)
  {
    guLogError( wxT( "Other exception found while executing:\n'%s'" ), query.c_str() );
  }
#ifdef DBLIBRARY_SHOW_TIMES
  time = wxGetLocalTimeMillis() - time;
  if( time > DBLIBRARY_MIN_TIME )
    guLogWarning( wxT( "Query: %u ms\n%s" ), time.GetLo(), query.c_str() );
#endif
  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDb::ExecuteUpdate( const wxString &query )
{
#ifdef  DBLIBRARY_SHOW_QUERIES
  guLogMessage( wxT( "ExecUpdate:\n%s" ), query.c_str() );
#endif
#ifdef DBLIBRARY_SHOW_TIMES
  wxLongLong time = wxGetLocalTimeMillis();
#endif
  int RetVal = 0;
  try {
    RetVal = m_Db->ExecuteUpdate( query );
  }
  catch( wxSQLite3Exception &e )
  {
    guLogError( wxT( "guDbLibrary::ExecuteUpdate exception '%s'\n%u: %s" ),
        query.c_str(), e.GetErrorCode(), e.GetMessage().c_str() );
  }
  catch(...)
  {
    guLogError( wxT( "Other exception found while executing:\n'%s'" ), query.c_str() );
  }
#ifdef DBLIBRARY_SHOW_TIMES
  time = wxGetLocalTimeMillis() - time;
  if( time > DBLIBRARY_MIN_TIME )
    guLogWarning( wxT( "Query: %u ms\n%s" ), time.GetLo(), query.c_str() );
#endif
  return RetVal;
}

// -------------------------------------------------------------------------------- //
wxSQLite3ResultSet guDb::ExecuteQuery( const wxSQLite3StatementBuffer &query )
{
  return m_Db->ExecuteQuery( query );
}

// -------------------------------------------------------------------------------- //
int guDb::ExecuteUpdate( const wxSQLite3StatementBuffer &query )
{
  return m_Db->ExecuteUpdate( query );
}

// -------------------------------------------------------------------------------- //
void guDb::SetInitParams( void )
{
  wxString query;
  query = wxT( "PRAGMA legacy_file_format=false; PRAGMA page_size=4096; PRAGMA cache_size=4096; PRAGMA count_changes=1; PRAGMA synchronous='OFF'; PRAGMA short_column_names=0; PRAGMA full_column_names=0;" );
  //query = wxT( "PRAGMA legacy_file_format=false; PRAGMA page_size=102400; PRAGMA cache_size=204800; PRAGMA count_changes=1; PRAGMA synchronous='OFF'; PRAGMA short_column_names=0; PRAGMA full_column_names=0;" );
  //query = wxT( "PRAGMA page_size=10240; PRAGMA cache_size=65536; PRAGMA count_changes=1; PRAGMA synchronous='OFF'; PRAGMA short_column_names=0; PRAGMA full_column_names=0;" );
  ExecuteUpdate( query );
}

}

// -------------------------------------------------------------------------------- //
