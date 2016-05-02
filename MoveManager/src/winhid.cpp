#include "../include/MovePrecompiled.h"

#ifdef WIN32
// Playstation Move on the PC
// Windows platform specific code for HID and USB
// by Carl Kenner 4 October 2010 - 16 November 2010
// this uses normal windows HID, MotionInJoy, and also libusb-win32 if available
/*
   This file (and anything derived from it, based on it, or using it) is released under a 
   non-military license, it may not be used for any military purpose, including military 
   research, or military funded research, training, recruitment, troop entertainment, 
   design of military systems, or anything else involving the military. I didn't write this
   to help kill people, sorry.
*/


#ifdef _MSC_VER 
#pragma comment(lib, "setupapi.lib")	
#endif
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <setupapi.h>

#include "../include/moveonpc.h"

// only pointers to this type are used, it doesn't matter what type it really is
typedef struct {} usb_dev_handle;

HANDLE MoveHandles[MAXMOVES] = {0};
HANDLE NavHandles[MAXMOVES] = {0};
HANDLE SixaxisHandles[MAXMOVES] = {0};
HANDLE MotionInJoyHandles[MAXMOVES] = {0};
HANDLE MoveHidHandles[MAXMOVES] = {0};
HANDLE NavHidHandles[MAXMOVES] = {0};
HANDLE SixaxisHidHandles[MAXMOVES] = {0};
HANDLE GametrakHidHandles[MAXMOVES] = {0};
usb_dev_handle *MoveUsbDevs[MAXMOVES] = {NULL};
usb_dev_handle *NavUsbDevs[MAXMOVES] = {NULL};
usb_dev_handle *SixaxisUsbDevs[MAXMOVES] = {NULL};
usb_dev_handle *GametrakUsbDevs[MAXMOVES] = {NULL};
int MoveUsbItfs[MAXMOVES] = {0};
int NavUsbItfs[MAXMOVES] = {0};
int SixaxisUsbItfs[MAXMOVES] = {0};
int GametrakUsbItfs[MAXMOVES] = {0};

int MoveCount = 0, NavCount = 0, SixaxisCount = 0, GametrakCount = 0, MotionInJoyCount = 0;

HMODULE	hid_dll = NULL, libusb_dll = NULL;
typedef struct _HIDD_ATTRIBUTES {
  ULONG  Size;
  USHORT VendorID;
  USHORT ProductID;
  USHORT VersionNumber;
} HIDD_ATTRIBUTES, *PHIDD_ATTRIBUTES;
typedef BOOLEAN (__stdcall *HidReportFunction)(HANDLE HidDeviceObject, PVOID  ReportBuffer, ULONG  ReportBufferLength);
typedef void (__stdcall *HidGetGuidFunction)(LPGUID HidGUID);
typedef BOOLEAN (__stdcall *HidAttribFunction)(HANDLE HidDeviceObject, PHIDD_ATTRIBUTES Attributes);
HidReportFunction HidD_SetOutputReport = NULL;
HidReportFunction HidD_GetInputReport = NULL;
HidReportFunction HidD_SetFeature = NULL;
HidReportFunction HidD_GetFeature = NULL;
HidAttribFunction HidD_GetAttributes = NULL;
HidGetGuidFunction HidD_GetHidGuid = NULL;

// libusb-win32 defines
#define USB_DIR_IN           0x80
#define USB_DIR_OUT          0
#define USB_TYPE_CLASS      (1 << 5)
#define USB_RECIP_INTERFACE	 1
#define LIBUSB_PATH_MAX      512
#ifdef interface
#undef interface
#endif
#include <pshpack1.h>

struct usb_endpoint_descriptor {
  unsigned char  bLength;
  unsigned char  bDescriptorType;
  unsigned char  bEndpointAddress;
  unsigned char  bmAttributes;
  unsigned short wMaxPacketSize;
  unsigned char  bInterval;
  unsigned char  bRefresh;
  unsigned char  bSynchAddress;
  unsigned char *extra;
  int extralen;
};
struct usb_interface_descriptor {
  unsigned char  bLength;
  unsigned char  bDescriptorType;
  unsigned char  bInterfaceNumber;
  unsigned char  bAlternateSetting;
  unsigned char  bNumEndpoints;
  unsigned char  bInterfaceClass;
  unsigned char  bInterfaceSubClass;
  unsigned char  bInterfaceProtocol;
  unsigned char  iInterface;
  struct usb_endpoint_descriptor *endpoint;
  unsigned char *extra;
  int extralen;
};
struct usb_interface {
  struct usb_interface_descriptor *altsetting;
  int num_altsetting;
};
struct usb_config_descriptor {
  unsigned char  bLength;
  unsigned char  bDescriptorType;
  unsigned short wTotalLength;
  unsigned char  bNumInterfaces;
  unsigned char  bConfigurationValue;
  unsigned char  iConfiguration;
  unsigned char  bmAttributes;
  unsigned char  MaxPower;
  struct usb_interface *interface;
  unsigned char *extra;
  int extralen;
};
struct usb_device_descriptor {
  unsigned char  bLength;
  unsigned char  bDescriptorType;
  unsigned short bcdUSB;
  unsigned char  bDeviceClass;
  unsigned char  bDeviceSubClass;
  unsigned char  bDeviceProtocol;
  unsigned char  bMaxPacketSize0;
  unsigned short idVendor;
  unsigned short idProduct;
  unsigned short bcdDevice;
  unsigned char  iManufacturer;
  unsigned char  iProduct;
  unsigned char  iSerialNumber;
  unsigned char  bNumConfigurations;
};
struct usb_device {
  struct usb_device *next, *prev;
  char filename[LIBUSB_PATH_MAX];
  struct usb_bus *bus;
  struct usb_device_descriptor descriptor;
  struct usb_config_descriptor *config;
  void *dev;
  unsigned char devnum;
  unsigned char num_children;
  struct usb_device **children;
};
struct usb_bus {
  struct usb_bus *next, *prev;
  char dirname[LIBUSB_PATH_MAX];
  struct usb_device *devices;
  unsigned long location;
  struct usb_device *root_dev;
};
#include <poppack.h>

typedef void (*usb_init_t)(void);
typedef usb_dev_handle * (*usb_open_t)(struct usb_device *dev);
typedef int (*usb_close_t)(usb_dev_handle *dev);
typedef int (*usb_control_msg_t)(usb_dev_handle *dev, int requesttype,
                                 int request, int value, int index,
                                 unsigned char *bytes, int size, int timeout);
typedef struct usb_bus * (*usb_get_busses_t)(void);
typedef int (*UsbReadFunction)(usb_dev_handle *dev, int ep, char *bytes,
                                    int size, int timeout);
typedef int (*UsbIntFunction)(usb_dev_handle *dev, int i);
typedef int (*UsbVoidFunction)(void);

