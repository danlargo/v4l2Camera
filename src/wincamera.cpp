#include <cstring>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
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
#include <comdef.h>
#include <atlstr.h> // Add this include for CString
#include <tchar.h>  // Add this include for _T

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
	m_pSource = NULL;
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
                        if( friendlyName && (friendlyName == m_devName) )
                        {
                            hr = ppDevices[i]->ActivateObject(IID_PPV_ARGS(&m_pSource));
                            if( SUCCEEDED(hr) ) 
                            {
                                hr = MFCreateSourceReaderFromMediaSource(m_pSource, NULL, &m_pReader);
                                if( SUCCEEDED(hr) ) {
                                    m_isOpen = true;
                                }
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
	if (m_pSource) {
		m_pSource->Release();
		m_pSource = nullptr;
	}

    if (m_pReader) {
        m_pReader->Release();
        m_pReader = nullptr;
        m_isOpen = false;
    }
}


bool WinCamera::setFrameFormat( struct v4l2cam_video_mode vm )
{
    bool ret = false;

    HRESULT hr = S_OK;
    DWORD streamIndex = MF_SOURCE_READER_FIRST_VIDEO_STREAM;
    DWORD mediaTypeIndex = 0;
    IMFMediaType* pType = NULL;

    // walk thru the native media types to find a matching one

    while (SUCCEEDED(hr))
    {
        hr = m_pReader->GetNativeMediaType(streamIndex, mediaTypeIndex, &pType);
        if (hr == MF_E_NO_MORE_TYPES) break;
		else if (FAILED(hr)) break;

        // Extract and print media type information
        UINT32 width = 0, height = 0;
        MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &width, &height);

        UINT32 numerator = 0, denominator = 0;
        MFGetAttributeRatio(pType, MF_MT_FRAME_RATE, &numerator, &denominator);
        float frameRate = (float)numerator / (float)denominator;

        GUID subtype = { 0 };
        pType->GetGUID(MF_MT_SUBTYPE, &subtype);

        // only check records that match the frame rate
        if (30. == frameRate)
        {
            if (GetFourCCFromGUID(subtype) == vm.fourcc)
            {
                if ((width == vm.width) && (height == vm.height))
                {
                    hr = m_pReader->SetCurrentMediaType(streamIndex, NULL, pType);
                    if (FAILED(hr)) log("SetCurrentMediaType failed : " + HResultToString(hr), error);
                    else
                    {
                        ret = true;
                        break;
                    }
                }
            }
        }

        pType->Release();
        mediaTypeIndex++;
    }

/*
    HRESULT hr = m_pReader->GetCurrentMediaType(streamIndex, &pType);
    if (FAILED(hr)) log("GetCurrentMediaType failed : " + HResultToString(hr), error);
    else
    {


        
		GUID videoType = GetGuidFromFourCC(vm.fourcc);
        if (videoType == GUID({ 0 }) ) log("Unknonw Video Type Requested", error);
        else
        {
            hr = pType->SetGUID(MF_MT_SUBTYPE, videoType);
            if (FAILED(hr)) log("SetGUID MF_MT_SUBTYPE YUY2 failed : " + HResultToString(hr), error);
            else
            {
                hr = MFSetAttributeSize(pType, MF_MT_FRAME_SIZE, vm.width, vm.height);
                if (FAILED(hr)) log("MFSetAttributeSize failed : " + HResultToString(hr), error);

                hr = m_pReader->SetCurrentMediaType(streamIndex, NULL, pType);
                if (FAILED(hr)) log("SetCurrentMediaType : " + HResultToString(hr), error);
                else ret = true;

            }
            pType->Release();
        
    }
*/

    return ret;
}


std::string WinCamera::HResultToString(HRESULT hr)
{
    std::string ret = "";

    _com_error error(hr); 
    CString cs; 
    cs.Format(_T("Error 0x%08x: %s"), hr, error.ErrorMessage());
	ret = CStringA(cs).GetString();
    return ret; 
}


bool WinCamera::init( enum v4l2cam_fetch_mode newMode )
{
    bool ret = false;

    if( !this->isOpen() ) log( "Unable to call init() as device is NOT open", info );
    else 
    {
        ret = true;
    }

    return ret;
}

struct v4l2cam_image_buffer * WinCamera::fetch( bool lastOne )
{
    struct v4l2cam_image_buffer * retBuffer = nullptr;
    IMFSample* pSample = NULL;

    if( !this->isOpen() ) log( "Unable to call fetch() as no device is open", warning );

    else 
    {
        DWORD dwFlags = 0;
        DWORD streamIndex = MF_SOURCE_READER_FIRST_VIDEO_STREAM;

        for (int i = 0; i < 10; ++i)
        {
            HRESULT hr = m_pReader->ReadSample(streamIndex, 0, NULL, &dwFlags, NULL, &pSample);
            if (SUCCEEDED(hr) && pSample) break;
            Sleep(100);
        }
    }

