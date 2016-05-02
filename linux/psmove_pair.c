/*
 * psmove_pair.c version 2010-10-17
 * Compile with: gcc -o psmove_pair psmove_pair.c -lusb
 */

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <usb.h>

//#include <bluetooth/bluetooth.h> //not used at the moment
//#include <bluetooth/sdp.h>       //will need appended `pkg-config --libs --cflags bluez`
//#include <bluetooth/sdp_lib.h>

#define VENDOR 0x054c
#define PRODUCT 0x03d5

#define USB_DIR_IN 0x80
#define USB_DIR_OUT 0

void fatal(char *msg) { perror(msg); exit(1); }

void show_master(usb_dev_handle *devh, int itfnum) {
  unsigned char msg[16];
  unsigned char buf[18];
  int res;
  res = usb_control_msg
    (devh, USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
     0x01, 0x0304, itfnum, (void*)msg, sizeof(msg), 5000);
  if ( res < 0 ) { perror("USB_REQ_GET_CONFIGURATION"); return; }
  printf("Bluetooth device address: ");
  printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
	 msg[6], msg[5], msg[4], msg[3], msg[2], msg[1]);
  printf("Current Bluetooth master: ");
  printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
	 msg[15], msg[14], msg[13], msg[12], msg[11], msg[10]);
}

void set_master(usb_dev_handle *devh, int itfnum, int mac[6]) {
  printf("Setting master bd_addr to %02x:%02x:%02x:%02x:%02x:%02x\n",
	 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  char msg[]= { 0x05, mac[5],mac[4],mac[3],mac[2],mac[1],mac[0], 0x6B, 0x72, 0x81, 0xcf, 0x11, 0xf7, 0x25, 0xe4, 0x08, 0x8b, 0x1b, 0xd5, 0x58, 0xbb, 0xb2, 0x81 };
  int res = usb_control_msg
    (devh,
     USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
     0x09,
     0x0305, itfnum, msg, sizeof(msg),
     5000);
  if ( res < 0 ) fatal("USB_REQ_SET_CONFIGURATION");
}

void process_device(int argc, char **argv, struct usb_device *dev,
		    struct usb_config_descriptor *cfg, int itfnum) {
  int mac[6], have_mac=0;

  usb_dev_handle *devh = usb_open(dev);
  if ( ! devh ) fatal("usb_open");

  usb_detach_kernel_driver_np(devh, itfnum);

  int res = usb_claim_interface(devh, itfnum);
  if ( res < 0 ) fatal("usb_claim_interface");

  show_master(devh, itfnum);

  if ( argc >= 2 ) {
    if ( sscanf(argv[1], "%x:%x:%x:%x:%x:%x",
		&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]) != 6 ) {

      printf("usage: %s [<bd_addr of master>]\n", argv[0]);
      exit(1);
    }
  } else {
    FILE *f = popen("hcitool dev", "r");
    if ( !f ||
	 fscanf(f, "%*s\n%*s %x:%x:%x:%x:%x:%x",
		&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]) != 6 ) {
      printf("Unable to retrieve local bd_addr from `hcitool dev`.\n");
      printf("Please enable Bluetooth or specify an address manually.\n");
      exit(1);
    }
    pclose(f);
  }

  set_master(devh, itfnum, mac);

  usb_close(devh);
}

int main(int argc, char *argv[]) {

  usb_init();
  if ( usb_find_busses() < 0 ) fatal("usb_find_busses");
  if ( usb_find_devices() < 0 ) fatal("usb_find_devices");
  struct usb_bus *busses = usb_get_busses();
  if ( ! busses ) fatal("usb_get_busses");

  int found = 0;

  struct usb_bus *bus;
  for ( bus=busses; bus; bus=bus->next ) {
    struct usb_device *dev;
    for ( dev=bus->devices; dev; dev=dev->next) {
      struct usb_config_descriptor *cfg;
      for ( cfg = dev->config;
	    cfg < dev->config + dev->descriptor.bNumConfigurations;
	    ++cfg ) {
	int itfnum;
	for ( itfnum=0; itfnum<cfg->bNumInterfaces; ++itfnum ) {
	  struct usb_interface *itf = &cfg->interface[itfnum];
	  struct usb_interface_descriptor *alt;
	  for ( alt = itf->altsetting;
		alt < itf->altsetting + itf->num_altsetting;
		++alt ) {
	    if ( dev->descriptor.idVendor == VENDOR &&
		 dev->descriptor.idProduct == PRODUCT &&
		 alt->bInterfaceClass == 3 ) {
	      process_device(argc, argv, dev, cfg, itfnum);
	      ++found;
	    }
	  }
	}
      }
    }
  }

  if ( ! found ) printf("No controller found on USB busses.\n");
  return 0;

}

