# opencvsteampunkgoggles gstreamer plugin

How to run opencvsteampunkgoggles plugin

gst-launch-1.0 --gst-plugin-path=/home/warren/gstreamer/SteamPunkGogglesOpenCVPlugin v4l2src device=/dev/video0 ! 'video/x-raw,format=RGB,width=800,height=600,framerate=30/1' ! opencvsteampunkgoggles ! videoconvert ! autovideosink
