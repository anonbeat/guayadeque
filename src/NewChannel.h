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
#ifndef NEWCHANNEL_H
#define NEWCHANNEL_H

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/treectrl.h>
#include <wx/textctrl.h>
#include <wx/statbox.h>
#include <wx/dialog.h>
#include <wx/xml/xml.h>

#include <wx/dynarray.h>

// -------------------------------------------------------------------------------- //
class guNewPodcastItem : public wxTreeItemData
{
  public :
    wxString    m_Name;
    wxString    m_Url;
};
WX_DECLARE_OBJARRAY(guNewPodcastItem, guNewPodcastItemArray);

// -------------------------------------------------------------------------------- //
class guNewPodcastCategory
{
  public :
    wxString                m_Name;
    guNewPodcastItemArray   m_Items;
};
WX_DECLARE_OBJARRAY(guNewPodcastCategory, guNewPodcastChannelArray);

// -------------------------------------------------------------------------------- //
// guPLNamesTreeCtrl
// -------------------------------------------------------------------------------- //
class guPodcastTreeCtrl : public wxTreeCtrl
{
  private :
    wxImageList *               m_ImageList;
    wxTreeItemId                m_RootId;
    guNewPodcastChannelArray *  m_NewPodcasts;
    int                         m_NewItemsCount;

  public :
    guPodcastTreeCtrl( wxWindow * parent, guNewPodcastChannelArray * newpodcasts );
    ~guPodcastTreeCtrl();

    void ReloadItems( void );
    int  GetCategoryCount( void ) { return m_NewPodcasts->Count(); }
    int  GetItemsCount( void ) { return m_NewItemsCount; }
    void ExpandRoot( void ) { }

};

// -------------------------------------------------------------------------------- //
class guNewPodcastChannelSelector : public wxDialog
{
  private:

  protected:
    wxStaticText *              m_DirectoryInfoStaticText;
    wxBitmapButton *            m_DirectoryReload;
    wxBitmapButton *            m_FilterDirectory;
    guPodcastTreeCtrl *         m_DirectoryTreeCtrl;
    wxTextCtrl *                m_UrlTextCtrl;
    guNewPodcastChannelArray    m_NewPodcasts;
    wxArrayString               m_Filters;

    void OnFilterDirectoryClicked( wxCommandEvent& event );
    void OnReloadDirectoryClicked( wxCommandEvent& event );
    void OnDirectoryItemSelected( wxTreeEvent& event );
    void OnDirectoryItemChanged( wxTreeEvent& event );

    void LoadPodcastDirectory( void );
    int ReadNewPodcastChannel( wxXmlNode * XmlNode, guNewPodcastCategory * podcastchannel );
    int ReadNewPodcastChannels( wxXmlNode * XmlNode );
    int LoadNewPodcastsFromXml( const wxString &filename );

  public:
    guNewPodcastChannelSelector( wxWindow * parent );
    ~guNewPodcastChannelSelector();
    wxString GetValue( void );

};


#endif
// -------------------------------------------------------------------------------- //
