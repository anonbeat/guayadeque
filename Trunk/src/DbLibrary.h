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
#ifndef DBLIBRARY_H
#define DBLIBRARY_H

#include "Podcasts.h"

// wxWidgets
#include <wx/wx.h>
#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/utils.h>
#include <wx/filefn.h>
#include <wx/dir.h>

// wxSqlite3
#include "wx/wxsqlite3.h"

// PLAYLISTS
#define GUPLAYLIST_STATIC       0
#define GUPLAYLIST_DYNAMIC      1

enum guTrackType {
    guTRACK_TYPE_DB,
    guTRACK_TYPE_NOTDB,
    guTRACK_TYPE_RADIOSTATION,
    guTRACK_TYPE_PODCAST
};

enum guTrackMode {
    guTRACK_MODE_USER,
    guTRACK_MODE_SMART,
    guTRACK_MODE_RANDOM,
    guTRACK_MODE_RADIO
};

// -------------------------------------------------------------------------------- //
class guTrack
{
  public:
    guTrackType     m_Type;
    int             m_SongId;
    wxString        m_SongName;
    int             m_AlbumId;
    wxString        m_AlbumName;
    int             m_ArtistId;
    wxString        m_ArtistName;
    int             m_GenreId;
    wxString        m_GenreName;
    int             m_PathId;
    wxString        m_FileName;           // mp3 filename
    unsigned int    m_FileSize;
    int             m_Number;             // the track num of the song into the album
    int             m_Year;               // the year of the song
    int             m_Length;             // the length of the song in seconds
    int             m_Bitrate;
    int             m_Rating;
    int             m_PlayCount;
    int             m_LastPlay;
    int             m_AddedTime;
    int             m_CoverId;
    guTrackMode     m_TrackMode;          // Indicate how the track was created

    guTrack() {
        m_Type = guTRACK_TYPE_DB;
        m_TrackMode = guTRACK_MODE_USER;
    };

    ~guTrack() {};
};
WX_DECLARE_OBJARRAY(guTrack, guTrackArray);

// -------------------------------------------------------------------------------- //
class guListItem //: public wxObject
{
  public:
    int m_Id;
    wxString m_Name;
    guListItem() {};
    guListItem( int NewId, const wxString &NewName = wxEmptyString ) { m_Id = NewId; m_Name = NewName; };
};
WX_DECLARE_OBJARRAY(guListItem,guListItems);

// -------------------------------------------------------------------------------- //
class guArrayListItem //: public wxObject
{
  private :
    int m_Id;
//    wxString m_Name;
    wxArrayInt m_Data;
  public :
    guArrayListItem() { m_Id = -1; };
    guArrayListItem( int NewId ) { m_Id = NewId; };
//    guArrayListItem( int NewId, const wxString &NewName ) { m_Id = NewId; m_Name = NewName; };
    ~guArrayListItem() {};
    void SetId( int NewId ) { m_Id = NewId; };
    void AddData( int m_Id ) { m_Data.Add( m_Id ); };
    void DelData( int m_Id ) { m_Data.Remove( m_Id ); };
    wxArrayInt GetData( void ) { return m_Data; };
    int GetId() { return m_Id; };
//    const wxString GetName( void ) { return m_Name; };
//    void SetName( const wxString &NewName ) { m_Name = NewName; };
};
WX_DECLARE_OBJARRAY(guArrayListItem, guArrayListItems);

// -------------------------------------------------------------------------------- //
class guCoverInfo
{
  public:
    int         m_AlbumId;
    wxString    m_AlbumName;
    wxString    m_ArtistName;
    wxString    m_PathName;

    guCoverInfo( const int m_Id, const wxString &Album, const wxString &Artist, const wxString &Path )
    {
      m_AlbumId = m_Id;
      m_AlbumName = Album;
      m_ArtistName = Artist;
      m_PathName = Path;
    }
};
WX_DECLARE_OBJARRAY(guCoverInfo, guCoverInfos);

// -------------------------------------------------------------------------------- //
class guAlbumItem : public guListItem
{
  public:
    int         m_ArtistId;
    int         m_CoverId;
    wxString    m_CoverPath;
    wxBitmap *  m_Thumb;
    int         m_Year;

    guAlbumItem() : guListItem()
    {
      m_Thumb = NULL;
      m_Year = 0;
    }

