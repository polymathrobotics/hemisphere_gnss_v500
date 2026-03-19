#!/bin/bash

# kill any tcp process running
pkill -9 -f test_nmea_data.py
cd ..
cd test
python3 test_nmea_data.py
