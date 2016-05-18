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
//    the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#include "Accelerators.h"

#include "Utils.h"
#include "Config.h"
#include "Commands.h"

    // Build the Accelerators
static wxArrayInt guAccelCmdIds;
static wxArrayInt guAccelKeys;

// -------------------------------------------------------------------------------- //
void guAccelInit( void )
{
    if( !guAccelCmdIds.Count() )
    {
        guAccelCmdIds.Add( ID_MENU_LIBRARY_ADD_PATH );
        guAccelCmdIds.Add( ID_MAINFRAME_SETAUDIOSCROBBLE );
        guAccelCmdIds.Add( ID_PLAYER_PLAYLIST_CLEAR );
        guAccelCmdIds.Add( ID_MENU_VIEW_CLOSEWINDOW );
        guAccelCmdIds.Add( ID_MENU_COMMUNITY );
        guAccelCmdIds.Add( ID_PLAYER_PLAYLIST_EDITLABELS );
        guAccelCmdIds.Add( ID_PLAYER_PLAYLIST_EDITTRACKS );
        guAccelCmdIds.Add( ID_TRACKS_ENQUEUE_AFTER_ALL );
        guAccelCmdIds.Add( ID_TRACKS_ENQUEUE_AFTER_TRACK );
        guAccelCmdIds.Add( ID_TRACKS_ENQUEUE_AFTER_ALBUM );
        //
        guAccelCmdIds.Add( ID_TRACKS_ENQUEUE_AFTER_ARTIST );
        guAccelCmdIds.Add( ID_MENU_VIEW_FULLSCREEN );
        guAccelCmdIds.Add( ID_MAINFRAME_SETFORCEGAPLESS );
        guAccelCmdIds.Add( ID_MENU_HELP );
        guAccelCmdIds.Add( ID_TRACKS_PLAY );
        guAccelCmdIds.Add( ID_TRACKS_PLAYALL );
        guAccelCmdIds.Add( ID_PLAYERPANEL_PLAY );
        guAccelCmdIds.Add( ID_MENU_PLAY_STREAM );
        guAccelCmdIds.Add( ID_MENU_PREFERENCES );
        guAccelCmdIds.Add( ID_MENU_QUIT );
        //
        guAccelCmdIds.Add( ID_PLAYER_PLAYLIST_RANDOMPLAY );
        guAccelCmdIds.Add( ID_PLAYERPANEL_SETRATING_0 );
        guAccelCmdIds.Add( ID_PLAYERPANEL_SETRATING_1 );
        guAccelCmdIds.Add( ID_PLAYERPANEL_SETRATING_2 );
        guAccelCmdIds.Add( ID_PLAYERPANEL_SETRATING_3 );
        guAccelCmdIds.Add( ID_PLAYERPANEL_SETRATING_4 );
        guAccelCmdIds.Add( ID_PLAYERPANEL_SETRATING_5 );
        guAccelCmdIds.Add( ID_PLAYER_PLAYLIST_REPEATPLAYLIST );
        guAccelCmdIds.Add( ID_PLAYER_PLAYLIST_REPEATTRACK );
        guAccelCmdIds.Add( ID_MENU_LAYOUT_CREATE );
        //
        guAccelCmdIds.Add( ID_PLAYER_PLAYLIST_SAVE );
        guAccelCmdIds.Add( ID_PLAYER_PLAYLIST_SEARCH );
        guAccelCmdIds.Add( ID_MENU_VIEW_ALBUMBROWSER );
        guAccelCmdIds.Add( ID_MENU_VIEW_MAIN_SHOWCOVER );
        guAccelCmdIds.Add( ID_MENU_VIEW_PLAYER_PLAYLIST );
        guAccelCmdIds.Add( ID_MENU_VIEW_FILEBROWSER );
        guAccelCmdIds.Add( ID_MENU_VIEW_PLAYER_FILTERS );
        guAccelCmdIds.Add( ID_MENU_VIEW_LASTFM );
        guAccelCmdIds.Add( ID_MENU_VIEW_LIBRARY );
        guAccelCmdIds.Add( ID_MENU_VIEW_LYRICS );
        //
        guAccelCmdIds.Add( ID_MENU_VIEW_RADIO );
        guAccelCmdIds.Add( ID_MENU_VIEW_PLAYLISTS );
        guAccelCmdIds.Add( ID_MENU_VIEW_PODCASTS );
        guAccelCmdIds.Add( ID_MENU_VIEW_MAIN_LOCATIONS );
        guAccelCmdIds.Add( ID_MENU_VIEW_STATUSBAR );
        guAccelCmdIds.Add( ID_MENU_VIEW_PLAYER_VUMETERS );
        guAccelCmdIds.Add( ID_PLAYERPANEL_NEXTTRACK );
        guAccelCmdIds.Add( ID_PLAYERPANEL_NEXTALBUM );
        guAccelCmdIds.Add( ID_PLAYERPANEL_PREVTRACK );
        guAccelCmdIds.Add( ID_PLAYERPANEL_PREVALBUM );
        //
        guAccelCmdIds.Add( ID_PLAYER_PLAYLIST_SMARTPLAY );
        guAccelCmdIds.Add( ID_PLAYERPANEL_STOP );
        guAccelCmdIds.Add( ID_PLAYER_PLAYLIST_STOP_ATEND );
        guAccelCmdIds.Add( ID_MENU_UPDATE_COVERS );
        guAccelCmdIds.Add( ID_MENU_UPDATE_LIBRARY );
        guAccelCmdIds.Add( ID_MENU_UPDATE_LIBRARYFORCED );
        guAccelCmdIds.Add( ID_MENU_UPDATE_PODCASTS );
        guAccelCmdIds.Add( ID_MENU_VOLUME_DOWN );
        guAccelCmdIds.Add( ID_MENU_VOLUME_UP );
    }

    guAccelOnConfigUpdated();
}