    guAlbumItem( int id, const wxString &label ) : guListItem( id, label )
    {
      m_Thumb = NULL;
      m_Year = 0;
    };

    ~guAlbumItem()
    {
      if( m_Thumb )
        delete m_Thumb;
    };

};
WX_DECLARE_OBJARRAY(guAlbumItem,guAlbumItems);

// -------------------------------------------------------------------------------- //
class guRadioStation
{
  public :
    int         m_Id;
    long        m_SCId;
    wxString    m_Name;
    wxString    m_Link;
    long        m_GenreId;
    bool        m_IsUser;
    wxString    m_Type;
    long        m_BitRate;
    long        m_Listeners;
};
WX_DECLARE_OBJARRAY(guRadioStation,guRadioStations);

enum guTRACKS_ORDER {
    guTRACKS_ORDER_NUMBER,
    guTRACKS_ORDER_TITLE,
    guTRACKS_ORDER_ARTIST,
    guTRACKS_ORDER_ALBUM,
    guTRACKS_ORDER_GENRE,
    guTRACKS_ORDER_LENGTH,
    guTRACKS_ORDER_YEAR,
    guTRACKS_ORDER_BITRATE,
    guTRACKS_ORDER_RATING,
    guTRACKS_ORDER_PLAYCOUNT,
    guTRACKS_ORDER_LASTPLAY,
    guTRACKS_ORDER_ADDEDDATE
};

#define guRADIOSTATIONS_ORDER_NAME        0
#define guRADIOSTATIONS_ORDER_BITRATE     1
#define guRADIOSTATIONS_ORDER_LISTENERS   2

#define ALBUMS_ORDER_NAME           0
#define ALBUMS_ORDER_YEAR           1
#define ALBUMS_ORDER_YEAR_REVERSE   2

// -------------------------------------------------------------------------------- //
class guAS_SubmitInfo //guAudioScrobbler_SubmitInfo
{
  public:
    int      m_Id;
    wxString m_ArtistName;
    wxString m_AlbumName;
    wxString m_TrackName;
    int      m_PlayedTime; // UTC Time since 1/1/1970
    wxChar   m_Source;
    wxChar   m_Ratting;
    int      m_TrackLen;
    int      m_TrackNum;
    int      m_MBTrackId;
};
WX_DECLARE_OBJARRAY(guAS_SubmitInfo, guAS_SubmitInfoArray);

// -------------------------------------------------------------------------------- //
void inline escape_query_str( wxString * Str )
{
  Str->Replace( _T( "'" ), _T( "''" ) );
  Str->Replace( _T( "\"" ), _T( "\"\"" ) );
  Str->Replace( _T( "\\" ), _T( "\\\\" ) );
}

// -------------------------------------------------------------------------------- //
wxString inline escape_query_str( const wxString &str )
{
    wxString QueryStr = str;
    escape_query_str( &QueryStr );
    return QueryStr;
}

#define GULIBRARY_FILTER_LABELS     0
#define GULIBRARY_FILTER_GENRES     1
#define GULIBRARY_FILTER_ARTISTS    2
#define GULIBRARY_FILTER_ALBUMS     3
#define GULIBRARY_FILTER_SONGS      4

class guDynPlayList;

// -------------------------------------------------------------------------------- //
class DbLibrary {
  private :
    wxSQLite3Database  m_Db;
    wxArrayString      m_LibPaths;
    guTrack            m_CurSong;
    bool               m_NeedUpdate;
    //wxString           m_UpTag;

    // Library Filter Options
    wxArrayInt         m_GeFilters;
    wxArrayInt         m_LaFilters;
    wxArrayInt         m_ArFilters;
    wxArrayInt         m_AlFilters;
    wxArrayString      m_TeFilters; // Text string filters

    // Radio Filters Options
    wxArrayInt         m_RaGeFilters;
    bool               m_RadioIsUser;
    wxArrayInt         m_RaLaFilters;
    wxArrayString      m_RaTeFilters;

    guTRACKS_ORDER     m_TracksOrder;
    bool               m_TracksOrderDesc;
    int                m_StationsOrder; // 0 -> Name, 1 -> BitRate, 2 -> Listeners
    bool               m_StationsOrderDesc;
    int                m_AlbumsOrder;   // 0 ->

//    guListItems         Labels;
    guListItems         m_GenresCache;
    guListItems         m_ArtistsCache;
    guAlbumItems        m_AlbumsCache;
    guListItems         m_PathsCache;

