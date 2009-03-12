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
#include "TagInfo.h"
#include "Utils.h"

#include <wx/tokenzr.h>

// -------------------------------------------------------------------------------- //
bool TagInfo::ReadID3Tags( const ID3_Tag * tag )
{
  char * pStr;
  if( m_TrackName.IsEmpty() )
  {
    pStr = ID3_GetTitle( tag );
    m_TrackName = wxString( pStr, wxConvISO8859_1 );
    ID3_FreeString( pStr );
  }

  if( m_ArtistName.IsEmpty() )
  {
    pStr = ID3_GetArtist( tag );
    m_ArtistName = wxString( pStr, wxConvISO8859_1 );
    ID3_FreeString( pStr );
  }

  if( m_AlbumName.IsEmpty() )
  {
    pStr = ID3_GetAlbum( tag );
    m_AlbumName = wxString( pStr, wxConvISO8859_1 );
    ID3_FreeString( pStr );
  }

  if( m_Track == 0 )
  {
    m_Track = ID3_GetTrackNum( tag );
  }

  if( m_Year == 0 )
  {
    pStr = ID3_GetYear( tag );
    wxString( pStr, wxConvISO8859_1 ).ToLong( ( long int * ) &m_Year );
    ID3_FreeString( pStr );
  }

  if( m_GenreName.IsEmpty() )
  {
    pStr = ID3_GetGenre( tag );
    //printf( "GenreTag : %s\n", pStr );
    m_GenreName = wxString( pStr, wxConvISO8859_1 );
//    if( GenreName.IsEmpty() )
//      printf( "Genre Error '%s'\n", pStr );
    ID3_FreeString( pStr );
    if( m_GenreName[ 0 ] == '(' )
    {
      int GenreIndex;
      m_GenreName.AfterFirst( '(' ).BeforeFirst( ')' ).ToLong( ( long int * ) &GenreIndex );
      m_GenreName = wxString( ID3_V1GENRE2DESCRIPTION( GenreIndex ), wxConvISO8859_1 );
      //guLogMessage( wxT( "Genre is a index ID3v1 '%s'" ), GenreName.c_str() );
    }
  }

  if( m_Length == 0 )
  {
    const Mp3_Headerinfo * mp3info;
    mp3info = tag->GetMp3HeaderInfo();
    if( mp3info )
    {
      m_Length = mp3info->time;
    }
  }

  if( m_TrackLabels.Count() == 0 )
  {
    ID3_Frame * Frame = tag->Find( ID3FID_USERTEXT, ID3FN_DESCRIPTION, "guTRLABELS" );
    if( Frame )
    {
      char Buffer[ 1024 ];
      Frame->Field( ID3FN_TEXT ).Get( Buffer, sizeof( Buffer ) );
      m_TrackLabelsStr = wxString( Buffer, wxConvUTF8 );
      m_TrackLabels = wxStringTokenize( m_TrackLabelsStr, wxT( "|" ) );
    }
  }

  if( m_ArtistLabels.Count() == 0 )
  {
    ID3_Frame * Frame = tag->Find( ID3FID_USERTEXT, ID3FN_DESCRIPTION, "guARLABELS" );
    if( Frame )
    {
      char Buffer[ 1024 ];
      Frame->Field( ID3FN_TEXT ).Get( Buffer, sizeof( Buffer ) );
      m_ArtistLabelsStr = wxString( Buffer, wxConvUTF8 );
      m_ArtistLabels = wxStringTokenize( m_ArtistLabelsStr, wxT( "|" ) );
    }
  }

  if( m_AlbumLabels.Count() == 0 )
  {
    ID3_Frame * Frame = tag->Find( ID3FID_USERTEXT, ID3FN_DESCRIPTION, "guALLABELS" );
    if( Frame )
    {
      char Buffer[ 1024 ];
      Frame->Field( ID3FN_TEXT ).Get( Buffer, sizeof( Buffer ) );
      m_AlbumLabelsStr = wxString( Buffer, wxConvUTF8 );
      m_AlbumLabels = wxStringTokenize( m_AlbumLabelsStr, wxT( "|" ) );
    }
  }

  return true;
}

// -------------------------------------------------------------------------------- //
size_t ID3_GetGenreIndex( char * GenreName )
{
    size_t index;
    if( GenreName && strlen( GenreName ) )
    {
        for( index = 0; index < ID3_NR_OF_V1_GENRES; index++ )
        {
            if( !strcasecmp( GenreName, ID3_v1_genre_description[ index ] ) )
            {
                return index;
            }
        }
    }
    return 0xFF;
}

// -------------------------------------------------------------------------------- //
ID3_Frame * TagInfo::Find( ID3_Tag * tag, const char * Name )
{
    ID3_Frame * RetVal = NULL;
    ID3_Frame * Frame;
    ID3_Tag::Iterator * iter;

    iter = tag->CreateIterator();
    while( !RetVal && ( Frame = iter->GetNext() ) )
    {
      if( Frame->GetID() == ID3FID_USERTEXT )
      {
        char * sDesc = ID3_GetString( Frame, ID3FN_DESCRIPTION );
        if( !strcmp( sDesc, Name ) )
        {
            RetVal = Frame;
        }
        delete [] sDesc;
      }
    }
    delete iter;

    return RetVal;
}

