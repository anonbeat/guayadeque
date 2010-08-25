// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2010 J.Rios
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
#include "AudioScrobble.h"
#include "MD5.h"
#include "Config.h"
#include "Utils.h"
#include "PlayerPanel.h"

#include <wx/curl/http.h>
#include <wx/tokenzr.h>

// -------------------------------------------------------------------------------- //
guAudioScrobbleSender::guAudioScrobbleSender( guDbLibrary * db, const wxString &serverurl )
{
    m_Db            = db;
    m_ServerUrl     = serverurl;

    ReadUserConfig();

    m_SessionId = wxEmptyString;
    m_NowPlayUrl = wxEmptyString;
    m_SubmitUrl = wxEmptyString;
    m_ErrorCode = guAS_ERROR_NOSESSION;

}

// -------------------------------------------------------------------------------- //
guAudioScrobbleSender::~guAudioScrobbleSender()
{
}

// -------------------------------------------------------------------------------- //
void guAudioScrobbleSender::OnConfigUpdated( void )
{
    ReadUserConfig();
}

// -------------------------------------------------------------------------------- //
int guAudioScrobbleSender::ProcessError( const wxString &ErrorStr )
{
    if( ErrorStr.Contains( wxT( "BANNED" ) ) )
    {
        m_ErrorCode = guAS_ERROR_BANNED;
        guLogError( wxT( "This Client version have been banned. You should upgrade to the newest version" ) );
        m_SessionId = wxEmptyString;
    }
    else if( ErrorStr.Contains( wxT( "BADAUTH" ) ) )
    {
        m_ErrorCode = guAS_ERROR_BADAUTH;
        guLogError( wxT( "The username or password is incorrect" ) );
        m_SessionId = wxEmptyString;
    }
    else if( ErrorStr.Contains( wxT( "BADTIME" ) ) )
    {
        m_ErrorCode = guAS_ERROR_BADTIME;
        guLogError( wxT( "The system clock is incorrect. Please adjuts time/date and try again." ) );
    }
    else if( ErrorStr.Contains( wxT( "FAILED" ) ) )
    {
        m_ErrorCode = guAS_ERROR_FAILED;
        guLogError( wxT( "Server Error when connecting to LastFM server : " ) + ErrorStr );
    }
    else
    {
        m_ErrorCode = guAS_ERROR_UNKNOWN;
        m_SessionId = wxEmptyString;
        guLogError( wxT( "Unknown Error autenticating to LastFM server " ) + ErrorStr );
    }

    return m_ErrorCode;
}

// -------------------------------------------------------------------------------- //
bool guAudioScrobbleSender::GetSessionId( void )
{
    //guLogMessage( wxT( "guAudioScrobbleSender:GetSessionId" ) );
    //http://post.audioscrobbler.com/?hs=true&p=1.2.1&c=<client-id>&v=<client-ver>&u=<user>&t=<timestamp>&a=<auth>
    wxString Content;
    long LocalTime = wxGetUTCTime();
    wxString AS_Url = m_ServerUrl + wxT( "?hs=true" )\
                      wxT( "&p=" ) guAS_PROTOCOL_VERSION\
                      wxT( "&c=" ) guAS_CLIENT_ID\
                      wxT( "&v=" ) guAS_CLIENT_VERSION\
                      wxT( "&u=" ) + m_UserName +
                      wxT( "&t=" ) + wxString::Format( wxT( "%u" ), LocalTime ) +
                      wxT( "&a=" ) + GetAuthToken( LocalTime );

    //guLogMessage( wxT( "AudioScrobble:GetSesionId : " ) + AS_Url );
    char * Buffer = NULL;
    wxCurlHTTP http;
    http.AddHeader( wxT( "User-Agent: " ) guDEFAULT_BROWSER_USER_AGENT );
    http.AddHeader( wxT( "Accept: text/html" ) );
    http.AddHeader( wxT( "Accept-Charset: utf-8" ) );
    http.SetOpt( CURLOPT_FOLLOWLOCATION, 1 );
    http.Get( Buffer, AS_Url );
    if( Buffer )
    {
        Content = wxString( Buffer, wxConvUTF8 );
        free( Buffer );

        if( !Content.IsEmpty() )
        {
            //guLogMessage( wxT( "AudioScrobble:Response : " ) + Content );
            wxArrayString Response = wxStringTokenize( Content );
            if( Response.Count() && !Response[ 0 ].IsEmpty() )
            {
                if( Response[ 0 ] == wxT( "OK" ) && Response.Count() == 4 )
                {
                    // All went Ok
                    m_SessionId = Response[ 1 ];
                    m_NowPlayUrl = Response[ 2 ];
                    m_SubmitUrl = Response[ 3 ];
                    //guLogMessage( wxT( "SessionId  : " ) + SessionId );
                    //guLogMessage( wxT( "NowPlayUrl : " ) + NowPlayUrl );
                    //guLogMessage( wxT( "SubmitUrl  : " ) + SubmitUrl );
                    m_ErrorCode = guAS_ERROR_NOERROR;
                    guLogMessage( wxT( "Loged in to AudioScrobble service." ) );
                    //return !SessionId.IsEmpty() && !SubmitUrl.IsEmpty() && !NowPlayUrl.IsEmpty();

                }
                else
                {
                    ProcessError( Content );
                    guLogError( wxT( "Could not get a LastFM:AudioScrobble SessionId" ) );
                }
            }
        }
    }
    return m_ErrorCode == guAS_ERROR_NOERROR;
}

