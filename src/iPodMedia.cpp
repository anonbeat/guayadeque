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
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#include "iPodMedia.h"

#include "CoverEdit.h"
#include "MainFrame.h"
#include "MD5.h"
#include "SelCoverFile.h"
#include "TagInfo.h"
#include "Transcode.h"
#include "ShowImage.h"
#include "Utils.h"


#include <wx/tokenzr.h>

namespace Guayadeque {

#ifdef WITH_LIBGPOD_SUPPORT

// -------------------------------------------------------------------------------- //
void inline CheckUpdateField( gchar ** fieldptr, const wxString &newval )
{
    if( wxString( * fieldptr, wxConvUTF8 ) != newval )
    {
        free( * fieldptr );
        * fieldptr = strdup( newval.ToUTF8() );
    }
}

// -------------------------------------------------------------------------------- //
// guIpodLibrary
// -------------------------------------------------------------------------------- //
guIpodLibrary::guIpodLibrary( const wxString &dbpath, guPortableMediaDevice * portablemediadevice, Itdb_iTunesDB * ipoddb )
    : guPortableMediaLibrary( dbpath, portablemediadevice )
{
    m_iPodDb = ipoddb;
}

// -------------------------------------------------------------------------------- //
guIpodLibrary::~guIpodLibrary()
{
    if( m_iPodDb )
    {
        itdb_free( m_iPodDb );
    }
}

// -------------------------------------------------------------------------------- //
int guIpodLibrary::GetAlbumId( const wxString &album, const wxString &artist, const wxString &albumartist, const wxString &disk )
{
    //guLogMessage( wxT( "guIpodLibrary::GetAlbumId" ) );
    wxString query;
    wxSQLite3ResultSet dbRes;
    int RetVal = 0;

    query = wxString::Format( wxT( "SELECT song_albumid FROM songs WHERE song_album = '%s' " ), escape_query_str( album ).c_str() );

    if( !albumartist.IsEmpty() )
    {
        query += wxString::Format( wxT( "AND song_albumartist = '%s' " ), escape_query_str( albumartist ).c_str() );
    }
    else
    {
        query += wxString::Format( wxT( "AND song_artist = '%s' " ), escape_query_str( artist ).c_str() );
    }

    if( !disk.IsEmpty() )
    {
        query += wxString::Format( wxT( "AND song_disk = '%s' " ), escape_query_str( disk ).c_str() );
    }

    query += wxT( "LIMIT 1;" );

    dbRes = m_Db->ExecuteQuery( query );

    if( dbRes.NextRow() )
    {
        RetVal = dbRes.GetInt( 0 );
    }
    else
    {
        dbRes.Finalize();

        query = wxT( "SELECT MAX(song_albumid) FROM songs;" );

        dbRes = ExecuteQuery( query );

        if( dbRes.NextRow() )
        {
          RetVal = dbRes.GetInt( 0 ) + 1;
        }
        else
        {
            RetVal = 1;
        }
    }
    dbRes.Finalize();

    return RetVal;
}

// -------------------------------------------------------------------------------- //
int guIpodLibrary::CreateStaticPlayList( const wxString &name, const wxArrayInt &trackids )
{
    guLogMessage( wxT( "guIpodMediaLibPanel::CreateStaticPlayList( '%s' )" ), name.c_str() );

    wxString PlayListName = name;
    Itdb_Playlist * OldPlayList = itdb_playlist_by_name( m_iPodDb, PlayListName.char_str( wxConvUTF8 ) );
    if( OldPlayList )
    {
        guLogMessage( wxT( "Playlist with same name exists so delete it" ) );
        itdb_playlist_remove( OldPlayList );
    }

    Itdb_Playlist * iPodPlayList = itdb_playlist_new( name.mb_str( wxConvFile ), false );
    if( iPodPlayList )
    {
        guLogMessage( wxT( "Created the playlist" ) );
        itdb_playlist_add( m_iPodDb, iPodPlayList, wxNOT_FOUND );
        guLogMessage( wxT( "Attached to the database" ) );
        guTrackArray Tracks;
        GetSongs( trackids, &Tracks );
        int Index;
        int Count = Tracks.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            guLogMessage( wxT( "Searching for track %s" ), Tracks[ Index ].m_FileName.c_str() );
            Itdb_Track * iPodTrack = iPodFindTrack( Tracks[ Index ].m_FileName );
            if( iPodTrack )
            {
                guLogMessage( wxT( "Adding track %s" ), Tracks[ Index ].m_FileName.c_str() );
                itdb_playlist_add_track( iPodPlayList, iPodTrack, wxNOT_FOUND );
            }
        }
        guLogMessage( wxT( "Savint the database" ) );
        iPodFlush();
    }
    else
    {
        guLogMessage( wxT( "Could not create the playlist " ) );
    }
    return guDbLibrary::CreateStaticPlayList( name, trackids );
}

// -------------------------------------------------------------------------------- //
int guIpodLibrary::UpdateStaticPlayList( const int plid, const wxArrayInt &trackids )
{
    guLogMessage( wxT( "guIpodLibrary::UpdateStaticPlayList" ) );
    wxString PlayListName = GetPlayListName( plid );
    Itdb_Playlist * OldPlayList = itdb_playlist_by_name( m_iPodDb, PlayListName.char_str( wxConvUTF8 ) );
    if( OldPlayList )
    {
        GList * Tracks = OldPlayList->members;
        while( Tracks )
        {
            Itdb_Track * CurTrack = ( Itdb_Track * ) Tracks->data;
            if( CurTrack )
                itdb_playlist_remove_track( OldPlayList, CurTrack );

            Tracks = Tracks->next;
        }

        guTrackArray PlayListTracks;
        GetSongs( trackids, &PlayListTracks );
        int Index;
        int Count = PlayListTracks.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            Itdb_Track * iPodTrack = iPodFindTrack( PlayListTracks[ Index ].m_FileName );
            if( iPodTrack )
            {
                itdb_playlist_add_track( OldPlayList, iPodTrack, wxNOT_FOUND );
            }
        }
        iPodFlush();
    }

    return guDbLibrary::UpdateStaticPlayList( plid, trackids );
}

// -------------------------------------------------------------------------------- //
int guIpodLibrary::AppendStaticPlayList( const int plid, const wxArrayInt &trackids )
{
    guLogMessage( wxT( "guIpodLibrary::AppendStaticPlayList" ) );
    wxString PlayListName = GetPlayListName( plid );
    Itdb_Playlist * OldPlayList = itdb_playlist_by_name( m_iPodDb, PlayListName.char_str( wxConvUTF8 ) );
    if( OldPlayList )
    {
        guTrackArray PlayListTracks;
        GetSongs( trackids, &PlayListTracks );
        int Index;
        int Count = PlayListTracks.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            Itdb_Track * iPodTrack = iPodFindTrack( PlayListTracks[ Index ].m_FileName );
            if( iPodTrack )
            {
                itdb_playlist_add_track( OldPlayList, iPodTrack, wxNOT_FOUND );
            }
        }
        iPodFlush();
    }
    return guDbLibrary::AppendStaticPlayList( plid, trackids );
}

// -------------------------------------------------------------------------------- //
int guIpodLibrary::DelPlaylistSetIds( const int plid, const wxArrayInt &trackids )
{
    guLogMessage( wxT( "guIpodLibrary::DelPlaylistSetIds" ) );
    wxString PlayListName = GetPlayListName( plid );
    Itdb_Playlist * OldPlayList = itdb_playlist_by_name( m_iPodDb, PlayListName.char_str( wxConvUTF8 ) );
    if( OldPlayList )
    {
        guTrackArray PlayListTracks;
        GetSongs( trackids, &PlayListTracks );
        int Index;
        int Count = PlayListTracks.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            Itdb_Track * iPodTrack = iPodFindTrack( PlayListTracks[ Index ].m_FileName );
            if( iPodTrack )
            {
                itdb_playlist_remove_track( OldPlayList, iPodTrack );
            }
        }
        iPodFlush();
    }
    return guDbLibrary::DelPlaylistSetIds( plid, trackids );
}

// -------------------------------------------------------------------------------- //
void guIpodLibrary::SetPlayListName( const int plid, const wxString &newname )
{
    guLogMessage( wxT( "guIpodLibrary::SetPlayListName" ) );
    wxString PlayListName = GetPlayListName( plid );
    Itdb_Playlist * OldPlayList = itdb_playlist_by_name( m_iPodDb, PlayListName.char_str( wxConvUTF8 ) );
    if( OldPlayList )
    {
        CheckUpdateField( &OldPlayList->name, newname );
        iPodFlush();
    }
    else
    {
        guLogMessage( wxT( "Could not find the playlist '%s'" ), PlayListName.c_str() );
    }

    guDbLibrary::SetPlayListName( plid, newname );
}

// -------------------------------------------------------------------------------- //
void guIpodLibrary::DeletePlayList( const int plid )
{
    guLogMessage( wxT( "guIpodLibrary::DeletePlayList" ) );
    wxString PlayListName = GetPlayListName( plid );
    Itdb_Playlist * OldPlayList = itdb_playlist_by_name( m_iPodDb, PlayListName.char_str( wxConvUTF8 ) );
    if( OldPlayList )
    {
        itdb_playlist_remove( OldPlayList );
        iPodFlush();
    }

    guDbLibrary::DeletePlayList( plid );
}

// -------------------------------------------------------------------------------- //
void guIpodLibrary::UpdateStaticPlayListFile( const int plid )
{
}

