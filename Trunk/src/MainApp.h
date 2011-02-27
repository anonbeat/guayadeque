// -------------------------------------------------------------------------------- //
//	Copyright (C) 2008-2011 J.Rios
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
#ifndef MAINAPP_H
#define MAINAPP_H

#include "Config.h"
#include "DbLibrary.h"
#include "DbCache.h"

#include <wx/app.h>
#include <wx/snglinst.h>

// -------------------------------------------------------------------------------- //
class guMainApp : public wxApp
{
  protected :
    guDbLibrary *               m_Db;
    guDbCache *                 m_DbCache;
    guConfig *                  m_Config;
    wxSingleInstanceChecker *   m_SingleInstanceChecker;
    wxLocale                    m_Locale;

  public:
    guMainApp();
    ~guMainApp();
    virtual bool OnInit();
    virtual int  OnExit();
    void OnFatalException();
    wxLocale * GetLocale() { return &m_Locale; };

};

#endif
// -------------------------------------------------------------------------------- //
