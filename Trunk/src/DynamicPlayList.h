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
#ifndef DYNAMICPLAYLIST_H
#define DYNAMICPLAYLIST

#include "RatingCtrl.h"

#include <wx/dynarray.h>
#include <wx/arrimpl.cpp>
#include <wx/string.h>
#include <wx/listbox.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/stattext.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/dialog.h>


#define guDYNAMIC_FILTER_TYPE_TITLE                     0
#define guDYNAMIC_FILTER_TYPE_ARTIST                    1
#define guDYNAMIC_FILTER_TYPE_ALBUM                     2
#define guDYNAMIC_FILTER_TYPE_GENRE                     3
#define guDYNAMIC_FILTER_TYPE_LABEL                     4
#define guDYNAMIC_FILTER_TYPE_PATH                      5
#define guDYNAMIC_FILTER_TYPE_YEAR                      6
#define guDYNAMIC_FILTER_TYPE_RATING                    7
#define guDYNAMIC_FILTER_TYPE_LENGTH                    8
#define guDYNAMIC_FILTER_TYPE_PLAYCOUNT                 9
#define guDYNAMIC_FILTER_TYPE_LASTPLAY                  10
#define guDYNAMIC_FILTER_TYPE_ADDEDDATE                 11

#define guDYNAMIC_FILTER_ORDER_TITLE                    0
#define guDYNAMIC_FILTER_ORDER_ARTIST                   1
#define guDYNAMIC_FILTER_ORDER_ALBUM                    2
#define guDYNAMIC_FILTER_ORDER_GENRE                    3
#define guDYNAMIC_FILTER_ORDER_LABEL                    4
//#define guDYNAMIC_FILTER_ORDER_PATH                     5
#define guDYNAMIC_FILTER_ORDER_YEAR                     5
#define guDYNAMIC_FILTER_ORDER_RATING                   6
#define guDYNAMIC_FILTER_ORDER_LENGTH                   7
#define guDYNAMIC_FILTER_ORDER_PLAYCOUNT                8
#define guDYNAMIC_FILTER_ORDER_LASTPLAY                 9
#define guDYNAMIC_FILTER_ORDER_ADDEDDATE                10
#define guDYNAMIC_FILTER_ORDER_RANDOM                   11

#define guDYNAMIC_FILTER_OPTION_STRING_CONTAINS         0
#define guDYNAMIC_FILTER_OPTION_STRING_NOT_CONTAINS     1
#define guDYNAMIC_FILTER_OPTION_STRING_EQUAL            2
#define guDYNAMIC_FILTER_OPTION_STRING_START_WITH       3
#define guDYNAMIC_FILTER_OPTION_STRING_ENDS_WITH        4

#define guDYNAMIC_FILTER_OPTION_YEAR_EQUAL              0
#define guDYNAMIC_FILTER_OPTION_YEAR_AFTER              1
#define guDYNAMIC_FILTER_OPTION_YEAR_BEFORE             2

#define guDYNAMIC_FILTER_OPTION_NUMERIC_EQUALS          0
#define guDYNAMIC_FILTER_OPTION_NUMERIC_AT_LEAST        1
#define guDYNAMIC_FILTER_OPTION_NUMERIC_AT_MOST         2

#define guDYNAMIC_FILTER_OPTION_DATE_IN_THE_LAST        0
#define guDYNAMIC_FILTER_OPTION_DATE_BEFORE_THE_LAST    1

#define guDYNAMIC_FILTER_LIMIT_TRACKS                   0
#define guDYNAMIC_FILTER_LIMIT_MINUTES                  1
#define guDYNAMIC_FILTER_LIMIT_MEGABYTES                2
#define guDYNAMIC_FILTER_LIMIT_GIGABYTES                3


// -------------------------------------------------------------------------------- //
class guFilterItem
{
  public :
    int         m_Type;
    int         m_Option;
    wxString    m_Text;
    int         m_Number;
    int         m_Option2;
    wxString    m_Label;

    guFilterItem() { m_Type = 0; };
    ~guFilterItem() {};
    void        Set( int type, int option, const wxString &text );
    void        Set( int type, int option, int number, int option2 );
    wxString    GetLabel( void ) { return m_Label; };
    void        SetFilterLabel( void );

};
WX_DECLARE_OBJARRAY(guFilterItem, guFilterItemArray);

// -------------------------------------------------------------------------------- //
class guDynPlayList
{
  public :
    int                 m_Id;
    wxString            m_Name;
    guFilterItemArray   m_Filters;
    bool                m_Limited;
    int                 m_LimitValue;
    int                 m_LimitType;
    bool                m_Sorted;
    int                 m_SortType;
    bool                m_SortDesc;
    bool                m_AnyOption;

    guDynPlayList() { m_Id = 0; m_Limited = false; m_LimitValue = 0; m_LimitType = 0;
                      m_Sorted = false; m_SortType = 0; m_SortDesc = false; m_AnyOption = false; };
    ~guDynPlayList() {};
};

// -------------------------------------------------------------------------------- //
class guDynPlayListEditor : public wxDialog
{
	private:

	protected:
        guDynPlayList *     m_PlayList;
        guFilterItemArray * m_Filters;
        //guFilterItem        m_FilterEdit;
        int                 m_CurFilter;
        bool                m_HasChanged;

		wxListBox * m_FiltersListBox;
		wxChoice * m_FilterFieldChoice;
		wxChoice * m_FilterOptionChoice;
		wxTextCtrl * m_FilterText;
		guRating * m_FilterRating;
		wxChoice * m_FilterDateOption2Choice;
		wxSpinCtrl * m_LengthHours;
		wxStaticText * m_LengthSeparator1;
		wxSpinCtrl * m_LengthMinutes;
		wxStaticText * m_LengthSeparator2;
		wxSpinCtrl * m_LengthSeconds;
        wxBoxSizer * m_FilterEditSizer;
		wxBitmapButton * m_FilterAdd;
		wxBitmapButton * m_FilterDel;
		wxBitmapButton * m_FilterAccept;
		wxCheckBox * m_LimitCheckBox;
		wxSpinCtrl * m_LimitSpinCtrl;
		wxChoice * m_LimitChoice;
		wxCheckBox * m_SortCheckBox;
		wxChoice * m_SortChoice;
		wxCheckBox * m_DescCheckBox;

		wxCheckBox * m_AddOnAnyCheckBox;

		wxButton * m_BtnOk;

		// event handlers, overide them in your derived class
		void OnFiltersSelected( wxCommandEvent &event );
		void OnFilterFieldSelected( wxCommandEvent& event );
		void OnFilterOptionSelected( wxCommandEvent& event );
		void OnFilterTextChanged( wxCommandEvent& event );
        void OnFilterDateOption2Selected( wxCommandEvent &event );
		void OnFilterAddClicked( wxCommandEvent& event );
		void OnFilterDelClicked( wxCommandEvent& event );
		void OnFilterUpdateClicked( wxCommandEvent& event );
		void OnLImitChecked( wxCommandEvent& event );
		void OnSortChecked( wxCommandEvent& event );
		void OnRatingChanged( guRatingEvent &event );

		bool FilterHasChanged( void );
		guFilterItem GetFilterItem( void );
		void UpdateEditor( int FilterType );


	public:
		guDynPlayListEditor( wxWindow * parent, guDynPlayList * playlist );
		~guDynPlayListEditor();

		void FillPlayListEditData( void );

};

#endif
// -------------------------------------------------------------------------------- //
