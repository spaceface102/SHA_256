#!/usr/bin/python3
from sys import argv

if len(argv) == 1:
    print("figure out your bash script bro...")
else:
    mess = input("Please enter a message!: ")
    f = open(argv[1], "wb")
    f.write(bytes(mess, encoding = 'utf-8'))
    f.close()
