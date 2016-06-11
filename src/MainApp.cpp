// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2016 J.Rios anonbeat@gmail.com
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
//    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#include "MainApp.h"

#include "Config.h"
#include "Curl.h"
#include "Images.h"
#include "MainFrame.h"
#include "mpris.h"
#include "SplashWin.h"
#include "Settings.h"
#include "Utils.h"

#include "wx/clipbrd.h"
#include <wx/image.h>
#include <wx/tooltip.h>
#include <wx/stdpaths.h>
#include <wx/utils.h>

#include <wx/debugrpt.h>

IMPLEMENT_APP( Guayadeque::guMainApp );

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
guMainApp::guMainApp() : wxApp()
{
//    m_Db = NULL;
    m_DbCache = NULL;

#if wxUSE_ON_FATAL_EXCEPTION    // Thanks TheBigRed
        wxHandleFatalExceptions();
#endif

    if( !wxDirExists( guPATH_CONFIG ) )
    {
        wxMkdir( guPATH_CONFIG, 0770 );
        guLogMessage( wxT( "Created the configuration folder" ) );
    }

    if( !wxDirExists( guPATH_LYRICS ) )
    {
        wxMkdir( guPATH_LYRICS, 0770 );
        guLogMessage( wxT( "Created the lyrics folder" ) );
    }

    if( !wxDirExists( guPATH_COLLECTIONS ) )
    {
        wxMkdir( guPATH_COLLECTIONS, 0770 );
        guLogMessage( wxT( "Created the collections folder" ) );
    }

    if( !wxDirExists( guPATH_RADIOS ) )
    {
        wxMkdir( guPATH_RADIOS, 0770 );
        guLogMessage( wxT( "Created the Radios folder" ) );
    }

    if( !wxDirExists( guPATH_JAMENDO ) )
    {
        wxMkdir( guPATH_JAMENDO, 0770 );
        wxMkdir( guPATH_JAMENDO_COVERS, 0770 );
        guLogMessage( wxT( "Created the Jamendo folder" ) );
    }

    if( !wxDirExists( guPATH_MAGNATUNE ) )
    {
        wxMkdir( guPATH_MAGNATUNE, 0770 );
        wxMkdir( guPATH_MAGNATUNE_COVERS, 0770 );
        guLogMessage( wxT( "Created the Magnatune folder" ) );
    }

    if( !wxDirExists( guPATH_PODCASTS ) )
    {
        wxMkdir( guPATH_PODCASTS, 0770 );
        guLogMessage( wxT( "Created the Podcasts folder" ) );
    }

    if( !wxDirExists( guPATH_DEVICES ) )
    {
        wxMkdir( guPATH_DEVICES, 0770 );
        guLogMessage( wxT( "Created the Devices folder" ) );
    }

    if( !wxDirExists( guPATH_LAYOUTS ) )
    {
        wxMkdir( guPATH_LAYOUTS, 0770 );
        guLogMessage( wxT( "Created the Layouts folder" ) );
    }

    if( !wxFileExists( guPATH_CONFIG_FILENAME ) )
    {
        if( wxFileExists( wxT( "/usr/share/guayadeque/guayadeque.default.conf" ) ) )
        {
            wxCopyFile( wxT( "/usr/share/guayadeque/guayadeque.default.conf" ),
                        guPATH_CONFIG_FILENAME, false );
        }
        else if( wxFileExists( wxT( "/usr/local/share/guayadeque/guayadeque.default.conf" ) ) )
        {
            wxCopyFile( wxT( "/usr/local/share/guayadeque/guayadeque.default.conf" ),
                        guPATH_CONFIG_FILENAME, false );
        }
        guLogMessage( wxT( "Created the default configuration file" ) );
    }

    if( !wxFileExists( guPATH_EQUALIZERS_FILENAME ) )
    {
        if( wxFileExists( wxT( "/usr/share/guayadeque/equalizers.default.conf" ) ) )
        {
            wxCopyFile( wxT( "/usr/share/guayadeque/equalizers.default.conf" ),
                        guPATH_EQUALIZERS_FILENAME, false );
        }
        else if( wxFileExists( wxT( "/usr/local/share/guayadeque/equalizers.default.conf" ) ) )
        {
            wxCopyFile( wxT( "/usr/local/share/guayadeque/equalizers.default.conf" ),
                        guPATH_EQUALIZERS_FILENAME, false );
        }
        guLogMessage( wxT( "Created the default equalizers file" ) );
    }

    if( !wxFileExists( guPATH_LYRICS_SOURCES_FILENAME ) )
    {
        if( wxFileExists( wxT( "/usr/share/guayadeque/lyrics_sources.xml" ) ) )
        {
            wxCopyFile( wxT( "/usr/share/guayadeque/lyrics_sources.xml" ),
                        guPATH_LYRICS_SOURCES_FILENAME, false );
        }
        else if( wxFileExists( wxT( "/usr/local/share/guayadeque/lyrics_sources.xml" ) ) )
        {
            wxCopyFile( wxT( "/usr/local/share/guayadeque/lyrics_sources.xml" ),
                        guPATH_LYRICS_SOURCES_FILENAME, false );
        }
        guLogMessage( wxT( "Created the default lyrics sources file" ) );
    }

    m_Config = new guConfig();
    m_Config->Set( m_Config );

}