// -------------------------------------------------------------------------------- //
Itdb_Playlist * guIpodLibrary::CreateiPodPlayList( const wxString &path, const wxArrayString &filenames )
{
    guLogMessage( wxT( "guIpodLibrary::CreateiPodPlayList( '%s' )" ), wxFileNameFromPath( path ).BeforeLast( wxT( '.' ) ).c_str() );

    Itdb_Playlist * iPodPlayList = itdb_playlist_new( wxFileNameFromPath( path ).BeforeLast( wxT( '.' ) ).mb_str( wxConvFile ), false );
    if( iPodPlayList )
    {
        itdb_playlist_add( m_iPodDb, iPodPlayList, wxNOT_FOUND );
        int Index;
        int Count = filenames.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            guLogMessage( wxT( "Trying to search for: '%s'" ), filenames[ Index ].c_str() );
            Itdb_Track * iPodTrack = iPodFindTrack( filenames[ Index ] );
            if( iPodTrack )
            {
                guLogMessage( wxT( "Found the track" ) );
                itdb_playlist_add_track( iPodPlayList, iPodTrack, wxNOT_FOUND );
            }
        }
        guLogMessage( wxT( "Playlist: '%s' with %i tracks" ), wxString( iPodPlayList->name, wxConvUTF8 ).c_str(), iPodPlayList->num );
        iPodFlush();
    }
    else
    {
        guLogMessage( wxT( "Could not create the playlist " ) );
    }
    return iPodPlayList;
}

// -------------------------------------------------------------------------------- //
int guIpodLibrary::CreateDynamicPlayList( const wxString &name, const guDynPlayList * playlist )
{
    guLogMessage( wxT( "CreateDynamicPlaylist..." ) );
    Itdb_Playlist * DynPlayList = itdb_playlist_new( name.mb_str( wxConvFile ), true );
    if( DynPlayList )
    {
        itdb_playlist_add( m_iPodDb, DynPlayList, wxNOT_FOUND );

        if( !playlist->m_Sorted )
        {
            DynPlayList->sortorder = ITDB_PSO_MANUAL;
        }
        else
        {
            switch( playlist->m_SortType )
            {
                case guDYNAMIC_FILTER_ORDER_TITLE :
                    DynPlayList->sortorder = ITDB_PSO_TITLE;
                    break;

                case guDYNAMIC_FILTER_ORDER_ARTIST :
                    DynPlayList->sortorder = ITDB_PSO_ARTIST;
                    break;

                case guDYNAMIC_FILTER_ORDER_ALBUMARTIST :
                    DynPlayList->sortorder = ITDB_PSO_MANUAL;
                    break;

                case guDYNAMIC_FILTER_ORDER_ALBUM :
                    DynPlayList->sortorder = ITDB_PSO_ALBUM;
                    break;

                case guDYNAMIC_FILTER_ORDER_GENRE :
                    DynPlayList->sortorder = ITDB_PSO_GENRE;
                    break;

                case guDYNAMIC_FILTER_ORDER_LABEL :
                    DynPlayList->sortorder = ITDB_PSO_GROUPING;
                    break;

                case guDYNAMIC_FILTER_ORDER_COMPOSER :
                    DynPlayList->sortorder = ITDB_PSO_COMPOSER;
                    break;

                case guDYNAMIC_FILTER_ORDER_YEAR :
                    DynPlayList->sortorder = ITDB_PSO_YEAR;
                    break;

                case guDYNAMIC_FILTER_ORDER_RATING :
                    DynPlayList->sortorder = ITDB_PSO_RATING;
                    break;

                case guDYNAMIC_FILTER_ORDER_LENGTH :
                    DynPlayList->sortorder = ITDB_PSO_TIME;
                    break;

                case guDYNAMIC_FILTER_ORDER_PLAYCOUNT :
                    DynPlayList->sortorder = ITDB_PSO_PLAYCOUNT;
                    break;

                case guDYNAMIC_FILTER_ORDER_LASTPLAY :
                    DynPlayList->sortorder = ITDB_PSO_TIME_PLAYED;
                    break;

                case guDYNAMIC_FILTER_ORDER_ADDEDDATE :
                    DynPlayList->sortorder = ITDB_PSO_TIME_ADDED;
                    break;

                case guDYNAMIC_FILTER_ORDER_RANDOM :
                    DynPlayList->sortorder = ITDB_PSO_MANUAL;
                    break;
            }
        }

        Itdb_SPLPref * PlaylistPref = &DynPlayList->splpref;
        PlaylistPref->liveupdate = true;
        PlaylistPref->checkrules = playlist->m_Filters.Count();
        PlaylistPref->checklimits = playlist->m_Limited;
        if( playlist->m_Limited )
        {
            PlaylistPref->limitvalue = playlist->m_LimitValue;

            switch( playlist->m_LimitType )
            {
                case guDYNAMIC_FILTER_LIMIT_TRACKS :
                    PlaylistPref->limittype = ITDB_LIMITTYPE_SONGS;
                    break;

                case guDYNAMIC_FILTER_LIMIT_MINUTES :
                    PlaylistPref->limittype = ITDB_LIMITTYPE_MINUTES;
                    break;

                case guDYNAMIC_FILTER_LIMIT_MEGABYTES :
                    PlaylistPref->limittype = ITDB_LIMITTYPE_MB;
                    break;

                case guDYNAMIC_FILTER_LIMIT_GIGABYTES :
                    PlaylistPref->limittype = ITDB_LIMITTYPE_GB;
                    break;
            }
        }

        Itdb_SPLRules * PlaylistRules = &DynPlayList->splrules;

        PlaylistRules->match_operator = playlist->m_AnyOption ? ITDB_SPLMATCH_OR : ITDB_SPLMATCH_AND;

        guLogMessage( wxT( "*** Creating new Playlist : '%s'" ), name.c_str() );
        guLogMessage( wxT( "liveupdate  : %i" ), PlaylistPref->liveupdate );
        guLogMessage( wxT( "checkrules  : %i" ), PlaylistPref->checkrules );
        guLogMessage( wxT( "checklimits : %i" ), PlaylistPref->checklimits );
        guLogMessage( wxT( "limittype   : %i" ), PlaylistPref->limittype );

        int Index;
        int Count = playlist->m_Filters.Count();
        for( Index = 0; Index < Count; Index++ )
        {
            Itdb_SPLRule * PlaylistRule;
            if( Index > 0 )
                PlaylistRule = itdb_splr_add_new( DynPlayList, wxNOT_FOUND );
            else
                PlaylistRule = ( Itdb_SPLRule * ) PlaylistRules->rules->data;
            if( PlaylistRule )
            {
                guFilterItem * FilterItem = &playlist->m_Filters[ Index ];

                switch( FilterItem->m_Type )
                {
                    case guDYNAMIC_FILTER_TYPE_TITLE :
                        PlaylistRule->field = ITDB_SPLFIELD_SONG_NAME;
                        break;

                    case guDYNAMIC_FILTER_TYPE_ARTIST :
                        PlaylistRule->field = ITDB_SPLFIELD_ARTIST;
                        break;

                    case guDYNAMIC_FILTER_TYPE_ALBUMARTIST :
                        PlaylistRule->field = ITDB_SPLFIELD_ALBUMARTIST;
                        break;

                    case guDYNAMIC_FILTER_TYPE_ALBUM :
                        PlaylistRule->field = ITDB_SPLFIELD_ALBUM;
                        break;

                    case guDYNAMIC_FILTER_TYPE_GENRE :
                        PlaylistRule->field = ITDB_SPLFIELD_GENRE;
                        break;

                    case guDYNAMIC_FILTER_TYPE_LABEL :
                        PlaylistRule->field = ITDB_SPLFIELD_GROUPING;
                        break;

                    case guDYNAMIC_FILTER_TYPE_COMPOSER :
                        PlaylistRule->field = ITDB_SPLFIELD_COMPOSER;
                        break;

                    case guDYNAMIC_FILTER_TYPE_COMMENT :
                        PlaylistRule->field = ITDB_SPLFIELD_COMMENT;
                        break;

                    case guDYNAMIC_FILTER_TYPE_PATH :
                        break;

                    case guDYNAMIC_FILTER_TYPE_YEAR :
                        PlaylistRule->field = ITDB_SPLFIELD_YEAR;
                        break;

                    case guDYNAMIC_FILTER_TYPE_RATING :
                        PlaylistRule->field = ITDB_SPLFIELD_RATING;
                        break;

                    case guDYNAMIC_FILTER_TYPE_LENGTH :
                        PlaylistRule->field = ITDB_SPLFIELD_TIME;
                        break;

                    case guDYNAMIC_FILTER_TYPE_PLAYCOUNT :
                        PlaylistRule->field = ITDB_SPLFIELD_PLAYCOUNT;
                        break;

                    case guDYNAMIC_FILTER_TYPE_LASTPLAY :
                        PlaylistRule->field = ITDB_SPLFIELD_LAST_PLAYED;
                        break;

                    case guDYNAMIC_FILTER_TYPE_ADDEDDATE :
                        PlaylistRule->field = ITDB_SPLFIELD_DATE_ADDED;
                        break;

                    case guDYNAMIC_FILTER_TYPE_TRACKNUMBER :
                        PlaylistRule->field = ITDB_SPLFIELD_TRACKNUMBER;
                        break;

                    case guDYNAMIC_FILTER_TYPE_BITRATE :
                        PlaylistRule->field = ITDB_SPLFIELD_BITRATE;
                        break;

                    case guDYNAMIC_FILTER_TYPE_SIZE :
                        PlaylistRule->field = ITDB_SPLFIELD_SIZE;
                        break;

                    case guDYNAMIC_FILTER_TYPE_DISK :
                        PlaylistRule->field = ITDB_SPLFIELD_DISC_NUMBER;
                        break;

                    case guDYNAMIC_FILTER_TYPE_HASARTWORK :
                        break;
                }

                switch( FilterItem->m_Type )
                {
                    case guDYNAMIC_FILTER_TYPE_TITLE : // String
                    case guDYNAMIC_FILTER_TYPE_ARTIST :
                    case guDYNAMIC_FILTER_TYPE_ALBUMARTIST :
                    case guDYNAMIC_FILTER_TYPE_ALBUM :
                    case guDYNAMIC_FILTER_TYPE_GENRE :
                    case guDYNAMIC_FILTER_TYPE_COMPOSER :
                    case guDYNAMIC_FILTER_TYPE_COMMENT :
                    case guDYNAMIC_FILTER_TYPE_PATH :
                    case guDYNAMIC_FILTER_TYPE_DISK :
                    case guDYNAMIC_FILTER_TYPE_LABEL :
                    {
                        switch( FilterItem->m_Option )
                        {
                            case guDYNAMIC_FILTER_OPTION_STRING_CONTAINS :
                                PlaylistRule->action = ITDB_SPLACTION_CONTAINS;
                                break;

                            case guDYNAMIC_FILTER_OPTION_STRING_NOT_CONTAINS :
                                PlaylistRule->action = ITDB_SPLACTION_DOES_NOT_CONTAIN;
                                break;

                            case guDYNAMIC_FILTER_OPTION_STRING_IS :
                                PlaylistRule->action = ITDB_SPLACTION_IS_STRING;
                                break;

                            case guDYNAMIC_FILTER_OPTION_STRING_ISNOT :
                                PlaylistRule->action = ITDB_SPLACTION_IS_NOT;
                                break;

                            case guDYNAMIC_FILTER_OPTION_STRING_START_WITH :
                                PlaylistRule->action = ITDB_SPLACTION_STARTS_WITH;
                                break;

                            case guDYNAMIC_FILTER_OPTION_STRING_ENDS_WITH :
                                PlaylistRule->action = ITDB_SPLACTION_ENDS_WITH;
                                break;
                        }
                        PlaylistRule->string = strdup( FilterItem->m_Text.ToUTF8() );
                        PlaylistRule->fromvalue = 0;
                        PlaylistRule->tovalue = 0;
                        PlaylistRule->fromdate = 0;
                        PlaylistRule->todate = 0;
                        PlaylistRule->fromunits = 0;
                        PlaylistRule->tounits = 0;
                        break;
                    }

                    case guDYNAMIC_FILTER_TYPE_YEAR : // Year
                    case guDYNAMIC_FILTER_TYPE_RATING : // Numbers
                    case guDYNAMIC_FILTER_TYPE_PLAYCOUNT :
                    case guDYNAMIC_FILTER_TYPE_TRACKNUMBER :
                    case guDYNAMIC_FILTER_TYPE_BITRATE :
                    case guDYNAMIC_FILTER_TYPE_SIZE :
                    case guDYNAMIC_FILTER_TYPE_LENGTH : // Time
                    {
                        switch( FilterItem->m_Option )
                        {
                            case guDYNAMIC_FILTER_OPTION_NUMERIC_IS :
                                PlaylistRule->action = ITDB_SPLACTION_IS_INT;
                                break;

                            case guDYNAMIC_FILTER_OPTION_NUMERIC_ISNOT :
                                PlaylistRule->action = ITDB_SPLACTION_IS_NOT_INT;
                                break;

                            case guDYNAMIC_FILTER_OPTION_NUMERIC_AT_LEAST :
                                PlaylistRule->action = ITDB_SPLACTION_IS_GREATER_THAN;
                                break;

                            case guDYNAMIC_FILTER_OPTION_NUMERIC_AT_MOST :
                                PlaylistRule->action = ITDB_SPLACTION_IS_LESS_THAN;
                                break;
                        }
                        PlaylistRule->fromvalue = FilterItem->m_Number;
                        PlaylistRule->tovalue = FilterItem->m_Number;
                        PlaylistRule->fromdate = 0;
                        PlaylistRule->todate = 0;
                        PlaylistRule->fromunits = 1;
                        PlaylistRule->tounits = 1;
                        break;
                    }

                    case guDYNAMIC_FILTER_TYPE_LASTPLAY :
                    case guDYNAMIC_FILTER_TYPE_ADDEDDATE :
                    {
                        switch( FilterItem->m_Option )
                        {
                            case guDYNAMIC_FILTER_OPTION_DATE_IN_THE_LAST :
                                PlaylistRule->action = ITDB_SPLACTION_IS_IN_THE_LAST;
                                break;

                            case guDYNAMIC_FILTER_OPTION_DATE_BEFORE_THE_LAST :
                                PlaylistRule->action = ITDB_SPLACTION_IS_NOT_IN_THE_LAST;
                                break;
                        }
                        PlaylistRule->fromvalue = ITDB_SPL_DATE_IDENTIFIER;
                        PlaylistRule->tovalue   = ITDB_SPL_DATE_IDENTIFIER;
                        PlaylistRule->fromdate  = FilterItem->m_Number * -1;
                        PlaylistRule->todate    = 0;
                        PlaylistRule->fromunits = DynPLDateOption2[ FilterItem->m_Option2 ];
                        PlaylistRule->tounits   = 1;
                        break;
                    }

                    //case guDYNAMIC_FILTER_TYPE_HASARTWORK :
                }

                guLogMessage( wxT( "field   : %i" ), PlaylistRule->field );
                guLogMessage( wxT( "action  : %08X" ), PlaylistRule->action );
                if( PlaylistRule->string )
                    guLogMessage( wxT( "String  : '%s'" ), wxString( PlaylistRule->string, wxConvUTF8 ).c_str() );
                guLogMessage( wxT( "Values  : %016llX %016llX" ), PlaylistRule->fromvalue, PlaylistRule->tovalue );
                guLogMessage( wxT( "Dates   : %016llX %016llX" ), PlaylistRule->fromdate, PlaylistRule->todate);
                guLogMessage( wxT( "Units   : %016llX %016llX" ), PlaylistRule->fromunits, PlaylistRule->tounits);

                itdb_splr_validate( PlaylistRule );
                guLogMessage( wxT( "Validated the Rule" ) );
            }
        }

        itdb_spl_update( DynPlayList );

        iPodFlush();
    }
    else
    {
        guLogMessage( wxT( "Could not create the playlist " ) );
    }

    return guDbLibrary::CreateDynamicPlayList( name, playlist );
}

