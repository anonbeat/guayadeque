// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2010 J.Rios
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
#ifndef GUCONFIG_H
#define GUCONFIG_H

#include <wx/wx.h>
#include <wx/fileconf.h>

WX_DEFINE_ARRAY_PTR( wxEvtHandler *, guEvtHandlerArray );
extern const wxEventType guConfigUpdatedEvent;


// -------------------------------------------------------------------------------- //
class guConfig : public wxConfig
{
  protected :
    wxMutex             m_ConfigMutex;
    guEvtHandlerArray   m_Objects;
    bool                m_IgnoreLayouts;

  public :
    guConfig( const wxString &conffile = wxT( ".guayadeque/guayadeque.conf" ) );
    ~guConfig();

    long            ReadNum( const wxString &KeyName, long Default = 0, const wxString &Category = wxEmptyString );
    bool            WriteNum( const wxString &KeyName, long Value = 0, const wxString &Category = wxEmptyString );
    bool            ReadBool( const wxString &KeyName, bool Default = true, const wxString &Category = wxEmptyString );
    bool            WriteBool( const wxString &KeyName, bool Value, const wxString &Category = wxEmptyString );
    wxString        ReadStr( const wxString &KeyName, const wxString &Default, const wxString &Category = wxEmptyString );
    bool            WriteStr( const wxString &KeyName, const wxString &Value, const wxString &Category = wxEmptyString );
    wxArrayString   ReadAStr( const wxString &Key, const wxString &Default, const wxString &Category = wxEmptyString );
    bool            WriteAStr( const wxString &Key, const wxArrayString &Value, const wxString &Category = wxEmptyString, bool ResetGroup = true );
    wxArrayInt      ReadANum( const wxString &Key, const int Default, const wxString &Category = wxEmptyString );
    bool            WriteANum( const wxString &Key, const wxArrayInt &Value, const wxString &Category = wxEmptyString, bool ResetGroup = true );

    void            RegisterObject( wxEvtHandler * object );
    void            UnRegisterObject( wxEvtHandler * object );
    void            SendConfigChangedEvent( const int flags = 0 );

    bool            GetIgnoreLayouts( void ) { return m_IgnoreLayouts; }
    void            SetIgnoreLayouts( const bool ignorelayouts ) { m_IgnoreLayouts = ignorelayouts; };

};

#endif
// -------------------------------------------------------------------------------- //
