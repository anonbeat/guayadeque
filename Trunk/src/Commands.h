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
#ifndef COMMANDS_H
#define COMMANDS_H

#define ID_GUAYADEQUE_VERSION           "v0.0.3 (Beta)"

// -------------------------------------------------------------------------------- //
//
#define ID_MENU_UPDATE_LIBRARY          1000
#define ID_MENU_UPDATE_COVERS           1001
#define ID_MENU_QUIT                    1002
#define ID_MENU_PREFERENCES             1003
#define ID_MENU_VIEW_LIBRARY            1004
#define ID_MENU_VIEW_LASTFM             1005
#define ID_MENU_VIEW_RADIO              1006
#define ID_MENU_VIEW_LYRICS             1007
#define ID_MENU_ABOUT                   1008

#define ID_MAINFRAME_COPYTO             1018
#define ID_LIBRARY_UPDATED              1019

//
#define ID_GENRE_PLAY                   1020
#define ID_GENRE_ENQUEUE                1021
#define ID_GENRE_COPYTO                 1022

//
#define ID_LABEL_ADD                    1030
#define ID_LABEL_DELETE                 1031
#define ID_LABEL_EDIT                   1032
#define ID_LABEL_PLAY                   1033
#define ID_LABEL_ENQUEUE                1034
#define ID_LABEL_CLEARSELECTION         1035
#define ID_LABEL_COPYTO                 1036

//
#define ID_ARTIST_PLAY                  1040
#define ID_ARTIST_ENQUEUE               1041
#define ID_ARTIST_EDITLABELS            1042
#define ID_ARTIST_EDITTRACKS            1043
#define ID_ARTIST_COPYTO                1044

//
#define ID_ALBUM_PLAY                   1050
#define ID_ALBUM_ENQUEUE                1051
#define ID_ALBUM_EDITLABELS             1052
#define ID_ALBUM_EDITTRACKS             1053
#define ID_ALBUM_MANUALCOVER            1054
#define ID_ALBUM_COVER_DOWNLOADED       1055
#define ID_ALBUM_COVER_DELETE           1056
#define ID_ALBUM_COPYTO                 1057

//
#define ID_SONG_PLAY                    1060
#define ID_SONG_ENQUEUE                 1061
#define ID_SONG_EDITLABELS              1062
#define ID_SONG_EDITTRACKS              1063
#define ID_SONG_COPYTO                  1064

//
#define ID_PLAYLIST_UPDATELIST          1070
#define ID_PLAYLIST_CLEAR               1071
#define ID_PLAYLIST_REMOVE              1072
#define ID_PLAYLIST_SAVE                1073
#define ID_PLAYLIST_SMARTPLAY           1074
#define ID_PLAYLIST_RANDOMPLAY          1075
#define ID_PLAYLIST_REPEATPLAY          1076
#define ID_PLAYLIST_SMART_ADDTRACK      1077
#define ID_PLAYLIST_COPYTO              1078

//
#define ID_RADIO_DOUPDATE               1080
#define ID_RADIO_UPDATED                1081
#define ID_RADIO_UPDATE_END             1082
#define ID_RADIO_GENRE_ADD              1083
#define ID_RADIO_GENRE_EDIT             1084
#define ID_RADIO_GENRE_DELETE           1085
#define ID_RADIO_EDIT_LABELS            1086

//
#define ID_PLAYERPANEL_PLAY             1090
#define ID_PLAYERPANEL_STOP             1091
#define ID_PLAYERPANEL_NEXTTRACK        1092
#define ID_PLAYERPANEL_PREVTRACK        1093
#define ID_PLAYERPANEL_UPDATERADIOTRACK 1094
#define ID_PLAYERPANEL_TRACKCHANGED     1095
#define ID_PLAYERPANEL_CAPSCHANGED      1096
#define ID_PLAYERPANEL_STATUSCHANGED    1097

//
#define ID_AUDIOSCROBBLE_UPDATED        1100

// Commands for the CoverEditor
#define ID_COVEREDITOR_ADDCOVERIMAGE    1110

// Commands for LastFM Panel
//#define ID_LASTFM_UPDATE_TRACK          1120    // The Player notifies the panel to update the content
#define ID_LASTFM_UPDATE_ARTISTINFO     1121    // The thread update the Artist Info
#define ID_LASTFM_UPDATE_ALBUMINFO      1122    // The thread update the top albums
#define ID_LASTFM_UPDATE_SIMARTIST      1123    // The thread update the Similar artists
#define ID_LASTFM_UPDATE_SIMTRACK       1124    // The thread update the Similar tracks

#define ID_LASTFM_VISIT_URL             1130
#define ID_LASTFM_SEARCH_LINK           2000    // From 2000 for Link #0 to ...

#define ID_GAUGE_PULSE                  1140
#define ID_GAUGE_SETMAX                 1141
#define ID_GAUGE_UPDATE                 1142
#define ID_GAUGE_REMOVE                 1143

#define ID_LYRICS_UPDATE_LYRICINFO      1150

#define ID_MULTIMEDIAKEYS_DBUS          1160

#endif
// -------------------------------------------------------------------------------- //
