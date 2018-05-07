import os
import sys
import socket
from threading import Thread
from time import sleep
MSGLEN = 128

class Target:
  '''
  '''

  def __init__(self, sock=None):
    if sock is None:
      self.sock = socket.socket(
        socket.AF_INET, socket.SOCK_STREAM)
      self.sock.setblocking(True)         
    else:
      self.sock = sock

  def connect(self, host, port):
    self.sock.connect((host, port))

  def mysend(self, msg):
    totalsent = 0
    while totalsent < len(msg):
      sent = self.sock.send(msg[totalsent:])
      if sent == 0:
        raise RuntimeError("socket connection broken")
      totalsent = totalsent + sent
    return totalsent

  def myreceive(self, func):
    while 1:
      data = self.sock.recv(4096)
      if not data:
        raise RuntimeError("socket connection broken")
      
      func(data)


def get_immediate_subdirectories(a_dir):
  return [name for name in os.listdir(a_dir)
          if os.path.isdir(os.path.join(a_dir, name))]

def handleRecv(data):
  print(data)

def recvThread(s):
  try:
    s.myreceive(handleRecv)  
  except RuntimeError, e:
    print e

if __name__ == "__main__":
  targets = get_immediate_subdirectories(sys.argv[1])

  target = Target()
  address = targets[0][3:]
  print address

  target.connect(address, 80)
  thread = Thread(target = recvThread, args = (target,))
  thread.start()

  print(target.mysend("http://192.168.1.130/\r\n\r\n"))
  sleep(10)