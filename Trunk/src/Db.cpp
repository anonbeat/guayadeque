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
#include "Db.h"

#include "Utils.h"

//#define DBLIBRARY_SHOW_QUERIES    1

// -------------------------------------------------------------------------------- //
guDb::guDb()
{
}

// -------------------------------------------------------------------------------- //
guDb::guDb( const wxString &dbname )
{
    Open( dbname );
}

// -------------------------------------------------------------------------------- //
guDb::~guDb()
{
    if( m_Db.IsOpen() )
    {
        Close();
    }
}

// -------------------------------------------------------------------------------- //
int guDb::Open( const wxString &dbname )
{
  m_DbName = dbname;
  m_Db.Open( dbname );

  if( m_Db.IsOpen() )
  {
    SetInitParams();

    return true;
  }
  return false;
}

// -------------------------------------------------------------------------------- //
int guDb::Close()
{
  if( m_Db.IsOpen() )
    m_Db.Close();
  return 1;
}

// -------------------------------------------------------------------------------- //
wxSQLite3ResultSet guDb::ExecuteQuery( const wxString &query )
{
#ifdef  DBLIBRARY_SHOW_QUERIES
  guLogMessage( query );
#endif
  wxSQLite3ResultSet RetVal;
  try {
    RetVal = m_Db.ExecuteQuery( query );
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
  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDb::ExecuteUpdate( const wxString &query )
{
#ifdef  DBLIBRARY_SHOW_QUERIES
  guLogMessage( query );
#endif
  int RetVal = 0;
  try {
    RetVal = m_Db.ExecuteUpdate( query );
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
  return RetVal;
}

// -------------------------------------------------------------------------------- //
wxSQLite3ResultSet guDb::ExecuteQuery( const wxSQLite3StatementBuffer &query )
{
  return m_Db.ExecuteQuery( query );
}

// -------------------------------------------------------------------------------- //
int guDb::ExecuteUpdate( const wxSQLite3StatementBuffer &query )
{
  return m_Db.ExecuteUpdate( query );
}

// -------------------------------------------------------------------------------- //
void guDb::SetInitParams( void )
{
  wxString query;
  query = wxT( "PRAGMA page_size=8192; PRAGMA cache_size=4096; PRAGMA count_changes=1; PRAGMA synchronous='OFF'; PRAGMA short_column_names=0; PRAGMA full_column_names=0;" );
  ExecuteUpdate( query );
}

// -------------------------------------------------------------------------------- //
