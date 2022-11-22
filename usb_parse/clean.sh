if [[ -f target/debug/usb_parse ]]; then
  mv target/debug/usb_parse usb_parse
fi
rm -rf target/debug
rm -rf target/rls