    wxArrayString       m_CoverSearchWords;

    // Podcasts
    wxArrayInt          m_PodChFilters;
    int                 m_PodcastOrder;
    bool                m_PodcastOrderDesc;

    wxString            FiltersSQL( int Level );


    void inline         FillTrackFromDb( guTrack * Song, wxSQLite3ResultSet * dbRes );
    int                 GetRadioFiltersCount( void ) const;
    wxString            RadioFiltersSQL( void );

  public :
                        DbLibrary();
                        DbLibrary( const wxString &DbName );
                        ~DbLibrary();
    int                 Open( const wxString &DbPath );
    int                 Close();
    bool                NeedUpdate( void ) { return m_NeedUpdate; };

    unsigned long       GetDbVersion( void );
    void                DoCleanUp( void );
    bool                CheckDbVersion( const wxString &DbName );
    void                LoadCache( void );

    int                 GetGenreId( int * GenreId, wxString &GenreName );
    const wxString      GetArtistName( const int ArtistId );
    bool                GetArtistId( int * ArtistId, wxString &ArtistName, bool Create = true );
    int                 AddCoverFile( const wxString &coverfile, const wxString &coverhash = wxEmptyString );
    void                UpdateCoverFile( int coverid, const wxString &coverfile, const wxString &coverhash );
    int                 FindCoverFile( const wxString &DirName );
    int                 SetAlbumCover( const int AlbumId, const wxString & CoverPath, const wxString &coverhash = wxEmptyString );
    bool                GetAlbumInfo( const int AlbumId, wxString * AlbumName, wxString * ArtistName, wxString * AlbumPath );
    int                 GetAlbumCoverId( const int AlbumId );

    wxString            GetCoverPath( const int CoverId );
    int                 GetAlbumId( int * AlbumId, int * CoverId, wxString &AlbumName, const int ArtistId, const int PathId );
    int                 GetSongId( int * SongId, wxString &FileName, const int PathId );
    int                 GetSongId( int * SongId, wxString &FileName, wxString &FilePath );
    wxArrayInt          GetLabelIds( const wxArrayString &Labels );
    wxArrayString       GetLabelNames( const wxArrayInt &LabelIds );
    int                 GetLabelId( int * LabelId, wxString &LabelName );
    int                 GetPathId( int * PathId, wxString &PathValue );
    int                 UpdateSong( void );

    void                GetGenres( guListItems * Genres, bool FullList = false );
    void                GetLabels( guListItems * Labels, bool FullList = false );
    void                GetRadioLabels( guListItems * Labels, bool FullList = false );
    void                GetArtists( guListItems * Artists, bool FullList = false );
    void                GetAlbums( guAlbumItems * Albums, bool FullList = false );
    wxArrayString       GetAlbumsPaths( const wxArrayInt &AlbumIds );

    int                 CreateStaticPlayList( const wxString &name, const wxArrayInt &tracks );
    int                 UpdateStaticPlayList( const int plid, const wxArrayInt &tracks );
    int                 AppendStaticPlayList( const int plid, const wxArrayInt &tracks );
    int                 DeleteStaticPlaylistTracks( const int plid, const wxArrayInt &tracks );
    int                 GetPlayListFiles( const int plid, wxFileDataObject * Files );
    void                GetPlayLists( guListItems * PlayLists, const int type );
    int                 GetPlayListSongIds( const int plid, wxArrayInt * tracks );
    int                 GetPlayListSongs( const int plid, const int pltype, guTrackArray * tracks );
    void                DeletePlayList( const int plid );
    void                SetPlayListName( const int plid, const wxString &plname );
    void                GetDynamicPlayList( const int plid, guDynPlayList * playlist );
    int                 CreateDynamicPlayList( const wxString &name, guDynPlayList * playlist );
    void                UpdateDynPlayList( const int plid, const guDynPlayList * playlist );

    void                GetPaths( guListItems * Paths, bool FullList = false );