// -------------------------------------------------------------------------------- //
wxString guAudioScrobbleSender::GetAuthToken( int TimeStamp )
{
    guMD5 md5;
    return md5.MD5( m_Password + wxString::Format( wxT( "%u" ), TimeStamp ) );
}

// -------------------------------------------------------------------------------- //
bool guAudioScrobbleSender::SubmitPlayedSongs( const guAS_SubmitInfoArray &PlayedSongs )
{
    wxString    PostData;
    wxString    Content;
    int         index;
    int         count;
    wxString    Artist;
    wxString    Album;
    wxString    Track;
    wxCurlHTTP  http;

    /* Info from MediaPortal as seems more complete than the LastFM/api v1.2.1 protocol info
        s=<sessionID>            The Session ID string as returned by the handshake. Required.
        a[0]=<artist>            The artist name. Required.
        t[0]=<track>             The track title. Required.
        i[0]=<time>              The time the track started playing, in UNIX timestamp format (integer number of seconds since 00:00:00, January 1st 1970 UTC). This must be in the UTC time zone, and is required.
        o[0]=<source>            The source of the track. Required, must be one of the following codes:

            P = Chosen by the user, no shuffle
            S = Chosen by the user, shuffle enabled
            T = Chosen by the user, unknown shuffle status (e.g. iPod)
            R = Non-personalised broadcast (e.g. Shoutcast, BBC Radio 1)
            E = Personalised recommendation except Last.fm (e.g. Pandora, Launchcast)
            L = Last.fm (any mode)
            U = Source unknown

        r[0]=<rating>

            L = Love
            B = Ban
            S = Skip (only if source=L)

        b[0]=<album>             The album title, or empty if not known.
        l[0]=<secs>              The length of the track in seconds, or empty if not known.
        n[0]=<tracknumber>       The position of the track on the album, or empty if not known.
        m[0]=<mb-trackid>        The MusicBrainz Track ID, or empty if not known.
    */


    if( ( count = PlayedSongs.Count() ) )
    {
        if( m_SessionId.IsEmpty() && !GetSessionId() )
            return false;

        PostData = wxT( "s=" ) + m_SessionId + wxT( "&" );
        for( index = 0; index < count; index++ )
        {
            Artist = guURLEncode( PlayedSongs[ index ].m_ArtistName );
            Track  = guURLEncode( PlayedSongs[ index ].m_TrackName );
            Album  = guURLEncode( PlayedSongs[ index ].m_AlbumName );
            //
            PostData += wxString::Format( wxT( "a[%u]=%s&t[%u]=%s&i[%u]=%u&o[%u]=%c&r[%u]=%c&l[%u]=%u&b[%u]=%s&n[%u]=%s&m[%u]=&" ),
                                index, Artist.c_str(),
                                index, Track.c_str(),
                                index, PlayedSongs[ index ].m_PlayedTime,
                                index, PlayedSongs[ index ].m_Source,
                                index, PlayedSongs[ index ].m_Rating,
                                index, PlayedSongs[ index ].m_TrackLen,
                                index, Album.c_str(),
                                index, ( PlayedSongs[ index ].m_TrackNum > 0 ) ?
                                           wxString::Format( wxT( "%u" ), PlayedSongs[ index ].m_TrackNum ).c_str() :
                                           wxEmptyString,
                                index );
        }
        PostData.RemoveLast( 1 ); // we remove the last & added

        //guLogMessage( wxT( "AudioScrobble::Played : " ) + PostData );
        http.AddHeader( wxT( "Content-Type: application/x-www-form-urlencoded" ) );
        http.SetOpt( CURLOPT_FOLLOWLOCATION, 1 );
        if( http.Post( wxCURL_STRING2BUF( PostData ), PostData.Length(), m_SubmitUrl ) )
        {
            Content = http.GetResponseBody();
            if( !Content.IsEmpty() )
            {
                //guLogMessage( wxT( "AudioScrobble::Response : " ) + Content );
                if( Content.Contains( wxT( "OK" ) ) )
                {
                    m_ErrorCode = guAS_ERROR_NOERROR;

                    return true;
                }
                else
                    ProcessError( Content );
            }
        }
    }
    return false;
}

