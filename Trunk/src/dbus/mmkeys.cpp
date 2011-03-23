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
#include "mmkeys.h"

#include "Commands.h"
#include "Utils.h"

// -------------------------------------------------------------------------------- //
guMMKeys::guMMKeys( guDBusServer * server, guPlayerPanel * playerpanel ) : guDBusClient( server )
{
    m_PlayerPanel = playerpanel;

    RegisterClient();

    // Support for the MultimediaKeys
    AddMatch( "type='signal',interface='org.gnome.SettingsDaemon'" );
    AddMatch( "type='signal',interface='org.gnome.SettingsDaemon.MediaKeys'" );
}

// -------------------------------------------------------------------------------- //
guMMKeys::~guMMKeys()
{
}

// -------------------------------------------------------------------------------- //
DBusHandlerResult guMMKeys::HandleMessages( guDBusMessage * msg, guDBusMessage * reply )
{
    wxASSERT( msg );

    DBusHandlerResult RetVal = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    //const char *    Interface = msg->GetInterface();
    const char *    Member = msg->GetMember();
    int             Type = msg->GetType();
    const char *    Path = msg->GetPath();

    // MULTIMEDIA KEYS
    if( Type == DBUS_MESSAGE_TYPE_SIGNAL )  // If its a Signal message
    {
        if( !strcmp( Path, "/org/gnome/SettingsDaemon" ) ||
            !strcmp( Path, "/org/gnome/SettingsDaemon/MediaKeys" ) )
        {
            if( !strcmp( Member, "MediaPlayerKeyPressed" ) )
            {
                DBusError error;
                dbus_error_init( &error );

                const char * s = NULL;
                const char * KeyName = NULL;

                dbus_message_get_args( msg->GetMessage(), &error,
                      DBUS_TYPE_STRING, &s,
                      DBUS_TYPE_STRING, &KeyName,
                      DBUS_TYPE_INVALID );

                if( dbus_error_is_set( &error ) )
                {
                    printf( "Could not read the MediaPlayerKeyPressed parameters : %s\n", error.message );
                    dbus_error_free( &error );
                    //RetVal =  DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
                }
                else
                {
                    if( !strcmp( KeyName, "Play" ) )
                    {
                        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_PLAY );
                        wxPostEvent( m_PlayerPanel, event );
                        RetVal = DBUS_HANDLER_RESULT_HANDLED;
                    }
                    else if( !strcmp( KeyName, "Stop" ) )
                    {
                        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_STOP );
                        wxPostEvent( m_PlayerPanel, event );
                        RetVal = DBUS_HANDLER_RESULT_HANDLED;
                    }
                    else if( !strcmp( KeyName, "Previous" ) )
                    {
                        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_PREVTRACK );
                        wxPostEvent( m_PlayerPanel, event );
                        RetVal = DBUS_HANDLER_RESULT_HANDLED;
                    }
                    else if( !strcmp( KeyName, "Next" ) )
                    {
                        wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_NEXTTRACK );
                        wxPostEvent( m_PlayerPanel, event );
                        RetVal = DBUS_HANDLER_RESULT_HANDLED;
                    }
                    else if( !strcmp( KeyName, "Pause" ) )
                    {
                        if( m_PlayerPanel->GetState() == guMEDIASTATE_PLAYING )
                        {
                            wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_PLAYERPANEL_PLAY );
                            wxPostEvent( m_PlayerPanel, event );
                        }
                        RetVal = DBUS_HANDLER_RESULT_HANDLED;
                    }
                }
            }
        }
    }

    return RetVal;
}

// -------------------------------------------------------------------------------- //
