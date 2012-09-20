// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2012 J.Rios
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
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#ifndef DBLIBRARY_H
#define DBLIBRARY_H

#include "Db.h"
#include "Podcasts.h"

// wxWidgets
#include <wx/wx.h>
#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/utils.h>
#include <wx/filefn.h>
#include <wx/dir.h>
#include <wx/dynarray.h>

// wxSqlite3
#include "wx/wxsqlite3.h"

#define GU_TRACKS_QUERYSTR   wxT( "SELECT song_id, song_name, song_genreid, song_genre, song_artistid, song_artist, "\
               "song_albumartistid, song_albumartist, song_composerid, song_composer, song_albumid, song_album, "\
               "song_pathid, song_path, song_filename, song_format, song_disk, song_number, song_year, song_comment, "\
               "song_coverid, song_offset, song_length, song_bitrate, song_rating, song_playcount, "\
               "song_addedtime, song_lastplay, song_filesize "\
               "FROM songs " )

// PLAYLISTS
#define guPLAYLIST_TYPE_STATIC       0
#define guPLAYLIST_TYPE_DYNAMIC      1

extern unsigned long DynPLDateOption2[];

#define guTRACK_TYPE_STOP_HERE      0x80000000

enum guTrackType {
    guTRACK_TYPE_DB = 0,
    guTRACK_TYPE_NOTDB,
    guTRACK_TYPE_RADIOSTATION,
    guTRACK_TYPE_PODCAST,
    guTRACK_TYPE_JAMENDO,
    guTRACK_TYPE_MAGNATUNE,
    guTRACK_TYPE_IPOD
};

enum guTrackMode {
    guTRACK_MODE_USER,
    guTRACK_MODE_SMART,
    guTRACK_MODE_RANDOM,
    guTRACK_MODE_RADIO
};

enum guRandomMode {
    guRANDOM_MODE_TRACK,
    guRANDOM_MODE_ALBUM
};

class guMediaViewer;
class guLibPanel;
class guDbLibrary;

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
    int             m_AlbumArtistId;
    wxString        m_AlbumArtist;
    int             m_GenreId;
    wxString        m_GenreName;
    int             m_PathId;
    wxString        m_Path;
    wxString        m_FileName;           // mp3 path + filename
    unsigned int    m_FileSize;
    int             m_Number;             // the track num of the song into the album
    int             m_Year;               // the year of the song
    unsigned int    m_Length;             // the length of the song in seconds
    unsigned int    m_Offset;
    int             m_Bitrate;
    int             m_Rating;
    int             m_PlayCount;
    int             m_LastPlay;
    int             m_AddedTime;
    int             m_CoverId;
    //
    wxString        m_Disk;
    wxString        m_Comments;
    wxString        m_Composer;
    int             m_ComposerId;
    wxString        m_Format;
    //
    guTrackMode     m_TrackMode;          // Indicate how the track was created
    guMediaViewer * m_MediaViewer;

    guTrack() {
        m_MediaViewer = NULL;
        m_Type = guTRACK_TYPE_DB;
        m_TrackMode = guTRACK_MODE_USER;
        m_Bitrate = 0;
        m_Number = 0;
        m_Year = 0;
        m_Length = 0;
        m_Offset = 0;
        m_Bitrate = 0;
        m_Rating = wxNOT_FOUND;
        m_PlayCount = 0;
        m_LastPlay = 0;
        m_AddedTime = 0;
        m_CoverId = 0;
    };

    ~guTrack() {};

    bool ReadFromFile( const wxString &file );

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
    int Index( int id ) { return m_Data.Index( id ); }
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
    wxString    m_AlbumPath;

    guCoverInfo( const int m_Id, const wxString &Album, const wxString &Artist, const wxString &Path )
    {
      m_AlbumId = m_Id;
      m_AlbumName = Album;
      m_ArtistName = Artist;
      m_AlbumPath = Path;
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
    wxString    m_ArtistName;

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

enum guRadioSource {
    guRADIO_SOURCE_SHOUTCAST_GENRE = 0,
    guRADIO_SOURCE_USER,
    guRADIO_SOURCE_SHOUTCAST_SEARCH,
    guRADIO_SOURCE_TUNEIN
};

enum guTRACKS_ORDER {
    guTRACKS_ORDER_NUMBER,
    guTRACKS_ORDER_TITLE,
    guTRACKS_ORDER_ARTIST,
    guTRACKS_ORDER_ALBUMARTIST,
    guTRACKS_ORDER_ALBUM,
    guTRACKS_ORDER_GENRE,
    guTRACKS_ORDER_COMPOSER,
    guTRACKS_ORDER_DISK,
    guTRACKS_ORDER_LENGTH,
    guTRACKS_ORDER_YEAR,
    guTRACKS_ORDER_BITRATE,
    guTRACKS_ORDER_RATING,
    guTRACKS_ORDER_PLAYCOUNT,
    guTRACKS_ORDER_LASTPLAY,
    guTRACKS_ORDER_ADDEDDATE,
    guTRACKS_ORDER_FORMAT,
    guTRACKS_ORDER_FILEPATH
};

enum guALBUMS_ORDER {
    guALBUMS_ORDER_NAME = 0,
    guALBUMS_ORDER_YEAR,
    guALBUMS_ORDER_YEAR_REVERSE,
    guALBUMS_ORDER_ARTIST_NAME,
    guALBUMS_ORDER_ARTIST_YEAR,
    guALBUMS_ORDER_ARTIST_YEAR_REVERSE,
    guALBUMS_ORDER_ADDEDTIME
};

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
    wxChar   m_Rating;
    int      m_TrackLen;
    int      m_TrackNum;
    int      m_MBTrackId;
};
WX_DECLARE_OBJARRAY(guAS_SubmitInfo, guAS_SubmitInfoArray);

