# libmosquitto

MacOS dependencies

```aiignore
brew install mosquitto
```

Linux dependencies

```aiignore
apt install mosquitto-dev libmosquitto-dev
```

## Deployment Model

We will need a service unit that defines the mosquitto service.
The mosquitto service is installed in linux
as a systemd service.
It's runtime is configured as mosquitto.conf
which will include files in
```
/etc/mosquitto/conf.d/
```

The basic configuration we will distribute for testing will be:

```aiignore

listener 1833 0.0.0.0

allow_anonymous true
```


## Monitoring

It is possible to monitor messages sent over the mosquitto service 
by subscribing with a wildcard :

```aiignore
 mosquitto_sub -h localhost -t "sdrcomms/#"
```

The topic has the structure "sdrcomms/$subtopic/$message"

subscribing with "sdrcomms/#" will receive all messages in that path.

