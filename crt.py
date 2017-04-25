#!/usr/bin/python
#encoding: utf-8
import sys
sys.path.append('../thrift/gen-py')
sys.path.append('../proto')

from thrift import Thrift
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol
from media_server import MediaServer
import taskinfo_pb2

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
  		token = client.login("202.169.88.111")
		info = taskinfo_pb2.TaskInfo()
		info.imgpath = "/mnt/unused/ftms/images"
		info.desc = "for test"
		tskinfo = info.SerializeToString()
  		taskid = client.creatTask(tskinfo)
		print "token:"+token
		print "taskid:"+taskid
  
  		# Close
  		transport.close()
	except Thrift.TException, tx: 
  		print '%s' % (tx.message)



