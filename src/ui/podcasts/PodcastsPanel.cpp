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
#include "PodcastsPanel.h"

#include "Accelerators.h"
#include "AuiDockArt.h"
#include "ChannelEditor.h"
#include "EventCommandIds.h"
#include "Config.h"
#include "Images.h"
#include "MainFrame.h"
#include "NewChannel.h"
#include "Utils.h"

#include <wx/regex.h>
#include <wx/sstream.h>
#include <wx/uri.h>
#include <wx/xml/xml.h>
#include <wx/zstream.h>

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
// guDbPodcasts
// -------------------------------------------------------------------------------- //
guDbPodcasts::guDbPodcasts( const wxString &dbname ) : guDb( dbname )
{
    wxArrayString query;

    query.Add( wxT( "CREATE TABLE IF NOT EXISTS podcastchs( podcastch_id INTEGER PRIMARY KEY AUTOINCREMENT, " \
                  "podcastch_url VARCHAR, podcastch_title VARCHAR COLLATE NOCASE, podcastch_description VARCHAR, " \
                  "podcastch_language VARCHAR, podcastch_time INTEGER, podcastch_sumary VARCHAR, " \
                  "podcastch_author VARCHAR, podcastch_ownername VARCHAR, podcastch_owneremail VARCHAR, " \
                  "podcastch_category VARCHAR, podcastch_image VARCHAR, podcastch_downtype INTEGER, " \
                  "podcastch_downtext VARCHAR, podcastch_allowdel BOOLEAN );" ) );
    //query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'podcastch_id' on podcastchs(podcastch_id ASC);" ) );
    query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'podcastch_title' on podcastchs(podcastch_title ASC);" ) );
    query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'podcastch_url' on podcastchs(podcastch_url ASC);" ) );

    query.Add( wxT( "CREATE TABLE IF NOT EXISTS podcastitems( podcastitem_id INTEGER PRIMARY KEY AUTOINCREMENT, " \
                  "podcastitem_chid INTEGER, podcastitem_title VARCHAR COLLATE NOCASE, podcastitem_summary VARCHAR, " \
                  "podcastitem_author VARCHAR COLLATE NOCASE, podcastitem_enclosure VARCHAR, podcastitem_time INTEGER, " \
                  "podcastitem_file VARCHAR, podcastitem_filesize INTEGER, podcastitem_length INTEGER, " \
                  "podcastitem_addeddate INTEGER, podcastitem_playcount INTEGER, " \
                  "podcastitem_lastplay INTEGER, podcastitem_status INTEGER );" ) );
    //query.Add( wxT( "CREATE UNIQUE INDEX IF NOT EXISTS 'podcastitem_id' on podcastitems(podcastitem_id ASC);" ) );
    query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'podcastitem_title' on podcastitems(podcastitem_title ASC);" ) );
    query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'podcastitem_file' on podcastitems(podcastitem_file ASC);" ) );
    query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'podcastitem_chid' on podcastitems(podcastitem_chid ASC);" ) );
    query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'podcastitem_time' on podcastitems(podcastitem_time ASC);" ) );
    query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'podcastitem_enclosure' on podcastitems(podcastitem_enclosure ASC);" ) );
    query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'podcastitem_author' on podcastitems(podcastitem_author ASC);" ) );
    query.Add( wxT( "CREATE INDEX IF NOT EXISTS 'podcastitem_length' on podcastitems(podcastitem_length ASC);" ) );


    int Index;
    int Count = query.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        ExecuteUpdate( query[ Index ] );
    }
}

// -------------------------------------------------------------------------------- //
guDbPodcasts::~guDbPodcasts()
{
}

// -------------------------------------------------------------------------------- //
int guDbPodcasts::GetPodcastChannels( guPodcastChannelArray * channels )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxT( "SELECT podcastch_id, podcastch_url, podcastch_title, podcastch_description, " \
               "podcastch_language, podcastch_sumary, " \
               "podcastch_author, podcastch_ownername, podcastch_owneremail, " \
               "podcastch_category, podcastch_image, " \
               "podcastch_downtype, podcastch_downtext, podcastch_allowdel " \
               "FROM podcastchs " \
               "ORDER BY podcastch_title" );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    guPodcastChannel * Channel = new guPodcastChannel();
    Channel->m_Id = dbRes.GetInt( 0 );
    Channel->m_Url = dbRes.GetString( 1 );
    Channel->m_Title = dbRes.GetString( 2 );
    Channel->m_Description = dbRes.GetString( 3 );
    Channel->m_Lang = dbRes.GetString( 4 );
    Channel->m_Summary = dbRes.GetString( 5 );
    Channel->m_Author = dbRes.GetString( 6 );
    Channel->m_OwnerName = dbRes.GetString( 7 );
    Channel->m_OwnerEmail = dbRes.GetString( 8 );
    Channel->m_Category = dbRes.GetString( 9 );
    Channel->m_Image = dbRes.GetString( 10 );
    Channel->m_DownloadType = dbRes.GetInt( 11 );
    Channel->m_DownloadText = dbRes.GetString( 12 );
    Channel->m_AllowDelete = dbRes.GetBool( 13 );
    channels->Add( Channel );
  }
  dbRes.Finalize();
  return channels->Count();
}

// -------------------------------------------------------------------------------- //
void guDbPodcasts::SavePodcastChannel( guPodcastChannel * channel, bool onlynew )
{
  wxString query;
  int ChannelId;
  if( ( ChannelId = GetPodcastChannelUrl( channel->m_Url ) ) == wxNOT_FOUND )
  {
    query = wxString::Format( wxT( "INSERT INTO podcastchs( podcastch_id, podcastch_url, podcastch_title, " \
        "podcastch_description, podcastch_language, podcastch_time, podcastch_sumary, " \
        "podcastch_author, podcastch_ownername, podcastch_owneremail, " \
        "podcastch_category, podcastch_image, " \
        "podcastch_downtype, podcastch_downtext, podcastch_allowdel ) " \
        "VALUES( NULL, '%s', '%s', " \
        "'%s', '%s', 0, '%s', " \
        "'%s', '%s', '%s', " \
        "'%s', '%s', %u, '%s', %u );" ),
        escape_query_str( channel->m_Url ).c_str(),
        escape_query_str( channel->m_Title ).c_str(),
        escape_query_str( channel->m_Description ).c_str(),
        escape_query_str( channel->m_Lang ).c_str(),
        escape_query_str( channel->m_Summary ).c_str(),
        escape_query_str( channel->m_Author ).c_str(),
        escape_query_str( channel->m_OwnerName ).c_str(),
        escape_query_str( channel->m_OwnerEmail ).c_str(),
        escape_query_str( channel->m_Category ).c_str(),
        escape_query_str( channel->m_Image ).c_str(),
        channel->m_DownloadType,
        escape_query_str( channel->m_DownloadText ).c_str(),
        channel->m_AllowDelete );

    ExecuteUpdate( query );
    ChannelId = GetLastRowId();
    channel->m_Id = ChannelId;
  }
  else if( !onlynew )
  {
    query = wxString::Format( wxT( "UPDATE podcastchs " \
        "SET podcastch_url = '%s', podcastch_title = '%s', " \
        "podcastch_description = '%s', podcastch_language = '%s', podcastch_sumary = '%s', " \
        "podcastch_author = '%s', podcastch_ownername = '%s', podcastch_owneremail = '%s', " \
        "podcastch_category = '%s', podcastch_image  = '%s', " \
        "podcastch_downtype = %u, podcastch_downtext = '%s', podcastch_allowdel = %u " \
        "WHERE podcastch_id = %u" ),
        escape_query_str( channel->m_Url ).c_str(),
        escape_query_str( channel->m_Title ).c_str(),
        escape_query_str( channel->m_Description ).c_str(),
        escape_query_str( channel->m_Lang ).c_str(),
        escape_query_str( channel->m_Summary ).c_str(),
        escape_query_str( channel->m_Author ).c_str(),
        escape_query_str( channel->m_OwnerName ).c_str(),
        escape_query_str( channel->m_OwnerEmail ).c_str(),
        escape_query_str( channel->m_Category ).c_str(),
        escape_query_str( channel->m_Image ).c_str(),
        channel->m_DownloadType,
        escape_query_str( channel->m_DownloadText ).c_str(),
        channel->m_AllowDelete,
        channel->m_Id );
    ExecuteUpdate( query );

    ChannelId = channel->m_Id;
  }

  // Save the Items
  SavePodcastItems( ChannelId, &channel->m_Items, onlynew );

}

// -------------------------------------------------------------------------------- //
int guDbPodcasts::SavePodcastChannels( guPodcastChannelArray * channels, bool onlynew )
{
    int Index;
    int Count = channels->Count();
    for( Index = 0; Index < Count; Index++ )
    {
        SavePodcastChannel( &channels->Item( Index ), onlynew );
    }
    return 1;
}

// -------------------------------------------------------------------------------- //
int guDbPodcasts::GetPodcastChannelUrl( const wxString &url, guPodcastChannel * channel )
{
  int RetVal = wxNOT_FOUND;
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxString::Format( wxT( "SELECT podcastch_id, podcastch_url, podcastch_title, podcastch_description, " \
               "podcastch_language, podcastch_sumary, " \
               "podcastch_author, podcastch_ownername, podcastch_owneremail, " \
               "podcastch_category, podcastch_image, " \
               "podcastch_downtype, podcastch_downtext, podcastch_allowdel " \
               "FROM podcastchs " \
               "WHERE podcastch_url = '%s' LIMIT 1;" ),
               escape_query_str( url ).c_str() );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    RetVal = dbRes.GetInt( 0 );
    if( channel )
    {
      channel->m_Id = RetVal;
      channel->m_Url = dbRes.GetString( 1 );
      channel->m_Title = dbRes.GetString( 2 );
      channel->m_Description = dbRes.GetString( 3 );
      channel->m_Lang = dbRes.GetString( 4 );
      channel->m_Summary = dbRes.GetString( 5 );
      channel->m_Author = dbRes.GetString( 6 );
      channel->m_OwnerName = dbRes.GetString( 7 );
      channel->m_OwnerEmail = dbRes.GetString( 8 );
      channel->m_Category = dbRes.GetString( 9 );
      channel->m_Image = dbRes.GetString( 10 );
      channel->m_DownloadType = dbRes.GetInt( 11 );
      channel->m_DownloadText = dbRes.GetString( 12 );
      channel->m_AllowDelete = dbRes.GetBool( 13 );
    }
  }
  dbRes.Finalize();
  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbPodcasts::GetPodcastChannelId( const int id, guPodcastChannel * channel )
{
  int RetVal = wxNOT_FOUND;
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxString::Format( wxT( "SELECT podcastch_id, podcastch_url, podcastch_title, podcastch_description, " \
               "podcastch_language, podcastch_sumary, " \
               "podcastch_author, podcastch_ownername, podcastch_owneremail, " \
               "podcastch_category, podcastch_image, " \
               "podcastch_downtype, podcastch_downtext, podcastch_allowdel " \
               "FROM podcastchs " \
               "WHERE podcastch_id = %u LIMIT 1;" ),
               id );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    RetVal = dbRes.GetInt( 0 );
    if( channel )
    {
      channel->m_Id = RetVal;
      channel->m_Url = dbRes.GetString( 1 );
      channel->m_Title = dbRes.GetString( 2 );
      channel->m_Description = dbRes.GetString( 3 );
      channel->m_Lang = dbRes.GetString( 4 );
      channel->m_Summary = dbRes.GetString( 5 );
      channel->m_Author = dbRes.GetString( 6 );
      channel->m_OwnerName = dbRes.GetString( 7 );
      channel->m_OwnerEmail = dbRes.GetString( 8 );
      channel->m_Category = dbRes.GetString( 9 );
      channel->m_Image = dbRes.GetString( 10 );
      channel->m_DownloadType = dbRes.GetInt( 11 );
      channel->m_DownloadText = dbRes.GetString( 12 );
      channel->m_AllowDelete = dbRes.GetBool( 13 );
    }
  }
  dbRes.Finalize();
  return RetVal;
}

