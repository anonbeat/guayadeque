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
#include "Config.h"

#include "Commands.h"

const wxEventType guConfigUpdatedEvent = wxNewEventType();

// -------------------------------------------------------------------------------- //
guConfig::guConfig( const wxString &conffile ) :
          wxFileConfig( wxT( "guayadeque" ), wxEmptyString, conffile, wxEmptyString, wxCONFIG_USE_SUBDIR )
{
    m_IgnoreLayouts = false;
    //SetRecordDefaults( true );
}

// -------------------------------------------------------------------------------- //
guConfig::~guConfig()
{
    //printf( "guConfig Deleted\n" );
    Flush();
}

// -------------------------------------------------------------------------------- //
long guConfig::ReadNum( const wxString &KeyName, const long Default, const wxString &Category )
{
    wxMutexLocker Locker( m_ConfigMutex );
    long RetVal;
    if( !Category.IsEmpty() )
    {
        SetPath( wxT( "/" ) + Category );
    }
    Read( KeyName, &RetVal, Default );
    SetPath( wxT( "/" ) );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
bool guConfig::WriteNum( const wxString &KeyName, long Value, const wxString &Category )
{
    wxMutexLocker Locker( m_ConfigMutex );
    bool RetVal;
    if( !Category.IsEmpty() )
    {
        SetPath( wxT( "/" ) + Category );
    }
    RetVal = Write( KeyName, Value );
    SetPath( wxT( "/" ) );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
bool guConfig::ReadBool( const wxString &KeyName, bool Default, const wxString &Category )
{
    wxMutexLocker Locker( m_ConfigMutex );
    bool RetVal;
    if( !Category.IsEmpty() )
    {
        SetPath( wxT( "/" ) + Category );
    }
    Read( KeyName, &RetVal, Default );
    SetPath( wxT( "/" ) );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
bool guConfig::WriteBool( const wxString &KeyName, bool Value, const wxString &Category  )
{
    wxMutexLocker Locker( m_ConfigMutex );
    bool RetVal;
    if( !Category.IsEmpty() )
    {
        SetPath( wxT( "/" ) + Category );
    }
    RetVal = Write( KeyName, Value );
    SetPath( wxT( "/" ) );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
wxString guConfig::ReadStr( const wxString &KeyName, const wxString &Default, const wxString &Category  )
{
    wxMutexLocker Locker( m_ConfigMutex );
    wxString RetVal;
    if( !Category.IsEmpty() )
    {
        SetPath( wxT( "/" ) + Category );
    }
    RetVal = Read( KeyName, Default );
    SetPath( wxT( "/" ) );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
bool guConfig::WriteStr( const wxString &KeyName, const wxString &Value, const wxString &Category  )
{
    wxMutexLocker Locker( m_ConfigMutex );
    bool RetVal;
    if( !Category.IsEmpty() )
    {
        SetPath( wxT( "/" ) + Category );
    }
    RetVal = Write( KeyName, Value );
    SetPath( wxT( "/" ) );
    return RetVal;
}

// -------------------------------------------------------------------------------- //
wxArrayString guConfig::ReadAStr( const wxString &Key, const wxString &Default, const wxString &Category  )
{
    wxMutexLocker Locker( m_ConfigMutex );
    wxString Entry;
    wxArrayString RetVal;
    if( HasGroup( Category ) )
    {
        RetVal.Empty();
        SetPath( Category );
        int index = 0;
        do {
            if( !Read( wxString::Format( Key + wxT( "%i" ), index++ ), &Entry, Default ) )
                break;
            RetVal.Add( Entry );
        } while( 1 );
        SetPath( wxT( "/" ) );
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
bool guConfig::WriteAStr( const wxString &Key, const wxArrayString &Value, const wxString &Category, bool ResetGroup )
{
    wxMutexLocker Locker( m_ConfigMutex );
    int index;
    int count = Value.Count();
    if( ResetGroup )
        DeleteGroup( Category );
    SetPath( Category );
    for( index = 0; index < count; index++ )
    {
        if( !Write( wxString::Format( Key + wxT( "%i" ), index ), Value[ index ] ) )
            break;
    }
    SetPath( wxT( "/" ) );
    return ( index = count );
}

// -------------------------------------------------------------------------------- //
wxArrayInt guConfig::ReadANum( const wxString &Key, const int Default, const wxString &Category  )
{
    wxMutexLocker Locker( m_ConfigMutex );
    int Entry;
    wxArrayInt RetVal;
    if( HasGroup( Category ) )
    {
        RetVal.Empty();
        SetPath( Category );
        int index = 0;
        do {
            if( !Read( wxString::Format( Key + wxT( "%i" ), index++ ), &Entry, Default ) )
                break;
            RetVal.Add( Entry );
        } while( 1 );
        SetPath( wxT( "/" ) );
    }
    return RetVal;
}

// -------------------------------------------------------------------------------- //
bool guConfig::WriteANum( const wxString &Key, const wxArrayInt &Value, const wxString &Category, bool ResetGroup )
{
    wxMutexLocker Locker( m_ConfigMutex );
    int index;
    int count = Value.Count();
    if( ResetGroup )
        DeleteGroup( Category );
    SetPath( Category );
    for( index = 0; index < count; index++ )
    {
        if( !Write( wxString::Format( Key + wxT( "%i" ), index ), Value[ index ] ) )
            break;
    }
    SetPath( wxT( "/" ) );
    return ( index = count );
}

// -------------------------------------------------------------------------------- //
void guConfig::RegisterObject( wxEvtHandler * object )
{
    if( m_Objects.Index( object ) == wxNOT_FOUND )
    {
        m_Objects.Add( object );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::UnRegisterObject( wxEvtHandler * object )
{
    int Index = m_Objects.Index( object );
    if( Index != wxNOT_FOUND )
    {
        m_Objects.RemoveAt( Index );
    }
}

// -------------------------------------------------------------------------------- //
void guConfig::SendConfigChangedEvent( void )
{
    wxCommandEvent event( guConfigUpdatedEvent, ID_CONFIG_UPDATED );

    int Index;
    int Count = m_Objects.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        m_Objects[ Index ]->AddPendingEvent( event );
    }
}

// -------------------------------------------------------------------------------- //
