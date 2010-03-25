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
#ifndef FILEBROWSER_H
#define FILEBROWSER_H

#include "DbLibrary.h"
#include "PlayerPanel.h"

#include <wx/colour.h>
#include <wx/dirctrl.h>
#include <wx/dynarray.h>
#include <wx/font.h>
#include <wx/gdicmn.h>
#include <wx/panel.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/string.h>

#define     guPANEL_FILEBROWSER_DIRCTRL             ( 1 << 0 )
#define     guPANEL_FILEBROWSER_FILECTRL            ( 1 << 1 )
#define     guPANEL_FILEBROWSER_FILEDETAILS         ( 1 << 2 )

#define     guPANEL_FILEBROWSER_VISIBLE_DEFAULT     ( guPANEL_FILEBROWSER_DIRCTRL | guPANEL_FILEBROWSER_FILECTRL |\
                                                      guPANEL_FILEBROWSER_FILEDETAILS )
#define guFILEITEM_FILE     0
#define guFILEITEM_DIR      1

#define guFILEBROWSER_COLUMN_TYPE       0
#define guFILEBROWSER_COLUMN_NAME       1
#define guFILEBROWSER_COLUMN_SIZE       2
#define guFILEBROWSER_COLUMN_TIME       3

// -------------------------------------------------------------------------------- //
class guFileItem
{
  public :
    int             m_Type;
    wxString        m_Name;
    wxFileOffset    m_Size;
    int             m_Time;
};
WX_DECLARE_OBJARRAY(guFileItem, guFileItemArray);

// -------------------------------------------------------------------------------- //
class guFileBrowserDirCtrl : public wxPanel
{
  protected :
    wxGenericDirCtrl *  m_DirCtrl;

    wxImageList *       GetImageList( void ) { return m_DirCtrl->GetTreeCtrl()->GetImageList(); }

  public :
    guFileBrowserDirCtrl( wxWindow * parent, const wxString &dirpath );
    ~guFileBrowserDirCtrl();

    wxString GetPath( void ) { return m_DirCtrl->GetPath(); }
    void     SetPath( const wxString &path ) { m_DirCtrl->SetPath( path ); }

  friend class guFileBrowserFileCtrl;
  friend class guFileBrowser;
};

// -------------------------------------------------------------------------------- //
class guFilesListBox : public guListView
{
  protected :
    guDbLibrary *       m_Db;
    wxString            m_CurDir;
    guFileItemArray     m_Files;
    wxImageList *       m_TreeImageList;
    int                 m_Order;
    bool                m_OrderDesc;

    virtual void                CreateContextMenu( wxMenu * Menu ) const;
    virtual wxString            OnGetItemText( const int row, const int column ) const;
    virtual void                DrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const;
    virtual void                GetItemsList( void );
    virtual int                 GetSelectedSongs( guTrackArray * tracks ) const;

    virtual int                 GetDragFiles( wxFileDataObject * files );

  public :
    guFilesListBox( wxWindow * parent, guDbLibrary * db );
    ~guFilesListBox();

    virtual void                ReloadItems( bool reset = true );

    virtual wxString inline     GetItemName( const int item ) const;
    virtual int inline          GetItemId( const int item ) const;

    void                        SetOrder( int order );
    void                        SetPath( const wxString &path );
    wxString                    GetPath( const int item );
    void                        SetTreeImageList( wxImageList * imagelist ) { m_TreeImageList = imagelist; }

};

// -------------------------------------------------------------------------------- //
class guFileBrowserFileCtrl : public wxPanel
{
  protected :
    guDbLibrary *           m_Db;
    guFilesListBox *        m_FilesListBox;
    guFileBrowserDirCtrl *  m_DirCtrl;

  public :
    guFileBrowserFileCtrl( wxWindow * parent, guDbLibrary * db, guFileBrowserDirCtrl * dirctrl );
    ~guFileBrowserFileCtrl();

    void                    SetPath( const wxString &path ) { m_FilesListBox->SetPath( path ); }
    const wxString          GetPath( const int item ) { return m_FilesListBox->GetPath( item ); }
    wxArrayInt              GetSelectedItems( void ) { return m_FilesListBox->GetSelectedItems(); }
    void                    SetOrder( const int order ) { m_FilesListBox->SetOrder( order ); }

};

// -------------------------------------------------------------------------------- //
class guFileBrowser : public wxPanel
{
  protected :
    wxAuiManager            m_AuiManager;
    int                     m_VisiblePanels;
	guDbLibrary *           m_Db;
	guPlayerPanel *         m_PlayerPanel;

	guFileBrowserDirCtrl *  m_DirCtrl;
	guFileBrowserFileCtrl * m_FilesCtrl;

    void                    OnDirItemChanged( wxTreeEvent &event );
    void                    OnFileItemActivated( wxListEvent &Event );
    void                    OnFilesColClick( wxListEvent &event );
    void                    OnDirBeginDrag( wxTreeEvent &event );

    DECLARE_EVENT_TABLE()

  public :
    guFileBrowser( wxWindow * parent, guDbLibrary * db, guPlayerPanel * playerpanel );
    ~guFileBrowser();

};

#endif
// -------------------------------------------------------------------------------- //
