//
//  ObjCCamera.h
//  v4l2cam-macos
//
//  Created by Stephen Davis on 2024-12-15.
//
#import <Foundation/Foundation.h>

@interface ObjCCamera : NSObject

- (bool) open:(NSString*)device_name;
- (bool) close;
- (NSArray*) discoverCameras;
- (NSArray*) getVideoFormats;
- (bool) setCaptureFormat:(int)format;


@end