enum guLIBRARY_FILTER {
    guLIBRARY_FILTER_LABELS = 0,
    guLIBRARY_FILTER_GENRES,
    guLIBRARY_FILTER_COMPOSERS,
    guLIBRARY_FILTER_ALBUMARTISTS,
    guLIBRARY_FILTER_ARTISTS,
    guLIBRARY_FILTER_YEARS,
    guLIBRARY_FILTER_ALBUMS,
    guLIBRARY_FILTER_RATINGS,
    guLIBRARY_FILTER_PLAYCOUNTS,
    guLIBRARY_FILTER_SONGS
};

class guDynPlayList;
class guAlbumBrowserItemArray;
class guTreeViewFilterArray;
class guCurrentTrack;

// -------------------------------------------------------------------------------- //
class guDbLibrary : public guDb
{
  private :
    wxString     m_LastAlbum;
    int          m_LastAlbumId;
    wxString     m_LastGenre;
    int          m_LastGenreId;
    int          m_LastCoverId;
    wxString     m_LastCoverPath;
    guListItems  m_LastItems;
    wxString     m_LastPath;
    int          m_LastPathId;
    wxString     m_LastArtist;
    int          m_LastArtistId;
    wxString     m_LastAlbumArtist;
    int          m_LastAlbumArtistId;
    wxString     m_LastComposer;
    int          m_LastComposerId;

  protected :
    guMediaViewer *     m_MediaViewer;
//    wxString            m_ConfigPath;
    bool                m_NeedUpdate;

    // Library Filter Options
    wxArrayInt         m_GeFilters;
    wxArrayInt         m_LaFilters; // Label
    wxArrayInt         m_CoFilters; // Composers
    wxArrayInt         m_AAFilters; // AlbumArtist
    wxArrayInt         m_ArFilters; // Artist
    wxArrayInt         m_AlFilters; // Album
    wxArrayString      m_TeFilters; // Text string filters
    wxArrayInt         m_YeFilters; // Year
    wxArrayInt         m_RaFilters; // Ratting
    wxArrayInt         m_PcFilters; // PlayCount

    int                m_TracksOrder;
    bool               m_TracksOrderDesc;
    int                m_AlbumsOrder;   // 0 ->

    wxArrayString       m_CoverSearchWords;

    wxString            FiltersSQL( int Level );

    void inline         FillTrackFromDb( guTrack * Song, wxSQLite3ResultSet * dbRes );

  public :
                        guDbLibrary();
                        guDbLibrary( const wxString &DbName );
                        guDbLibrary( guDb * db );
                        ~guDbLibrary();

    virtual int         GetDbVersion( void );

//    void                ConfigChanged( void );

    bool                NeedUpdate( void ) { return m_NeedUpdate; };

