#!/usr/local/bin/python

from httplib import HTTPConnection

def hex(buffer):
  str = ''
  for item in range(len(buffer)):
    str += '%02X' % ord(buffer[item]) + ' '
  return str

def GetTemperature(data, offset):
  temp = ord(data[offset + 8]) + (ord(data[offset + 9]) << 8)
  #print temp
  sign = not not (temp & 0x8000)
  if sign:
    temp = -temp
  t100 = temp * 6 + (temp + 2) / 4;  # * 100 * 1/16 = * 6.25
  if sign:
    t100 = -t100
  return t100/100.

def GetTemperatures():
  conn = HTTPConnection("192.168.1.111", timeout = 2)
  conn.request("GET", "/")
  resp = conn.getresponse()
  data = resp.read()
  #print hex(data)
  seq = ord(data[0]) + (ord(data[1]) << 8) + (ord(data[2]) << 16) + \
        (ord(data[3]) << 24)
  length = ord(data[4])
  assert length == 20
  temp = []
  for button in range(2):
    temp.append(GetTemperature(data, 5 + button * 10))
  return (seq, temp)
  
#print GetTemperatures()
(seq, temp) = GetTemperatures()
print int(temp[0] * 100)
print int(temp[1] * 100)
print str(seq / 60) + ':' + str(seq % 60)
print "Nanode 1-wire"