// -------------------------------------------------------------------------------- //
int guAccelGetActionNames( wxArrayString &actionnames )
{
	actionnames.Add( _( "Add Folder" ) );
	actionnames.Add( _( "Audio Scrobbling" ) );
	actionnames.Add( _( "Clear Playlist" ) );
	actionnames.Add( _( "Close Window" ) );
	actionnames.Add( _( "Community Forums" ) );
	actionnames.Add( _( "Edit Labels" ) );
	actionnames.Add( _( "Edit Tracks" ) );
    actionnames.Add( _( "Enqueue" ) );
    actionnames.Add( _( "Enqueue After Track" ) );
    actionnames.Add( _( "Enqueue After Album" ) );
    //
    actionnames.Add( _( "Enqueue After Artist" ) );
	actionnames.Add( _( "Full Screen" ) );
	actionnames.Add( _( "Gapless/Crossfading" ) );
	actionnames.Add( _( "Help" ) );
    actionnames.Add( _( "Play" ) );
    actionnames.Add( _( "Play All" ) );
	actionnames.Add( _( "Play/Pause" ) );
	actionnames.Add( _( "Play Stream" ) );
	actionnames.Add( _( "Preferences" ) );
	actionnames.Add( _( "Quit" ) );
	//
	actionnames.Add( _( "Randomize Playlist" ) );
	actionnames.Add( wxString( _( "Rating" ) ) + wxT( " ☆☆☆☆☆" ) );
	actionnames.Add( wxString( _( "Rating" ) ) + wxT( " ★☆☆☆☆" ) );
	actionnames.Add( wxString( _( "Rating" ) ) + wxT( " ★★☆☆☆" ) );
	actionnames.Add( wxString( _( "Rating" ) ) + wxT( " ★★★☆☆" ) );
	actionnames.Add( wxString( _( "Rating" ) ) + wxT( " ★★★★☆" ) );
	actionnames.Add( wxString( _( "Rating" ) ) + wxT( " ★★★★★" ) );
	actionnames.Add( _( "Repeat All Tracks" ) );
	actionnames.Add( _( "Repeat Current Track" ) );
	actionnames.Add( _( "Save Layout" ) );
	//
	actionnames.Add( _( "Save to Playlist" ) );
	actionnames.Add( _( "Search" ) );
	actionnames.Add( _( "Show Browser" ) );
	actionnames.Add( _( "Show Covers" ) );
	actionnames.Add( _( "Show Playlist" ) );
	actionnames.Add( _( "Show Files" ) );
	actionnames.Add( _( "Show Filters" ) );
	actionnames.Add( _( "Show Last.fm" ) );
	actionnames.Add( _( "Show Library" ) );
	actionnames.Add( _( "Show Lyrics" ) );
	//
	actionnames.Add( _( "Show Radio" ) );
	actionnames.Add( _( "Show Playlists" ) );
	actionnames.Add( _( "Show Podcasts" ) );
	actionnames.Add( _( "Show Sources" ) );
	actionnames.Add( _( "Show Status Bar" ) );
	actionnames.Add( _( "Show VU Meters" ) );
	actionnames.Add( _( "Skip Next" ) );
	actionnames.Add( _( "Skip Next Album" ) );
	actionnames.Add( _( "Skip Previous" ) );
	actionnames.Add( _( "Skip Previous Album" ) );
	//
	actionnames.Add( _( "Smart Play" ) );
	actionnames.Add( _( "Stop" ) );
	actionnames.Add( _( "Stop at End" ) );
	actionnames.Add( _( "Update Covers" ) );
	actionnames.Add( _( "Update" ) );
	actionnames.Add( _( "Update (Forced)" ) );
	actionnames.Add( _( "Update Podcasts" ) );
	actionnames.Add( _( "Volume Down" ) );
	actionnames.Add( _( "Volume Up" ) );

	return actionnames.Count();
}