usb_init_t usb_init=NULL;
usb_open_t usb_open=NULL;
usb_close_t usb_close=NULL;
usb_control_msg_t usb_control_msg=NULL;
usb_get_busses_t usb_get_busses=NULL;
UsbIntFunction usb_set_configuration=NULL, usb_claim_interface=NULL, usb_release_interface=NULL;
UsbReadFunction usb_interrupt_read=NULL, usb_interrupt_write=NULL, usb_bulk_read=NULL, usb_bulk_write=NULL;
UsbVoidFunction usb_find_busses=NULL, usb_find_devices=NULL;

//typedef void (*usb_set_debug_t)(int level);
//typedef struct usb_device * (*usb_device_t)(usb_dev_handle *dev);
//typedef int (*usb_get_string_t)(usb_dev_handle *dev, int index, int langid,
//                                char *buf, size_t buflen);
//typedef int (*usb_get_string_simple_t)(usb_dev_handle *dev, int index,
//                                       char *buf, size_t buflen);
//typedef int (*usb_get_descriptor_by_endpoint_t)(usb_dev_handle *udev, int ep,
//                                                unsigned char type,
//                                                unsigned char index,
//                                                void *buf, int size);
//typedef int (*usb_get_descriptor_t)(usb_dev_handle *udev, unsigned char type,
//                                    unsigned char index, void *buf, int size);
//typedef int (*usb_set_altinterface_t)(usb_dev_handle *dev, int alternate);
//typedef int (*usb_resetep_t)(usb_dev_handle *dev, unsigned int ep);
//typedef int (*usb_clear_halt_t)(usb_dev_handle *dev, unsigned int ep);
//typedef int (*usb_reset_t)(usb_dev_handle *dev);
//typedef char * (*usb_strerror_t)(void);
//typedef void (*usb_init_t)(void);
//typedef void (*usb_set_debug_t)(int level);
//typedef int (*usb_find_busses_t)(void);
//typedef int (*usb_find_devices_t)(void);
//typedef struct usb_device * (*usb_device_t)(usb_dev_handle *dev);
//typedef struct usb_bus * (*usb_get_busses_t)(void);
//typedef int (*usb_install_service_np_t)(void);
//typedef int (*usb_uninstall_service_np_t)(void);
//typedef int (*usb_install_driver_np_t)(const char *inf_file);
//typedef const struct usb_version * (*usb_get_version_t)(void);
//typedef int (*usb_isochronous_setup_async_t)(usb_dev_handle *dev,
//                                             void **context,
//                                             unsigned char ep, int pktsize);
//typedef int (*usb_bulk_setup_async_t)(usb_dev_handle *dev, void **context,
//                                      unsigned char ep);
//typedef int (*usb_interrupt_setup_async_t)(usb_dev_handle *dev, void **context,
//                                           unsigned char ep);
//typedef int (*usb_submit_async_t)(void *context, char *bytes, int size);
//typedef int (*usb_reap_async_t)(void *context, int timeout);
//typedef int (*usb_free_async_t)(void **context);

int load_hid_dll() {
	if (hid_dll) return TRUE;
	hid_dll = LoadLibrary("hid.dll");
	if (!hid_dll) return FALSE;
	HidD_SetOutputReport = (HidReportFunction)GetProcAddress(hid_dll, "HidD_SetOutputReport");
	HidD_GetInputReport = (HidReportFunction)GetProcAddress(hid_dll, "HidD_SetOutputReport");
	HidD_SetFeature = (HidReportFunction)GetProcAddress(hid_dll, "HidD_SetFeature");
	HidD_GetFeature = (HidReportFunction)GetProcAddress(hid_dll, "HidD_GetFeature");
	HidD_GetHidGuid = (HidGetGuidFunction)GetProcAddress(hid_dll, "HidD_GetHidGuid");
	HidD_GetAttributes = (HidAttribFunction)GetProcAddress(hid_dll, "HidD_GetAttributes");
	return TRUE;
}

int unload_hid_dll() {
	if (!hid_dll) return TRUE;
	HidD_SetOutputReport = NULL;
	HidD_GetInputReport = NULL;
	HidD_SetFeature = NULL;
	HidD_GetFeature = NULL;
	HidD_GetHidGuid = NULL;
	HidD_GetAttributes = NULL;
	FreeLibrary(hid_dll);
	hid_dll = NULL;
	return TRUE;
}

int load_libusb_dll() {
	if (libusb_dll) return TRUE;
	libusb_dll = LoadLibrary("libusb0.dll");
	if (!libusb_dll) return FALSE;
  usb_init = (usb_init_t)GetProcAddress(libusb_dll, "usb_init");
  usb_open = (usb_open_t)GetProcAddress(libusb_dll, "usb_open");
  usb_close = (usb_close_t)GetProcAddress(libusb_dll, "usb_close");
  usb_get_busses = (usb_get_busses_t)GetProcAddress(libusb_dll, "usb_get_busses");
  usb_control_msg = (usb_control_msg_t)GetProcAddress(libusb_dll, "usb_control_msg");
  usb_find_busses = (UsbVoidFunction)GetProcAddress(libusb_dll, "usb_find_busses");
  usb_find_devices = (UsbVoidFunction)GetProcAddress(libusb_dll, "usb_find_devices");
  usb_claim_interface = (UsbIntFunction)GetProcAddress(libusb_dll, "usb_claim_interface");
  usb_release_interface = (UsbIntFunction)GetProcAddress(libusb_dll, "usb_release_interface");
  usb_set_configuration = (UsbIntFunction)GetProcAddress(libusb_dll, "usb_set_configuration");
  usb_bulk_read = (UsbReadFunction)GetProcAddress(libusb_dll, "usb_bulk_read");
  usb_bulk_write = (UsbReadFunction)GetProcAddress(libusb_dll, "usb_bulk_write");
  usb_interrupt_read = (UsbReadFunction)GetProcAddress(libusb_dll, "usb_interrupt_read");
  usb_interrupt_write = (UsbReadFunction)GetProcAddress(libusb_dll, "usb_interrupt_write");
  if (usb_init) usb_init();
  else return FALSE;
	return TRUE;
}

int unload_libusb_dll() {
	if (!libusb_dll) return TRUE;
  usb_init = NULL;
  usb_open = NULL;
  usb_close = NULL;
  usb_get_busses = NULL;
  usb_control_msg = NULL;
  usb_find_busses = NULL;
  usb_find_devices = NULL;
  usb_claim_interface = NULL;
  usb_release_interface = NULL;
  usb_set_configuration = NULL;
  usb_bulk_read = NULL;
  usb_bulk_write = NULL;
  usb_interrupt_read = NULL;
  usb_interrupt_write = NULL;
	FreeLibrary(libusb_dll);
	libusb_dll = NULL;
	return TRUE;
}

int load_dll() {
  int res = load_hid_dll();
  return load_libusb_dll() || res;
}

int unload_dll() {
  int res = unload_hid_dll();
  return unload_libusb_dll() || res;
}