    guMediaViewer *     GetMediaViewer( void ) { return m_MediaViewer; }
    void                SetMediaViewer( guMediaViewer * mediaviewer );

    void                DoCleanUp( void );
    void                CleanItems( const wxArrayInt &tracks, const wxArrayInt &covers );
    void                CleanFiles( const wxArrayString &files );
    virtual bool        CheckDbVersion( void );
    //void                LoadCache( void );

    int                 GetGenreId( wxString &GenreName );
    int                 GetComposerId( wxString &composername, bool create = true );
    const wxString      GetArtistName( const int ArtistId );
    int                 GetArtistId( wxString &ArtistName, bool Create = true );
    int                 GetAlbumArtistId( wxString &albumartist, bool create = true );
    int                 AddCoverImage( const wxImage &image );
    int                 AddCoverFile( const wxString &coverfile, const wxString &coverhash = wxEmptyString );
    void                UpdateCoverFile( int coverid, const wxString &coverfile, const wxString &coverhash );
//    int                 FindCoverFile( const wxString &DirName );
    int                 SetAlbumCover( const int AlbumId, const wxString & CoverPath, const wxString &coverhash = wxEmptyString );
    int                 SetAlbumCover( const int AlbumId, const wxImage &image );
    bool                GetAlbumInfo( const int AlbumId, wxString * AlbumName, wxString * ArtistName, wxString * AlbumPath );
    int                 GetAlbumCoverId( const int AlbumId );

    wxString            GetCoverPath( const int CoverId );
    int                 GetAlbumId( wxString &AlbumName, const int ArtistId, const int PathId, const wxString &Path, const int coverid = 0 );
    int                 GetSongId( wxString &filename, const int pathid, const time_t filedate, const int start = 0, bool * created = NULL );
    int                 GetSongId( wxString &FileName, wxString &FilePath, const time_t filedate, const int start = 0, bool * created = NULL );
    wxArrayInt          GetLabelIds( const wxArrayString &Labels );
    wxArrayString       GetLabelNames( const wxArrayInt &LabelIds );
    int                 GetLabelId( int * LabelId, wxString &LabelName );
    int                 PathExists( const wxString &path );
    int                 GetPathId( wxString &PathValue );
    virtual int         UpdateSong( const guTrack &track, const bool allowrating = false );
    int                 AddFiles( const wxArrayString &files );

    void                GetGenres( guListItems * Genres, const bool FullList = false );
    void                GetLabels( guListItems * Labels, const bool FullList = false );
    void                GetArtists( guListItems * Artists, const bool FullList = false );
    void                GetYears( guListItems * items, const bool FullList = false );
    void                GetRatings( guListItems * items, const bool FullList = false );
    void                GetPlayCounts( guListItems * items, const bool FullList = false );
    void                GetAlbumArtists( guListItems * items, const bool FullList = false );
    void                GetComposers( guListItems * items, const bool FullList = false );
    void                SetAlbumsOrder( const int order );
    int                 GetAlbumsOrder( void ) { return m_AlbumsOrder; };
    void                GetAlbums( guListItems * Albums, bool FullList = false );
    void                GetAlbums( guAlbumItems * Albums, bool FullList = false );
    wxArrayString       GetAlbumsPaths( const wxArrayInt &AlbumIds );
    int                 GetAlbums( guAlbumBrowserItemArray * items, guDynPlayList * filter,
                            const wxArrayString &textfilters, const int start, const int count, const int order );
    int                 GetAlbumsCount( guDynPlayList * filter, const wxArrayString &textfilters );
    bool                GetAlbumDetails( const int albumid, int * year, int * trackcount );
    int                 GetAlbumYear( const int albumid );
    int                 GetAlbumTrackCount( const int albumid );

    void                GetPaths( guListItems * Paths, bool FullList = false );

    bool                GetSong( const int songid, guTrack * song );
    void                GetSongsCounters( const guTreeViewFilterArray &filters, const wxArrayString &textfilters, wxLongLong * count, wxLongLong * len, wxLongLong * size );
    int                 GetSongs( const guTreeViewFilterArray &filters, guTrackArray * Songs, const wxArrayString &textfilters, const int order, const bool orderdesc );
    int                 GetSongs( const wxArrayInt &SongIds, guTrackArray * Songs );
    int                 GetSongsCount( void );
    int                 GetSongs( guTrackArray * Songs, const int start, const int end );
    int                 GetSongsId( const int start );
    wxString            GetSongsName( const int start );
    void                GetTracksCounters( wxLongLong * count, wxLongLong * len, wxLongLong * size );
    void                DeleteLibraryTracks( const guTrackArray * tracks, const bool savedeleted );
    bool                FindDeletedFile( const wxString &file, const bool create );