// -------------------------------------------------------------------------------- //
int guAccelGetDefaultKeys( wxArrayInt &accelkeys )
{
    accelkeys.Add( 131140 );
    accelkeys.Add( 131156 );
    accelkeys.Add( 131199 );
    accelkeys.Add( 131159 );
    accelkeys.Add( 262484 );
    accelkeys.Add( 131148 );
    accelkeys.Add( 131141 );
    accelkeys.Add( 131085 );
    accelkeys.Add( 393229 );
    accelkeys.Add( 65549 );
    //
    accelkeys.Add( 327693 );
    accelkeys.Add( 350 );
    accelkeys.Add( 131143 );
    accelkeys.Add( 340 );
    accelkeys.Add( 13 );
    accelkeys.Add( 196621 );
    accelkeys.Add( 131104 );
    accelkeys.Add( 131151 );
    accelkeys.Add( 131421 );
    accelkeys.Add( 131153 );
    //
    accelkeys.Add( 131154 );
    accelkeys.Add( 131120 );
    accelkeys.Add( 131121 );
    accelkeys.Add( 131122 );
    accelkeys.Add( 131123 );
    accelkeys.Add( 131124 );
    accelkeys.Add( 131125 );
    accelkeys.Add( 342 );
    accelkeys.Add( 262486 );
    accelkeys.Add( 343 );
    //
    accelkeys.Add( 131155 );
    accelkeys.Add( 131142 );
    accelkeys.Add( 0 );
    accelkeys.Add( 0 );
    accelkeys.Add( 0 );
    accelkeys.Add( 0 );
    accelkeys.Add( 0 );
    accelkeys.Add( 0 );
    accelkeys.Add( 0 );
    accelkeys.Add( 0 );
    //
    accelkeys.Add( 0 );
    accelkeys.Add( 0 );
    accelkeys.Add( 0 );
    accelkeys.Add( 0 );
    accelkeys.Add( 351 );
    accelkeys.Add( 0 );
    accelkeys.Add( 131388 );
    accelkeys.Add( 393532 );
    accelkeys.Add( 131386 );
    accelkeys.Add( 393530 );
    //
    accelkeys.Add( 341 );
    accelkeys.Add( 196640 );
    accelkeys.Add( 393248 );
    accelkeys.Add( 131139 );
    accelkeys.Add( 131157 );
    accelkeys.Add( 393301 );
    accelkeys.Add( 131152 );
    accelkeys.Add( 131389 );
    accelkeys.Add( 131387 );

    return accelkeys.Count();
}