// -------------------------------------------------------------------------------- //
bool guAudioScrobbleSender::SubmitNowPlaying( const guAS_SubmitInfo * cursong )
{
    wxCurlHTTP  http;
    wxString    PostData;
    wxString    Content;
    wxString    Artist;
    wxString    Album;
    wxString    Track;

    // If we have not a session id try to get it and abort if not valid
    if( m_SessionId.IsEmpty() && !GetSessionId() )
        return false;

    PostData = wxT( "s=" ) + m_SessionId + wxT( "&" );
    Artist = guURLEncode( cursong->m_ArtistName );
    Track  = guURLEncode( cursong->m_TrackName );
    Album  = guURLEncode( cursong->m_AlbumName );
    //
    PostData += wxString::Format( wxT( "a=%s&t=%s&l=%u&b=%s&n=%s&m=" ),
                        Artist.c_str(),
                        Track.c_str(),
                        cursong->m_TrackLen,
                        Album.c_str(),
                        ( cursong->m_TrackNum > 0 ) ?
                                   wxString::Format( wxT( "%u" ), cursong->m_TrackNum ).c_str() :
                                   wxEmptyString
                        );

    //guLogMessage( wxT( "AudioScrobble::NowPlaying : " ) + m_NowPlayUrl + PostData );

    http.AddHeader( wxT( "Content-Type: application/x-www-form-urlencoded" ) );
    http.SetOpt( CURLOPT_FOLLOWLOCATION, 1 );
    if( http.Post( wxCURL_STRING2BUF( PostData ), PostData.Length(), m_NowPlayUrl ) )
    {
        Content = http.GetResponseBody();
        //guLogMessage( wxT( "%s" ), Content.c_str() );
        if( !Content.IsEmpty() )
        {
            //guLogMessage( wxT( "AudioScrobble::Response : " ) + Content );
            if( Content.Contains( wxT( "OK" ) ) )
            {
                m_ErrorCode = guAS_ERROR_NOERROR;
                return true;
            }
            else
                ProcessError( Content );
        }
    }
    else
    {
        guLogMessage( wxT( "Error submitting the data to the scrobble server" ) );
    }
    return false;
}




// -------------------------------------------------------------------------------- //
guLastFMAudioScrobble::guLastFMAudioScrobble( guDbLibrary * db ) :
    guAudioScrobbleSender( db, guLASTFM_POST_SERVER )
{
    ReadUserConfig();
}

// -------------------------------------------------------------------------------- //
guLastFMAudioScrobble::~guLastFMAudioScrobble()
{
}

// -------------------------------------------------------------------------------- //
void guLastFMAudioScrobble::ReadUserConfig( void )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        m_UserName = Config->ReadStr( wxT( "UserName" ), wxEmptyString, wxT( "LastFM" ) );
        m_Password = Config->ReadStr( wxT( "Password" ), wxEmptyString, wxT( "LastFM" ) );
    }
}




