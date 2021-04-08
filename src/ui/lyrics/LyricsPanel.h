// -------------------------------------------------------------------------------- //
//    Copyright (C) 2008-2020 J.Rios anonbeat@gmail.com
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
#ifndef __LYRICSPANEL_H__
#define __LYRICSPANEL_H__

#include "AuiNotebook.h"
#include "DbLibrary.h"
#include "TrackChangeInfo.h"

#include <wx/bitmap.h>
#include <wx/checkbox.h>
#include <wx/colour.h>
#include <wx/dnd.h>
#include <wx/event.h>
#include <wx/font.h>
#include <wx/gdicmn.h>
#include <wx/image.h>
#include <wx/settings.h>
#include <wx/stattext.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/icon.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/process.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/html/htmlwin.h>
#include <wx/panel.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
wxString DoExtractTags( const wxString &content, const wxString &begin, const wxString &end );
wxString DoExtractTag( const wxString &content, const wxString &tag );


class guLyricSearchEngine;
class guLyricSearchContext;

// -------------------------------------------------------------------------------- //
class guLyricsPanel : public wxPanel
{
  protected :
    guDbLibrary *           m_Db;
    guLyricSearchEngine *   m_LyricSearchEngine;
    guLyricSearchContext *  m_LyricSearchContext;
    wxTimer                 m_LyricTextTimer;

	wxBoxSizer *            m_TitleSizer;
    wxStaticText *          m_LyricTitle;
    wxTextCtrl *            m_LyricText;
    guMediaViewer *         m_MediaViewer;

    wxCheckBox *            m_UpdateCheckBox;

	wxBitmapButton *        m_SetupButton;
	wxBitmapButton *        m_ReloadButton;
	wxBitmapButton *        m_EditButton;
	wxBitmapButton *        m_SaveButton;
	wxBitmapButton *        m_WebSearchButton;

	wxTextCtrl *            m_ArtistTextCtrl;
	wxTextCtrl *            m_TrackTextCtrl;
	wxBitmapButton *        m_SearchButton;
	wxColour                m_EditModeBGColor;
	wxColour                m_EditModeFGColor;

	bool                    m_UpdateEnabled;
    guTrackChangeInfo       m_CurrentTrackInfo;

    int                     m_LyricAlign;
	guTrack *               m_CurrentTrack;
    wxString                m_CurrentLyricText;
    wxString                m_CurrentSourceName;
	wxString                m_LastLyricText;
	wxString                m_LastSourceName;

    void                    SetTitle( const wxString &title );
    void                    SetText( const wxString &text );

    void                    OnTextTimer( wxTimerEvent &event );
	void                    OnReloadBtnClick( wxCommandEvent& event );
    void                    OnEditBtnClick( wxCommandEvent& event );
    void                    OnSaveBtnClick( wxCommandEvent& event );
    void                    OnWebSearchBtnClick( wxCommandEvent& event );
	void                    OnUpdateChkBoxClicked( wxCommandEvent& event );
	void                    SetAutoUpdate( const bool autoupdate );
    void                    OnTextUpdated( wxCommandEvent& event );
    void                    OnContextMenu( wxContextMenuEvent &event );
    void                    CreateContextMenu( wxMenu * menu );

    void                    OnLyricsCopy( wxCommandEvent &event );
    void                    OnLyricsPaste( wxCommandEvent &event );
    void                    OnLyricsPrint( wxCommandEvent &event );

    void                    OnConfigUpdated( wxCommandEvent &event );
    void                    OnSetupSelected( wxCommandEvent &event );

    void                    OnLyricFound( wxCommandEvent &event );

  public :
    guLyricsPanel( wxWindow * parent, guDbLibrary * db, guLyricSearchEngine * lyricsearchengine );
    ~guLyricsPanel();

    void                    OnSetCurrentTrack( wxCommandEvent &event );
    void                    SetCurrentTrack( const guTrack * track );
    void                    SetTrack( const guTrackChangeInfo * trackchangeinfo, const bool onlinesearch = false );
    //void                    ClearLyricThread( void );
    void                    OnDropFiles( const wxArrayString &files );
    void                    OnDropFiles( const guTrackArray * tracks );

    void                    SetLyricText( const wxString * lyrictext, const bool forceupdate = false );

    void                    SetLyricSearchEngine( guLyricSearchEngine * searchengine ) { m_LyricSearchEngine = searchengine; }

