
#include "GstTypeFinder.h"

namespace Guayadeque {

// -------------------------------------------------------------------------------- //
// guMediaFileExtensions
// -------------------------------------------------------------------------------- //
guMediaFileExtensions::guMediaFileExtensions() : guMediaFileExtensionsHashMap()
{
    // do nothing
}

// -------------------------------------------------------------------------------- //
void guMediaFileExtensions::join(guMediaFileExtensions what)
{
    guMediaFileExtensions::iterator it;
    for( it = what.begin(); it != what.end(); ++it )
    {
        wxString key = it->first;
        wxArrayString arr = it->second;
        wxArrayString * myptr = &( this->operator[]( key ) );
        for( size_t i=0; i<arr.Count(); i++ )
        {
            wxString larri = arr[i].Lower();
            if( myptr->Index( larri ) == wxNOT_FOUND )
                myptr->Add( larri );
        }
    }
}

// -------------------------------------------------------------------------------- //
// guGstTypeFinder
// -------------------------------------------------------------------------------- //
guGstTypeFinder::guGstTypeFinder()
{
    FetchMedia();
}


// -------------------------------------------------------------------------------- //
bool guGstTypeFinder::FetchMedia( void )
{
    
    if( READY )
        return true;

    if( !gst_is_initialized() )
        return false;

    m_MediaMutex.Lock();

    GList *iter, *plugin_features = gst_registry_get_feature_list(
            gst_registry_get(),
            GST_TYPE_TYPE_FIND_FACTORY );

    for( iter=plugin_features; iter != NULL; iter=iter->next )
    {
        if( iter->data != NULL )
        {
            GstPluginFeature *p_feature = GST_PLUGIN_FEATURE( iter->data );
            GstTypeFindFactory *factory;
            const gchar *const *extensions;
            factory = GST_TYPE_FIND_FACTORY( p_feature );
            wxArrayString t_value;
            extensions = gst_type_find_factory_get_extensions( factory );
            if( extensions != NULL )
                for( guint i = 0; extensions[i]; i++ )
                {
                    wxString ext = extensions[i];
                    t_value.Add( ext.Lower() );
                }
            wxString t_name = gst_plugin_feature_get_name( p_feature );
            m_Media[ t_name.Lower() ] = t_value;
        }
    }
    gst_plugin_feature_list_free(plugin_features);
    READY = !m_Media.empty();
    if( READY )
        InitMediaTypes();
    m_MediaMutex.Unlock();
    return READY;
}

// -------------------------------------------------------------------------------- //
guMediaFileExtensions guGstTypeFinder::GetMediaByPrefix( const wxString& media_type_prefix )
{
    FetchMedia();
    guMediaFileExtensions res;
    guMediaFileExtensions::iterator it;
    for( it = m_Media.begin(); it != m_Media.end(); ++it )
    {
        wxString key = it->first;

        if( media_type_prefix.IsNull() || key.StartsWith( media_type_prefix ) )
            res[key] = it->second;
    }
    return res;
}

// -------------------------------------------------------------------------------- //
guMediaFileExtensions guGstTypeFinder::GetMedia( void )
{
    guMediaFileExtensions res;
    for( size_t i = 0; i<m_MediaTypePrefixes.Count(); ++i )
        res.join( GetMediaByPrefix( m_MediaTypePrefixes[i] ) );
    return res;
}

// -------------------------------------------------------------------------------- //
wxArrayString guGstTypeFinder::GetMediaTypes( void )
{
    wxArrayString res;
    for( size_t i = 0; i<m_MediaTypePrefixes.Count(); ++i )
    {
        wxArrayString sub = GetMediaTypesByPrefix(m_MediaTypePrefixes[i]);
        for( size_t j = 0; j<sub.Count(); ++j )
            res.Add(sub[j]);

    }
    return res;
}

// -------------------------------------------------------------------------------- //
wxArrayString guGstTypeFinder::GetMediaTypesByPrefix( const wxString &media_type_prefix )
{
    wxArrayString res;
    guMediaFileExtensions med = GetMediaByPrefix(media_type_prefix);
    guMediaFileExtensions::iterator it;
    for( it = med.begin(); it != med.end(); ++it )
        res.Add(it->first);
    return res;
}

// -------------------------------------------------------------------------------- //
wxArrayString guGstTypeFinder::GetExtensions( void )
{
    wxArrayString res;
    for( size_t i = 0; i<m_MediaTypePrefixes.Count(); ++i )
    {
        wxArrayString sub = GetExtensionsByPrefix(m_MediaTypePrefixes[i]);
        for( size_t j = 0; j<sub.Count(); ++j )
            res.Add(sub[j]);

    }
    return res;
}

// -------------------------------------------------------------------------------- //
wxArrayString guGstTypeFinder::GetExtensionsByPrefix( const wxString &media_type_prefix )
{
    FetchMedia();
    wxArrayString res;
    guMediaFileExtensions med = GetMediaByPrefix(media_type_prefix);
    guMediaFileExtensions::iterator it;
    for( it = med.begin(); it != med.end(); ++it )
    {
        wxArrayString arr = it->second;
        for( size_t i = 0; i<arr.Count(); i++)
            res.Add(arr[i]);
    }
    return res;
}

// -------------------------------------------------------------------------------- //
void guGstTypeFinder::AddMediaTypePrefix( const wxString &media_type_prefix )
{
    if( m_MediaTypePrefixes.Index( media_type_prefix ) == wxNOT_FOUND )
        m_MediaTypePrefixes.Add(media_type_prefix);
}

// -------------------------------------------------------------------------------- //
bool guGstTypeFinder::HasPrefixes( void )
{
    return m_MediaTypePrefixes.Count() > 0;
}

void guGstTypeFinder::AddMediaExtension( const wxString &media_type, const wxString &extension )
{
    AddMediaTypePrefix(media_type);
    if( m_Media.count( media_type ) )
        m_Media[ media_type ].Add( extension );
    else
    {
        wxArrayString addme;
        addme.Add(extension);
        m_Media[ media_type ] = addme;
    }
}


void guGstTypeFinder::InitMediaTypes( void )
{

    // supported media types
    //
    AddMediaTypePrefix( "audio/" ); // all gstreamer audio
    AddMediaTypePrefix( "video/" ); // all gstreamer video
    AddMediaTypePrefix( "avtype_" ); // additional gstreamer formats from libav
    AddMediaTypePrefix( "application/mxf" ); // Material Exchange Format
    AddMediaTypePrefix( "application/ogg" ); // OGG formats
    AddMediaTypePrefix( "application/sdp" ); // SDP mediastream file
    AddMediaTypePrefix( "application/smil" ); // Synchronized Multimedia Integration Language
    AddMediaTypePrefix( "application/vnd.rn-realmedia" ); // RealMedia
    AddMediaTypePrefix( "application/x-pn-realaudio" ); // RealAudio
    AddMediaTypePrefix( "application/x-3gp" ); // 3GP video
    AddMediaTypePrefix( "application/x-ape" ); // APE
    AddMediaTypePrefix( "application/x-hls" ); // UTF8 M3U
    AddMediaTypePrefix( "application/x-ogm-audio" ); // sounds like something with sound :) 
    AddMediaTypePrefix( "application/x-yuv4mpeg" ); // sounds like something with sound :)

    // some gstreamer supported formats (DSD & exotic tracker audio)
    // are not listed as GST_TYPE_FIND_FACTORY for some reason
    // but those are tested to work with gstreamer-libav 1.16.2
    //
    AddMediaExtension( "application/x-gst-av-dsf",  "dsf" ); // DSD
    AddMediaExtension( "application/x-gst-av-iff", "maud"  ); // Amiga MAUD audio format
    AddMediaExtension( "audio/x-ay",  "emul" ); // AY Emul
    AddMediaExtension( "audio/x-mod",  "mmd1" ); // OctaMED MMD1
    AddMediaExtension( "audio/x-mod", "mptm" ); // OpenMPT
    AddMediaExtension( "audio/x-mod", "okta" ); // Oktalyzer
    AddMediaExtension( "audio/x-sid", "psid" ); // PlaySID
    AddMediaExtension( "audio/x-svx", "8svx" ); // 8-Bit Sampled Voice
    AddMediaExtension( "audio/x-vgm", "vgz"  ); // Video Game Music - Sega Megadrive (somtimes only after gunzip)

}

}

// the end :)