// -------------------------------------------------------------------------------- //
void guAccelOnConfigUpdated( void )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    guAccelKeys = Config->ReadANum( wxT( "AccelKey" ), 0, wxT( "accelerators" ) );
    if( guAccelKeys.Count() == 0 )
    {
        guAccelGetDefaultKeys( guAccelKeys );
    }

    while( guAccelKeys.Count() < guAccelCmdIds.Count() )
        guAccelKeys.Add( 0 );
}

// -------------------------------------------------------------------------------- //
wxString guAccelGetKeyCodeString( const int keycode )
{
    wxString KeyStr;

    if( keycode )
    {
        int Modifier = keycode >> 16;
        int KeyCode = keycode & 0xFFFF;
        if( Modifier )
        {
            if( Modifier & wxMOD_SHIFT )
            {
                KeyStr += wxString( _( "Shift" ) ) + wxT( "-" );
            }
            if( Modifier & wxMOD_CONTROL )
            {
                KeyStr += wxString( _( "Ctrl" ) ) + wxT( "-" );
            }
            if( Modifier & wxMOD_ALT )
            {
                KeyStr += wxString( _( "Alt" ) ) + wxT( "-" );
            }
            if( Modifier & wxMOD_META )
            {
                KeyStr += wxString( _( "Meta" ) ) + wxT( "-" );
            }
        }

        switch( KeyCode )
        {
            case WXK_NUMPAD0 :
            case WXK_NUMPAD1 :
            case WXK_NUMPAD2 :
            case WXK_NUMPAD3 :
            case WXK_NUMPAD4 :
            case WXK_NUMPAD5 :
            case WXK_NUMPAD6 :
            case WXK_NUMPAD7 :
            case WXK_NUMPAD8 :
            case WXK_NUMPAD9 :          KeyStr += wxString::Format( wxT( "%d" ), KeyCode - WXK_NUMPAD0 ); break;

            case WXK_BACK :             KeyStr += _( "Back" ); break;

            case WXK_ESCAPE :           KeyStr += _( "Escape" ); break;

            case WXK_SPACE :
            case WXK_NUMPAD_SPACE :     KeyStr += _( "Space" ); break;

            case WXK_TAB :
            case WXK_NUMPAD_TAB :       KeyStr += _( "Tab" ); break;

            case WXK_RETURN :           KeyStr += _( "Return" ); break;

            case WXK_NUMPAD_ENTER :     KeyStr += _( "Enter" ); break;

            case WXK_NUMPAD_F1 :
            case WXK_NUMPAD_F2 :
            case WXK_NUMPAD_F3 :
            case WXK_NUMPAD_F4 :        KeyStr += wxString::Format( wxT( "F%d" ), KeyCode- WXK_NUMPAD_F1 + 1 ); break;

            case WXK_F1 :
            case WXK_F2 :
            case WXK_F3 :
            case WXK_F4 :
            case WXK_F5 :
            case WXK_F6 :
            case WXK_F7 :
            case WXK_F8 :
            case WXK_F9 :
            case WXK_F10 :
            case WXK_F11 :
            case WXK_F12 :
            case WXK_F13 :
            case WXK_F14 :
            case WXK_F15 :
            case WXK_F16 :
            case WXK_F17 :
            case WXK_F18 :
            case WXK_F19 :
            case WXK_F20 :
            case WXK_F21 :
            case WXK_F22 :
            case WXK_F23 :
            case WXK_F24 :              KeyStr += wxString::Format( wxT( "F%d" ), KeyCode - WXK_F1 + 1 ); break;

            case WXK_LEFT :
            case WXK_NUMPAD_LEFT :      KeyStr += _( "Left" ); break;

            case WXK_UP :
            case WXK_NUMPAD_UP :        KeyStr += _( "Up" ); break;

            case WXK_RIGHT :
            case WXK_NUMPAD_RIGHT :     KeyStr += _( "Right" ); break;

            case WXK_DOWN:
            case WXK_NUMPAD_DOWN :      KeyStr += _( "Down" ); break;

            case WXK_NUMPAD_HOME :      KeyStr += _( "Home" ); break;
        #if not wxCHECK_VERSION(2, 8, 0)
            case WXK_PRIOR:
            case WXK_NUMPAD_PRIOR :     KeyStr += _( "Page Up" ); break;

            case WXK_NEXT:
            case WXK_NUMPAD_NEXT :      KeyStr += _( "Page Down" ); break;
        #endif

            case WXK_PAGEUP :
            case WXK_NUMPAD_PAGEUP :    KeyStr += _( "Page Up"); break;

            case WXK_PAGEDOWN :
            case WXK_NUMPAD_PAGEDOWN :  KeyStr += _( "Page Down" ); break;

            case WXK_END :
            case WXK_NUMPAD_END :       KeyStr += _( "End" ); break;

            case WXK_NUMPAD_BEGIN :     KeyStr += _( "Begin" ); break;

            case WXK_NUMPAD_INSERT :    KeyStr += _( "Insert" ); break;

            case WXK_DELETE:
            case WXK_NUMPAD_DELETE :    KeyStr += _( "Delete" ); break;

            case WXK_NUMPAD_EQUAL :     KeyStr += wxT( "=" ); break;

            case WXK_MULTIPLY:
            case WXK_NUMPAD_MULTIPLY :  KeyStr += wxT( "*" ); break;

            case WXK_ADD :
            case WXK_NUMPAD_ADD :       KeyStr += wxT( "+" ); break;

            case WXK_SEPARATOR:
            case WXK_NUMPAD_SEPARATOR : KeyStr += _( "Separator" ); break;

            case WXK_SUBTRACT:
            case WXK_NUMPAD_SUBTRACT :  KeyStr += wxT( "-" ); break;

            case WXK_DECIMAL :
            case WXK_NUMPAD_DECIMAL :   KeyStr += wxT( "." ); break;

            case WXK_DIVIDE :
            case WXK_NUMPAD_DIVIDE :    KeyStr += wxT( "/" ); break;



            case WXK_SELECT :           KeyStr += _( "Select" ); break;
            case WXK_PRINT :            KeyStr += _( "Print" ); break;
            case WXK_EXECUTE :          KeyStr += _( "Execute" ); break;
            case WXK_SNAPSHOT :         KeyStr += _( "Snapshot" ); break;
            case WXK_INSERT :           KeyStr += _( "Insert" ); break;
            case WXK_HELP :             KeyStr += _( "Help" ); break;
            case WXK_CANCEL :           KeyStr += _( "Cancel" ); break;
            case WXK_MENU :             KeyStr += _( "Menu" ); break;
            case WXK_CAPITAL :          KeyStr += _( "Capital" ); break;
            case WXK_HOME :             KeyStr += _( "Home" ); break;


            default :
                if( wxIsalnum( KeyCode ) || wxIsprint( KeyCode ) ) // ASCII chars...
                {
                      KeyStr += ( wxChar ) KeyCode;
                      break;

                }
        }
    }

    return KeyStr;
}


