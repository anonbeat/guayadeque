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
#include "./images/guayadeque_png.h"
#include "./images/lastfm_as_off_png.h"
#include "./images/lastfm_as_on_png.h"
#include "./images/lastfm_on.h"
#include "./images/left_png.h"
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
#include "./images/right_png.h"
#include "./images/search_png.h"
#include "./images/skip_backward_png.h"
#include "./images/skip_forward_png.h"
#include "./images/splash_png.h"
#include "./images/system_run_png.h"
#include "./images/tags_png.h"
#include "./images/tiny_accept_png.h"
#include "./images/tiny_add_png.h"
#include "./images/tiny_del_png.h"
#include "./images/tiny_playback_pause_png.h"
#include "./images/tiny_playback_start_png.h"
#include "./images/up_png.h"
#include "./images/volume_high_png.h"
#include "./images/volume_low_png.h"
#include "./images/volume_medium_png.h"
#include "./images/volume_muted_png.h"
#include "./images/grey_star_tiny.h"
#include "./images/grey_star_mid.h"
#include "./images/grey_star_big.h"
#include "./images/yellow_star_tiny.h"
#include "./images/yellow_star_mid.h"
#include "./images/yellow_star_big.h"
#include "./images/track_png.h"
#include "./images/tiny_search.h"
#include "./images/sort_asc.h"
#include "./images/sort_desc.h"

// -------------------------------------------------------------------------------- //
typedef struct {
    const unsigned char * imgdata;
    unsigned int          imgsize;
    long                  imgtype;
} guImage_Item;

