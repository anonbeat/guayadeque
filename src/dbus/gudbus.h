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
#ifndef GUDBUS_H
#define GUDBUS_H

#define DBUS_THREAD_IDLE_TIMEOUT    50

#include <dbus/dbus.h>
#include <wx/thread.h>

class guDBusThread;

// -------------------------------------------------------------------------------- //
class guDBusMessage
{
  protected :
    DBusMessage * m_DBusMsg;

  public :
    guDBusMessage( int type = DBUS_MESSAGE_TYPE_METHOD_CALL );
    guDBusMessage( DBusMessage * msg );
    guDBusMessage( guDBusMessage * msg );
    ~guDBusMessage();

     DBusMessage *   GetMessage();
     int             GetType();
     const char *    GetErrorName();
     const char *    GetInterface();
     const char *    GetMember();
     bool            NeedReply();
     const char *    GetPath();
     unsigned int    GetReplySerial();
     const char *    GetSender();
     const char *    GetDestination();
     unsigned int    GetSerial();
     bool            HasDestination( const char * dest );
     bool            HasInterface( const char * iface );
     bool            HasMember( const char * member );
     bool            HasPath( const char * member );
     bool            HasSender( const char * member );
     bool            IsError( const char * errname );
     bool            IsMethodCall( const char * iface, const char * method );
     bool            IsSignal( const char * iface, const char * signal_name );

};

// -------------------------------------------------------------------------------- //
class guDBusMethodReturn : public guDBusMessage
{
  public :
    guDBusMethodReturn( guDBusMessage * msg );
    guDBusMethodReturn( DBusMessage * msg );
};

// -------------------------------------------------------------------------------- //
class guDBusSignal : public guDBusMessage
{
  public:
    guDBusSignal( const char * path, const char * iface, const char * name );
};

// -------------------------------------------------------------------------------- //
class guDBus
{
  protected :
    DBusConnection *    m_DBusConn;
    DBusError           m_DBusErr;
    guDBusThread *      m_DBusThread;

  public :
    guDBus( const char * name, bool System = false );
    ~guDBus();

    bool                        RequestName( const char * name );
    virtual DBusHandlerResult   HandleMessages( guDBusMessage * msg, guDBusMessage * reply = NULL );
    DBusConnection *            GetConnection();
    bool                        RegisterObjectPath( const char * objname );
    bool                        UnRegisterObjectPath( const char * objname );
    bool                        AddMatch( const char * rule );
    bool                        Send( guDBusMessage * msg );
    void                        Flush();

};

// -------------------------------------------------------------------------------- //
class guDBusThread : public wxThread
{
  protected :
    guDBus * m_DBusOwner;

  public :
    guDBusThread( guDBus * dbusowner );
    ~guDBusThread();

    virtual ExitCode Entry();

    friend class guDBus;
};

#endif
// -------------------------------------------------------------------------------- //
