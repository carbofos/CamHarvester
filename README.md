
# CamHarvester - multipart/x-mixed-replace IP camera video capture application

CamHarvester is a C++ console application developed to save multiple video streams from multipart/x-mixed-replace sources to a JPEG sequence. You can manually convert the sequence of JPEG files into a video using FFMPEG. Usually, the source of the video is an old IP camera with an MJPEG push feature: Axis, Linksys, Mobotix, Sony, Foscam, or a desktop or mobile application like MJPG-Streamer or WebcamXP. Since this application uses ASIO boost, it theoretically allows you to save multiple streams at the same time without a high CPU load. But you should understand that an MJPEG stream is not an optimal and effective way to stream video, so 30-50 streams can completely consume your internet connection.

CamHarvester does not use any HTTP library because there is not a big choice of ones that support multipart/x-mixed-replace stream. So it's implemented on a very low level, receiving a TCP stream from Boost:ASIO asynchronously in one thread.

(C) Novak Sergey 2023

You can get MJPEG sources from web sites like http://insecam.org/
