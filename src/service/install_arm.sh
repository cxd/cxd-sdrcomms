#!/bin/bash
sudo systemctl stop soapyremote-custom.service &>> /dev/null
sudo systemctl stop mosquitto &>> /dev/null
sudo rm -rf /opt/sdrcomms

sudo mkdir -p /opt/sdrcomms
sudo cp -r sdrcomms/bin /opt/sdrcomms/
sudo cp -r sdrcomms/lib /opt/sdrcomms/
sudo cp -r resources /opt/sdrcomms/
sudo chmod +x /opt/sdrcomms/bin/*.sh
sudo cp services/soapyremote-custom-arm.service /etc/systemd/system/soapyremote-custom.service
sudo cp services/mosquitto.service /etc/systemd/system/

sudo cp services/etc/mosquitto/conf.d/*.conf /etc/mosquitto/conf.d/



sudo systemctl daemon-reload
sudo systemctl enable soapyremote-custom.service
sudo systemctl start soapyremote-custom.service

sudo systemctl enable mosquitto.service
sudo systemctl start mosquitto.service



