# V4l2Camera Class definitions

## V4l2Camera

- V4l2Camera class is a super class, providing structure for the platform dependent variants of the interface. It is not meant to be instantiated.
- Provides logging support for the LInux, Mac and Windows versions of the classes.

### Set Log Mode For Messages

**Definition**
```
//
// Log Mode
//
// logOff : messages are discarded
// logInternal : messages are kept in a local buffer for this camera object, but are not sent to the console
// logToStdErr : messages are kep in local buffer and sent to the console std::cerr
// logToStdOut : messages are kept in local buffer and sent to the console via std::cout
//
enum v4l2cam_logging_mode
{
    logOff, logInternal, logToStdErr, logToStdOut
};

//
// Log Message Type
//
// info : displays with a blue [info] tag
// warning : displays with a yellow [warn] tag
// error : displays with an orange [ERR ] tag
// critical : displays with a red [CRIT] tag
//
enum v4l2cam_msg_type
{
    info, warning, error, critical
};

void setLogMode( enum v4l2cam_logging_mode );

```

**Usage**
```
my_dev = new V4l2Camera( "/dev/video0" );

my_dev->setLogMode( v4l2cam_logging_mode::logInternal );
my_dev->log( "v4l2Camera object created", v4l2cam_msg_type::info );

if( my_dev->open() )
{

} else {

}

```

### Log Message
    void log( std::string msg, enum v4l2cam_msg_type tag = v4l2cam_msg_type::info );
    void clearLog();
    std::vector<std::string>getLogMsgs( int num );




## LinuxCamera

### Constructor LinuxCamera( std::string devName);

**Usage**
```
LinuxCamera * my_dev = new LinuxCamera( "/dev/video0" );

```

- Creating this object directly requires a video device path to be provided. The LinuxCamera class uses the [video4linux2 api](https://www.kernel.org/doc/html/v6.12/userspace-api/media/v4l/v4l2.html) which interacts with the low level drivers provided for UVC Camera support.

- <std::string devName> is in the form of "/dev/videoX", where "X" is a number, usually between 0 and 63. The /dev folder may be in a different location on your operating system

- *Note : It is not required to call this constructor directly, you can call [discoverCameras](#discover-list-of-system-cameras--static-stdvectorlinuxcamera---discovercameras) static method instead which will provide a list of available cameras in yoru system*


### Destructor virtual ~LinuxCamera();

**Usage**
```
delete my_dev;

```

- Cleans up memory and ensures the device is closed so that other resources may access the camera.
- *Note : if you delete a camera that is in the list provided by [DiscoverCameras](#discover-list-of-system-cameras--static-stdvectorlinuxcamera---discovercameras) be sure to remove the camera from the list as it will remain in the list as an invalid pointer.*


## Discover List of System Cameras : static std::vector<LinuxCamera *>  discoverCameras();

**Usage**
```
std::vector<LinuxCamera *> camList = LinuxCamera::DiscoverCameras();

if( camList.size() > 0 )
    // select a specific camera and begin operations

endif

```

- Call this method at before trying to access any of the cameras in the system.
- It will return ALL the accessible cameras in the system
- *Note : it is possible that cameras will be added and removed from the system, if you think that this is going to occur you can call this method again to provide a new list.*
- *Note : V4l2Camera does not implement USB Hotplug callbacks*


## Open Camera for use : virtual bool open() override;

**Usage**
```
LinuxCamera * my_cam = camList[0];

if( my_cam->open() )
{
    // you can now use the camera

} else {
    std::cerr << "USB Cameras : << my_cam->getDevName() << "failed to open" << std:endl;
}

```

- Camera must be successfully opened before any other camera operation can occur.
- Once camera is open some (but not necessarily all) of the other camera operations can be performed.
- *Note : A camera can be opened that is already opened by another application. Only one app can fetch images at a time but the controls for a camera can be access while another app is streaming images. In this way you can modify the visual parameters of the cameras while a streaming session is in progress.*


## Check If Camera is open : virtual bool isOpen() override;

**Usage**
```
if( !my_dev->isOpen()  && !my_dev->open() )
{
    std::cerr << "USB Cameras : << my_cam->getDevName() << "is not open AND/OR failed to open" << std:endl;

} else {
    // do camera stuff

}


```

- App should check isOpen before any camera operation to verify it is actually open.


## Close Camera : virtual void close() override;

**Usage**
```
my_cam->close();

```

- Once a camera is closed no other operations may be performed.
- It is not necessary to delete a camera object after close. The camera object can be opened and closed as many times as required.


## Discover Camera Information

### Capabilities : virtual bool enumCapabilities() override;
### User Control List : virtual bool enumControls() override;
### Supported Video Modes : virtual bool enumVideoModes() override;

**Usage**
```
    // these should all be done at the same time
    if( my_dev->open() )
    {
        my_dev->enumCapabilities();
        my_dev->ennumControls();
        my_dev->enumVideoModes();

        // do more camera stuff

    }
```

- Queries the camera for it's capabilities, including Vendor supplied name for the camera, list of supported controls, and all the supported video modes.
- These calls only need to be done once after the initilization of the camera object.
- Camera must be open for these functions to work.


## Query Camera Information

    virtual std::string getUserName() override;
    virtual std::string getDevName() override;
    virtual bool canFetch() override;
    virtual bool canRead() override;


## Interact With Camera Controls

    virtual std::string cntrlTypeToString(int type) override;
    virtual int setValue( int id, int val, bool openOnDemand = false ) override;
    virtual int getValue( int id, bool openOnDemand = false ) override;

    

## Grab An Image from the Camera
    virtual bool setFrameFormat( struct v4l2cam_video_mode ) override;
    
    virtual bool init( enum v4l2cam_fetch_mode ) override;

    virtual struct v4l2cam_image_buffer * fetch( bool lastOne ) override;