// -------------------------------------------------------------------------------- //
guImage_Item guImage_Items[] = {
    { guImage_add,                  sizeof( guImage_add ),                  wxBITMAP_TYPE_PNG },
    { guImage_blank_cd_cover,       sizeof( guImage_blank_cd_cover ),       wxBITMAP_TYPE_PNG },
    { guImage_bookmark,             sizeof( guImage_bookmark ),             wxBITMAP_TYPE_PNG },
    { guImage_default_lastfm_image, sizeof( guImage_default_lastfm_image ), wxBITMAP_TYPE_PNG },
    { guImage_del,                  sizeof( guImage_del ),                  wxBITMAP_TYPE_PNG },
    { guImage_doc_new,              sizeof( guImage_doc_new ),              wxBITMAP_TYPE_PNG },
    { guImage_doc_save,             sizeof( guImage_doc_save ),             wxBITMAP_TYPE_PNG },
    { guImage_download_covers,      sizeof( guImage_download_covers ),      wxBITMAP_TYPE_PNG },
    { guImage_down,                 sizeof( guImage_down ),                 wxBITMAP_TYPE_PNG },
    { guImage_edit_clear,           sizeof( guImage_edit_clear ),           wxBITMAP_TYPE_PNG },
    { guImage_edit_copy,            sizeof( guImage_edit_copy ),            wxBITMAP_TYPE_PNG },
    { guImage_edit_delete,          sizeof( guImage_edit_delete ),          wxBITMAP_TYPE_PNG },
    { guImage_edit,                 sizeof( guImage_edit ),                 wxBITMAP_TYPE_PNG },
    { guImage_exit,                 sizeof( guImage_exit ),                 wxBITMAP_TYPE_PNG },
    { guImage_guayadeque,           sizeof( guImage_guayadeque ),           wxBITMAP_TYPE_PNG },
    { guImage_lastfm_as_off,        sizeof( guImage_lastfm_as_off ),        wxBITMAP_TYPE_PNG },
    { guImage_lastfm_as_on,         sizeof( guImage_lastfm_as_on ),         wxBITMAP_TYPE_PNG },
    { guImage_lastfm_on,            sizeof( guImage_lastfm_on ),            wxBITMAP_TYPE_PNG },
    { guImage_left,                 sizeof( guImage_left ),                 wxBITMAP_TYPE_PNG },
    { guImage_net_radio,            sizeof( guImage_net_radio ),            wxBITMAP_TYPE_JPEG },
    { guImage_no_cover,             sizeof( guImage_no_cover ),             wxBITMAP_TYPE_JPEG },
    { guImage_no_photo,             sizeof( guImage_no_photo ),             wxBITMAP_TYPE_PNG },
    { guImage_numerate,             sizeof( guImage_numerate ),             wxBITMAP_TYPE_PNG },
    { guImage_playback_pause,       sizeof( guImage_playback_pause ),       wxBITMAP_TYPE_PNG },
    { guImage_playback_start,       sizeof( guImage_playback_start ),       wxBITMAP_TYPE_PNG },
    { guImage_playback_stop,        sizeof( guImage_playback_stop ),        wxBITMAP_TYPE_PNG },
    { guImage_playlist_repeat,      sizeof( guImage_playlist_repeat ),      wxBITMAP_TYPE_PNG },
    { guImage_playlist_shuffle,     sizeof( guImage_playlist_shuffle ),     wxBITMAP_TYPE_PNG },
    { guImage_playlist_smart,       sizeof( guImage_playlist_smart ),       wxBITMAP_TYPE_PNG },
    { guImage_right,                sizeof( guImage_right ),                wxBITMAP_TYPE_PNG },
    { guImage_search,               sizeof( guImage_search ),               wxBITMAP_TYPE_PNG },
    { guImage_skip_backward,        sizeof( guImage_skip_backward ),        wxBITMAP_TYPE_PNG },
    { guImage_skip_forward,         sizeof( guImage_skip_forward ),         wxBITMAP_TYPE_PNG },
    { guImage_splash,               sizeof( guImage_splash ),               wxBITMAP_TYPE_PNG },
    { guImage_system_run,           sizeof( guImage_system_run ),           wxBITMAP_TYPE_PNG },
    { guImage_tags,                 sizeof( guImage_tags ),                 wxBITMAP_TYPE_PNG },
    { guImage_tiny_accept,          sizeof( guImage_tiny_accept ),          wxBITMAP_TYPE_PNG },
    { guImage_tiny_add,             sizeof( guImage_tiny_add ),             wxBITMAP_TYPE_PNG },
    { guImage_tiny_del,             sizeof( guImage_tiny_del ),             wxBITMAP_TYPE_PNG },
    { guImage_tiny_playback_pause,  sizeof( guImage_tiny_playback_pause ),  wxBITMAP_TYPE_PNG },
    { guImage_tiny_playback_start,  sizeof( guImage_tiny_playback_start ),  wxBITMAP_TYPE_PNG },
    { guImage_up,                   sizeof( guImage_up ),                   wxBITMAP_TYPE_PNG },
    { guImage_volume_high,          sizeof( guImage_volume_high ),          wxBITMAP_TYPE_PNG },
    { guImage_volume_low,           sizeof( guImage_volume_low ),           wxBITMAP_TYPE_PNG },
    { guImage_volume_medium,        sizeof( guImage_volume_medium ),        wxBITMAP_TYPE_PNG },
    { guImage_volume_muted,         sizeof( guImage_volume_muted ),         wxBITMAP_TYPE_PNG },
    { guImage_grey_star_tiny,       sizeof( guImage_grey_star_tiny ),       wxBITMAP_TYPE_PNG },
    { guImage_grey_star_mid,        sizeof( guImage_grey_star_mid ),        wxBITMAP_TYPE_PNG },
    { guImage_grey_star_big,        sizeof( guImage_grey_star_big ),        wxBITMAP_TYPE_PNG },
    { guImage_yellow_star_tiny,     sizeof( guImage_yellow_star_tiny ),     wxBITMAP_TYPE_PNG },
    { guImage_yellow_star_mid,      sizeof( guImage_yellow_star_mid ),      wxBITMAP_TYPE_PNG },
    { guImage_yellow_star_big,      sizeof( guImage_yellow_star_big ),      wxBITMAP_TYPE_PNG },
    { guImage_track,                sizeof( guImage_track ),                wxBITMAP_TYPE_PNG },
    { guImage_tiny_search,          sizeof( guImage_tiny_search ),          wxBITMAP_TYPE_PNG },
    { guImage_sort_asc,             sizeof( guImage_sort_asc ),             wxBITMAP_TYPE_PNG },
    { guImage_sort_desc,            sizeof( guImage_sort_desc ),            wxBITMAP_TYPE_PNG }
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
