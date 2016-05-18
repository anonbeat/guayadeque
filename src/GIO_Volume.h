// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2016 J.Rios
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
#ifndef GIO_VOLUME_H
#define GIO_VOLUME_H

#include <gio/gio.h>

#include "Utils.h"

#include <wx/arrstr.h>
#include <wx/string.h>
#include <wx/dynarray.h>

class guGIO_Mount;
class guMainFrame;

// -------------------------------------------------------------------------------- //
class guGIO_Volume
{
  private :
    GVolume * m_Volume;

  protected :

  public :
    guGIO_Volume( GVolume * volume ) { m_Volume = volume; }
    ~guGIO_Volume( void ) { if( m_Volume ) g_object_unref( m_Volume ); }

    wxString        GetName( void );
    wxString        GetUUID( void );
    wxString        GetIcon( void );
    //GDrive          GetDrive( void );
    guGIO_Mount     GetMount( void );
    bool            CanMount( void );
    bool            ShouldAutoMount( void );
    //void            Mount( int flags,
    bool            CanEject( void );

};

// -------------------------------------------------------------------------------- //
class guGIO_Mount
{
  private :
    GMount * m_Mount;

  protected :
//    int             m_PanelActive;
    wxString        m_Id;
    bool            m_IsReadOnly;
    wxString        m_Name;
    wxString        m_MountPath;
    wxString        m_IconString;

  public :
    guGIO_Mount( GMount * mount );
    guGIO_Mount( GMount * mount, wxString &mountpath );
    ~guGIO_Mount();

    bool            IsMount( GMount * mount ) { return m_Mount == mount; }

    bool            IsReadOnly( void ) { return m_IsReadOnly; }

//    int             PanelActive( void ) { guLogMessage( wxT( "PanelActive: %i" ), m_PanelActive ); return m_PanelActive; }
//    void            SetPanelActive( int active ) { m_PanelActive = active; }
    void            SetId( const wxString &id ) { m_Id = id; }
    wxString        GetId( void ) { return m_Id; }

    wxString        GetName( void ) { return m_Name; }
    wxString        GetMountPath( void ) { return m_MountPath; }
    wxString        IconString( void ) { return m_IconString; }

    GVolume         GetVolume( void );

    bool            CanUnmount( void );
    void            Unmount( void );
};
WX_DEFINE_ARRAY_PTR( guGIO_Mount *, guGIO_MountArray );

// -------------------------------------------------------------------------------- //
class guGIO_VolumeMonitor
{
  private :
    int                 m_VolumeAddedId;
    int                 m_VolumeRemovedId;
    int                 m_MountAddedId;
    int                 m_MountPreUnmountId;
    int                 m_MountRemovedId;

  protected :
    guMainFrame *       m_MainFrame;
    GVolumeMonitor *    m_VolumeMonitor;
    guGIO_MountArray *  m_MountedVolumes;

    int                 FindMount( GMount * mount );

    void                GetCurrentMounts( void );

  public :
    guGIO_VolumeMonitor( guMainFrame * mainframe );
    ~guGIO_VolumeMonitor();

    void                OnMountAdded( GMount * mount );
    void                OnMountRemoved( GMount * mount );
    void                OnVolumeAdded( GVolume * volume );
    void                OnVolumeRemoved( GVolume * volume );

    wxArrayString       GetMountNames( void );
    int                 GetMountCount( void ) { return m_MountedVolumes->Count(); }
    guGIO_Mount *       GetMount( const int index ) { return m_MountedVolumes->Item( index ); }
    guGIO_Mount *       GetMountById( const wxString &id );
    guGIO_Mount *       GetMountByPath( const wxString &path );
    guGIO_Mount *       GetMountByName( const wxString &name );

//    int                 PanelActive( const int index ) { return m_MountedVolumes->Item( index )->PanelActive(); }
};

#endif
// -------------------------------------------------------------------------------- //