    int                 GetTracksOrder( void ) const { return m_TracksOrder; }
    void                SetTracksOrder( const int order ) { m_TracksOrder = order; }
    bool                GetTracksOrderDesc( void ) const { return m_TracksOrderDesc; }
    void                SetTracksOrderDesc( const bool orderdesc ) { m_TracksOrderDesc = orderdesc; }


    void                UpdateSongs( const guTrackArray * Songs, const wxArrayInt &changedflags );
    void                UpdateTracksWithValue( guTrackArray * tracks, const int fieldid, void * value );
    void                UpdateTrackLength( const int trackid, const int length );
    void                UpdateTrackBitRate( const int trackid, const int bitrate );
    int                 GetAlbumsSongs( const wxArrayInt &Albums, guTrackArray * Songs, bool ordertoedit = false );
    int                 GetArtistsSongs( const wxArrayInt &Artists, guTrackArray * Songs,
                                         guTrackMode trackmode = guTRACK_MODE_USER );
    int                 GetArtistsSongs( const wxArrayInt &Artists, guTrackArray * Songs,
                                         const int count, const int filterallow, const int filterdeny );
    int                 GetArtistsAlbums( const wxArrayInt &Artists, wxArrayInt * Albums );
    int                 GetAlbumArtistsAlbums( const wxArrayInt &albumartists, wxArrayInt * Albums );
    int                 GetGenresSongs( const wxArrayInt &Genres, guTrackArray * Songs );
    int                 GetRandomTracks( guTrackArray * Tracks, const int count, const int rndmode,
                                         const int allowplaylist, const int denyplaylist );
    int                 GetLabelsSongs( const wxArrayInt &Labels, guTrackArray * Songs );
    int                 AddLabel( wxString LabelName );
    int                 SetLabelName( const int labelid, const wxString &oldlabel, const wxString &newlabel );
    int                 DelLabel( const int LabelId );
    wxArrayInt          GetLabels( void );

    int                 GetStaticPlayList( const wxString &name );
    virtual int         CreateStaticPlayList( const wxString &name, const wxArrayInt &tracks );
    virtual int         UpdateStaticPlayList( const int plid, const wxArrayInt &tracks );
    virtual int         AppendStaticPlayList( const int plid, const wxArrayInt &tracks );
    virtual void        UpdateStaticPlayListFile( const int plid );

    virtual int         DelPlaylistSetIds( const int plid, const wxArrayInt &setids );
    int                 GetPlayListFiles( const int plid, wxFileDataObject * Files );
    int                 GetPlayListsCount( void );
    void                GetPlayLists( guListItems &playlists );
    void                GetPlayLists( wxArrayString &playlistnames );
    void                GetPlayLists( wxArrayInt &playlistids );
    void                GetPlayLists( guListItems * PlayLists, const int type, const wxArrayString * textfilters = NULL );
    int                 GetPlayListSongIds( const int plid, wxArrayInt * tracks );
    int                 GetPlayListSongs( const int plid, guTrackArray * tracks );
    int                 GetPlayListSongs( const int plid, const int pltype, guTrackArray * tracks,
                            wxArrayInt * setids = NULL, wxLongLong * len = NULL, wxLongLong * size = NULL,
                            const int order = wxNOT_FOUND, const bool orderdesc = false );
    int                 GetPlayListSongs( const wxArrayInt &ids, const wxArrayInt &types, guTrackArray * tracks,
                            wxArrayInt * setids = NULL, wxLongLong * len = NULL, wxLongLong * size = NULL,
                            const int order = wxNOT_FOUND, const bool orderdesc = false );
    int                 GetPlayListSetIds( const int plid, wxArrayInt * setids );
    int                 GetPlayListSetIds( const wxArrayInt &plid, wxArrayInt * setids );
    virtual void        DeletePlayList( const int plid );
    virtual void        SetPlayListName( const int plid, const wxString &plname );
    wxString            GetPlayListName( const int plid );
    wxString            GetPlayListPath( const int plid );
    void                SetPlayListPath( const int plid, const wxString &path );
    void                GetDynamicPlayList( const int plid, guDynPlayList * playlist );
    virtual int         CreateDynamicPlayList( const wxString &name, const guDynPlayList * playlist );
    virtual void        UpdateDynamicPlayList( const int plid, const guDynPlayList * playlist );
    wxString            GetPlayListQuery( const int plid );
    int                 GetPlayListType( const int plid );

//    void                SetLibPath( const wxArrayString &NewPaths );
    int                 ReadFileTags( const wxString &filename, const bool allowrating = false );
    void                UpdateImageFile( const char * filename, const char * saveto,
                                         const int type = wxBITMAP_TYPE_JPEG, const int maxsize = wxNOT_FOUND );

