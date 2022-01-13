// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2022 J.Rios anonbeat@gmail.com
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
#ifndef __RADIOTAGINFO_H__
#define __RADIOTAGINFO_H__

#include <gst/gst.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
class guRadioTagInfo
{
  public :
    gchar * m_Organization;
    gchar * m_Location;
    gchar * m_Title;
    gchar * m_Genre;

    guRadioTagInfo() { m_Organization = NULL; m_Location = NULL; m_Title = NULL; m_Genre = NULL; }
    ~guRadioTagInfo()
    {
        if( m_Organization )
            g_free( m_Organization );
        if( m_Location )
            g_free( m_Location );
        if( m_Title )
            g_free( m_Title );
        if( m_Genre )
            g_free( m_Genre );
    }
};

}

#endif // RADIOTAGINFO_H

// -------------------------------------------------------------------------------- //
