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
//    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//    http://www.gnu.org/copyleft/gpl.h"tml
//
// -------------------------------------------------------------------------------- //
#include "Images.h"

#include <map>
// -------------------------------------------------------------------------------- //
#include "images/add.h"
#include "images/blank_cd_cover.h"
#include "images/bookmark.h"
#include "images/default_lastfm_image.h"
#include "images/del.h"
#include "images/doc_new.h"
#include "images/doc_save.h"
#include "images/download_covers.h"
#include "images/down.h"
#include "images/edit_clear.h"
#include "images/edit_copy.h"
#include "images/edit_delete.h"
#include "images/edit.h"
#include "images/exit.h"
#include "images/filter.h"
#include "images/guayadeque.h"
#include "images/guayadeque_taskbar.h"
#include "images/lastfm_as_off.h"
#include "images/lastfm_as_on.h"
#include "images/lastfm_on.h"
#include "images/left.h"
#include "images/musicbrainz.h"
#include "images/net_radio.h"
#include "images/no_cover.h"
#include "images/no_photo.h"
#include "images/numerate.h"
#include "images/podcast.h"
#include "images/right.h"
#include "images/search_engine.h"
#include "images/search.h"
#include "images/splash.h"
#include "images/system_run.h"
#include "images/tags.h"
#include "images/tiny_accept.h"
#include "images/tiny_add.h"
#include "images/tiny_del.h"
#include "images/tiny_doc_save.h"
#include "images/tiny_edit_clear.h"
#include "images/tiny_edit.h"
#include "images/tiny_edit_copy.h"
#include "images/tiny_filter.h"
#include "images/tiny_left.h"
#include "images/tiny_net_radio.h"
#include "images/tiny_numerate.h"
#include "images/mid_podcast.h"
#include "images/tiny_podcast.h"
#include "images/tiny_reload.h"
#include "images/tiny_right.h"
#include "images/tiny_search.h"
#include "images/tiny_search_again.h"
#include "images/tiny_search_engine.h"
#include "images/tiny_shoutcast.h"
#include "images/tiny_tunein.h"
#include "images/tiny_status_error.h"
#include "images/tiny_status_pending.h"
#include "images/track.h"
#include "images/up.h"
#include "images/tiny_library.h"
#include "images/tiny_record.h"
#include "images/pref_commands.h"
#include "images/pref_copy_to.h"
#include "images/pref_general.h"
#include "images/pref_last_fm.h"
#include "images/pref_library.h"
#include "images/pref_links.h"
#include "images/pref_lyrics.h"
#include "images/pref_online_services.h"
#include "images/pref_playback.h"
#include "images/pref_podcasts.h"
#include "images/pref_record.h"
#include "images/pref_crossfader.h"
#include "images/pref_jamendo.h"
#include "images/pref_magnatune.h"
#include "images/pref_accelerators.h"
//
#include "images/loc_library.h"
#include "images/loc_portable_device.h"
#include "images/loc_net_radio.h"
#include "images/loc_podcast.h"
#include "images/loc_magnatune.h"
#include "images/loc_jamendo.h"
#include "images/loc_lastfm.h"
#include "images/loc_lyrics.h"
//
#include "images/tiny_close_normal.h"
#include "images/tiny_close_highlight.h"
//
#include "images/player_highlight_equalizer.h"
#include "images/player_highlight_muted.h"
#include "images/player_highlight_next.h"
#include "images/player_highlight_pause.h"
#include "images/player_highlight_play.h"
#include "images/player_highlight_prev.h"
#include "images/player_highlight_random.h"
#include "images/player_highlight_record.h"
#include "images/player_highlight_repeat.h"
#include "images/player_highlight_repeat_single.h"
#include "images/player_highlight_search.h"
#include "images/player_highlight_setup.h"
#include "images/player_highlight_smart.h"
#include "images/player_highlight_stop.h"
#include "images/player_highlight_vol_hi.h"
#include "images/player_highlight_vol_low.h"
#include "images/player_highlight_vol_mid.h"
#include "images/player_highlight_love.h"
#include "images/player_highlight_ban.h"
#include "images/player_light_equalizer.h"
#include "images/player_light_muted.h"
#include "images/player_light_next.h"
#include "images/player_light_pause.h"
#include "images/player_light_play.h"
#include "images/player_light_prev.h"
#include "images/player_light_random.h"
#include "images/player_light_record.h"
#include "images/player_light_repeat.h"
#include "images/player_light_repeat_single.h"
#include "images/player_light_search.h"
#include "images/player_light_setup.h"
#include "images/player_light_smart.h"
#include "images/player_light_stop.h"
#include "images/player_light_vol_hi.h"
#include "images/player_light_vol_low.h"
#include "images/player_light_vol_mid.h"
#include "images/player_light_love.h"
#include "images/player_light_ban.h"
#include "images/player_normal_equalizer.h"
#include "images/player_normal_muted.h"
#include "images/player_normal_next.h"
#include "images/player_normal_pause.h"
#include "images/player_normal_play.h"
#include "images/player_normal_prev.h"
#include "images/player_normal_random.h"
#include "images/player_normal_record.h"
#include "images/player_normal_repeat.h"
#include "images/player_normal_repeat_single.h"
#include "images/player_normal_search.h"
#include "images/player_normal_setup.h"
#include "images/player_normal_smart.h"
#include "images/player_normal_stop.h"
#include "images/player_normal_vol_hi.h"
#include "images/player_normal_vol_low.h"
#include "images/player_normal_vol_mid.h"
#include "images/player_normal_love.h"
#include "images/player_normal_ban.h"
#include "images/player_tiny_light_play.h"
#include "images/player_tiny_light_stop.h"
#include "images/player_tiny_red_stop.h"
//
#include "images/star_normal_tiny.h"
#include "images/star_normal_mid.h"
#include "images/star_normal_big.h"
#include "images/star_highlight_tiny.h"
#include "images/star_highlight_mid.h"
#include "images/star_highlight_big.h"
//
#include "images/tiny_crossfade.h"
#include "images/tiny_gapless.h"
//
#include "images/tiny_mv_library.h"
#include "images/tiny_mv_albumbrowser.h"
#include "images/tiny_mv_treeview.h"
#include "images/tiny_mv_playlists.h"

