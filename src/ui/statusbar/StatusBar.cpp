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
#include "StatusBar.h"

#include "AuiNotebook.h"
#include "EventCommandIds.h"
#include "Images.h"
#include "Preferences.h"
#include "MainFrame.h"
#include "Utils.h"

namespace Guayadeque {

#define guTRACKCOUNT_PANEL_SIZE         300
#define guAUDIOSCROBBLE_PANEL_SIZE      40
#define guFORCEGAPLESS_PANEL_SIZE       24

enum guStatusBarClickAction {
    guSTATUSBAR_CLICK_ACTION_NONE = -1,
    guSTATUSBAR_CLICK_ACTION_AUDIOSCROBBLE,
    guSTATUSBAR_CLICK_ACTION_FORCEGAPLESS
};

#define guSTATUSBAR_CLICK_TIMEOUT   250

// -------------------------------------------------------------------------------- //
// guGauge
// -------------------------------------------------------------------------------- //
BEGIN_EVENT_TABLE(guGauge, wxControl)
  EVT_PAINT          (guGauge::OnPaint)
END_EVENT_TABLE()

// -------------------------------------------------------------------------------- //
guGauge::guGauge( wxWindow * parent, const wxString &label, bool showporcent,
            wxWindowID id, unsigned int max, const wxPoint &pos, const wxSize &size, long style ) :
         wxControl( parent, id, pos, size, style )
{
    m_LastValue = wxNOT_FOUND;
    m_ShowPorcent = showporcent;
    m_PaintWidth = 0;
    //m_Value     = 0;
    //m_Range     = max;
    m_Label     = label;
    m_Font      = wxSystemSettings::GetFont( wxSYS_DEVICE_DEFAULT_FONT );
    m_FgColor1  = wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT );
    m_FgColor2  = wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHTTEXT );
    m_GradStart = wxSystemSettings::GetColour( wxSYS_COLOUR_HIGHLIGHT );
    m_GradEnd   = wxSystemSettings::GetColour( wxSYS_COLOUR_HOTLIGHT );


    SetRange( max );
    SetValue( 0 );
}

// -------------------------------------------------------------------------------- //
guGauge::~guGauge()
{
}

// -------------------------------------------------------------------------------- //
void guGauge::OnPaint( wxPaintEvent &event )
{
    //guLogMessage( wxT( "Gauge::OnPaint" ) );

    wxSize s = GetSize();

    wxPaintDC dc( this );

    dc.SetFont( m_Font );

    dc.SetPen( * wxTRANSPARENT_PEN );
    dc.SetBrush( * wxTRANSPARENT_BRUSH );
    dc.DrawRectangle( 1, 1, s.x, s.y );

    dc.SetTextForeground( m_FgColor1 );
    wxString LabelStr;
    if( m_ShowPorcent )
    {
        LabelStr = m_Label + wxString::Format( wxT( " %u%%" ), ( ( m_PaintWidth * 100 ) / s.x ) );
    }
    else
    {
        LabelStr = m_Label + wxString::Format( wxT( " %i / %i" ), m_Value, m_Range );
    }
    dc.DrawText( LabelStr, 4, 2 );

    if( m_PaintWidth )
    {
        wxRect rect;
        rect.x = 1;
        rect.y = 1;
        rect.width = m_PaintWidth;
        rect.height = s.y;

        wxDCClipper clip( dc, rect );

        //dc.DrawRectangle( 1, 1, ( long ) m_Value, s.y );
        rect.width = s.x;
        dc.GradientFillLinear( rect, m_GradStart, m_GradEnd, wxEAST );

        dc.SetTextForeground( m_FgColor2 );
        dc.DrawText( LabelStr, 4, 2 );
    }
}

// -------------------------------------------------------------------------------- //
bool guGauge::SetValue( int value )
{
    //guLogMessage( wxT( "Value: %u (%f) of %u -> %u%%" ), value, m_LastValue, m_Range, (value * 100) / m_Range );
    m_Value = value;
    m_PaintWidth = value * m_Factor;
    if( m_LastValue != m_PaintWidth )
    {
        m_LastValue = m_PaintWidth;
        Refresh();
//        Update();
    }
    return true;
}

// -------------------------------------------------------------------------------- //
void guGauge::SetRange( int range )
{
    //guLogMessage( wxT( "Range: %u" ), range );
    m_Range  = range;
    m_Factor = (float) GetSize().x / (float) range;
    SetValue( GetValue() );
    Refresh();
//    Update();
}

