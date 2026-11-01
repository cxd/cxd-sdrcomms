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


The spectrum is large area, so we may well want to experiment with different scanning
strategies.

In a larger application, multiple instances of receivers and drivers would be applied
to potentially scan different segments of the spectrum. 
This would make sense to subdivide the ranges amongst multiple instances.

Data aggregation can be performed by mqtt clients in that case.