// -------------------------------------------------------------------------------- //
guLibreFMAudioScrobble::guLibreFMAudioScrobble( guDbLibrary * db ) :
    guAudioScrobbleSender( db, guLIBREFM_POST_SERVER )
{
    ReadUserConfig();
}

// -------------------------------------------------------------------------------- //
guLibreFMAudioScrobble::~guLibreFMAudioScrobble()
{
}

// -------------------------------------------------------------------------------- //
void guLibreFMAudioScrobble::ReadUserConfig( void )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        m_UserName = Config->ReadStr( wxT( "UserName" ), wxEmptyString, wxT( "LibreFM" ) );
        m_Password = Config->ReadStr( wxT( "Password" ), wxEmptyString, wxT( "LibreFM" ) );
    }
}




// -------------------------------------------------------------------------------- //
guAudioScrobble::guAudioScrobble( guDbLibrary * db )
{
    m_Db = db;
    m_LastFMAudioScrobble = NULL;
    m_LibreFMAudioScrobble = NULL;
    m_PlayedThread = NULL;
    m_NowPlayingInfo = NULL;

    guConfig * Config = ( guConfig * ) guConfig::Get();

    if( Config->ReadBool( wxT( "SubmitEnabled" ), false, wxT( "LastFM" ) ) )
    {
        m_LastFMAudioScrobble = new guLastFMAudioScrobble( db );
    }

    if( Config->ReadBool( wxT( "SubmitEnabled" ), false, wxT( "LibreFM" ) ) )
    {
        m_LibreFMAudioScrobble = new guLibreFMAudioScrobble( db );
    }

    // Update the MainFrame AudioScrobble Status
    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_AUDIOSCROBBLE_UPDATED );
    event.SetEventObject( ( wxObject * ) this );
    event.SetInt( 0 );
    wxPostEvent( wxTheApp->GetTopWindow(), event );

}


// -------------------------------------------------------------------------------- //
guAudioScrobble::~guAudioScrobble()
{
    if( m_PlayedThread )
    {
        m_PlayedThread->Pause();
        m_PlayedThread->Delete();
    }
}

// -------------------------------------------------------------------------------- //
bool guAudioScrobble::SubmitNowPlaying( const guAS_SubmitInfo * curtrack )
{
    //guLogMessage( wxT( "guAudioScrobbler:SubmitNowPlaying: %s" ), curtrack->m_TrackName.c_str() );
    if( m_LastFMAudioScrobble )
    {
        m_LastFMAudioScrobble->SubmitNowPlaying( curtrack );
    }

    if( m_LibreFMAudioScrobble )
    {
        m_LibreFMAudioScrobble->SubmitNowPlaying( curtrack );
    }

    int HasError = ( m_LastFMAudioScrobble && m_LastFMAudioScrobble->GetErrorCode() ) ||
                   ( m_LibreFMAudioScrobble && m_LibreFMAudioScrobble->GetErrorCode() );

    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_AUDIOSCROBBLE_UPDATED );
    event.SetEventObject( ( wxObject * ) this );
    event.SetInt( HasError );
    wxPostEvent( wxTheApp->GetTopWindow(), event );

    return !HasError;
}

// -------------------------------------------------------------------------------- //
bool guAudioScrobble::SubmitPlayedSongs( const guAS_SubmitInfoArray &playedtracks )
{
    //guLogMessage( wxT( "guAudioScrobbler:SubmitPlayedSongs" ) );
    if( m_LastFMAudioScrobble )
    {
        m_LastFMAudioScrobble->SubmitPlayedSongs( playedtracks );
    }

    if( m_LibreFMAudioScrobble )
    {
        m_LibreFMAudioScrobble->SubmitPlayedSongs( playedtracks );
    }

    int HasError = ( m_LastFMAudioScrobble && m_LastFMAudioScrobble->GetErrorCode() ) ||
                   ( m_LibreFMAudioScrobble && m_LibreFMAudioScrobble->GetErrorCode() );

    wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_AUDIOSCROBBLE_UPDATED );
    event.SetEventObject( ( wxObject * ) this );
    event.SetInt( HasError );
    wxPostEvent( wxTheApp->GetTopWindow(), event );

    return !HasError;
}