// -------------------------------------------------------------------------------- //
namespace guNS_Image
{
// -------------------------------------------------------------------------------- //
    std::map<unsigned short, guIMG_ITEM> guImage_Items = {
        { guIMAGE_INDEX_add,                            GUIMAGE( guImage_add )},
        { guIMAGE_INDEX_blank_cd_cover,                 GUIMAGE( guImage_blank_cd_cover )},
        { guIMAGE_INDEX_bookmark,                       GUIMAGE( guImage_bookmark )},
        { guIMAGE_INDEX_default_lastfm_image,           GUIMAGE( guImage_default_lastfm_image )},
        { guIMAGE_INDEX_del,                            GUIMAGE( guImage_del )},
        { guIMAGE_INDEX_doc_new,                        GUIMAGE( guImage_doc_new )},
        { guIMAGE_INDEX_doc_save,                       GUIMAGE( guImage_doc_save )},
        { guIMAGE_INDEX_download_covers,                GUIMAGE( guImage_download_covers )},
        { guIMAGE_INDEX_down,                           GUIMAGE( guImage_down )},
        { guIMAGE_INDEX_edit_clear,                     GUIMAGE( guImage_edit_clear )},
        { guIMAGE_INDEX_edit_copy,                      GUIMAGE( guImage_edit_copy )},
        { guIMAGE_INDEX_edit_delete,                    GUIMAGE( guImage_edit_delete )},
        { guIMAGE_INDEX_edit,                           GUIMAGE( guImage_edit )},
        { guIMAGE_INDEX_exit,                           GUIMAGE( guImage_exit )},
        { guIMAGE_INDEX_filter,                         GUIMAGE( guImage_filter )},
        { guIMAGE_INDEX_guayadeque,                     GUIMAGE( guImage_guayadeque )},
        { guIMAGE_INDEX_guayadeque_taskbar,             GUIMAGE( guImage_guayadeque_taskbar )},
        { guIMAGE_INDEX_lastfm_as_off,                  GUIMAGE( guImage_lastfm_as_off )},
        { guIMAGE_INDEX_lastfm_as_on,                   GUIMAGE( guImage_lastfm_as_on )},
        { guIMAGE_INDEX_lastfm_on,                      GUIMAGE( guImage_lastfm_on )},
        { guIMAGE_INDEX_left,                           GUIMAGE( guImage_left )},
        { guIMAGE_INDEX_net_radio,                      GUIMAGE( guImage_net_radio )},
        { guIMAGE_INDEX_no_cover,                       GUIMAGE( guImage_no_cover )},
        { guIMAGE_INDEX_no_photo,                       GUIMAGE( guImage_no_photo )},
        { guIMAGE_INDEX_numerate,                       GUIMAGE( guImage_numerate )},
        { guIMAGE_INDEX_right,                          GUIMAGE( guImage_right )},
        { guIMAGE_INDEX_search,                         GUIMAGE( guImage_search )},
        { guIMAGE_INDEX_splash,                         GUIMAGE( guImage_splash )},
        { guIMAGE_INDEX_system_run,                     GUIMAGE( guImage_system_run )},
        { guIMAGE_INDEX_tags,                           GUIMAGE( guImage_tags )},
        { guIMAGE_INDEX_tiny_accept,                    GUIMAGE( guImage_tiny_accept )},
        { guIMAGE_INDEX_tiny_add,                       GUIMAGE( guImage_tiny_add )},
        { guIMAGE_INDEX_tiny_del,                       GUIMAGE( guImage_tiny_del )},
        { guIMAGE_INDEX_up,                             GUIMAGE( guImage_up )},
        { guIMAGE_INDEX_track,                          GUIMAGE( guImage_track )},
        { guIMAGE_INDEX_tiny_search,                    GUIMAGE( guImage_tiny_search )},
        { guIMAGE_INDEX_search_engine,                  GUIMAGE( guImage_search_engine )},
        { guIMAGE_INDEX_musicbrainz,                    GUIMAGE( guImage_musicbrainz )},
        { guIMAGE_INDEX_tiny_edit,                      GUIMAGE( guImage_tiny_edit )},
        { guIMAGE_INDEX_tiny_edit_copy,                 GUIMAGE( guImage_tiny_edit_copy )},
        { guIMAGE_INDEX_tiny_filter,                    GUIMAGE( guImage_tiny_filter )},
        { guIMAGE_INDEX_tiny_search_again,              GUIMAGE( guImage_tiny_search_again )},
        { guIMAGE_INDEX_tiny_numerate,                  GUIMAGE( guImage_tiny_numerate )},
        { guIMAGE_INDEX_tiny_edit_clear,                GUIMAGE( guImage_tiny_edit_clear )},
        { guIMAGE_INDEX_podcast,                        GUIMAGE( guImage_podcast )},
        { guIMAGE_INDEX_mid_podcast,                    GUIMAGE( guImage_mid_podcast )},
        { guIMAGE_INDEX_tiny_podcast,                   GUIMAGE( guImage_tiny_podcast )},
        { guIMAGE_INDEX_tiny_status_pending,            GUIMAGE( guImage_tiny_status_pending )},
        { guIMAGE_INDEX_tiny_status_error,              GUIMAGE( guImage_tiny_status_error )},
        { guIMAGE_INDEX_tiny_doc_save,                  GUIMAGE( guImage_tiny_doc_save )},
        { guIMAGE_INDEX_tiny_reload,                    GUIMAGE( guImage_tiny_reload )},
        { guIMAGE_INDEX_tiny_shoutcast,                 GUIMAGE( guImage_tiny_shoutcast )},
        { guIMAGE_INDEX_tiny_tunein,                    GUIMAGE( guImage_tiny_tunein )},
        { guIMAGE_INDEX_tiny_net_radio,                 GUIMAGE( guImage_tiny_net_radio )},
        { guIMAGE_INDEX_tiny_left,                      GUIMAGE( guImage_tiny_left )},
        { guIMAGE_INDEX_tiny_right,                     GUIMAGE( guImage_tiny_right )},
        { guIMAGE_INDEX_tiny_search_engine,             GUIMAGE( guImage_tiny_search_engine )},
        { guIMAGE_INDEX_tiny_library,                   GUIMAGE( guImage_tiny_library )},
        { guIMAGE_INDEX_tiny_record,                    GUIMAGE( guImage_tiny_record )},
        { guIMAGE_INDEX_pref_commands,                  GUIMAGE( guImage_pref_commands )},
        { guIMAGE_INDEX_pref_copy_to,                   GUIMAGE( guImage_pref_copy_to )},
        { guIMAGE_INDEX_pref_general,                   GUIMAGE( guImage_pref_general )},
        { guIMAGE_INDEX_pref_last_fm,                   GUIMAGE( guImage_pref_last_fm )},
        { guIMAGE_INDEX_pref_library,                   GUIMAGE( guImage_pref_library )},
        { guIMAGE_INDEX_pref_links,                     GUIMAGE( guImage_pref_links )},
        { guIMAGE_INDEX_pref_lyrics,                    GUIMAGE( guImage_pref_lyrics )},
        { guIMAGE_INDEX_pref_online_services,           GUIMAGE( guImage_pref_online_services )},
        { guIMAGE_INDEX_pref_playback,                  GUIMAGE( guImage_pref_playback )},
        { guIMAGE_INDEX_pref_podcasts,                  GUIMAGE( guImage_pref_podcasts )},
        { guIMAGE_INDEX_pref_record,                    GUIMAGE( guImage_pref_record )},
        { guIMAGE_INDEX_pref_crossfader,                GUIMAGE( guImage_pref_crossfader )},
        { guIMAGE_INDEX_pref_jamendo,                   GUIMAGE( guImage_pref_jamendo )},
        { guIMAGE_INDEX_pref_magnatune,                 GUIMAGE( guImage_pref_magnatune )},
        { guIMAGE_INDEX_pref_accelerators,              GUIMAGE( guImage_pref_accelerators )},
        //
        { guIMAGE_INDEX_loc_library,                    GUIMAGE( guImage_loc_library )},
        { guIMAGE_INDEX_loc_portable_device,            GUIMAGE( guImage_loc_portable_device )},
        { guIMAGE_INDEX_loc_net_radio,                  GUIMAGE( guImage_loc_net_radio )},
        { guIMAGE_INDEX_loc_podcast,                    GUIMAGE( guImage_loc_podcast )},
        { guIMAGE_INDEX_loc_magnatune,                  GUIMAGE( guImage_loc_magnatune )},
        { guIMAGE_INDEX_loc_jamendo,                    GUIMAGE( guImage_loc_jamendo )},
        { guIMAGE_INDEX_loc_lastfm,                     GUIMAGE( guImage_loc_lastfm )},
        { guIMAGE_INDEX_loc_lyrics,                     GUIMAGE( guImage_loc_lyrics )},
        //
        { guIMAGE_INDEX_tiny_close_normal,              GUIMAGE( guImage_tiny_close_normal )},
        { guIMAGE_INDEX_tiny_close_highlight,           GUIMAGE( guImage_tiny_close_highlight )},
        //
        { guIMAGE_INDEX_player_highlight_equalizer,     GUIMAGE( guImage_player_highlight_equalizer )},
        { guIMAGE_INDEX_player_highlight_muted,         GUIMAGE( guImage_player_highlight_muted )},
        { guIMAGE_INDEX_player_highlight_next,          GUIMAGE( guImage_player_highlight_next )},
        { guIMAGE_INDEX_player_highlight_pause,         GUIMAGE( guImage_player_highlight_pause )},
        { guIMAGE_INDEX_player_highlight_play,          GUIMAGE( guImage_player_highlight_play )},
        { guIMAGE_INDEX_player_highlight_prev,          GUIMAGE( guImage_player_highlight_prev )},
        { guIMAGE_INDEX_player_highlight_random,        GUIMAGE( guImage_player_highlight_random )},
        { guIMAGE_INDEX_player_highlight_record,        GUIMAGE( guImage_player_highlight_record )},
        { guIMAGE_INDEX_player_highlight_repeat,        GUIMAGE( guImage_player_highlight_repeat )},
        { guIMAGE_INDEX_player_highlight_repeat_single, GUIMAGE( guImage_player_highlight_repeat_single )},
        { guIMAGE_INDEX_player_highlight_search,        GUIMAGE( guImage_player_highlight_search )},
        { guIMAGE_INDEX_player_highlight_setup,         GUIMAGE( guImage_player_highlight_setup )},
        { guIMAGE_INDEX_player_highlight_smart,         GUIMAGE( guImage_player_highlight_smart )},
        { guIMAGE_INDEX_player_highlight_stop,          GUIMAGE( guImage_player_highlight_stop )},
        { guIMAGE_INDEX_player_highlight_vol_hi,        GUIMAGE( guImage_player_highlight_vol_hi )},
        { guIMAGE_INDEX_player_highlight_vol_low,       GUIMAGE( guImage_player_highlight_vol_low )},
        { guIMAGE_INDEX_player_highlight_vol_mid,       GUIMAGE( guImage_player_highlight_vol_mid )},
        { guIMAGE_INDEX_player_highlight_love,          GUIMAGE( guImage_player_highlight_love )},
        { guIMAGE_INDEX_player_highlight_ban,           GUIMAGE( guImage_player_highlight_ban )},
        { guIMAGE_INDEX_player_light_equalizer,         GUIMAGE( guImage_player_light_equalizer )},
        { guIMAGE_INDEX_player_light_muted,             GUIMAGE( guImage_player_light_muted )},
        { guIMAGE_INDEX_player_light_next,              GUIMAGE( guImage_player_light_next )},
        { guIMAGE_INDEX_player_light_pause,             GUIMAGE( guImage_player_light_pause )},
        { guIMAGE_INDEX_player_light_play,              GUIMAGE( guImage_player_light_play )},
        { guIMAGE_INDEX_player_light_prev,              GUIMAGE( guImage_player_light_prev )},
        { guIMAGE_INDEX_player_light_random,            GUIMAGE( guImage_player_light_random )},
        { guIMAGE_INDEX_player_light_record,            GUIMAGE( guImage_player_light_record )},
        { guIMAGE_INDEX_player_light_repeat,            GUIMAGE( guImage_player_light_repeat )},
        { guIMAGE_INDEX_player_light_repeat_single,     GUIMAGE( guImage_player_light_repeat_single )},
        { guIMAGE_INDEX_player_light_search,            GUIMAGE( guImage_player_light_search )},
        { guIMAGE_INDEX_player_light_setup,             GUIMAGE( guImage_player_light_setup )},
        { guIMAGE_INDEX_player_light_smart,             GUIMAGE( guImage_player_light_smart )},
        { guIMAGE_INDEX_player_light_stop,              GUIMAGE( guImage_player_light_stop )},
        { guIMAGE_INDEX_player_light_vol_hi,            GUIMAGE( guImage_player_light_vol_hi )},
        { guIMAGE_INDEX_player_light_vol_low,           GUIMAGE( guImage_player_light_vol_low )},
        { guIMAGE_INDEX_player_light_vol_mid,           GUIMAGE( guImage_player_light_vol_mid )},
        { guIMAGE_INDEX_player_light_love,              GUIMAGE( guImage_player_light_love )},
        { guIMAGE_INDEX_player_light_ban,               GUIMAGE( guImage_player_light_ban )},
        { guIMAGE_INDEX_player_normal_equalizer,        GUIMAGE( guImage_player_normal_equalizer )},
        { guIMAGE_INDEX_player_normal_muted,            GUIMAGE( guImage_player_normal_muted )},
        { guIMAGE_INDEX_player_normal_next,             GUIMAGE( guImage_player_normal_next )},
        { guIMAGE_INDEX_player_normal_pause,            GUIMAGE( guImage_player_normal_pause )},
        { guIMAGE_INDEX_player_normal_play,             GUIMAGE( guImage_player_normal_play )},
        { guIMAGE_INDEX_player_normal_prev,             GUIMAGE( guImage_player_normal_prev )},
        { guIMAGE_INDEX_player_normal_random,           GUIMAGE( guImage_player_normal_random )},
        { guIMAGE_INDEX_player_normal_record,           GUIMAGE( guImage_player_normal_record )},
        { guIMAGE_INDEX_player_normal_repeat,           GUIMAGE( guImage_player_normal_repeat )},
        { guIMAGE_INDEX_player_normal_repeat_single,    GUIMAGE( guImage_player_normal_repeat_single )},
        { guIMAGE_INDEX_player_normal_search,           GUIMAGE( guImage_player_normal_search )},
        { guIMAGE_INDEX_player_normal_setup,            GUIMAGE( guImage_player_normal_setup )},
        { guIMAGE_INDEX_player_normal_smart,            GUIMAGE( guImage_player_normal_smart )},
        { guIMAGE_INDEX_player_normal_stop,             GUIMAGE( guImage_player_normal_stop )},
        { guIMAGE_INDEX_player_normal_vol_hi,           GUIMAGE( guImage_player_normal_vol_hi )},
        { guIMAGE_INDEX_player_normal_vol_low,          GUIMAGE( guImage_player_normal_vol_low )},
        { guIMAGE_INDEX_player_normal_vol_mid,          GUIMAGE( guImage_player_normal_vol_mid )},
        { guIMAGE_INDEX_player_normal_love,             GUIMAGE( guImage_player_normal_love )},
        { guIMAGE_INDEX_player_normal_ban,              GUIMAGE( guImage_player_normal_ban )},
        { guIMAGE_INDEX_player_tiny_light_play,         GUIMAGE( guImage_player_tiny_light_play )},
        { guIMAGE_INDEX_player_tiny_light_stop,         GUIMAGE( guImage_player_tiny_light_stop )},
        { guIMAGE_INDEX_player_tiny_red_stop,           GUIMAGE( guImage_player_tiny_red_stop )},
        { guIMAGE_INDEX_star_normal_tiny,               GUIMAGE( guImage_star_normal_tiny )},
        { guIMAGE_INDEX_star_normal_mid,                GUIMAGE( guImage_star_normal_mid )},
        { guIMAGE_INDEX_star_normal_big,                GUIMAGE( guImage_star_normal_big )},
        { guIMAGE_INDEX_star_highlight_tiny,            GUIMAGE( guImage_star_highlight_tiny )},
        { guIMAGE_INDEX_star_highlight_mid,             GUIMAGE( guImage_star_highlight_mid )},
        { guIMAGE_INDEX_star_highlight_big,             GUIMAGE( guImage_star_highlight_big )},
        //
        { guIMAGE_INDEX_tiny_crossfade,                 GUIMAGE( guImage_tiny_crossfade )},
        { guIMAGE_INDEX_tiny_gapless,                   GUIMAGE( guImage_tiny_gapless )},
        //
        { guIMAGE_INDEX_tiny_mv_library,                GUIMAGE( guImage_tiny_mv_library )},
        { guIMAGE_INDEX_tiny_mv_albumbrowser,           GUIMAGE( guImage_tiny_mv_albumbrowser )},
        { guIMAGE_INDEX_tiny_mv_treeview,               GUIMAGE( guImage_tiny_mv_treeview )},
        { guIMAGE_INDEX_tiny_mv_playlists,              GUIMAGE( guImage_tiny_mv_playlists )}
    };

// -------------------------------------------------------------------------------- //
    wxBitmap GetBitmap( guIMAGE_INDEX imageindex )
    {
        wxBitmap retBM(wxNullBitmap);
        guIMG_ITEM item = guImage_Items[imageindex];
        retBM = wxBitmap::NewFromPNGData( item.IMGdata, item.IMGsize );
        // For wxBITMAP_TYPE_XPM
        // retBM = wxBitmap( item.IMGdata );
        return retBM;
    }

// -------------------------------------------------------------------------------- //
    wxIcon GetIcon( guIMAGE_INDEX imageindex )
    {
        wxIcon retIcon;
        retIcon.CopyFromBitmap( GetBitmap(imageindex) );
        return retIcon;
    }

// -------------------------------------------------------------------------------- //
    wxImage GetImage( guIMAGE_INDEX imageindex )
    {
        wxBitmap getBM = GetBitmap( imageindex );
        return getBM.ConvertToImage();
    }

// -------------------------------------------------------------------------------- //
} //end guNS_Image
// -------------------------------------------------------------------------------- //
