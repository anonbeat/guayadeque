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
#include "TagInfo.h"
#include "Utils.h"

#include <unistd.h>

// -------------------------------------------------------------------------------- //
int inline GetFileLastChangeTime( const wxString &FileName )
{
    wxStructStat St;
    wxStat( FileName, &St );
    return St.st_ctime;
}

// -------------------------------------------------------------------------------- //
guLibUpdateThread::guLibUpdateThread( guDbLibrary * db, int gaugeid )
{
    m_Db = db;
    m_MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
    m_GaugeId = gaugeid;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        m_LibPaths = Config->ReadAStr( wxT( "LibPath" ), wxEmptyString, wxT( "LibPaths" ) );

        CheckSymLinks( m_LibPaths );

        m_LastUpdate = Config->ReadNum( wxT( "LastUpdate" ), 0, wxT( "General" ) );
        //guLogMessage( wxT( "LastUpdate: %s" ), LastTime.Format().c_str() );
        m_CoverSearchWords = Config->ReadAStr( wxT( "Word" ), wxEmptyString, wxT( "CoverSearch" ) );
    }

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guLibUpdateThread::~guLibUpdateThread()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        wxDateTime Now = wxDateTime::Now();
        Config->WriteNum( wxT( "LastUpdate" ), Now.GetTicks(), wxT( "General" ) );
        Config->Flush();
    }

    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_LIBRARY_UPDATED );
    event.SetEventObject( ( wxObject * ) this );
    wxPostEvent( wxTheApp->GetTopWindow(), event );
    //
    event.SetId( ID_GAUGE_REMOVE );
    event.SetInt( m_GaugeId );
    wxPostEvent( wxTheApp->GetTopWindow(), event );
}

// -------------------------------------------------------------------------------- //
int guLibUpdateThread::ScanDirectory( wxString dirname, bool includedir )
{
  wxDir         Dir;
  wxString      FileName;
  wxString      LowerFileName;

  if( !dirname.EndsWith( wxT( "/" ) ) )
    dirname += wxT( "/" );

  //guLogMessage( wxT( "Scanning dir '%s'" ), dirname.c_str() );
  Dir.Open( dirname );

  if( !TestDestroy() && Dir.IsOpened() )
  {
    if( Dir.GetFirst( &FileName, wxEmptyString, wxDIR_FILES | wxDIR_DIRS ) )
    {
      do {
        if( ( FileName[ 0 ] != '.' ) )
        {
          int FileDate = GetFileLastChangeTime( dirname + FileName );
          if( Dir.Exists( dirname + FileName ) )
          {
            //guLogMessage( wxT( "Scanning dir '%s' : FileDate: %u  -> %u\n%u Tracks found" ), ( dirname + FileName ).c_str(), m_LastUpdate, FileDate, m_TrackFiles.Count() );
            ScanDirectory( dirname + FileName, includedir || ( FileDate > m_LastUpdate ) );

            wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_GAUGE_SETMAX );
            event.SetInt( m_GaugeId );
            event.SetExtraLong( m_TrackFiles.Count() );
            wxPostEvent( m_MainFrame, event );
          }
          else
          {
            //guLogMessage( wxT( "%s : FileDate: %u  -> %u" ), ( dirname + FileName ).c_str(), m_LastUpdate, FileDate );
            if( includedir || ( FileDate > m_LastUpdate ) )
            {
              LowerFileName = FileName.Lower();

              if( guIsValidAudioFile( LowerFileName ) )
              {
                m_TrackFiles.Add( dirname + FileName );
              }
              else if( SearchCoverWords( LowerFileName, m_CoverSearchWords ) &&
                  ( LowerFileName.EndsWith( wxT( ".jpg" ) ) ||
                    LowerFileName.EndsWith( wxT( ".jpeg" ) ) ||
                    LowerFileName.EndsWith( wxT( ".png" ) ) ||
                    LowerFileName.EndsWith( wxT( ".bmp" ) ) ||
                    LowerFileName.EndsWith( wxT( ".gif" ) ) ) )
              {
                m_ImageFiles.Add( dirname + FileName );
              }
            }
          }
        }
      } while( !TestDestroy() && Dir.GetNext( &FileName ) );
    }
  }
  else
  {
      guLogMessage( wxT( "Could not open the dir '%s'" ), dirname.c_str() );
  }
  return 1;
}

// -------------------------------------------------------------------------------- //
guLibUpdateThread::ExitCode guLibUpdateThread::Entry()
{
    wxCommandEvent evtup( wxEVT_COMMAND_MENU_SELECTED, ID_GAUGE_UPDATE );
    evtup.SetInt( m_GaugeId );

    wxCommandEvent evtmax( wxEVT_COMMAND_MENU_SELECTED, ID_GAUGE_SETMAX );
    evtmax.SetInt( m_GaugeId );

    int index;
    int count = m_LibPaths.Count();
    if( !count )
    {
        guLogError( wxT( "No library directories to scan" ) );
        return 0;
    }

    // For every directory in the library scan for new files and add them to m_TrackFiles
    index = 0;
    while( !TestDestroy() && ( index < count ) )
    {
        //guLogMessage( wxT( "Doing Library Update in %s" ), m_LibPaths[ index ].c_str() );
        ScanDirectory( m_LibPaths[ index ] );
        index++;
    }

    // For every new track file update it in the database
    count = m_TrackFiles.Count();
    if( count )
    {
        index = 0;
        evtmax.SetExtraLong( count );
        wxPostEvent( m_MainFrame, evtmax );
        while( !TestDestroy() )
        {
            //guLogMessage( wxT( "%i - %i" ), index, count );
            if( ( index >= count ) )
                break;

             //guLogMessage( wxT( "Scanning: '%s'" ), m_TrackFiles[ index ].c_str() );
             m_Db->ReadFileTags( m_TrackFiles[ index ].char_str() );
                //Sleep( 1 );
            index++;
            evtup.SetExtraLong( index );
            wxPostEvent( m_MainFrame, evtup );
        }
    }

    count = m_ImageFiles.Count();
    if( count )
    {
        index = 0;
        evtmax.SetExtraLong( count );
        wxPostEvent( m_MainFrame, evtmax );
        while( !TestDestroy() )
        {
            //guLogMessage( wxT( "%i - %i" ), index, count );
            if( ( index >= count ) )
                break;

             m_Db->UpdateImageFile( m_ImageFiles[ index ].char_str() );
            index++;
            evtup.SetExtraLong( index );
            wxPostEvent( m_MainFrame, evtup );
        }
    }

    //
    // This cant be called here as wxBitmap do X11 calls and this can only be done from the main
    // thread. So we must call DoCleanUp from the main thread once this thread is finished.
    // in the OnLibraryUpdated Event handler
    //
    // delete all orphans entries
    // m_Db->DoCleanUp();

    return 0;
}

// -------------------------------------------------------------------------------- //
