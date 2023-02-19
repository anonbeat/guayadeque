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
#ifndef __SETTINGS_H__
#define __SETTINGS_H__

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
#define guPATH_CONFIG                       wxGetHomeDir() + wxT( "/.guayadeque/" )
#define guPATH_CONFIG_FILENAME              guPATH_CONFIG wxT( "guayadeque.conf" )

#define guPATH_OLD_DBNAME                   guPATH_CONFIG wxT( "guayadeque.db" )
#define guPATH_DBNAME                       wxT( "guayadeque.db" )
#define guPATH_DBCACHE                      guPATH_CONFIG wxT( "cache.db" )

#define guPATH_COLLECTIONS                  guPATH_CONFIG wxT( "Collections/" )

#define guPATH_LYRICS                       guPATH_CONFIG wxT( "Lyrics/" )
#define guPATH_LYRICS_SOURCES_FILENAME      guPATH_CONFIG wxT( "lyrics_sources.xml" )

#define guPATH_LAYOUTS                      guPATH_CONFIG wxT( "Layouts/" )

#define guPATH_RADIOS                       guPATH_CONFIG wxT( "Radios/" )
#define guPATH_RADIOS_DBNAME                guPATH_RADIOS wxT( "radios.db" )

#define guPATH_PODCASTS                     guPATH_CONFIG wxT( "Podcasts/" )
#define guPATH_PODCASTS_DBNAME              guPATH_PODCASTS wxT( "podcasts.db" )

#define guPATH_JAMENDO                      guPATH_COLLECTIONS wxT( "Jamendo/" )
#define guPATH_JAMENDO_COVERS               guPATH_JAMENDO wxT( "Covers/" )

#define guPATH_MAGNATUNE                    guPATH_COLLECTIONS wxT( "Magnatune/" )
#define guPATH_MAGNATUNE_COVERS             guPATH_MAGNATUNE wxT( "Covers/" )

#define guPATH_LINKICONS                    guPATH_CONFIG wxT( "LinkIcons/" )

#define guPATH_PLAYLISTS                    guPATH_CONFIG wxT( "PlayLists/" )

#define guPATH_DEVICES                      guPATH_COLLECTIONS

#define guPATH_EQUALIZERS                   guPATH_CONFIG
#define guPATH_EQUALIZERS_FILENAME          guPATH_CONFIG wxT( "equalizers.conf" )

#define guPATH_DEFAULT_RECORDINGS           wxGetHomeDir() + wxT( "/Recordings" )

#define guCOPYTO_MAXCOUNT                   199
#define guCOPYTO_DEVICE_BASE                100

#define guCOMMANDS_MAXCOUNT                 99

#define guLINKS_MAXCOUNT                    99

#define guCOLLECTIONS_ID_FILENAME           wxT( ".guayadeque" )
#define guCOLLECTIONS_MAXCOUNT              40

// -------------------------------------------------------------------------------- //
// Patterns
// -------------------------------------------------------------------------------- //
#define guLINKS_LANGUAGE                    wxT( "{lang}" )
#define guLINKS_TEXT                        wxT( "{text}" )

#define guCOMMAND_ALBUMPATH                 wxT( "{bp}" )
#define guCOMMAND_COVERPATH                 wxT( "{bc}" )
#define guCOMMAND_TRACKPATH                 wxT( "{tp}" )

#define guCOPYTO_ARTIST                     wxT( "{a}" )
#define guCOPYTO_ARTIST_LOWER               wxT( "{al}" )
#define guCOPYTO_ARTIST_UPPER               wxT( "{au}" )
#define guCOPYTO_ARTIST_FIRST               wxT( "{a1}" )
#define guCOPYTO_ALBUMARTIST                wxT( "{aa}" )
#define guCOPYTO_ALBUMARTIST_LOWER          wxT( "{aal}" )
#define guCOPYTO_ALBUMARTIST_UPPER          wxT( "{aau}" )
#define guCOPYTO_ALBUMARTIST_FIRST          wxT( "{aa1}" )
#define guCOPYTO_ANYARTIST                  wxT( "{A}" )
#define guCOPYTO_ANYARTIST_LOWER            wxT( "{Al}" )
#define guCOPYTO_ANYARTIST_UPPER            wxT( "{Au}" )
#define guCOPYTO_ANYARTIST_FIRST            wxT( "{A1}" )
#define guCOPYTO_ALBUM                      wxT( "{b}" )
#define guCOPYTO_ALBUM_LOWER                wxT( "{bl}" )
#define guCOPYTO_ALBUM_UPPER                wxT( "{bu}" )
#define guCOPYTO_ALBUM_FIRST                wxT( "{b1}" )
#define guCOPYTO_ALBUM_PATH                 wxT( "{bp}" )
#define guCOPYTO_COMPOSER                   wxT( "{c}" )
#define guCOPYTO_COMPOSER_LOWER             wxT( "{cl}" )
#define guCOPYTO_COMPOSER_UPPER             wxT( "{cu}" )
#define guCOPYTO_COMPOSER_FIRST             wxT( "{c1}" )
#define guCOPYTO_FILENAME                   wxT( "{f}" )
#define guCOPYTO_GENRE                      wxT( "{g}" )
#define guCOPYTO_GENRE_LOWER                wxT( "{gl}" )
#define guCOPYTO_GENRE_UPPER                wxT( "{gu}" )
#define guCOPYTO_GENRE_FIRST                wxT( "{g1}" )
#define guCOPYTO_NUMBER                     wxT( "{n}" )
#define guCOPYTO_TITLE                      wxT( "{t}" )
#define guCOPYTO_TITLE_LOWER                wxT( "{tl}" )
#define guCOPYTO_TITLE_UPPER                wxT( "{tu}" )
#define guCOPYTO_TITLE_FIRST                wxT( "{t1}" )
#define guCOPYTO_YEAR                       wxT( "{y}" )
#define guCOPYTO_DISC                       wxT( "{d}" )
#define guCOPYTO_INDEX                      wxT( "{i}" )