// -------------------------------------------------------------------------------- //
void guDbPodcasts::DelPodcastChannel( const int id )
{
  wxString query;

  query = wxString::Format( wxT( "DELETE FROM podcastchs WHERE podcastch_id = %u;" ), id );

  ExecuteUpdate( query );

  DelPodcastItems( id );
}

// -------------------------------------------------------------------------------- //
int guDbPodcasts::GetPodcastItems( guPodcastItemArray * items, const wxArrayInt &filters, const int order, const bool desc )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxT( "SELECT podcastitem_id, podcastitem_chid, podcastitem_title, " \
            "podcastitem_summary, podcastitem_author, podcastitem_enclosure, podcastitem_time, " \
            "podcastitem_file, podcastitem_filesize, podcastitem_length, " \
            "podcastitem_playcount, podcastitem_addeddate, podcastitem_lastplay, " \
            "podcastitem_status, " \
            "podcastch_title, podcastch_category " \
            "FROM podcastitems, podcastchs " \
            "WHERE podcastitem_chid = podcastch_id AND podcastitem_status != 4" ); // dont get the deleted items

  if( filters.Count() )
  {
        query += wxT( " AND " ) + ArrayToFilter( filters, wxT( "podcastitem_chid" ) );
  }

  query += wxT( " ORDER BY " );

  switch( order )
  {
      case guPODCASTS_COLUMN_TITLE :
        query += wxT( "podcastitem_title COLLATE NOCASE" );
        break;
      case guPODCASTS_COLUMN_CHANNEL :
        query += wxT( "podcastch_title COLLATE NOCASE" );
        break;
      case guPODCASTS_COLUMN_CATEGORY :
        query += wxT( "podcastch_category COLLATE NOCASE" );
        break;
      case guPODCASTS_COLUMN_DATE :
        query += wxT( "podcastitem_time" );
        break;
      case guPODCASTS_COLUMN_LENGTH :
        query += wxT( "podcastitem_length" );
        break;
      case guPODCASTS_COLUMN_AUTHOR :
        query += wxT( "podcastitem_author COLLATE NOCASE" );
        break;
      case guPODCASTS_COLUMN_PLAYCOUNT :
        query += wxT( "podcastitem_playcount" );
        break;
      case guPODCASTS_COLUMN_LASTPLAY :
        query += wxT( "podcastitem_lastplay" );
        break;
      case guPODCASTS_COLUMN_ADDEDDATE :
        query += wxT( "podcastitem_addeddate" );
        break;
      case guPODCASTS_COLUMN_STATUS :
        query += wxT( "podcastitem_status" );
        break;
  }

  if( desc )
    query += wxT( " DESC;" );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    guPodcastItem * Item = new guPodcastItem();
    Item->m_Id = dbRes.GetInt( 0 );
    Item->m_ChId = dbRes.GetInt( 1 );
    Item->m_Title = dbRes.GetString( 2 );
    Item->m_Summary = dbRes.GetString( 3 );
    Item->m_Author = dbRes.GetString( 4 );
    Item->m_Enclosure = dbRes.GetString( 5 );
    Item->m_Time = dbRes.GetInt( 6 );
    Item->m_FileName = dbRes.GetString( 7 );
    Item->m_FileSize = dbRes.GetInt( 8 );
    Item->m_Length = dbRes.GetInt( 9 );
    Item->m_PlayCount = dbRes.GetInt( 10 );
    Item->m_AddedDate = dbRes.GetInt( 11 );
    Item->m_LastPlay = dbRes.GetInt( 12 );
    Item->m_Status = dbRes.GetInt( 13 );

    Item->m_Channel = dbRes.GetString( 14 );
    Item->m_Category = dbRes.GetString( 15 );
    items->Add( Item );
  }
  dbRes.Finalize();
  return items->Count();
}

// -------------------------------------------------------------------------------- //
void guDbPodcasts::GetPodcastCounters( const wxArrayInt &filters, wxLongLong * count, wxLongLong * len, wxLongLong * size )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxT( "SELECT COUNT(), SUM( podcastitem_length ), SUM( podcastitem_filesize ) "
            "FROM podcastitems, podcastchs "
            "WHERE podcastitem_chid = podcastch_id AND podcastitem_status != 4" ); // dont get the deleted items

  if( filters.Count() )
  {
        query += wxT( " AND " ) + ArrayToFilter( filters, wxT( "podcastitem_chid" ) );
  }

  dbRes = ExecuteQuery( query );

  if( dbRes.NextRow() )
  {
      * count = dbRes.GetInt64( 0 );
      * len   = dbRes.GetInt64( 1 );
      * size  = dbRes.GetInt64( 2 );
  }
  dbRes.Finalize();
}

// -------------------------------------------------------------------------------- //
int guDbPodcasts::GetPodcastItems( const wxArrayInt &ids, guPodcastItemArray * items, const int order, const bool desc )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxT( "SELECT podcastitem_id, podcastitem_chid, podcastitem_title, " \
            "podcastitem_summary, podcastitem_author, podcastitem_enclosure, podcastitem_time, " \
            "podcastitem_file, podcastitem_filesize, podcastitem_length, " \
            "podcastitem_playcount, podcastitem_addeddate, podcastitem_lastplay, " \
            "podcastitem_status, " \
            "podcastch_title, podcastch_category " \
            "FROM podcastitems, podcastchs " \
            "WHERE podcastitem_chid = podcastch_id " \
            "AND " ) + ArrayToFilter( ids, wxT( "podcastitem_id" ) );

  query += wxT( " ORDER BY " );

  switch( order )
  {
      case guPODCASTS_COLUMN_TITLE :
        query += wxT( "podcastitem_title COLLATE NOCASE" );
        break;
      case guPODCASTS_COLUMN_CHANNEL :
        query += wxT( "podcastch_title COLLATE NOCASE" );
        break;
      case guPODCASTS_COLUMN_CATEGORY :
        query += wxT( "podcastch_category COLLATE NOCASE" );
        break;
      case guPODCASTS_COLUMN_DATE :
        query += wxT( "podcastitem_time" );
        break;
      case guPODCASTS_COLUMN_LENGTH :
        query += wxT( "podcastitem_length" );
        break;
      case guPODCASTS_COLUMN_AUTHOR :
        query += wxT( "podcastitem_author COLLATE NOCASE" );
        break;
      case guPODCASTS_COLUMN_PLAYCOUNT :
        query += wxT( "podcastitem_playcount" );
        break;
      case guPODCASTS_COLUMN_LASTPLAY :
        query += wxT( "podcastitem_lastplay" );
        break;
      case guPODCASTS_COLUMN_ADDEDDATE :
        query += wxT( "podcastitem_addeddate" );
        break;
      case guPODCASTS_COLUMN_STATUS :
        query += wxT( "podcastitem_status" );
        break;
  }

  if( desc )
    query += wxT( " DESC;" );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    guPodcastItem * Item = new guPodcastItem();
    Item->m_Id = dbRes.GetInt( 0 );
    Item->m_ChId = dbRes.GetInt( 1 );
    Item->m_Title = dbRes.GetString( 2 );
    Item->m_Summary = dbRes.GetString( 3 );
    Item->m_Author = dbRes.GetString( 4 );
    Item->m_Enclosure = dbRes.GetString( 5 );
    Item->m_Time = dbRes.GetInt( 6 );
    Item->m_FileName = dbRes.GetString( 7 );
    Item->m_FileSize = dbRes.GetInt( 8 );
    Item->m_Length = dbRes.GetInt( 9 );
    Item->m_PlayCount = dbRes.GetInt( 10 );
    Item->m_AddedDate = dbRes.GetInt( 11 );
    Item->m_LastPlay = dbRes.GetInt( 12 );
    Item->m_Status = dbRes.GetInt( 13 );

    Item->m_Channel = dbRes.GetString( 14 );
    Item->m_Category = dbRes.GetString( 15 );
    items->Add( Item );
  }
  dbRes.Finalize();
  return items->Count();
}

// -------------------------------------------------------------------------------- //
void guDbPodcasts::SavePodcastItem( const int channelid, guPodcastItem * item, bool onlynew )
{
  wxString query;
  int ItemId;
  if( ( ItemId = GetPodcastItemEnclosure( item->m_Enclosure ) ) == wxNOT_FOUND )
  {
    //guLogMessage( wxT( "Inserting podcastitem '%s'" ), item->m_Title.c_str() );
    query = wxString::Format( wxT( "INSERT INTO podcastitems( " \
                "podcastitem_id, podcastitem_chid, podcastitem_title, " \
                "podcastitem_summary, podcastitem_author, podcastitem_enclosure, podcastitem_time, " \
                "podcastitem_file, podcastitem_filesize, podcastitem_length, " \
                "podcastitem_addeddate, podcastitem_playcount, podcastitem_lastplay, " \
                "podcastitem_status ) " \
                "VALUES( NULL, %u, '%s', '%s', '%s', '%s', %u, " \
                "'%s', %u, %u, %lu, %u, %u, %u );" ),
                channelid,
                escape_query_str( item->m_Title ).c_str(),
                escape_query_str( item->m_Summary ).c_str(),
                escape_query_str( item->m_Author ).c_str(),
                escape_query_str( item->m_Enclosure ).c_str(),
                item->m_Time,
                escape_query_str( item->m_FileName ).c_str(),
                item->m_FileSize,
                item->m_Length,
                wxDateTime::GetTimeNow(),
                0, 0, 0 );

    ExecuteUpdate( query );
    ItemId = GetLastRowId();
  }
  else if( !onlynew )
  {
    query = wxString::Format( wxT( "UPDATE podcastitems SET " \
                "podcastitem_chid = %u, podcastitem_title = '%s', " \
                "podcastitem_summary = '%s', podcastitem_author = '%s', " \
                "podcastitem_enclosure = '%s', podcastitem_time = %u, " \
                "podcastitem_file = '%s', podcastitem_filesize = %u, podcastitem_length = %u, " \
                "podcastitem_status = %u " \
                "WHERE podcastitem_id = %u;" ),
                channelid,
                escape_query_str( item->m_Title ).c_str(),
                escape_query_str( item->m_Summary ).c_str(),
                escape_query_str( item->m_Author ).c_str(),
                escape_query_str( item->m_Enclosure ).c_str(),
                item->m_Time,
                escape_query_str( item->m_FileName ).c_str(),
                item->m_FileSize,
                item->m_Length,
                item->m_Status,
                ItemId );

    ExecuteUpdate( query );
  }
}

// -------------------------------------------------------------------------------- //
void guDbPodcasts::SavePodcastItems( const int channelid, guPodcastItemArray * items, bool onlynew )
{
    int Index;
    int Count = items->Count();
    for( Index = 0; Index < Count; Index++ )
    {
        SavePodcastItem( channelid, &items->Item( Index ), onlynew );
    }
}

// -------------------------------------------------------------------------------- //
void guDbPodcasts::SetPodcastItemStatus( const int itemid, const int status )
{
  wxString query;
  query = wxString::Format( wxT( "UPDATE podcastitems SET " \
                "podcastitem_status = %u WHERE podcastitem_id = %u;" ),
            status, itemid );

  ExecuteUpdate( query );
}

