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
#ifndef COPYTO_H
#define COPYTO_H

#include "DbLibrary.h"
#include "MainFrame.h"
#include "PortableMedia.h"


#include <wx/string.h>
#include <wx/dynarray.h>

enum guCopyToActionType {
    guCOPYTO_ACTION_NONE,
    guCOPYTO_ACTION_COPYTO,
    guCOPYTO_ACTION_COPYTODEVICE
};


// -------------------------------------------------------------------------------- //
class guCopyToAction
{
  private :
    int                     m_Type;
    guTrackArray *          m_Tracks;
    wxString                m_DestDir;
    wxString                m_Pattern;
    int                     m_Format;
    int                     m_Quality;
    bool                    m_MoveFiles;
    guLibPanel *            m_LibPanel;
    guDbLibrary *           m_Db;
    guPortableMediaPanel *  m_PortableMediaPanel;

  public :
    guCopyToAction();
    guCopyToAction( guTrackArray * tracks, guLibPanel * libpanel, const wxString &destdir,
            const wxString &pattern, int format, int quality, bool movefiles );
    guCopyToAction( guTrackArray * tracks, guDbLibrary * db, guPortableMediaPanel * portablemediapanel );
    ~guCopyToAction();

    int                     Type( void ) { return m_Type; }
    guTrackArray *          Tracks( void ) { return m_Tracks; }
    wxString                DestDir( void ) { return m_DestDir; }
    wxString                Pattern( void ) { return m_Pattern; }
    int                     Format( void ) { return m_Format; }
    int                     Quality( void ) { return m_Quality; };
    bool                    MoveFiles( void ) { return m_MoveFiles; }

    size_t                  Count( void  ) { return m_Tracks ? m_Tracks->Count() : 0; }
    guTrack *               Track( const int index ) { return &m_Tracks->Item( index ); }

    guPortableMediaPanel *  PortableMediaPanel( void ) { return m_PortableMediaPanel; }
    guPortableMediaDevice * PortableMediaDevice( void ) { return m_PortableMediaPanel->PortableMediaDevice(); }

    guDbLibrary *           Db( void ) { return m_Db; }
    guLibPanel *            LibPanel( void ) { return m_LibPanel; }

};
WX_DECLARE_OBJARRAY( guCopyToAction, guCopyToActionArray );

// -------------------------------------------------------------------------------- //
class guCopyToThread : public wxThread
{
  private:
    guMainFrame *               m_MainFrame;
    int                         m_GaugeId;
    wxFileOffset                m_SizeCounter;
    int                         m_CurrentFile;
    int                         m_FileCount;
    wxArrayString               m_FilesToAdd;
    wxArrayString               m_CoversToAdd;

    guCopyToActionArray *       m_CopyToActions;
    wxMutex                     m_CopyToActionsMutex;

    void                        CopyFile( const wxString &from, const wxString &to );
    void                        TranscodeFile( const wxString &from, const wxString &to, int format, int quality );
    void                        DoCopyToAction( guCopyToAction &copytoaction );


  public:
    guCopyToThread( guMainFrame * mainframe, int gaugeid );
    ~guCopyToThread();

    void    AddAction( guTrackArray * tracks, guLibPanel * libpanel, const wxString &destdir,
                    const wxString &pattern, int format, int quality, bool movefiles );
    void    AddAction( guTrackArray * tracks, guDbLibrary * db, guPortableMediaPanel * portablemediapanel );

    virtual ExitCode Entry();

};

#endif
// -------------------------------------------------------------------------------- //