// -------------------------------------------------------------------------------- //
int TagInfo::RemoveLabelFrame( ID3_Tag * tag, const char * Name )
{
    int RetVal = 0;
    ID3_Frame * Frame;
    ID3_Tag::Iterator * iter;

    iter = tag->CreateIterator();
    while( ( Frame = iter->GetNext() ) )
    {
      if( Frame->GetID() == ID3FID_USERTEXT )
      {
        //char * sText = ID3_GetString( Frame, ID3FN_TEXT );
        char * sDesc = ID3_GetString( Frame, ID3FN_DESCRIPTION );
        if( !strcmp( sDesc, Name ) )
        {
          tag->RemoveFrame( Frame );
          RetVal++;
        }
        //delete [] sText;
        delete [] sDesc;
      }
    }
    delete iter;

    return RetVal;
}

// -------------------------------------------------------------------------------- //
bool TagInfo::WriteID3Tags( ID3_Tag * tag )
{
  ID3_Frame * frame;

  // Title
  if( !m_TrackName.IsEmpty() )
  {
    ID3_AddTitle( tag, m_TrackName.char_str( wxConvISO8859_1 ), true );
  }

  // Artist
  if( !m_ArtistName.IsEmpty() )
  {
    ID3_AddArtist( tag, m_ArtistName.char_str( wxConvISO8859_1 ), true );
  }

  // Album
  if( !m_AlbumName.IsEmpty() )
  {
    ID3_AddAlbum( tag, m_AlbumName.char_str( wxConvISO8859_1 ), true );
  }

  // Track
  if( m_Track > 0 )
  {
    ID3_RemoveTracks( tag );
    frame = new ID3_Frame( ID3FID_TRACKNUM );
    if( frame )
    {
      frame->GetField( ID3FN_TEXT )->Set( wxString::Format( wxT( "%02u" ), m_Track ).char_str() );
      tag->AttachFrame( frame );
    }
  }

  // Year
  if( m_Year > 0 )
  {
    ID3_AddYear( tag, wxString::Format( wxT( "%u" ), m_Year ).char_str(), true );
  }

  // Genre
  if( !m_GenreName.IsEmpty() )
  {
    ID3_RemoveGenres( tag );
    int GenreIndex = ID3_GetGenreIndex( m_GenreName.char_str( wxConvISO8859_1 ) );
    // IDV1 Genres is index to table If valid but not in table must be 0xFF
    frame = new ID3_Frame( ID3FID_CONTENTTYPE );
    if( frame )
    {
      if( GenreIndex == 0xFF )
      {
        frame->GetField( ID3FN_TEXT )->Set( m_GenreName.char_str( wxConvISO8859_1 ) );
      }
      else
      {
        frame->GetField( ID3FN_TEXT )->Set( wxString::Format( wxT( "(%u)" ), GenreIndex ).char_str( wxConvISO8859_1 ) );
      }
      tag->AttachFrame( frame );
    }
  }

  // The Labels
  if( m_ArtistLabelsStr.IsEmpty() )
  {
    if( m_ArtistLabels.Count() )
    {
      int index;
      int count = m_ArtistLabels.Count();
      for( index = 0; index < count; index++ )
      {
        m_ArtistLabelsStr += m_ArtistLabels[ index ];
        m_ArtistLabelsStr += wxT( "|" );
      }
      m_ArtistLabelsStr.RemoveLast( 1 );
    }
  }


//  if( !ArtistLabelsStr.IsEmpty() )
  {
    frame = Find( tag, "guARLABELS" );
    if( !frame )
    {
      frame = new ID3_Frame( ID3FID_USERTEXT );
      frame->Field( ID3FN_TEXTENC ) = ID3TE_ASCII;
      frame->Field( ID3FN_DESCRIPTION ) = "guARLABELS";
      tag->AttachFrame( frame );
    }
    frame->Field( ID3FN_TEXT ) = m_ArtistLabelsStr.char_str();
  }


  if( m_AlbumLabelsStr.IsEmpty() )
  {
    if( m_AlbumLabels.Count() )
    {
      int index;
      int count = m_AlbumLabels.Count();
      for( index = 0; index < count; index++ )
      {
        m_AlbumLabelsStr += m_AlbumLabels[ index ];
        m_AlbumLabelsStr += wxT( "|" );
      }
      m_AlbumLabelsStr.RemoveLast( 1 );
    }
  }


//  if( !AlbumLabelsStr.IsEmpty() )
  {
    frame = Find( tag, "guALLABELS" );
    if( !frame )
    {
      //guLogMessage( wxT( "Creating guALLABELS Frame..." ) );
      frame = new ID3_Frame( ID3FID_USERTEXT );
      frame->Field( ID3FN_TEXTENC ) = ID3TE_ASCII;
      frame->Field( ID3FN_DESCRIPTION ) = "guALLABELS";
      tag->AttachFrame( frame );
    }
    frame->Field( ID3FN_TEXT ) = m_AlbumLabelsStr.char_str();
  }


  if( m_TrackLabelsStr.IsEmpty() )
  {
    if( m_TrackLabels.Count() )
    {
      int index;
      int count = m_TrackLabels.Count();
      for( index = 0; index < count; index++ )
      {
        m_TrackLabelsStr += m_TrackLabels[ index ];
        m_TrackLabelsStr += wxT( "|" );
      }
      m_TrackLabelsStr.RemoveLast( 1 );
    }
  }


//  if( !TrackLabelsStr.IsEmpty() )
  {
    frame = Find( tag, "guTRLABELS" );
    if( !frame )
    {
      frame = new ID3_Frame( ID3FID_USERTEXT );
      frame->Field( ID3FN_TEXTENC ) = ID3TE_ASCII;
      frame->Field( ID3FN_DESCRIPTION ) = "guTRLABELS";
      tag->AttachFrame( frame );
    }
    frame->Field( ID3FN_TEXT ) = m_TrackLabelsStr.char_str();
  }

  tag->SetPadding( false );
  tag->Update();

  return true;
}

// -------------------------------------------------------------------------------- //
