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
#include "MediaViewer.h"
#include "TagInfo.h"
#include "Utils.h"

#include <unistd.h>
#include <wx/event.h>

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

//// -------------------------------------------------------------------------------- //
//guLibUpdateThread::guLibUpdateThread( guLibPanel * libpanel, int gaugeid, const wxString &scanpath )
//{
//    m_MediaViewer = NULL;
//    m_LibPanel = libpanel;
//    m_Db = libpanel->GetDb();
//    m_MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
//    m_GaugeId = gaugeid;
//    m_ScanPath = scanpath;
//
//    guConfig * Config = ( guConfig * ) guConfig::Get();
//    //m_CoverSearchWords = Config->ReadAStr( wxT( "Word" ), wxEmptyString, wxT( "CoverSearch" ) );
//    m_CoverSearchWords = libpanel->GetCoverSearchWords();
//    int Index;
//    int Count = m_CoverSearchWords.Count();
//    for( Index = 0; Index < Count; Index++ )
//    {
//        m_CoverSearchWords[ Index ].MakeLower();
//    }
//
//    if( scanpath.IsEmpty() )
//    {
//        m_LibPaths = libpanel->GetLibraryPaths();
//        wxString PlaylistPath = libpanel->GetPlaylistPath();
//        if( !PlaylistPath.IsEmpty() )
//            m_LibPaths.Add( PlaylistPath );
//
//        CheckSymLinks( m_LibPaths );
//    }
//
//    m_LastUpdate = libpanel->LastUpdate();
//    //guLogMessage( wxT( "LastUpdate: %s" ), LastTime.Format().c_str() );
//    m_ScanAddPlayLists = Config->ReadBool( wxT( "ScanAddPlayLists" ), true, wxT( "general" ) );
//    m_ScanEmbeddedCovers = Config->ReadBool( wxT( "ScanEmbeddedCovers" ), true, wxT( "general" ) );
//    m_ScanSymlinks = Config->ReadBool( wxT( "ScanSymlinks" ), false, wxT( "general" ) );
//
//    if( Create() == wxTHREAD_NO_ERROR )
//    {
//        SetPriority( WXTHREAD_DEFAULT_PRIORITY );
//        Run();
//    }
//}
//
//// -------------------------------------------------------------------------------- //
//guLibUpdateThread::guLibUpdateThread( guDbLibrary * db, int gaugeid, const wxString &scanpath )
//{
//    m_MediaViewer = NULL;
//    m_LibPanel = NULL;
//    m_Db = db;
//    m_MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();
//    m_GaugeId = gaugeid;
//    m_ScanPath = scanpath;
//
//    guConfig * Config = ( guConfig * ) guConfig::Get();
//    m_CoverSearchWords = Config->ReadAStr( wxT( "Word" ), wxEmptyString, wxT( "coversearch" ) );
//
//    if( scanpath.IsEmpty() )
//    {
//        m_LibPaths = Config->ReadAStr( wxT( "LibPath" ), wxEmptyString, wxT( "libpaths" ) );
//
//        CheckSymLinks( m_LibPaths );
//    }
//
//    m_LastUpdate = Config->ReadNum( wxT( "LastUpdate" ), 0, wxT( "general" ) );
//    //guLogMessage( wxT( "LastUpdate: %s" ), LastTime.Format().c_str() );
//    m_ScanAddPlayLists = Config->ReadBool( wxT( "ScanAddPlayLists" ), true, wxT( "general" ) );
//    m_ScanEmbeddedCovers = Config->ReadBool( wxT( "ScanEmbeddedCovers" ), true, wxT( "general" ) );
//    m_ScanSymlinks = Config->ReadBool( wxT( "ScanSymlinks" ), false, wxT( "general" ) );
//
//    if( Create() == wxTHREAD_NO_ERROR )
//    {
//        SetPriority( WXTHREAD_DEFAULT_PRIORITY + 10 );
//        Run();
//    }
//}

