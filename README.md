# opencvsteampunkgoggles gstreamer plugin

How to run opencvsteampunkgoggles plugin

Ubuntu install libraries
sudo apt-get install libgstreamer1.0-0 gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-doc gstreamer1.0-tools libgstreamer-plugins-base1.0-dev

gst-launch-1.0 --gst-plugin-path=$HOME/gstreamer/SteamPunkGogglesOpenCVPlugin v4l2src device=/dev/video0 ! 'video/x-raw,format=RGB,width=800,height=600,framerate=30/1' ! opencvsteampunkgoggles ! videoconvert ! autovideosink