#define guLYRICS_ARTIST                     wxT( "{a}" )
#define guLYRICS_ARTIST_LOWER               wxT( "{al}" )
#define guLYRICS_ARTIST_UPPER               wxT( "{au}" )
#define guLYRICS_ARTIST_FIRST               wxT( "{a1}" )
#define guLYRICS_ARTIST_LOWER_FIRST         wxT( "{al}" )
#define guLYRICS_ARTIST_UPPER_FIRST         wxT( "{au}" )
#define guLYRICS_ARTIST_CAPITALIZE          wxT( "{as}" )
#define guLYRICS_ALBUM                      wxT( "{b}" )
#define guLYRICS_ALBUM_LOWER                wxT( "{bl}" )
#define guLYRICS_ALBUM_UPPER                wxT( "{bu}" )
#define guLYRICS_ALBUM_FIRST                wxT( "{b1}" )
#define guLYRICS_ALBUM_LOWER_FIRST          wxT( "{bl}" )
#define guLYRICS_ALBUM_UPPER_FIRST          wxT( "{bu}" )
#define guLYRICS_ALBUM_CAPITALIZE           wxT( "{bs}" )
#define guLYRICS_TITLE                      wxT( "{t}" )
#define guLYRICS_TITLE_LOWER                wxT( "{tl}" )
#define guLYRICS_TITLE_UPPER                wxT( "{tu}" )
#define guLYRICS_TITLE_FIRST                wxT( "{t1}" )
#define guLYRICS_TITLE_LOWER_FIRST          wxT( "{tl}" )
#define guLYRICS_TITLE_UPPER_FIRST          wxT( "{tu}" )
#define guLYRICS_TITLE_CAPITALIZE           wxT( "{ts}" )
#define guLYRICS_ALBUM_PATH                 wxT( "{bp}" )
#define guLYRICS_FILENAME                   wxT( "{f}" )

// -------------------------------------------------------------------------------- //
// Splash
// -------------------------------------------------------------------------------- //
#define guSPLASH_NAME                       wxT( "J.Rios" )
#define guSPLASH_EMAIL                      wxT( "anonbeat@gmail.com" )
#define guSPLASH_HOMEPAGE                   wxT( "http://guayadeque.org" )
#define guSPLASH_DONATION_LINK              wxT( "https://goo.gl/GHDEEO" )

// -------------------------------------------------------------------------------- //
// Colors
// -------------------------------------------------------------------------------- //
#define guCOLOR_BASE                        wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW )
#define guCOLOR_CAPTION_INACTIVE            wxSystemSettings::GetColour( wxSYS_COLOUR_INACTIVECAPTION )
#define guCOLOR_CAPTION_ACTIVE              wxSystemSettings::GetColour( wxSYS_COLOUR_ACTIVECAPTION )
#define guCOLOR_CAPTION_GRADIENT_INACTIVE   wxSystemSettings::GetColour( wxSYS_COLOUR_GRADIENTINACTIVECAPTION )
#define guCOLOR_CAPTION_GRADIENT_ACTIVE     wxSystemSettings::GetColour( wxSYS_COLOUR_GRADIENTACTIVECAPTION )
#define guCOLOR_CAPTION_TEXT_INACTIVE       wxSystemSettings::GetColour( wxSYS_COLOUR_INACTIVECAPTIONTEXT )
#define guCOLOR_CAPTION_TEXT_ACTIVE         wxSystemSettings::GetColour( wxSYS_COLOUR_CAPTIONTEXT )
#define guCOLOR_SASH                        wxAuiStepColour( guCOLOR_BASE, 85 )

#define guSIZE_CAPTION                      22
#define guSIZE_BORDER                       0
#define guSIZE_SASH                         5
#define guGRADIENT_TYPE                     wxAUI_GRADIENT_VERTICAL

}

#endif
// -------------------------------------------------------------------------------- //
