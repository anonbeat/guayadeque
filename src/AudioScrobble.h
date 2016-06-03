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
#ifndef AUDIOSCROBBLE_H
#define AUDIOSCROBBLE_H

#include <wx/wx.h>
#include "EventCommandIds.h"
#include "DbLibrary.h"
#include "Version.h"


#define guAS_PROTOCOL_VERSION       wxT( "1.2.1" )

//#ifdef _DEBUG
//  #define guAS_DEVELOPMENT
//#endif

#ifndef guAS_DEVELOPMENT
    #define guAS_CLIENT_ID          wxT( "gua" ) // Assigned by Adrian Woodhead <adrian@last.fm>
    #define guAS_CLIENT_VERSION     ID_GUAYADEQUE_VERSION
#else
    #define guAS_CLIENT_ID          wxT( "tst" )
    #define guAS_CLIENT_VERSION     wxT( "1.0" )
#endif

#define guLASTFM_POST_SERVER          wxT( "http://post.audioscrobbler.com/" )
#define guLIBREFM_POST_SERVER         wxT( "http://turtle.libre.fm/" )

#define guAS_MIN_PLAYTIME           240
#define guAS_MIN_TRACKLEN           30
#define guAS_SUBMITTRACKS           10
#define guAS_MAX_SUBMITTRACKS       50

#define guAS_SUBMIT_RETRY_TIMEOUT   30000
#define guAS_SUBMIT_TIMEOUT         1000

#define guAS_ERROR_NOERROR          0
#define guAS_ERROR_BANNED           1
#define guAS_ERROR_BADAUTH          2
#define guAS_ERROR_BADTIME          3
#define guAS_ERROR_FAILED           4
#define guAS_ERROR_UNKNOWN          5
#define guAS_ERROR_NOSESSION        6

namespace Guayadeque {

enum guAS_RATING {
    guAS_RATING_NONE = 0,
    guAS_RATING_LOVE,
    guAS_RATING_BAN,
    guAS_RATING_SKIP
};

class guASPlayedThread;

class guCurrentTrack;

// -------------------------------------------------------------------------------- //
class guAudioScrobbleSender
{
  protected :
    guDbLibrary *               m_Db;
    wxString                    m_UserName;
    wxString                    m_Password;
    wxString                    m_ServerUrl;
    wxString                    m_SessionId;
    wxString                    m_NowPlayUrl;
    wxString                    m_SubmitUrl;
    int                         m_ErrorCode;

    wxString                    GetAuthToken( int TimeStamp );
    int                         DoRequest( const wxString &Url, int Timeout = 60, const wxString &PostData = wxEmptyString );
    int                         ProcessError( const wxString &ErrorStr );

    virtual void                ReadUserConfig( void ) {};

  public:
                                guAudioScrobbleSender( guDbLibrary * db, const wxString &serverurl );
                                virtual ~guAudioScrobbleSender();

    void                        SetUserName( const wxString &username ) { m_UserName = username; }
    void                        SetPassword( const wxString &password ) { m_Password = password; }
    bool                        GetSessionId( void );

    bool                        IsOk() { return ( m_ErrorCode == guAS_ERROR_NOERROR ) ||
                                                ( m_ErrorCode == guAS_ERROR_NOSESSION ); }
    int                         GetErrorCode() { return m_ErrorCode; }

    void                        OnConfigUpdated( void );

    bool                        SubmitNowPlaying( const guAS_SubmitInfo * curtrack );
    bool                        SubmitPlayedSongs( const guAS_SubmitInfoArray &playedtracks );
};

// -------------------------------------------------------------------------------- //
class guLastFMAudioScrobble : public guAudioScrobbleSender
{
  protected :
    virtual void                ReadUserConfig( void );

  public :
    guLastFMAudioScrobble( guDbLibrary * db );
    ~guLastFMAudioScrobble();

};

// -------------------------------------------------------------------------------- //
class guLibreFMAudioScrobble : public guAudioScrobbleSender
{
  protected :
    virtual void                ReadUserConfig( void );

  public :
    guLibreFMAudioScrobble( guDbLibrary * db );
    ~guLibreFMAudioScrobble();

};

class guMainFrame;
class guASNowPlayingThread;

// -------------------------------------------------------------------------------- //
class guAudioScrobble : public wxEvtHandler
{
    protected :
        guDbLibrary *               m_Db;
        guMainFrame *               m_MainFrame;

        guLastFMAudioScrobble *     m_LastFMAudioScrobble;
        guLibreFMAudioScrobble *    m_LibreFMAudioScrobble;

        guASPlayedThread *          m_PlayedThread;
        bool                        m_PendingNowPlaying;
        guAS_SubmitInfo *           m_NowPlayingInfo;
        guASNowPlayingThread *      m_NowPlayingThread;
        wxMutex                     m_NowPlayingInfoMutex;

    public :
        guAudioScrobble( guDbLibrary * db );
        ~guAudioScrobble();

    bool                        SubmitNowPlaying( const guAS_SubmitInfo * curtrack );
    bool                        SubmitPlayedSongs( const guAS_SubmitInfoArray &playedtracks );

    void                        SendNowPlayingTrack( const guCurrentTrack &track );
    void                        SendPlayedTrack( const guCurrentTrack &track );
    void                        EndPlayedThread( void );
    void                        EndNowPlayingThread( void );

    void                        OnConfigUpdated( void );
    bool                        IsOk( void );
};

// -------------------------------------------------------------------------------- //
class guASNowPlayingThread : public wxThread
{
  private:
    guAudioScrobble *           m_AudioScrobble;
    const guAS_SubmitInfo *     m_CurrentSong;

  public:
    guASNowPlayingThread( guAudioScrobble * audioscrobble, const guAS_SubmitInfo * playingsong );
    ~guASNowPlayingThread();

    virtual ExitCode Entry();

};

// -------------------------------------------------------------------------------- //
class guASPlayedThread : public wxThread
{
  private:
    guAudioScrobble *       m_AudioScrobble;
    guDbLibrary *           m_Db;

  public:
    guASPlayedThread( guAudioScrobble * audioscrobble, guDbLibrary * db );
    ~guASPlayedThread();

    virtual ExitCode Entry();
};

}

#endif
// -------------------------------------------------------------------------------- //