// -------------------------------------------------------------------------------- //
guMainApp::~guMainApp()
{
    if( m_SingleInstanceChecker )
        delete m_SingleInstanceChecker;

    if( m_DbCache )
    {
        m_DbCache->Close();
        delete m_DbCache;
    }

    // config
    if( m_Config )
      delete m_Config;
}


// -------------------------------------------------------------------------------- //
// Its done this way to avoid the warning of temporary address
void inline Append_String( DBusMessageIter * iter, const char * str )
{
    dbus_message_iter_append_basic( iter, DBUS_TYPE_STRING, &str );
}

// -------------------------------------------------------------------------------- //
bool SendFilesByMPRIS( const int argc, wxChar * argv[] )
{
    DBusError dberr;
    DBusConnection * dbconn;
    DBusMessage * dbmsg, * dbreply;
    DBusMessageIter dbiter;
    DBusMessageIter dbitertracks;

    dbus_error_init( &dberr );
    dbconn = dbus_bus_get( DBUS_BUS_SESSION, &dberr );

    if( dbus_error_is_set( &dberr ) )
    {
         printf( "getting session bus failed: %s\n", dberr.message );
         dbus_error_free( &dberr );
         return false;
    }


    dbmsg = dbus_message_new_method_call( GUAYADEQUE_MPRIS_SERVICENAME,
                                          GUAYADEQUE_MPRIS_TRACKLIST_PATH,
                                          GUAYADEQUE_MPRIS_INTERFACE,
                                          "AddTracks" );
    if( dbmsg == NULL )
    {
         guLogError( wxT( "Couldn’t create a DBusMessage" ) );
         return false;
    }

    dbus_message_iter_init_append( dbmsg, &dbiter );
    dbus_message_iter_open_container( &dbiter, DBUS_TYPE_ARRAY, "s", &dbitertracks );

    wxString FilePath;
    int index;
    for( index = 1; index < argc; index++ )
    {
        FilePath = argv[ index ];
        //guLogMessage( wxT( "Trying to add file '%s'" ), argv[ index ] );

        Append_String( &dbitertracks, FilePath.char_str() );
    }

    dbus_message_iter_close_container( &dbiter, &dbitertracks );

    dbus_bool_t PlayTrack = false;
    dbus_message_iter_append_basic( &dbiter, DBUS_TYPE_BOOLEAN, &PlayTrack );

    dbreply = dbus_connection_send_with_reply_and_block( dbconn, dbmsg, 5000, &dberr );
    if( dbus_error_is_set( &dberr ) )
    {
          guLogMessage( wxT( "Error adding files: '%s'" ), wxString( dberr.message, wxConvUTF8 ).c_str() );
          dbus_message_unref( dbmsg );
          dbus_error_free( &dberr );
          return false;
    }

    if( dbreply )
        dbus_message_unref( dbreply );

    /* Don’t need this anymore */
    dbus_message_unref( dbmsg );

    dbus_connection_unref( dbconn );

    return true;
}

// -------------------------------------------------------------------------------- //
bool MakeWindowVisible( void )
{
    DBusError dberr;
    DBusConnection * dbconn;
    DBusMessage * dbmsg, * dbreply;

    dbus_error_init( &dberr );
    dbconn = dbus_bus_get( DBUS_BUS_SESSION, &dberr );

    if( dbus_error_is_set( &dberr ) )
    {
         printf( "getting session bus failed: %s\n", dberr.message );
         dbus_error_free( &dberr );
         return false;
    }


    dbmsg = dbus_message_new_method_call( "org.mpris.MediaPlayer2.guayadeque",
                                          "/org/mpris/MediaPlayer2",
                                          "org.mpris.MediaPlayer2",
                                          "Raise" );
    if( dbmsg == NULL )
    {
         guLogError( wxT( "Couldn’t create a DBusMessage" ) );
         return false;
    }

    dbreply = dbus_connection_send_with_reply_and_block( dbconn, dbmsg, 5000, &dberr );
    if( dbus_error_is_set( &dberr ) )
    {
          guLogMessage( wxT( "Error showing window" ) );
          dbus_message_unref( dbmsg );
          dbus_error_free( &dberr );
          return false;
    }

    if( dbreply )
        dbus_message_unref( dbreply );

    /* Don’t need this anymore */
    dbus_message_unref( dbmsg );

    dbus_connection_unref( dbconn );

    return true;
}