    void                    UpdatedTracks( const guTrackArray * tracks );
    void                    UpdatedTrack( const guTrack * track );

    void                    SetLastSource( const int sourceindex );

    wxString                GetLyricSource( void );

    bool                    UpdateEnabled( void ) { return m_UpdateEnabled; }

    guMediaViewer *         GetMediaViewer( void ) { return m_MediaViewer; }

};

// -------------------------------------------------------------------------------- //
class guLyricsPanelDropTarget : public wxDropTarget
{
  private:
    guLyricsPanel *         m_LyricsPanel;

  public:
    guLyricsPanelDropTarget( guLyricsPanel * lyricspanel );
    ~guLyricsPanelDropTarget();

    virtual wxDragResult    OnData( wxCoord x, wxCoord y, wxDragResult def );
};

enum guLyricSourceType {
    guLYRIC_SOURCE_TYPE_INVALID = -1,
    guLYRIC_SOURCE_TYPE_EMBEDDED,
    guLYRIC_SOURCE_TYPE_FILE,
    guLYRIC_SOURCE_TYPE_COMMAND,
    guLYRIC_SOURCE_TYPE_DOWNLOAD
};

enum guLyricSourceOptionType {
    guLYRIC_SOURCE_OPTION_TYPE_REPLACE,
    guLYRIC_SOURCE_OPTION_TYPE_EXTRACT,
    guLYRIC_SOURCE_OPTION_TYPE_EXCLUDE,
    guLYRIC_SOURCE_OPTION_TYPE_NOTFOUND
};

// -------------------------------------------------------------------------------- //
class guLyricSourceOption
{
  protected :
    wxString        m_Text1;
    wxString        m_Text2;

  public :
    guLyricSourceOption() {}
    guLyricSourceOption( const wxString &string1, const wxString &string2 = wxEmptyString ) { m_Text1 = string1; m_Text2 = string2; }
    ~guLyricSourceOption() {}

    guLyricSourceOption( wxXmlNode * xmlnode, const wxString &tag1, const wxString &tag2 = wxEmptyString );

    wxString        Text1( void ) { return m_Text1; }
    void            Text1( const wxString &text1 ) { m_Text1 = text1; }
    wxString        Text2( void ) { return m_Text2; }
    void            Text2( const wxString &text2 ) { m_Text2 = text2; }

    wxString        ToStr( void ) { return IsSingleOption() ? m_Text1 : m_Text1 + wxT( " → " ) + m_Text2; }

    bool            IsSingleOption( void ) { return m_Text2.IsEmpty(); }
};

// -------------------------------------------------------------------------------- //
class guLyricSourceReplace : public guLyricSourceOption
{
  protected :
  public :
    guLyricSourceReplace() : guLyricSourceOption() {}
    guLyricSourceReplace( const wxString &string1, const wxString &string2 ) : guLyricSourceOption( string1, string2 ) {}
    ~guLyricSourceReplace() {}

    guLyricSourceReplace( wxXmlNode * xmlnode ) : guLyricSourceOption( xmlnode, wxT( "replace" ), wxT( "with" ) ) {}

    wxString    Search( void ) { return m_Text1; }
    void        Search( const wxString &search ) { m_Text1 = search; }
    wxString    Replace( void ) { return m_Text2; }
    void        Replace( const wxString &replace ) { m_Text2 = replace; }
};
WX_DECLARE_OBJARRAY(guLyricSourceReplace, guLyricSourceReplaceArray);

// -------------------------------------------------------------------------------- //
class guLyricSourceExtract : public guLyricSourceOption
{
  protected :
  public :
    guLyricSourceExtract() : guLyricSourceOption() {}
    guLyricSourceExtract( const wxString &string1, const wxString &string2 = wxEmptyString ) : guLyricSourceOption( string1, string2 ) {}
    ~guLyricSourceExtract() {}

    guLyricSourceExtract( wxXmlNode * xmlnode );

    wxString    Begin( void ) { return m_Text1; }
    void        Begin( const wxString &begin ) { m_Text1 = begin; }
    wxString    End( void ) { return m_Text2; }
    void        End( const wxString &end ) { m_Text2 = end; }

    wxString    Tag( void ) { return Begin(); }
    void        Tag( const wxString &tag ) { Begin( tag ); }
};
WX_DECLARE_OBJARRAY(guLyricSourceExtract, guLyricSourceExtractArray);

