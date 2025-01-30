//
//  ObjCCamera.mm
//  v4l2cam-macos
//
//  Created by Stephen Davis on 2024-12-15.
//
#include <iostream>
#import <AVFoundation/AVFoundation.h>

#import "objccamera.h"

@implementation ObjCCamera
{

    // declare the class varialbes
    AVCaptureDevice * m_camera;
}
// Methods
//
// Open the camera
//
- (bool)open:(NSString *)device_name
{
    // indicate what we are doing
    m_camera = [AVCaptureDevice deviceWithUniqueID:device_name];
    if( m_camera == nil ) {
        std::cout << "Failed to open camera: " << std::string([device_name UTF8String]) << std::endl;
        return false;
    }

    return true;
}


// Close the camera
//
- (bool)close
{
    // close the camera
    m_camera = nil;
    return true;
}


// Init frame capture
//
-(void)initCapture
{ 
    NSError *error=nil;

    //Capture Session
    AVCaptureSession *session = [[AVCaptureSession alloc]init];
    session.sessionPreset = AVCaptureSessionPresetPhoto;

    //Add device
    AVCaptureDevice *inputDevice = m_camera;
    [session addInput:inputDevice];

    // Add an Output
    AVCapturePhotoOutput *output = [[AVCapturePhotoOutput alloc] init];
    AVCapturePhotoSettings *settings = [[AVCapturePhotoSettings alloc] init];
    [settings setFlashMode:AVCaptureFlashModeAuto];
    [settings setHighResolutionPhotoEnabled:YES];
    [settings setAutoStillImageStabilizationEnabled:YES];
    [settings setCaptureFormat:JPEG];
    [output setPreparedPhotoSettingsArray:@[[AVCapturePhotoSettings settings]]];

    [session addOutput:output];
    [session commitConfiguration];

    [session startRunning];

    //Preview Layer - we dont need this rifght now
    //AVCaptureVideoPreviewLayer *previewLayer = [[AVCaptureVideoPreviewLayer alloc] initWithSession:session];
    //previewLayer.frame = viewForCamera.bounds;
    //previewLayer.videoGravity = AVLayerVideoGravityResizeAspectFill;
    //[viewForCamera.layer addSublayer:previewLayer];

}

// Capture a frame
//
- (unsigned char *)captureFrame:(int*)size
{
    unsigned char * ret = nullptr;

    return ret;
}

// set the preferred video format
//
- (bool)setCaptureFormat:(int)format
{
    // set the format
    if( m_camera ) 
    {
        // set the format
        m_camera.activeFormat = [m_camera.formats objectAtIndex:format];
        // set the frame rate
        m_camera.activeVideoMinFrameDuration = CMTimeMake(1, 15);
        m_camera.activeVideoMaxFrameDuration = CMTimeMake(1, 30);

        return true;
    }
    
    return false;
}


// Video Format discovery
//
- (NSArray*)getVideoFormats
{
    NSArray<AVCaptureDeviceFormat *> * formats = nil;

    if( m_camera == nil ) std::cout << "Unable to getVideoFormats as cameras is not open" << std::endl;
    
    // get the supported video formats    
    else formats = [m_camera formats];

    return formats;
}


// Device discovery
//
- (NSArray*)discoverCameras
{
    // instantiate a camera object
    NSMutableArray *deviceTypes = nil;
    
    deviceTypes = [NSMutableArray arrayWithArray:@[AVCaptureDeviceTypeBuiltInWideAngleCamera]];

    [deviceTypes addObject: AVCaptureDeviceTypeDeskViewCamera];
    [deviceTypes addObject: AVCaptureDeviceTypeContinuityCamera];
    [deviceTypes addObject: AVCaptureDeviceTypeExternal];

    AVCaptureDeviceDiscoverySession *captureDeviceDiscoverySession =
        [AVCaptureDeviceDiscoverySession
        discoverySessionWithDeviceTypes:deviceTypes
                              mediaType:AVMediaTypeVideo
                               position:AVCaptureDevicePositionUnspecified];
    
    NSArray * devs = [captureDeviceDiscoverySession devices];
    
    std::cout << "Found " << devs.count << " AVFoundation Cameras" << std::endl;

    return devs;
}

@end