// -------------------------------------------------------------------------------- //
wxString guAccelGetKeyCodeMenuString( const int keycode )
{
    wxString KeyStr;
    if( keycode )
    {
        KeyStr += wxT( "\t" );
        int Modifier = keycode >> 16;
        int KeyCode = keycode & 0xFFFF;
        if( Modifier )
        {
            if( Modifier & wxMOD_SHIFT )
            {
                KeyStr += wxT( "Shift-" );
            }
            if( Modifier & wxMOD_CONTROL )
            {
                KeyStr += wxT( "Ctrl-" );
            }
            if( Modifier & wxMOD_ALT )
            {
                KeyStr += wxT( "Alt-" );
            }
            if( Modifier & wxMOD_META )
            {
                KeyStr += wxT( "Meta-" );
            }
        }

        switch( KeyCode )
        {
            case WXK_NUMPAD0 :
            case WXK_NUMPAD1 :
            case WXK_NUMPAD2 :
            case WXK_NUMPAD3 :
            case WXK_NUMPAD4 :
            case WXK_NUMPAD5 :
            case WXK_NUMPAD6 :
            case WXK_NUMPAD7 :
            case WXK_NUMPAD8 :
            case WXK_NUMPAD9 :          KeyStr += wxString::Format( wxT( "%d" ), KeyCode - WXK_NUMPAD0 ); break;

            case WXK_BACK :             KeyStr += wxT( "Back" ); break;

            case WXK_ESCAPE :           KeyStr += wxT( "Escape" ); break;

            case WXK_SPACE :
            case WXK_NUMPAD_SPACE :     KeyStr += wxT( "Space" ); break;

            case WXK_TAB :
            case WXK_NUMPAD_TAB :       KeyStr += wxT( "Tab" ); break;

            case WXK_RETURN :           KeyStr += wxT( "Return" ); break;

            case WXK_NUMPAD_ENTER :     KeyStr += wxT( "Enter" ); break;

            case WXK_NUMPAD_F1 :
            case WXK_NUMPAD_F2 :
            case WXK_NUMPAD_F3 :
            case WXK_NUMPAD_F4 :        KeyStr += wxString::Format( wxT( "F%d" ), KeyCode- WXK_NUMPAD_F1 + 1 ); break;

            case WXK_F1 :
            case WXK_F2 :
            case WXK_F3 :
            case WXK_F4 :
            case WXK_F5 :
            case WXK_F6 :
            case WXK_F7 :
            case WXK_F8 :
            case WXK_F9 :
            case WXK_F10 :
            case WXK_F11 :
            case WXK_F12 :
            case WXK_F13 :
            case WXK_F14 :
            case WXK_F15 :
            case WXK_F16 :
            case WXK_F17 :
            case WXK_F18 :
            case WXK_F19 :
            case WXK_F20 :
            case WXK_F21 :
            case WXK_F22 :
            case WXK_F23 :
            case WXK_F24 :              KeyStr += wxString::Format( wxT( "F%d" ), KeyCode - WXK_F1 + 1 ); break;

            case WXK_LEFT :
            case WXK_NUMPAD_LEFT :      KeyStr += wxT( "Left" ); break;

            case WXK_UP :
            case WXK_NUMPAD_UP :        KeyStr += wxT( "Up" ); break;

            case WXK_RIGHT :
            case WXK_NUMPAD_RIGHT :     KeyStr += wxT( "Right" ); break;

            case WXK_DOWN:
            case WXK_NUMPAD_DOWN :      KeyStr += wxT( "Down" ); break;

            case WXK_NUMPAD_HOME :      KeyStr += wxT( "Home" ); break;
        #if not wxCHECK_VERSION(2, 8, 0)
            case WXK_PRIOR:
            case WXK_NUMPAD_PRIOR :     KeyStr += wxT( "PgUp" ); break;

            case WXK_NEXT:
            case WXK_NUMPAD_NEXT :      KeyStr += wxT( "PgDown" ); break;
        #endif

            case WXK_PAGEUP :
            case WXK_NUMPAD_PAGEUP :    KeyStr += wxT( "PgUp"); break;

            case WXK_PAGEDOWN :
            case WXK_NUMPAD_PAGEDOWN :  KeyStr += wxT( "PgDown" ); break;

            case WXK_END :
            case WXK_NUMPAD_END :       KeyStr += wxT( "End" ); break;

            case WXK_NUMPAD_BEGIN :     KeyStr += wxT( "Begin" ); break;

            case WXK_NUMPAD_INSERT :    KeyStr += wxT( "Insert" ); break;

            case WXK_DELETE:
            case WXK_NUMPAD_DELETE :    KeyStr += wxT( "Delete" ); break;

            case WXK_NUMPAD_EQUAL :     KeyStr += wxT( "=" ); break;

            case WXK_MULTIPLY:
            case WXK_NUMPAD_MULTIPLY :  KeyStr += wxT( "*" ); break;

            case WXK_ADD :
            case WXK_NUMPAD_ADD :       KeyStr += wxT( "+" ); break;

            case WXK_SEPARATOR:
            case WXK_NUMPAD_SEPARATOR : KeyStr += wxT( "Separator" ); break;

            case WXK_SUBTRACT:
            case WXK_NUMPAD_SUBTRACT :  KeyStr += wxT( "-" ); break;

            case WXK_DECIMAL :
            case WXK_NUMPAD_DECIMAL :   KeyStr += wxT( "." ); break;

            case WXK_DIVIDE :
            case WXK_NUMPAD_DIVIDE :    KeyStr += wxT( "/" ); break;



            case WXK_SELECT :           KeyStr += wxT( "Select" ); break;
            case WXK_PRINT :            KeyStr += wxT( "Print" ); break;
            case WXK_EXECUTE :          KeyStr += wxT( "Execute" ); break;
            case WXK_SNAPSHOT :         KeyStr += wxT( "Snapshot" ); break;
            case WXK_INSERT :           KeyStr += wxT( "Insert" ); break;
            case WXK_HELP :             KeyStr += wxT( "Help" ); break;
            case WXK_CANCEL :           KeyStr += wxT( "Cancel" ); break;
            case WXK_MENU :             KeyStr += wxT( "Menu" ); break;
            case WXK_CAPITAL :          KeyStr += wxT( "Capital" ); break;
            case WXK_HOME :             KeyStr += wxT( "Home" ); break;


            default :
                if( wxIsalnum( KeyCode ) || wxIsprint( KeyCode ) ) // ASCII chars...
                {
                      KeyStr += ( wxChar ) KeyCode;
                      break;
                }
        }
    }

    return KeyStr;
}