int ReadMoveReport(int index, void *report) {
	BYTE temp[52];
	if (!report) return FALSE;
	ZeroMemory(report, 49);
	if (index<0 || index>=MAXMOVES) return FALSE;
  // try MotionInJoy via Windows HID
  if (MoveHandles[index]!=INVALID_HANDLE_VALUE) {
	  if (!HidD_GetFeature) return FALSE;
	  ZeroMemory(temp, 52);
	  if (HidD_GetFeature(MoveHandles[index], temp, 52))
		  memcpy(report,&temp[1],49);
  // try libusb-win32
  } else if (MoveUsbDevs[index]) {
	  if (usb_control_msg(MoveUsbDevs[index], USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
        1, 0x100 | 0x01, MoveUsbItfs[index], temp, 49, 100)>=0)
		  memcpy(report,temp,49);
  } else return FALSE;
	return TRUE;
}

int WriteMoveReport(int index, void *report) {
	BYTE temp[52];
	if (!report) return FALSE;
	if (index<0 || index>=MAXMOVES) return FALSE;
	ZeroMemory(temp, 52);
  // try MotionInJoy via Windows HID
  if (MoveHandles[index]!=INVALID_HANDLE_VALUE) {
	  if (!HidD_SetFeature) return FALSE;
	  memcpy(&temp[3],report,49);
	  if (!HidD_SetFeature(MoveHandles[index], temp, 52)) return FALSE;
  // try libusb-win32
  } else if (MoveUsbDevs[index]) {
    memcpy(temp, report, 49);
    return usb_control_msg(MoveUsbDevs[index], USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
    9, 0x200 | temp[0], MoveUsbItfs[index], temp, 49, 1000)>=0;
  } else return FALSE;
	return TRUE;
}

int ReadMoveFeature(int index, int feature, void *report) {
	BYTE temp[52];
	if (!report) return FALSE;
	ZeroMemory(report, 49);
	if (index<0 || index>=MAXMOVES) return FALSE;
	ZeroMemory(temp, 52);
  temp[0]=feature;
  // try Windows HID directly
  if (MoveHidHandles[index]!=INVALID_HANDLE_VALUE) {
	  if (!HidD_GetFeature) return FALSE;
	  if (HidD_GetFeature(MoveHidHandles[index], temp, 49))
		  memcpy(report,temp,49);
  // try MotionInJoy via Windows HID (not really sure how to do this, this probably doesn't work)
  } else if (MoveHandles[index]!=INVALID_HANDLE_VALUE) {
	  if (!HidD_GetFeature) return FALSE;
    return FALSE; // currently impossible
  // try libusb-win32
  } else if (MoveUsbDevs[index]) {
	  if (usb_control_msg(MoveUsbDevs[index], USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
        1, 0x300 | feature, MoveUsbItfs[index], temp, 49, 100)>=0)
		  memcpy(report,temp,49);
  } else return FALSE;
	return TRUE;
}

int ReadNavReport(int index, void *report) {
	BYTE temp[52];
	if (!report) return FALSE;
	ZeroMemory(report, 49);
	if (index<0 || index>=MAXMOVES) return FALSE;
  // try MotionInJoy via Windows HID
  if (NavHandles[index]!=INVALID_HANDLE_VALUE) {
	  if (!HidD_GetFeature) return FALSE;
	  ZeroMemory(temp, 52);
	  if (HidD_GetFeature(NavHandles[index], temp, 52))
		  memcpy(report,&temp[1],49);
  // try libusb-win32
  } else if (NavUsbDevs[index]) {
	  if (usb_control_msg(NavUsbDevs[index], USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
        1, 0x100 | 0x01, NavUsbItfs[index], temp, 49, 100)>=0)
		  memcpy(report,temp,49);
  } else return FALSE;
	return TRUE;
}

int WriteNavReport(int index, void *report) {
  return FALSE;
}

int ReadNavFeature(int index, int feature, void *report) {
	BYTE temp[52];
	if (!report) return FALSE;
	ZeroMemory(report, 49);
	if (index<0 || index>=MAXMOVES) return FALSE;
	ZeroMemory(temp, 52);
  temp[0]=feature;
  // try Windows HID directly
  if (NavHidHandles[index]!=INVALID_HANDLE_VALUE) {
	  if (!HidD_GetFeature) return FALSE;
	  if (HidD_GetFeature(NavHidHandles[index], temp, 49))
		  memcpy(report,temp,49);
  // try libusb-win32
  } else if (NavUsbDevs[index]) {
	  if (usb_control_msg(NavUsbDevs[index], USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
        1, 0x300 | feature, NavUsbItfs[index], temp, 49, 100)>=0)
		  memcpy(report,temp,49);
  } else return FALSE;
	return TRUE;
}

int ReadSixaxisReport(int index, void *report) {
	BYTE temp[52];
	if (!report) return FALSE;
	ZeroMemory(report, 49);
	if (index<0 || index>=MAXMOVES) return FALSE;
  // try MotionInJoy via Windows HID
  if (SixaxisHandles[index]!=INVALID_HANDLE_VALUE) {
	  if (!HidD_GetFeature) return FALSE; // this currently happens if its a USB device
	  ZeroMemory(temp, 52);
	  if (HidD_GetFeature(SixaxisHandles[index], temp, 52))
		  memcpy(report,&temp[1],49);
  // try libusb-win32
  } else if (NavUsbDevs[index]) {
	  if (usb_control_msg(SixaxisUsbDevs[index], USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
      1, 0x100 | 0x01, SixaxisUsbItfs[index], temp, 49, 100)>=0)
		  memcpy(report,temp,49);
  } else return FALSE;
	return TRUE;
}

int WriteSixaxisReport(int index, void *report) {
	BYTE temp[52];
	if (!report) return FALSE;
	if (index<0 || index>=MAXMOVES) return FALSE;
	ZeroMemory(temp, 52);
  // try MotionInJoy via Windows HID
  if (SixaxisHandles[index]!=INVALID_HANDLE_VALUE) {
	  if (!HidD_SetFeature) return FALSE;
	  memcpy(&temp[3],report,49);
	  if (!HidD_SetFeature(SixaxisHandles[index], temp, 52)) return FALSE;
  // try libusb-win32
  } else if (SixaxisUsbDevs[index]) {
    memcpy(temp,report,49);
    return usb_control_msg(SixaxisUsbDevs[index], USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
    9, 0x200 | temp[0], SixaxisUsbItfs[index], temp, 49, 1000)>=0;
  } else return FALSE;
	return TRUE;
}

int ReadSixaxisFeature(int index, int feature, void *report) {
	BYTE temp[52];
	if (!report) return FALSE;
	ZeroMemory(report, 49);
	if (index<0 || index>=MAXMOVES) return FALSE;
	ZeroMemory(temp, 52);
  temp[0]=feature;
  // try Windows HID directly
  if (SixaxisHidHandles[index]!=INVALID_HANDLE_VALUE) {
	  if (!HidD_GetFeature) return FALSE;
	  if (HidD_GetFeature(SixaxisHidHandles[index], temp, 49))
		  memcpy(report,temp,49);
  // try libusb-win32
  } else if (SixaxisUsbDevs[index]) {
	  if (usb_control_msg(SixaxisUsbDevs[index], USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
        1, 0x300 | feature, SixaxisUsbItfs[index], temp, 49, 100)>=0)
		  memcpy(report,temp,49);
  } else return FALSE;
	return TRUE;
}