// -------------------------------------------------------------------------------- //
void guIpodLibrary::UpdateDynamicPlayList( const int plid, const guDynPlayList * playlist )
{
    wxString PlayListName = GetPlayListName( plid );
    DeletePlayList( plid );

    CreateDynamicPlayList( PlayListName, playlist );
}

// -------------------------------------------------------------------------------- //
Itdb_Track * guIpodLibrary::iPodFindTrack( const wxString &filename )
{
    Itdb_Track * Track = NULL;
    wxString FileName = filename;

    if( FileName.StartsWith( m_PortableDevice->MountPath() ) )
    {
        FileName = FileName.Mid( m_PortableDevice->MountPath().Length() - 1 );
    }
    FileName.Replace( wxT( "/" ), wxT( ":" ) );
    //guLogMessage( wxT( "Trying to find %s" ), FileName.c_str() );

    GList * Tracks = m_iPodDb->tracks;
    while( Tracks )
    {
        Itdb_Track * CurTrack = ( Itdb_Track * ) Tracks->data;
        //guLogMessage( wxT( "Checking: '%s" ), wxString( CurTrack->ipod_path, wxConvUTF8 ).c_str() );
        if( wxString( CurTrack->ipod_path, wxConvUTF8 ) == FileName )
        {
            Track = CurTrack;
            break;
        }
        Tracks = Tracks->next;
    }
    return Track;
}

// -------------------------------------------------------------------------------- //
Itdb_Track * guIpodLibrary::iPodFindTrack( const wxString &artist, const wxString &albumartist, const wxString &album, const wxString &title )
{
    Itdb_Track * Track = NULL;
    GList * Tracks = m_iPodDb->tracks;
    while( Tracks )
    {
        Itdb_Track * CurTrack = ( Itdb_Track * ) Tracks->data;
        if( ( albumartist.IsEmpty() ? ( wxString( CurTrack->artist, wxConvUTF8 ) == artist ) :
                                    ( wxString( CurTrack->albumartist, wxConvUTF8 ) == albumartist ) ) &&
            ( wxString( CurTrack->album, wxConvUTF8 ) == album ) &&
            ( wxString( CurTrack->title, wxConvUTF8 ) == title ) )
        {
            Track = CurTrack;
            break;
        }
        Tracks = Tracks->next;
    }
    return Track;
}

// -------------------------------------------------------------------------------- //
void guIpodLibrary::iPodRemoveTrack( Itdb_Track * track )
{
    if( track )
    {
        GList * Playlists = m_iPodDb->playlists;
        while( Playlists )
        {
            Itdb_Playlist * CurPlaylist = ( Itdb_Playlist * ) Playlists->data;
            if( itdb_playlist_contains_track( CurPlaylist, track ) )
            {
                itdb_playlist_remove_track( CurPlaylist, track );
            }
            Playlists = Playlists->next;
        }

        itdb_track_remove_thumbnails( track );

        itdb_track_remove( track );
    }
}

// -------------------------------------------------------------------------------- //
void guIpodLibrary::iPodRemoveTrack( const wxString &filename )
{
    Itdb_Track * Track = iPodFindTrack( filename );

    iPodRemoveTrack( Track );

    if( !wxRemoveFile( filename ) )
        guLogMessage( wxT( "Couldnt remove the file '%s'" ), filename.c_str() );
}

