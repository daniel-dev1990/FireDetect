set exe=FireDetect\bin_cpu\x64\FireDetect_cpu.exe
set modelpath=model\deploy.prototxt
set weightpath=model\fire_SSD_300x300_iter_150000.caffemodel

%exe% %modelpath% %weightpath% 640 360 -vdf sample.mp4 -th 0.3
pause
