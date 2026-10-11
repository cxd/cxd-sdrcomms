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

