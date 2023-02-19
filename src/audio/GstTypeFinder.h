// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2023 J.Rios anonbeat@gmail.com
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

#ifndef __GSTTYPEFINDER_H__
#define __GSTTYPEFINDER_H__

#include <gst/gst.h>

#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/thread.h>
#include <wx/hashmap.h>


namespace Guayadeque {

WX_DECLARE_STRING_HASH_MAP( wxArrayString, guMediaFileExtensionsHashMap );

//
// hashmap to keep stuff
//
// -------------------------------------------------------------------------------- //
class guMediaFileExtensions : public guMediaFileExtensionsHashMap
{
  public :
	guMediaFileExtensions();
	void join(guMediaFileExtensions what);
};

//
// lazy & safe singleton object with some mutex-based sync
// ...otherwise gstreamer crashes the player on me  :}
//
// -------------------------------------------------------------------------------- //
class guGstTypeFinder
{
  private :
	guGstTypeFinder();
	guGstTypeFinder(guGstTypeFinder const &copy);
	guGstTypeFinder& operator=(guGstTypeFinder const &copy);

  protected :

	guMediaFileExtensions	m_Media;
	wxMutex			m_MediaMutex;

	wxArrayString		m_MediaTypePrefixes;

	bool 			READY = false;

	void AddMediaExtension( const wxString &media_type, const wxString &extension = wxEmptyString );
	bool FetchMedia( void );
	void InitMediaTypes( void );
	guMediaFileExtensions GetMediaByPrefix( const wxString &media_type_prefix = wxEmptyString );
	wxArrayString GetExtensionsByPrefix( const wxString &media_type_prefix = wxEmptyString );
	wxArrayString GetMediaTypesByPrefix( const wxString &media_type_prefix = wxEmptyString );
	void AddMediaTypePrefix( const wxString &media_type_prefix );

  public :

	static guGstTypeFinder& getGTF() { static guGstTypeFinder inst; return inst; }

	bool HasPrefixes( void );

	wxArrayString GetExtensions( void );
	wxArrayString GetMediaTypes( void );
	guMediaFileExtensions GetMedia( void );

};

}

#endif
// -------------------------------------------------------------------------------- //