// -------------------------------------------------------------------------------- //
bool guMainApp::OnInit()
{
    guRandomInit();

    wxLog::SetActiveTarget( new wxLogStderr( stdout ) );

    const wxString AppName = wxString::Format( wxT( ".guayadeque/.guayadeque-%s" ), wxGetUserId().c_str() );
    //guLogMessage( wxT( "Init: %s" ), AppName.c_str() );
    m_SingleInstanceChecker = new wxSingleInstanceChecker( AppName );
    if( m_SingleInstanceChecker->IsAnotherRunning() )
    {
        if( argc > 1 )
        {
            int RetryCnt = 0;
            while( RetryCnt++ < 25 )
            {
                if( SendFilesByMPRIS( argc, argv ) )
                {
                    break;
                }
                wxMilliSleep( 100 );
            }
        }

        MakeWindowVisible();

        guLogMessage( wxT( "Another program instance is already running, aborting." ) );
        return false;
    }

    // Init all image handlers
    wxInitAllImageHandlers();

    // If enabled Show the Splash Screen on Startup
    if( m_Config->ReadBool( wxT( "ShowSplashScreen" ), true, wxT( "general" ) ) )
    {
        guSplashFrame * SplashFrame = new guSplashFrame( 0 );
        if( !SplashFrame )
            guLogError( wxT( "Could not create splash object" ) );
        SplashFrame->Show( true );
        wxYield();
        //wxMilliSleep( 300 );
    }

    // Use the primary clipboard which is shared with other applications
    wxTheClipboard->UsePrimarySelection( false );

    // Init the Curl Library
    guCurl::CurlInit();

    int LangId = m_Config->ReadNum( wxT( "Language" ), wxLANGUAGE_DEFAULT, wxT( "general" ) );
    if( m_Locale.Init( LangId ) )
    {
        m_Locale.AddCatalogLookupPathPrefix( wxT( "/usr/share/locale" ) );
        m_Locale.AddCatalog( wxT( "guayadeque" ) );
        guLogMessage( wxT( "Initialized locale ( %s )" ), m_Locale.GetName().c_str() );
    }
    else
    {
        const wxLanguageInfo * LangInfo = wxLocale::GetLanguageInfo( LangId );
        if( LangInfo )
        {
            guLogError( wxT( "Could not initialize the translations engine for ( %s )" ), LangInfo->CanonicalName.c_str() );
            guLogError( wxT( "Locale directory '%s'" ), wxStandardPaths::Get().GetLocalizedResourcesDir( LangInfo->CanonicalName, wxStandardPaths::ResourceCat_Messages).c_str() );
        }
        else
        {
            guLogError( wxT( "Could not initialize the translations engine for (%d)" ), LangId );
        }
    }

    // Enable tooltips
    wxToolTip::Enable( true );

    m_DbCache = new guDbCache( guPATH_DBCACHE );
    if( !m_DbCache )
    {
        guLogError( wxT( "Could not open the guayadeque cache database" ) );
    }

    m_DbCache->SetDbCache();

    // Initialize the MainFrame object
    guMainFrame * Frame = new guMainFrame( 0, m_DbCache );
    SetTopWindow( Frame );
    //Frame->SetMainFrame();
    wxIcon MainIcon;
    MainIcon.CopyFromBitmap( guImage( guIMAGE_INDEX_guayadeque ) );
    Frame->SetIcon( MainIcon );

    // If Minimize is enabled minimized or hide it if Taskbar Icon is enabled
    if( m_Config->ReadBool( wxT( "StartMinimized" ), false, wxT( "general" ) ) )
    {
        if( m_Config->ReadBool( wxT( "ShowTaskBarIcon" ), false, wxT( "general" ) ) &&
            m_Config->ReadBool( wxT( "CloseToTaskBar" ), false, wxT( "general" ) ) )
        {
            Frame->Show( false );
            //Frame->Hide();
        }
        else
        {
            Frame->Show();
            Frame->Iconize( true );
        }
    }
    else
    {
        Frame->Show();
    }

    return true;
}

// -------------------------------------------------------------------------------- //
int guMainApp::OnExit()
{
    // Free any resources used by Curl Library
    guCurl::CurlDone();

    return 0;
}

// -------------------------------------------------------------------------------- //
void guMainApp::OnFatalException()
{
    wxDebugReport Report;
    wxDebugReportPreviewStd Preview;

    Report.AddAll();

    if( Preview.Show( Report ) )
        Report.Process();
}

}

// -------------------------------------------------------------------------------- //