// -------------------------------------------------------------------------------- //
int guIpodLibrary::UpdateSong( const guTrack &track, const bool allowrating )
{
    wxString query = wxString::Format( wxT( "UPDATE songs SET song_name = '%s', " \
            "song_genreid = %u, song_genre = '%s', " \
            "song_artistid = %u, song_artist = '%s', " \
            "song_albumartistid = %u, song_albumartist = '%s', " \
            "song_albumid = %u, song_album = '%s', " \
            "song_pathid = %u, song_path = '%s', " \
            "song_filename = '%s', song_format = '%s', " \
            "song_number = %u, song_year = %u, " \
            "song_composerid = %u, song_composer = '%s', " \
            "song_comment = '%s', song_coverid = %i, song_disk = '%s', " \
            "song_length = %u, song_offset = %u, song_bitrate = %u, " \
            "song_rating = %i, song_filesize = %u, " \
            "song_lastplay = %u, song_addedtime = %u, " \
            "song_playcount = %u " \
            "WHERE song_id = %u;" ),
            escape_query_str( track.m_SongName ).c_str(),
            track.m_GenreId,
            escape_query_str( track.m_GenreName ).c_str(),
            track.m_ArtistId,
            escape_query_str( track.m_ArtistName ).c_str(),
            track.m_AlbumArtistId,
            escape_query_str( track.m_AlbumArtist ).c_str(),
            track.m_AlbumId,
            escape_query_str( track.m_AlbumName ).c_str(),
            track.m_PathId,
            escape_query_str( track.m_Path ).c_str(),
            escape_query_str( track.m_FileName ).c_str(),
            escape_query_str( track.m_FileName.AfterLast( '.' ) ).c_str(),
            track.m_Number,
            track.m_Year,
            track.m_ComposerId, //escape_query_str( track.m_Composer ).c_str(),
            escape_query_str( track.m_Composer ).c_str(),
            escape_query_str( track.m_Comments ).c_str(),
            track.m_CoverId,
            escape_query_str( track.m_Disk ).c_str(),
            track.m_Length,
            0, //track.m_Offset,
            track.m_Bitrate,
            track.m_Rating,
            track.m_FileSize,
            track.m_LastPlay,
            track.m_AddedTime,
            track.m_PlayCount,
            track.m_SongId );

  //guLogMessage( wxT( "%s" ), query.c_str() );
  return ExecuteUpdate( query );
}