    int                 GetFiltersCount() const;
    void                SetTeFilters( const wxArrayString &tefilters, const bool locked );
    void                SetGeFilters( const wxArrayInt &filters, const bool locked  );
    void                SetLaFilters( const wxArrayInt &filters, const bool locked );
    void                SetArFilters( const wxArrayInt &filters, const bool locked );
    void                SetAlFilters( const wxArrayInt &filters, const bool locked );
    void                SetYeFilters( const wxArrayInt &filter, const bool locked );
    void                SetRaFilters( const wxArrayInt &filter );
    void                SetPcFilters( const wxArrayInt &filter );
    void                SetCoFilters( const wxArrayInt &filter, const bool locked );
    void                SetAAFilters( const wxArrayInt &filter, const bool locked );

    //
    guArrayListItems    GetArtistsLabels( const wxArrayInt &Artists );
    guArrayListItems    GetAlbumsLabels( const wxArrayInt &Albums );
    guArrayListItems    GetSongsLabels( const wxArrayInt &Songs );
    void                SetArtistsLabels( const wxArrayInt &Artists, const wxArrayInt &Labels );
    void                SetAlbumsLabels( const wxArrayInt &Albums, const wxArrayInt &Labels );
    void                SetSongsLabels( const wxArrayInt &Songs, const wxArrayInt &Labels );

    virtual void        UpdateArtistsLabels( const guArrayListItems &labelsets );
    virtual void        UpdateAlbumsLabels( const guArrayListItems &labelsets );
    virtual void        UpdateSongsLabels( const guArrayListItems &labelsets );
    virtual void        UpdateSongLabel( const guTrack &tracks, const wxString &label, const wxString &newlabel );
    virtual void        UpdateSongsLabel( const guTrackArray &tracks, const wxString &label, const wxString &newlabel );

    void                SetTrackRating( const int songid, const int rating, const bool writetags = false );
    void                SetTracksRating( const wxArrayInt &songids, const int rating, const bool writetags = false );
    void                SetTracksRating( const guTrackArray * tracks, const int rating, const bool writetags = false );

    void                SetTrackPlayCount( const int songid, const int playcount, const bool writetags = false );


    int                 GetYearsSongs( const wxArrayInt &Years, guTrackArray * Songs );
    int                 GetRatingsSongs( const wxArrayInt &Ratings, guTrackArray * Songs );
    int                 GetPlayCountsSongs( const wxArrayInt &PlayCounts, guTrackArray * Songs );
    int                 GetComposersSongs( const wxArrayInt &Composers, guTrackArray * Songs );
    int                 GetAlbumArtistsSongs( const wxArrayInt &albumartists, guTrackArray * tracks );

    wxBitmap *          GetCoverBitmap( const int coverid, const bool thumb = true );
    int                 GetEmptyCovers( guCoverInfos &coverinfos );

    // Smart Playlist and LastFM Panel support functions
    int                 FindArtist( const wxString &Artist );
    int                 FindAlbum( const wxString &Artist, const wxString &Album );
    int                 FindTrack( const wxString &Artist, const wxString &m_Name );
    int                 GetTrackIndex( const int trackid );
    guTrack *           FindSong( const wxString &artist, const wxString &track,
                                  const int filterallow, const int filterdeny );
    int                 FindTrackFile( const wxString &filename, guTrack * track );
    int                 FindTrackId( const int trackid, guTrack * track );

