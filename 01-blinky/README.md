# 1 Simple "blinky" for the icebreaker

1. Check that the tools are working. Run:

       $ make prog

and in a couple of seconds, the LEDs should be blinking
and respond to button presses. Note how fast the tools
can build a small design (can you see which part of
the flow is taking the most time?)

2. Use the tools to visualise the design. Run

       $ make show-rtl
       $ make show-pnr

The first command will use GraphViz and xdot to visualise
the elaborated RTL. The latter will open nextpnr in GUI
mode. Click the "Place" and "Route" buttons to see the
steps (unfortunately this won't work on the Pis)

3. Hack the design! Try one of the following, or something
of your own choosing!
    - Change the pattern/order that the LEDs blink in
    - Make the LEDs fade on or off
    - Try and break the tools...
