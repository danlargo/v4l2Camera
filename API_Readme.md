<hr/>

# V4l2Camera class definition

### Overview
- V4l2Camera
    - super class, defines structure of platform specific classes
    - provides [logging](#logging-support) functions for sub-classes

- [LinuxCamera](#linuxcamera-class)
    - Linux variant sub-class
    - uses video4linux2 api

- MacCamera
    - MacOS variant
    - in progress, will use AVFoundation api

- WinCamera
    - Windows variant
    - in progress, will use Media.Capture api



<br/><br/><hr/>

# V4l2Camera
- V4l2Camera class is a super class
    - provides structure for the platform dependent variants of the interface
    - it is not meant to be instantiated.
- Provides logging support for sub-classes


<br/><br/><hr/>

### Logging Support

**Logging Mode**
- logOff : messages are discarded
- logInternal : messages are kept in local buffer for this camera object, but are not sent to the console
- logToStdErr : messages are kept in local buffer and sent to the console via std::cerr
- logToStdOut : messages are kept in local buffer and sent to the console via std::cout

<br/>

**Logging Messasge Type**
- info : displays with a blue [info] tag
- warning : displays with a yellow [warn] tag
- error : displays with an orange [ERR ] tag
- critical : displays with a red [CRIT] tag

<br/>

**Logging Setup**

*Declaration*
```
// Send Log Message
//
void log( std::string msg, enum v4l2cam_msg_type tag = v4l2cam_msg_type::info );

// Set Log Mode
//
void setLogMode( enum v4l2cam_logging_mode );

// Clear Device Log
//
void clearLog();

// Get Last N Log Messages
//
std::vector<std::string>getLogMsgs( int num );

```
*Usage*
```
// Example Usage
//

// set up logging mode on new camera obect
//

my_dev = new V4l2Camera( "/dev/video0" );

my_dev->setLogMode( v4l2cam_logging_mode::logInternal );
my_dev->log( "v4l2Camera object created", v4l2cam_msg_type::info );

if( my_dev->open() )
{
    // do camera stuff

} else {
    // complain
}

```

<br/>

**Get / Set Log Messages**

*Usage*
```
// get last 10 messages, destructive get, oldest 10 messages are removed from the log
//

std::string msgs = my_dev=>getLogMsgs(10);

std::cout << "Retrieved " << msgs.size() << " messages" << std:endl;

// clear all messages
//
my_dev->clearLog();

```

<br/>

**Configuration - Change Size of Device Log**

*In v4l2camera.h*
```
static const int s_logDepth = 500;      // change this to whatever you want, circular buffer, will automatically overwrite old entries

```


<br/><br/><hr/>

# LinuxCamera class
- sub-class of V4l2Camera
- Implements actual interface to video4linux2 api and devices.
- The LinuxCamera class uses the [video4linux2 api](https://www.kernel.org/doc/html/v6.12/userspace-api/media/v4l/v4l2.html) which interacts with the low level drivers provided for UVC Camera support.

<br/><br/><hr/>

### Constructor
*Declaration*
```
LinuxCamera( std::string devName );

```

- Creating this object directly requires a video device path to be provided (example /dev/video0)
- <std::string devName> is in the form of "/dev/videoX", where "X" is a number, usually between 0 and 63. 
- The /dev folder may be in a different location on your operating system
- *Note : It is not required to call this constructor directly, you can call [discoverCameras()](#discover-system-cameras) static method instead which will provide a list of available cameras in yoru system*

*Usage*
```
LinuxCamera * my_dev = new LinuxCamera( "/dev/video0" );

```


<br/><br/><hr/>

### Destructor 
*Declaration*
```
virtual ~LinuxCamera();

```

- Cleans up memory and ensures the device is closed so that other resources may access the camera.
- *Note : if you delete a camera that is in the list provided by [discoverCameras](#discover-system-cameras) be sure to remove the camera from the list as it will remain in the list as an invalid pointer.*

*Usage*
```
delete my_dev;


```


<br/><br/><hr/>

### Discover System Cameras
*Declaration*
```
static std::vector<LinuxCamera *>  discoverCameras();

```

- Call this method before trying to access any of the cameras in the system.
- It will return ALL the accessible cameras in the system
- *Note : it is possible that cameras will be added and removed from the system, if you think that this is going to occur you can call this method again to provide a new list.*
- *Note : V4l2Camera does not implement USB Hotplug callbacks*

*Usage*
```
std::vector<LinuxCamera *> camList = LinuxCamera::DiscoverCameras();

if( camList.size() > 0 )
    // select a specific camera and begin operations

endif

```


<br/><br/><hr/>

### Open Camera
*Declaration*
```
virtual bool open() override;

```

- Camera must be successfully opened before any other camera operation can occur.
- Once camera is open, some (but not necessarily all) of the other camera operations can be performed.
- *Note : A camera can be opened that is already opened by another application. Only one app can fetch images at a time but the controls for a camera can be accessed while another app is streaming images. In this way you can modify the visual parameters of the cameras while a streaming session is in progress.*

*Usage*
```
LinuxCamera * my_cam = camList[0];

if( my_cam->open() )
{
    // you can now use the camera

} else {
    std::cerr << "USB Cameras : << my_cam->getDevName() << "failed to open" << std:endl;
}

```


<br/><br/><hr/>

### Check Camera Is Open
*Declaration*
```
virtual bool isOpen() override;

```

- App should check isOpen() before any camera operation to verify it is actually open.

*Usage*
```
if( !my_dev->isOpen() && !my_dev->open() )
{
    std::cerr << "USB Cameras : << my_cam->getDevName() << "is not open AND/OR failed to open" << std:endl;

} else {
    // do camera stuff

}


```


<br/><br/><hr/>

### Check Camera Is Healthy
*Declaration*
```
virtual bool isHealthy() override;

```

- App should periodically check isHealthy() to know if camera is interacting properly with system.
- Low rent way to determine if perhaps camera has been unplugged from system.
- VERY implementation specific, current LinuxCamera class simply counts number of failed calls and reported not healthy after 10 failure.

*Usage*
```
if( !my_dev->isHealthy() )
{
    std::cerr << "USB Cameras : << my_cam->getDevName() << "is has been removed or other failure" << std:endl;

} else {
    // do normal camera stuff

}


```

<br/><br/><hr/>

### Close Camera
*Declararion*
```
virtual void close() override;

```

- Once a camera is closed no other operations may be performed.
- It is not necessary to delete a camera object after close. The camera object can be opened and closed as many times as required.


*Usage*
```
my_cam->close();


```

<br/><br/><hr/>

### Discover Camera Information
*Declaration*
```
// Discover Camera name and image read modes
//
virtual bool enumCapabilities() override;

// Discover list of user controls, ranges and step sizes
//
virtual bool enumControls() override;

// Discover list of video modes supported, image compression and size
//
virtual bool enumVideoModes() override;

```

- Queries the camera for it's capabilities, including Vendor supplied name for the camera, list of supported controls, and all the supported video modes.
- These calls only need to be done once after the initilization of the camera object.
- Camera must be open for these functions to work.
- Added as separate operation as can be expensive in terms of processing, can be moved into construction if confident that operations can be completed quickly.

*Usage*
```
    // these should/can all be done at the same time
    //
    if( my_dev->open() )
    {
        my_dev->enumCapabilities();
        my_dev->ennumControls();
        my_dev->enumVideoModes();

        // do more camera stuff

    }
```


<br/><br/><hr/>

### Query Camera Information
*Declaration*
```
// Get the vendor supplied name for the camera
//
virtual std::string getUserName() override;

// Get Descriptive Name for Camera Type
//
std::string getCameraType();

// Get the device specific name for the camera
// - usually /dev/videoX
//
virtual std::string getDevName() override;

// Determine if camera is able to fetch images
//
virtual bool canFetch() override;

```

- access information previously queried by the camera information enum methods

*Usage*
```

if( !my_dev->isOpen() && !my_dev->open() )
{
    std::cerr << "USB Cameras : << my_cam->getUserName() << "is not open AND/OR failed to open" << std:endl;

} else {
    // check if we can actually fetch images
    if( my_dev->canFetch() )
    {
        // do camera stuff

    } else {
        std::cerr << "USB Cameras : << my_cam->getUserName() << " : unable to fetch images from this camera" << std:endl;
    }
}

```


<br/><br/><hr/>

### Interact With Camera Controls
*Declaration*
```
// Control Structure
//
struct v4l2cam_control
{
    std::string name;               // name provided by API for control
    int type;                       // type of control (INTEGER, BOOLEAN, MAP)
    int id;                         // ID of control as provided by API (this is the MAP key)
    std::string typeStr;            // type of control converted to string for UI display as required
    int min;                        // Min value for control
    int max;                        // Max value for control
    std::map<int, std::string> menuItems;       // if item is a menu then list is provided, map key is ID to use when setting or getting value
    int step;                       // step size of control, vakue passed to set between values will be assigned up or down by API
    int value;                      // current value of control
};

// Retrieve list of controls enumerated by enumControls
// - NOTE : value is only current as of time of the enumControls call
// - NOTE : value is NOT updated by calls to getvalue()
//
std::map<int, struct v4l2cam_control> getControls() { return this->m_controls; };

// Retrieves Control structure for control identified by ID, corresponds to map key in cotrol list
//
struct v4l2cam_control getOneCntrl( int index );

// Get current value for control
// - returns -1 on failure
// - openOnDemand allows this call to function regardless of whether current LinuxCamera object is open or closed
//
virtual int getValue( int id, bool openOnDemand = false ) override;

// Set current value for control
// - returns current value of control after set (should be passed value), will return -1 on failure
// - openOnDemand allows this call to function regardless of whether current LinuxCamera object is open or closed
//
virtual int setValue( int id, int val, bool openOnDemand = false ) override;

```

- this call can be made without the camera device being open (use openOnDemand to override current isOpen state)
- this call can (should be able to) be made when another app has the camera open, allowing an app to manipulate camera settings even during a video streaming session.

*Usage*
```
// Objective, set exposure control to 40%
//
.
.
.
int bright_id = 9963776;    // defined by video4linux2 api

struct v4l2cam_control s_cntrl = my_dev->getOneCtrl( bright_id );

// calculate 40% of range of control
//
int new_value = s_cntrl.min + ((float)(s_cntrl.max - s_cntrl.min) * .4);

if( my_dev->setValue( bright_id, new_value, true ) == new_value )
{
    std::cout << "Set brightness of : << my_cam->getUserName() << " to " << new_value << std:endl;

} else {
    // call failed, tell user
    std::cerr << "Unable to set brightness of : << my_cam->getUserName() << std:endl;
}

```


<br/><br/><hr/>

### Query Video Modes Supported by Camera
std::vector<struct v4l2cam_video_mode> getVideoModes() { return this->m_modes; };
struct v4l2cam_video_mode getOneVM( int index );


<br/><br/><hr/>

### Grab An Image from the Camera
virtual bool init( enum v4l2cam_fetch_mode ) override;
virtual bool setFrameFormat( struct v4l2cam_video_mode ) override;
bool setFrameFormat( std::string mode, int width, int height );

virtual struct v4l2cam_image_buffer * fetch( bool lastOne ) override;


