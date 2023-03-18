// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2023 J.Rios anonbeat@gmail.com
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
//    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
//    Boston, MA 02110-1301 USA.
//
//    http://www.gnu.org/copyleft/gpl.html
//
// -------------------------------------------------------------------------------- //
#include "SmartMode.h"

#include "EventCommandIds.h"
#include "MainFrame.h"
#include "MediaViewer.h"
#include "PlayListAppend.h"

namespace Guayadeque {

#define guSMARTMODE_MIN_TRACK_MATCH         0.1
#define guSMARTMODE_MIN_ARTIST_MATCH        0.2

// -------------------------------------------------------------------------------- //
guSmartModeThread::guSmartModeThread( guDbLibrary * db, wxEvtHandler * owner,
            const wxString &artistname, const wxString &trackname,
             wxArrayInt * smartaddedtracks, wxArrayString * smartaddedartists,
             const int maxtracks, const int maxartists,
             const uint tracklimit, const int limittype,
             const int filterallow, const int filterdeny,
             const int gaugeid )  :
    wxThread()
{
    //guLogMessage( wxT( "guSmartModeThread::guSmartModeThread" ) );
    m_Db = db;
    m_Owner = owner;
    m_ArtistName = wxString( artistname.c_str() );
    m_TrackName = wxString( trackname.c_str() );
    m_SmartAddedTracks = smartaddedtracks;
    m_SmartAddedArtists = smartaddedartists;
    m_MaxSmartTracksList = maxtracks;
    m_MaxSmartArtistsList = maxartists;
    m_FilterAllowPlayList = filterallow;
    m_FilterDenyPlayList = filterdeny;
    m_GaugeId = gaugeid;

    m_LimitType = limittype;
    switch( limittype )
    {
        case guSMARTMODE_TRACK_LIMIT_TIME_MINUTES :
            m_TrackLimit = tracklimit * 60000;
            break;

        case guSMARTMODE_TRACK_LIMIT_TIME_HOURS :
            m_TrackLimit = ( tracklimit * 60 * 60000 );
            break;

        case guSMARTMODE_TRACK_LIMIT_SIZE_MB :
            m_TrackLimit = ( tracklimit * 1024 * 1024 );
            break;

        case guSMARTMODE_TRACK_LIMIT_SIZE_GB :
            m_TrackLimit = ( tracklimit * 1024 * 1024 * 1024 );
            break;

        //case guSMARTMODE_TRACK_LIMIT_TRACKS :
        default :
            m_TrackLimit = tracklimit;
            m_LimitType = guSMARTMODE_TRACK_LIMIT_TRACKS;
            break;
    }
    m_LimitCounter = 0;
    m_LimitReached = false;

    if( gaugeid != wxNOT_FOUND )
    {
        wxCommandEvent Event( wxEVT_MENU, ID_STATUSBAR_GAUGE_SETMAX );
        Event.SetInt( m_GaugeId );
        Event.SetExtraLong( 100 );
        wxPostEvent( guMainFrame::GetMainFrame(), Event );
    }

    m_LastFM = new guLastFM();

    if( Create() == wxTHREAD_NO_ERROR )
    {
        SetPriority( WXTHREAD_DEFAULT_PRIORITY - 30 );
        Run();
    }
}


// -------------------------------------------------------------------------------- //
guSmartModeThread::~guSmartModeThread()
{
    if( !TestDestroy() )
    {
        wxCommandEvent event( wxEVT_MENU, ID_SMARTMODE_THREAD_END );
        wxPostEvent( m_Owner, event );
    }

    if( m_GaugeId != wxNOT_FOUND )
    {
        wxCommandEvent Event( wxEVT_MENU, ID_STATUSBAR_GAUGE_REMOVE );
        Event.SetInt( m_GaugeId );
        wxPostEvent( guMainFrame::GetMainFrame(), Event );
    }

    if( m_LastFM )
        delete m_LastFM;
}

// -------------------------------------------------------------------------------- //
void guSmartModeThread::SendTracks( guTrackArray * tracks )
{
    if( !TestDestroy() )
    {
        wxCommandEvent event( wxEVT_MENU, ID_SMARTMODE_ADD_TRACKS );
        event.SetClientData( ( void * ) tracks );
        wxPostEvent( m_Owner, event );
    }
    else
    {
        if( tracks )
        {
            delete tracks;
        }
    }
}

// -------------------------------------------------------------------------------- //
bool guSmartModeThread::CheckLimit( const guTrack * track )
{
    //guLogMessage( wxT( "%i ) CheckLimit %lli => %lli" ), m_LimitType, m_LimitCounter.GetValue(), m_TrackLimit.GetValue() );
    if( track )
    {

        switch( m_LimitType )
        {
            case guSMARTMODE_TRACK_LIMIT_TRACKS :
                if( m_LimitCounter >= m_TrackLimit )
                {
                    return true;
                }
                m_LimitCounter++;
                break;

            case guSMARTMODE_TRACK_LIMIT_TIME_MINUTES :
            case guSMARTMODE_TRACK_LIMIT_TIME_HOURS :
                if( ( m_LimitCounter + track->m_Length ) >= m_TrackLimit )
                {
                    m_LimitReached = true;
                    return true;
                }
                m_LimitCounter += track->m_Length;
                break;

            case guSMARTMODE_TRACK_LIMIT_SIZE_MB :
            case guSMARTMODE_TRACK_LIMIT_SIZE_GB :
                if( ( m_LimitCounter + track->m_FileSize ) >= m_TrackLimit )
                {
                    m_LimitReached = true;
                    return true;
                }
                m_LimitCounter += track->m_FileSize;
                break;
        }
    }
    else
    {
        return m_LimitReached || ( m_LimitCounter >= m_TrackLimit );
    }

    if( m_GaugeId != wxNOT_FOUND )
    {
        wxCommandEvent Event( wxEVT_MENU, ID_STATUSBAR_GAUGE_UPDATE );
        Event.SetInt( m_GaugeId );
        Event.SetExtraLong( long( ( m_LimitCounter * 100 ) / m_TrackLimit ) );
        wxPostEvent( guMainFrame::GetMainFrame(), Event );
    }

    return false;
}

// -------------------------------------------------------------------------------- //
bool guSmartModeThread::CheckAddTrack( const wxString &artistname, const wxString &trackname, guTrackArray * tracks )
{
    //guLogMessage( wxT( "CheckAddTrack: '%s' - '%s'" ), artistname.c_str(), trackname.c_str() );
    // Check if we added the artist before
    if( !m_SmartAddedArtists || ( m_SmartAddedArtists->Index( artistname.Upper() ) == wxNOT_FOUND ) )
    {
        guTrack * Track = m_Db->FindSong( artistname, trackname, m_FilterAllowPlayList, m_FilterDenyPlayList );
        if( Track )
        {
            //guLogMessage( wxT( "Similar: '%s' - '%s'" ), artistname.c_str(), trackname.c_str() );
            // Check if we previously added the track
            if( !m_SmartAddedTracks || ( m_SmartAddedTracks->Index( Track->m_SongId ) == wxNOT_FOUND ) )
            {
                //guLogMessage( wxT( "Found Track '%s' '%s'" ), LenToString( Track->m_Length ).c_str(), SizeToString( Track->m_FileSize ).c_str() );
                Track->m_TrackMode = guTRACK_MODE_SMART;
                tracks->Add( Track );
                return true;
            }
            else
            {
                delete Track;
            }
        }
    }
    return false;
}

// -------------------------------------------------------------------------------- //
inline double MatchStr2Double( const wxString &match )
{
    double Match;
    wxString MatchStr = match;
    MatchStr.Replace( wxT( "." ), wxLocale::GetInfo( wxLOCALE_DECIMAL_POINT, wxLOCALE_CAT_NUMBER ) );
    if( MatchStr.ToDouble( &Match ) )
    {
        return Match;
    }
    return 0;
}

// -------------------------------------------------------------------------------- //
int guSmartModeThread::AddSimilarTracks( const wxString &artist, const wxString &track, guTrackArray * tracks )
{
    //guLogMessage( wxT( "Searching for %s - %s" ), artist.c_str(), track.c_str() );
    int AddedTracks = 0;
    guTrackArray FoundTracks;
    guSimilarTrackInfoArray SimilarTracks = m_LastFM->TrackGetSimilar( artist, track );
    if( SimilarTracks.Count() )
    {
        int Count = SimilarTracks.Count();
        for( int Index = 0; Index < Count; Index++ )
        {
            double Match = MatchStr2Double( SimilarTracks[ Index ].m_Match );
            //guLogMessage( wxT( "Match: %f" ), Match );
            if( Match >= guSMARTMODE_MIN_TRACK_MATCH )
            {
                CheckAddTrack( SimilarTracks[ Index ].m_ArtistName, SimilarTracks[ Index ].m_TrackName, &FoundTracks );
            }
            else    // Results comes sorted by match
            {
                break;
            }
            // TODO: CHANGE LIMIT
            if( TestDestroy() )
                break;
        }
    }

    if( FoundTracks.Count() < 4 )
    {
        guSimilarArtistInfoArray SimilarArtists = m_LastFM->ArtistGetSimilar( artist );
        int Count = SimilarArtists.Count();
        for( int Index = 0; Index < Count; Index++ )
        {
            if( !m_SmartAddedArtists || ( m_SmartAddedArtists->Index( SimilarArtists[ Index ].m_Name.Upper() ) == wxNOT_FOUND ) )
            {
                double Match = MatchStr2Double( SimilarArtists[ Index ].m_Match );
                if( Match >= guSMARTMODE_MIN_ARTIST_MATCH )
                {
                    if( m_Db->GetArtistId( SimilarArtists[ Index ].m_Name, false ) != wxNOT_FOUND )
                    {
                        //guLogMessage( wxT( "Found similar artist: '%s'" ), SimilarArtists[ Index ].m_Name.c_str() );
                        guTopTrackInfoArray ArtistTopTracks = m_LastFM->ArtistGetTopTracks( SimilarArtists[ Index ].m_Name );
                        int TTIndex;
                        int TTCount = ArtistTopTracks.Count();
                        for( TTIndex = 0; TTIndex < TTCount; TTIndex++ )
                        {
                            if( TestDestroy() ||
                                CheckAddTrack( ArtistTopTracks[ TTIndex ].m_ArtistName, ArtistTopTracks[ TTIndex ].m_TrackName, &FoundTracks ) )
                                break;
                        }
                    }
                }
                else
                {
                    break;
                }
            }
            if( TestDestroy() )
                break;
        }
    }

    // Select some random tracks from the available
    while( !TestDestroy() && FoundTracks.Count() )
    {
        int TrackIndex = guRandom( FoundTracks.Count() );

        const guTrack & RandomTrack = FoundTracks[ TrackIndex ];

        if( !m_SmartAddedArtists || ( m_SmartAddedArtists->Index( RandomTrack.m_ArtistName.Upper() ) == wxNOT_FOUND ) )
        {
            if( !m_SmartAddedTracks || ( m_SmartAddedTracks->Index( RandomTrack.m_SongId ) == wxNOT_FOUND ) )
            {
                if( !CheckLimit( &RandomTrack ) )
                {
                    tracks->Add( new guTrack( RandomTrack ) );

                    if( m_SmartAddedArtists )
                    {
                        m_SmartAddedArtists->Add( RandomTrack.m_ArtistName.Upper() );
                    }
                    if( m_SmartAddedTracks )
                    {
                        m_SmartAddedTracks->Add( RandomTrack.m_SongId );
                    }
                    AddedTracks++;
                }
                else
                {
                    break;
                }
            }
        }

        FoundTracks.RemoveAt( TrackIndex );
    }

    return AddedTracks;
}

// -------------------------------------------------------------------------------- //
guSmartModeThread::ExitCode guSmartModeThread::Entry()
{
    if( m_LastFM )
    {
        guTrackArray * Tracks = new guTrackArray();
        size_t TrackIndex = 0;
        if( Tracks )
        {
            while( !TestDestroy() && !CheckLimit() )
            {
                AddSimilarTracks( m_ArtistName, m_TrackName, Tracks );

                if( TrackIndex < Tracks->Count() )
                {
                    const guTrack &AddedTrack = Tracks->Item( TrackIndex );
                    m_ArtistName = AddedTrack.m_ArtistName;
                    m_TrackName = AddedTrack.m_SongName;
                    TrackIndex++;
                    //guLogMessage( wxT( "SmartModeThread:: TrackIndex: %i" ), TrackIndex );
                }
                else
                {
                    break;
                }
            }

            // Remove the oldest added tracks or artists
            if( m_SmartAddedTracks && ( ( int ) m_SmartAddedTracks->Count() > m_MaxSmartTracksList ) )
            {
                m_SmartAddedTracks->RemoveAt( 0, m_SmartAddedTracks->Count() - m_MaxSmartTracksList );
            }

            if( m_SmartAddedArtists && ( ( int ) m_SmartAddedArtists->Count() > m_MaxSmartArtistsList ) )
            {
                m_SmartAddedArtists->RemoveAt( 0, m_SmartAddedArtists->Count() - m_MaxSmartArtistsList );
            }

            //guLogDebug( wxT( "========" ) );
            //for( int Index = 0; Index < m_SmartAddedArtists->Count(); Index++ )
            //    guLogDebug( wxT( "Artist: '%s'" ), ( * m_SmartAddedArtists )[ Index ].c_str() );

            SendTracks( Tracks );
        }
    }
    return 0;
}




// -------------------------------------------------------------------------------- //
// guGenSmartPlaylist
// -------------------------------------------------------------------------------- //
guGenSmartPlaylist::guGenSmartPlaylist( wxWindow * parent, guMediaViewer * mediaviewer, const wxString &playlistname ) :
    wxDialog( parent, wxID_ANY, _( "Generate Smart Playlist" ), wxDefaultPosition, wxSize( 500, 250 ), wxDEFAULT_DIALOG_STYLE )
{
    m_MediaViewer = mediaviewer;
    m_Db = mediaviewer->GetDb();

    m_Playlists.Add( new guListItem( wxNOT_FOUND, _( "All" ) ) );
    m_Db->GetPlayLists( &m_Playlists, guPLAYLIST_TYPE_STATIC );
    m_Db->GetPlayLists( &m_Playlists, guPLAYLIST_TYPE_DYNAMIC );

	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer * MainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer * StaticBoxSizer = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxEmptyString ), wxHORIZONTAL );

    wxFlexGridSizer * FlexGridSizer = new wxFlexGridSizer( 2, 0, 0 );
	FlexGridSizer->AddGrowableCol( 1 );
	FlexGridSizer->SetFlexibleDirection( wxBOTH );
	FlexGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticText * SaveToLabel = new wxStaticText( this, wxID_ANY, _( "Save to" ), wxDefaultPosition, wxDefaultSize, 0 );
	SaveToLabel->Wrap( -1 );
	FlexGridSizer->Add( SaveToLabel, 0, wxALL|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 5 );

	m_SaveToComboBox = new wxComboBox( this, wxID_ANY, playlistname, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
	FlexGridSizer->Add( m_SaveToComboBox, 1, wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	int Count = m_Playlists.Count();
	for( int Index = 1; Index < Count; Index++ )
    {
        m_SaveToComboBox->Append( m_Playlists[ Index ].m_Name );
    }

	wxStaticLine * StaticLineL = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	FlexGridSizer->Add( StaticLineL, 0, wxEXPAND|wxTOP|wxBOTTOM|wxLEFT, 5 );

	wxStaticLine * StaticLineR = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	FlexGridSizer->Add( StaticLineR, 0, wxEXPAND|wxTOP|wxBOTTOM|wxRIGHT, 5 );

	wxStaticText * AllowLabel = new wxStaticText( this, wxID_ANY, _( "Allow" ), wxDefaultPosition, wxDefaultSize, 0 );
	AllowLabel->Wrap( -1 );
	FlexGridSizer->Add( AllowLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

    wxArrayString FilterChoices;
    Count = m_Playlists.Count();
    for( int Index = 0; Index < Count; Index++ )
    {
        FilterChoices.Add( m_Playlists[ Index ].m_Name );
    }

	m_FilterAlowChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, FilterChoices, 0 );
	m_FilterAlowChoice->SetSelection( 0 );
	FlexGridSizer->Add( m_FilterAlowChoice, 1, wxEXPAND|wxBOTTOM|wxRIGHT, 5 );

	wxStaticText * DenyLabel = new wxStaticText( this, wxID_ANY, _( "Deny" ), wxDefaultPosition, wxDefaultSize, 0 );
	DenyLabel->Wrap( -1 );
	FlexGridSizer->Add( DenyLabel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

    FilterChoices[ 0 ] = _( "None" );

	m_FilterDenyChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, FilterChoices, 0 );
	m_FilterDenyChoice->SetSelection( 0 );
	FlexGridSizer->Add( m_FilterDenyChoice, 0, wxEXPAND|wxBOTTOM|wxRIGHT, 5 );

	wxStaticText * LimitLabel = new wxStaticText( this, wxID_ANY, _( "Limit to" ), wxDefaultPosition, wxDefaultSize, 0 );
	LimitLabel->Wrap( -1 );
	FlexGridSizer->Add( LimitLabel, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	wxBoxSizer* LimitSizer;
	LimitSizer = new wxBoxSizer( wxHORIZONTAL );

	m_LimitTextCtrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	LimitSizer->Add( m_LimitTextCtrl, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	wxArrayString LimitChoices;
	LimitChoices.Add( _( "Tracks" ) );
	LimitChoices.Add( _( "Minutes" ) );
	LimitChoices.Add( _( "Hours" ) );
	LimitChoices.Add( _( "MiBytes" ) );
	LimitChoices.Add( _("GiBytes" ) );
	m_LimitChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, LimitChoices, 0 );
	m_LimitChoice->SetSelection( 0 );
	LimitSizer->Add( m_LimitChoice, 0, wxALIGN_CENTER_VERTICAL|wxBOTTOM|wxRIGHT, 5 );

	FlexGridSizer->Add( LimitSizer, 0, wxEXPAND, 5 );

	StaticBoxSizer->Add( FlexGridSizer, 1, wxALIGN_CENTER_VERTICAL, 5 );

	MainSizer->Add( StaticBoxSizer, 1, wxEXPAND|wxALL, 5 );

	wxStdDialogButtonSizer * DlgButtons = new wxStdDialogButtonSizer();
	wxButton * DlgButtonsOK = new wxButton( this, wxID_OK );
	DlgButtons->AddButton( DlgButtonsOK );
	wxButton * DlgButtonsCancel = new wxButton( this, wxID_CANCEL );
	DlgButtons->AddButton( DlgButtonsCancel );
	DlgButtons->Realize();
	MainSizer->Add( DlgButtons, 0, wxEXPAND|wxALL, 5 );

	this->SetSizer( MainSizer );
	this->Layout();
    this->Fit();
}

// -------------------------------------------------------------------------------- //
guGenSmartPlaylist::~guGenSmartPlaylist()
{
}

// -------------------------------------------------------------------------------- //
int guGenSmartPlaylist::GetPlayListId( void )
{
    int Selection = m_SaveToComboBox->GetSelection();
    if( Selection == wxNOT_FOUND && m_SaveToComboBox->GetCount() != 0 )
    {
        Selection = FindPlayListItem( &m_Playlists, m_SaveToComboBox->GetValue().Lower().Trim().Trim( false ) );
    }
    return Selection;
}

}

// -------------------------------------------------------------------------------- //
