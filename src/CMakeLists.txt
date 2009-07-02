IF( DEBUG )
	ADD_DEFINITIONS( -g )
ENDIF( DEBUG )

INCLUDE_DIRECTORIES(
	${guayadeque_SOURCE_DIR}
	${guayadeque_SOURCE_DIR}/wx
	${guayadeque_SOURCE_DIR}/wx/curl
	${guayadeque_SOURCE_DIR}/wxsqlite3
        ${guayadeque_SOURCE_DIR}/dbus
        ${GSTREAMER_INCLUDE_DIRS}
        ${SQLITE3_INCLUDE_DIRS}
        ${LIBCURL_INCLUDE_DIRS}
        ${LIBTAG_INCLUDE_DIRS}
        ${LIBDBUS_INCLUDE_DIRS} )


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
	DbLibrary.cpp
	DbLibrary.h
	DbListBox.cpp
	DbListBox.h
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
	wxsqlite3/wxsqlite3.cpp 
	curl/base.cpp
	curl/http.cpp 
        dbus/gudbus.cpp
        dbus/gudbus.h
	dbus/mpris.cpp
	dbus/mpris.h
       )

TARGET_LINK_LIBRARIES( guayadeque
	${wxWidgets_LIBRARIES}
	${GSTREAMER_LIBRARIES}
	${SQLITE3_LIBRARIES}
	${LIBCURL_LIBRARIES}
        ${LIBTAG_LIBRARIES}
        ${LIBDBUS_LIBRARIES}
	)

INSTALL( TARGETS guayadeque
         RUNTIME DESTINATION bin )

