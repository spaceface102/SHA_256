#!/usr/bin/bash
fname="message_file.txt"
numargs=0
if [ "$#" -eq $numargs ] 
then
	./forbashmessage/temp_file_message.py $fname
	./mySHA256.exe $fname
	./forbashmessage/delete_temp_message.py $fname
else
	./mySHA256.exe $1
fi
