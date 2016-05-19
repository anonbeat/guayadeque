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
#ifndef LASTFM_H
#define LASTFM_H

#include "DbLibrary.h"
#include "MD5.h"

#define LASTFM_API_KEY          wxT( "96a881180c49ba8ec586675172c3ef36" )
#define LASTFM_SHARED_SECRET    wxT( "cecb145e943f307c7e7488e0ff6ebbbe" )
#define LASTFM_API_ROOT         wxT( "http://ws.audioscrobbler.com/2.0/" )
#define LASTFM_POST_PATH        wxT( "/2.0/" )
#define LASTFM_AUTH_ROOT        wxT( "http://www.last.fm/api/auth/" )
//#define LASTFM_HOST             wxT( "ws.audioscrobbler.com" )

// -------------------------------------------------------------------------------- //
class guLastFMRequest
{
  private:
    wxSortedArrayString m_Args;
    //wxString Method;
    wxString GetSign();

  public:
    guLastFMRequest();
    ~guLastFMRequest();

    wxString DoRequest( const bool AddSign = true, const bool IsGetAction = true );
    void AddArgument( const wxString &ArgName, const wxString &ArgValue, bool Filter );
    void AddArgument( const wxString &ArgName, const wxString &ArgValue );
    void SetMethod( const wxString &MethodName );
};

// -------------------------------------------------------------------------------- //
class guArtistInfo {
  public:
    wxString        m_Name;
    wxString        m_Url;
    wxString        m_ImageLink;
    wxArrayString   m_Tags;
    wxString        m_BioSummary;
    wxString        m_BioContent;
};

// -------------------------------------------------------------------------------- //
class guAlbumInfo {
  public:
    wxString m_Name;
    wxString m_Artist;
    wxString m_Url;
    wxString m_ReleaseDate;
    wxString m_ImageLink;
    wxString m_Tags;
    wxString m_Rank;
};
WX_DECLARE_OBJARRAY(guAlbumInfo, guAlbumInfoArray);

// -------------------------------------------------------------------------------- //
class guTrackInfo {
  public:
    wxString        m_TrackName;
    wxString        m_ArtistName;
    wxString        m_AlbumName;
    wxString        m_Url;
    wxArrayString   m_TopTags;
    wxString        m_Summary;
    wxString        m_Content;
};

// -------------------------------------------------------------------------------- //
class guTopTrackInfo {
  public:
    wxString        m_TrackName;
    int             m_PlayCount;
    int             m_Listeners;
    wxString        m_ArtistName;
    wxString        m_Url;
    wxString        m_ImageLink;
};
WX_DECLARE_OBJARRAY(guTopTrackInfo, guTopTrackInfoArray);

// -------------------------------------------------------------------------------- //
class guSimilarArtistInfo {
  public:
    wxString m_Name;
    wxString m_Match;
    wxString m_Url;
    wxString m_ImageLink;

    guSimilarArtistInfo() {};
    guSimilarArtistInfo( guSimilarArtistInfo * info )
    {
        wxASSERT( info );
        m_Name = wxString( info->m_Name.c_str() );
        m_Match = wxString( info->m_Match.c_str() );
        m_Url = wxString( info->m_Url.c_str() );
        m_ImageLink = wxString( info->m_ImageLink.c_str() );
    }

};
WX_DECLARE_OBJARRAY(guSimilarArtistInfo, guSimilarArtistInfoArray);


// -------------------------------------------------------------------------------- //
class guSimilarTrackInfo {
  public:
    wxString m_TrackName;
    wxString m_ArtistName;
    wxString m_Match;
    wxString m_Url;
    wxString m_ImageLink;

    guSimilarTrackInfo() {};
    guSimilarTrackInfo( guSimilarTrackInfo * info )
    {
        wxASSERT( info );
        m_TrackName = wxString( info->m_TrackName.c_str() );
        m_ArtistName = wxString( info->m_ArtistName.c_str() );
        m_Match = wxString( info->m_Match.c_str() );
        m_Url = wxString( info->m_Url.c_str() );
        m_ImageLink = wxString( info->m_ImageLink.c_str() );
    }
};
WX_DECLARE_OBJARRAY(guSimilarTrackInfo,guSimilarTrackInfoArray);

// -------------------------------------------------------------------------------- //
class guEventInfo {
  public :
    int             m_Id;
    wxString        m_Title;
    wxArrayString   m_Artists;
    wxString        m_LocationName;
    wxString        m_LocationCity;
    wxString        m_LocationCountry;
    wxString        m_LocationStreet;
    wxString        m_LocationZipCode;
    wxString        m_LocationGeoLat;
    wxString        m_LocationGeoLong;
    wxString        m_LocationTimeZone;
    wxString        m_LocationLink;
    wxString        m_Date;
    wxString        m_Time;
    wxString        m_Description;
    wxString        m_ImageLink;
    wxString        m_Url;
};
WX_DECLARE_OBJARRAY(guEventInfo,guEventInfoArray);

// -------------------------------------------------------------------------------- //
class guLastFM
{
  private:
    guTrackArray    m_Songs;
    wxString        m_UserName;
    wxString        m_Password;
    wxString        m_AuthToken;
    wxString        m_AuthSession;
    wxString        m_AuthKey;
    wxString        m_Language;
    long            m_ErrorCode;

  public:
    guLastFM();
    ~guLastFM();

    void                        SetUserName( const wxString &NewUser ) { m_UserName = NewUser; };
    void                        SetPassword( const wxString &NewPass ) { m_Password = NewPass; };
    void                        SetLanguage( const wxString &lang ) { m_Language = lang; };
    wxString                    GetAuthSession();
    wxString                    GetAuthURL();
    bool                        DoAuthentication();
    int                         GetLastError() { return m_ErrorCode; };
    bool                        IsOk();

    wxString                    AuthGetSession();
    wxString                    AuthGetToken();

    // Album Methods
    bool                        AlbumAddTags( const wxString &Artist, const wxString &Album, const wxString &Tags );
    guAlbumInfo                 AlbumGetInfo( const wxString &Artist, const wxString &Album );
    wxArrayString               AlbumGetTags( const wxString &Artist, const wxString &Album );
    bool                        AlbumRemoveTag( const wxString &Artist, const wxString &Album, const wxString &Tag );
    // Artist Methods
    bool                        ArtistAddTags( const wxString &Artist, const wxString &Tags );
    guArtistInfo                ArtistGetInfo( const wxString &Artist );
    guSimilarArtistInfoArray    ArtistGetSimilar( const wxString &Artist );
    wxArrayString               ArtistGetTags( const wxString &Artist );
    guAlbumInfoArray            ArtistGetTopAlbums( const wxString &Artist );
    wxArrayString               ArtistGetTopTags( const wxString &Artist );
    guTopTrackInfoArray         ArtistGetTopTracks( const wxString &Artist );
    guEventInfoArray            ArtistGetEvents( const wxString &Artist );


    // Track Methods
    guTrackInfo                 TrackGetInfo( const wxString &Artist, const wxString &Track );
    guSimilarTrackInfoArray     TrackGetSimilar( const wxString &Artist, const wxString &Track );
    wxArrayString               TrackGetTags( const wxString &Artist, const wxString &Track );
    wxArrayString               TrackGetTopTags( const wxString &Artist, const wxString &Track );
    bool                        TrackRemoveTag( const wxString &Artist, const wxString &Track, const wxString &Tag );
    bool                        TrackLove( const wxString &artist, const wxString &title );
    bool                        TrackBan( const wxString &artist, const wxString &title );

};


#endif
// -------------------------------------------------------------------------------- //
