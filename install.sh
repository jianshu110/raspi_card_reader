#!/bin/sh
#build the program and copy it to proper folder
#gcc mult_adaptor.cpp -o card_reader_adaptor_bin -lwiringPi -lcurl


sudo ps aux |grep /usr/bin/card_reader_adaptor_bin|awk '{print $2}'|xargs kill -9

PID_FILE=/var/run/supervisord.pid

PID=$(cat "${PID_FILE}")
kill -9 ${PID}


sudo chown root:root card_reader_adaptor_bin
sudo cp card_reader_adaptor_bin /usr/bin/

sudo cp libwiringPi.so.2.29 /usr/local/lib/
sudo ln -sf /usr/local/lib/libwiringPi.so.2.29 /usr/lib/libwiringPi.so
sudo ln -sf /usr/local/lib/libwiringPi.so.2.29 /usr/local/lib/libwiringPi.so
sudo cp libwiringPiDev.so.2.29 /usr/local/lib/
sudo ln -sf /usr/local/lib/libwiringPiDev.so.2.29 /usr/lib/libwiringPiDev.so
sudo ln -sf /usr/local/lib/libwiringPiDev.so.2.29 /usr/local/lib/libwiringPiDev.so

sudo chown root:root gpio
sudo cp gpio /usr/local/bin/

sudo rm *.cpp
sudo rm *.h
sudo rm -rf .git

sudo chmod +x /usr/bin/card_reader_adaptor_bin

#install supervisor and make it supervise card_reader_adaptor.
sudo apt-get install supervisor --force-yes -y
sudo cp card_reader_adaptor.conf /etc/supervisor/conf.d/ 
sudo /etc/init.d/supervisor restart
