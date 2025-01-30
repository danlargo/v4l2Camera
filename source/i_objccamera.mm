#import "i_objccamera.h"
#import "objccamera.h"

#import <vector>
#import <map>
#import <string>
#import <iostream>

#import <AVFoundation/AVFoundation.h>
#import <CoreMedia/CoreMedia.h>

i_ObjCCamera::i_ObjCCamera()
{

}

i_ObjCCamera::~i_ObjCCamera()
{
    close();
}

bool i_ObjCCamera::open( std::string device_name )
{
    // create the AVFoundation camera object
    ObjCCamera * tmpCam = [[ObjCCamera alloc] init];
    if( tmpCam == nil ) {
        std::cout << "Failed to create ObjCCamera object" << std::endl;
        return false;
    }

    // open the camera
    NSString * tmp = [NSString stringWithUTF8String:device_name.c_str()];

    bool ret = [tmpCam open:tmp];

    if( ret ) m_objcCamera = tmpCam;
    else m_objcCamera = nil;

    return ret;
}

bool i_ObjCCamera::close()
{
    // close the camera
    bool ret = [(ObjCCamera*)m_objcCamera close];
    m_objcCamera = nil;

    return ret;
}


bool i_ObjCCamera::setCaptureFormat( int i )
{
    // check if it is out of range
    NSArray<AVCaptureDeviceFormat *> * devFormats = [(ObjCCamera*)m_objcCamera getVideoFormats];

    if( i < 0 || i >= devFormats.count ) return false;

    // set the format
    return [(ObjCCamera*)m_objcCamera setCaptureFormat:i];

}

bool i_ObjCCamera::initCapture()
{
    return false;
}


unsigned char * i_ObjCCamera::captureFrame( int * size )
{
    unsigned char * ret = nullptr;

    if( ret == nullptr ) std::cout << "Failed to capture frame" << std::endl;

    return ret;
}


// Video Format discovery
//
std::vector<struct avFormat*> i_ObjCCamera::getVideoFormats()
{
    std::vector<struct avFormat*> ret;

    NSArray<AVCaptureDeviceFormat *> * formats;

    if( m_objcCamera == nil ) std::cout << "Camera not open" << std::endl;
    else 
    {
        NSArray<AVCaptureDeviceFormat *> * devFormats = [(ObjCCamera*)m_objcCamera getVideoFormats];

        if( devFormats == nil ) std::cout << "Failed to get video formats" << std::endl;

        else 
        {
            for( AVCaptureDeviceFormat * format in devFormats )
            {
                CMFormatDescriptionRef formatDescription;
                CMVideoDimensions dimensions;

                formatDescription = (CMFormatDescriptionRef) [format performSelector:@selector(formatDescription)];
                dimensions = CMVideoFormatDescriptionGetDimensions(formatDescription);
                int fourCC = CMFormatDescriptionGetMediaSubType(formatDescription);

                struct avFormat * tmp = new struct avFormat();
                tmp->fourCC = reverseBytes(fourCC);
                tmp->width = dimensions.width;
                tmp->height = dimensions.height;

                ret.push_back( tmp );
            }
        }
    }
    return ret;
}

int i_ObjCCamera::reverseBytes( int in )
{
    int out = 0;
    out |= (in & 0x000000FF) << 24;
    out |= (in & 0x0000FF00) << 8;
    out |= (in & 0x00FF0000) >> 8;
    out |= (in & 0xFF000000) >> 24;

    return out;
}

// Device discovery
//
std::vector<struct avCamera*> i_ObjCCamera::discoverCameras()
{
    NSArray * ret;
    std::vector<struct avCamera*> devs;

    ObjCCamera * m_objcCamera = [[ObjCCamera alloc] init];
    if( m_objcCamera == nil ) std::cout << "Failed to create ObjCCamera object" << std::endl;

    else {
        ret = [m_objcCamera discoverCameras];
                
        for (AVCaptureDevice * device in ret )
        {
            // allocate memory
            struct avCamera * tmp = new struct avCamera();
            tmp->name = std::string([[device localizedName] UTF8String]);
            tmp->uniqueID = std::string([[device uniqueID] UTF8String]);
            tmp->modelID = std::string([[device modelID] UTF8String]);
            tmp->manufacturer = std::string([[device manufacturer] UTF8String]);
            tmp->deviceType = std::string([[device deviceType] UTF8String]);
            if( [device position] == AVCaptureDevicePositionBack ) tmp->rearFacing = true;
            else tmp->rearFacing = false;

            devs.push_back( tmp );
        }

        m_objcCamera = nil;
    }

    return devs;
}