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
#ifndef AUDIOSCROBBLE_H
#define AUDIOSCROBBLE_H

#include <wx/wx.h>
#include "DbLibrary.h"


#define guAS_PROTOCOL_VERSION       wxT( "1.2.1" )

//#ifdef _DEBUG
//  #define guAS_DEVELOPMENT
//#endif

#ifndef guAS_DEVELOPMENT
    #define guAS_CLIENT_ID          wxT( "gua" ) // Assigned by Adrian Woodhead <adrian@last.fm>
    #define guAS_CLIENT_VERSION     wxT( "0.01" )
#else
    #define guAS_CLIENT_ID          wxT( "tst" )
    #define guAS_CLIENT_VERSION     wxT( "1.0" )
#endif

#define guAS_HANDSHAKE_URL          wxT( "http://post.audioscrobbler.com/" )

#define guAS_MIN_PLAYTIME           240
#define guAS_MIN_TRACKLEN           30
#define guAS_SUBMITTRACKS           10
#define guAS_MAX_SUBMITTRACKS       50

#define guAS_SUBMIT_RETRY_TIMEOUT   30000
#define guAS_SUBMIT_TIMEOUT         120000

#define guAS_ERROR_NOERROR          0
#define guAS_ERROR_BANNED           1
#define guAS_ERROR_BADAUTH          2
#define guAS_ERROR_BADTIME          3
#define guAS_ERROR_FAILED           4
#define guAS_ERROR_UNKNOWN          5
#define guAS_ERROR_NOSESSION        6

class guASPlayedThread;

// -------------------------------------------------------------------------------- //
class guAudioScrobble
{
  private:
    DbLibrary *                 m_Db;
    wxString                    m_UserName;
    wxString                    m_Password;
    wxString                    m_SessionId;
    wxString                    m_NowPlayUrl;
    wxString                    m_SubmitUrl;
    int                         m_ErrorCode;
    guASPlayedThread *          m_SubmitPlayedSongsThread;

    wxString                    GetAuthToken( int TimeStamp );
    int                         DoRequest( const wxString &Url, int Timeout = 60, const wxString &PostData = wxEmptyString );
    int                         ProcessError( const wxString &ErrorStr );

  public:
                                guAudioScrobble( DbLibrary * NewDb );
                                ~guAudioScrobble();

    void                        SetUserName( const wxString &username ) { m_UserName = username; }
    void                        SetPassword( const wxString &password ) { m_Password = password; }
    bool                        GetSessionId( void );

    bool                        AddSongInfo( const guTrack &SubmitInfo );

    //
    void                        SetNowPlayingSong( const guAS_SubmitInfo &PlayingSong );
    bool                        SubmitNowPlaying( const guAS_SubmitInfo &PlayingSong );
    bool                        SubmitPlayedSongs( const guAS_SubmitInfoArray &PlayedSongs );
    bool                        IsOk() { return m_ErrorCode == guAS_ERROR_NOERROR; };
    int                         GetErrorCode() { return m_ErrorCode; };
    void                        EndSubmitThread();

};

// -------------------------------------------------------------------------------- //
class guASNowPlayingThread : public wxThread
{
  private:
    guAudioScrobble * m_AudioScrobble;
    guAS_SubmitInfo   m_CurrentSong;

  public:
    guASNowPlayingThread( guAudioScrobble * audioscrobble, const guAS_SubmitInfo &playingsong );
    ~guASNowPlayingThread();

    virtual ExitCode Entry();

};

// -------------------------------------------------------------------------------- //
class guASPlayedThread : public wxThread
{
  private:
    guAudioScrobble * m_AudioScrobble;
    DbLibrary * m_Db;

  public:
    guASPlayedThread( guAudioScrobble * audioscrobble, DbLibrary * db );
    ~guASPlayedThread();

    virtual ExitCode Entry();

};

#endif
// -------------------------------------------------------------------------------- //
