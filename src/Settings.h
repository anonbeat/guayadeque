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
#ifndef guSETTINGS_H
#define guSETTINGS_H

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

#endif

// -------------------------------------------------------------------------------- //