// -------------------------------------------------------------------------------- //
class guLyricSourceExclude : public guLyricSourceExtract
{
  protected :
  public :
    guLyricSourceExclude() : guLyricSourceExtract() {}
    guLyricSourceExclude( const wxString &string1, const wxString &string2 = wxEmptyString ) : guLyricSourceExtract( string1, string2 ) {}
    ~guLyricSourceExclude() {}

    guLyricSourceExclude( wxXmlNode * xmlnode ) : guLyricSourceExtract( xmlnode ) {}
};
WX_DECLARE_OBJARRAY(guLyricSourceExclude, guLyricSourceExcludeArray);

// -------------------------------------------------------------------------------- //
class guLyricSource
{
  protected :
    wxString                    m_Name;
    int                         m_Type;
    bool                        m_Enabled;
    wxString                    m_Source;
    guLyricSourceReplaceArray   m_ReplaceItems;
    guLyricSourceExtractArray   m_ExtractItems;
    guLyricSourceExcludeArray   m_ExcludeItems;
    wxArrayString               m_NotFoundItems;

    void                        ReadReplaceItems( wxXmlNode * xmlnode );
    void                        ReadExtractItems( wxXmlNode * xmlnode );
    void                        ReadExcludeItems( wxXmlNode * xmlnode );
    void                        ReadNotFoundItems( wxXmlNode * xmlnode );

  public :
    guLyricSource() { m_Type = guLYRIC_SOURCE_TYPE_INVALID; m_Enabled = false; }
    ~guLyricSource() {}

    guLyricSource( wxXmlNode * xmlnode );

    wxString                    Name( void ) { return m_Name; }
    void                        Name( const wxString &name ) { m_Name = name; }
    int                         Type( void ) { return m_Type; }
    void                        Type( const int type ) { m_Type = type; }
    bool                        Enabled( void ) { return m_Enabled; }
    void                        Enabled( const bool enabled ) { m_Enabled = enabled; }
    wxString                    Source( void ) { return m_Source; }
    void                        Source( const wxString &source ) { m_Source = source; }

    wxString                    SourceFieldClean( const wxString &field );

    int                         ReplaceCount( void ) { return m_ReplaceItems.Count(); }
    int                         ExtractCount( void ) { return m_ExtractItems.Count(); }
    int                         ExcludeCount( void ) { return m_ExcludeItems.Count(); }
    int                         NotFoundCount( void ) { return m_NotFoundItems.Count(); }

    guLyricSourceReplaceArray   ReplaceItems( void ) { return m_ReplaceItems; }
    void                        ReplaceItems( guLyricSourceReplaceArray &replaceitems ) { m_ReplaceItems = replaceitems; }
    guLyricSourceReplace *      ReplaceItem( const int index ) { return &m_ReplaceItems[ index ]; }

    guLyricSourceExtractArray   ExtractItems( void ) { return m_ExtractItems; }
    void                        ExtractItems( guLyricSourceExtractArray &extractitems ) { m_ExtractItems = extractitems; }
    guLyricSourceExtract *      ExtractItem( const int index ) { return &m_ExtractItems[ index ]; }

    guLyricSourceExcludeArray   ExcludeItems( void ) { return m_ExcludeItems; }
    void                        ExcludeItems( guLyricSourceExcludeArray &excludeitems ) { m_ExcludeItems = excludeitems; }
    guLyricSourceExclude *      ExcludeItem( const int index ) { return &m_ExcludeItems[ index ]; }

    wxArrayString               NotFoundItems( void ) { return m_NotFoundItems; }
    void                        NotFoundItems( wxArrayString &notfounditems ) { m_NotFoundItems = notfounditems; }
    wxString                    NotFoundItem( const int index ) { return m_NotFoundItems[ index ]; }

};
WX_DECLARE_OBJARRAY(guLyricSource, guLyricSourceArray);

class guLyricSearchEngine;

// -------------------------------------------------------------------------------- //
class guLyricSearchContext
{
  protected :
    guLyricSearchEngine *   m_LyricSearchEngine;
    guTrack                 m_Track;
    int                     m_CurrentIndex;
    wxEvtHandler *          m_Owner;
    bool                    m_DoSaveProcess;