// -------------------------------------------------------------------------------- //
void guAudioScrobble::SendPlayedTrack( const guCurrentTrack &track )
{
    if( !m_Db->AddCachedPlayedSong( track ) )
        guLogError( wxT( "Could not add Song to CachedSongs Database" ) );

    if( !m_PlayedThread )
    {
        m_PlayedThread = new guASPlayedThread( this, m_Db );
        if( !m_PlayedThread )
            guLogError( wxT( "Could no create the AudioScrobble Played thread" ) );
    }
}

// -------------------------------------------------------------------------------- //
void guAudioScrobble::SendNowPlayingTrack( const guCurrentTrack &track )
{
    wxMutexLocker Lock( m_NowPlayingInfoMutex );

    if( m_NowPlayingInfo )
    {
        delete m_NowPlayingInfo;
    }
    m_NowPlayingInfo = new guAS_SubmitInfo();

    m_NowPlayingInfo->m_ArtistName = track.m_ArtistName;
    m_NowPlayingInfo->m_AlbumName  = track.m_AlbumName;
    m_NowPlayingInfo->m_TrackName  = track.m_SongName;
    m_NowPlayingInfo->m_TrackLen   = track.m_Length;
    m_NowPlayingInfo->m_TrackNum   = track.m_Number;

    guAS_SubmitInfoArray    SubmitInfo;
    SubmitInfo = m_Db->GetCachedPlayedSongs( guAS_SUBMITTRACKS );
    if( !SubmitInfo.Count() )
    {
        guASNowPlayingThread * NowPlayingThread  = new guASNowPlayingThread( this, m_NowPlayingInfo );
        if( !NowPlayingThread )
            guLogError( wxT( "Could no create the AudioScrobble NowPlaying thread" ) );

        m_NowPlayingInfo = NULL;
    }
}

// -------------------------------------------------------------------------------- //
void guAudioScrobble::EndPlayedThread( void )
{
    wxMutexLocker Lock( m_NowPlayingInfoMutex );

    m_PlayedThread = NULL;
    if( m_NowPlayingInfo )
    {
        guASNowPlayingThread * NowPlayingThread  = new guASNowPlayingThread( this, m_NowPlayingInfo );
        if( !NowPlayingThread )
            guLogError( wxT( "Could no create the AudioScrobble NowPlaying thread" ) );
        m_NowPlayingInfo = NULL;
    }
}

// -------------------------------------------------------------------------------- //
void guAudioScrobble::OnConfigUpdated( void )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();

    if( Config->ReadBool( wxT( "SubmitEnabled" ), false, wxT( "LastFM" ) ) )
    {
        if( !m_LastFMAudioScrobble )
            m_LastFMAudioScrobble = new guLastFMAudioScrobble( m_Db );
    }
    else
    {
        if( m_LastFMAudioScrobble )
        {
            delete m_LastFMAudioScrobble;
            m_LastFMAudioScrobble = NULL;
        }
    }

    if( Config->ReadBool( wxT( "SubmitEnabled" ), false, wxT( "LibreFM" ) ) )
    {
        if( !m_LibreFMAudioScrobble )
            m_LibreFMAudioScrobble = new guLibreFMAudioScrobble( m_Db );
    }
    else
    {
        if( m_LibreFMAudioScrobble )
        {
            delete m_LibreFMAudioScrobble;
            m_LibreFMAudioScrobble = NULL;
        }
    }

    if( m_LastFMAudioScrobble )
    {
        m_LastFMAudioScrobble->OnConfigUpdated();
    }

    if( m_LibreFMAudioScrobble )
    {
        m_LibreFMAudioScrobble->OnConfigUpdated();
    }
}

// -------------------------------------------------------------------------------- //
bool guAudioScrobble::IsOk( void )
{
    return ( m_LastFMAudioScrobble && m_LastFMAudioScrobble->IsOk() ) ||
           ( m_LibreFMAudioScrobble && m_LibreFMAudioScrobble->IsOk() );
}