    //
    // AudioScrobbler functions
    //
    bool                    AddCachedPlayedSong( const guCurrentTrack &Song );
    guAS_SubmitInfoArray    GetCachedPlayedSongs( int MaxCount = 10 );
    bool                    DeleteCachedPlayedSongs( const guAS_SubmitInfoArray &SubmitInfo );

    // File Browser related functions
    void                    UpdateTrackFileName( const wxString &oldname, const wxString &newname );
    void                    UpdatePaths( const wxString &oldpath, const wxString &newpath );

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
wxString TextFilterToSQL( const wxArrayString &TeFilters );
wxString LabelFilterToSQL( const wxArrayInt &LaFilters );

// -------------------------------------------------------------------------------- //
wxString inline ArrayIntToStrList( const wxArrayInt &Data )
{
  int index;
  int count;
  wxString RetVal = wxT( "(" );
  count = Data.Count();
  for( index = 0; index < count; index++ )
  {
    RetVal += wxString::Format( wxT( "%u," ), Data[ index ]  );
  }
  if( RetVal.Length() > 1 )
    RetVal = RetVal.RemoveLast() + wxT( ")" );
  else
    RetVal += wxT( ")" );
  return RetVal;
}

// -------------------------------------------------------------------------------- //
wxString inline ArrayToFilter( const wxArrayInt &Filters, const wxString &VarName )
{
  return VarName + wxT( " IN " ) + ArrayIntToStrList( Filters );
}

// -------------------------------------------------------------------------------- //
void inline guDbLibrary::FillTrackFromDb( guTrack * Song, wxSQLite3ResultSet * dbRes )
{
/*
#define GU_TRACKS_QUERYSTR   wxT( "SELECT song_id, song_name, song_genreid, song_genre, song_artistid, song_artist, "\
               "song_albumartistid, song_albumartist, song_composerid, song_composer, song_albumid, song_album, "\
               "song_pathid, song_path, song_filename, song_format, song_disk, song_number, song_year, song_comment, "\
               "song_coverid, song_offset, song_length, song_bitrate, song_rating, song_playcount, "\
               "song_addedtime, song_lastplay, song_filesize "\
               "FROM songs " )
*/
  Song->m_SongId        = dbRes->GetInt( 0 );
  Song->m_SongName      = dbRes->GetString( 1 );
  Song->m_GenreId       = dbRes->GetInt( 2 );
  Song->m_GenreName     = dbRes->GetString( 3 );
  Song->m_ArtistId      = dbRes->GetInt( 4 );
  Song->m_ArtistName    = dbRes->GetString( 5 );
  Song->m_AlbumArtistId = dbRes->GetInt( 6 );
  Song->m_AlbumArtist   = dbRes->GetString( 7 );
  Song->m_ComposerId    = dbRes->GetInt( 8 );
  Song->m_Composer      = dbRes->GetString( 9 );
  Song->m_AlbumId       = dbRes->GetInt( 10 );
  Song->m_AlbumName     = dbRes->GetString( 11 );
  Song->m_PathId        = dbRes->GetInt( 12 );
  Song->m_Path          = dbRes->GetString( 13 );
  Song->m_FileName      = Song->m_Path + dbRes->GetString( 14 );
  Song->m_Format        = dbRes->GetString( 15 );
  Song->m_Disk          = dbRes->GetString( 16 );
  Song->m_Number        = dbRes->GetInt( 17 );
  Song->m_Year          = dbRes->GetInt( 18 );
  Song->m_Comments      = dbRes->GetString( 19 );
  Song->m_CoverId       = dbRes->GetInt( 20 );
  Song->m_Offset        = dbRes->GetInt( 21 );
  Song->m_Length        = dbRes->GetInt( 22 );
  Song->m_Bitrate       = dbRes->GetInt( 23 );
  Song->m_Rating        = dbRes->GetInt( 24 );
  Song->m_PlayCount     = dbRes->GetInt( 25 );
  Song->m_AddedTime     = dbRes->GetInt( 26 );
  Song->m_LastPlay      = dbRes->GetInt( 27 );
  Song->m_FileSize      = dbRes->GetInt( 28 );
  Song->m_MediaViewer   = m_MediaViewer;
}

#endif
// -------------------------------------------------------------------------------- //