// -------------------------------------------------------------------------------- //
// guStatusBar
// -------------------------------------------------------------------------------- //
guStatusBar::guStatusBar( wxWindow * parent ) :
    wxStatusBar( parent, wxID_ANY, wxST_SIZEGRIP | wxSB_FLAT )
{
    m_LastClickAction = guSTATUSBAR_CLICK_ACTION_NONE;

    int FieldWidths[] = { -1, guTRACKCOUNT_PANEL_SIZE, guFORCEGAPLESS_PANEL_SIZE, guAUDIOSCROBBLE_PANEL_SIZE };
    SetFieldsCount( 4 );
    SetStatusWidths( 4, FieldWidths );
    int FieldStyles[] = { wxSB_FLAT, wxSB_FLAT, wxSB_FLAT, wxSB_FLAT };
    SetStatusStyles( 4, FieldStyles );

    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->RegisterObject( this );

    int FadeOutTime = Config->ReadNum( CONFIG_KEY_CROSSFADER_FADEOUT_TIME, 50, CONFIG_PATH_CROSSFADER ) * 100;
    m_ForceGapless = ( !FadeOutTime || Config->ReadBool( CONFIG_KEY_CROSSFADER_FORCE_GAPLESS, false, CONFIG_PATH_CROSSFADER ) );

    m_ASBitmap = new wxStaticBitmap( this, wxID_ANY, guImage( guIMAGE_INDEX_lastfm_as_off ) );
    m_ASBitmap->SetToolTip( _( "Enable audioscrobbling" ) );

    m_PlayMode = new wxStaticBitmap( this, wxID_ANY, guImage( m_ForceGapless ? guIMAGE_INDEX_tiny_gapless : guIMAGE_INDEX_tiny_crossfade ) );
    m_PlayMode->SetToolTip( m_ForceGapless ? _( "Enable crossfading" ) : _( "Disable crossfading" ) );

    m_SelInfo = new wxStaticText( this, wxID_ANY, wxEmptyString );
    m_SelInfo->SetToolTip( _( "Shows information about the selected items" ) );

    m_ClickTimer.SetOwner( this );

    Bind( wxEVT_SIZE, &guStatusBar::OnSize, this );

//	m_ASBitmap->Bind( wxEVT_LEFT_DOWN, &guStatusBar::OnAudioScrobbleLClicked, this );
//	m_ASBitmap->Bind( wxEVT_RIGHT_DOWN, &guStatusBar::OnAudioScrobbleRClicked, this );
//
//	m_PlayMode->Bind( wxEVT_LEFT_DOWN, &guStatusBar::OnPlayModeLClicked, this );
//	m_PlayMode->Bind( wxEVT_RIGHT_DOWN, &guStatusBar::OnPlayModeRClicked, this );
	m_ASBitmap->Bind( wxEVT_LEFT_DOWN, &guStatusBar::OnButtonClick, this );
	m_ASBitmap->Bind( wxEVT_LEFT_DCLICK, &guStatusBar::OnButtonDClick, this );
	m_PlayMode->Bind( wxEVT_LEFT_DOWN, &guStatusBar::OnButtonClick, this );
	m_PlayMode->Bind( wxEVT_LEFT_DCLICK, &guStatusBar::OnButtonDClick, this );

    Bind( guConfigUpdatedEvent, &guStatusBar::OnConfigUpdated, this, ID_CONFIG_UPDATED );

    Bind( wxEVT_TIMER, &guStatusBar::OnTimerEvent, this );
}

// -------------------------------------------------------------------------------- //
guStatusBar::~guStatusBar()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->UnRegisterObject( this );

    Unbind( wxEVT_SIZE, &guStatusBar::OnSize, this );

//  m_ASBitmap->Unbind( wxEVT_LEFT_DOWN, &guStatusBar::OnAudioScrobbleLClicked, this );
//  m_ASBitmap->Unbind( wxEVT_RIGHT_DOWN, &guStatusBar::OnAudioScrobbleRClicked, this );
//
//  m_PlayMode->Unbind( wxEVT_LEFT_DOWN, &guStatusBar::OnPlayModeLClicked, this );
//  m_PlayMode->Unbind( wxEVT_RIGHT_DOWN, &guStatusBar::OnPlayModeRClicked, this );
    m_ASBitmap->Unbind( wxEVT_LEFT_DOWN, &guStatusBar::OnButtonClick, this );
    m_ASBitmap->Unbind( wxEVT_LEFT_DCLICK, &guStatusBar::OnButtonDClick, this );
    m_PlayMode->Unbind( wxEVT_LEFT_DOWN, &guStatusBar::OnButtonClick, this );
    m_PlayMode->Unbind( wxEVT_LEFT_DCLICK, &guStatusBar::OnButtonDClick, this );

    Unbind( guConfigUpdatedEvent, &guStatusBar::OnConfigUpdated, this, ID_CONFIG_UPDATED );

    Unbind( wxEVT_TIMER, &guStatusBar::OnTimerEvent, this );

    if( m_ASBitmap )
        delete m_ASBitmap;

    if( m_PlayMode )
        delete m_PlayMode;
}

