#include <cstring>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <locale>
#include <codecvt>
#include <fcntl.h>

#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>

#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")


#include "wincamera.h"

WinCamera::WinCamera( std::wstring device_name )
    : V4l2Camera()
{
    m_devName = device_name;
    m_userName = "";
    m_capabilities = 0;
    m_isOpen = false;
	m_pReader = NULL;
    m_bufferMode = notset;

    m_cameraType = "MediaFoundation device";

    m_healthCounter = 0;

}


WinCamera::~WinCamera()
{
}


void WinCamera::initMF()
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if( FAILED(hr) ) 
    {
        std::cerr << "Failed to initialize COM library: " << std::hex << hr << std::endl;
        return;
    }
    else {
        hr = S_OK;
        hr = MFStartup(MF_VERSION);
        if( FAILED(hr) )  std::cerr << "Failed to initialize Media Foundation: " << std::hex << hr << std::endl;
    }
}


void WinCamera::shutdownMF()
{
	MFShutdown();
    CoUninitialize();

}

std::vector< WinCamera *> WinCamera::discoverCameras()
{
    std::vector< WinCamera *> camList;
    
    HRESULT hr = S_OK;

    IMFAttributes* pAttributes = NULL;
    IMFActivate** ppDevices = NULL;
    UINT32 count = 0;

    // Create an attribute store to specify the source type we are interested in
    hr = MFCreateAttributes(&pAttributes, 1);
    if (SUCCEEDED(hr)) {
        // Request video capture devices
        hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
        if (SUCCEEDED(hr)) {
            // Enumerate devices
            hr = MFEnumDeviceSources(pAttributes, &ppDevices, &count);
            if (SUCCEEDED(hr)) {
                for (UINT32 i = 0; i < count; i++) 
                {
                    WCHAR* friendlyName = NULL;
					unsigned int  length = 0;
                    hr = ppDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &friendlyName, &length);
                    if (SUCCEEDED(hr)) {
                        if (friendlyName)
                        {
                            // create a WinCamera object
                            WinCamera* tmp = new WinCamera(friendlyName);
                            camList.push_back(tmp);
                            CoTaskMemFree(friendlyName);

							// now open the camera, get the controls and videos modes
                            if (tmp->open())
                            {
                                tmp->enumCapabilities();
                                tmp->enumControls();
                                tmp->enumVideoModes();
								tmp->close();
                            }
                        }
                    }
                }
                // Release the devices
                for (UINT32 i = 0; i < count; i++) 
                {
                    ppDevices[i]->Release();
                }
                CoTaskMemFree(ppDevices);
            }
        }
        pAttributes->Release();
    }

    return camList;
}


std::string WinCamera::getDevName()
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::string narrowString = converter.to_bytes( m_devName );

    return narrowString;
}


bool WinCamera::canFetch()
{
    if (m_isOpen) return true;
    return false;
}


bool WinCamera::canRead()
{
    return false;
}


int WinCamera::setValue( int id, int newVal, bool openOnDemand )
{
    int ret = -1;

    return ret;
}

int WinCamera::getValue( int id, bool openOnDemand )
{
    int ret = -1;

    return ret;
}

bool WinCamera::open()
{
    bool ret = false;
    
    IMFAttributes* pAttributes = NULL;
    IMFActivate** ppDevices = NULL;
    UINT32 count = 0;

    HRESULT hr = MFCreateAttributes(&pAttributes, 1);
    if (SUCCEEDED(hr)) {
        hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
        if (SUCCEEDED(hr)) {
            hr = MFEnumDeviceSources(pAttributes, &ppDevices, &count);
            if (SUCCEEDED(hr)) {
                for (UINT32 i = 0; i < count; i++) {
                    WCHAR* friendlyName = NULL;
                    unsigned int length = 0;
                    hr = ppDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &friendlyName, &length);
                    if (SUCCEEDED(hr) ) 
                    {
                        if (friendlyName == m_devName) 
                        {
                            IMFMediaSource* pSource = NULL;
                            hr = ppDevices[i]->ActivateObject(IID_PPV_ARGS(&pSource));
                            if( SUCCEEDED(hr) ) 
                            {
                                hr = MFCreateSourceReaderFromMediaSource(pSource, NULL, &m_pReader);
                                if( SUCCEEDED(hr) ) {
                                    m_isOpen = true;
                                }
                                pSource->Release();
                            }
                        }
                        CoTaskMemFree(friendlyName);
                    }
                }
                for (UINT32 i = 0; i < count; i++) {
                    ppDevices[i]->Release();
                }
                CoTaskMemFree(ppDevices);
            }
        }
        pAttributes->Release();
    }
    return m_isOpen;
}


bool WinCamera::isOpen()
{
    return m_isOpen;
}


bool WinCamera::enumCapabilities()
{
    m_capabilities = 1;
    return true;
}


void WinCamera::close()
{
    if (m_pReader) {
        m_pReader->Release();
        m_pReader = nullptr;
        m_isOpen = false;
    }
}


bool WinCamera::setFrameFormat( struct v4l2cam_video_mode vm )
{
    return false;
}