  public :
    guLyricSearchContext( guLyricSearchEngine * searchengine, wxEvtHandler * owner, guTrack * track, const bool dosaveprocess = true ) { m_LyricSearchEngine = searchengine; m_Owner = owner; m_Track = * track; m_CurrentIndex = wxNOT_FOUND; m_DoSaveProcess = dosaveprocess; }
    ~guLyricSearchContext();

    bool                    GetNextSource( guLyricSource * lyricsource, const bool allowloop = true );
    wxEvtHandler *          Owner( void ) { return m_Owner; }

    bool                    DoSaveProcess( void ) { return m_DoSaveProcess; }

    void                    ResetIndex( void ) { m_CurrentIndex = wxNOT_FOUND; }

    friend class guLyricSearchEngine;
    friend class guLyricSearchThread;
};

// -------------------------------------------------------------------------------- //
class guLyricSearchThread : public wxThread
{
  protected :
    wxString                    m_LyricText;
    bool                        m_ForceSaveProcess;
    bool                        m_CommandIsExecuting;
    guLyricSearchContext *      m_LyricSearchContext;
    bool *                      m_NotifyHere;


    void                        LyricFile( guLyricSource &lyricsource );
    void                        LyricCommand( guLyricSource &lyricsource );
    void                        LyricDownload( guLyricSource &lyricsource );

    bool                        CheckNotFound( guLyricSource &lyricsource );
    wxString                    CheckExtract( const wxString &content, guLyricSource &lyricsource );
    wxString                    CheckExclude( const wxString &content, guLyricSource &lyricsource );

    wxString                    GetSource( guLyricSource &lyricsource );
    wxString                    DoReplace( const wxString &text, const wxString &search, const wxString &replace );
    wxString                    DoReplace( const wxString &text, guLyricSource &lyricsource );

    void                        ProcessTags( wxString * text, guLyricSource &lyricsource );

    void                        ProcessSave( guLyricSource &lyricsource );

  public :
    guLyricSearchThread( guLyricSearchContext * context, const wxString &lyrictext = wxEmptyString, const bool forcesaveprocess = false );
    ~guLyricSearchThread();

    ExitCode                    Entry();

    wxString                    LyricText( void ) { return m_LyricText; }
    guLyricSearchContext *      LyricSearchContext( void ) { return m_LyricSearchContext; }

    void                        FinishExecCommand( const wxString &lyrictext );
    void                        SetNotificationPtr(bool * ptr) { m_NotifyHere = ptr; }

    friend class guLyricSearchEngine;
};
WX_DEFINE_ARRAY_PTR( guLyricSearchThread *, guLyricSearchThreadArray );

// -------------------------------------------------------------------------------- //
class guLyricExecCommandTerminate : public wxProcess
{
  protected :
    guLyricSearchThread *   m_LyricSearchThread;
    bool                    m_IsSaveCommand;
    bool                    m_ThreadActive;

  public :
	guLyricExecCommandTerminate( guLyricSearchThread * searchthread, const bool issavecommand = false );
    ~guLyricExecCommandTerminate() { guLogDebug("guLyricExecCommandTerminate::~guLyricExecCommandTerminate"); }

    bool * GetNotificationPtr() { return &m_ThreadActive; }

	virtual void OnTerminate( int pid, int status );

};

// -------------------------------------------------------------------------------- //
class guLyricSearchEngine
{
  protected :
    guLyricSourceArray          m_LyricSources;
    guLyricSourceArray          m_LyricTargets;
    guLyricSearchThreadArray    m_LyricSearchThreads;
    wxMutex                     m_LyricSearchThreadsMutex;
    bool                        m_TargetsEnabled;

    void                        ReadSources( wxXmlNode * xmlnode );
    void                        ReadTargets( wxXmlNode * xmlnode );

    void                        UpdateTargetsEnabled( void );

  public :
    guLyricSearchEngine();
    ~guLyricSearchEngine();

    guLyricSearchContext *      CreateContext( wxEvtHandler * owner, guTrack * track, const bool dosaveprocess = true );

    bool                        GetNextSource( guLyricSearchContext * context, guLyricSource * lyricsource, const bool allowloop = true );
    void                        SearchStart( guLyricSearchContext * context );
    void                        SetLyricText( guLyricSearchContext * context, const wxString &lyrictext );
    void                        SearchFinished( guLyricSearchThread * searchthread );

    void                        Load( void );