// debugging stuff
static int ReadHidFeatureReport(int num, HANDLE handle) {
  unsigned char report[256];
  report[0]=num;
  for (int j=0; j<=255; j++) {
    if (HidD_GetFeature && HidD_GetFeature(handle, report, j)) {
      printf("Feature Report %02X: ", num);
      for (int i=0; i<j; i++) printf("%02X ",report[i]);
      printf("\n");
      return TRUE;
    }
  }
  return FALSE;
}

// debugging stuff
static int ReadHidInputReport(int num, HANDLE handle) {
  unsigned char report[256];
  report[0]=num;
  for (int j=0; j<=255; j++) {
    if (HidD_GetInputReport && HidD_GetInputReport(handle, report, j)) {
      printf("Input Report %02X: ", num);
      for (int i=0; i<j; i++) printf("%02X ",report[i]);
      printf("\n");
      return TRUE;
    }
  }
  return FALSE;
}

// 04:76:6e:bf:f3:6a
// Feature Report 04 (Bluetooth MAC): 04 (6A F3 BF 6E 76 04) 08 25 00 (2A 02 00 86 19 00) 
// Feature Report 10: 10 01 07 46 7F 1B 80 A9 80 00 00 00 00 00 00 00 00 00 01 FC 08 E0 01 04 [95 2E] [80 71] [80 E0] 01 65 [7F D5] [90 9F] [80 E0] 01 6C [7F 04] [80 D8] [94 D4] 07 BC 00
// Feature Report 11: 11 02 00 01 28 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
// Feature Report E0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
// Feature Report F1: 56 0B 30 01 10 BF 0A 07 08 0E 1A 00 00 00
//                                      0A 07 0B 08 36 00 00 5A
// Feature Report F7: EF 7B 0A BD 07 00 03 00 00 00 00
                 //or EF 7A 0A 5C 08 00 03 00 00 00 00
                 //or EF 7A 0A 5D 08 00 03 00 00 00 00
// Feature Report F8: 00 00 00 00 00 00 00 00
// Feature Report F9: 01 50 06 99 00 82 00 00 00 00 00 00 00
// Feature Report FB: 00 00 04 0C 00 0C
// debugging stuff
static int AllZeroes(unsigned char *r, int length) {
  for (;length>0;length--,r++) if (*r) return FALSE;
  return TRUE;
}

static int ReadUsbFeatureReport(int num, usb_dev_handle *devh, int itfnum) {
  unsigned char report[256];
  report[0]=num;
  if (usb_control_msg(devh, USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
    1, 0x300 | num, itfnum, report, 255, 5000)>0 && !AllZeroes(report, 49)) {
    printf("Feature Report %02X: ", num);
    for (int i=0; i<49; i++) printf("%02X ",report[i]);
    printf("\n");
    return TRUE;
  } else return FALSE;
}

// debugging stuff
static int ReadUsbInputReport(int num, usb_dev_handle *devh, int itfnum) {
  unsigned char report[256];
  report[0]=num;
  if (usb_control_msg(devh, USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
      1, 0x100 | num, itfnum, report, 49, 100)>0 && !AllZeroes(report, 49)) {
    printf("Input Report %02X: ", num);
    for (int i=0; i<49; i++) printf("%02X ",report[i]);
    printf("\n");
    return TRUE;
  } else return FALSE;
}

// debugging stuff, note that what I'm trying to do here makes no sense,
// since I'm READING an output report, however it might be dangerous.
static int ReadUsbOutputReport(int num, usb_dev_handle *devh, int itfnum) {
  unsigned char report[256];
  report[0]=num;
  if (usb_control_msg(devh, USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
      1, 0x200 | num, itfnum, report, 49, 100)>0 && !AllZeroes(report, 49)) {
    printf("Output Report %02X: ", num);
    for (int i=0; i<49; i++) printf("%02X ",report[i]);
    printf("\n");
    return TRUE;
  } else return FALSE;
}

// debugging stuff
static int TestUsbDevice(usb_dev_handle *devh, int itfnum) {
  for (int r=0; r<=255; r++) { ReadUsbFeatureReport(r,devh,itfnum); ReadUsbFeatureReport(r,devh,itfnum); }
  for (int r=0; r<=255; r++) ReadUsbInputReport(r,devh,itfnum);
  for (int r=0; r<=255; r++) ReadUsbOutputReport(r,devh,itfnum);
  return TRUE;
}

// debugging stuff
static int TestHidDevice(HANDLE handle) {
  printf("\n");
  for (int r=0; r<=255; r++) ReadHidFeatureReport(r,handle);
  for (int r=2; r<=2; r++) ReadHidInputReport(r,handle);
  ReadHidInputReport(0,handle);
  return TRUE;
}

// Open MotionInJoy Virtual Device, via windows HID. It could be a Move, or a SIXAXIS.
// A Move will return valid Move input reports when queried for Feature report 0.
// A SIXAXIS will return all zeroes (or perhaps fail in a previous version?)
static int OpenMotionInJoy(HANDLE handle) {
  unsigned char report[52];
  if (!HidD_GetFeature) return FALSE;
  ZeroMemory(report, sizeof(report));
  // it's some kind of controller connected via bluetooth with MotionInJoy
  if (HidD_GetFeature(handle, report, 52) && !AllZeroes(report, 49)) {
    //printf(" Input Report %02X: ", report[1]);
    //for (int i=0; i<49; i++) printf("%02X ",report[i+2]);
    //printf("\n");
    // We need to receive input report 1
    if (report[0]!=0 || report[1]!=1) return FALSE;
    unsigned char *r = &report[1];
    // For the move, I'm guessing this is the analog sticks which don't exist,
    // and thus are always 7F 7F 7F 7F
    if (r[7]==0x7F && r[8]==0x7F && r[9]==0x7F && r[10]==0x7F) {
      // it's a Move
      printf(" (Move)");
      if (MoveCount >= MAXMOVES) return FALSE;
      MoveHandles[MoveCount++] = handle;
      return TRUE;
    } else {
      // it's a SIXAXIS connected via bluetooth
      printf(" (SIXAXIS)");
      if (SixaxisCount >= MAXMOVES) return FALSE;
      SixaxisHandles[SixaxisCount++] = handle;
      return TRUE;
    }
  } else {
    // it's a SIXAXIS connected via USB that doesn't send data
    printf(" (SIXAXIS)");
    if (SixaxisCount >= MAXMOVES) return FALSE;
    SixaxisHandles[SixaxisCount++] = handle;
    return TRUE;
  }
}

