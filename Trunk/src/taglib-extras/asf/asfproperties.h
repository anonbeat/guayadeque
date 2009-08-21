/**************************************************************************
    copyright            : (C) 2005-2007 by Lukáš Lalinský
    email                : lalinsky@gmail.com
 **************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License version   *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  *
 *   USA                                                                   *
 *                                                                         *
 *   Alternatively, this file is available under the Mozilla Public        *
 *   License Version 1.1.  You may obtain a copy of the License at         *
 *   http://www.mozilla.org/MPL/                                           *
 ***************************************************************************/

#ifndef TAGLIB_ASFPROPERTIES_H
#define TAGLIB_ASFPROPERTIES_H

#include <audioproperties.h>
#include <tstring.h>
#include <taglib-extras_export.h>

namespace TagLib {

  namespace ASF {

    //! An implementation of ASF audio properties
    class TAGLIB_EXTRAS_EXPORT Properties : public AudioProperties
    {
    public:

      /*!
       * Create an instance of ASF::Properties.
       */
      Properties();

      /*!
       * Destroys this ASF::Properties instance.
       */
      virtual ~Properties();

      // Reimplementations.
      virtual int length() const;
      virtual int bitrate() const;
      virtual int sampleRate() const;
      virtual int channels() const;

#ifndef DO_NOT_DOCUMENT
      void setLength(int value);
      void setBitrate(int value);
      void setSampleRate(int value);
      void setChannels(int value);
#endif

    private:
      class PropertiesPrivate;
      PropertiesPrivate *d;
    };

  }

}

#endif 