// -------------------------------------------------------------------------------- //
void guStatusBar::OnConfigUpdated( wxCommandEvent &event )
{
    int Flags = event.GetInt();
    if( Flags & guPREFERENCE_PAGE_FLAG_CROSSFADER )
    {
        //guLogMessage( wxT( "guSTatusBar::OnConfigUPdated..." ) );
        guConfig * Config = ( guConfig * ) guConfig::Get();
        Config->RegisterObject( this );

        int FadeOutTime = Config->ReadNum( CONFIG_KEY_CROSSFADER_FADEOUT_TIME, 50, CONFIG_PATH_CROSSFADER ) * 100;
        SetPlayMode( !FadeOutTime || m_ForceGapless );
    }
}

// -------------------------------------------------------------------------------- //
void guStatusBar::OnSize( wxSizeEvent &event )
{
    wxRect rect;

    UpdateGauges();
    //
    if( m_ASBitmap )
    {
        //size = ASBitmap->GetSize();
        GetFieldRect( GetFieldsCount() - 1, rect );
        m_ASBitmap->Move( rect.x + 1,
                        rect.y + 2 );
    }

    if( m_PlayMode )
    {
        //size = ASBitmap->GetSize();
        GetFieldRect( GetFieldsCount() - 2, rect );
        m_PlayMode->Move( rect.x + 1,
                        rect.y + 3 );
    }

    if( m_SelInfo )
    {
        GetFieldRect( GetFieldsCount() - 3, rect );
        m_SelInfo->Move( rect.x + 1, rect.y + 3 );
    }

    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guStatusBar::UpdateAudioScrobbleIcon( bool Enabled )
{
    if( m_ASBitmap )
    {
        m_ASBitmap->SetBitmap( guImage( Enabled ? guIMAGE_INDEX_lastfm_as_on : guIMAGE_INDEX_lastfm_as_off ) );
        m_ASBitmap->SetToolTip( Enabled ? _( "Disable audioscrobbling" ) : _( "Enable audioscrobbling" ) );
        m_ASBitmap->Refresh();
    }
}

// -------------------------------------------------------------------------------- //
void guStatusBar::SetPlayMode( const bool forcegapless )
{
    if( m_ForceGapless != forcegapless )
    {
        m_ForceGapless = forcegapless;
        guConfig * Config = ( guConfig * ) guConfig::Get();
        Config->WriteBool( CONFIG_KEY_CROSSFADER_FORCE_GAPLESS, m_ForceGapless, CONFIG_PATH_CROSSFADER );
        Config->Flush();
        if( m_PlayMode )
        {
            m_PlayMode->SetBitmap( guImage( forcegapless ? guIMAGE_INDEX_tiny_gapless : guIMAGE_INDEX_tiny_crossfade ) );
            m_PlayMode->SetToolTip( m_ForceGapless ? _( "Enable crossfading" ) : _( "Disable crossfading" ) );
            m_PlayMode->Refresh();
        }
    }
}

// -------------------------------------------------------------------------------- //
void guStatusBar::SetAudioScrobble( const bool audioscrobble )
{
    OnAudioScrobbleClicked();
}

// -------------------------------------------------------------------------------- //
void guStatusBar::OnButtonClick( wxMouseEvent &event )
{
    //guLogMessage( wxT( "OnButtonClick") );
    m_LastClickAction = guSTATUSBAR_CLICK_ACTION_NONE;
    if( event.GetEventObject() == m_ASBitmap )
    {
        m_LastClickAction = guSTATUSBAR_CLICK_ACTION_AUDIOSCROBBLE;
    }
    else if( event.GetEventObject() == m_PlayMode )
    {
        m_LastClickAction = guSTATUSBAR_CLICK_ACTION_FORCEGAPLESS;
    }

    if( m_ClickTimer.IsRunning() )
        m_ClickTimer.Stop();

    m_ClickTimer.Start( guSTATUSBAR_CLICK_TIMEOUT, wxTIMER_ONE_SHOT );
}

// -------------------------------------------------------------------------------- //
void guStatusBar::OnButtonDClick( wxMouseEvent &event )
{
    if( m_ClickTimer.IsRunning() )
        m_ClickTimer.Stop();

    if( event.GetEventObject() == m_ASBitmap )
    {
        OnAudioScrobbleDClicked();
    }
    else if( event.GetEventObject() == m_PlayMode )
    {
        OnPlayModeDClicked();
    }

    m_LastClickAction = guSTATUSBAR_CLICK_ACTION_NONE;
}

// -------------------------------------------------------------------------------- //
void guStatusBar::OnTimerEvent( wxTimerEvent &event )
{
    if( m_LastClickAction == guSTATUSBAR_CLICK_ACTION_AUDIOSCROBBLE )
        OnAudioScrobbleClicked();
    else if( m_LastClickAction == guSTATUSBAR_CLICK_ACTION_FORCEGAPLESS )
        OnPlayModeClicked();
    m_LastClickAction = guSTATUSBAR_CLICK_ACTION_NONE;
}

// -------------------------------------------------------------------------------- //
void guStatusBar::OnAudioScrobbleClicked( void )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();

    bool ConfigUpdated = false;
    int  LastFMEnabled = wxNOT_FOUND;
    int  LibreFMEnabled = wxNOT_FOUND;
    if( !Config->ReadStr( CONFIG_KEY_LASTFM_USERNAME, wxEmptyString, CONFIG_PATH_LASTFM ).IsEmpty() &&
        !Config->ReadStr( CONFIG_KEY_LASTFM_PASSWORD, wxEmptyString, CONFIG_PATH_LASTFM ).IsEmpty() )
    {
        LastFMEnabled = !Config->ReadBool( CONFIG_KEY_LASTFM_ENABLED, false, CONFIG_PATH_LASTFM );
        Config->WriteBool( CONFIG_KEY_LASTFM_ENABLED, LastFMEnabled, CONFIG_PATH_LASTFM );
        ConfigUpdated = true;
    }

    if( ( ( LastFMEnabled == ( LibreFMEnabled = !Config->ReadBool( CONFIG_KEY_LIBREFM_ENABLED, false, CONFIG_PATH_LIBREFM ) ) ) ||
          ( LastFMEnabled == wxNOT_FOUND ) ) &&
       !Config->ReadStr( CONFIG_KEY_LIBREFM_USERNAME, wxEmptyString, CONFIG_PATH_LIBREFM ).IsEmpty() &&
        !Config->ReadStr( CONFIG_KEY_LIBREFM_PASSWORD, wxEmptyString, CONFIG_PATH_LIBREFM ).IsEmpty() )
    {
        Config->WriteBool( CONFIG_KEY_LIBREFM_ENABLED, LibreFMEnabled, CONFIG_PATH_LIBREFM );
    }

    if( ConfigUpdated )
    {
        Config->Flush();
        Config->SendConfigChangedEvent( guPREFERENCE_PAGE_FLAG_AUDIOSCROBBLE );

        UpdateAudioScrobbleIcon( ( ( LastFMEnabled != wxNOT_FOUND ) && LastFMEnabled ) || LibreFMEnabled );
    }
}