// This represents a Bluetooth adapter, or USB adapter
static int OpenMotionInJoyInterface(HANDLE handle) {
  unsigned char report[300];
  if (!HidD_GetFeature) return FALSE;
  if (MotionInJoyCount >= MAXMOVES) return FALSE;
  MotionInJoyHandles[MotionInJoyCount++] = handle;
  ZeroMemory(report, sizeof(report));
  // I think this violates the HID specification, by passing data TO the HID device, 
  // in the buffer that we are using to read from it.
  report[0]=0;
  report[1]=0;
  report[2]=0xFF;
  if (HidD_GetFeature(handle, report, 300)) {
      printf(", reading:\n");
      //for (int i=0; i<300; i++) printf("%02X ",report[i]);
      //printf("\n");
      printf("MotionInJoy Driver = %01x.%01x.%04x\n", report[2] >> 4, report[2] & 15, (report[4] << 8) | report[3]);
      printf("Device Type = %d\n", report[5]);
      printf("Adapter MAC = %02x:%02x:%02x:%02x:%02x:%02x\n",report[6],report[7],report[8],report[9],report[10],report[11]);
      printf("Manufacturer = %04x\n", (report[0x11] << 8) | report[0x10]);
      printf("HCI Version = %02x.%04x\n", report[12], (report[14] << 8) | report[13]);
      printf("LMP Version = %02x.%04x\n", report[15], (report[0x13] << 8) | report[0x12]);
      printf("Feature = %02x%02x%02x%02x%02x%02x%02x%02x\n", report[0x1b], report[0x1a], report[0x19], report[0x18], report[0x17], report[0x16], report[0x15], report[0x14]);
      for (int i=0; i<4; i++) {
        printf("%d: connected=%02X, Type=%02X, Power=%02X\n", i, report[30+50*i], report[31+50*i], report[32+50*i]);
        printf("%d: BTAddr = %02x:%02x:%02x:%02x:%02x:%02x\n",i, report[0x21+50*i],report[0x22+50*i],report[0x23+50*i],report[0x24+50*i],report[0x25+50*i],report[0x26+50*i]);
      }
  }
  return TRUE;
}

// open with libusb-win32
static int OpenUsbMoveInt(struct usb_device *dev, int itfnum) {
  if (MoveCount>=MAXMOVES || !dev || !usb_control_msg || !usb_open) return FALSE;
  usb_dev_handle *devh=usb_open(dev);
  if (!devh) return FALSE;
  MoveUsbItfs[MoveCount]=itfnum;
  MoveUsbDevs[MoveCount++]=devh;
  int SetConfig=FALSE, ClaimedInterface=FALSE, Connected=FALSE;
  unsigned char Feature04[18];
  ZeroMemory(Feature04, sizeof(Feature04));
  Feature04[0]=0x04;
  // Check if already magically connected somehow without us having to do anything.
  // This is needed because Windows may have done stuff for us.
  if (usb_control_msg(devh, USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
    1, 0x300 | 0x04, itfnum, Feature04, 17, 5000)>=0) {
      printf("already done\n");
      SetConfig = FALSE;
      ClaimedInterface = FALSE;
      Connected = TRUE;
  } else {
    if (usb_set_configuration) usb_set_configuration(devh, dev->config->bConfigurationValue);
    SetConfig = TRUE;
    ZeroMemory(Feature04, sizeof(Feature04));
    Feature04[0]=0x04;
    // We had to set the configuration, but check if we need to claim the interface.
    // This is another thing Windows may have done for us.
    if (usb_control_msg(devh, USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
      1, 0x300 | 0x04, itfnum, Feature04, 17, 5000)>=0) {
        printf("already claimed\n");
        ClaimedInterface = FALSE;
        Connected = TRUE;
    } else {
      ClaimedInterface = (usb_claim_interface && (usb_claim_interface(devh, itfnum)==0));
      ZeroMemory(Feature04, sizeof(Feature04));
      Feature04[0]=0x04;
      if (usb_control_msg(devh, USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
        1, 0x300 | 0x04, itfnum, Feature04, 17, 5000)>=0) {
          Connected = TRUE;
        printf("connected\n");
      } else {
        Connected = FALSE;
        if (ClaimedInterface && usb_release_interface) usb_release_interface(devh, itfnum);
        if (usb_close) usb_close(devh);
        devh = NULL;
        MoveCount--;
        MoveUsbDevs[MoveCount]=NULL;
        MoveUsbItfs[MoveCount]=0;
        printf("failed to connect to Move\n");
      }
    }
  }
  // debug stuff
  for (int r=0; r<=255; r++) ReadUsbFeatureReport(r,devh,itfnum);
  for (int r=0; r<=255; r++) ReadUsbInputReport(r,devh,itfnum);
  for (int r=0; r<=255; r++) ReadUsbOutputReport(r,devh,itfnum);
  return Connected;
}

// open with libusb-win32
static int OpenUsbNavInt(struct usb_device *dev, int itfnum) {
  if (NavCount>=MAXMOVES || !dev || !usb_control_msg || !usb_open) return FALSE;
  usb_dev_handle *devh=usb_open(dev);
  if (!devh) return FALSE;
  NavUsbItfs[NavCount]=itfnum;
  NavUsbDevs[NavCount++]=devh;
  int SetConfig=FALSE, ClaimedInterface=FALSE, Connected=FALSE;
  unsigned char FeatureF2[18];
  ZeroMemory(FeatureF2, sizeof(FeatureF2));
  FeatureF2[0]=0xF2;
  // Check if already magically connected somehow without us having to do anything.
  // This is needed because Windows may have done stuff for us.
  if (usb_control_msg(devh, USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
    1, 0x300 | 0xF2, itfnum, FeatureF2, 17, 5000)>=0) {
      SetConfig = FALSE;
      ClaimedInterface = FALSE;
      Connected = TRUE;
  } else {
    if (usb_set_configuration) usb_set_configuration(devh, dev->config->bConfigurationValue);
    SetConfig = TRUE;
    ZeroMemory(FeatureF2, sizeof(FeatureF2));
    FeatureF2[0]=0xF2;
    // We had to set the configuration, but check if we need to claim the interface.
    // This is another thing Windows may have done for us.
    if (usb_control_msg(devh, USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
      1, 0x300 | 0xF2, itfnum, FeatureF2, 17, 5000)>=0) {
        ClaimedInterface = FALSE;
        Connected = TRUE;
    } else {
      ClaimedInterface = (usb_claim_interface && (usb_claim_interface(devh, itfnum)==0));
      ZeroMemory(FeatureF2, sizeof(FeatureF2));
      FeatureF2[0]=0xF2;
      if (usb_control_msg(devh, USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
        1, 0x300 | 0xF2, itfnum, FeatureF2, 17, 5000)>=0) {
          Connected = TRUE;
      } else {
        Connected = FALSE;
        if (ClaimedInterface && usb_release_interface) usb_release_interface(devh, itfnum);
        if (usb_close) usb_close(devh);
        devh = NULL;
        NavCount--;
        NavUsbDevs[NavCount]=NULL;
        NavUsbItfs[NavCount]=0;
      }
    }
  }
  TestUsbDevice(devh, itfnum);
  return Connected;
}

