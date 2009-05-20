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
#include "MainApp.h"

#include "Images.h"
#include "MainFrame.h"
#include "Config.h"
#include "Utils.h"

#include <wx/curl/base.h>
#include <wx/image.h>
#include <wx/tooltip.h>
#include <wx/stdpaths.h>

IMPLEMENT_APP(guMainApp);

// -------------------------------------------------------------------------------- //
guMainApp::guMainApp() : wxApp()
{
    if( !wxDirExists( wxGetHomeDir() + wxT( "/.guayadeque" ) ) )
    {
        wxMkdir( wxGetHomeDir() + wxT( "/.guayadeque" ), 0770 );
        guLogMessage( wxT( "Created the configuration directory" ) );
    }
    m_Config = new guConfig();
    guConfig::Set( m_Config );

}

// -------------------------------------------------------------------------------- //
guMainApp::~guMainApp()
{
    // config
    if( m_Config )
      delete m_Config;

    if( m_SingleInstanceChecker )
        delete m_SingleInstanceChecker;
}


// -------------------------------------------------------------------------------- //
bool guMainApp::OnInit()
{
    wxLog::SetActiveTarget( new wxLogStderr );

    const wxString AppName = wxString::Format( wxT( "guayadeque-%s" ), wxGetUserId().c_str() );
    m_SingleInstanceChecker = new wxSingleInstanceChecker( AppName );
    if( m_SingleInstanceChecker->IsAnotherRunning() )
    {
        guLogError( wxT( "Another program instance is already running, aborting." ) );
        return false;
    }

//    // Init the sockets
//    wxSocketBase::Initialize();

    // Init all image handlers
    wxInitAllImageHandlers();

    // Init the wxCurl Lib
    wxCurlBase::Init();

    //
    if( m_Locale.Init( wxLANGUAGE_DEFAULT,
                     /*wxLOCALE_LOAD_DEFAULT |*/ wxLOCALE_CONV_ENCODING ) )
    {
        m_Locale.AddCatalogLookupPathPrefix( wxT( "/usr/share/locale" ) );
        m_Locale.AddCatalog( wxT( "guayadeque" ) );
        guLogMessage( wxT( "Initialized locale ( %s )" ), m_Locale.GetName().c_str() );
    }
    else
    {
        int LangId = wxLocale::GetSystemLanguage();
        const wxLanguageInfo * LangInfo = wxLocale::GetLanguageInfo( LangId );
        if( LangInfo )
        {
            guLogError( wxT( "Could not initialize the translations engine for ( %s )" ), LangInfo->CanonicalName.c_str() );
        }
        else
        {
            guLogError( wxT( "Could not initialize the translations engine for (%d)" ), LangId );
        }
        wxStandardPaths StdPaths;
        guLogError( wxT( "Locale directory '%s'" ), StdPaths.GetLocalizedResourcesDir( wxT( "es_ES" ), wxStandardPaths::ResourceCat_Messages).c_str() );
    }

    // Enable tooltips
    wxToolTip::Enable( true );


    // If enabled Show the Splash Screen on Startup
    guSplashFrame * SplashFrame = NULL;
    if( m_Config->ReadBool( wxT( "ShowSplashScreen" ), true, wxT( "General" ) ) )
    {
        SplashFrame = new guSplashFrame( 0 );
        if( !SplashFrame )
            guLogError( wxT( "Could not create splash object" ) );
        SplashFrame->Show( true );
        wxYield();
    }

    // Initialize the MainFrame object
    guMainFrame* Frame = new guMainFrame( 0 );
    wxIcon MainIcon;
    MainIcon.CopyFromBitmap( wxBitmap( guImage_guayadeque ) );
    Frame->SetIcon( MainIcon );

    // If Minimize is enabled minimized or hide it if Taskbar Icon is enabled
    if( m_Config->ReadBool( wxT( "StartMinimized" ), false, wxT( "General" ) ) )
    {
        if( m_Config->ReadBool( wxT( "ShowTaskBarIcon" ), false, wxT( "General" ) ) &&
            m_Config->ReadBool( wxT( "CloseToTaskBar" ), false, wxT( "General" ) ) )
        {
            Frame->Show( false );
            //Frame->Hide();
        }
        else
        {
            Frame->Iconize( true );
        }
    }
    else
    {
        Frame->Show();
    }

    SetTopWindow( Frame );

    return true;
}

// -------------------------------------------------------------------------------- //
int guMainApp::OnExit()
{
    // Shutdown the wxCurl Lib
    wxCurlBase::Shutdown();

    return 0;
}

// -------------------------------------------------------------------------------- //
