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
#ifndef __CONFIGCONSTS_H__
#define __CONFIGCONSTS_H__

#include <wx/string.h>

namespace Guayadeque {

// Accelerators
#define CONFIG_PATH_ACCELERATORS                         "accelerators"
#define CONFIG_KEY_ACCELERATORS_ACCELKEY                 "AccelKey"


// AudioCd
#define CONFIG_PATH_AUDIOCD                              "audiocd"
#define CONFIG_KEY_AUDIOCD_ORDER                         "Order"
#define CONFIG_KEY_AUDIOCD_ORDERDESC                     "OrderDesc"


// Commands
#define CONFIG_PATH_COMMANDS_EXECS                       "commands/execs"
#define CONFIG_PATH_COMMANDS_NAMES                       "commands/names"
#define CONFIG_KEY_COMMANDS_EXEC                         "Exec"
#define CONFIG_KEY_COMMANDS_NAME                         "Name"


// CopyTo
#define CONFIG_PATH_COPYTO                               "copyto/options"
#define CONFIG_KEY_COPYTO_OPTION                         "Option"


// Crossfader
#define CONFIG_PATH_CROSSFADER                           "crossfader"
#define CONFIG_KEY_CROSSFADER_FORCE_GAPLESS              "ForceGapless"
#define CONFIG_KEY_CROSSFADER_FADEOUT_TIME               "FadeOutTime"
#define CONFIG_KEY_CROSSFADER_FADEIN_TIME                "FadeInTime"
#define CONFIG_KEY_CROSSFADER_FADEIN_VOL_START           "FadeInVolStar"
#define CONFIG_KEY_CROSSFADER_FADEIN_VOL_TRIGER          "FadeInVolTriger"



// Equalizer
#define CONFIG_PATH_EQUALIZER                            "equalizer"
#define CONFIG_KEY_EQUALIZER_POS_X                       "PosX"
#define CONFIG_KEY_EQUALIZER_POS_Y                       "PosY"
#define CONFIG_KEY_EQUALIZER_WIDTH                       "Width"
#define CONFIG_KEY_EQUALIZER_HEIGHT                      "Height"
#define CONFIG_KEY_EQUALIZER_LAST_PRESET                 "LastEqPreset"
#define CONFIG_KEY_EQUALIZER_BAND                        "Band"


// File Browser
#define CONFIG_PATH_FILE_BROWSER                         "filebrowser"
#define CONFIG_KEY_FILE_BROWSER_SHOW_LIB_PATHS           "ShowLibPaths"
#define CONFIG_KEY_FILE_BROWSER_ORDER                    "Order"
#define CONFIG_KEY_FILE_BROWSER_ORDERDESC                "OrderDesc"
#define CONFIG_KEY_FILE_BROWSER_VISIBLE_PANELS           "VisiblePanels"
#define CONFIG_KEY_FILE_BROWSER_LAST_LAYOUT              "LastLayout"
#define CONFIG_KEY_FILE_BROWSER_PATH                     "Path"
#define CONFIG_PATH_FILE_BROWSER_COLUMNS_IDS             "filebrowser/columns/ids"
#define CONFIG_PATH_FILE_BROWSER_COLUMNS_WIDTHS          "filebrowser/columns/widths"
#define CONFIG_PATH_FILE_BROWSER_COLUMNS_SHOWS           "filebrowser/columns/shows"


// File Renamer
#define CONFIG_PATH_FILE_RENAMER                         "filebrowser/filerenamer"
#define CONFIG_KEY_FILE_RENAMER_POS_X                    "PosX"
#define CONFIG_KEY_FILE_RENAMER_POS_Y                    "PosY"
#define CONFIG_KEY_FILE_RENAMER_SIZE_WIDTH               "SizeWidth"
#define CONFIG_KEY_FILE_RENAMER_SIZE_HEIGHT              "SizeHeight"
#define CONFIG_KEY_FILE_RENAMER_PATTERN                  "Pattern"


// General
#define CONFIG_PATH_GENERAL                              "general"
#define CONFIG_KEY_GENERAL_ACTION_ENQUEUE                "DefaultActionEnqueue"
#define CONFIG_KEY_GENERAL_BROWSER_COMMAND               "BrowserCommand"
#define CONFIG_KEY_GENERAL_BUFFER_SIZE                   "BufferSize"
#define CONFIG_KEY_GENERAL_CLOSE_TO_TASKBAR              "CloseToTaskBar"
#define CONFIG_KEY_GENERAL_COVER_FRAME                   "CoverFrame"
#define CONFIG_KEY_GENERAL_COVER_SEARCH_ENGINE           "CoverSearchEngine"
#define CONFIG_KEY_GENERAL_CURRENT_TRACK_POS             "CurrentTrackPos"
#define CONFIG_KEY_GENERAL_DROP_FILES_CLEAR_PLAYLIST     "DropFilesClearPlaylist"
#define CONFIG_KEY_GENERAL_EMBED_TO_FILES                "EmbedToFiles"
#define CONFIG_KEY_GENERAL_INSTANT_TEXT_SEARCH           "InstantTextSearchEnabled"
#define CONFIG_KEY_GENERAL_LANGUAGE                      "Language"
#define CONFIG_KEY_GENERAL_LAST_UPDATE                   "LastUpdate"
#define CONFIG_KEY_GENERAL_LOAD_DEFAULT_LAYOUTS          "LoadDefaultLayouts"
#define CONFIG_KEY_GENERAL_MIN_SAVE_PLAYL_POST_LENGTH    "MinSavePlayPosLength"
#define CONFIG_KEY_GENERAL_NOTIFICATION_TIME             "NotificationsTime"
//#define CONFIG_KEY_GENERAL_PLAYER_LOOP                   "PlayerLoop"
//#define CONFIG_KEY_GENERAL_PLAYER_SMART                  "PlayerSmart"
#define CONFIG_KEY_GENERAL_PLAYER_PLAYMODE               "PlayerPlayMode"
#define CONFIG_KEY_GENERAL_PLAYER_VOLUME                 "PlayerCurVol"
#define CONFIG_KEY_GENERAL_PLAYER_VOLUME_VISIBLE         "PlayerVolumeVisible"
#define CONFIG_KEY_GENERAL_RANDOM_MODE_ON_EMPTY_PLAYLIST "RndModeOnEmptyPlayList"
#define CONFIG_KEY_GENERAL_RANDOM_PLAY_ON_EMPTY_PLAYLIST "RndPlayOnEmptyPlayList"
#define CONFIG_KEY_GENERAL_REPLAY_GAIN_MODE              "ReplayGainMode"
#define CONFIG_KEY_GENERAL_REPLAY_GAIN_PREAMP            "ReplayGainPreAmp"
#define CONFIG_KEY_GENERAL_SAVE_CURRENT_TRACK_POSITION   "SaveCurrentTrackPos"
#define CONFIG_KEY_GENERAL_SHOW_CLOSE_BUTTON             "ShowPaneCloseButton"
#define CONFIG_KEY_GENERAL_SHOW_CLOSE_CONFIRM            "ShowCloseConfirm"
#define CONFIG_KEY_GENERAL_SHOW_NOTIFICATIONS            "ShowNotifications"
#define CONFIG_KEY_GENERAL_EQ_ENABLED                    "EqualizerEnabled"
#define CONFIG_KEY_GENERAL_VOLUME_ENABLED                "VolumeColtrolsEnabled"
#define CONFIG_KEY_GENERAL_SHOW_REV_TIME                 "ShowRevTime"
#define CONFIG_KEY_GENERAL_SHOW_SPLASH_SCREEN            "ShowSplashScreen"
#define CONFIG_KEY_GENERAL_SHOW_TASK_BAR_ICON            "ShowTaskBarIcon"
#define CONFIG_KEY_GENERAL_SOUND_MENU_INTEGRATE          "SoundMenuIntegration"
#define CONFIG_KEY_GENERAL_START_MINIMIZED               "StartMinimized"
#define CONFIG_KEY_GENERAL_TEXT_SEARCH_ENTER             "TextSearchEnterRelax"


// Import Files
#define CONFIG_PATH_IMPORT_FILES_POSITION                "positions/import_files/position"
#define CONFIG_KEY_IMPORT_FILES_POS_X                    "PosX"
#define CONFIG_KEY_IMPORT_FILES_POS_Y                    "PosY"
#define CONFIG_KEY_IMPORT_FILES_WIDTH                    "Width"
#define CONFIG_KEY_IMPORT_FILES_HEIGHT                   "Height"


// Jamendo
#define CONFIG_PATH_JAMENDO                              "jamendo"
#define CONFIG_KEY_JAMENDO_AUDIOFORMAT                   "AudioFormat"
#define CONFIG_KEY_JAMENDO_TORRENT_COMMAND               "TorrentCommand"
#define CONFIG_KEY_JAMENDO_LAST_UPDATE                   "LastUpdate"
#define CONFIG_KEY_JAMENDO_NEED_UPGRADE                  "NeedUpgrade"

#define CONFIG_PATH_JAMENDO_GENRES                       "jamendo/genres"
#define CONFIG_KEY_JAMENDO_GENRES_GENRE                  "Genre"


// LastFm
#define CONFIG_PATH_LASTFM                               "lastfm"
#define CONFIG_KEY_LASTFM_USERNAME                       "UserName"
#define CONFIG_KEY_LASTFM_PASSWORD                       "Password"
#define CONFIG_KEY_LASTFM_ENABLED                        "SubmitEnabled"
#define CONFIG_KEY_LASTFM_SESSIONKEY                     "SessionKey"
#define CONFIG_KEY_LASTFM_LANGUAGE                       "Language"

#define CONFIG_KEY_LASTFM_SHOW_LONG_BIO                  "ShowLongBioText"
#define CONFIG_KEY_LASTFM_SHOW_ARTIST_INFO               "ShowArtistInfo"
#define CONFIG_KEY_LASTFM_SHOW_ALBUMS                    "ShowAlbums"
#define CONFIG_KEY_LASTFM_SHOW_TOP_TRACKS                "ShowTopTracks"
#define CONFIG_KEY_LASTFM_SHOW_ARTISTS                   "ShowArtists"
#define CONFIG_KEY_LASTFM_SHOW_TRACKS                    "ShowTracks"
#define CONFIG_KEY_LASTFM_SHOW_EVENTS                    "ShowEvents"
#define CONFIG_KEY_LASTFM_FOLLOW_PLAYER                  "FollowPlayer"


// LibreFm
#define CONFIG_PATH_LIBREFM                              "librefm"
#define CONFIG_KEY_LIBREFM_USERNAME                      "UserName"
#define CONFIG_KEY_LIBREFM_PASSWORD                      "Password"
#define CONFIG_KEY_LIBREFM_ENABLED                       "SubmitEnabled"



// Lyrics
#define CONFIG_PATH_LYRICS                               "lyrics"
#define CONFIG_KEY_LYRICS_FOLLOW_PLAYER                  "FollowPlayer"
#define CONFIG_KEY_LYRICS_TEXT_ALIGN                     "TextAlign"
#define CONFIG_KEY_LYRICS_FONT                           "Font"


// Magnatune
#define CONFIG_PATH_MAGNATUNE                            "magnatune"
#define CONFIG_KEY_MAGNATUNE_LAST_UPDATE                 "LastUpdate"
#define CONFIG_KEY_MAGNATUNE_MEMBERSHIP                  "MemberShip"
#define CONFIG_KEY_MAGNATUNE_USERNAME                    "UserName"
#define CONFIG_KEY_MAGNATUNE_PASSWORD                    "Password"
#define CONFIG_KEY_MAGNATUNE_NEED_UPGRADE                "NeedUpgrade"
#define CONFIG_KEY_MAGNATUNE_AUDIO_FORMAT                "AudioFormat"
#define CONFIG_KEY_MAGNATUNE_DOWNLOAD_FORMAT             "DownloadFormat"

#define CONFIG_PATH_MAGNATUNE_GENRES                     "magnatune/genres"
#define CONFIG_PATH_MAGNATUNE_GENRELIST                  "magnatune/genrelist"
#define CONFIG_KEY_MAGNATUNE_GENRES_GENRE                "Genre"


// Main Sources
#define CONFIG_PATH_MAIN_SOURCES                         "mainsources"
#define CONFIG_KEY_MAIN_SOURCES_LOCAL_MUSIC              "LocalMusic"
#define CONFIG_KEY_MAIN_SOURCES_ONLINE_MUSIC             "OnlineMusic"
#define CONFIG_KEY_MAIN_SOURCES_PORTABLE_DEVICES         "PortableDevices"
#define CONFIG_KEY_MAIN_SOURCES_CONTEXT                  "ContextExpanded"


// MainWindow
#define CONFIG_PATH_MAIN_WINDOW                          "mainwindow"
#define CONFIG_KEY_MAIN_WINDOW_VISIBLE_PANELS            "VisiblePanels"
#define CONFIG_KEY_MAIN_WINDOW_NOTEBOOK_LAYOUT           "NotebookLayout"
#define CONFIG_KEY_MAIN_WINDOW_LAST_LAYOUT               "LastLayout"
#define CONFIG_KEY_MAIN_WINDOW_FULLSCREEN                "ShowFullScreen"
#define CONFIG_KEY_MAIN_WINDOW_STATUS_BAR                "ShowStatusBar"
#define CONFIG_KEY_MAIN_WINDOW_CAPTIONS                  "ShowCaptions"

#define CONFIG_PATH_MAIN_WINDOW_FULLSCREEN               "mainwindow/fullscreen"
#define CONFIG_KEY_MAIN_WINDOW_FULLSCREEN_LAST_LAYOUT    "LastLayout"
#define CONFIG_KEY_MAIN_WINDOW_FULLSCREEN_VISIBLE_PANELS "VisiblePanels"

#define CONFIG_PATH_MAIN_WINDOW_POSITIONS                "mainwindow/positions"
#define CONFIG_KEY_MAIN_WINDOW_POSITIONS_POSX            "PosX"
#define CONFIG_KEY_MAIN_WINDOW_POSITIONS_POSY            "PosY"
#define CONFIG_KEY_MAIN_WINDOW_POSITIONS_WIDTH           "Width"
#define CONFIG_KEY_MAIN_WINDOW_POSITIONS_HEIGHT          "Height"

#define CONFIG_PATH_MAIN_WINDOW_NOTEBOOK                 "mainwindow/notebook"
#define CONFIG_KEY_MAIN_WINDOW_NOTEBOOK_LAST_LAYOUT      "LastLayout"


// Playback
#define CONFIG_PATH_PLAYBACK                             "playback"
#define CONFIG_KEY_PLAYBACK_OUTPUT_DEVICE                "OutputDevice"
#define CONFIG_KEY_PLAYBACK_OUTPUT_DEVICE_NAME           "OutputDeviceName"
#define CONFIG_KEY_PLAYBACK_NUM_TRACKS_TO_ADD            "NumTracksToAdd"
#define CONFIG_KEY_PLAYBACK_MAX_TRACKS_PLAYED            "MaxTracksPlayed"
#define CONFIG_KEY_PLAYBACK_MIN_TRACKS_PLAY              "MinTracksToPlay"
#define CONFIG_KEY_PLAYBACK_DEL_TRACKS_PLAYED            "DelTracksPlayed"
#define CONFIG_KEY_PLAYBACK_SMART_FILTER_ARTISTS         "SmartFilterArtists"
#define CONFIG_KEY_PLAYBACK_SMART_FILTER_TRACKS          "SmartFilterTracks"
#define CONFIG_KEY_PLAYBACK_PLAYLIST_ALLOW_FILTER        "PlayListAllowFilter"
#define CONFIG_KEY_PLAYBACK_PLAYLIST_DENY_FILTER         "PlayListDenyFilter"
#define CONFIG_KEY_PLAYBCK_SILENCE_DETECTOR              "SilenceDetector"
#define CONFIG_KEY_PLAYBCK_SILENCE_LEVEL                 "SilenceLevel"
#define CONFIG_KEY_PLAYBCK_SILENCE_AT_END                "SilenceAtEnd"
#define CONFIG_KEY_PLAYBCK_SILENCE_END_TIME              "SilenceEndTime"


// Playlist
#define CONFIG_PATH_PLAYLIST                             "playlist"
#define CONFIG_KEY_PLAYLIST_SAVE_ON_CLOSE                "SaveOnClose"
#define CONFIG_PATH_PLAYLIST_NOWPLAYING                  "playlist/nowplaying"
#define CONFIG_KEY_PLAYLIST_CURITEM                      "CurItem"


// Podcasts
#define CONFIG_PATH_PODCASTS                             "podcasts"
#define CONFIG_KEY_PODCASTS_PATH                         "Path"
#define CONFIG_KEY_PODCASTS_UPDATE                       "Update"
#define CONFIG_KEY_PODCASTS_LASTUPDATE                   "LastPodcastUpdate"
#define CONFIG_KEY_PODCASTS_UPDATEPERIOD                 "UpdatePeriod"
#define CONFIG_KEY_PODCASTS_DELETE                       "Delete"
#define CONFIG_KEY_PODCASTS_DELETETIME                   "DeleteTime"
#define CONFIG_KEY_PODCASTS_DELETEPERIOD                 "DeletePeriod"
#define CONFIG_KEY_PODCASTS_DELETEPLAYED                 "DeletePlayed"
#define CONFIG_KEY_PODCASTS_VISIBLE_PANELS               "VisiblePanels"
#define CONFIG_KEY_PODCASTS_LASTLAYOUT                   "LastLayout"
#define CONFIG_KEY_PODCASTS_ORDER                        "Order"
#define CONFIG_KEY_PODCASTS_ORDERDESC                    "OrderDesc"


// Positions
#define CONFIG_PATH_POSITIONS                            "positions"
#define CONFIG_KEY_POSITIONS_LABELEDIT_POSX              "LabelEditPosX"
#define CONFIG_KEY_POSITIONS_LABELEDIT_POSY              "LabelEditPosX"
#define CONFIG_KEY_POSITIONS_LABELEDIT_WIDTH             "LabelEditWidth"
#define CONFIG_KEY_POSITIONS_LABELEDIT_HEIGHT            "LabelEditHeight"
#define CONFIG_KEY_POSITIONS_LABELEDIT_SASHPOS           "LabelEditSashPos"

#define CONFIG_KEY_POSITIONS_PMPROPERTIES_POSX           "PMPropertiesPosX"
#define CONFIG_KEY_POSITIONS_PMPROPERTIES_POSY           "PMPropertiesPosY"
#define CONFIG_KEY_POSITIONS_PMPROPERTIES_WIDTH          "PMPropertiesWidth"
#define CONFIG_KEY_POSITIONS_PMPROPERTIES_HEIGHT         "PMPropertiesHeight"

#define CONFIG_KEY_POSITIONS_TRACKEDIT_POSX              "TrackEditPosX"
#define CONFIG_KEY_POSITIONS_TRACKEDIT_POSY              "TrackEditPosY"
#define CONFIG_KEY_POSITIONS_TRACKEDIT_WIDTH             "TrackEditWidth"
#define CONFIG_KEY_POSITIONS_TRACKEDIT_HEIGHT            "TrackEditHeight"
#define CONFIG_KEY_POSITIONS_TRACKEDIT_SASHPOS           "TrackEditSashPos"

// Preferences
#define CONFIG_PATH_PREFERENCES                          "preferences"
#define CONFIG_KEY_PREFERENCES_POSX                      "PosX"
#define CONFIG_KEY_PREFERENCES_POSY                      "PosY"
#define CONFIG_KEY_PREFERENCES_WIDTH                     "Width"
#define CONFIG_KEY_PREFERENCES_HEIGHT                    "Height"
#define CONFIG_KEY_PREFERENCES_LAST_PAGE                 "LastPage"


// Radios
#define CONFIG_PATH_RADIOS                               "radios"
#define CONFIG_KEY_RADIOS_STATIONS_ORDER                 "StationsOrder"
#define CONFIG_KEY_RADIOS_STATIONS_ORDERDESC             "StationsOrderDesc"
#define CONFIG_KEY_RADIOS_MIN_BITRATE                    "MinBitrate"
#define CONFIG_KEY_RADIOS_VISIBLE_PANELS                 "VisiblePanels"
#define CONFIG_KEY_RADIOS_LAST_LAYOUT                    "LastLayout"


// Record
#define CONFIG_PATH_RECORD                               "record"
#define CONFIG_KEY_RECORD_ENABLED                        "Enabled"
#define CONFIG_KEY_RECORD_PATH                           "Path"
#define CONFIG_KEY_RECORD_FORMAT                         "Format"
#define CONFIG_KEY_RECORD_QUALITY                        "Quality"
#define CONFIG_KEY_RECORD_SPLIT                          "Split"
#define CONFIG_KEY_RECORD_DELETE                         "DeleteTracks"
#define CONFIG_KEY_RECORD_DELETE_TIME                    "DeleteTime"


// Search Filters
#define CONFIG_PATH_SEARCH_FILTERS                       "searchfilters"
#define CONFIG_KEY_SEARCH_FILTERS_FILTER                 "Filter"


// Search Links
#define CONFIG_PATH_SEARCHLINKS_LINKS                    "searchlinks/links"
#define CONFIG_PATH_SEARCHLINKS_NAMES                    "searchlinks/names"
#define CONFIG_KEY_SEARCHLINKS_LINK                      "Link"
#define CONFIG_KEY_SEARCHLINKS_NAME                      "Name"

// Proxy
#define CONFIG_PATH_PROXY                               "proxy"
#define CONFIG_KEY_PROXY_ENABLED                        "enabled"
#define CONFIG_KEY_PROXY_HOSTNAME                       "hostname"
#define CONFIG_KEY_PROXY_PORT                           "port"
#define CONFIG_KEY_PROXY_USERNAME                       "username"
#define CONFIG_KEY_PROXY_PASSWORD                       "password"

}

#endif
// -------------------------------------------------------------------------------- //
