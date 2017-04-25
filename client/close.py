#!/usr/bin/python
#encoding: utf-8
import sys
sys.path.append('../thrift/gen-py')

from thrift import Thrift
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol
from media_server import MediaServer

if __name__ == '__main__':
	try:

  		# Make socket
  		transport = TSocket.TSocket('localhost', 8888)

  		# Buffering is critical. Raw sockets are very slow
  		transport = TTransport.TBufferedTransport(transport)

  		# Wrap in a protocol
  		protocol = TBinaryProtocol.TBinaryProtocol(transport)

  		# Create a client to use the protocol encoder
  		client = MediaServer.Client(protocol)

  		# Connect!
  		transport.open()
  
  		client.login("202.169.88.111")
  		client.shutdownServer()
  
  		# Close
  		transport.close()
	except Thrift.TException, tx: 
  		print '%s' % (tx.message)



