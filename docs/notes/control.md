# Control Behaviours

The server will by default scan over frequency ranges
for a number of samples per scan.

However a control protocol should be 
able to override this behaviour.

So there is at least two states:

- automatic scanning
- controller mode.

A control channel protocol will be defined
to communicate the mode with the server.

By default at startup the
server mode is automatic scanning.