// -------------------------------------------------------------------------------- //
wxString guAccelGetCommandKeyCodeString( const int cmd )
{
    int CmdIndex = guAccelCmdIds.Index( cmd );
    if( CmdIndex != wxNOT_FOUND )
    {
        try {
            return guAccelGetKeyCodeMenuString( guAccelKeys[ CmdIndex ] );
        }
        catch(...)
        {
            guLogError( wxT( "Adding menu accelerator for cmd %i at pos %i" ), cmd, CmdIndex );
        }
    }
    return wxEmptyString;
}

// -------------------------------------------------------------------------------- //
int guAccelGetCommandKeyCode( const int cmd )
{
    int CmdIndex = guAccelCmdIds.Index( cmd );
    if( CmdIndex != wxNOT_FOUND )
    {
        return guAccelKeys[ CmdIndex ];
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
int guAccelDoAcceleratorTable( const wxArrayInt &aliascmds, const wxArrayInt &realcmds, wxAcceleratorTable &acceltable )
{
    wxAcceleratorEntry AccelEntries[ 20 ];

    int EntryCount = 0;
    int Index;
    int Count = aliascmds.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        int AccelKeyCode = guAccelGetCommandKeyCode( aliascmds[ Index ] );
        if( AccelKeyCode )
        {
            //guLogMessage( wxT( "Creating Accelerator %i) %04X  %04X %i  '%s'" ), Index, AccelKeyCode >> 16, AccelKeyCode & 0xFFFF, AccelKeyCode & 0xFFFF, guAccelGetKeyCodeString( AccelKeyCode ).c_str() );
            AccelEntries[ EntryCount++ ].Set( AccelKeyCode >> 16, AccelKeyCode & 0xFFFF, realcmds[ Index ] );
        }
    }

    if( EntryCount )
    {
        acceltable = wxAcceleratorTable( EntryCount, AccelEntries );
    }
    return EntryCount;
}

// -------------------------------------------------------------------------------- //
