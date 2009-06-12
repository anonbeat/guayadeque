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
#include "LibUpdate.h"

#include "Commands.h"
#include "Config.h"
#include "MainApp.h"
#include "Utils.h"

// -------------------------------------------------------------------------------- //
guLibUpdateThread::guLibUpdateThread( DbLibrary * db )
{
    m_Db = db;
    m_MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
    m_GaugeId = ( ( guStatusBar * ) m_MainFrame->GetStatusBar() )->AddGauge();

    m_LibCountThread = new guLibCountThread( this );

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guLibUpdateThread::~guLibUpdateThread()
{
    if( m_LibCountThread )
    {
        m_LibCountThread->Pause();
        m_LibCountThread->Delete();
    }
    //
    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_GAUGE_REMOVE );
    event.SetInt( m_GaugeId );
    wxPostEvent( wxTheApp->GetTopWindow(), event );
}

// -------------------------------------------------------------------------------- //
guLibUpdateThread::ExitCode guLibUpdateThread::Entry()
{
    wxCommandEvent evtup( wxEVT_COMMAND_MENU_SELECTED, ID_GAUGE_UPDATE );
    evtup.SetInt( m_GaugeId );
    wxCommandEvent evtmax( wxEVT_COMMAND_MENU_SELECTED, ID_GAUGE_SETMAX );
    evtmax.SetInt( m_GaugeId );
    int lastcount = wxNOT_FOUND;
    int count = 0;
    int index = 0;

    while( !TestDestroy() )
    {
        m_FilesMutex.Lock();
        count = m_Files.Count();
        m_FilesMutex.Unlock();

        //guLogMessage( wxT( "%i - %i" ), index, count );
        if( !m_LibCountThread && ( index >= count ) )
            break;

        if( count )
        {
            m_Db->ReadFileTags( m_Files[ index ].char_str() );
            //Sleep( 1 );
            index++;
            evtup.SetExtraLong( index );
            wxPostEvent( m_MainFrame, evtup );

            if( lastcount != count )
            {
                evtmax.SetExtraLong( count );
                wxPostEvent( m_MainFrame, evtmax );
                lastcount = count;
            }
        }
    }
    return 0;
}


// -------------------------------------------------------------------------------- //
// guLibCountThread
// -------------------------------------------------------------------------------- //
guLibCountThread::guLibCountThread( guLibUpdateThread * libupdatethread )
{
    m_LibUpdateThread = libupdatethread;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
         m_LibPaths = Config->ReadAStr( wxT( "LibPath" ), wxEmptyString, wxT( "LibPaths" ) );
    }

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guLibCountThread::~guLibCountThread()
{
    if( m_LibUpdateThread && !TestDestroy() )
    {
        m_LibUpdateThread->m_LibCountThread = NULL;
    }
}

// -------------------------------------------------------------------------------- //
void guLibCountThread::AddFileToList( wxString filename )
{
    if( m_LibUpdateThread )
    {
        m_LibUpdateThread->m_FilesMutex.Lock();
        m_LibUpdateThread->m_Files.Add( filename );
        m_LibUpdateThread->m_FilesMutex.Unlock();
    }
}

// -------------------------------------------------------------------------------- //
int guLibCountThread::ScanDirectory( wxString dirname )
{
  wxDir Dir;
  wxString FileName;
  wxString SavedDir( wxGetCwd() );

  Dir.Open( dirname );
  wxSetWorkingDirectory( dirname );
  //guLogMessage( wxT( "Scanning dir '%s'" ), dirname.c_str() );

  if( Dir.IsOpened() )
  {
    if( Dir.GetFirst( &FileName, wxEmptyString, wxDIR_FILES | wxDIR_DIRS ) )
    {
      do {
        if( ( FileName[ 0 ] != '.' ) )
        {
          if( Dir.Exists( FileName ) )
          {
            //guLogMessage( wxT( "Scanning dir '%s'" ), FileName.c_str() );
            ScanDirectory( FileName );
          }
          else
          {
            // TODO: add other file formats ?
            if( FileName.EndsWith( wxT( ".mp3" ) ) )
            {
                AddFileToList( SavedDir + wxT( '/' ) + dirname + wxT( '/' ) + FileName );
            }
          }
        }
      } while( Dir.GetNext( &FileName ) && !TestDestroy() );
    }
  }
  wxSetWorkingDirectory( SavedDir );
  return 1;
}

// -------------------------------------------------------------------------------- //
guLibCountThread::ExitCode guLibCountThread::Entry()
{
    int index;
    int count = m_LibPaths.Count();
    if( m_LibUpdateThread && count )
    {
        index = 0;
        while( !TestDestroy() && ( index < count ) )
        {
            ScanDirectory( m_LibPaths[ index ] );
            index++;
        }
    }
}

// -------------------------------------------------------------------------------- //
