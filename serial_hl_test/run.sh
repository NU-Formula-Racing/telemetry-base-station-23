if [ $# -gt 0 ]; then
  mv test_data.csv test_data_0.csv
fi

python3 serial_hl_test.py