    int                 GetSongs( const wxArrayInt &SongIds, guTrackArray * Songs );
    int                 GetSongs( guTrackArray * Songs );
    void                SetSongsOrder( const guTRACKS_ORDER order );
    guTRACKS_ORDER      GetSongsOrder( void ) const;
    bool                GetSongsOrderDesc( void ) const;
    void                UpdateSongs( guTrackArray * Songs );
    int                 GetAlbumsSongs( const wxArrayInt &Albums, guTrackArray * Songs, bool ordertoedit = false );
    int                 GetArtistsSongs( const wxArrayInt &Artists, guTrackArray * Songs, guTrackMode trackmode = guTRACK_MODE_USER );
    int                 GetArtistsAlbums( const wxArrayInt &Artists, wxArrayInt * Albums );
    int                 GetGenresSongs( const wxArrayInt &Genres, guTrackArray * Songs );
    int                 GetRandomTracks( guTrackArray * Tracks );

    int                 GetLabelsSongs( const wxArrayInt &Labels, guTrackArray * Songs );
    int                 AddLabel( wxString LabelName );
    int                 SetLabelName( const int labelid, const wxString &oldlabel, const wxString &newlabel );
    int                 DelLabel( const int LabelId );
    wxArrayInt          GetLabels( void );

    int                 GetRadioLabelsSongs( const wxArrayInt &Labels, guTrackArray * Songs );
    int                 AddRadioLabel( wxString LabelName );
    int                 SetRadioLabelName( const int LabelId, wxString NewName );
    int                 DelRadioLabel( const int LabelId );
    wxArrayInt          GetRadioLabels( void );

    int                 SongCount( void );


    void                SetLibPath( const wxArrayString &NewPaths );
    int                 ReadFileTags( const char * filename );
    void                UpdateImageFile( const char * filename );

    int                 GetFiltersCount() const;
    void                SetTeFilters( const wxArrayString &NewTeFilters );
    void                SetGeFilters( const wxArrayInt &NewGeFilters );
    void                SetTaFilters( const wxArrayInt &NewTaFilters );
    void                SetArFilters( const wxArrayInt &NewArFilters );
    void                SetAlFilters( const wxArrayInt &NewAlFilters );

    //
    guArrayListItems    GetArtistsLabels( const wxArrayInt &Artists );
    guArrayListItems    GetAlbumsLabels( const wxArrayInt &Albums );
    guArrayListItems    GetSongsLabels( const wxArrayInt &Songs );
    void                SetArtistsLabels( const wxArrayInt &Artists, const wxArrayInt &Labels );
    void                SetAlbumsLabels( const wxArrayInt &Albums, const wxArrayInt &Labels );
    void                SetSongsLabels( const wxArrayInt &Songs, const wxArrayInt &Labels );

    void                UpdateArtistsLabels( const wxArrayInt &Artists, const wxArrayInt &Labels );
    void                UpdateAlbumsLabels( const wxArrayInt &Albums, const wxArrayInt &Labels );
    void                UpdateSongsLabels( const wxArrayInt &Songs, const wxArrayInt &Labels );
    void                UpdateSongsLabel( const guTrackArray * tracks, const wxString &label, const wxString &newlabel );

    void                SetTrackRating( const int songid, const int rating );
    void                SetTracksRating( const wxArrayInt &songids, const int rating );

    void                SetTrackPlayCount( const int songid, const int playcount );


    wxSQLite3ResultSet  ExecuteQuery(  const wxSQLite3StatementBuffer &query );
    int                 ExecuteUpdate( const wxString &query );
    int                 ExecuteUpdate( const wxSQLite3StatementBuffer &query );
    wxSQLite3ResultSet  ExecuteQuery( const wxString &query );
    wxBitmap *          GetCoverThumb( int CoverId );
    guCoverInfos        GetEmptyCovers( void );

    // Smart Playlist and LastFM Panel support functions
    int                 FindArtist( const wxString &Artist );
    int                 FindAlbum( const wxString &Artist, const wxString &Album );
    int                 FindTrack( const wxString &Artist, const wxString &m_Name );
    guTrack *           FindSong( const wxString &Artist, const wxString &Track );
    int                 FindTrackFile( const wxString &filename, guTrack * song );