    size_t                      SourcesCount( void ) { return m_LyricSources.Count(); }
    guLyricSource *             GetSource( const int index ) { return &m_LyricSources[ index ]; }

    size_t                      TargetsCount( void ) { return m_LyricTargets.Count(); }
    guLyricSource *             GetTarget( const int index ) { return &m_LyricTargets[ index ]; }
    bool                        TargetsEnabled( void ) { return m_TargetsEnabled; }

    void                        SourceAdd( const guLyricSource * lyricsource ) { m_LyricSources.Add( lyricsource ); }
    void                        SourceRemoveAt( const int index ) { m_LyricSources.RemoveAt( index ); }
    void                        SourceMoveUp( const int index );
    void                        SourceMoveDown( const int index );

    void                        TargetAdd( const guLyricSource * lyricsource ) { m_LyricTargets.Add( lyricsource ); }
    void                        TargetRemoveAt( const int index ) { m_LyricTargets.RemoveAt( index ); }
    void                        TargetMoveUp( const int index );
    void                        TargetMoveDown( const int index );

    bool                        Save( void );

    void                        RemoveContextThread( guLyricSearchContext * searchcontext );

};

// -------------------------------------------------------------------------------- //
class guLyricSourceEditor : public wxDialog
{
  protected:
    guLyricSource *             m_LyricSource;
    bool                        m_IsTarget;

    guLyricSourceReplaceArray   m_ReplaceItems;
    guLyricSourceExtractArray   m_ExtractItems;
    guLyricSourceExcludeArray   m_ExcludeItems;
    wxArrayString               m_NotFoundItems;

    wxTextCtrl  *       m_NameTextCtrl;
    wxChoice *          m_TypeChoice;
    wxTextCtrl *        m_SourceTextCtrl;
    wxListBox *         m_ReplaceListBox;
    wxBitmapButton *    m_ReplaceAdd;
    wxBitmapButton *    m_ReplaceDel;
    wxListBox *         m_ExtractListBox;
    wxBitmapButton *    m_ExtractAdd;
    wxBitmapButton *    m_ExtractDel;
    wxListBox *         m_ExcludeListBox;
    wxBitmapButton *    m_ExcludeAdd;
    wxBitmapButton *    m_ExcludeDel;
    wxListBox *         m_NotFoundListBox;
    wxBitmapButton *    m_NotFoundAdd;
    wxBitmapButton *    m_NotFoundDel;

    void                OnTypeChanged( wxCommandEvent &event );
    void                OnReplaceSelected( wxCommandEvent &event );
    void                OnReplaceAddClicked( wxCommandEvent &event );
    void                OnReplaceDelClicked( wxCommandEvent &event );
    void                OnReplaceDClicked( wxCommandEvent &event );
    void                OnExtractSelected( wxCommandEvent &event );
    void                OnExtractAddClicked( wxCommandEvent &event );
    void                OnExtractDelClicked( wxCommandEvent &event );
    void                OnExtractDClicked( wxCommandEvent &event );
    void                OnExcludeSelected( wxCommandEvent &event );
    void                OnExcludeAddClicked( wxCommandEvent &event );
    void                OnExcludeDelClicked( wxCommandEvent &event );
    void                OnExcludeDClicked( wxCommandEvent &event );
    void                OnNotFoundSelected( wxCommandEvent &event );
    void                OnNotFoundAddClicked( wxCommandEvent &event );
    void                OnNotFoundDelClicked( wxCommandEvent &event );
    void                OnNotFoundDClicked( wxCommandEvent &event );

  public:
    guLyricSourceEditor( wxWindow * parent, guLyricSource * lyricsource, const bool istarget = false );
    ~guLyricSourceEditor();

    void                UpdateLyricSource( void );

};

// -------------------------------------------------------------------------------- //
class guLyricSourceOptionEditor : public wxDialog
{
  protected :
    guLyricSourceOption *   m_SourceOption;
    wxStaticText *          m_SearchLabel;
    wxTextCtrl *            m_SearchTextCtrl;
    wxStaticText *          m_ReplaceLabel;
    wxTextCtrl *            m_ReplaceTextCtrl;

  public :
    guLyricSourceOptionEditor( wxWindow * parent, guLyricSourceOption * lyricsourceoption, const int optiontype );
    ~guLyricSourceOptionEditor();

    void                    UpdateSourceOption( void );

};

}

#endif
// -------------------------------------------------------------------------------- //