// -------------------------------------------------------------------------------- //
void guDbPodcasts::SetPodcastItemPlayCount( const int itemid, const int playcount )
{
  wxString query;
  query = wxString::Format( wxT( "UPDATE podcastitems SET " \
                "podcastitem_playcount = %u, podcastitem_lastplay = %lu WHERE podcastitem_id = %u;" ),
            playcount, wxDateTime::GetTimeNow(), itemid );

  ExecuteUpdate( query );
}

// -------------------------------------------------------------------------------- //
void guDbPodcasts::UpdatePodcastItemLength( const int itemid, const int length )
{
  wxString query;
  query = wxString::Format( wxT( "UPDATE podcastitems SET " \
                "podcastitem_length = %u WHERE podcastitem_id = %u;" ),
            length, itemid );

  ExecuteUpdate( query );
}

// -------------------------------------------------------------------------------- //
int guDbPodcasts::GetPodcastItemEnclosure( const wxString &enclosure, guPodcastItem * item )
{
  int RetVal = wxNOT_FOUND;
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxString::Format( wxT( "SELECT podcastitem_id, podcastitem_chid, podcastitem_title, " \
            "podcastitem_summary, podcastitem_author, podcastitem_enclosure, podcastitem_time, " \
            "podcastitem_file, podcastitem_filesize, podcastitem_length, " \
            "podcastitem_playcount, podcastitem_addeddate, podcastitem_lastplay, " \
            "podcastitem_status, " \
            "podcastch_title, podcastch_category " \
            "FROM podcastitems, podcastchs " \
            "WHERE podcastitem_chid = podcastch_id AND " \
            "podcastitem_enclosure = '%s';" ),
            escape_query_str( enclosure ).c_str() );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    RetVal = dbRes.GetInt( 0 );
    if( item )
    {
      item->m_Id = RetVal;
      item->m_ChId = dbRes.GetInt( 1 );
      item->m_Title = dbRes.GetString( 2 );
      item->m_Summary = dbRes.GetString( 3 );
      item->m_Author = dbRes.GetString( 4 );
      item->m_Enclosure = dbRes.GetString( 5 );
      item->m_Time = dbRes.GetInt( 6 );
      item->m_FileName = dbRes.GetString( 7 );
      item->m_FileSize = dbRes.GetInt( 8 );
      item->m_Length = dbRes.GetInt( 9 );
      item->m_PlayCount = dbRes.GetInt( 10 );
      item->m_AddedDate = dbRes.GetInt( 11 );
      item->m_LastPlay = dbRes.GetInt( 12 );
      item->m_Status = dbRes.GetInt( 13 );

      item->m_Channel = dbRes.GetString( 14 );
      item->m_Category = dbRes.GetString( 15 );
    }
  }
  dbRes.Finalize();
  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbPodcasts::GetPodcastItemId( const int itemid, guPodcastItem * item )
{
  int RetVal = wxNOT_FOUND;
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxString::Format( wxT( "SELECT podcastitem_id, podcastitem_chid, podcastitem_title, " \
            "podcastitem_summary, podcastitem_author, podcastitem_enclosure, podcastitem_time, " \
            "podcastitem_file, podcastitem_filesize, podcastitem_length, " \
            "podcastitem_playcount, podcastitem_addeddate, podcastitem_lastplay, " \
            "podcastitem_status, " \
            "podcastch_title, podcastch_category " \
            "FROM podcastitems, podcastchs " \
            "WHERE podcastitem_chid = podcastch_id AND " \
            "podcastitem_id = %u LIMIT 1;" ),
            itemid );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    RetVal = dbRes.GetInt( 0 );
    if( item )
    {
      item->m_Id = RetVal;
      item->m_ChId = dbRes.GetInt( 1 );
      item->m_Title = dbRes.GetString( 2 );
      item->m_Summary = dbRes.GetString( 3 );
      item->m_Author = dbRes.GetString( 4 );
      item->m_Enclosure = dbRes.GetString( 5 );
      item->m_Time = dbRes.GetInt( 6 );
      item->m_FileName = dbRes.GetString( 7 );
      item->m_FileSize = dbRes.GetInt( 8 );
      item->m_Length = dbRes.GetInt( 9 );
      item->m_PlayCount = dbRes.GetInt( 10 );
      item->m_AddedDate = dbRes.GetInt( 11 );
      item->m_LastPlay = dbRes.GetInt( 12 );
      item->m_Status = dbRes.GetInt( 13 );

      item->m_Channel = dbRes.GetString( 14 );
      item->m_Category = dbRes.GetString( 15 );
    }
  }
  dbRes.Finalize();
  return RetVal;
}

// -------------------------------------------------------------------------------- //
int guDbPodcasts::GetPodcastItemFile( const wxString &filename, guPodcastItem * item )
{
  int RetVal = 0;
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxString::Format( wxT( "SELECT podcastitem_id, podcastitem_chid, podcastitem_title, " \
            "podcastitem_summary, podcastitem_author, podcastitem_enclosure, podcastitem_time, " \
            "podcastitem_file, podcastitem_filesize, podcastitem_length, " \
            "podcastitem_playcount, podcastitem_addeddate, podcastitem_lastplay, " \
            "podcastitem_status, " \
            "podcastch_title, podcastch_category " \
            "FROM podcastitems, podcastchs " \
            "WHERE podcastitem_chid = podcastch_id AND " \
            "podcastitem_file = '%s' LIMIT 1;" ),
            escape_query_str( filename ).c_str() );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    RetVal = dbRes.GetInt( 0 );
    if( item )
    {
      item->m_Id = RetVal;
      item->m_ChId = dbRes.GetInt( 1 );
      item->m_Title = dbRes.GetString( 2 );
      item->m_Summary = dbRes.GetString( 3 );
      item->m_Author = dbRes.GetString( 4 );
      item->m_Enclosure = dbRes.GetString( 5 );
      item->m_Time = dbRes.GetInt( 6 );
      item->m_FileName = dbRes.GetString( 7 );
      item->m_FileSize = dbRes.GetInt( 8 );
      item->m_Length = dbRes.GetInt( 9 );
      item->m_PlayCount = dbRes.GetInt( 10 );
      item->m_AddedDate = dbRes.GetInt( 11 );
      item->m_LastPlay = dbRes.GetInt( 12 );
      item->m_Status = dbRes.GetInt( 13 );

      item->m_Channel = dbRes.GetString( 14 );
      item->m_Category = dbRes.GetString( 15 );
    }
  }
  dbRes.Finalize();
  return RetVal;
}

// -------------------------------------------------------------------------------- //
void guDbPodcasts::DelPodcastItem( const int itemid )
{
  wxString query;

  query = wxString::Format( wxT( "UPDATE podcastitems SET " \
            "podcastitem_status = %u WHERE podcastitem_id = %u;" ),
            guPODCAST_STATUS_DELETED, itemid );

  ExecuteUpdate( query );
}

// -------------------------------------------------------------------------------- //
void guDbPodcasts::DelPodcastItems( const int channelid )
{
  wxString query;

  query = wxString::Format( wxT( "DELETE FROM podcastitems " \
            "WHERE podcastitem_chid = %u;" ), channelid );

  ExecuteUpdate( query );
}

// -------------------------------------------------------------------------------- //
int guDbPodcasts::GetPendingPodcasts( guPodcastItemArray * items )
{
  wxString query;
  wxSQLite3ResultSet dbRes;

  query = wxT( "SELECT podcastitem_id, podcastitem_chid, podcastitem_title, " \
            "podcastitem_summary, podcastitem_author, podcastitem_enclosure, podcastitem_time, " \
            "podcastitem_file, podcastitem_filesize, podcastitem_length, " \
            "podcastitem_playcount, podcastitem_addeddate, podcastitem_lastplay, " \
            "podcastitem_status, " \
            "podcastch_title, podcastch_category " \
            "FROM podcastitems, podcastchs " \
            "WHERE podcastitem_chid = podcastch_id " \
            "AND podcastitem_status IN ( 1, 2 ) " \
            "ORDER BY podcastitem_status DESC;" ); 

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
    guPodcastItem * Item = new guPodcastItem();
    Item->m_Id = dbRes.GetInt( 0 );
    Item->m_ChId = dbRes.GetInt( 1 );
    Item->m_Title = dbRes.GetString( 2 );
    Item->m_Summary = dbRes.GetString( 3 );
    Item->m_Author = dbRes.GetString( 4 );
    Item->m_Enclosure = dbRes.GetString( 5 );
    Item->m_Time = dbRes.GetInt( 6 );
    Item->m_FileName = dbRes.GetString( 7 );
    Item->m_FileSize = dbRes.GetInt( 8 );
    Item->m_Length = dbRes.GetInt( 9 );
    Item->m_PlayCount = dbRes.GetInt( 10 );
    Item->m_AddedDate = dbRes.GetInt( 11 );
    Item->m_LastPlay = dbRes.GetInt( 12 );
    Item->m_Status = dbRes.GetInt( 13 );

    Item->m_Channel = dbRes.GetString( 14 );
    Item->m_Category = dbRes.GetString( 15 );
    items->Add( Item );
  }
  dbRes.Finalize();
  return items->Count();
}

// -------------------------------------------------------------------------------- //
int guDbPodcasts::GetPodcastFiles( const wxArrayInt &channels, guDataObjectComposite * files )
{
  int Count = 0;
  wxString query;
  wxArrayString Filenames;
  wxSQLite3ResultSet dbRes;

  query = wxT( "SELECT podcastitem_file FROM podcastitems WHERE " ) +
          ArrayToFilter( channels, wxT( "podcastitem_chid" ) );

  dbRes = ExecuteQuery( query );

  while( dbRes.NextRow() )
  {
      Filenames.Add( guFileDnDEncode( dbRes.GetString( 0 ) ) );
      Count++;
  }
  files->SetFiles( Filenames );
  dbRes.Finalize();
  return Count;
}

// -------------------------------------------------------------------------------- //
void guDbPodcasts::UpdateItemPaths( const wxString &oldpath, const wxString &newpath )
{
  wxString query;

  query = wxString::Format( wxT( "UPDATE podcastitems SET podcastitem_file = replace( podcastitem_file, '%s', '%s' )" ),
            escape_query_str( oldpath ).c_str(), escape_query_str( newpath ).c_str() );

  guLogMessage( wxT( "Updating path: %s" ), query.c_str() );
  ExecuteUpdate( query );
}


