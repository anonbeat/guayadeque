// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2011 J.Rios
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
//    http://www.gnu.org/copyleft/gpl.h"tml
//
// -------------------------------------------------------------------------------- //
#include "Images.h"

#include <wx/mstream.h>

// -------------------------------------------------------------------------------- //
#include "./images/add.h"
#include "./images/blank_cd_cover.h"
#include "./images/bookmark.h"
#include "./images/default_lastfm_image.h"
#include "./images/del.h"
#include "./images/doc_new.h"
#include "./images/doc_save.h"
#include "./images/download_covers.h"
#include "./images/down.h"
#include "./images/edit_clear.h"
#include "./images/edit_copy.h"
#include "./images/edit_delete.h"
#include "./images/edit.h"
#include "./images/exit.h"
#include "./images/filter.h"
#include "./images/guayadeque.h"
#include "./images/guayadeque_taskbar.h"
#include "./images/lastfm_as_off.h"
#include "./images/lastfm_as_on.h"
#include "./images/lastfm_on.h"
#include "./images/left.h"
#include "./images/musicbrainz.h"
#include "./images/net_radio.h"
#include "./images/no_cover.h"
#include "./images/no_photo.h"
#include "./images/numerate.h"
#include "./images/podcast.h"
#include "./images/right.h"
#include "./images/search_engine.h"
#include "./images/search.h"
#include "./images/splash.h"
#include "./images/system_run.h"
#include "./images/tags.h"
#include "./images/tiny_accept.h"
#include "./images/tiny_add.h"
#include "./images/tiny_del.h"
#include "./images/tiny_doc_save.h"
#include "./images/tiny_edit_clear.h"
#include "./images/tiny_edit.h"
#include "./images/tiny_edit_copy.h"
#include "./images/tiny_filter.h"
#include "./images/tiny_left.h"
#include "./images/tiny_net_radio.h"
#include "./images/tiny_numerate.h"
#include "./images/mid_podcast.h"
#include "./images/tiny_podcast.h"
#include "./images/tiny_reload.h"
#include "./images/tiny_right.h"
#include "./images/tiny_search.h"
#include "./images/tiny_search_again.h"
#include "./images/tiny_search_engine.h"
#include "./images/tiny_shoutcast.h"
#include "./images/tiny_status_error.h"
#include "./images/tiny_status_pending.h"
#include "./images/track.h"
#include "./images/up.h"
#include "./images/tiny_library.h"
#include "./images/tiny_record.h"
#include "./images/pref_commands.h"
#include "./images/pref_copy_to.h"
#include "./images/pref_general.h"
#include "./images/pref_last_fm.h"
#include "./images/pref_library.h"
#include "./images/pref_links.h"
#include "./images/pref_lyrics.h"
#include "./images/pref_online_services.h"
#include "./images/pref_playback.h"
#include "./images/pref_podcasts.h"
#include "./images/pref_record.h"
#include "./images/pref_crossfader.h"
#include "./images/pref_jamendo.h"
#include "./images/pref_magnatune.h"
#include "./images/pref_accelerators.h"
//
#include "./images/loc_library.h"
#include "./images/loc_portable_device.h"
#include "./images/loc_net_radio.h"
#include "./images/loc_podcast.h"
#include "./images/loc_magnatune.h"
#include "./images/loc_jamendo.h"
#include "./images/loc_lastfm.h"
#include "./images/loc_lyrics.h"
//
#include "./images/tiny_close_normal.h"
#include "./images/tiny_close_highlight.h"
//
#include "./images/player_highlight_equalizer.h"
#include "./images/player_highlight_muted.h"
#include "./images/player_highlight_next.h"
#include "./images/player_highlight_pause.h"
#include "./images/player_highlight_play.h"
#include "./images/player_highlight_prev.h"
#include "./images/player_highlight_random.h"
#include "./images/player_highlight_record.h"
#include "./images/player_highlight_repeat.h"
#include "./images/player_highlight_repeat_single.h"
#include "./images/player_highlight_search.h"
#include "./images/player_highlight_setup.h"
#include "./images/player_highlight_smart.h"
#include "./images/player_highlight_stop.h"
#include "./images/player_highlight_vol_hi.h"
#include "./images/player_highlight_vol_low.h"
#include "./images/player_highlight_vol_mid.h"
#include "./images/player_highlight_love.h"
#include "./images/player_highlight_ban.h"
#include "./images/player_light_equalizer.h"
#include "./images/player_light_muted.h"
#include "./images/player_light_next.h"
#include "./images/player_light_pause.h"
#include "./images/player_light_play.h"
#include "./images/player_light_prev.h"
#include "./images/player_light_random.h"
#include "./images/player_light_record.h"
#include "./images/player_light_repeat.h"
#include "./images/player_light_repeat_single.h"
#include "./images/player_light_search.h"
#include "./images/player_light_setup.h"
#include "./images/player_light_smart.h"
#include "./images/player_light_stop.h"
#include "./images/player_light_vol_hi.h"
#include "./images/player_light_vol_low.h"
#include "./images/player_light_vol_mid.h"
#include "./images/player_light_love.h"
#include "./images/player_light_ban.h"
#include "./images/player_normal_equalizer.h"
#include "./images/player_normal_muted.h"
#include "./images/player_normal_next.h"
#include "./images/player_normal_pause.h"
#include "./images/player_normal_play.h"
#include "./images/player_normal_prev.h"
#include "./images/player_normal_random.h"
#include "./images/player_normal_record.h"
#include "./images/player_normal_repeat.h"
#include "./images/player_normal_repeat_single.h"
#include "./images/player_normal_search.h"
#include "./images/player_normal_setup.h"
#include "./images/player_normal_smart.h"
#include "./images/player_normal_stop.h"
#include "./images/player_normal_vol_hi.h"
#include "./images/player_normal_vol_low.h"
#include "./images/player_normal_vol_mid.h"
#include "./images/player_normal_love.h"
#include "./images/player_normal_ban.h"
#include "./images/player_tiny_light_play.h"
#include "./images/player_tiny_light_stop.h"
#include "./images/player_tiny_red_stop.h"
//
#include "./images/star_normal_tiny.h"
#include "./images/star_normal_mid.h"
#include "./images/star_normal_big.h"
#include "./images/star_highlight_tiny.h"
#include "./images/star_highlight_mid.h"
#include "./images/star_highlight_big.h"
//
#include "./images/tiny_crossfade.h"
#include "./images/tiny_gapless.h"
//
#include "./images/tiny_mv_library.h"
#include "./images/tiny_mv_albumbrowser.h"
#include "./images/tiny_mv_treeview.h"
#include "./images/tiny_mv_playlists.h"

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
    GUIMAGE( guImage_filter,                        wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_guayadeque,                    wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_guayadeque_taskbar,            wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_lastfm_as_off,                 wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_lastfm_as_on,                  wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_lastfm_on,                     wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_left,                          wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_net_radio,                     wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_no_cover,                      wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_no_photo,                      wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_numerate,                      wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_right,                         wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_search,                        wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_splash,                        wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_system_run,                    wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tags,                          wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_accept,                   wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_add,                      wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_del,                      wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_up,                            wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_track,                         wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_search,                   wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_search_engine,	                wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_musicbrainz,	                wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_edit,                     wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_edit_copy,	            wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_filter,	                wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_search_again,             wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_numerate,	                wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_edit_clear,               wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_podcast,                  	    wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_mid_podcast,                   wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_podcast,                  wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_status_pending,           wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_status_error,             wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_doc_save,                 wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_reload,                   wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_shoutcast,                wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_net_radio,                wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_left,                     wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_right,                    wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_search_engine,            wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_library,                  wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_record,                   wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_pref_commands,                 wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_pref_copy_to,                  wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_pref_general,                  wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_pref_last_fm,                  wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_pref_library,                  wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_pref_links,                    wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_pref_lyrics,                   wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_pref_online_services,          wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_pref_playback,                 wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_pref_podcasts,                 wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_pref_record,                   wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_pref_crossfader,		        wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_pref_jamendo,		            wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_pref_magnatune,		        wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_pref_accelerators,		        wxBITMAP_TYPE_PNG ),
    //
    GUIMAGE( guImage_loc_library,		            wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_loc_portable_device,	        wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_loc_net_radio,	                wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_loc_podcast,	                wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_loc_magnatune,	                wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_loc_jamendo,	                wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_loc_lastfm,	                wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_loc_lyrics,			        wxBITMAP_TYPE_PNG ),
    //
    GUIMAGE( guImage_tiny_close_normal,		        wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_close_highlight,	        wxBITMAP_TYPE_PNG ),
    //
    GUIMAGE( guImage_player_highlight_equalizer,    wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_highlight_muted,        wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_highlight_next,         wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_highlight_pause,        wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_highlight_play,         wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_highlight_prev,         wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_highlight_random,       wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_highlight_record,       wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_highlight_repeat,       wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_highlight_repeat_single, wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_highlight_search,       wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_highlight_setup,        wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_highlight_smart,        wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_highlight_stop,         wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_highlight_vol_hi,       wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_highlight_vol_low,      wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_highlight_vol_mid,      wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_highlight_love,         wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_highlight_ban,          wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_light_equalizer,        wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_light_muted,            wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_light_next,             wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_light_pause,            wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_light_play,             wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_light_prev,             wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_light_random,           wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_light_record,           wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_light_repeat,           wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_light_repeat_single,    wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_light_search,           wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_light_setup,            wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_light_smart,            wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_light_stop,             wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_light_vol_hi,           wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_light_vol_low,          wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_light_vol_mid,          wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_light_love,             wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_light_ban,              wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_normal_equalizer,       wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_normal_muted,           wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_normal_next,            wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_normal_pause,           wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_normal_play,            wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_normal_prev,            wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_normal_random,          wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_normal_record,          wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_normal_repeat,          wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_normal_repeat_single,   wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_normal_search,          wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_normal_setup,           wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_normal_smart,           wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_normal_stop,            wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_normal_vol_hi,          wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_normal_vol_low,         wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_normal_vol_mid,         wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_normal_love,            wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_normal_ban,             wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_tiny_light_play,        wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_tiny_light_stop,        wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_player_tiny_red_stop,          wxBITMAP_TYPE_PNG ),
    //
    GUIMAGE( guImage_star_normal_tiny,              wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_star_normal_mid,               wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_star_normal_big,               wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_star_highlight_tiny,           wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_star_highlight_mid,            wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_star_highlight_big,            wxBITMAP_TYPE_PNG ),
    //
    GUIMAGE( guImage_tiny_crossfade,                wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_gapless,                  wxBITMAP_TYPE_PNG ),
    //
    GUIMAGE( guImage_tiny_mv_library,               wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_mv_albumbrowser,          wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_mv_treeview,              wxBITMAP_TYPE_PNG ),
    GUIMAGE( guImage_tiny_mv_playlists,             wxBITMAP_TYPE_PNG )
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