// -------------------------------------------------------------------------------- //
// guASNowPlayingThread
// -------------------------------------------------------------------------------- //
guASNowPlayingThread::guASNowPlayingThread( guAudioScrobble * audioscrobble,
                                               const guAS_SubmitInfo * currentsong )
{
    m_AudioScrobble = audioscrobble;
    m_CurrentSong = currentsong;

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guASNowPlayingThread::~guASNowPlayingThread()
{
    if( m_CurrentSong )
        delete m_CurrentSong;
}

// -------------------------------------------------------------------------------- //
guASNowPlayingThread::ExitCode guASNowPlayingThread::Entry()
{
    int FailCnt = 0;

    // While the Thread have not been destroyed
    while( !TestDestroy() )
    {
        // Send the info to lastfm
        if( m_AudioScrobble->SubmitNowPlaying( m_CurrentSong ) )
            break;
        FailCnt++;
        if( FailCnt > 2 )
        {
//            guLogError( wxT( "Reached max number of fail retry submitting to lastfm AudioScrobble service." ) );
//            FailCnt = 0;
//            while( !m_AudioScrobble->GetSessionId() && !TestDestroy() )
//            {
//                Sleep( guAS_SUBMIT_RETRY_TIMEOUT );
//                FailCnt++;
//                if( FailCnt > 2 )
//                    return 0;
//            }
            break;
        }
        // If have not been destroyed wait 2 mins between submits.
        if( !TestDestroy() )
            Sleep( guAS_SUBMIT_RETRY_TIMEOUT ); // Wait 10 secs timeout between each try
    }
    return 0;
}




// -------------------------------------------------------------------------------- //
// guASPlayedThread
// -------------------------------------------------------------------------------- //
guASPlayedThread::guASPlayedThread( guAudioScrobble * audioscrobble, guDbLibrary * db )
{
    m_AudioScrobble = audioscrobble;
    m_Db = db;

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}

// -------------------------------------------------------------------------------- //
guASPlayedThread::~guASPlayedThread()
{
    if( !TestDestroy() )
    {
        if( m_AudioScrobble )
            m_AudioScrobble->EndPlayedThread();
    }
}

// -------------------------------------------------------------------------------- //
guASPlayedThread::ExitCode guASPlayedThread::Entry()
{
    guAS_SubmitInfoArray    SubmitInfo;
    bool                    Submit = false;
    int                     FailCnt;

    // While the Thread have not been destroyed
    while( !TestDestroy() )
    {
        // Query the Database if there are cached played songs to submit
        SubmitInfo = m_Db->GetCachedPlayedSongs( guAS_SUBMITTRACKS );
        // If we have records to send
        if( SubmitInfo.Count() )
        {
            //guLogMessage( wxT( "**** Trying a AudioScrobble Submit ****" ) );
            // We attempt to submit the info every 30 secs
            FailCnt = 0;
            while( !TestDestroy() && !( Submit = m_AudioScrobble->SubmitPlayedSongs( SubmitInfo ) ) )
            {
//                // Update the MainFrame AudioScrobble Status
//                wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_AUDIOSCROBBLE_UPDATED );
//                event.SetEventObject( ( wxObject * ) m_AudioScrobble );
//                event.SetInt( m_AudioScrobble->GetErrorCode() );
//                wxPostEvent( wxTheApp->GetTopWindow(), event );
                //
//                if( m_AudioScrobble->GetErrorCode() != guAS_ERROR_NOERROR )
//                {
//                    FailCnt++;
//                    if( FailCnt > 2 )
//                    {
//                        guLogError( wxT( "Reached max number of fail retry submitting to lastfm AudioScrobble service." ) );
//                        FailCnt = 0;
//                        while( !m_AudioScrobble->GetSessionId() && !TestDestroy() )
//                        {
//                            Sleep( guAS_SUBMIT_RETRY_TIMEOUT );
//                            FailCnt++;
//                            if( FailCnt > 2 )
//                                return 0;
//                        }
//                    }
//                }
//                else
//                    return 0;
                if( FailCnt++ > 2 )
                    break;
                Sleep( guAS_SUBMIT_RETRY_TIMEOUT ); // Wait 30 Secs between submit attempts
            }
            // if the submit was ok then delete the songs from cache
            if( Submit )
            {
                m_Db->DeleteCachedPlayedSongs( SubmitInfo );
            }
        }
        else
        {
            break;
        }
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
