#!/usr/bin/python
# -*- coding: utf-8 -*-

import os, random, time

logfile = "explore.log"

def main():
  while 1:
    l = random.choice(range(5, 10))
    string = ""
    for _ in range(l):
      string += random.choice("LRSB")
    ret = os.system("./antsim -a 1 -c %s 2>> %s" % (string, logfile))
    if ret != 0:
      break

if __name__ == "__main__": main()