// open with libusb-win32
static int OpenUsbSixaxisInt(struct usb_device *dev, int itfnum) {
  if (SixaxisCount>=MAXMOVES || !dev || !usb_control_msg || !usb_open) return FALSE;
  usb_dev_handle *devh=usb_open(dev);
  if (!devh) return FALSE;
  SixaxisUsbItfs[SixaxisCount]=itfnum;
  SixaxisUsbDevs[SixaxisCount++]=devh;
  int SetConfig=FALSE, ClaimedInterface=FALSE, Connected=FALSE;
  unsigned char FeatureF2[18];
  ZeroMemory(FeatureF2, sizeof(FeatureF2));
  FeatureF2[0]=0xF2;
  // Check if already magically connected somehow without us having to do anything.
  // This is needed because Windows may have done stuff for us.
  if (usb_control_msg(devh, USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
    1, 0x300 | 0xF2, itfnum, FeatureF2, 17, 5000)>=0) {
      SetConfig = FALSE;
      ClaimedInterface = FALSE;
      Connected = TRUE;
  } else {
    if (usb_set_configuration) usb_set_configuration(devh, dev->config->bConfigurationValue);
    SetConfig = TRUE;
    ZeroMemory(FeatureF2, sizeof(FeatureF2));
    FeatureF2[0]=0xF2;
    // We had to set the configuration, but check if we need to claim the interface.
    // This is another thing Windows may have done for us.
    if (usb_control_msg(devh, USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
      1, 0x300 | 0xF2, itfnum, FeatureF2, 17, 5000)>=0) {
        ClaimedInterface = FALSE;
        Connected = TRUE;
    } else {
      ClaimedInterface = (usb_claim_interface && (usb_claim_interface(devh, itfnum)==0));
      ZeroMemory(FeatureF2, sizeof(FeatureF2));
      FeatureF2[0]=0xF2;
      if (usb_control_msg(devh, USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
        1, 0x300 | 0xF2, itfnum, FeatureF2, 17, 5000)>=0) {
          Connected = TRUE;
      } else {
        Connected = FALSE;
        if (ClaimedInterface && usb_release_interface) usb_release_interface(devh, itfnum);
        if (usb_close) usb_close(devh);
        devh = NULL;
        SixaxisCount--;
        SixaxisUsbDevs[SixaxisCount]=NULL;
        SixaxisUsbItfs[SixaxisCount]=0;
      }
    }
  }
  TestUsbDevice(devh, itfnum);
  return Connected;
}

// open with libusb-win32
static int OpenUsbMove(struct usb_device *dev) {
  if (MoveCount>=MAXMOVES || !dev) return FALSE;
  for (int i=0; i<dev->descriptor.bNumConfigurations; i++) {
    for (int j=0; j<dev->config[i].bNumInterfaces; j++) {
      struct usb_interface *itf = &dev->config[i].interface[j];
      for (int k=0; k<itf->num_altsetting; k++) {
        if (itf->altsetting[k].bInterfaceClass==3)
          OpenUsbMoveInt(dev, j);
      } // check next alt setting for a HID device
    } // next interface for this Nav
  } // next config for this Nav
  return TRUE;
}

// open with libusb-win32
static int OpenUsbNav(struct usb_device *dev) {
  if (NavCount>=MAXMOVES || !dev) return FALSE;
  for (int i=0; i<dev->descriptor.bNumConfigurations; i++) {
    for (int j=0; j<dev->config[i].bNumInterfaces; j++) {
      struct usb_interface *itf = &dev->config[i].interface[j];
      for (int k=0; k<itf->num_altsetting; k++) {
        if (itf->altsetting[k].bInterfaceClass==3)
          OpenUsbNavInt(dev, j);
      } // check next alt setting for a HID device
    } // next interface for this Nav
  } // next config for this Nav
  return TRUE;
}

// open with libusb-win32
static int OpenUsbSixaxis(struct usb_device *dev) {
  if (SixaxisCount>=MAXMOVES || !dev) return FALSE;
  for (int i=0; i<dev->descriptor.bNumConfigurations; i++) {
    for (int j=0; j<dev->config[i].bNumInterfaces; j++) {
      struct usb_interface *itf = &dev->config[i].interface[j];
      for (int k=0; k<itf->num_altsetting; k++) {
        if (itf->altsetting[k].bInterfaceClass==3)
          OpenUsbSixaxisInt(dev, j);
      } // check next alt setting for a HID device
    } // next interface for this Nav
  } // next config for this Nav
  return TRUE;
}

