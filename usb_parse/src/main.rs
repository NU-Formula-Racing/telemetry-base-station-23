use libusb::{
    Context,
};

fn main() {
    // let teensy: DeviceHandle;
    for dev in Context::new().unwrap().devices().unwrap().iter() {
        let dev_desc = dev.device_descriptor().unwrap();

        // println!("Bus {:03} Device {:03} ID {:04x}:{:04x}",
        //     device.bus_number(),
        //     device.address(),
        //     device_desc.vendor_id(),
        //     device_desc.product_id()
        // );
        if dev_desc.vendor_id() == 0x16c0_u16 && dev_desc.product_id() == 0x0483_u16 {
            println!("Teensy detected!");
            // teensy = device.open().unwrap();
            break;
        }
    }
}