// -------------------------------------------------------------------------------- //
// guPostcastPanel
// -------------------------------------------------------------------------------- //
guPodcastPanel::guPodcastPanel( wxWindow * parent, guDbPodcasts * db, guMainFrame * mainframe, guPlayerPanel * playerpanel ) :
    guAuiManagerPanel( parent )
{
    m_Db = db;
    m_MainFrame = mainframe;
    m_PlayerPanel = playerpanel;
    m_LastChannelInfoId = wxNOT_FOUND;
    m_LastPodcastInfoId = wxNOT_FOUND;

    wxPanel * ChannelsPanel;
    wxPanel * PodcastsPanel;
    wxStaticText * DetailDescLabel;
    wxStaticText * DetailAuthorLabel;
    wxStaticText * DetailOwnerLabel;
    wxStaticLine * DetailStaticLine1;
    wxStaticLine * DetailStaticLine2;
    wxStaticText * DetailItemTitleLabel;
    wxStaticText * DetailItemSumaryLabel;
    wxStaticText * DetailItemDateLabel;
    wxStaticText * DetailItemLengthLabel;


    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->RegisterObject( this );

    InitPanelData();

    // Check that the directory to store podcasts are created
    m_PodcastsPath = Config->ReadStr( CONFIG_KEY_PODCASTS_PATH,
                                      guPATH_PODCASTS,
                                      CONFIG_PATH_PODCASTS );
    if( !wxDirExists( m_PodcastsPath ) )
    {
        wxMkdir( m_PodcastsPath, 0770 );
    }

    m_VisiblePanels = Config->ReadNum( CONFIG_KEY_PODCASTS_VISIBLE_PANELS,
                                       guPANEL_PODCASTS_VISIBLE_DEFAULT,
                                       CONFIG_PATH_PODCASTS );

	ChannelsPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer * ChannelsMainSizer;
	ChannelsMainSizer = new wxBoxSizer( wxVERTICAL );

	m_ChannelsListBox = new guChannelsListBox( ChannelsPanel, m_Db, _( "Channels" ) );
	ChannelsMainSizer->Add( m_ChannelsListBox, 1, wxEXPAND, 5 );

	ChannelsPanel->SetSizer( ChannelsMainSizer );
	ChannelsPanel->Layout();
	ChannelsMainSizer->Fit( ChannelsPanel );

    m_AuiManager.AddPane( ChannelsPanel,
            wxAuiPaneInfo().Name( wxT( "PodcastsChannels" ) ).Caption( _( "Channels" ) ).
            MinSize( 50, 50 ).
            CloseButton( Config->ReadBool( CONFIG_KEY_GENERAL_SHOW_CLOSE_BUTTON, true, CONFIG_PATH_GENERAL ) ).
            Dockable( true ).Left() );



	PodcastsPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer * MainPodcastsSizer;
	MainPodcastsSizer = new wxBoxSizer( wxVERTICAL );

	m_PodcastsListBox = new guPodcastListBox( PodcastsPanel, m_Db );
	MainPodcastsSizer->Add( m_PodcastsListBox, 1, wxEXPAND, 5 );

	PodcastsPanel->SetSizer( MainPodcastsSizer );
	PodcastsPanel->Layout();
	MainPodcastsSizer->Fit( PodcastsPanel );

    m_AuiManager.AddPane( PodcastsPanel, wxAuiPaneInfo().Name( wxT( "PodcastsItems" ) ).Caption( _( "Podcasts" ) ).
            MinSize( 50, 50 ).
            CenterPane() );



	wxPanel * DetailsPanel;
	DetailsPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );

	m_DetailMainSizer = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* DetailSizer;
	DetailSizer = new wxStaticBoxSizer( new wxStaticBox( DetailsPanel, wxID_ANY, _(" Details ") ), wxVERTICAL );

	m_DetailScrolledWindow = new wxScrolledWindow( DetailsPanel, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), wxHSCROLL );
	m_DetailScrolledWindow->SetScrollRate( 5, 5 );
	m_DetailScrolledWindow->SetMinSize( wxSize( -1,100 ) );

    m_DetailFlexGridSizer = new wxFlexGridSizer( 2, 0, 0 );
	m_DetailFlexGridSizer->AddGrowableCol( 1 );
	m_DetailFlexGridSizer->SetFlexibleDirection( wxBOTH );
	m_DetailFlexGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_DetailImage = new wxStaticBitmap( m_DetailScrolledWindow, wxID_ANY, guImage( guIMAGE_INDEX_mid_podcast ), wxDefaultPosition, wxSize( 60,60 ), 0 );
	m_DetailFlexGridSizer->Add( m_DetailImage, 0, wxALL, 5 );

	m_DetailChannelTitle = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_DetailChannelTitle->Wrap( -1 );
	m_DetailChannelTitle->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );

	m_DetailFlexGridSizer->Add( m_DetailChannelTitle, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	DetailDescLabel = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, _("Description:"), wxDefaultPosition, wxDefaultSize, 0 );
	DetailDescLabel->Wrap( -1 );
	DetailDescLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );

	m_DetailFlexGridSizer->Add( DetailDescLabel, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_DetailDescText = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE );
	//m_DetailDescText->Wrap( 445 );
	m_DetailFlexGridSizer->Add( m_DetailDescText, 0, wxEXPAND|wxBOTTOM|wxRIGHT, 5 );

	DetailAuthorLabel = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, _("Author:"), wxDefaultPosition, wxDefaultSize, 0 );
	DetailAuthorLabel->Wrap( -1 );
	DetailAuthorLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );

	m_DetailFlexGridSizer->Add( DetailAuthorLabel, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_DetailAuthorText = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_DetailAuthorText->Wrap( -1 );
	m_DetailFlexGridSizer->Add( m_DetailAuthorText, 0, wxEXPAND|wxBOTTOM|wxRIGHT, 5 );

	DetailOwnerLabel = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, _("Owner:"), wxDefaultPosition, wxDefaultSize, 0 );
	DetailOwnerLabel->Wrap( -1 );
	DetailOwnerLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );

	m_DetailFlexGridSizer->Add( DetailOwnerLabel, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_DetailOwnerText = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_DetailOwnerText->Wrap( -1 );
	m_DetailFlexGridSizer->Add( m_DetailOwnerText, 0, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	wxStaticText * DetailLinkLabel;
	DetailLinkLabel = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, _("Link:"), wxDefaultPosition, wxDefaultSize, 0 );
	DetailLinkLabel->Wrap( -1 );
	DetailLinkLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );

	m_DetailFlexGridSizer->Add( DetailLinkLabel, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

    m_DetailLinkText = new wxHyperlinkCtrl( m_DetailScrolledWindow, wxID_ANY, " ", " ", wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
	m_DetailFlexGridSizer->Add( m_DetailLinkText, 0, wxBOTTOM|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );

	DetailStaticLine1 = new wxStaticLine( m_DetailScrolledWindow, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	m_DetailFlexGridSizer->Add( DetailStaticLine1, 0, wxEXPAND|wxBOTTOM|wxLEFT, 5 );

	DetailStaticLine2 = new wxStaticLine( m_DetailScrolledWindow, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	m_DetailFlexGridSizer->Add( DetailStaticLine2, 0, wxEXPAND|wxBOTTOM|wxRIGHT, 5 );

	DetailItemTitleLabel = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, _("Title:"), wxDefaultPosition, wxDefaultSize, 0 );
	DetailItemTitleLabel->Wrap( -1 );
	DetailItemTitleLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );

	m_DetailFlexGridSizer->Add( DetailItemTitleLabel, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_DetailItemTitleText = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_DetailItemTitleText->Wrap( -1 );
	m_DetailFlexGridSizer->Add( m_DetailItemTitleText, 0, wxBOTTOM|wxRIGHT, 5 );

	DetailItemSumaryLabel = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, _("Sumary:"), wxDefaultPosition, wxDefaultSize, 0 );
	DetailItemSumaryLabel->Wrap( -1 );
	DetailItemSumaryLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );

	m_DetailFlexGridSizer->Add( DetailItemSumaryLabel, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_DetailItemSumaryText = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE );
	//m_DetailItemSumaryText->Wrap( 300 );
	m_DetailFlexGridSizer->Add( m_DetailItemSumaryText, 0, wxBOTTOM|wxRIGHT|wxEXPAND, 5 );

	DetailItemDateLabel = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, _("Date:"), wxDefaultPosition, wxDefaultSize, 0 );
	DetailItemDateLabel->Wrap( -1 );
	DetailItemDateLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );

	m_DetailFlexGridSizer->Add( DetailItemDateLabel, 0, wxALIGN_RIGHT|wxBOTTOM|wxRIGHT|wxLEFT, 5 );

	m_DetailItemDateText = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_DetailItemDateText->Wrap( -1 );
	m_DetailFlexGridSizer->Add( m_DetailItemDateText, 0, wxBOTTOM|wxRIGHT, 5 );

	DetailItemLengthLabel = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, _("Length:"), wxDefaultPosition, wxDefaultSize, 0 );
	DetailItemLengthLabel->Wrap( -1 );
	DetailItemLengthLabel->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );

	m_DetailFlexGridSizer->Add( DetailItemLengthLabel, 0, wxBOTTOM|wxRIGHT|wxLEFT|wxALIGN_RIGHT, 5 );

	m_DetailItemLengthText = new wxStaticText( m_DetailScrolledWindow, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_DetailItemLengthText->Wrap( -1 );
	m_DetailFlexGridSizer->Add( m_DetailItemLengthText, 0, wxBOTTOM|wxRIGHT, 5 );

	m_DetailScrolledWindow->SetSizer( m_DetailFlexGridSizer );
	m_DetailScrolledWindow->Layout();
	m_DetailFlexGridSizer->Fit( m_DetailScrolledWindow );
	DetailSizer->Add( m_DetailScrolledWindow, 1, wxEXPAND|wxALL, 5 );

	m_DetailMainSizer->Add( DetailSizer, 1, wxEXPAND|wxALL, 5 );

	DetailsPanel->SetSizer( m_DetailMainSizer );
	DetailsPanel->Layout();
	DetailSizer->Fit( DetailsPanel );

    m_AuiManager.AddPane( DetailsPanel, wxAuiPaneInfo().Name( wxT( "PodcastsDetails" ) ).Caption( _( "Podcast Details" ) ).
            MinSize( 100, 100 ).
            CloseButton( Config->ReadBool( CONFIG_KEY_GENERAL_SHOW_CLOSE_BUTTON, true, CONFIG_PATH_GENERAL ) ).
            Dockable( true ).Bottom() );

    wxString PodcastLayout = Config->ReadStr( CONFIG_KEY_PODCASTS_LASTLAYOUT,
                                              wxEmptyString,
                                              CONFIG_PATH_PODCASTS );
    if( Config->GetIgnoreLayouts() || PodcastLayout.IsEmpty() )
    {
        m_VisiblePanels = guPANEL_PODCASTS_VISIBLE_DEFAULT;

        PodcastLayout = wxT( "layout2|name=PodcastsChannels;caption=" ) + wxString( _( "Channels" ) );
        PodcastLayout += wxT( ";state=2099196;dir=4;layer=0;row=0;pos=0;prop=100000;bestw=50;besth=50;minw=50;minh=50;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|" );
        PodcastLayout += wxT( "name=PodcastsItems;caption=Podcasts;state=768;dir=5;layer=0;row=0;pos=0;prop=100000;bestw=50;besth=50;minw=50;minh=50;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|" );
        PodcastLayout += wxT( "name=PodcastsDetails;caption=" ) + wxString( _( "Details" ) );
        PodcastLayout += wxT( ";state=2099196;dir=3;layer=0;row=0;pos=0;prop=100000;bestw=100;besth=132;minw=100;minh=100;maxw=-1;maxh=-1;floatx=-1;floaty=-1;floatw=-1;floath=-1|dock_size(4,0,0)=181|" );
        PodcastLayout += wxT( "dock_size(5,0,0)=52|dock_size(3,0,0)=479|" );
        //m_AuiManager.Update();
    }

    m_AuiManager.LoadPerspective( PodcastLayout, true );

    // Bind events
    Bind( wxEVT_MENU, &guPodcastPanel::AddChannel, this, ID_PODCASTS_CHANNEL_ADD );
    Bind( wxEVT_MENU, &guPodcastPanel::DeleteChannels, this, ID_PODCASTS_CHANNEL_DEL );
    Bind( wxEVT_MENU, &guPodcastPanel::ChannelProperties, this, ID_PODCASTS_CHANNEL_PROPERTIES );
    Bind( wxEVT_MENU, &guPodcastPanel::UpdateChannels, this, ID_PODCASTS_CHANNEL_UPDATE );
    m_ChannelsListBox->Bind( wxEVT_MENU, &guPodcastPanel::ChannelsCopyTo, this, ID_COPYTO_BASE, ID_COPYTO_BASE + guCOPYTO_MAXCOUNT );

    Bind( guPodcastEvent, &guPodcastPanel::OnPodcastItemUpdated, this, ID_PODCASTS_ITEM_UPDATED );

    m_ChannelsListBox->Bind( wxEVT_LISTBOX, &guPodcastPanel::OnChannelsSelected, this );
    m_ChannelsListBox->Bind( wxEVT_LISTBOX_DCLICK, &guPodcastPanel::OnChannelsActivated, this );
    m_PodcastsListBox->Bind( wxEVT_LIST_COL_CLICK, &guPodcastPanel::OnPodcastsColClick, this );
    m_PodcastsListBox->Bind( wxEVT_LISTBOX, &guPodcastPanel::OnPodcastItemSelected, this );
    m_PodcastsListBox->Bind( wxEVT_LISTBOX_DCLICK, &guPodcastPanel::OnPodcastItemActivated, this );

    Bind( wxEVT_MENU, &guPodcastPanel::OnPodcastItemPlay, this, ID_PODCASTS_ITEM_PLAY );
    Bind( wxEVT_MENU, &guPodcastPanel::OnPodcastItemEnqueue, this, ID_PODCASTS_ITEM_ENQUEUE_AFTER_ALL, ID_PODCASTS_ITEM_ENQUEUE_AFTER_ARTIST );
    Bind( wxEVT_MENU, &guPodcastPanel::OnPodcastItemDelete, this, ID_PODCASTS_ITEM_DEL );
    Bind( wxEVT_MENU, &guPodcastPanel::OnPodcastItemDownload, this, ID_PODCASTS_ITEM_DOWNLOAD );
    m_PodcastsListBox->Bind( wxEVT_MENU, &guPodcastPanel::OnPodcastItemCopyTo, this, ID_COPYTO_BASE, ID_COPYTO_BASE + guCOPYTO_MAXCOUNT );

    Bind( guConfigUpdatedEvent, &guPodcastPanel::OnConfigUpdated, this, ID_CONFIG_UPDATED );
}