bool WinCamera::init( enum v4l2cam_fetch_mode newMode )
{
    bool ret = false;

    if( !this->isOpen() ) log( "Unable to call init() as device is NOT open", info );

    else 
    {
        ret = false;
    }

    return ret;
}

struct v4l2cam_image_buffer * WinCamera::fetch( bool lastOne )
{
    struct v4l2cam_image_buffer * retBuffer = nullptr;

    if( !this->isOpen() ) log( "Unable to call fetch() as no device is open", warning );

    else 
    {
    }

    return retBuffer;
}


bool WinCamera::enumVideoModes()
{
    bool ret = false;

    // clear the existing video structure
    this->m_modes.clear();

    // make sure fid is valid
    if( !this->isOpen() ) log( "Unable to call enumVideoModes() as device is NOT open", warning );
    else
    {
        HRESULT hr = S_OK;
        DWORD streamIndex = 0;
        DWORD mediaTypeIndex = 0;
        IMFMediaType* pType = NULL;

        while (SUCCEEDED(hr)) 
        {
            hr = m_pReader->GetNativeMediaType(streamIndex, mediaTypeIndex, &pType);
            if (hr == MF_E_NO_MORE_TYPES) {
                // No more media types for this stream, move to the next stream
                streamIndex++;
                mediaTypeIndex = 0;
                hr = S_OK;
                continue;
            }
            else if (FAILED(hr)) {
                break;
            }

            // Extract and print media type information
            UINT32 width = 0, height = 0;
            MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &width, &height);

            UINT32 numerator = 0, denominator = 0;
            MFGetAttributeRatio(pType, MF_MT_FRAME_RATE, &numerator, &denominator);
            float frameRate = (float)numerator / (float)denominator;

            GUID subtype = { 0 };
            pType->GetGUID(MF_MT_SUBTYPE, &subtype);
            std::string pixelFormat = GetLongNameFromGUID(subtype);

            // only grab the records with a frame rate of 30
            if (30. == frameRate)
            {
                //std::cerr << "Stream " << streamIndex << ", Media Type " << mediaTypeIndex << ": "
                //    << width << "x" << height << ", "
                //    << pixelFormat << std::endl;

				struct v4l2cam_video_mode vm;

				vm.fourcc = GetFourCCFromGUID(subtype);
				vm.height = height;
				vm.width = width;
				vm.format_str = pixelFormat;
				m_modes.push_back(vm);
            }

            pType->Release();
            mediaTypeIndex++;
        }

        ret = true;
    }

    return ret;
}

int WinCamera::GetFourCCFromGUID(const GUID& guid)
{
    if (guid == MFVideoFormat_RGB32) return 'RGB ';
    if (guid == MFVideoFormat_ARGB32) return 'ARGB';
    if (guid == MFVideoFormat_RGB24) return 'RGB3';
    if (guid == MFVideoFormat_RGB555) return 'RGB5';
    if (guid == MFVideoFormat_RGB565) return 'RGB6';
    if (guid == MFVideoFormat_YUY2) return 'YUY2';
    if (guid == MFVideoFormat_NV12) return 'NV12';
    if (guid == MFVideoFormat_I420) return 'I420';
    if (guid == MFVideoFormat_H264) return 'H264';
    if (guid == MFVideoFormat_MJPG) return 'MJPG';

    // Add more formats as needed
    return 0;
}

std::string WinCamera::GetLongNameFromGUID(const GUID& guid)
{
    if (guid == MFVideoFormat_RGB32) return "RGB32";
    if (guid == MFVideoFormat_ARGB32) return "ARGB (32-bit bitmap)";
    if (guid == MFVideoFormat_RGB24) return "RGB (24-bit bitmap)";
    if (guid == MFVideoFormat_RGB555) return "RGB555";
    if (guid == MFVideoFormat_RGB565) return "RGB565";
    if (guid == MFVideoFormat_YUY2) return "Interleaveed YVU 4:2:0";
    if (guid == MFVideoFormat_NV12) return "Interleaved YUV 4:2:0";
    if (guid == MFVideoFormat_I420) return "Planar YUV 4:2:0";
    if (guid == MFVideoFormat_H264) return "H264 Video";
    if (guid == MFVideoFormat_MJPG) return "Motion JPEG";

    // Add more formats as needed
    return "Unknown";
}


bool WinCamera::enumControls()
{
    bool ret = false;

    // clear the existing control structure
    m_controls.clear();

    // make sure fid is valid
    if( !isOpen() ) log( "Unable to call enumControls() as device is NOT open", warning );

    else
    {
    }

    return ret;
}

std::string WinCamera::cntrlTypeToString( int type )
{
    std::string ret = "unknown";

    switch( type )
    {
        default:
            break;
    }

    return ret;
}

std::vector<std::string> WinCamera::capabilitiesToStr()
{
    std::vector<std::string> ret = {};

    if (0 == m_capabilities) return ret;


    ret.push_back("can stream");
    ret.push_back("can query pixel formats ");

    ret.push_back("can read metadata ");
    
	return ret;
}
