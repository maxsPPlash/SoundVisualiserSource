ffmpeg -r 60 -f image2 -s 3840x2160 -start_number 1 -i img_%%5d.png -i Alexander_Platz.mp3 -acodec copy -vcodec libx264 -crf 15 -pix_fmt yuv420p Alexander_Platz.mp4
pause