// -------------------------------------------------------------------------------- //
guLibUpdateThread::guLibUpdateThread( guMediaViewer * mediaviewer, int gaugeid, const wxString &scanpath )
{
    m_MediaViewer = mediaviewer;
//    m_LibPanel = mediaviewer->GetLibPanel();
    m_Db = mediaviewer->GetDb();
    m_MainFrame = mediaviewer->GetMainFrame();
    m_GaugeId = gaugeid;
    m_ScanPath = scanpath;

    m_CoverSearchWords = mediaviewer->GetCoverWords();

    if( scanpath.IsEmpty() )
    {
        m_LibPaths = mediaviewer->GetPaths();

        CheckSymLinks( m_LibPaths );
    }

    m_LastUpdate = mediaviewer->GetLastUpdate();
    m_ScanAddPlayLists = mediaviewer->GetScanPlaylists();
    m_ScanEmbeddedCovers = mediaviewer->GetScanEmbeddedCovers();
    m_ScanSymlinks = mediaviewer->GetScanFollowSymLinks();

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY + 10 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guLibUpdateThread::~guLibUpdateThread()
{
//    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_LIBRARY_UPDATED );
    if( !TestDestroy() )
    {
        if( m_MediaViewer )
        {
            m_MediaViewer->UpdateFinished();
        }
//        else
//        {
//            if( m_LibPanel )
//            {
//                m_LibPanel->SetLastUpdate();
//            }
//            else
//            {
//                guConfig * Config = ( guConfig * ) guConfig::Get();
//                wxDateTime Now = wxDateTime::Now();
//                Config->WriteNum( wxT( "LastUpdate" ), Now.GetTicks(), wxT( "general" ) );
//                Config->Flush();
//            }
//
//            //event.SetEventObject( ( wxObject * ) this );
//            event.SetClientData( ( void * ) m_LibPanel );
//            wxPostEvent( m_MainFrame, event );
//        }
    }
    //
//    event.SetId( ID_STATUSBAR_GAUGE_REMOVE );
    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_STATUSBAR_GAUGE_REMOVE );
    event.SetInt( m_GaugeId );
    wxPostEvent( m_MainFrame, event );
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
    int Index;
    int Count;
    int LastIndex;
    wxCommandEvent evtup( wxEVT_COMMAND_MENU_SELECTED, ID_STATUSBAR_GAUGE_UPDATE );
    evtup.SetInt( m_GaugeId );

    wxCommandEvent evtmax( wxEVT_COMMAND_MENU_SELECTED, ID_STATUSBAR_GAUGE_SETMAX );
    evtmax.SetInt( m_GaugeId );

    if( m_ScanPath.IsEmpty() )
    {
        Count = m_LibPaths.Count();
        if( !Count )
        {
            guLogError( wxT( "No library directories to scan" ) );
            return 0;
        }

        // For every directory in the library scan for new files and add them to m_TrackFiles
        Index = 0;
        while( !TestDestroy() && ( Index < Count ) )
        {
            guLogMessage( wxT( "Doing Library Update in %s" ), m_LibPaths[ Index ].c_str() );
            ScanDirectory( m_LibPaths[ Index ] );
            Index++;
        }
    }
    else
    {
        ScanDirectory( m_ScanPath, true );
    }

    bool AllowGenres = m_MediaViewer->GetMediaCollection()->m_EmbeddMetadata;
    // For every new track file update it in the database
    Count = m_TrackFiles.Count();
    if( Count )
    {
        m_Db->ExecuteUpdate( wxT( "BEGIN TRANSACTION;" ) );

        Index = 0;
        evtmax.SetExtraLong( Count );
        wxPostEvent( m_MainFrame, evtmax );
        LastIndex = -1;
        while( !TestDestroy() )
        {
            //guLogMessage( wxT( "%i - %i" ), Index, Count );
            if( ( Index >= Count ) )
                break;

            //guLogMessage( wxT( "Scanning: '%s'" ), m_TrackFiles[ Index ].c_str() );
            m_Db->ReadFileTags( m_TrackFiles[ Index ], AllowGenres );
            //Sleep( 1 );
            Index++;
            if( Index > LastIndex )
            {
                evtup.SetExtraLong( Index );
                wxPostEvent( m_MainFrame, evtup );
                LastIndex = Index + 5;
            }
        }

        m_Db->ExecuteUpdate( wxT( "COMMIT TRANSACTION;" ) );
    }


    wxString CoverName = m_MediaViewer->GetCoverName( wxNOT_FOUND ) + wxT( ".jpg" );
    //int CoverType = m_MediaViewer->GetCoverType();
    int CoverMaxSize = m_MediaViewer->GetCoverMaxSize();

    Count = m_ImageFiles.Count();
    if( Count )
    {
        m_Db->ExecuteUpdate( wxT( "BEGIN TRANSACTION;" ) );
        Index = 0;
        evtmax.SetExtraLong( Count );
        wxPostEvent( m_MainFrame, evtmax );
        LastIndex = -1;
        while( !TestDestroy() )
        {
            //guLogMessage( wxT( "%i - %i" ), Index, Count );
            if( ( Index >= Count ) )
                break;

            m_Db->UpdateImageFile( m_ImageFiles[ Index ].ToUTF8(), CoverName.ToUTF8(), wxBITMAP_TYPE_JPEG, CoverMaxSize );
            Index++;
            if( Index > LastIndex )
            {
                evtup.SetExtraLong( Index );
                wxPostEvent( m_MainFrame, evtup );
                LastIndex = Index + 5;
            }
        }

        m_Db->ExecuteUpdate( wxT( "COMMIT TRANSACTION;" ) );
    }

    Count = m_PlayListFiles.Count();
    if( Count )
    {
        m_Db->ExecuteUpdate( wxT( "BEGIN TRANSACTION;" ) );

        Index = 0;
        evtmax.SetExtraLong( Count );
        wxPostEvent( m_MainFrame, evtmax );
        LastIndex = -1;
        while( !TestDestroy() )
        {
            //guLogMessage( wxT( "%i - %i" ), Index, Count );
            if( ( Index >= Count ) )
                break;

            guPlayListFile PlayList( m_PlayListFiles[ Index ] );
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
                    if( m_Db->GetStaticPlayList( m_PlayListFiles[ Index ] ) == wxNOT_FOUND )
                    {
                        m_Db->CreateStaticPlayList( m_PlayListFiles[ Index ], PlayListIds );

                        wxCommandEvent evt( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYLIST_UPDATED );
                        evt.SetClientData( ( void * ) m_MediaViewer );
                        wxPostEvent( m_MainFrame, evt );
                    }
                }
            }

            Index++;
            if( Index > LastIndex )
            {
                evtup.SetExtraLong( Index );
                wxPostEvent( m_MainFrame, evtup );
                LastIndex = Index + 5;
            }
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
guLibCleanThread::guLibCleanThread( guMediaViewer * mediaviewer )
{
    m_MediaViewer = mediaviewer;
    m_Db = mediaviewer->GetDb();
    m_MainFrame = mediaviewer->GetMainFrame();

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
        //event.SetEventObject( ( wxObject * ) this );
        //event.SetClientData( ( void * ) m_LibPanel );
        wxPostEvent( m_MediaViewer, event );
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

    wxArrayString LibPaths = m_MediaViewer->GetPaths();

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
