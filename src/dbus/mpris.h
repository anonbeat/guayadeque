// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2020 J.Rios anonbeat@gmail.com
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
//    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
//    Boston, MA 02110-1301 USA.
//
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#ifndef __MPRIS_H__
#define __MPRIS_H__

#include "gudbus.h"
#include "PlayerPanel.h"

namespace Guayadeque {

#define GUAYADEQUE_MPRIS_SERVICENAME        "org.mpris.guayadeque"
#define GUAYADEQUE_MPRIS_INTERFACE          "org.freedesktop.MediaPlayer"
#define GUAYADEQUE_MPRIS_ROOT_PATH          "/"
#define GUAYADEQUE_MPRIS_PLAYER_PATH        "/Player"
#define GUAYADEQUE_MPRIS_TRACKLIST_PATH     "/TrackList"

#define GUAYADEQUE_MPRIS_VERSION_MAJOR      1
#define GUAYADEQUE_MPRIS_VERSION_MINOR      0

#define MPRIS_CAPS_NONE                  0
#define MPRIS_CAPS_CAN_GO_NEXT           ( 1 << 0 )
#define MPRIS_CAPS_CAN_GO_PREV           ( 1 << 1 )
#define MPRIS_CAPS_CAN_PAUSE             ( 1 << 2 )
#define MPRIS_CAPS_CAN_PLAY              ( 1 << 3 )
#define MPRIS_CAPS_CAN_SEEK              ( 1 << 4 )
#define MPRIS_CAPS_CAN_PROVIDE_METADATA  ( 1 << 5 )
#define MPRIS_CAPS_CAN_HAS_TRACKLIST     ( 1 << 6 )

// -------------------------------------------------------------------------------- //
class guMPRIS : public guDBusClient
{
  protected :
    guPlayerPanel * m_PlayerPanel;

  public :
    guMPRIS( guDBusServer * server, guPlayerPanel * playerpanel );
    virtual ~guMPRIS();

    virtual DBusHandlerResult   HandleMessages( guDBusMessage * msg, guDBusMessage * reply = NULL );

    virtual void                OnPlayerTrackChange();
    virtual void                OnPlayerStatusChange();
    virtual void                OnPlayerCapsChange();
    virtual void                OnTrackListChange();

};

}

#endif
// -------------------------------------------------------------------------------- //
