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
//    http://www.gnu.org/copyleft/gpl.h"tml
//
// -------------------------------------------------------------------------------- //
#include "Images.h"

#include <wx/mstream.h>

// -------------------------------------------------------------------------------- //
#include "./images/add_png.h"
#include "./images/blank_cd_cover_png.h"
#include "./images/bookmark_png.h"
#include "./images/default_lastfm_image_png.h"
#include "./images/del_png.h"
#include "./images/doc_new_png.h"
#include "./images/doc_save_png.h"
#include "./images/download_covers_png.h"
#include "./images/down_png.h"
#include "./images/edit_clear_png.h"
#include "./images/edit_copy_png.h"
#include "./images/edit_delete_png.h"
#include "./images/edit_png.h"
#include "./images/exit_png.h"
#include "./images/grey_star_big.h"
#include "./images/grey_star_mid.h"
#include "./images/grey_star_tiny.h"
#include "./images/guayadeque.h"
#include "./images/guayadeque_taskbar.h"
#include "./images/lastfm_as_off_png.h"
#include "./images/lastfm_as_on_png.h"
#include "./images/lastfm_on.h"
#include "./images/left_png.h"
#include "./images/musicbrainz.h"
#include "./images/net_radio_jpg.h"
#include "./images/no_cover_jpg.h"
#include "./images/no_photo_png.h"
#include "./images/numerate_png.h"
#include "./images/playback_pause_png.h"
#include "./images/playback_start_png.h"
#include "./images/playback_stop_png.h"
#include "./images/playlist_repeat_png.h"
#include "./images/playlist_shuffle_png.h"
#include "./images/playlist_smart_png.h"
#include "./images/podcast_icon.h"
#include "./images/right_png.h"
#include "./images/search_engine.h"
#include "./images/search_png.h"
#include "./images/skip_backward_png.h"
#include "./images/skip_forward_png.h"
#include "./images/splash.h"
#include "./images/system_run_png.h"
#include "./images/tags_png.h"
#include "./images/tiny_accept_png.h"
#include "./images/tiny_add_png.h"
#include "./images/tiny_del_png.h"
#include "./images/tiny_doc_save.h"
#include "./images/tiny_edit_clear.h"
#include "./images/tiny_edit_copy.h"
#include "./images/tiny_left.h"
#include "./images/tiny_mixer.h"
#include "./images/tiny_net_radio.h"
#include "./images/tiny_numerate.h"
#include "./images/tiny_playback_pause_png.h"
#include "./images/tiny_playback_start_png.h"
#include "./images/tiny_playlist_repeat.h"
#include "./images/tiny_playlist_repeat_single.h"
#include "./images/tiny_playlist_shufle.h"
#include "./images/tiny_podcast_icon.h"
#include "./images/tiny_reload.h"
#include "./images/tiny_right.h"
#include "./images/tiny_search.h"
#include "./images/tiny_search_again.h"
#include "./images/tiny_search_engine.h"
#include "./images/tiny_shoutcast.h"
#include "./images/tiny_skip_backward.h"
#include "./images/tiny_skip_forward.h"
#include "./images/tiny_status_error.h"
#include "./images/tiny_status_pending.h"
#include "./images/tiny_volume_high.h"
#include "./images/tiny_volume_low.h"
#include "./images/tiny_volume_medium.h"
#include "./images/tiny_volume_muted.h"
#include "./images/track_png.h"
#include "./images/up_png.h"
#include "./images/yellow_star_big.h"
#include "./images/yellow_star_mid.h"
#include "./images/yellow_star_tiny.h"
#include "./images/pref_commands.h"
#include "./images/pref_copy_to.h"
#include "./images/pref_general.h"
#include "./images/pref_last_fm.h"
#include "./images/pref_library.h"
#include "./images/pref_links.h"
#include "./images/pref_online_services.h"
#include "./images/pref_playback.h"
#include "./images/pref_podcasts.h"

// -------------------------------------------------------------------------------- //
typedef struct {
    const unsigned char * imgdata;
    unsigned int          imgsize;
    long                  imgtype;
} guImage_Item;

#define GUIMAGE( IMAGENAME, IMAGETYPE )    { IMAGENAME, sizeof( IMAGENAME ), IMAGETYPE }

// -------------------------------------------------------------------------------- //
guImage_Item guImage_Items[] = {
    GUIMAGE( guImage_add,                           wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_blank_cd_cover,                wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_bookmark,                      wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_default_lastfm_image,          wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_del,                           wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_doc_new,                       wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_doc_save,                      wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_download_covers,               wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_down,                          wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_edit_clear,                    wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_edit_copy,                     wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_edit_delete,                   wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_edit,                          wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_exit,                          wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_guayadeque,                    wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_guayadeque_taskbar,            wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_lastfm_as_off,                 wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_lastfm_as_on,                  wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_lastfm_on,                     wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_left,                          wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_net_radio,                     wxBITMAP_TYPE_JPEG ),
    GUIMAGE( guImage_no_cover,                      wxBITMAP_TYPE_JPEG ),
    GUIMAGE( guImage_no_photo,                      wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_numerate,                      wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_playback_pause,                wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_playback_start,                wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_playback_stop,                 wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_playlist_repeat,               wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_playlist_shuffle,              wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_playlist_smart,                wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_right,                         wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_search,                        wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_skip_backward,                 wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_skip_forward,                  wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_splash,                        wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_system_run,                    wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tags,                          wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_accept,                   wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_add,                      wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_del,                      wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_playback_pause,           wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_playback_start,           wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_up,                            wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_volume_high,              wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_volume_low,               wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_volume_medium,            wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_volume_muted,             wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_grey_star_tiny,                wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_grey_star_mid,                 wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_grey_star_big,                 wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_yellow_star_tiny,              wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_yellow_star_mid,               wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_yellow_star_big,               wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_track,                         wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_search,                   wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_search_engine,	                wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_musicbrainz,	                wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_edit_copy,	            wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_search_again,             wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_numerate,	                wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_edit_clear,               wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_podcast_icon,                  wxBITMAP_TYPE_JPEG ),
    GUIMAGE( guImage_tiny_podcast_icon,             wxBITMAP_TYPE_JPEG ),
    GUIMAGE( guImage_tiny_status_pending,           wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_status_error,             wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_doc_save,                 wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_reload,                   wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_shoutcast,                wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_net_radio,                wxBITMAP_TYPE_JPEG ),
    GUIMAGE( guImage_tiny_left,                     wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_right,                    wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_skip_backward,            wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_skip_forward,             wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_search_engine,            wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_playlist_shuffle,         wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_playlist_repeat,          wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_playlist_repeat_single,   wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_mixer,                    wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_pref_commands,                 wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_pref_copy_to,                  wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_pref_general,                  wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_pref_last_fm,                  wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_pref_library,                  wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_pref_links,                    wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_pref_online_services,          wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_pref_playback,                 wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_pref_podcasts,                 wxBITMAP_TYPE_JPEG )
};


// -------------------------------------------------------------------------------- //
wxBitmap guBitmap( guIMAGE_INDEX imageindex )
{
    return wxBitmap( guImage( imageindex ) );
}

// -------------------------------------------------------------------------------- //
wxImage guImage( guIMAGE_INDEX imageindex )
{
    wxMemoryInputStream image_stream( guImage_Items[ imageindex ].imgdata,
                                      guImage_Items[ imageindex ].imgsize );
    return wxImage( image_stream, guImage_Items[ imageindex ].imgtype );
}

// -------------------------------------------------------------------------------- //
