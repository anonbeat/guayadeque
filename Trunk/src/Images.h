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
#ifndef IMAGES_H
#define IMAGES_H

#include <wx/bitmap.h>
#include <wx/image.h>

enum guIMAGE_INDEX {
    guIMAGE_INDEX_add = 0,
    guIMAGE_INDEX_blank_cd_cover,
    guIMAGE_INDEX_bookmark,
    guIMAGE_INDEX_default_lastfm_image,
    guIMAGE_INDEX_del,
    guIMAGE_INDEX_doc_new,
    guIMAGE_INDEX_doc_save,
    guIMAGE_INDEX_download_covers,
    guIMAGE_INDEX_down,
    guIMAGE_INDEX_edit_clear,
    guIMAGE_INDEX_edit_copy,
    guIMAGE_INDEX_edit_delete,
    guIMAGE_INDEX_edit,
    guIMAGE_INDEX_exit,
    guIMAGE_INDEX_guayadeque,
    guIMAGE_INDEX_guayadeque_taskbar,
    guIMAGE_INDEX_lastfm_as_off,
    guIMAGE_INDEX_lastfm_as_on,
    guIMAGE_INDEX_lastfm_on,
    guIMAGE_INDEX_left,
    guIMAGE_INDEX_net_radio,
    guIMAGE_INDEX_no_cover,
    guIMAGE_INDEX_no_photo,
    guIMAGE_INDEX_numerate,
    guIMAGE_INDEX_playback_pause,
    guIMAGE_INDEX_playback_start,
    guIMAGE_INDEX_playback_stop,
    guIMAGE_INDEX_playlist_repeat,
    guIMAGE_INDEX_playlist_shuffle,
    guIMAGE_INDEX_playlist_smart,
    guIMAGE_INDEX_right,
    guIMAGE_INDEX_search,
    guIMAGE_INDEX_skip_backward,
    guIMAGE_INDEX_skip_forward,
    guIMAGE_INDEX_splash,
    guIMAGE_INDEX_system_run,
    guIMAGE_INDEX_tags,
    guIMAGE_INDEX_tiny_accept,
    guIMAGE_INDEX_tiny_add,
    guIMAGE_INDEX_tiny_del,
    guIMAGE_INDEX_tiny_playback_pause,
    guIMAGE_INDEX_tiny_playback_start,
    guIMAGE_INDEX_up,
    guIMAGE_INDEX_tiny_volume_high,
    guIMAGE_INDEX_tiny_volume_low,
    guIMAGE_INDEX_tiny_volume_medium,
    guIMAGE_INDEX_tiny_volume_muted,
    guIMAGE_INDEX_grey_star_tiny,
    guIMAGE_INDEX_grey_star_mid,
    guIMAGE_INDEX_grey_star_big,
    guIMAGE_INDEX_yellow_star_tiny,
    guIMAGE_INDEX_yellow_star_mid,
    guIMAGE_INDEX_yellow_star_big,
    guIMAGE_INDEX_track,
    guIMAGE_INDEX_tiny_search,
    guIMAGE_INDEX_search_engine,
    guIMAGE_INDEX_musicbrainz,
    guIMAGE_INDEX_tiny_edit_copy,
    guIMAGE_INDEX_tiny_search_again,
    guIMAGE_INDEX_tiny_numerate,
    guIMAGE_INDEX_tiny_edit_clear,
    guIMAGE_INDEX_podcast_icon,
    guIMAGE_INDEX_tiny_podcast_icon,
    guIMAGE_INDEX_tiny_status_pending,
    guIMAGE_INDEX_tiny_status_error,
    guIMAGE_INDEX_tiny_doc_save,
    guIMAGE_INDEX_tiny_reload,
    guIMAGE_INDEX_tiny_shoutcast,
    guIMAGE_INDEX_tiny_net_radio,
    guIMAGE_INDEX_tiny_left,
    guIMAGE_INDEX_tiny_right,
    guIMAGE_INDEX_tiny_skip_backward,
    guIMAGE_INDEX_tiny_skip_forward,
    guIMAGE_INDEX_tiny_search_engine,
    guIMAGE_INDEX_tiny_playlist_shuffle,
    guIMAGE_INDEX_tiny_playlist_repeat,
    guIMAGE_INDEX_tiny_playlist_repeat_single,
    guIMAGE_INDEX_tiny_mixer,
    guIMAGE_INDEX_pref_commands,
    guIMAGE_INDEX_pref_copy_to,
    guIMAGE_INDEX_pref_general,
    guIMAGE_INDEX_pref_last_fm,
    guIMAGE_INDEX_pref_library,
    guIMAGE_INDEX_pref_links,
    guIMAGE_INDEX_pref_online_services,
    guIMAGE_INDEX_pref_playback,
    guIMAGE_INDEX_pref_podcasts
};


// -------------------------------------------------------------------------------- //
wxBitmap guBitmap( guIMAGE_INDEX imageindex );
wxImage guImage( guIMAGE_INDEX imageindex );

#endif
// -------------------------------------------------------------------------------- //