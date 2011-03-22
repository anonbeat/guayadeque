// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2011 J.Rios
//	anonbeat@gmail.com
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
bool inline IsFileSymbolicLink( const wxString &FileName )
{
    wxStructStat St;
    wxLstat( FileName, &St );
    return S_ISLNK( St.st_mode );
}

// -------------------------------------------------------------------------------- //
guLibUpdateThread::guLibUpdateThread( guLibPanel * libpanel, int gaugeid, const wxString &scanpath )
{
    m_LibPanel = libpanel;
    m_Db = libpanel->GetDb();
    m_MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
    m_GaugeId = gaugeid;
    m_ScanPath = scanpath;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    //m_CoverSearchWords = Config->ReadAStr( wxT( "Word" ), wxEmptyString, wxT( "CoverSearch" ) );
    m_CoverSearchWords = libpanel->GetCoverSearchWords();
    int Index;
    int Count = m_CoverSearchWords.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        m_CoverSearchWords[ Index ].MakeLower();
    }

    if( scanpath.IsEmpty() )
    {
        m_LibPaths = libpanel->GetLibraryPaths();
        wxString PlaylistPath = libpanel->GetPlaylistPath();
        if( !PlaylistPath.IsEmpty() )
            m_LibPaths.Add( PlaylistPath );

        CheckSymLinks( m_LibPaths );
    }

    m_LastUpdate = libpanel->LastUpdate();
    //guLogMessage( wxT( "LastUpdate: %s" ), LastTime.Format().c_str() );
    m_ScanAddPlayLists = Config->ReadBool( wxT( "ScanAddPlayLists" ), true, wxT( "General" ) );
    m_ScanEmbeddedCovers = Config->ReadBool( wxT( "ScanEmbeddedCovers" ), true, wxT( "General" ) );
    m_ScanSymlinks = Config->ReadBool( wxT( "ScanSymlinks" ), false, wxT( "General" ) );

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guLibUpdateThread::guLibUpdateThread( guDbLibrary * db, int gaugeid, const wxString &scanpath )
{
    m_LibPanel = NULL;
    m_Db = db;
    m_MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
    m_GaugeId = gaugeid;
    m_ScanPath = scanpath;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    m_CoverSearchWords = Config->ReadAStr( wxT( "Word" ), wxEmptyString, wxT( "CoverSearch" ) );

    if( scanpath.IsEmpty() )
    {
        m_LibPaths = Config->ReadAStr( wxT( "LibPath" ), wxEmptyString, wxT( "LibPaths" ) );

        CheckSymLinks( m_LibPaths );
    }

    m_LastUpdate = Config->ReadNum( wxT( "LastUpdate" ), 0, wxT( "General" ) );
    //guLogMessage( wxT( "LastUpdate: %s" ), LastTime.Format().c_str() );
    m_ScanAddPlayLists = Config->ReadBool( wxT( "ScanAddPlayLists" ), true, wxT( "General" ) );
    m_ScanEmbeddedCovers = Config->ReadBool( wxT( "ScanEmbeddedCovers" ), true, wxT( "General" ) );
    m_ScanSymlinks = Config->ReadBool( wxT( "ScanSymlinks" ), false, wxT( "General" ) );

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY + 10 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guLibUpdateThread::~guLibUpdateThread()
{
    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_LIBRARY_UPDATED );
    if( !TestDestroy() )
    {
        if( m_LibPanel )
        {
            m_LibPanel->SetLastUpdate();
        }
        else
        {
            guConfig * Config = ( guConfig * ) guConfig::Get();
            wxDateTime Now = wxDateTime::Now();
            Config->WriteNum( wxT( "LastUpdate" ), Now.GetTicks(), wxT( "General" ) );
            Config->Flush();
        }

        event.SetEventObject( ( wxObject * ) this );
        event.SetClientData( ( void * ) m_LibPanel );
        wxPostEvent( m_MainFrame, event );
    }
    //
    event.SetId( ID_STATUSBAR_GAUGE_REMOVE );
    event.SetInt( m_GaugeId );
    wxPostEvent( m_MainFrame, event );
}