    //
    // Radio support functions
    //
    void                    SetRaTeFilters( const wxArrayString &filters );
    void                    SetRadioLabelsFilters( const wxArrayInt &filters );
    void                    SetRadioGenresFilters( const wxArrayInt &filters );
    void                    SetRadioIsUserFilter( bool isuserradio );
    void                    GetRadioGenresList( guListItems * RadioGenres, const wxArrayInt &GenreIds );
    void                    GetRadioGenres( guListItems * RadioGenres, bool AllowFilter = true );
    void                    SetRadioGenres( const wxArrayString &Genres );
    int                     GetRadioStations( guRadioStations * RadioStations );
    void                    SetRadioStation( const guRadioStation * RadioStation );
    bool                    GetRadioStation( const int id, guRadioStation * radiostation );
    void                    SetRadioStations( const guRadioStations * RadioStations );
    guArrayListItems        GetStationsLabels( const wxArrayInt &Stations );
    void                    SetRadioStationsLabels( const wxArrayInt &Stations, const wxArrayInt &Labels );
    int                     DelRadioStations( const wxArrayInt &RadioGenresIds );
    int                     DelRadioStation( const int radioid );
    void                    SetRadioStationsOrder( int OrderValue );

    int                     AddRadioGenre( wxString GenreName );
    int                     SetRadioGenreName( const int GenreId, wxString GenreName );
    int                     DelRadioGenre( const int GenreId );

    //
    // AudioScrobbler functions
    //
    bool                    AddCachedPlayedSong( const guTrack &Song );
    guAS_SubmitInfoArray    GetCachedPlayedSongs( int MaxCount = 10 );
    bool                    DeleteCachedPlayedSongs( const guAS_SubmitInfoArray &SubmitInfo );

    //
    // Lyrics functions
    //
    wxString                SearchLyric( const wxString &artist, const wxString &trackname );
    bool                    CreateLyric( const wxString &artist, const wxString &trackname, const wxString &text );

    //
    // Podcasts functions
    //
    int                     GetPodcastChannels( guPodcastChannelArray * channels );
    void                    SavePodcastChannel( guPodcastChannel * channel, bool onlynew = false );
    int                     SavePodcastChannels( guPodcastChannelArray * channels, bool onlynew = false );
    int                     GetPodcastChannelUrl( const wxString &url, guPodcastChannel * channel = NULL );
    int                     GetPodcastChannelId( const int id, guPodcastChannel * channel = NULL );
    void                    DelPodcastChannel( const int id );

    int                     GetPodcastItems( guPodcastItemArray * items );
    int                     GetPodcastItems( const wxArrayInt &ids, guPodcastItemArray * items );
    void                    SavePodcastItem( const int channelid, guPodcastItem * item, bool onlynew = false );
    void                    SavePodcastItems( const int channelid, guPodcastItemArray * items, bool onlynew = false );
    void                    SetPodcastItemStatus( const int itemid, const int status );
    void                    SetPodcastItemPlayCount( const int itemid, const int playcount );
    int                     GetPodcastItemEnclosure( const wxString &enclosure, guPodcastItem * item = NULL );
    int                     GetPodcastItemId( const int itemid, guPodcastItem * item = NULL );
    int                     GetPodcastItemFile( const wxString &filename, guPodcastItem * item = NULL );
    void                    DelPodcastItem( const int itemid );
    void                    DelPodcastItems( const int channelid );
    void                    SetPodcastChannelFilters( const wxArrayInt &filters );
    void                    SetPodcastOrder( int order );
    int                     GetPendingPodcasts( guPodcastItemArray * items );
    int                     GetPodcastFiles( const wxArrayInt &channelid, wxFileDataObject * files );

};

// -------------------------------------------------------------------------------- //
// Array Functions
// -------------------------------------------------------------------------------- //
int guAlbumItemSearch( const guAlbumItems &Items, int StartPos, int EndPos, int m_Id );
wxString guAlbumItemsGetName( const guAlbumItems &Items, int m_Id );
int guAlbumItemsGetCoverId( const guAlbumItems &Items, int m_Id );
int guListItemSearch( const guListItems &Items, int StartPos, int EndPos, int m_Id );
wxString guListItemsGetName( const guListItems &Items, int m_Id );
wxArrayInt GetArraySameItems( const wxArrayInt &Source, const wxArrayInt &Oper );
wxArrayInt GetArrayDiffItems( const wxArrayInt &Source, const wxArrayInt &Oper );

#endif
// -------------------------------------------------------------------------------- //