// -------------------------------------------------------------------------------- //
// guIpodLibraryUpdate
// -------------------------------------------------------------------------------- //
guIpodLibraryUpdate::guIpodLibraryUpdate( guMediaVieweriPodDevice * mediaviewer, const int gaugeid )
{
    m_MediaViewer = mediaviewer;
    m_GaugeId = gaugeid;

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guIpodLibraryUpdate::~guIpodLibraryUpdate()
{
    if( !TestDestroy() )
    {
        m_MediaViewer->UpdateFinished();
    }

    wxCommandEvent GaugeEvent( wxEVT_MENU, ID_STATUSBAR_GAUGE_REMOVE );
    GaugeEvent.SetInt( m_GaugeId );
    wxPostEvent( wxTheApp->GetTopWindow(), GaugeEvent );
}

extern "C" {
extern gboolean gdk_pixbuf_save_to_buffer( GdkPixbuf *pixbuf,
                                           gchar **buffer,
                                           gsize *buffer_size,
                                           const char *type,
                                           GError **error,
                                           ... );
}

// -------------------------------------------------------------------------------- //
guIpodLibraryUpdate::ExitCode guIpodLibraryUpdate::Entry( void )
{
    wxArrayPtrVoid CoveriPodTracks;
    wxArrayInt     CoverAlbumIds;
    guIpodLibrary * Db = ( guIpodLibrary * ) m_MediaViewer->GetDb();
    Itdb_iTunesDB * iPodDb = Db->GetiPodDb();
    if( iPodDb )
    {
        //Db->ExecuteUpdate( wxT( "DELETE FROM songs" ) );
        guPortableMediaDevice * PortableMediaDevice = m_MediaViewer->GetPortableMediaDevice();

        guMainFrame * MainFrame = ( guMainFrame * ) wxTheApp->GetTopWindow();

        wxCommandEvent GaugeEvent( wxEVT_MENU, ID_STATUSBAR_GAUGE_SETMAX );
        GaugeEvent.SetInt( m_GaugeId );
        GaugeEvent.SetExtraLong( itdb_tracks_number( iPodDb ) );
        wxPostEvent( MainFrame, GaugeEvent );

        GaugeEvent.SetId( ID_STATUSBAR_GAUGE_UPDATE );

        int TrackIndex = 0;

        // Add any missing track to the database or update the existing ones
        GList * Tracks = iPodDb->tracks;
        while( !TestDestroy() && Tracks )
        {
            Itdb_Track * iPodTrack = ( Itdb_Track * ) Tracks->data;
            if( iPodTrack )
            {
                guTrack Track;
                Track.m_SongName = wxString( iPodTrack->title, wxConvUTF8 );
                Track.m_ArtistName = wxString( iPodTrack->artist, wxConvUTF8 );
                Track.m_ArtistId = Db->GetArtistId( Track.m_ArtistName );
                Track.m_GenreName = wxString( iPodTrack->genre, wxConvUTF8 );
                Track.m_GenreId = Db->GetGenreId( Track.m_GenreName );
                Track.m_Composer = wxString( iPodTrack->composer, wxConvUTF8 );
                Track.m_ComposerId = Db->GetComposerId( Track.m_Composer );
                Track.m_Comments = wxString( iPodTrack->comment, wxConvUTF8 );
                Track.m_AlbumArtist = wxString( iPodTrack->albumartist, wxConvUTF8 );
                Track.m_AlbumArtistId = Db->GetAlbumArtistId( Track.m_AlbumArtist );
                Track.m_AlbumName = wxString( iPodTrack->album, wxConvUTF8 );
                Track.m_FileSize = iPodTrack->size;
                Track.m_Length = iPodTrack->tracklen;
                if( iPodTrack->cd_nr || iPodTrack->cds )
                {
                    Track.m_Disk = wxString::Format( wxT( "%u/%u" ), iPodTrack->cd_nr, iPodTrack->cds );
                }
                Track.m_Number = iPodTrack->track_nr;
                Track.m_Bitrate = iPodTrack->bitrate;
                Track.m_Year = iPodTrack->year;
                Track.m_Rating = iPodTrack->rating / ITDB_RATING_STEP;
                Track.m_PlayCount = iPodTrack->playcount;
                Track.m_AddedTime = iPodTrack->time_added;
                Track.m_LastPlay = iPodTrack->time_played;

                Track.m_Path = PortableMediaDevice->MountPath() + wxString( iPodTrack->ipod_path, wxConvUTF8 ).Mid( 1 );
                Track.m_Path.Replace( wxT( ":" ), wxT( "/" ) );
                Track.m_FileName = Track.m_Path.AfterLast( wxT( '/' ) );
                Track.m_Path = wxPathOnly( Track.m_Path ) + wxT( "/" );

                Track.m_PathId = Db->GetPathId( Track.m_Path );

                Track.m_SongId = Db->GetSongId( Track.m_FileName, Track.m_PathId, iPodTrack->time_added );

                Track.m_AlbumId = Db->GetAlbumId( Track.m_AlbumName, Track.m_ArtistName, Track.m_AlbumArtist, Track.m_Disk );

                Db->UpdateSong( Track, true );

                if( itdb_track_has_thumbnails( iPodTrack ) )
                {
                    if( CoverAlbumIds.Index( Track.m_AlbumId ) == wxNOT_FOUND )
                    {
                        CoveriPodTracks.Add( iPodTrack );
                        CoverAlbumIds.Add( Track.m_AlbumId );
                    }
                }
            }
            Tracks = Tracks->next;

            //
            TrackIndex++;
            GaugeEvent.SetExtraLong( TrackIndex );
            wxPostEvent( MainFrame, GaugeEvent );
        }

        // Find all tracks that have been removed
        wxString FileName;
        wxArrayInt SongsToDel;
        wxSQLite3ResultSet dbRes;

        dbRes = Db->ExecuteQuery( wxT( "SELECT DISTINCT song_id, song_filename, song_path FROM songs " ) );

        while( !TestDestroy() && dbRes.NextRow() )
        {
            FileName = dbRes.GetString( 2 ) + dbRes.GetString( 1 );

            if( !wxFileExists( FileName ) )
            {
                SongsToDel.Add( dbRes.GetInt( 0 ) );
            }
        }
        dbRes.Finalize();

        // Add the covers to the albums
        int Index;
        int Count;
        if( ( Count = CoveriPodTracks.Count() ) )
        {
            for( Index = 0; Index < Count; Index++ )
            {
                if( TestDestroy() )
                    break;

                Itdb_Track * iPodTrack = ( Itdb_Track * ) CoveriPodTracks[ Index ];
                int AlbumId = CoverAlbumIds[ Index ];

                GdkPixbuf * Pixbuf = ( GdkPixbuf * ) itdb_track_get_thumbnail( iPodTrack, -1, -1 );
                if( Pixbuf )
                {
                    void * Buffer = NULL;
                    gsize BufferSize = 0;
                    if( gdk_pixbuf_save_to_buffer( Pixbuf, ( gchar ** ) &Buffer, &BufferSize, "jpeg", NULL, "quality", "100", NULL ) )
                    {
                        wxMemoryInputStream ImageStream( Buffer, BufferSize );
                        wxImage CoverImage( ImageStream, wxBITMAP_TYPE_JPEG );
                        if( CoverImage.IsOk() )
                        {
                            Db->SetAlbumCover( AlbumId, CoverImage );
                        }
                        else
                        {
                            guLogMessage( wxT( "Error in image from ipod..." ) );
                        }
                        g_free( Buffer );
                    }
                    else
                    {
                        guLogMessage( wxT( "Couldnt save to a buffer the pixbuf for album %i" ), AlbumId );
                    }
                    g_object_unref( Pixbuf );
                }
                else
                {
                    guLogMessage( wxT( "Couldnt get the pixbuf for album %i" ), AlbumId );
                }
            }
        }

        // Delete all playlists
        Db->ExecuteUpdate( wxT( "DELETE FROM playlists;" ) );
        Db->ExecuteUpdate( wxT( "DELETE FROM plsets;" ) );

        GList * Playlists = iPodDb->playlists;
        while( !TestDestroy() && Playlists )
        {
            Itdb_Playlist * Playlist = ( Itdb_Playlist * ) Playlists->data;
            if( Playlist && !Playlist->podcastflag && !Playlist->type )
            {
                wxString PlaylistName = wxString( Playlist->name, wxConvUTF8 );
                guLogMessage( wxT( "Playlist: '%s'" ), PlaylistName.c_str() );

                if( !Playlist->is_spl ) // Its not a smart playlist
                {
                    wxArrayInt PlaylistTrackIds;
                    Tracks = Playlist->members;
                    while( Tracks )
                    {
                        Itdb_Track * PlaylistTrack = ( Itdb_Track * ) Tracks->data;
                        if( PlaylistTrack )
                        {
                            wxString Path = PortableMediaDevice->MountPath() + wxString( PlaylistTrack->ipod_path, wxConvUTF8 ).Mid( 1 );
                            Path.Replace( wxT( ":" ), wxT( "/" ) );
                            wxString FileName = Path.AfterLast( wxT( '/' ) );
                            Path = wxPathOnly( Path ) + wxT( "/" );

                            int PathId = Db->GetPathId( Path );

                            int TrackId = Db->GetSongId( FileName, PathId, PlaylistTrack->time_added );
                            if( TrackId )
                            {
                                PlaylistTrackIds.Add( TrackId );
                            }
                        }
                        Tracks = Tracks->next;
                    }

                    Db->CreateStaticPlayList( PlaylistName, PlaylistTrackIds, true );
                }
                else                // Its a dynamic playlist
                {
                    guDynPlayList NewPlayList;
                    bool ErrorInPlaylist = false;

                    guLogMessage( wxT( "Found a dynamic playlist '%s' with %i tracks" ), PlaylistName.c_str(), Playlist->num );
                    guLogMessage( wxT( "id          : %i" ), Playlist->id );
                    guLogMessage( wxT( "sortorder   : %i" ), Playlist->sortorder );

                    NewPlayList.m_Sorted = ( Playlist->sortorder != ITDB_PSO_MANUAL );


                    switch( Playlist->sortorder )
                    {
                        case ITDB_PSO_MANUAL :
                            break;

//                        case ITDB_PSO_BIRATE :
//                        case ITDB_PSO_FILETYPE :
//                        case ITDB_PSO_TRACK_NR :
//                        case ITDB_PSO_SIZE :
//                        case ITDB_PSO_SAMPLERATE :
//                        case ITDB_PSO_COMMENT :
//                        case ITDB_PSO_EQUALIZER :
//                        case ITDB_PSO_CD_NR :
//                        case ITDB_PSO_BPM :
                        case ITDB_PSO_GROUPING :
                            NewPlayList.m_SortType = guDYNAMIC_FILTER_ORDER_LABEL;
                            break;

//                        case ITDB_PSO_CATEGORY :
//                        case ITDB_PSO_DESCRIPTION :
//                            break;

                        case ITDB_PSO_TITLE :
                            NewPlayList.m_SortType = guDYNAMIC_FILTER_ORDER_TITLE;
                            break;

                        case ITDB_PSO_ALBUM :
                            NewPlayList.m_SortType = guDYNAMIC_FILTER_ORDER_ALBUM;
                            break;

                        case ITDB_PSO_ARTIST :
                            NewPlayList.m_SortType = guDYNAMIC_FILTER_ORDER_ARTIST;
                            break;

                        case ITDB_PSO_GENRE :
                            NewPlayList.m_SortType = guDYNAMIC_FILTER_ORDER_GENRE;
                            break;

                        case ITDB_PSO_TIME_MODIFIED :
                            NewPlayList.m_SortType = guDYNAMIC_FILTER_ORDER_GENRE;
                            break;

                        case ITDB_PSO_TIME :
                            NewPlayList.m_SortType = guDYNAMIC_FILTER_ORDER_LENGTH;
                            break;

                        case ITDB_PSO_YEAR :
                            NewPlayList.m_SortType = guDYNAMIC_FILTER_ORDER_YEAR;
                            break;

                        case ITDB_PSO_TIME_ADDED :
                            NewPlayList.m_SortType = guDYNAMIC_FILTER_ORDER_ADDEDDATE;
                            break;

                        case ITDB_PSO_COMPOSER :
                            NewPlayList.m_SortType = guDYNAMIC_FILTER_ORDER_COMPOSER;
                            break;

                        case ITDB_PSO_PLAYCOUNT :
                            NewPlayList.m_SortType = guDYNAMIC_FILTER_ORDER_PLAYCOUNT;
                            break;

                        case ITDB_PSO_TIME_PLAYED :
                            NewPlayList.m_SortType = guDYNAMIC_FILTER_ORDER_LASTPLAY;
                            break;

                        case ITDB_PSO_RATING :
                            NewPlayList.m_SortType = guDYNAMIC_FILTER_ORDER_RATING;
                            break;

                        case ITDB_PSO_RELEASE_DATE :
                            NewPlayList.m_SortType = guDYNAMIC_FILTER_ORDER_YEAR;
                            break;

                        default :
                            ErrorInPlaylist = true;
                            guLogMessage( wxT( "Sorting not supported %i" ), Playlist->sortorder );
                            break;
                    }

                    Itdb_SPLPref * PlaylistPref = &Playlist->splpref;
                    guLogMessage( wxT( "liveupdate  : %i" ), PlaylistPref->liveupdate );
                    guLogMessage( wxT( "checkrules  : %i" ), PlaylistPref->checkrules );
                    guLogMessage( wxT( "checklimits : %i" ), PlaylistPref->checklimits );
                    guLogMessage( wxT( "limittype   : %i" ), PlaylistPref->limittype );
                    if( PlaylistPref->checklimits )
                    {
                        NewPlayList.m_Limited = true;
                        NewPlayList.m_LimitValue = PlaylistPref->limitvalue;

                        switch( PlaylistPref->limittype )
                        {
                            case ITDB_LIMITTYPE_MINUTES :
                                NewPlayList.m_LimitType = guDYNAMIC_FILTER_LIMIT_MINUTES;
                                break;

                            case ITDB_LIMITTYPE_MB      :
                                NewPlayList.m_LimitType = guDYNAMIC_FILTER_LIMIT_MEGABYTES;
                                break;

                            case ITDB_LIMITTYPE_SONGS   :
                                NewPlayList.m_LimitType = guDYNAMIC_FILTER_LIMIT_TRACKS;
                                break;

                            case ITDB_LIMITTYPE_HOURS   :
                                NewPlayList.m_LimitType = guDYNAMIC_FILTER_LIMIT_MINUTES;
                                NewPlayList.m_LimitValue *= 60;
                                break;

                            case ITDB_LIMITTYPE_GB      :
                                NewPlayList.m_LimitType = guDYNAMIC_FILTER_LIMIT_GIGABYTES;
                                break;
                        }
                    }

                    Itdb_SPLRules * PlaylistRules = &Playlist->splrules;
                    guLogMessage( wxT( "operator    : %i" ), PlaylistRules->match_operator );
                    if( PlaylistRules->match_operator == ITDB_SPLMATCH_OR )
                        NewPlayList.m_AnyOption = true;

                    GList * Rules = PlaylistRules->rules;
                    int RuleCount = 0;
                    while( !ErrorInPlaylist && Rules && ( RuleCount < PlaylistPref->checkrules ) )
                    {
                        guLogMessage( wxT( "###===---Rule---===###" ) );
                        Itdb_SPLRule * PlaylistRule = ( Itdb_SPLRule * ) Rules->data;
                        if( PlaylistRule )
                        {
                            guFilterItem * FilterItem = new guFilterItem();
                            guLogMessage( wxT( "field       : %i" ), PlaylistRule->field );
                            guLogMessage( wxT( "action      : %08X" ), PlaylistRule->action );
                            if( PlaylistRule->string )
                                guLogMessage( wxT( "String  : '%s'" ), wxString( PlaylistRule->string, wxConvUTF8 ).c_str() );
                            guLogMessage( wxT( "Values  : %016llX %016llX" ), PlaylistRule->fromvalue, PlaylistRule->tovalue );
                            guLogMessage( wxT( "Dates   : %016llX %016llX" ), PlaylistRule->fromdate, PlaylistRule->todate);
                            guLogMessage( wxT( "Units   : %016llX %016llX" ), PlaylistRule->fromunits, PlaylistRule->tounits);

                            switch( PlaylistRule->field )
                            {
                                case ITDB_SPLFIELD_SONG_NAME :
                                    FilterItem->m_Type = guDYNAMIC_FILTER_TYPE_TITLE;
                                    break;

                                case ITDB_SPLFIELD_ALBUM :
                                    FilterItem->m_Type = guDYNAMIC_FILTER_TYPE_ALBUM;
                                    break;

                                case ITDB_SPLFIELD_ARTIST :
                                    FilterItem->m_Type = guDYNAMIC_FILTER_TYPE_ARTIST;
                                    break;

                                case ITDB_SPLFIELD_BITRATE :
                                    FilterItem->m_Type = guDYNAMIC_FILTER_TYPE_BITRATE;
                                    break;

                                case ITDB_SPLFIELD_YEAR :
                                    FilterItem->m_Type = guDYNAMIC_FILTER_TYPE_YEAR;
                                    break;

                                case ITDB_SPLFIELD_GENRE :
                                    FilterItem->m_Type = guDYNAMIC_FILTER_TYPE_GENRE;
                                    break;

                                case ITDB_SPLFIELD_TRACKNUMBER :
                                    FilterItem->m_Type = guDYNAMIC_FILTER_TYPE_TRACKNUMBER;
                                    break;

                                case ITDB_SPLFIELD_SIZE :
                                    FilterItem->m_Type = guDYNAMIC_FILTER_TYPE_SIZE;
                                    break;

                                case ITDB_SPLFIELD_TIME :
                                    FilterItem->m_Type = guDYNAMIC_FILTER_TYPE_LENGTH;
                                    break;

                                case ITDB_SPLFIELD_COMMENT :
                                    FilterItem->m_Type = guDYNAMIC_FILTER_TYPE_COMMENT;
                                    break;

                                case ITDB_SPLFIELD_DATE_ADDED :
                                    FilterItem->m_Type = guDYNAMIC_FILTER_TYPE_ADDEDDATE;
                                    break;

                                case ITDB_SPLFIELD_COMPOSER :
                                    FilterItem->m_Type = guDYNAMIC_FILTER_TYPE_COMPOSER;
                                    break;

                                case ITDB_SPLFIELD_PLAYCOUNT :
                                    FilterItem->m_Type = guDYNAMIC_FILTER_TYPE_PLAYCOUNT;
                                    break;

                                case ITDB_SPLFIELD_LAST_PLAYED :
                                    FilterItem->m_Type = guDYNAMIC_FILTER_TYPE_LASTPLAY;
                                    break;

                                case ITDB_SPLFIELD_DISC_NUMBER :
                                    FilterItem->m_Type = guDYNAMIC_FILTER_TYPE_DISK;
                                    break;

                                case ITDB_SPLFIELD_RATING :
                                    FilterItem->m_Type = guDYNAMIC_FILTER_TYPE_RATING;
                                    break;

                                case ITDB_SPLFIELD_ALBUMARTIST :
                                    FilterItem->m_Type = guDYNAMIC_FILTER_TYPE_ALBUMARTIST;
                                    break;

                                default :
                                    ErrorInPlaylist = true;
                                    break;
                            }

                            switch( PlaylistRule->action )
                            {
                                case ITDB_SPLACTION_IS_INT :
                                    FilterItem->m_Option = guDYNAMIC_FILTER_OPTION_NUMERIC_IS;
                                    FilterItem->m_Number = PlaylistRule->fromvalue;
                                    break;

                                case ITDB_SPLACTION_IS_GREATER_THAN :
                                    FilterItem->m_Option = guDYNAMIC_FILTER_OPTION_NUMERIC_AT_LEAST;
                                    FilterItem->m_Number = PlaylistRule->fromvalue;
                                    break;

                                case ITDB_SPLACTION_IS_LESS_THAN :
                                    FilterItem->m_Option = guDYNAMIC_FILTER_OPTION_NUMERIC_AT_MOST;
                                    FilterItem->m_Number = PlaylistRule->fromvalue;
                                    break;

                                //case ITDB_SPLACTION_IS_IN_THE_RANGE :

                                //case ITDB_SPLACTION_IS_IN_THE_LAST :
                                //    break;

                                //case ITDB_SPLACTION_BINARY_AND :

                                case ITDB_SPLACTION_IS_STRING :
                                    FilterItem->m_Option = guDYNAMIC_FILTER_OPTION_STRING_IS;
                                    FilterItem->m_Text = wxString( PlaylistRule->string, wxConvUTF8 );
                                    break;

                                case ITDB_SPLACTION_CONTAINS :
                                    FilterItem->m_Option = guDYNAMIC_FILTER_OPTION_STRING_CONTAINS;
                                    FilterItem->m_Text = wxString( PlaylistRule->string, wxConvUTF8 );
                                    break;

                                case ITDB_SPLACTION_STARTS_WITH :
                                    FilterItem->m_Option = guDYNAMIC_FILTER_OPTION_STRING_START_WITH;
                                    FilterItem->m_Text = wxString( PlaylistRule->string, wxConvUTF8 );
                                    break;

                                case ITDB_SPLACTION_ENDS_WITH :
                                    FilterItem->m_Option = guDYNAMIC_FILTER_OPTION_STRING_ENDS_WITH;
                                    FilterItem->m_Text = wxString( PlaylistRule->string, wxConvUTF8 );
                                    break;

                                case ITDB_SPLACTION_IS_NOT_INT :
                                    FilterItem->m_Option = guDYNAMIC_FILTER_OPTION_NUMERIC_ISNOT;
                                    FilterItem->m_Number = PlaylistRule->fromvalue;
                                    break;

                                case ITDB_SPLACTION_IS_NOT_GREATER_THAN :
                                    FilterItem->m_Option = guDYNAMIC_FILTER_OPTION_NUMERIC_AT_MOST;
                                    FilterItem->m_Number = PlaylistRule->fromvalue;
                                    break;

                                case ITDB_SPLACTION_IS_NOT_LESS_THAN :
                                    FilterItem->m_Option = guDYNAMIC_FILTER_OPTION_NUMERIC_AT_LEAST;
                                    FilterItem->m_Number = PlaylistRule->fromvalue;
                                    break;

                                //case ITDB_SPLACTION_IS_NOT_IN_THE_RANGE :
                                //case ITDB_SPLACTION_IS_NOT_IN_THE_LAST :

                                case ITDB_SPLACTION_IS_NOT :
                                    FilterItem->m_Option = guDYNAMIC_FILTER_OPTION_STRING_ISNOT;
                                    FilterItem->m_Text = wxString( PlaylistRule->string, wxConvUTF8 );
                                    break;

                                case ITDB_SPLACTION_DOES_NOT_CONTAIN :
                                    FilterItem->m_Option = guDYNAMIC_FILTER_OPTION_STRING_NOT_CONTAINS;
                                    FilterItem->m_Text = wxString( PlaylistRule->string, wxConvUTF8 );
                                    break;

                                //case ITDB_SPLACTION_DOES_NOT_START_WITH :
                                //case ITDB_SPLACTION_DOES_NOT_END_WITH :

                                default :
                                    ErrorInPlaylist = true;
                                    break;
                            }

                            if( !ErrorInPlaylist )
                            {
                                NewPlayList.m_Filters.Add( FilterItem );
                            }
                            else
                            {
                                guLogMessage( wxT( "The playlist have been discarded by rule %08X %08X" ), PlaylistRule->field, PlaylistRule->action );
                                guLogMessage( wxT( "Value: %016llX %016llX" ), PlaylistRule->fromvalue, PlaylistRule->tovalue );
                                guLogMessage( wxT( "Dates: %016llX %016llX" ), PlaylistRule->fromdate, PlaylistRule->todate);
                                guLogMessage( wxT( "Units: %016llX %016llX" ), PlaylistRule->fromunits, PlaylistRule->tounits);
                                delete FilterItem;
                            }
                        }

                        Rules = Rules->next;
                        RuleCount++;
                    }

                    if( !ErrorInPlaylist )
                    {
                        // Save the dynamic playlist now
                        Db->CreateDynamicPlayList( PlaylistName, &NewPlayList, true );
                    }

                }
            }
            else
            {
                guLogMessage( wxT( "Playlist was podcast or master" ) );
            }

            Playlists = Playlists->next;
        }

        wxCommandEvent evt( wxEVT_MENU, ID_PLAYLIST_UPDATED );
        //evt.SetClientData( ( void * ) m_iPodPanel );
        wxPostEvent( MainFrame, evt );

        // Clean all the deleted items
        if( !TestDestroy() )
        {
            wxArrayString Queries;

            if( SongsToDel.Count() )
            {
                Queries.Add( wxT( "DELETE FROM songs WHERE " ) + ArrayToFilter( SongsToDel, wxT( "song_id" ) ) );
            }

            // Delete all posible orphan entries
            Queries.Add( wxT( "DELETE FROM covers WHERE cover_id NOT IN ( SELECT DISTINCT song_coverid FROM songs );" ) );
            Queries.Add( wxT( "DELETE FROM plsets WHERE plset_type = 0 AND plset_songid NOT IN ( SELECT DISTINCT song_id FROM songs );" ) );
            Queries.Add( wxT( "DELETE FROM settags WHERE settag_songid NOT IN ( SELECT DISTINCT song_id FROM songs );" ) );

            int Index;
            int Count = Queries.Count();
            for( Index = 0; Index < Count; Index++ )
            {
                if( TestDestroy() )
                    break;
                //guLogMessage( wxT( "Executing: '%s'" ), Queries[ Index ].c_str() );
                Db->ExecuteUpdate( Queries[ Index ] );
            }
        }

    }
    return 0;
}


// -------------------------------------------------------------------------------- //
// guMediaVieweriPodDevice
// -------------------------------------------------------------------------------- //
guMediaVieweriPodDevice::guMediaVieweriPodDevice( wxWindow * parent, guMediaCollection &mediacollection,
                          const int basecmd, guMainFrame * mainframe, const int mode,
                          guPlayerPanel * playerpanel, guGIO_Mount * mount ) :
    guMediaViewerPortableDeviceBase( parent, mediacollection, basecmd, mainframe, mode, playerpanel, mount )
{
    m_PendingSaving = false;

    InitMediaViewer( mode );
}

// -------------------------------------------------------------------------------- //
guMediaVieweriPodDevice::~guMediaVieweriPodDevice()
{
}

// -------------------------------------------------------------------------------- //
void guMediaVieweriPodDevice::LoadMediaDb( void )
{
    Itdb_iTunesDB * iPodDb = itdb_parse( m_PortableDevice->MountPath().mb_str( wxConvFile ), NULL );

    m_PortableDevice->SetType( guPORTABLE_MEDIA_TYPE_IPOD );
    m_Db = new guIpodLibrary( guPATH_COLLECTIONS + m_MediaCollection->m_UniqueId + wxT( "/guayadeque.db" ), m_PortableDevice, iPodDb );
    m_Db->SetMediaViewer( this );

    guLogMessage( wxT( "Detected an Ipod with %i tracks and %i playlists" ),  itdb_tracks_number( iPodDb ), itdb_playlists_number( iPodDb ) );
    Itdb_Device * iPodDevice = iPodDb->device;
    if( iPodDevice )
    {
        //guLogMessage( wxT( "Artwork support : %i" ), itdb_device_supports_artwork( iPodDevice ) );
        const Itdb_IpodInfo * iPodInfo = itdb_device_get_ipod_info( iPodDevice );
        if( iPodInfo )
        {
            guLogMessage( wxT( "Model Number    : %s" ), wxString( iPodInfo->model_number, wxConvUTF8 ).c_str() );
            guLogMessage( wxT( "Capacity        : %.0f" ), iPodInfo->capacity );
            guLogMessage( wxT( "Model Name      : %s" ), wxString( itdb_info_get_ipod_model_name_string( iPodInfo->ipod_model ), wxConvUTF8 ).c_str() );
            guLogMessage( wxT( "Generation      : %s" ), wxString( itdb_info_get_ipod_generation_string( iPodInfo->ipod_generation ), wxConvUTF8 ).c_str() );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guMediaVieweriPodDevice::CreateContextMenu( wxMenu * menu, const int windowid )
{
    wxMenu * Menu = new wxMenu();
    wxMenuItem * MenuItem;

    int BaseCommand = GetBaseCommand();

    MenuItem = new wxMenuItem( Menu, BaseCommand + guCOLLECTION_ACTION_UPDATE_LIBRARY, _( "Update" ), _( "Update the collection library" ), wxITEM_NORMAL );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, BaseCommand + guCOLLECTION_ACTION_RESCAN_LIBRARY, _( "Rescan" ), _( "Rescan the collection library" ), wxITEM_NORMAL );
    Menu->Append( MenuItem );

    MenuItem = new wxMenuItem( Menu, BaseCommand + guCOLLECTION_ACTION_SEARCH_COVERS, _( "Search Covers" ), _( "Search the collection missing covers" ), wxITEM_NORMAL );
    Menu->Append( MenuItem );

    Menu->AppendSeparator();

    MenuItem = new wxMenuItem( menu, BaseCommand + guCOLLECTION_ACTION_UNMOUNT, _( "Unmount" ), _( "Unmount the device" ) );
    Menu->Append( MenuItem );

    Menu->AppendSeparator();

    MenuItem = new wxMenuItem( Menu, BaseCommand + guCOLLECTION_ACTION_VIEW_PROPERTIES, _( "Properties" ), _( "Show collection properties" ), wxITEM_NORMAL );
    Menu->Append( MenuItem );

    menu->AppendSeparator();
    menu->AppendSubMenu( Menu, m_MediaCollection->m_Name );
}

// -------------------------------------------------------------------------------- //
void guMediaVieweriPodDevice::HandleCommand( const int command )
{
    if( command == guCOLLECTION_ACTION_UNMOUNT )
    {
        if( m_PortableDevice->CanUnmount() )
        {
            m_PortableDevice->Unmount();
        }
    }
    else
    {
        guMediaViewer::HandleCommand( command );
    }
}

// -------------------------------------------------------------------------------- //
void guMediaVieweriPodDevice::EditProperties( void )
{
    guPortableMediaProperties * PortableMediaProperties = new guPortableMediaProperties( this, m_PortableDevice );
    if( PortableMediaProperties )
    {
        if( PortableMediaProperties->ShowModal() == wxID_OK )
        {
            PortableMediaProperties->WriteConfig();
            //
//            UpdateCollectionProperties();
        }
        PortableMediaProperties->Destroy();
    }
}

// -------------------------------------------------------------------------------- //
void guMediaVieweriPodDevice::UpdateLibrary( void )
{
    int GaugeId;
    GaugeId = m_MainFrame->AddGauge( m_MediaCollection->m_Name );

    if( m_UpdateThread )
    {
        m_UpdateThread->Pause();
        m_UpdateThread->Delete();
    }

    m_UpdateThread = new guIpodLibraryUpdate( this, GaugeId );
}

// -------------------------------------------------------------------------------- //
void guMediaVieweriPodDevice::CleanLibrary( void )
{
    wxCommandEvent event( wxEVT_MENU, ID_LIBRARY_CLEANFINISHED );
    AddPendingEvent( event );
}

// -------------------------------------------------------------------------------- //
void guMediaVieweriPodDevice::NormalizeTracks( guTrackArray * tracks, const bool isdrag )
{
    int Index;
    int Count = tracks->Count();
    for( Index = 0; Index < Count; Index++ )
    {
        tracks->Item( Index ).m_Type = guTRACK_TYPE_IPOD;
    }
}

// -------------------------------------------------------------------------------- //
bool guMediaVieweriPodDevice::SetAlbumCover( const int albumid, const wxString &albumpath, wxImage * coverimg )
{
    guTrackArray Tracks;
    wxArrayInt   Albums;
    Albums.Add( albumid );
    m_Db->GetAlbumsSongs( Albums, &Tracks );
    int Index;
    int Count;
    if( ( Count = Tracks.Count() ) )
    {
        int MaxSize = GetCoverMaxSize();
        if( MaxSize )
        {
            coverimg->Rescale( MaxSize, MaxSize, wxIMAGE_QUALITY_HIGH );
        }

        wxString TmpFile = wxFileName::CreateTempFileName( wxT( "guTmpImg_" ) );
        wxRemoveFile( TmpFile );
        TmpFile += wxT( ".jpg" );
        if( coverimg->SaveFile( TmpFile, wxBITMAP_TYPE_JPEG ) )
        {
            for( Index = 0; Index < Count; Index++ )
            {
                Itdb_Track * iPodTrack = ( ( guIpodLibrary * ) m_Db )->iPodFindTrack( Tracks[ Index ].m_FileName );
                if( iPodTrack )
                {
                    if( itdb_track_has_thumbnails( iPodTrack ) )
                    {
                        itdb_artwork_remove_thumbnails( iPodTrack->artwork );
                    }


                    if( !itdb_track_set_thumbnails( iPodTrack, TmpFile.mb_str( wxConvFile ) ) )
                        guLogMessage( wxT( "Couldnt set the cover image %s" ), TmpFile.c_str() );
                }

                if( m_PortableDevice->CoverFormats() & guPORTABLEMEDIA_COVER_FORMAT_EMBEDDED )
                {
                    guTagSetPicture( Tracks[ Index ].m_FileName, coverimg );
                }
            }

            ( ( guIpodLibrary * ) m_Db )->iPodFlush();

            wxRemoveFile( TmpFile );

            m_Db->SetAlbumCover( albumid, * coverimg );
        }
        else
        {
            guLogMessage( wxT( "Couldnt save the temporary image file" ) );
        }
    }

    return true;
}

// -------------------------------------------------------------------------------- //
bool guMediaVieweriPodDevice::SetAlbumCover( const int albumid, const wxString &albumpath, wxString &coverpath )
{
    wxURI Uri( coverpath );
    if( Uri.IsReference() )
    {
        wxImage CoverImage( coverpath );
        if( CoverImage.IsOk() )
        {
            return SetAlbumCover( albumid, albumpath, &CoverImage );
        }
        else
        {
            guLogError( wxT( "Could not load the imate '%s'" ), coverpath.c_str() );
        }
    }
    else
    {
        wxString TmpFile = wxFileName::CreateTempFileName( wxT( "guTmpImg_" ) );
        wxRemoveFile( TmpFile );
        TmpFile += wxT( ".jpg" );
        if( DownloadImage( coverpath, TmpFile, wxBITMAP_TYPE_JPEG ) )
        {
            bool Result = SetAlbumCover( albumid, albumpath, TmpFile );
            wxRemoveFile( TmpFile );
            return Result;
        }
        else
        {
            guLogError( wxT( "Failed to download file '%s'" ), coverpath.c_str() );
        }
    }

    return false;
}

// -------------------------------------------------------------------------------- //
void guMediaVieweriPodDevice::DeleteAlbumCover( const int albumid )
{
    guTrackArray Tracks;
    wxArrayInt   Albums;
    Albums.Add( albumid );
    m_Db->GetAlbumsSongs( Albums, &Tracks );
    int Index;
    int Count;
    if( ( Count = Tracks.Count() ) )
    {
        for( Index = 0; Index < Count; Index++ )
        {
            Itdb_Track * iPodTrack = ( ( guIpodLibrary * ) m_Db )->iPodFindTrack( Tracks[ Index ].m_FileName );
            if( iPodTrack )
            {
                guLogMessage( wxT( "Deleting cover for track '%s'" ), wxString( iPodTrack->ipod_path, wxConvUTF8 ).c_str() );
                itdb_artwork_remove_thumbnails( iPodTrack->artwork );
            }
        }

        ( ( guIpodLibrary * ) m_Db )->iPodFlush();

    }

    guMediaViewer::DeleteAlbumCover( albumid );
}

// -------------------------------------------------------------------------------- //
void guMediaVieweriPodDevice::DeleteTracks( const guTrackArray * tracks )
{
    int Index;
    int Count;
    if( ( Count = tracks->Count() ) )
    {
        m_Db->DeleteLibraryTracks( tracks, false );
        Itdb_iTunesDB * iPodDb = ( ( guIpodLibrary * ) m_Db )->GetiPodDb();

        Itdb_Playlist * MasterPlaylist = itdb_playlist_mpl( iPodDb );

        for( Index = 0; Index < Count; Index++ )
        {
            Itdb_Track * iPodTrack = ( ( guIpodLibrary * ) m_Db )->iPodFindTrack( tracks->Item( Index ).m_FileName );
            if( iPodTrack )
            {
                itdb_playlist_remove_track( MasterPlaylist, iPodTrack );
                itdb_track_remove( iPodTrack );
            }
            //
            if( !wxRemoveFile( tracks->Item( Index ).m_FileName ) )
            {
                guLogMessage( wxT( "Couldnt remove file %s" ), tracks->Item( Index ).m_FileName.c_str() );
            }
        }

        ( ( guIpodLibrary * ) m_Db )->iPodFlush();

    }
}

// -------------------------------------------------------------------------------- //
int guMediaVieweriPodDevice::CopyTo( const guTrack * track, wxString &filename )
{
    if( !track )
        return 0;

    Itdb_Track * iPodTrack = itdb_track_new();

    iPodTrack->title = strdup( track->m_SongName.ToUTF8() );
    iPodTrack->album = strdup( track->m_AlbumName.ToUTF8() );
    iPodTrack->artist = strdup( track->m_ArtistName.ToUTF8() );
    iPodTrack->genre = strdup( track->m_GenreName.ToUTF8() );
    iPodTrack->comment = strdup( track->m_Comments.ToUTF8() );
    iPodTrack->composer = strdup( track->m_Composer.ToUTF8() );
    iPodTrack->albumartist = strdup( track->m_AlbumArtist.ToUTF8() );
    iPodTrack->size = track->m_FileSize;
    iPodTrack->tracklen = track->m_Length;
    iPodTrack->track_nr = track->m_Number;
    guStrDiskToDiskNum( track->m_Disk, iPodTrack->cd_nr, iPodTrack->cds );
    iPodTrack->bitrate = track->m_Bitrate;
    iPodTrack->year = track->m_Year;
    iPodTrack->BPM = 0;
    iPodTrack->rating = ( track->m_Rating == wxNOT_FOUND ) ? 0 : track->m_Rating * ITDB_RATING_STEP;
    iPodTrack->playcount = 0;
    iPodTrack->type1 = 0;
    iPodTrack->type2 = track->m_Format == wxT( "mp3" ) ? 1 : 0;
    iPodTrack->compilation = 0; //!track->m_AlbumArtist.IsEmpty();
    iPodTrack->mediatype = ITDB_MEDIATYPE_AUDIO;
    iPodTrack->time_added = track->m_AddedTime;

    wxString CoverPath;
    if( track->m_CoverId && track->m_MediaViewer )
    {
        guDbLibrary * TrackDb = track->m_MediaViewer->GetDb();

        if( TrackDb )
        {
            CoverPath = TrackDb->GetCoverPath( track->m_CoverId );
        }

        if( wxFileExists( CoverPath ) )
        {
            if( !itdb_track_set_thumbnails( iPodTrack, CoverPath.mb_str( wxConvFile ) ) )
                guLogMessage( wxT( "Could not add cover to the ipod track" ) );
        }
    }

    Itdb_iTunesDB * iPodDb = ( ( guIpodLibrary * ) m_Db )->GetiPodDb();
    // Add the track to the iPod Database
    itdb_track_add( iPodDb, iPodTrack, wxNOT_FOUND );
    // Add th etrack to the master playlist
    Itdb_Playlist * MasterPlaylist = itdb_playlist_mpl( iPodDb );
    itdb_playlist_add_track( MasterPlaylist, iPodTrack, wxNOT_FOUND );

    int FileFormat = guGetTranscodeFileFormat( track->m_FileName.Lower().AfterLast( wxT( '.' ) ) );

    wxString TmpFile;

    // Copy the track file
    if( track->m_Offset ||
        !( m_PortableDevice->AudioFormats() & FileFormat ) ||
        ( m_PortableDevice->TranscodeScope() == guPORTABLEMEDIA_TRANSCODE_SCOPE_ALWAYS ) )
    {
        // We need to transcode the file to a temporary file and then copy it
        guLogMessage( wxT( "guIpodMediaLibPanel Transcode File start" ) );
        TmpFile = wxFileName::CreateTempFileName( wxT( "guTrcde_" ) );
        wxRemoveFile( TmpFile );
        TmpFile += wxT( "." ) + guTranscodeFormatString( m_PortableDevice->TranscodeFormat() );

        guTranscodeThread * TranscodeThread = new guTranscodeThread( track, TmpFile.wchar_str(), m_PortableDevice->TranscodeFormat(), m_PortableDevice->TranscodeQuality() );
        if( TranscodeThread && TranscodeThread->IsOk() )
        {
                // TODO : Make a better aproach to be sure its running
                // This need to be called from a thread
                wxThread::Sleep( 1000 );
                while( TranscodeThread->IsTranscoding() )
                {
                    wxThread::Sleep( 200 );
                }
        }

        if( !TranscodeThread->IsOk() )
        {
            guLogMessage( wxT( "Error transcoding the file '%s'" ), track->m_FileName.c_str() );
            wxRemoveFile( TmpFile );
            return wxNOT_FOUND;
        }

        iPodTrack->bitrate = guGetMp3QualityBitRate( m_PortableDevice->TranscodeQuality() );
        iPodTrack->size = guGetFileSize( TmpFile );

        //wxString guTagGetLyrics( const wxString &filename )
        if( !track->m_Offset )
        {
            wxString LyricText = guTagGetLyrics( track->m_FileName );
            if( !LyricText.IsEmpty() )
            {
                guTagSetLyrics( TmpFile, LyricText );
            }
        }
    }
    else    // The file is supported
    {
        TmpFile = wxFileName::CreateTempFileName( wxT( "guTrcde_" ) );
        wxRemoveFile( TmpFile );
        TmpFile += wxT( "." ) + track->m_FileName.Lower().AfterLast( wxT( '.' ) );
        if( !wxCopyFile( track->m_FileName, TmpFile ) )
        {
            guLogMessage( wxT( "Error copying file '%s' into '%s'" ), track->m_FileName.c_str(), TmpFile.c_str() );
            wxRemoveFile( TmpFile );
            return wxNOT_FOUND;
        }
    }

    //if( !CoverPath.IsEmpty() )
    if( m_PortableDevice->CoverFormats() & guPORTABLEMEDIA_COVER_FORMAT_EMBEDDED )
    {
        guTagSetPicture( TmpFile, CoverPath );
    }

    if( !itdb_cp_track_to_ipod( iPodTrack, TmpFile.mb_str( wxConvFile ), NULL ) )
    {
        guLogMessage( wxT( "Error trying to copy the file '%s'" ), TmpFile.c_str() );
        wxRemoveFile( TmpFile );
        return wxNOT_FOUND;
    }

    wxRemoveFile( TmpFile );

    filename = wxString( iPodTrack->ipod_path, wxConvUTF8 );

    return iPodTrack->size;
}

// -------------------------------------------------------------------------------- //
void guMediaVieweriPodDevice::UpdateTracks( const guTrackArray &tracks, const guImagePtrArray &images,
                                const wxArrayString &lyrics, const wxArrayInt &changedflags )
{
    int Index;
    int Count = tracks.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guTrack &Track = tracks[ Index ];
        Itdb_Track * iPodTrack = ( ( guIpodLibrary * ) m_Db )->iPodFindTrack( Track.m_FileName );
        if( iPodTrack )
        {
            CheckUpdateField( &iPodTrack->title, Track.m_SongName );
            CheckUpdateField( &iPodTrack->album, Track.m_AlbumName );
            CheckUpdateField( &iPodTrack->artist, Track.m_ArtistName );
            CheckUpdateField( &iPodTrack->genre, Track.m_GenreName );
            CheckUpdateField( &iPodTrack->comment, Track.m_Comments );
            CheckUpdateField( &iPodTrack->composer, Track.m_Composer );
            CheckUpdateField( &iPodTrack->albumartist, Track.m_AlbumArtist );
            iPodTrack->size = Track.m_FileSize;
            iPodTrack->tracklen = Track.m_Length;
            iPodTrack->track_nr = Track.m_Number;
            guStrDiskToDiskNum( Track.m_Disk, iPodTrack->cd_nr, iPodTrack->cds );
            iPodTrack->bitrate = Track.m_Bitrate;
            iPodTrack->year = Track.m_Year;
            iPodTrack->BPM = 0;
            iPodTrack->rating = ( Track.m_Rating == wxNOT_FOUND ) ? 0 : Track.m_Rating * ITDB_RATING_STEP;
            iPodTrack->type2 = Track.m_Format == wxT( "mp3" ) ? 1 : 0;
        }
    }
    ( ( guIpodLibrary * ) m_Db )->iPodFlush();

    guMediaViewer::UpdateTracks( tracks, images, lyrics, changedflags );
}

// -------------------------------------------------------------------------------- //
wxImage * guMediaVieweriPodDevice::GetAlbumCover( const int albumid, int &coverid, wxString &coverpath,
                                           const wxString &artistname, const wxString &albumname )
{
    Itdb_iTunesDB * iPodDb = ( ( guIpodLibrary * ) m_Db )->GetiPodDb();

    if( iPodDb )
    {
        wxString SongPath;
        wxSQLite3ResultSet dbRes;
        wxString query;

        if( albumid )
        {
            query = wxString::Format( wxT( "SELECT song_path, song_filename FROM songs WHERE song_albumid = %i LIMIT 1;" ), albumid );
        }
        else if( coverid )
        {
            query = wxString::Format( wxT( "SELECT song_path, song_filename FROM songs WHERE song_coverid = %i LIMIT 1;" ), coverid );
        }

        if( !query.IsEmpty() )
        {
            dbRes = m_Db->ExecuteQuery( query );

            if( dbRes.NextRow() )
            {
                SongPath = dbRes.GetString( 0 ) + dbRes.GetString( 1 );
            }
            dbRes.Finalize();

            if( !SongPath.IsEmpty() )
            {
                Itdb_Track * iPodTrack = ( ( guIpodLibrary * ) m_Db )->iPodFindTrack( SongPath );
                if( iPodTrack )
                {
                    GdkPixbuf * Pixbuf = ( GdkPixbuf * ) itdb_track_get_thumbnail( iPodTrack, -1, -1 );
                    if( Pixbuf )
                    {
                        void * Buffer = NULL;
                        gsize BufferSize = 0;
                        if( gdk_pixbuf_save_to_buffer( Pixbuf, ( gchar ** ) &Buffer, &BufferSize, "jpeg", NULL, "quality", "100", NULL ) )
                        {
                            wxMemoryInputStream ImageStream( Buffer, BufferSize );
                            wxImage * CoverImage = new wxImage( ImageStream, wxBITMAP_TYPE_JPEG );
                            if( CoverImage )
                            {
                                if( CoverImage->IsOk() )
                                {
                                    return CoverImage;
                                }
                                else
                                {
                                    delete CoverImage;
                                    guLogMessage( wxT( "Error in image from ipod..." ) );
                                }
                            }
                            g_free( Buffer );
                        }
                        else
                        {
                            guLogMessage( wxT( "Couldnt save to a buffer the pixbuf for cover %i" ), coverid );
                        }
                        g_object_unref( Pixbuf );
                    }
                    else
                    {
                        guLogMessage( wxT( "Couldnt get the pixbuf for cover %i" ), coverid );
                    }
                }
            }
        }
    }
    return NULL;
}

}

#endif
// -------------------------------------------------------------------------------- //
