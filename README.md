
# CamHarvester - multipart/x-mixed-replace IP camera video capture application

CamHarvester is a C++ console application developed to save multiple video streams from multipart/x-mixed-replace sources to JPEG sequence. You can manually convert the sequence of JPEG files into video using FFMPEG. Usualy the source of video is an old IP camera with MJPEG push feature: Axis, Linksys, Mobotix, Sony, Foscam or desktop or mobile application linke MJPG-Streamer or WebcamXP. Since this application uses ASIO boost, it theoretically allows you to save multiple streams at the same time without high CPU load. But you should understand that MJPEG stream is not optimal and effective way to stream video so 30-50 streams can completely consume your internet connection.

CamHarvester does not use any HTTP library because there is not a big choice of ones that support multipart/x-mixed-replace stream.
So it's implemented on very low level receiving TCP stream from Boost:ASIO asynchronously in one thread.

You can get MJPEG sources from web sites like http://insecam.org/