    // check if we have data or a timeout
    if (pSample)
    {
        // allocate the return buffer
		retBuffer = new struct v4l2cam_image_buffer;
		retBuffer->length = 0;
		retBuffer->buffer = nullptr;
		// get the buffer
		IMFMediaBuffer* pBuffer = NULL;
		DWORD cbBuffer = 0;
		HRESULT hr = pSample->GetBufferByIndex(0, &pBuffer);
		if (SUCCEEDED(hr))
		{
			if (SUCCEEDED(hr))
			{
                // copy the data
                BYTE* pData = nullptr;
                DWORD maxLength = 0;
                DWORD currentLength = 0;

                // Lock the buffer to get a pointer to the memory
                HRESULT hr = pBuffer->Lock(&pData, &maxLength, &currentLength);
                if (FAILED(hr)) log("Failed to lock the IMFMediaBuffer: " + HResultToString(hr), error);
                else
                {
                    retBuffer->length = currentLength;
                    retBuffer->buffer = new unsigned char[currentLength];

                    // Copy the data to the destination buffer
                    memcpy(retBuffer->buffer, pData, currentLength);

                    // Unlock the buffer
                    hr = pBuffer->Unlock();
                    if (FAILED(hr)) log("Failed to unlock the IMFMediaBuffer: " + HResultToString(hr), error);
                }
			}
		}
		// release the buffer
		pBuffer->Release();
		pSample->Release();

    } else log("Timeout reading frame from the camera", error);

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
        DWORD streamIndex = MF_SOURCE_READER_FIRST_VIDEO_STREAM;
        DWORD mediaTypeIndex = 0;
        IMFMediaType* pType = NULL;

        while (SUCCEEDED(hr)) 
        {
            hr = m_pReader->GetNativeMediaType(streamIndex, mediaTypeIndex, &pType);
            if (hr == MF_E_NO_MORE_TYPES) {
                // No more media types for this stream, move to the next stream
                //streamIndex++;
                //mediaTypeIndex = 0;
                //hr = S_OK;
                //continue;
                break;
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

const GUID WinCamera::GetGuidFromFourCC(int fourCC)
{
    GUID ret = GUID({ 0 });

    if (fourCC == fourcc_charArray_to_int((unsigned char*)"RGB ")) ret = MFVideoFormat_RGB32;
    if (fourCC == fourcc_charArray_to_int((unsigned char*)"ARGB")) ret = MFVideoFormat_ARGB32;
    if (fourCC == fourcc_charArray_to_int((unsigned char*)"RGB3")) ret = MFVideoFormat_RGB24;
    if (fourCC == fourcc_charArray_to_int((unsigned char*)"RGB5")) ret = MFVideoFormat_RGB555;
    if (fourCC == fourcc_charArray_to_int((unsigned char*)"RGB6")) ret = MFVideoFormat_RGB565;
    if (fourCC == fourcc_charArray_to_int((unsigned char*)"YUY2")) ret = MFVideoFormat_YUY2;
    if (fourCC == fourcc_charArray_to_int((unsigned char*)"NV12")) ret = MFVideoFormat_NV12;
    if (fourCC == fourcc_charArray_to_int((unsigned char*)"I420")) ret = MFVideoFormat_I420;
    if (fourCC == fourcc_charArray_to_int((unsigned char*)"H264")) ret = MFVideoFormat_H264;
    if (fourCC == fourcc_charArray_to_int((unsigned char*)"MJPG")) ret = MFVideoFormat_MJPG;

    // Add more formats as needed
    return ret;
}

int WinCamera::GetFourCCFromGUID(const GUID& guid)
{
    if (guid == MFVideoFormat_RGB32) return fourcc_charArray_to_int((unsigned char*)"RGB ");
    if (guid == MFVideoFormat_ARGB32) return fourcc_charArray_to_int((unsigned char*)"ARGB");
    if (guid == MFVideoFormat_RGB24) return fourcc_charArray_to_int((unsigned char*)"RGB3");
    if (guid == MFVideoFormat_RGB555) return fourcc_charArray_to_int((unsigned char*)"RGB5");
    if (guid == MFVideoFormat_RGB565) return fourcc_charArray_to_int((unsigned char*)"RGB6");
    if (guid == MFVideoFormat_YUY2) return fourcc_charArray_to_int((unsigned char*)"YUY2");
    if (guid == MFVideoFormat_NV12) return fourcc_charArray_to_int((unsigned char*)"NV12");
    if (guid == MFVideoFormat_I420) return fourcc_charArray_to_int((unsigned char*)"I420");
    if (guid == MFVideoFormat_H264) return fourcc_charArray_to_int((unsigned char*)"H264");
    if (guid == MFVideoFormat_MJPG) return fourcc_charArray_to_int((unsigned char*)"MJPG");

    // Add more formats as needed
    return 0;
}

std::string WinCamera::GetLongNameFromGUID(const GUID& guid)
{
    if (guid == MFVideoFormat_RGB32) return "RGB32 bitmap";
    if (guid == MFVideoFormat_ARGB32) return "ARGB (32-bit bitmap)";
    if (guid == MFVideoFormat_RGB24) return "RGB (24-bit bitmap)";
    if (guid == MFVideoFormat_RGB555) return "RGB555";
    if (guid == MFVideoFormat_RGB565) return "RGB565";
    if (guid == MFVideoFormat_YUY2) return "YUY2 4:2:2 (interleaved)";
    if (guid == MFVideoFormat_NV12) return "Y:UV 4:2:0 (interleaved)";
    if (guid == MFVideoFormat_I420) return "YUV 4:2:0 (planar)";
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