// -------------------------------------------------------------------------------- //
void guStatusBar::OnAudioScrobbleDClicked( void )
{
    //guLogMessage( wxT( "AUdioScrobble clicked..." ) );
    wxCommandEvent CmdEvent( wxEVT_MENU, ID_MENU_PREFERENCES );
    CmdEvent.SetInt( guPREFERENCE_PAGE_AUDIOSCROBBLE );
    wxPostEvent( guMainFrame::GetMainFrame(), CmdEvent );
}

// -------------------------------------------------------------------------------- //
void guStatusBar::OnPlayModeClicked( void )
{
    //guLogMessage( wxT( "StatusBar::OnPlayModeClicked..." ) );
    guConfig * Config = ( guConfig * ) guConfig::Get();
    int FadeOutTime = Config->ReadNum( CONFIG_KEY_CROSSFADER_FADEOUT_TIME, 50, CONFIG_PATH_CROSSFADER ) * 100;
    if( FadeOutTime )
    {
        SetPlayMode( !m_ForceGapless );

        wxCommandEvent CmdEvent( wxEVT_MENU, ID_MAINFRAME_SETFORCEGAPLESS );
        CmdEvent.SetInt( m_ForceGapless );
        wxPostEvent( guMainFrame::GetMainFrame(), CmdEvent );
    }
}