// -------------------------------------------------------------------------------- //
guPodcastPanel::~guPodcastPanel()
{
    // Save the Splitter positions into the main config
    guConfig * Config = ( guConfig * ) guConfig::Get();
    if( Config )
    {
        Config->UnRegisterObject( this );

        Config->WriteNum( CONFIG_KEY_PODCASTS_VISIBLE_PANELS,
                          m_VisiblePanels,
                          CONFIG_PATH_PODCASTS );
        Config->WriteStr( CONFIG_KEY_PODCASTS_LASTLAYOUT,
                          m_AuiManager.SavePerspective(),
                          CONFIG_PATH_PODCASTS );
    }

    // Unbind events
    Unbind( wxEVT_MENU, &guPodcastPanel::AddChannel, this, ID_PODCASTS_CHANNEL_ADD );
    Unbind( wxEVT_MENU, &guPodcastPanel::DeleteChannels, this, ID_PODCASTS_CHANNEL_DEL );
    Unbind( wxEVT_MENU, &guPodcastPanel::ChannelProperties, this, ID_PODCASTS_CHANNEL_PROPERTIES );
    Unbind( wxEVT_MENU, &guPodcastPanel::UpdateChannels, this, ID_PODCASTS_CHANNEL_UPDATE );
    m_ChannelsListBox->Unbind( wxEVT_MENU, &guPodcastPanel::ChannelsCopyTo, this, ID_COPYTO_BASE, ID_COPYTO_BASE + guCOPYTO_MAXCOUNT );

    Unbind( guPodcastEvent, &guPodcastPanel::OnPodcastItemUpdated, this, ID_PODCASTS_ITEM_UPDATED );

    m_ChannelsListBox->Unbind( wxEVT_LISTBOX, &guPodcastPanel::OnChannelsSelected, this );
    m_ChannelsListBox->Unbind( wxEVT_LISTBOX_DCLICK, &guPodcastPanel::OnChannelsActivated, this );
    m_PodcastsListBox->Unbind( wxEVT_LIST_COL_CLICK, &guPodcastPanel::OnPodcastsColClick, this );
    m_PodcastsListBox->Unbind( wxEVT_LISTBOX, &guPodcastPanel::OnPodcastItemSelected, this );
    m_PodcastsListBox->Unbind( wxEVT_LISTBOX_DCLICK, &guPodcastPanel::OnPodcastItemActivated, this );

    Unbind( wxEVT_MENU, &guPodcastPanel::OnPodcastItemPlay, this, ID_PODCASTS_ITEM_PLAY );
    Unbind( wxEVT_MENU, &guPodcastPanel::OnPodcastItemEnqueue, this, ID_PODCASTS_ITEM_ENQUEUE_AFTER_ALL, ID_PODCASTS_ITEM_ENQUEUE_AFTER_ARTIST );
    Unbind( wxEVT_MENU, &guPodcastPanel::OnPodcastItemDelete, this, ID_PODCASTS_ITEM_DEL );
    Unbind( wxEVT_MENU, &guPodcastPanel::OnPodcastItemDownload, this, ID_PODCASTS_ITEM_DOWNLOAD );
    m_PodcastsListBox->Unbind( wxEVT_MENU, &guPodcastPanel::OnPodcastItemCopyTo, this, ID_COPYTO_BASE, ID_COPYTO_BASE + guCOPYTO_MAXCOUNT );

    Unbind( guConfigUpdatedEvent, &guPodcastPanel::OnConfigUpdated, this, ID_CONFIG_UPDATED );
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::InitPanelData( void )
{
    m_PanelNames.Add( wxT( "PodcastsChannels" ) );
    m_PanelNames.Add( wxT( "PodcastsDetails" ) );

    m_PanelIds.Add( guPANEL_PODCASTS_CHANNELS );
    m_PanelIds.Add( guPANEL_PODCASTS_DETAILS );

    m_PanelCmdIds.Add( ID_MENU_VIEW_POD_CHANNELS );
    m_PanelCmdIds.Add( ID_MENU_VIEW_POD_DETAILS );
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::OnConfigUpdated( wxCommandEvent &event )
{
    int Flags = event.GetInt();
    if( Flags & guPREFERENCE_PAGE_FLAG_PODCASTS )
    {
        guConfig * Config = ( guConfig * ) guConfig::Get();

        // Check that the directory to store podcasts are created
        m_PodcastsPath = Config->ReadStr( CONFIG_KEY_PODCASTS_PATH,
                                          guPATH_PODCASTS,
                                          CONFIG_PATH_PODCASTS );
        if( !wxDirExists( m_PodcastsPath ) )
        {
            wxMkdir( m_PodcastsPath, 0770 );
        }
    }
}

// -------------------------------------------------------------------------------- //
void NormalizePodcastChannel( guPodcastChannel * PodcastChannel )
{
    int ChId = PodcastChannel->m_Id;
    wxString ChName = PodcastChannel->m_Title;
    wxString Category = PodcastChannel->m_Category;
    int Index;
    int Count = PodcastChannel->m_Items.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guPodcastItem * PodcastItem = &PodcastChannel->m_Items[ Index ];
        PodcastItem->m_ChId = ChId;
        PodcastItem->m_Channel = ChName;
        PodcastItem->m_Category = Category;
    }
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::AddChannel( wxCommandEvent &event )
{
    guNewPodcastChannelSelector * NewPodcastChannel = new guNewPodcastChannelSelector( this );
    if( NewPodcastChannel->ShowModal() == wxID_OK )
    {
        wxSetCursor( * wxHOURGLASS_CURSOR );
        wxTheApp->Yield();

        wxString PodcastUrl = NewPodcastChannel->GetValue().Trim( false ).Trim( true );
        if( !PodcastUrl.IsEmpty() )
        {
            // If we find an itunes link we replace the itpc to the standard http
            if( PodcastUrl.StartsWith( wxT( "itpc://" ) ) )
            {
                PodcastUrl.Replace( wxT( "itpc://" ), wxT( "http://" ) );
            }

            guPodcastChannel PodcastChannel( PodcastUrl );

            wxSetCursor( * wxSTANDARD_CURSOR );
                    //
            guChannelEditor * ChannelEditor = new guChannelEditor( this, &PodcastChannel );
            if( ChannelEditor->ShowModal() == wxID_OK )
            {
                wxSetCursor( * wxHOURGLASS_CURSOR );

                ChannelEditor->GetEditData();

                // Create the channel dir
                wxFileName ChannelDir = wxFileName( m_PodcastsPath + wxT( "/" ) +
                                          PodcastChannel.m_Title );
                if( ChannelDir.Normalize( wxPATH_NORM_ALL | wxPATH_NORM_CASE ) )
                {
                    if( !wxDirExists( ChannelDir.GetFullPath() ) )
                    {
                        wxMkdir( ChannelDir.GetFullPath(), 0770 );
                    }
                }

                PodcastChannel.CheckLogo();

                //
                //guLogMessage( wxT( "The Channel have DownloadType : %u" ), PodcastChannel.m_DownloadType );
                m_Db->SavePodcastChannel( &PodcastChannel );

                PodcastChannel.CheckDeleteItems( m_Db );

                PodcastChannel.CheckDownloadItems( m_Db, m_MainFrame );

                m_ChannelsListBox->ReloadItems();
            }
            ChannelEditor->Destroy();
        }
        wxSetCursor( * wxSTANDARD_CURSOR );
    }
    NewPodcastChannel->Destroy();
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::DeleteChannels( wxCommandEvent &event )
{
    if( wxMessageBox( _( "Are you sure to delete the selected podcast channel?" ),
                      _( "Confirm" ),
                      wxICON_QUESTION|wxYES_NO|wxNO_DEFAULT, this ) == wxYES )
    {
        wxArrayInt SelectedItems = m_ChannelsListBox->GetSelectedItems();
        int Index;
        int Count;
        if( ( Count = SelectedItems.Count() ) )
        {
            wxSetCursor( * wxHOURGLASS_CURSOR );
            for( Index = 0; Index < Count; Index++ )
            {
                m_Db->DelPodcastChannel( SelectedItems[ Index ] );
            }
            m_ChannelsListBox->ReloadItems();
            wxSetCursor( * wxSTANDARD_CURSOR );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::ChannelProperties( wxCommandEvent &event )
{
    guPodcastChannel PodcastChannel;
    wxArrayInt SelectedItems = m_ChannelsListBox->GetSelectedItems();
    m_Db->GetPodcastChannelId( SelectedItems[ 0 ], &PodcastChannel );

    guChannelEditor * ChannelEditor = new guChannelEditor( this, &PodcastChannel );
    if( ChannelEditor->ShowModal() == wxID_OK )
    {
        wxSetCursor( * wxHOURGLASS_CURSOR );

        ChannelEditor->GetEditData();

        // Create the channel dir
        wxFileName ChannelDir = wxFileName( m_PodcastsPath + wxT( "/" ) +
                                  PodcastChannel.m_Title );
        if( ChannelDir.Normalize( wxPATH_NORM_ALL | wxPATH_NORM_CASE ) )
        {
            if( !wxDirExists( ChannelDir.GetFullPath() ) )
            {
                wxMkdir( ChannelDir.GetFullPath(), 0770 );
            }
        }

        PodcastChannel.CheckLogo();

        //
        //guLogMessage( wxT( "The Channel have DownloadType : %u" ), PodcastChannel.m_DownloadType );

        m_Db->SavePodcastChannel( &PodcastChannel );

        PodcastChannel.CheckDeleteItems( m_Db );

        PodcastChannel.CheckDownloadItems( m_Db, m_MainFrame );

        m_ChannelsListBox->ReloadItems();

        wxSetCursor( * wxSTANDARD_CURSOR );
    }
    ChannelEditor->Destroy();
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::ChannelsCopyTo( wxCommandEvent &event )
{
    guPodcastItemArray PodcastItems;
    guTrackArray * Tracks = new guTrackArray();

    m_Db->GetPodcastItems( &PodcastItems, m_PodcastsListBox->GetFilters(), m_PodcastsListBox->GetOrder(), m_PodcastsListBox->GetOrderDesc() );

    int Index;
    int Count = PodcastItems.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guTrack * Track = new guTrack();
        if( Track )
        {
            Track->m_Type = guTRACK_TYPE_PODCAST;
            Track->m_SongId = PodcastItems[ Index ].m_Id;
            Track->m_FileName = PodcastItems[ Index ].m_FileName;
            Track->m_SongName = PodcastItems[ Index ].m_Title;
            Track->m_ArtistName = PodcastItems[ Index ].m_Author;
            Track->m_AlbumId = PodcastItems[ Index ].m_ChId;
            Track->m_AlbumName = PodcastItems[ Index ].m_Channel;
            Track->m_Length = PodcastItems[ Index ].m_Length;
            Track->m_PlayCount = PodcastItems[ Index ].m_PlayCount;
            Track->m_GenreName = wxT( "Podcasts" );
            Track->m_Number = Index;
            Track->m_Rating = -1;
            Track->m_CoverId = 0;
            Track->m_Year = 0; // Get year from item date
            Tracks->Add( Track );
        }
    }

    Index = event.GetId() - ID_COPYTO_BASE;
    if( Index >= guCOPYTO_DEVICE_BASE )
    {
        Index -= guCOPYTO_DEVICE_BASE;
        event.SetId( ID_MAINFRAME_COPYTODEVICE_TRACKS );
    }
    else
    {
        event.SetId( ID_MAINFRAME_COPYTO );
    }
    event.SetInt( Index );
    event.SetClientData( ( void * ) Tracks );
    wxPostEvent( m_MainFrame, event );
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::UpdateChannels( wxCommandEvent &event )
{
    wxArrayInt Selected = m_ChannelsListBox->GetSelectedItems();
    int Count;
    if( ( Count = Selected.Count() ) )
    {
        wxSetCursor( * wxHOURGLASS_CURSOR );
        guPodcastChannel PodcastChannel;
        int Index;
        for( Index = 0; Index < Count; Index++ )
        {
            if( m_Db->GetPodcastChannelId( Selected[ Index ], &PodcastChannel ) != wxNOT_FOUND )
            {
                ProcessChannel( &PodcastChannel );
            }
        }
        wxSetCursor( * wxSTANDARD_CURSOR );
    }
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::OnChannelsSelected( wxCommandEvent &event )
{
    wxArrayInt SelectedItems = m_ChannelsListBox->GetSelectedItems();
    m_PodcastsListBox->SetFilters( SelectedItems );
    m_PodcastsListBox->ReloadItems();

    if( SelectedItems.Count() == 1 && SelectedItems[ 0 ] != 0 )
        UpdateChannelInfo( SelectedItems[ 0 ] );
    else
        UpdateChannelInfo( -1 );
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::ProcessChannel( guPodcastChannel * channel )
{
    wxSetCursor( * wxHOURGLASS_CURSOR );
    wxTheApp->Yield();

    channel->Update( m_Db, m_MainFrame );

    m_ChannelsListBox->ReloadItems( false );
    m_PodcastsListBox->ReloadItems( false );

    wxSetCursor( * wxSTANDARD_CURSOR );
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::OnChannelsActivated( wxCommandEvent &event )
{
    wxArrayInt Selected = m_ChannelsListBox->GetSelectedItems();

    if( Selected.Count() )
    {
        guPodcastChannel PodcastChannel;
        if( m_Db->GetPodcastChannelId( Selected[ 0 ], &PodcastChannel ) != wxNOT_FOUND )
        {
            ProcessChannel( &PodcastChannel );
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::OnPodcastsColClick( wxListEvent &event )
{
    int ColId = m_PodcastsListBox->GetColumnId( event.m_col );

    m_PodcastsListBox->SetOrder( ColId );
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::OnPodcastItemSelected( wxCommandEvent &event )
{
    wxArrayInt Selection = m_PodcastsListBox->GetSelectedItems();
    UpdatePodcastInfo( Selection.Count() ? Selection[ 0 ] : -1 );
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::UpdatePodcastInfo( int itemid )
{
    if( m_LastPodcastInfoId == itemid )
        return;

    m_LastPodcastInfoId = itemid;

    guPodcastItem       PodcastItem;
    //
    //guLogMessage( wxT( "Updating the podcast info of the item %u" ), itemid );
    if( itemid > 0 )
    {
        m_Db->GetPodcastItemId( itemid, &PodcastItem );

        UpdateChannelInfo( PodcastItem.m_ChId );

        m_DetailItemTitleText->SetLabel( PodcastItem.m_Title );
        m_DetailItemSumaryText->SetLabel( PodcastItem.m_Summary );
        wxDateTime AddedDate;
        AddedDate.Set( ( time_t ) PodcastItem.m_Time );
        m_DetailItemDateText->SetLabel( AddedDate.Format( wxT( "%a, %d %b %Y %T %z" ) ) );
        m_DetailItemLengthText->SetLabel( LenToString( PodcastItem.m_Length ) );
    }
    else
    {
        m_DetailItemTitleText->SetLabel( wxEmptyString );
        m_DetailItemSumaryText->SetLabel( wxEmptyString );
        m_DetailItemDateText->SetLabel( wxEmptyString );
        m_DetailItemLengthText->SetLabel( wxEmptyString );
    }
    m_DetailMainSizer->Layout();
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::UpdateChannelInfo( int itemid )
{
    if( m_LastChannelInfoId == itemid )
        return;

    m_LastChannelInfoId = itemid;

    guPodcastChannel    PodcastChannel;
    //
    //guLogMessage( wxT( "Updating the channel info of the item %u" ), itemid );
    if( itemid > 0 )
    {
        m_Db->GetPodcastChannelId( itemid, &PodcastChannel );

        // Set Image...
        wxFileName ImageFile = wxFileName( m_PodcastsPath + wxT( "/" ) +
                                           PodcastChannel.m_Title + wxT( "/" ) +
                                           PodcastChannel.m_Title + wxT( ".jpg" ) );
        if( ImageFile.Normalize( wxPATH_NORM_ALL|wxPATH_NORM_CASE ) )
        {
            wxImage PodcastImage;
            if( wxFileExists( ImageFile.GetFullPath() ) &&
                PodcastImage.LoadFile( ImageFile.GetFullPath() ) &&
                PodcastImage.IsOk() )
            {
                m_DetailImage->SetBitmap( PodcastImage );
            }
            else
                m_DetailImage->SetBitmap( guBitmap( guIMAGE_INDEX_mid_podcast ) );
        }

        m_DetailChannelTitle->SetLabel( PodcastChannel.m_Title );
        m_DetailDescText->SetLabel( PodcastChannel.m_Description );
        m_DetailAuthorText->SetLabel( PodcastChannel.m_Author );
        m_DetailOwnerText->SetLabel( PodcastChannel.m_OwnerName +
                                     wxT( " (" ) + PodcastChannel.m_OwnerEmail + wxT( ")" ) );
        m_DetailLinkText->SetLabel( PodcastChannel.m_Url );
        m_DetailLinkText->SetURL( PodcastChannel.m_Url );
    }
    else
    {
        m_DetailImage->SetBitmap( guBitmap( guIMAGE_INDEX_mid_podcast ) );
        m_DetailChannelTitle->SetLabel( wxEmptyString );
        m_DetailDescText->SetLabel( wxEmptyString );
        m_DetailAuthorText->SetLabel( wxEmptyString );
        m_DetailOwnerText->SetLabel( wxEmptyString );
        m_DetailLinkText->SetURL( wxEmptyString );
        m_DetailLinkText->SetLabel( wxEmptyString );
    }
    m_DetailMainSizer->FitInside( m_DetailScrolledWindow );
    m_DetailScrolledWindow->SetVirtualSize( m_DetailMainSizer->GetSize() );
    //m_DetailFlexGridSizer->FitInside( m_DetailScrolledWindow );
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::OnPodcastItemUpdated( wxCommandEvent &event )
{
    guPodcastItem * PodcastItem = ( guPodcastItem * ) event.GetClientData();

    //guLogMessage( wxT( "PodcastItem Updated... Item: %s Status: %u" ), PodcastItem->m_Title.c_str(), PodcastItem->m_Status );
    m_Db->SavePodcastItem( PodcastItem->m_ChId, PodcastItem );
    m_PodcastsListBox->ReloadItems( false );

    delete PodcastItem;
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::OnSelectPodcasts( bool enqueue, const int aftercurrent )
{
    int Index;
    int Count;
    wxArrayInt Selected = m_PodcastsListBox->GetSelectedItems();
    if( ( Count = Selected.Count() ) )
    {
        guTrackArray Tracks;
        for( Index = 0; Index < Count; Index++ )
        {
            guPodcastItem PodcastItem;
            if( m_Db->GetPodcastItemId( Selected[ Index ], &PodcastItem ) != wxNOT_FOUND )
            {
                if( PodcastItem.m_Status == guPODCAST_STATUS_READY )
                {
                    if( wxFileExists( PodcastItem.m_FileName ) )
                    {
                        guTrack * Track = new guTrack();
                        if( Track )
                        {
                            Track->m_Type = guTRACK_TYPE_PODCAST;
                            Track->m_SongId = PodcastItem.m_Id;
                            Track->m_FileName = PodcastItem.m_FileName;
                            Track->m_SongName = PodcastItem.m_Title;
                            Track->m_ArtistName = PodcastItem.m_Author;
                            Track->m_AlbumId = PodcastItem.m_ChId;
                            Track->m_AlbumName = PodcastItem.m_Channel;
                            Track->m_Length = PodcastItem.m_Length;
                            Track->m_PlayCount = PodcastItem.m_PlayCount;
                            Track->m_Rating = -1;
                            Track->m_CoverId = 0;
                            Track->m_Year = 0; // Get year from item date
                            Tracks.Add( Track );
                        }
                    }
                    else
                    {
                        PodcastItem.m_Status = guPODCAST_STATUS_ERROR;
                        wxCommandEvent event( guPodcastEvent, ID_PODCASTS_ITEM_UPDATED );
                        event.SetClientData( new guPodcastItem( PodcastItem ) );
                        wxPostEvent( this, event );
                    }
                }
                else if( ( PodcastItem.m_Status == guPODCAST_STATUS_NORMAL ) ||
                         ( PodcastItem.m_Status == guPODCAST_STATUS_ERROR ) )
                {
                    // Download the item
                    guPodcastItemArray AddList;
                    AddList.Add( PodcastItem );
                    m_MainFrame->AddPodcastsDownloadItems( &AddList );

                    PodcastItem.m_Status = guPODCAST_STATUS_PENDING;
                    wxCommandEvent event( guPodcastEvent, ID_PODCASTS_ITEM_UPDATED );
                    event.SetClientData( new guPodcastItem( PodcastItem ) );
                    wxPostEvent( this, event );
                }
            }
        }

        if( Tracks.Count() )
        {
            if( enqueue )
            {
                m_PlayerPanel->AddToPlayList( Tracks, true, aftercurrent );
            }
            else
            {
                m_PlayerPanel->SetPlayList( Tracks );
            }
        }
    }
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::OnPodcastItemActivated( wxCommandEvent &event )
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    OnSelectPodcasts( Config->ReadBool( CONFIG_KEY_GENERAL_ACTION_ENQUEUE, false, CONFIG_PATH_GENERAL ) );
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::OnPodcastItemPlay( wxCommandEvent &event )
{
    OnSelectPodcasts( false );
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::OnPodcastItemEnqueue( wxCommandEvent &event )
{
    OnSelectPodcasts( true, event.GetId() - ID_PODCASTS_ITEM_ENQUEUE_AFTER_ALL );
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::OnPodcastItemDelete( wxCommandEvent &event )
{
    if( wxMessageBox( _( "Are you sure to delete the selected podcast item?" ),
                      _( "Confirm" ),
                      wxICON_QUESTION|wxYES_NO|wxNO_DEFAULT, this ) == wxYES )
    {
        wxArrayInt Selection = m_PodcastsListBox->GetSelectedItems();
        int Index;
        int Count = Selection.Count();

        // Check if in the download thread this items are included and delete them
        guPodcastItemArray Podcasts;
        m_Db->GetPodcastItems( Selection, &Podcasts, m_PodcastsListBox->GetOrder(), m_PodcastsListBox->GetOrderDesc() );
        m_MainFrame->RemovePodcastDownloadItems( &Podcasts );

        for( Index = 0; Index < Count; Index++ )
        {
            m_Db->SetPodcastItemStatus( Selection[ Index ], guPODCAST_STATUS_DELETED );
        }

        m_PodcastsListBox->ReloadItems();
    }
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::OnPodcastItemDownload( wxCommandEvent &event )
{
    wxArrayInt Selection = m_PodcastsListBox->GetSelectedItems();
    guPodcastItemArray DownloadList;
    int Index;
    int Count = Selection.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guPodcastItem PodcastItem;
        m_Db->GetPodcastItemId( Selection[ Index ], &PodcastItem );
        DownloadList.Add( new guPodcastItem( PodcastItem ) );
    }
    m_MainFrame->AddPodcastsDownloadItems( &DownloadList );
    m_PodcastsListBox->ReloadItems( false );
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::OnPodcastItemCopyTo( wxCommandEvent &event )
{
    guTrackArray * Tracks = new guTrackArray();
    m_PodcastsListBox->GetSelectedSongs( Tracks );


    int Index = event.GetId() - ID_COPYTO_BASE;
    if( Index >= guCOPYTO_DEVICE_BASE )
    {
        Index -= guCOPYTO_DEVICE_BASE;
        event.SetId( ID_MAINFRAME_COPYTODEVICE_TRACKS );
    }
    else
    {
        event.SetId( ID_MAINFRAME_COPYTO );
    }
    event.SetInt( Index );
    event.SetClientData( ( void * ) Tracks );
    wxPostEvent( guMainFrame::GetMainFrame(), event );
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::SetSelection( const int type, const int id )
{
    if( type == guMEDIAVIEWER_SELECT_TRACK )
    {
        m_ChannelsListBox->SetSelection( wxNOT_FOUND );
        m_PodcastsListBox->SetSelection( m_PodcastsListBox->FindItem( id ) );
    }
    else if( type == guMEDIAVIEWER_SELECT_ALBUM )
    {
        m_ChannelsListBox->SetSelection( wxNOT_FOUND );
        m_ChannelsListBox->SetSelection( m_ChannelsListBox->FindItem( id ) );
    }
}

// -------------------------------------------------------------------------------- //
void guPodcastPanel::UpdateTrack( const guTrack * )
{
    if( m_PodcastsListBox )
        m_PodcastsListBox->ReloadItems( false );
}


// -------------------------------------------------------------------------------- //
// guChannelsListBox
// -------------------------------------------------------------------------------- //
void guChannelsListBox::GetItemsList( void )
{
    m_PodChannels.Empty();
    int Index;
    int Count;
    Count = ( ( guDbPodcasts * ) m_Db )->GetPodcastChannels( &m_PodChannels );
    for( Index = 0; Index < Count; Index++ )
    {
        m_Items->Add( new guListItem( m_PodChannels[ Index ].m_Id, m_PodChannels[ Index ].m_Title ) );
    }
}

// -------------------------------------------------------------------------------- //
void guChannelsListBox::OnKeyDown( wxKeyEvent &event )
{
    if( event.GetKeyCode() == WXK_DELETE )
    {
        wxCommandEvent CmdEvent( wxEVT_MENU, ID_PODCASTS_CHANNEL_DEL );
        wxPostEvent( this, CmdEvent );
        return;
    }
    event.Skip();
}

// -------------------------------------------------------------------------------- //
int guChannelsListBox::GetSelectedSongs( guTrackArray * Songs, const bool isdrag ) const
{
    return 0;
}

// -------------------------------------------------------------------------------- //
int guChannelsListBox::GetDragFiles( guDataObjectComposite * files )
{
    return ( ( guDbPodcasts * ) m_Db )->GetPodcastFiles( GetSelectedItems(), files );
}

// -------------------------------------------------------------------------------- //
void guChannelsListBox::CreateContextMenu( wxMenu * Menu ) const
{
    wxMenuItem * MenuItem;
    int SelCount = GetSelectedCount();

    MenuItem = new wxMenuItem( Menu, ID_PODCASTS_CHANNEL_ADD, _( "New Channel" ), _( "Add a new podcast channel" ) );
    MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
    Menu->Append( MenuItem );

    if( SelCount )
    {
        MenuItem = new wxMenuItem( Menu, ID_PODCASTS_CHANNEL_DEL, _( "Delete" ), _( "delete this podcast channels and all its items" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit_clear ) );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, ID_PODCASTS_CHANNEL_UPDATE, _( "Update" ), _( "Update the podcast items of the selected channels" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_doc_save ) );
        Menu->Append( MenuItem );

        if( SelCount == 1 )
        {
            MenuItem = new wxMenuItem( Menu, ID_PODCASTS_CHANNEL_PROPERTIES, _( "Properties" ), _( "Edit the podcast channel" ) );
            MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit ) );
            Menu->Append( MenuItem );
        }

        Menu->AppendSeparator();

        guMainFrame * MainFrame = ( guMainFrame * ) guMainFrame::GetMainFrame();
        MainFrame->CreateCopyToMenu( Menu );
    }
}

// -------------------------------------------------------------------------------- //
int guChannelsListBox::FindItem( const int channelid )
{
    int Index;
    int Count = m_Items->Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( m_Items->Item( Index ).m_Id == channelid )
        {
            return Index;
        }
    }
    return wxNOT_FOUND;
}

// -------------------------------------------------------------------------------- //
// guPodcastListBox
// -------------------------------------------------------------------------------- //
guPodcastListBox::guPodcastListBox( wxWindow * parent, guDbPodcasts * db ) :
    guListView( parent, wxLB_MULTIPLE | guLISTVIEW_COLUMN_SELECT | guLISTVIEW_COLUMN_SORTING | guLISTVIEW_ALLOWDRAG )
{
    m_Db = db;

    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->RegisterObject( this );

    m_ColumnNames.Add( _( "Status" ) );
    m_ColumnNames.Add( _( "Title" ) );
    m_ColumnNames.Add( _( "Channel" ) );
    m_ColumnNames.Add( _( "Category" ) );
    m_ColumnNames.Add( _( "Date" ) );
    m_ColumnNames.Add( _( "Length" ) );
    m_ColumnNames.Add( _( "Author" ) );
    m_ColumnNames.Add( _( "Plays" ) );
    m_ColumnNames.Add( _( "Last Played" ) );
    m_ColumnNames.Add( _( "Added" ) );

    m_Order = Config->ReadNum( CONFIG_KEY_PODCASTS_ORDER, 0, CONFIG_PATH_PODCASTS );
    m_OrderDesc = Config->ReadNum( CONFIG_KEY_PODCASTS_ORDERDESC, false, CONFIG_PATH_PODCASTS );

    // Construct the images for the status
    m_Images[ guPODCAST_STATUS_NORMAL ] = NULL;
    m_Images[ guPODCAST_STATUS_PENDING ] = new wxImage( guImage( guIMAGE_INDEX_tiny_status_pending ) );
    m_Images[ guPODCAST_STATUS_DOWNLOADING ] = new wxImage( guImage( guIMAGE_INDEX_tiny_doc_save ) );
    m_Images[ guPODCAST_STATUS_READY ] = new wxImage( guImage( guIMAGE_INDEX_tiny_accept ) );
    m_Images[ guPODCAST_STATUS_DELETED ] = new wxImage( guImage( guIMAGE_INDEX_tiny_status_error ) );
    m_Images[ guPODCAST_STATUS_ERROR ] = new wxImage( guImage( guIMAGE_INDEX_tiny_status_error ) );

    int ColId;
    wxString ColName;
    int index;
    int count = m_ColumnNames.Count();
    for( index = 0; index < count; index++ )
    {
        ColId = Config->ReadNum( wxString::Format( wxT( "id%u" ), index ), index, wxT( "podcasts/columns/ids" ) );

        ColName = m_ColumnNames[ ColId ];

        ColName += ( ( ColId == m_Order ) ? ( m_OrderDesc ? wxT( " ▼" ) : wxT( " ▲" ) ) : wxEmptyString );

        guListViewColumn * Column = new guListViewColumn(
            ColName,
            ColId,
            Config->ReadNum( wxString::Format( wxT( "width%u" ), index ), 80, wxT( "podcasts/columns/widths" ) ),
            Config->ReadBool( wxString::Format( wxT( "show%u" ), index ), true, wxT( "podcasts/columns/shows" ) )
            );
        InsertColumn( Column );
    }

    Bind( guConfigUpdatedEvent, &guPodcastListBox::OnConfigUpdated, this, ID_CONFIG_UPDATED );

    CreateAcceleratorTable();

    ReloadItems();
}


// -------------------------------------------------------------------------------- //
guPodcastListBox::~guPodcastListBox()
{
    guConfig * Config = ( guConfig * ) guConfig::Get();
    Config->UnRegisterObject( this );

    int index;
    int count = m_ColumnNames.Count();
    for( index = 0; index < count; index++ )
    {
        Config->WriteNum( wxString::Format( wxT( "id%u" ), index ),
                          ( * m_Columns )[ index ].m_Id, wxT( "podcasts/columns/ids" ) );
        Config->WriteNum( wxString::Format( wxT( "width%u" ), index ),
                          ( * m_Columns )[ index ].m_Width, wxT( "podcasts/columns/widths" ) );
        Config->WriteBool( wxString::Format( wxT( "show%u" ), index ),
                           ( * m_Columns )[ index ].m_Enabled, wxT( "podcasts/columns/shows" ) );
    }

    Config->WriteNum( CONFIG_KEY_PODCASTS_ORDER, m_Order, CONFIG_PATH_PODCASTS );
    Config->WriteBool( CONFIG_KEY_PODCASTS_ORDERDESC, m_OrderDesc, CONFIG_PATH_PODCASTS );


    for( index = 0; index < guPODCAST_STATUS_ERROR + 1; index++ )
    {
        delete m_Images[ index ];
    }

    Unbind( guConfigUpdatedEvent, &guPodcastListBox::OnConfigUpdated, this, ID_CONFIG_UPDATED );
}

// -------------------------------------------------------------------------------- //
void guPodcastListBox::OnConfigUpdated( wxCommandEvent &event )
{
    int Flags = event.GetInt();
    if( Flags & guPREFERENCE_PAGE_FLAG_ACCELERATORS )
    {
        CreateAcceleratorTable();
    }
}

// -------------------------------------------------------------------------------- //
void guPodcastListBox::CreateAcceleratorTable( void )
{
    wxAcceleratorTable AccelTable;
    wxArrayInt AliasAccelCmds;
    wxArrayInt RealAccelCmds;

    AliasAccelCmds.Add( ID_TRACKS_PLAY );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ALL );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_TRACK );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ALBUM );
    AliasAccelCmds.Add( ID_TRACKS_ENQUEUE_AFTER_ARTIST );

    RealAccelCmds.Add( ID_PODCASTS_ITEM_PLAY );
    RealAccelCmds.Add( ID_PODCASTS_ITEM_ENQUEUE_AFTER_ALL );
    RealAccelCmds.Add( ID_PODCASTS_ITEM_ENQUEUE_AFTER_TRACK );
    RealAccelCmds.Add( ID_PODCASTS_ITEM_ENQUEUE_AFTER_ALBUM );
    RealAccelCmds.Add( ID_PODCASTS_ITEM_ENQUEUE_AFTER_ARTIST );

    if( guAccelDoAcceleratorTable( AliasAccelCmds, RealAccelCmds, AccelTable ) )
    {
        SetAcceleratorTable( AccelTable );
    }
}

// -------------------------------------------------------------------------------- //
void guPodcastListBox::DrawItem( wxDC &dc, const wxRect &rect, const int row, const int col ) const
{
    if( ( * m_Columns )[ col ].m_Id == guPODCASTS_COLUMN_STATUS )
    {
        guPodcastItem * Podcast;
        Podcast = &m_PodItems[ row ];

        if( Podcast->m_Status )
        {
            dc.SetBackgroundMode( wxTRANSPARENT );
            dc.DrawBitmap( * m_Images[ Podcast->m_Status ], rect.x + 3, rect.y + 3, true );
        }
    }
    else
    {
        guListView::DrawItem( dc, rect, row, col );
    }
}

// -------------------------------------------------------------------------------- //
wxString guPodcastListBox::OnGetItemText( const int row, const int col ) const
{
    guPodcastItem * Podcast;
    Podcast = &m_PodItems[ row ];
    switch( ( * m_Columns )[ col ].m_Id )
    {
        case guPODCASTS_COLUMN_TITLE :
          return Podcast->m_Title;

        case guPODCASTS_COLUMN_CHANNEL :
          return Podcast->m_Channel;

        case guPODCASTS_COLUMN_CATEGORY :
          return Podcast->m_Category;

        case guPODCASTS_COLUMN_DATE :
        {
          wxDateTime AddedDate;
          AddedDate.Set( ( time_t ) Podcast->m_Time );
          return AddedDate.FormatDate();
          break;
        }

        case guPODCASTS_COLUMN_LENGTH :
          return LenToString( Podcast->m_Length );

        case guPODCASTS_COLUMN_AUTHOR :
          return Podcast->m_Author;

        case guPODCASTS_COLUMN_PLAYCOUNT :
          return wxString::Format( wxT( "%u" ), Podcast->m_PlayCount );

        case guPODCASTS_COLUMN_LASTPLAY :
          if( Podcast->m_LastPlay )
          {
            wxDateTime LastPlay;
            LastPlay.Set( ( time_t ) Podcast->m_LastPlay );
            return LastPlay.FormatDate();
          }
          else
            return _( "Never" );

        case guPODCASTS_COLUMN_ADDEDDATE :
          wxDateTime AddedDate;
          AddedDate.Set( ( time_t ) Podcast->m_AddedDate );
          return AddedDate.FormatDate();

    }
    return wxEmptyString;
}


// -------------------------------------------------------------------------------- //
void guPodcastListBox::GetItemsList( void )
{
    m_Db->GetPodcastItems( &m_PodItems, m_PodChFilters, m_Order, m_OrderDesc );

    wxCommandEvent event( wxEVT_MENU, ID_MAINFRAME_UPDATE_SELINFO );
    AddPendingEvent( event );
}

// -------------------------------------------------------------------------------- //
void guPodcastListBox::ReloadItems( bool reset )
{
    //
    wxArrayInt Selection;
    int FirstVisible = 0;

    if( reset )
    {
        SetSelection( -1 );
    }
    else
    {
        Selection = GetSelectedItems( false );
        FirstVisible = GetVisibleRowsBegin();
    }

    m_PodItems.Empty();

    GetItemsList();

    SetItemCount( m_PodItems.Count() );

    if( !reset )
    {
      SetSelectedItems( Selection );
      ScrollToRow( FirstVisible );
    }
    RefreshAll();
}

// -------------------------------------------------------------------------------- //
void guPodcastListBox::OnKeyDown( wxKeyEvent &event )
{
    if( event.GetKeyCode() == WXK_DELETE )
    {
        wxCommandEvent CmdEvent( wxEVT_MENU, ID_PODCASTS_ITEM_DEL );
        wxPostEvent( this, CmdEvent );
        return;
    }
    event.Skip();
}

// -------------------------------------------------------------------------------- //
void guPodcastListBox::CreateContextMenu( wxMenu * Menu ) const
{
    int SelCount = GetSelectedCount();
    if( SelCount )
    {
        wxMenuItem * MenuItem;
        MenuItem = new wxMenuItem( Menu, ID_PODCASTS_ITEM_PLAY,
                                wxString( _( "Play" ) ) + guAccelGetCommandKeyCodeString( ID_TRACKS_PLAY ),
                                _( "Play current selected songs" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_player_tiny_light_play ) );
        Menu->Append( MenuItem );

        MenuItem = new wxMenuItem( Menu, ID_PODCASTS_ITEM_ENQUEUE_AFTER_ALL,
                                wxString( _( "Enqueue" ) ) + guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ALL ),
                                _( "Add current selected songs to playlist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        Menu->Append( MenuItem );

        wxMenu * EnqueueMenu = new wxMenu();

        MenuItem = new wxMenuItem( EnqueueMenu, ID_PODCASTS_ITEM_ENQUEUE_AFTER_TRACK,
                                wxString( _( "Current Track" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_TRACK ),
                                _( "Add current selected tracks to playlist after the current track" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );

        MenuItem = new wxMenuItem( EnqueueMenu, ID_PODCASTS_ITEM_ENQUEUE_AFTER_ALBUM,
                                wxString( _( "Current Album" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ALBUM ),
                                _( "Add current selected tracks to playlist after the current album" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );

        MenuItem = new wxMenuItem( EnqueueMenu, ID_PODCASTS_ITEM_ENQUEUE_AFTER_ARTIST,
                                wxString( _( "Current Artist" ) ) +  guAccelGetCommandKeyCodeString( ID_TRACKS_ENQUEUE_AFTER_ARTIST ),
                                _( "Add current selected tracks to playlist after the current artist" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_add ) );
        EnqueueMenu->Append( MenuItem );

        Menu->Append( wxID_ANY, _( "Enqueue After" ), EnqueueMenu );

        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, ID_PODCASTS_ITEM_DEL, _( "Delete" ), _( "Delete the current selected podcasts" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_edit_clear ) );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();

        MenuItem = new wxMenuItem( Menu, ID_PODCASTS_ITEM_DOWNLOAD, _( "Download" ), _( "Download the current selected podcasts" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_doc_save ) );
        Menu->Append( MenuItem );

        Menu->AppendSeparator();

        guMainFrame * MainFrame = ( guMainFrame * ) guMainFrame::GetMainFrame();
        MainFrame->CreateCopyToMenu( Menu );
    }
    else
    {
        wxMenuItem * MenuItem;
        MenuItem = new wxMenuItem( Menu, wxID_ANY, _( "No selected items..." ), _( "Copy the current selected podcasts to a directory or device" ) );
        MenuItem->SetBitmap( guImage( guIMAGE_INDEX_tiny_status_error ) );
        Menu->Append( MenuItem );
    }
}

// -------------------------------------------------------------------------------- //
int inline guPodcastListBox::GetItemId( const int row ) const
{
    return m_PodItems[ row ].m_Id;
}

// -------------------------------------------------------------------------------- //
wxString inline guPodcastListBox::GetItemName( const int row ) const
{
    return m_PodItems[ row ].m_Title;
}

// -------------------------------------------------------------------------------- //
void guPodcastListBox::SetOrder( int columnid )
{
    if( m_Order != columnid )
    {
        m_Order = columnid;
        m_OrderDesc = ( columnid != 0 );
    }
    else
        m_OrderDesc = !m_OrderDesc;

    //m_Db->SetPodcastOrder( m_Order );

    int CurColId;
    int index;
    int count = m_ColumnNames.Count();
    for( index = 0; index < count; index++ )
    {
        CurColId = GetColumnId( index );
        SetColumnLabel( index,
            m_ColumnNames[ CurColId ]  + ( ( CurColId == m_Order ) ?
                ( m_OrderDesc ? wxT( " ▼" ) : wxT( " ▲" ) ) : wxEmptyString ) );
    }

    ReloadItems();
}

// -------------------------------------------------------------------------------- //
void guPodcastListBox::SetFilters( const wxArrayInt &filters )
{
    if( filters.Index( 0 ) != wxNOT_FOUND )
    {
        m_PodChFilters.Empty();
    }
    else
    {
        m_PodChFilters = filters;
    }
}

// -------------------------------------------------------------------------------- //
int guPodcastListBox::GetSelectedSongs( guTrackArray * tracks, const bool isdrag ) const
{
    wxArrayInt Selection = GetSelectedItems();
    int Index;
    int Count = Selection.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        guPodcastItem PodcastItem;
        if( ( m_Db->GetPodcastItemId( Selection[ Index ], &PodcastItem ) != wxNOT_FOUND ) &&
            ( PodcastItem.m_Status == guPODCAST_STATUS_READY ) &&
            ( wxFileExists( PodcastItem.m_FileName ) ) )
        {
            guTrack * Track = new guTrack();
            if( Track )
            {
                Track->m_Type = guTRACK_TYPE_PODCAST;
                Track->m_SongId = PodcastItem.m_Id;
                Track->m_FileName = PodcastItem.m_FileName;
                Track->m_SongName = PodcastItem.m_Title;
                Track->m_ArtistName = PodcastItem.m_Author;
                Track->m_AlbumId = PodcastItem.m_ChId;
                Track->m_AlbumName = PodcastItem.m_Channel;
                Track->m_Length = PodcastItem.m_Length;
                Track->m_PlayCount = PodcastItem.m_PlayCount;
                Track->m_GenreName = wxT( "Podcasts" );
                Track->m_Number = Index;
                Track->m_Rating = -1;
                Track->m_CoverId = 0;
                Track->m_Year = 0; // Get year from item date
                tracks->Add( Track );
            }
        }
    }
    return tracks->Count();
}

// -------------------------------------------------------------------------------- //
int guPodcastListBox::FindItem( const int podcastid )
{
    int Index;
    int Count = m_PodItems.Count();
    for( Index = 0; Index < Count; Index++ )
    {
        if( m_PodItems[ Index ].m_Id == podcastid )
        {
            return Index;
        }
    }
    return wxNOT_FOUND;
}

}

// -------------------------------------------------------------------------------- //