// -------------------------------------------------------------------------------- //
bool guIsValidImageFile( const wxString &filename )
{
    return filename.EndsWith( wxT( ".jpg" ) ) ||
           filename.EndsWith( wxT( ".jpeg" ) ) ||
           filename.EndsWith( wxT( ".png" ) ) ||
           filename.EndsWith( wxT( ".bmp" ) ) ||
           filename.EndsWith( wxT( ".gif" ) );
}

// -------------------------------------------------------------------------------- //
int guLibUpdateThread::ScanDirectory( wxString dirname, bool includedir )
{
  wxDir         Dir;
  wxString      FileName;
  wxString      LowerFileName;
  bool          FoundCover = false;
  wxString      FirstAudioFile;

  if( !dirname.EndsWith( wxT( "/" ) ) )
    dirname += wxT( "/" );

  //guLogMessage( wxT( "Scanning dir (%i) '%s'" ), includedir, dirname.c_str() );
  Dir.Open( dirname );

  if( !TestDestroy() && Dir.IsOpened() )
  {
    if( Dir.GetFirst( &FileName, wxEmptyString, wxDIR_FILES | wxDIR_DIRS ) )
    {
      do {
        if( FileName[ 0 ] == '.' )
          continue;

        if ( !m_ScanSymlinks && IsFileSymbolicLink( dirname + FileName ) )
            continue;

        int FileDate = GetFileLastChangeTime( dirname + FileName );
        if( Dir.Exists( dirname + FileName ) )
        {
          //guLogMessage( wxT( "Scanning dir '%s' : FileDate: %u  -> %u\n%u Tracks found" ), ( dirname + FileName ).c_str(), m_LastUpdate, FileDate, m_TrackFiles.Count() );
          ScanDirectory( dirname + FileName, includedir || ( FileDate > m_LastUpdate ) );

          wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_STATUSBAR_GAUGE_SETMAX );
          event.SetInt( m_GaugeId );
          event.SetExtraLong( m_TrackFiles.Count() );
          wxPostEvent( m_MainFrame, event );
        }
        else
        {
          //guLogMessage( wxT( "%s (%i): FileDate: %u  -> %u" ), ( dirname + FileName ).c_str(), includedir, m_LastUpdate, FileDate );
          if( includedir || ( FileDate > m_LastUpdate ) )
          {
            LowerFileName = FileName.Lower();

            if( guIsValidAudioFile( LowerFileName ) )
            {
              if( !m_Db->FindDeletedFile( dirname + FileName, false ) )
              {
                m_TrackFiles.Add( dirname + FileName );
                if( m_ScanEmbeddedCovers && FirstAudioFile.IsEmpty() )
                    FirstAudioFile = dirname + FileName;
              }
            }
            else if( guIsValidImageFile( LowerFileName ) )
            {
              if( SearchCoverWords( LowerFileName, m_CoverSearchWords ) )
              {
                //guLogMessage( wxT( "Adding image '%s'" ), wxString( dirname + FileName ).c_str() );
                m_ImageFiles.Add( dirname + FileName );
              }
            }
            else if( m_ScanAddPlayLists && guPlayListFile::IsValidPlayList( LowerFileName ) )
            {
              FoundCover = true;
              m_PlayListFiles.Add( dirname + FileName );
            }
            //else
            //{
            //    guLogMessage( wxT( "Unknown file: '%s'"), ( dirname + FileName ).c_str() );
            //}
          }
        }
      } while( !TestDestroy() && Dir.GetNext( &FileName ) );

      if( m_ScanEmbeddedCovers && !FoundCover )
      {
        m_ImageFiles.Add( FirstAudioFile );
      }
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
    int index;
    int count;
    wxCommandEvent evtup( wxEVT_COMMAND_MENU_SELECTED, ID_STATUSBAR_GAUGE_UPDATE );
    evtup.SetInt( m_GaugeId );

    wxCommandEvent evtmax( wxEVT_COMMAND_MENU_SELECTED, ID_STATUSBAR_GAUGE_SETMAX );
    evtmax.SetInt( m_GaugeId );

    if( m_ScanPath.IsEmpty() )
    {
        count = m_LibPaths.Count();
        if( !count )
        {
            guLogError( wxT( "No library directories to scan" ) );
            return 0;
        }

        // For every directory in the library scan for new files and add them to m_TrackFiles
        index = 0;
        while( !TestDestroy() && ( index < count ) )
        {
            guLogMessage( wxT( "Doing Library Update in %s" ), m_LibPaths[ index ].c_str() );
            ScanDirectory( m_LibPaths[ index ] );
            index++;
        }
    }
    else
    {
        ScanDirectory( m_ScanPath, true );
    }

    // For every new track file update it in the database
    count = m_TrackFiles.Count();
    if( count )
    {
        m_Db->ExecuteUpdate( wxT( "BEGIN TRANSACTION;" ) );

        index = 0;
        evtmax.SetExtraLong( count );
        wxPostEvent( m_MainFrame, evtmax );
        while( !TestDestroy() )
        {
            //guLogMessage( wxT( "%i - %i" ), index, count );
            if( ( index >= count ) )
                break;

            //guLogMessage( wxT( "Scanning: '%s'" ), m_TrackFiles[ index ].c_str() );
            m_Db->ReadFileTags( m_TrackFiles[ index ] );
            //Sleep( 1 );
            index++;
            evtup.SetExtraLong( index );
            wxPostEvent( m_MainFrame, evtup );
        }

        m_Db->ExecuteUpdate( wxT( "COMMIT TRANSACTION;" ) );
    }


    wxString CoverName;
    if( m_LibPanel )
    {
        CoverName = m_LibPanel->GetCoverName();
    }
    else
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        wxArrayString SearchCovers = Config->ReadAStr( wxT( "Word" ), wxEmptyString, wxT( "CoverSearch" ) );
        CoverName = ( SearchCovers.Count() ? SearchCovers[ 0 ] : wxT( "cover" ) ) + wxT( ".jpg" );
    }
    int CoverType = wxBITMAP_TYPE_JPEG;
    if( m_LibPanel )
    {
        CoverType = m_LibPanel->GetCoverType();
    }
    int CoverMaxSize = wxNOT_FOUND;
    if( m_LibPanel )
    {
        CoverMaxSize = m_LibPanel->GetCoverMaxSize();
    }

    count = m_ImageFiles.Count();
    if( count )
    {
        m_Db->ExecuteUpdate( wxT( "BEGIN TRANSACTION;" ) );
        index = 0;
        evtmax.SetExtraLong( count );
        wxPostEvent( m_MainFrame, evtmax );
        while( !TestDestroy() )
        {
            //guLogMessage( wxT( "%i - %i" ), index, count );
            if( ( index >= count ) )
                break;

            m_Db->UpdateImageFile( m_ImageFiles[ index ].ToUTF8(), CoverName.ToUTF8(), CoverType, CoverMaxSize );
            index++;
            evtup.SetExtraLong( index );
            wxPostEvent( m_MainFrame, evtup );
        }

        m_Db->ExecuteUpdate( wxT( "COMMIT TRANSACTION;" ) );
    }

    count = m_PlayListFiles.Count();
    if( count )
    {
        m_Db->ExecuteUpdate( wxT( "BEGIN TRANSACTION;" ) );

        index = 0;
        evtmax.SetExtraLong( count );
        wxPostEvent( m_MainFrame, evtmax );
        while( !TestDestroy() )
        {
            //guLogMessage( wxT( "%i - %i" ), index, count );
            if( ( index >= count ) )
                break;

            guPlayListFile PlayList( m_PlayListFiles[ index ] );
            wxArrayInt PlayListIds;
            int ItemTrackId;
            int ItemIndex;
            int ItemCount;
            if( ( ItemCount = PlayList.Count() ) )
            {
                for( ItemIndex = 0; ItemIndex < ItemCount; ItemIndex++ )
                {
                    ItemTrackId = m_Db->FindTrackFile( PlayList.GetItem( ItemIndex ).m_Location, NULL );
                    if( ItemTrackId )
                    {
                        PlayListIds.Add( ItemTrackId );
                    }
                }
                if( PlayListIds.Count() )
                {
                    if( m_Db->GetStaticPlayList( m_PlayListFiles[ index ] ) == wxNOT_FOUND )
                    {
                        m_Db->CreateStaticPlayList( m_PlayListFiles[ index ], PlayListIds );

                        wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYLIST_UPDATED );
                        evt.SetClientData( ( void * ) m_LibPanel );
                        wxPostEvent( wxTheApp->GetTopWindow(), evt );
                    }
                }
            }

            index++;
            evtup.SetExtraLong( index );
            wxPostEvent( m_MainFrame, evtup );
        }

        m_Db->ExecuteUpdate( wxT( "COMMIT TRANSACTION;" ) );
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
// guLibCleanThread
// -------------------------------------------------------------------------------- //
guLibCleanThread::guLibCleanThread( guLibPanel * libpanel )
{
    m_LibPanel = libpanel;
    m_Db = libpanel->GetDb();
    m_MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guLibCleanThread::guLibCleanThread( guDbLibrary * db )
{
    m_LibPanel = NULL;
    m_Db = db;
    m_MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guLibCleanThread::~guLibCleanThread()
{
    if( !TestDestroy() )
    {
        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_LIBRARY_CLEANFINISHED );
        event.SetEventObject( ( wxObject * ) this );
        event.SetClientData( ( void * ) m_LibPanel );
        wxPostEvent( m_MainFrame, event );
    }
}

// -------------------------------------------------------------------------------- //
guLibCleanThread::ExitCode guLibCleanThread::Entry()
{
    wxString query;
    wxArrayInt SongsToDel;
    wxArrayInt CoversToDel;
    wxSQLite3ResultSet dbRes;
    wxString FileName;

    wxArrayString LibPaths;
    if( m_LibPanel )
    {
        LibPaths = m_LibPanel->GetLibraryPaths();
    }
    else
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();
        LibPaths = Config->ReadAStr( wxT( "LibPath" ), wxEmptyString, wxT( "LibPaths" ) );
    }

    if( !TestDestroy() )
    {
        CheckSymLinks( LibPaths );

        query = wxT( "SELECT DISTINCT song_id, song_filename, song_path FROM songs " );

        dbRes = m_Db->ExecuteQuery( query );

        while( !TestDestroy() && dbRes.NextRow() )
        {
            FileName = dbRes.GetString( 2 ) + dbRes.GetString( 1 );
            //guLogMessage( wxT( "Checking %s" ), FileName.c_str() );

            if( !wxFileExists( FileName ) || !CheckFileLibPath( LibPaths, FileName ) )
            {
                SongsToDel.Add( dbRes.GetInt( 0 ) );
            }
        }
        dbRes.Finalize();


        if( !TestDestroy() )
        {
            query = wxT( "SELECT DISTINCT cover_id, cover_path FROM covers;" );

            dbRes = m_Db->ExecuteQuery( query );

            while( !TestDestroy() && dbRes.NextRow() )
            {
                if( !wxFileExists( dbRes.GetString( 1 ) ) )
                {
                    CoversToDel.Add( dbRes.GetInt( 0 ) );
                }
            }
            dbRes.Finalize();


            if( !TestDestroy() )
            {
                wxArrayString Queries;

                //m_Db->CleanItems( SongsToDel, CoversToDel );
                if( SongsToDel.Count() )
                {
                    Queries.Add( wxT( "DELETE FROM songs WHERE " ) + ArrayToFilter( SongsToDel, wxT( "song_id" ) ) );
                }

                if( CoversToDel.Count() )
                {
                    Queries.Add( wxT( "DELETE FROM covers WHERE " ) + ArrayToFilter( CoversToDel, wxT( "cover_id" ) ) );
                }

                // Delete all posible orphan entries
                Queries.Add( wxT( "DELETE FROM covers WHERE cover_id NOT IN ( SELECT DISTINCT song_coverid FROM songs );" ) );
                Queries.Add( wxT( "DELETE FROM plsets WHERE plset_type = 0 AND plset_songid > 0 AND plset_songid NOT IN ( SELECT DISTINCT song_id FROM songs );" ) );
                Queries.Add( wxT( "DELETE FROM settags WHERE settag_songid NOT IN ( SELECT DISTINCT song_id FROM songs );" ) );

                int Index;
                int Count = Queries.Count();
                for( Index = 0; Index < Count; Index++ )
                {
                    if( TestDestroy() )
                        break;
                    //guLogMessage( wxT( "Executing: '%s'" ), Queries[ Index ].c_str() );
                    m_Db->ExecuteUpdate( Queries[ Index ] );
                }

                //m_Db->LoadCache();
            }
        }
    }

    return 0;
}

// -------------------------------------------------------------------------------- //