// -------------------------------------------------------------------------------- //
void guStatusBar::OnPlayModeDClicked( void )
{
    wxCommandEvent CmdEvent( wxEVT_MENU, ID_MENU_PREFERENCES );
    CmdEvent.SetInt( guPREFERENCE_PAGE_CROSSFADER );
    wxPostEvent( guMainFrame::GetMainFrame(), CmdEvent );
}

// -------------------------------------------------------------------------------- //
void guStatusBar::SetSizes( int FieldCnt )
{
    int index;
    int * FieldWidths = ( int * ) malloc( sizeof( int ) * FieldCnt );
    if( FieldWidths )
    {
        for( index = 0; index < FieldCnt; index++ )
        {
            if( index == 0 )
                FieldWidths[ index ] = -1;
            else if( index == ( FieldCnt - 1 ) )
                FieldWidths[ index ] = guAUDIOSCROBBLE_PANEL_SIZE;
            else if( index == ( FieldCnt - 2 ) )
                FieldWidths[ index ] = guFORCEGAPLESS_PANEL_SIZE;
            else if( index == ( FieldCnt - 3 ) )
                FieldWidths[ index ] = guTRACKCOUNT_PANEL_SIZE;
            else
                FieldWidths[ index ] = 200;
            //printf( "Width: %i\n", FieldWidths[ index ] );
        }
        //SetStatusWidths( FieldCnt, FieldWidths );
        SetFieldsCount( FieldCnt, FieldWidths );
        free( FieldWidths );

        //Refresh();
    }
}

// -------------------------------------------------------------------------------- //
void guStatusBar::UpdateGauges( void )
{
    int     index;
    int     count;
    wxRect  rect;
    wxSize  size;
    int     PanelIndex = 0;
    // Update the pos for all the gauges in the statusbar
    count = m_Gauges.Count();
    for( index = 0; index < count; index++ )
    {
        if( m_Gauges[ index ] )
        {
            // The first Panel is the info with index 0, the first gauge will be 1
            PanelIndex++;
            GetFieldRect( PanelIndex, rect );
            size.x = rect.width - 2;
            size.y = rect.height - 2;
            m_Gauges[ index ]->Move( rect.x + 1, rect.y + 1 );
            m_Gauges[ index ]->SetSize( size );
        }
    }
    Refresh();
}

// -------------------------------------------------------------------------------- //
int guStatusBar::AddGauge( const wxString &text, bool showporcent )
{
    m_Gauges.Add( new guGauge( this, text, showporcent ) );
    SetSizes( GetFieldsCount() + 1 );

    UpdateGauges();

    return ( m_Gauges.Count() - 1 );
}

// -------------------------------------------------------------------------------- //
int guStatusBar::RemoveGauge( int gaugeid )
{

    SetSizes( GetFieldsCount() - 1 );
    RemoveChild( m_Gauges[ gaugeid ] );
    delete m_Gauges[ gaugeid ];
    m_Gauges[ gaugeid ] = NULL;
    //Gauges.RemoveAt( gaugeid );
//    if( gaugeid == ( Gauges.Count() - 1 )
//    {
//        Gauges.RemoveAt( gaugeid );
//    }

    // We remove all empty gauges from the array from the end
    // so the valid index dont get changed when remove a lower gauge index
    while( m_Gauges.Count() && ( m_Gauges[ m_Gauges.Count() - 1 ] == NULL ) )
    {
        m_Gauges.RemoveAt( m_Gauges.Count() - 1 );
    }

    UpdateGauges();

    return m_Gauges.Count();
}

// -------------------------------------------------------------------------------- //
void guStatusBar::SetSelInfo( const wxString &label )
{
    m_SelInfo->SetLabel( label );
}

// -------------------------------------------------------------------------------- //
void guStatusBar::DrawField( wxDC &dc, int i, int textHeight )
{
    wxRect rect;
    GetFieldRect( i, rect );
    if( i < ( GetFieldsCount() - 1 ) )
    {
        dc.SetPen( m_mediumShadowPen );
        dc.DrawLine( rect.x + rect.width - 1, rect.y + 1, rect.x + rect.width - 1, rect.y + rect.height - 1 );
        dc.SetPen( m_hilightPen );
        dc.DrawLine( rect.x + rect.width, rect.y + 1, rect.x + rect.width, rect.y + rect.height - 1 );
    }

    DrawFieldText( dc, rect, i, textHeight );
}

}

// -------------------------------------------------------------------------------- //
