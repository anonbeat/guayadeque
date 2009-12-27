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

#define ID_GUAYADEQUE_VERSION           "0.2.3"

// -------------------------------------------------------------------------------- //
//
enum guCommandIds {
    ID_MENU_UPDATE_LIBRARY        =  1000,
    ID_MENU_UPDATE_PODCASTS,
    ID_MENU_UPDATE_COVERS,
    ID_MENU_QUIT,
    ID_MENU_PREFERENCES,
    ID_MENU_VIEW_LIBRARY,
    ID_MENU_VIEW_LASTFM,
    ID_MENU_VIEW_RADIO,
    ID_MENU_VIEW_LYRICS,
    ID_MENU_VIEW_PLAYLISTS,
    ID_MENU_VIEW_PODCASTS,
    ID_MENU_ABOUT,

    //
    ID_MAINFRAME_COPYTO,
    ID_LIBRARY_UPDATED,
    ID_MAINFRAME_REMOVEPODCASTTHREAD,
    ID_MAINFRAME_SET_LIBTRACKS,
    ID_MAINFRAME_SET_RADIOSTATIONS,
    ID_MAINFRAME_SET_PLAYLISTTRACKS,
    ID_MAINFRAME_SET_PODCASTS,
    //
    ID_GENRE_PLAY,
    ID_GENRE_ENQUEUE,
    ID_GENRE_COPYTO,
    ID_GENRE_SELECTNAME,
    ID_GENRE_SETSELECTION,
    //
    ID_LABEL_ADD,
    ID_LABEL_DELETE,
    ID_LABEL_EDIT,
    ID_LABEL_PLAY,
    ID_LABEL_ENQUEUE,
    //ID_LABEL_CLEARSELECTION,
    ID_LABEL_COPYTO,
    ID_LABEL_UPDATELABELS,
    //
    ID_ARTIST_PLAY,
    ID_ARTIST_ENQUEUE,
    ID_ARTIST_EDITLABELS,
    ID_ARTIST_EDITTRACKS,
    ID_ARTIST_COPYTO,
    ID_ARTIST_SELECTNAME,
    ID_ARTIST_SETSELECTION,
    //
    ID_ALBUM_PLAY,
    ID_ALBUM_ENQUEUE,
    ID_ALBUM_EDITLABELS,
    ID_ALBUM_EDITTRACKS,
    ID_ALBUM_MANUALCOVER,
    ID_ALBUM_COVER_DOWNLOADED,
    ID_ALBUM_COVER_DELETE,
    ID_ALBUM_COPYTO,
    ID_ALBUM_SELECTNAME,
    ID_ALBUM_SETSELECTION,
    ID_ALBUM_SELECT_COVER,
    //
    ID_SONG_PLAY,
    ID_SONG_PLAYALL,
    ID_SONG_ENQUEUE,
    ID_SONG_ENQUEUEALL,
    ID_SONG_EDITLABELS,
    ID_SONG_EDITTRACKS,
    ID_SONG_COPYTO,
    ID_SONG_SAVEPLAYLIST,
    ID_SONG_DELETE,
    ID_SONG_BROWSE_GENRE,
    ID_SONG_BROWSE_ARTIST,
    ID_SONG_BROWSE_ALBUM,
    //
    ID_PLAYER_PLAYLIST_UPDATELIST,
    ID_PLAYER_PLAYLIST_CLEAR,
    ID_PLAYER_PLAYLIST_REMOVE,
    ID_PLAYER_PLAYLIST_SAVE,
    ID_PLAYER_PLAYLIST_SMARTPLAY,
    ID_PLAYER_PLAYLIST_RANDOMPLAY,
    ID_PLAYER_PLAYLIST_REPEATPLAY,
    ID_PLAYER_PLAYLIST_SMART_ADDTRACK,
    ID_PLAYER_PLAYLIST_COPYTO,
    ID_PLAYER_PLAYLIST_EDITLABELS,
    //
    ID_RADIO_PLAY,
    ID_RADIO_ENQUEUE,
    ID_RADIO_DOUPDATE,
    ID_RADIO_UPDATED,
    ID_RADIO_UPDATE_END,
    ID_RADIO_GENRE_ADD,
    ID_RADIO_GENRE_EDIT,
    ID_RADIO_GENRE_DELETE,
    ID_RADIO_EDIT_LABELS,
    ID_RADIO_USER_ADD,
    ID_RADIO_USER_EDIT,
    ID_RADIO_USER_DEL,
    //
    ID_PLAYERPANEL_PLAY,
    ID_PLAYERPANEL_STOP,
    ID_PLAYERPANEL_NEXTTRACK,
    ID_PLAYERPANEL_PREVTRACK,
    ID_PLAYERPANEL_UPDATERADIOTRACK,
    ID_PLAYERPANEL_TRACKCHANGED,
    ID_PLAYERPANEL_CAPSCHANGED,
    ID_PLAYERPANEL_STATUSCHANGED,
    ID_PLAYERPANEL_TRACKLISTCHANGED,
    //
    ID_AUDIOSCROBBLE_UPDATED,
    // Commands for the CoverEditor
    ID_COVEREDITOR_ADDCOVERIMAGE,
    ID_COVEREDITOR_DOWNLOADEDLINKS,
    // Commands for LastFM Panel
    //ID_LASTFM_UPDATE_TRACK,               // The Player notifies the panel to update the content
    ID_LASTFM_UPDATE_ARTISTINFO,            // The thread update the Artist Info
    ID_LASTFM_UPDATE_ALBUMINFO,             // The thread update the top albums
    ID_LASTFM_UPDATE_SIMARTIST,             // The thread update the Similar artists
    ID_LASTFM_UPDATE_SIMTRACK,              // The thread update the Similar tracks
    ID_LASTFM_PLAY,
    ID_LASTFM_ENQUEUE,
    ID_LASTFM_SELECT_ARTIST,
    ID_LASTFM_VISIT_URL,
    ID_LASTFM_COPYTOCLIPBOARD,
    //
    ID_GAUGE_PULSE,
    ID_GAUGE_SETMAX,
    ID_GAUGE_UPDATE,
    ID_GAUGE_REMOVE,
    //
    ID_LYRICS_UPDATE_LYRICINFO,
    //
    ID_MULTIMEDIAKEYS_DBUS,
    //
    ID_PLAYLIST_PLAY,
    ID_PLAYLIST_ENQUEUE,
    ID_PLAYLIST_NEWPLAYLIST,
    ID_PLAYLIST_EDIT,
    ID_PLAYLIST_RENAME,
    ID_PLAYLIST_DELETE,
    ID_PLAYLIST_COPYTO,
    ID_PLAYLIST_UPDATED,
    //
    ID_PODCASTS_CHANNEL_ADD,
    ID_PODCASTS_CHANNEL_DEL,
    ID_PODCASTS_CHANNEL_PROPERTIES,
    ID_PODCASTS_CHANNEL_COPYTO,
    ID_PODCASTS_CHANNEL_UPDATE,
    ID_PODCASTS_CHANNEL_UNDELETE,
    //
    ID_PODCASTS_ITEM_DEL,
    ID_PODCASTS_ITEM_COPYTO,
    ID_PODCASTS_ITEM_PLAY,
    ID_PODCASTS_ITEM_ENQUEUE,
    ID_PODCASTS_ITEM_DOWNLOAD,
    //
    ID_ARTIST_COMMANDS = 3000,          // 3000...3099
    ID_ALBUM_COMMANDS = 3100,           // 3100...3199
    ID_SONGS_COMMANDS = 3200,           // 3200...3299
    ID_PLAYER_PLAYLIST_COMMANDS = 3300, // 3300...3399
    //
    ID_LASTFM_SEARCH_LINK = 4000    // From 4000 for Link #0 to 4999
};

#endif
// -------------------------------------------------------------------------------- //