int OpenMoves() {
	GUID HidGuid;
  for (int m=0; m<MAXMOVES; m++) {
    MoveHandles[m] = INVALID_HANDLE_VALUE;
    SixaxisHandles[m] = INVALID_HANDLE_VALUE;
    MoveHidHandles[m] = INVALID_HANDLE_VALUE;
    NavHidHandles[m] = INVALID_HANDLE_VALUE;
    SixaxisHidHandles[m] = INVALID_HANDLE_VALUE;
    GametrakHidHandles[m] = INVALID_HANDLE_VALUE;
  }
	load_dll();
  if (HidD_GetHidGuid) {
	  HidD_GetHidGuid(&HidGuid);
	  // could also try including the flag DIGCF_PRESENT
	  HDEVINFO info = SetupDiGetClassDevs(&HidGuid, NULL, NULL, DIGCF_DEVICEINTERFACE); 
	  if (!info) return FALSE;
	  SP_DEVICE_INTERFACE_DATA didata;
	  didata.cbSize = sizeof(didata);
	  for (int index = 0; SetupDiEnumDeviceInterfaces(info, NULL, &HidGuid, index, &didata); index++) {
		  DWORD req_size = 0;
		  SetupDiGetDeviceInterfaceDetail(info, &didata, NULL, 0, &req_size, NULL);
		  SP_DEVICE_INTERFACE_DETAIL_DATA *didetail = (SP_DEVICE_INTERFACE_DETAIL_DATA*) new BYTE[req_size];
		  if (!didetail) return FALSE;
		  didetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		  if (!SetupDiGetDeviceInterfaceDetail(info, &didata, didetail, req_size, &req_size, NULL)) {
			  delete[] (BYTE*)didetail;
			  return FALSE;
		  }
		  // Open the device in non-exclusive mode
		  HANDLE Handle = CreateFile(didetail->DevicePath, 0, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		  if (Handle==INVALID_HANDLE_VALUE) {
			  //printf("can't get handle\n");
			  delete[] (BYTE*)didetail;
			  continue;
		  }
		  HIDD_ATTRIBUTES attrib;
		  attrib.Size = sizeof(attrib);
		  if (!HidD_GetAttributes(Handle, &attrib)) {
			  printf("can't get attrib\n");
			  delete[] (BYTE*)didetail;
			  CloseHandle(Handle);
			  return FALSE;
		  }
		  switch (attrib.VendorID) {
			  case VID_SONY: printf("  (Windows HID interface) Sony ");
				  switch (attrib.ProductID) {
					  case PID_BDREMOTE: printf("BDRemote"); break;
            case PID_SIXAXIS: printf("SIXAXIS/Dual Shock 3"); break;
            case PID_MOVE: printf("Playstation Move"); {
              // Move does actually work via normal HID, but doesn't output any data
              if (MoveCount >= MAXMOVES) break;
              printf(", saving to list\n");
              MoveHidHandles[MoveCount++] = Handle;
							delete[] (BYTE*)didetail;
							continue;
            }
            case PID_NAVIGATION: printf("Navigation Controller"); {
              //if (NavCount >= MAXMOVES) break;
              //printf(", saving to list\n");
              //NavHidHandles[NavCount++] = Handle;
							//delete[] (BYTE*)didetail;
							//continue;
            } break;
					  default: printf("%04X", attrib.ProductID); break;
				  }
				  break;
			  case VID_MOTIONINJOY: printf("  MotionInJoy ");
				  switch (attrib.ProductID) {
            case PID_MOTIONINJOY0: printf("Interface"); {
                if (OpenMotionInJoyInterface(Handle)) printf(", saving to list\n");
                else CloseHandle(Handle);
							  delete[] (BYTE*)didetail;
							  continue;
						  }
						  break;
					  case PID_MOTIONINJOY1: printf("Unknown 1"); break;
					  case PID_MOTIONINJOY2: printf("Unknown 2"); break;
					  case PID_MOVEMOTIONINJOY: printf("Virtual Game Controller"); 
						  {
                // could be a Move or a SIXAXIS (or maybe even a BD Remote???)
                if (OpenMotionInJoy(Handle)) printf(", saving to list\n");
                else CloseHandle(Handle);
							  delete[] (BYTE*)didetail;
							  continue;
						  }
						  break;
					  default: printf("%04X", attrib.ProductID); break;
				  }
				  break;
			  case VID_IN2GAMES: printf("  In2Games ");
				  switch (attrib.ProductID) {
            case PID_GAMETRAK: printf("Gametrak"); 						  {
							  printf(", saving as number %d\n", GametrakCount);
                TestHidDevice(Handle);
                if (GametrakCount<MAXMOVES) GametrakHidHandles[GametrakCount++] = Handle;
							  else CloseHandle(Handle);
							  delete[] (BYTE*)didetail;
							  continue;
						  }
					  default: printf("%04X", attrib.ProductID); break;
				  }
				  break;
        // this is a 3rd party imitation DualShock2 for the PC
			  case VID_RITMO: printf("  Ritmo ");
          switch (attrib.ProductID) {
            case PID_RITMOPAD: printf("Twin USB Vibration Gamepad"); break;
					  default: printf("%04X", attrib.ProductID); break;
				  }
          break;
        // DEBUG stuff only, these products are not supported, we just want to list what's plugged in
			  case VID_MICROSOFT: printf("  Microsoft ");
				  switch (attrib.ProductID) {
            case PID_XBOX360CONTROL: printf("Xbox 360 controller");
					  default: printf("%04X", attrib.ProductID); break;
				  }
				  break;
			  case VID_NINTENDO: printf("  Nintendo ");
				  switch (attrib.ProductID) {
					  case PID_WIIMOTE: printf("Wii Remote"); break;
					  default: printf("%04X", attrib.ProductID); break;
				  }
				  break;
			  case VID_PPJOY: printf("  PPJoy ");
				  if (attrib.ProductID>=PID_PPJOY1 && attrib.ProductID<=PID_PPJOY16) printf("Virtual Joystick %d", attrib.ProductID+1-PID_PPJOY1);
				  else printf("%04X", attrib.ProductID); 
				  break;
			  case VID_VUZIX: printf("  Vuzix ");
				  switch (attrib.ProductID) {
					  case PID_VR920: printf("VR920"); break;
					  default: printf("%04X", attrib.ProductID); break;
				  }
          break;
			  case VID_OCZNIA: printf("  Possible OCZ ");
				  switch (attrib.ProductID) {
					  case PID_OCZNIA: printf("NIA"); break;
					  default: printf("%04X", attrib.ProductID); break;
				  }
          break;
			  case VID_ESSENTIALREALITY: printf("  Essential Reality ");
				  switch (attrib.ProductID) {
					  case PID_P5: printf("P5 Glove"); break;
					  default: printf("%04X", attrib.ProductID); break;
				  }
          break;
			  case VID_PNI: printf("  PNI ");
				  switch (attrib.ProductID) {
					  case PID_SPACEPOINT: printf("SpacePoint Fusion"); break;
					  default: printf("%04X", attrib.ProductID); break;
				  }
          break;
			  case VID_LOGITECH: printf("  Logitech ");
					  printf("%04X", attrib.ProductID); break;
			  case VID_NOKIA: printf("  Nokia ");
					  printf("%04X", attrib.ProductID); break;
			  default: printf(" %04X:%04X ", attrib.VendorID, attrib.ProductID); break;
		  }
      printf("\n");
		  delete[] (BYTE*)didetail;
		  CloseHandle(Handle);
		  Handle = INVALID_HANDLE_VALUE;

	  }
	  SetupDiDestroyDeviceInfoList(info);
  }
  // now check with libusb-win32, if available
  if (!libusb_dll) return TRUE;
  if ((!usb_find_busses) || (usb_find_busses()<0)) return FALSE;
  if ((!usb_find_devices) || (usb_find_devices()<0)) return FALSE;
  if (!usb_get_busses) return FALSE;
  struct usb_bus *busses = usb_get_busses();
  if (!busses) return FALSE;
  struct usb_bus *bus = busses;
  while (bus) {
    struct usb_device *dev = bus->devices;
    while (dev) {
      switch (dev->descriptor.idVendor) {
        case 0: break;
			  case VID_SONY: printf("  (USB) Sony ");
				  switch (dev->descriptor.idProduct) {
					  case PID_BDREMOTE: printf("BDRemote"); break;
            case PID_SIXAXIS: {
              printf("SIXAXIS/Dual Shock 3\n");
              OpenUsbSixaxis(dev);
              dev=dev->next;
              continue;
            }
            case PID_MOVE: {
              printf("Playstation Move\n");
              OpenUsbMove(dev);
              dev=dev->next;
							continue;
            }
            case PID_NAVIGATION: {
              printf("Navigation Controller\n");
              OpenUsbNav(dev);
              dev=dev->next;
							continue;
            }
					  default: printf("%04X", dev->descriptor.idProduct); break;
				  }
				  break;
			  case VID_MOTIONINJOY: printf("  (USB) MotionInJoy ");
				  switch (dev->descriptor.idProduct) {
					  case PID_MOTIONINJOY0: printf("Interface"); break;
					  case PID_MOTIONINJOY1: printf("Unknown 1"); break;
					  case PID_MOTIONINJOY2: printf("Unknown 2"); break;
					  case PID_MOVEMOTIONINJOY: printf("Virtual Game Controller");
						  {
							  //printf(", saving as number %d\n", MoveCount);
							  //if (MoveCount<MAXMOVES) MoveHandles[MoveCount++] = Handle;
							  //else CloseHandle(Handle);
							  //delete[] (BYTE*)didetail;
							  //continue;
						  }
						  break;
					  default: printf("%04X", dev->descriptor.idProduct); break;
				  }
				  break;
			  case VID_IN2GAMES: printf("  (USB) In2Games ");
				  switch (dev->descriptor.idProduct) {
            case PID_GAMETRAK: printf("Gametrak"); break;
					  default: printf("%04X", dev->descriptor.idProduct); break;
				  }
				  break;
        case VID_IN2GAMESHUB: printf("  (USB) In2Games Hub ");
				  switch (dev->descriptor.idProduct) {
            case PID_GAMETRAKHUB: printf("Gametrak Hub"); break;
					  default: printf("%04X", dev->descriptor.idProduct); break;
				  }
				  break;
			  case VID_RITMO: printf("  (USB) Ritmo ");
				  switch (dev->descriptor.idProduct) {
					  case PID_RITMOPAD: printf("Twin USB Vibration Gamepad"); break;
					  default: printf("%04X", dev->descriptor.idProduct); break;
				  }
          break;
			  case VID_MICROSOFT: printf("  Microsoft ");
				  switch (dev->descriptor.idProduct) {
            case PID_KINECTCAMERA: printf("Kinect Camera"); break;
            case PID_KINECTMIC: printf("Kinect Microphone"); break;
            case PID_KINECTMOTOR: printf("Kinect Motor"); break;
            case PID_XBOX360CONTROL: printf("Xbox 360 controller"); break;
					  default: printf("%04X", dev->descriptor.idProduct); break;
				  }
				  break;
        // DEBUG stuff only, these products are not supported, we just want to list what's plugged in
        case VID_BROADCOM: printf("  (USB) Broadcom ");
					printf("%04X", dev->descriptor.idProduct);
				  break;
			  case VID_LOGITECH: printf("  (USB) Logitech ");
					printf("%04X", dev->descriptor.idProduct);
				  break;
			  case VID_MAXTOR: printf("  (USB) Maxtor ");
					printf("%04X", dev->descriptor.idProduct);
				  break;
			  case VID_NOKIA: printf("  (USB) Nokia ");
					printf("%04X", dev->descriptor.idProduct);
				  break;
			  case VID_NINTENDO: printf("  (USB) Nintendo ");
				  switch (dev->descriptor.idProduct) {
					  case PID_WIIMOTE: printf("Wii Remote"); break;
					  default: printf("%04X", dev->descriptor.idProduct); break;
				  }
				  break;
			  case VID_PPJOY: printf("  (USB) PPJoy ");
				  if (dev->descriptor.idProduct>=PID_PPJOY1 && dev->descriptor.idProduct<=PID_PPJOY16) printf("Virtual Joystick %d", dev->descriptor.idProduct+1-PID_PPJOY1);
				  else printf("%04X", dev->descriptor.idProduct); 
				  break;
			  case VID_VUZIX: printf("  (USB) Vuzix ");
				  switch (dev->descriptor.idProduct) {
					  case PID_VR920: printf("VR920"); break;
					  default: printf("%04X", dev->descriptor.idProduct); break;
				  }
          break;
			  case VID_OCZNIA: printf("  (USB) Possible OCZ ");
				  switch (dev->descriptor.idProduct) {
					  case PID_OCZNIA: printf("NIA"); break;
					  default: printf("%04X", dev->descriptor.idProduct); break;
				  }
          break;
			  case VID_ESSENTIALREALITY: printf("  (USB) Essential Reality ");
				  switch (dev->descriptor.idProduct) {
					  case PID_P5: printf("P5 Glove"); break;
					  default: printf("%04X", dev->descriptor.idProduct); break;
				  }
          break;
			  case VID_PNI: printf("  (USB) PNI ");
				  switch (dev->descriptor.idProduct) {
					  case PID_SPACEPOINT: printf("SpacePoint Fusion"); break;
					  default: printf("%04X", dev->descriptor.idProduct); break;
				  }
          break;
        default: printf(" (USB) %04X:%04X ", dev->descriptor.idVendor, dev->descriptor.idProduct); break;
		  }
      if (dev->descriptor.idVendor) printf("\n");
      dev=dev->next;
    }
    bus=bus->next;
  }

  
  return TRUE;
}

int GetMoveCount() {
	return MoveCount;
}

int GetNavCount() {
	return NavCount;
}

int GetSixaxisCount() {
	return SixaxisCount;
}

// Currently the only feature that works for Gametrak (PS2 version).
// I need to do some hacking.
int GetGametrakCount() {
	return GametrakCount;
}

int GetMotionInJoyCount() {
	return MotionInJoyCount;
}

int CloseMoves() {
  for (int i=0; i<MAXMOVES; i++) {
    // Close MotionInJoys
		if (MoveHandles[i]!=INVALID_HANDLE_VALUE) {
			CloseHandle(MoveHandles[i]);
			MoveHandles[i]=INVALID_HANDLE_VALUE;
		}
		if (NavHandles[i]!=INVALID_HANDLE_VALUE) {
			CloseHandle(NavHandles[i]);
			NavHandles[i]=INVALID_HANDLE_VALUE;
		}
		if (SixaxisHandles[i]!=INVALID_HANDLE_VALUE) {
			CloseHandle(SixaxisHandles[i]);
			SixaxisHandles[i]=INVALID_HANDLE_VALUE;
		}
    if (MotionInJoyHandles[i]!=INVALID_HANDLE_VALUE) {
			CloseHandle(MotionInJoyHandles[i]);
			MotionInJoyHandles[i]=INVALID_HANDLE_VALUE;
		}
    // Close Windows HIDs
		if (MoveHidHandles[i]!=INVALID_HANDLE_VALUE) {
			CloseHandle(MoveHidHandles[i]);
			MoveHidHandles[i]=INVALID_HANDLE_VALUE;
		}
		if (NavHidHandles[i]!=INVALID_HANDLE_VALUE) {
			CloseHandle(NavHidHandles[i]);
			NavHidHandles[i]=INVALID_HANDLE_VALUE;
		}
		if (SixaxisHidHandles[i]!=INVALID_HANDLE_VALUE) {
			CloseHandle(SixaxisHidHandles[i]);
			SixaxisHidHandles[i]=INVALID_HANDLE_VALUE;
		}
    if (GametrakHidHandles[i]!=INVALID_HANDLE_VALUE) {
      CloseHandle(GametrakHidHandles[i]);
			GametrakHidHandles[i]=INVALID_HANDLE_VALUE;
		}
    // Close libusb-win32s
    if (MoveUsbDevs[i] && usb_close) {
      // should we release the interface? doesn't seem necessary
      usb_close(MoveUsbDevs[i]);
      MoveUsbDevs[i]=NULL;
      MoveUsbItfs[i]=0;
    }
    if (NavUsbDevs[i] && usb_close) {
      // should we release the interface? doesn't seem necessary
      usb_close(NavUsbDevs[i]);
      NavUsbDevs[i]=NULL;
      NavUsbItfs[i]=0;
    }
    if (SixaxisUsbDevs[i] && usb_close) {
      // should we release the interface? doesn't seem necessary
      usb_close(SixaxisUsbDevs[i]);
      SixaxisUsbDevs[i]=NULL;
      SixaxisUsbItfs[i]=0;
    }
    if (GametrakUsbDevs[i] && usb_close) {
      // should we release the interface? doesn't seem necessary
      usb_close(GametrakUsbDevs[i]);
      GametrakUsbDevs[i]=NULL;
      GametrakUsbItfs[i]=0;
    }
  }
	return TRUE;
}

#endif