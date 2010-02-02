IF( DEBUG )
	ADD_DEFINITIONS( -g )
ENDIF( DEBUG )

IF( PROFILE )
	ADD_DEFINITIONS( -pg )
ENDIF( PROFILE )

INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/wx
	${CMAKE_CURRENT_SOURCE_DIR}/wx/curl
	${CMAKE_CURRENT_SOURCE_DIR}/wxsqlite3
        ${CMAKE_CURRENT_SOURCE_DIR}/dbus
	${CMAKE_CURRENT_SOURCE_DIR}/hmac
	${CMAKE_CURRENT_SOURCE_DIR}/taglib-extras
	${CMAKE_CURRENT_SOURCE_DIR}/taglib-extras/asf
	${CMAKE_CURRENT_SOURCE_DIR}/taglib-extras/mp4
	${CMAKE_CURRENT_SOURCE_DIR}/taglib-extras/wav
	${CMAKE_CURRENT_SOURCE_DIR}/taglib-extras/audible
	${CMAKE_CURRENT_SOURCE_DIR}/taglib-extras/rmff
        ${GSTREAMER_INCLUDE_DIRS}
        ${SQLITE3_INCLUDE_DIRS}
        ${LIBCURL_INCLUDE_DIRS}
        ${LIBTAG_INCLUDE_DIRS}
	${LIBMP4_INCLUDE_DIRS}
	${LIBFLAC_INCLUDE_DIRS}
        ${LIBDBUS_INCLUDE_DIRS} 
)

SET( WXSQLITE3_SOURCES
	wxsqlite3/wxsqlite3.cpp 
)

SET( CURL_SOURCES
	curl/base.cpp
	curl/http.cpp 
)

SET( DBUS_SOURCES
        dbus/gudbus.cpp
        dbus/gudbus.h
	dbus/mpris.cpp
	dbus/mpris.h
	dbus/mmkeys.cpp
	dbus/mmkeys.h
	dbus/gsession.cpp
	dbus/gsession.h
)

SET( HMAC_SHA2_SOURCES
	hmac/hmac_sha2.c
	hmac/hmac_sha2.h
	hmac/sha2.c
	hmac/sha2.h
)

ADD_EXECUTABLE( guayadeque
	AlListBox.cpp
	AlListBox.h
	ArListBox.cpp
	ArListBox.h
	ArrayStringArray.cpp
	ArrayStringArray.h
	AudioScrobble.cpp
	AudioScrobble.h
	Commands.h
	Config.cpp
	Config.h
	ConfirmExit.cpp
	ConfirmExit.h
	CoverEdit.cpp
	CoverEdit.h
	CoverFrame.cpp
	CoverFrame.h
	Db.cpp
	Db.h
	DbLibrary.cpp
	DbLibrary.h
	DbCache.cpp
	DbCache.h
	AutoPulseGauge.cpp
	AutoPulseGauge.h
	GeListBox.cpp
	GeListBox.h
	Images.cpp
	Images.h
	ItemListBox.cpp
	ItemListBox.h
	LabelEditor.cpp
	LabelEditor.h
	LastFM.cpp
	LastFM.h
	LastFMPanel.cpp
	LastFMPanel.h
	LibPanel.cpp
	LibPanel.h
	LyricsPanel.cpp
	LyricsPanel.h
	MainApp.cpp
	MainApp.h
	MainFrame.cpp
	MainFrame.h
	MD5.cpp
	MD5.h
	MediaCtrl.cpp
	MediaCtrl.h
	PlayerPanel.cpp
	PlayerPanel.h
	PlayList.cpp
	PlayList.h
	Preferences.cpp
	Preferences.h
	RadioGenreEditor.cpp
	RadioGenreEditor.h
	RadioPanel.cpp
	RadioPanel.h
	Shoutcast.cpp
	Shoutcast.h
	SoListBox.cpp
	SoListBox.h
	SplashWin.cpp
	SplashWin.h
	StatusBar.cpp
	StatusBar.h
	TagInfo.cpp
	TagInfo.h
	TaListBox.cpp
	TaListBox.h
	TaskBar.cpp
	TaskBar.h
	ThreadArray.h
	TrackEdit.cpp
	TrackEdit.h
	Utils.cpp
	Utils.h
	VolumeFrame.cpp
	VolumeFrame.h
        OnlineLinks.cpp
 	OnlineLinks.h
	LibUpdate.cpp
	LibUpdate.h
	CoverFetcher.cpp
	CoverFetcher.h
	Google.cpp
	Google.h
	Amazon.cpp
	Amazon.h
	RatingCtrl.cpp
	RatingCtrl.h
	PlayListPanel.cpp
	PlayListPanel.h
	DynamicPlayList.cpp
	DynamicPlayList.h
        ListView.cpp
	ListView.h
	PLSoListBox.cpp
	PLSoListBox.h
	Base64.cpp
	Base64.h
	ApeTag.cpp
	ApeTag.h
	Discogs.cpp
	Discogs.h
	MusicDns.cpp
	MusicDns.h
	MusicBrainz.cpp
	MusicBrainz.h
	Podcasts.cpp
	Podcasts.h
	PodcastsPanel.cpp
	PodcastsPanel.h
	ChannelEditor.cpp
	ChannelEditor.h
	NewChannel.cpp
	NewChannel.h
	RadioEditor.cpp
	RadioEditor.h
	PlayListAppend.cpp
	PlayListAppend.h
	TrackChangeInfo.cpp
	TrackChangeInfo.h
	Equalizer.cpp
	Equalizer.h
	ShowImage.cpp
	ShowImage.h
	StaticBitmap.cpp
	StaticBitmap.h
	LastFMCovers.cpp
	LastFMCovers.h
	PlayListFile.cpp
	PlayListFile.h
	AuiNotebook.cpp
	AuiNotebook.h
	${WXSQLITE3_SOURCES}
	${CURL_SOURCES}
    	${DBUS_SOURCES}
	${HMAC_SHA2_SOURCES}
       )

TARGET_LINK_LIBRARIES( guayadeque
	${wxWidgets_LIBRARIES}
	${GSTREAMER_LIBRARIES}
	${SQLITE3_LIBRARIES}
	${LIBCURL_LIBRARIES}
        ${LIBTAG_LIBRARIES}
	${LIBMP4_LIBRARIES}
	${LIBFLAC_LIBRARIES}
        ${LIBDBUS_LIBRARIES}
	)

INSTALL( TARGETS guayadeque
         RUNTIME DESTINATION bin )

