#
# Autogenerated by Thrift Compiler (0.9.2)
#
# DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
#
#  options string: py
#

from thrift.Thrift import TType, TMessageType, TException, TApplicationException
from ttypes import *
from thrift.Thrift import TProcessor
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol, TProtocol
try:
  from thrift.protocol import fastbinary
except:
  fastbinary = None


class Iface:
  def p4_program_config_add(self, program_name, config_str):
    """
    Parameters:
     - program_name
     - config_str
    """
    pass

  def p4_program_config_delete(self, program_name):
    """
    Parameters:
     - program_name
    """
    pass


class Client(Iface):
  def __init__(self, iprot, oprot=None):
    self._iprot = self._oprot = iprot
    if oprot is not None:
      self._oprot = oprot
    self._seqid = 0

  def p4_program_config_add(self, program_name, config_str):
    """
    Parameters:
     - program_name
     - config_str
    """
    self.send_p4_program_config_add(program_name, config_str)
    return self.recv_p4_program_config_add()

  def send_p4_program_config_add(self, program_name, config_str):
    self._oprot.writeMessageBegin('p4_program_config_add', TMessageType.CALL, self._seqid)
    args = p4_program_config_add_args()
    args.program_name = program_name
    args.config_str = config_str
    args.write(self._oprot)
    self._oprot.writeMessageEnd()
    self._oprot.trans.flush()

  def recv_p4_program_config_add(self):
    iprot = self._iprot
    (fname, mtype, rseqid) = iprot.readMessageBegin()
    if mtype == TMessageType.EXCEPTION:
      x = TApplicationException()
      x.read(iprot)
      iprot.readMessageEnd()
      raise x
    result = p4_program_config_add_result()
    result.read(iprot)
    iprot.readMessageEnd()
    if result.success is not None:
      return result.success
    raise TApplicationException(TApplicationException.MISSING_RESULT, "p4_program_config_add failed: unknown result");

  def p4_program_config_delete(self, program_name):
    """
    Parameters:
     - program_name
    """
    self.send_p4_program_config_delete(program_name)
    return self.recv_p4_program_config_delete()

  def send_p4_program_config_delete(self, program_name):
    self._oprot.writeMessageBegin('p4_program_config_delete', TMessageType.CALL, self._seqid)
    args = p4_program_config_delete_args()
    args.program_name = program_name
    args.write(self._oprot)
    self._oprot.writeMessageEnd()
    self._oprot.trans.flush()

  def recv_p4_program_config_delete(self):
    iprot = self._iprot
    (fname, mtype, rseqid) = iprot.readMessageBegin()
    if mtype == TMessageType.EXCEPTION:
      x = TApplicationException()
      x.read(iprot)
      iprot.readMessageEnd()
      raise x
    result = p4_program_config_delete_result()
    result.read(iprot)
    iprot.readMessageEnd()
    if result.success is not None:
      return result.success
    raise TApplicationException(TApplicationException.MISSING_RESULT, "p4_program_config_delete failed: unknown result");


class Processor(Iface, TProcessor):
  def __init__(self, handler):
    self._handler = handler
    self._processMap = {}
    self._processMap["p4_program_config_add"] = Processor.process_p4_program_config_add
    self._processMap["p4_program_config_delete"] = Processor.process_p4_program_config_delete

  def process(self, iprot, oprot):
    (name, type, seqid) = iprot.readMessageBegin()
    if name not in self._processMap:
      iprot.skip(TType.STRUCT)
      iprot.readMessageEnd()
      x = TApplicationException(TApplicationException.UNKNOWN_METHOD, 'Unknown function %s' % (name))
      oprot.writeMessageBegin(name, TMessageType.EXCEPTION, seqid)
      x.write(oprot)
      oprot.writeMessageEnd()
      oprot.trans.flush()
      return
    else:
      self._processMap[name](self, seqid, iprot, oprot)
    return True

  def process_p4_program_config_add(self, seqid, iprot, oprot):
    args = p4_program_config_add_args()
    args.read(iprot)
    iprot.readMessageEnd()
    result = p4_program_config_add_result()
    result.success = self._handler.p4_program_config_add(args.program_name, args.config_str)
    oprot.writeMessageBegin("p4_program_config_add", TMessageType.REPLY, seqid)
    result.write(oprot)
    oprot.writeMessageEnd()
    oprot.trans.flush()

  def process_p4_program_config_delete(self, seqid, iprot, oprot):
    args = p4_program_config_delete_args()
    args.read(iprot)
    iprot.readMessageEnd()
    result = p4_program_config_delete_result()
    result.success = self._handler.p4_program_config_delete(args.program_name)
    oprot.writeMessageBegin("p4_program_config_delete", TMessageType.REPLY, seqid)
    result.write(oprot)
    oprot.writeMessageEnd()
    oprot.trans.flush()


# HELPER FUNCTIONS AND STRUCTURES

class p4_program_config_add_args:
  """
  Attributes:
   - program_name
   - config_str
  """

  thrift_spec = (
    None, # 0
    (1, TType.STRING, 'program_name', None, None, ), # 1
    (2, TType.STRING, 'config_str', None, None, ), # 2
  )

  def __init__(self, program_name=None, config_str=None,):
    self.program_name = program_name
    self.config_str = config_str

  def read(self, iprot):
    if iprot.__class__ == TBinaryProtocol.TBinaryProtocolAccelerated and isinstance(iprot.trans, TTransport.CReadableTransport) and self.thrift_spec is not None and fastbinary is not None:
      fastbinary.decode_binary(self, iprot.trans, (self.__class__, self.thrift_spec))
      return
    iprot.readStructBegin()
    while True:
      (fname, ftype, fid) = iprot.readFieldBegin()
      if ftype == TType.STOP:
        break
      if fid == 1:
        if ftype == TType.STRING:
          self.program_name = iprot.readString();
        else:
          iprot.skip(ftype)
      elif fid == 2:
        if ftype == TType.STRING:
          self.config_str = iprot.readString();
        else:
          iprot.skip(ftype)
      else:
        iprot.skip(ftype)
      iprot.readFieldEnd()
    iprot.readStructEnd()

  def write(self, oprot):
    if oprot.__class__ == TBinaryProtocol.TBinaryProtocolAccelerated and self.thrift_spec is not None and fastbinary is not None:
      oprot.trans.write(fastbinary.encode_binary(self, (self.__class__, self.thrift_spec)))
      return
    oprot.writeStructBegin('p4_program_config_add_args')
    if self.program_name is not None:
      oprot.writeFieldBegin('program_name', TType.STRING, 1)
      oprot.writeString(self.program_name)
      oprot.writeFieldEnd()
    if self.config_str is not None:
      oprot.writeFieldBegin('config_str', TType.STRING, 2)
      oprot.writeString(self.config_str)
      oprot.writeFieldEnd()
    oprot.writeFieldStop()
    oprot.writeStructEnd()

  def validate(self):
    return


  def __hash__(self):
    value = 17
    value = (value * 31) ^ hash(self.program_name)
    value = (value * 31) ^ hash(self.config_str)
    return value

  def __repr__(self):
    L = ['%s=%r' % (key, value)
      for key, value in self.__dict__.iteritems()]
    return '%s(%s)' % (self.__class__.__name__, ', '.join(L))

  def __eq__(self, other):
    return isinstance(other, self.__class__) and self.__dict__ == other.__dict__

  def __ne__(self, other):
    return not (self == other)

class p4_program_config_add_result:
  """
  Attributes:
   - success
  """

  thrift_spec = (
    (0, TType.I32, 'success', None, None, ), # 0
  )

  def __init__(self, success=None,):
    self.success = success

  def read(self, iprot):
    if iprot.__class__ == TBinaryProtocol.TBinaryProtocolAccelerated and isinstance(iprot.trans, TTransport.CReadableTransport) and self.thrift_spec is not None and fastbinary is not None:
      fastbinary.decode_binary(self, iprot.trans, (self.__class__, self.thrift_spec))
      return
    iprot.readStructBegin()
    while True:
      (fname, ftype, fid) = iprot.readFieldBegin()
      if ftype == TType.STOP:
        break
      if fid == 0:
        if ftype == TType.I32:
          self.success = iprot.readI32();
        else:
          iprot.skip(ftype)
      else:
        iprot.skip(ftype)
      iprot.readFieldEnd()
    iprot.readStructEnd()

  def write(self, oprot):
    if oprot.__class__ == TBinaryProtocol.TBinaryProtocolAccelerated and self.thrift_spec is not None and fastbinary is not None:
      oprot.trans.write(fastbinary.encode_binary(self, (self.__class__, self.thrift_spec)))
      return
    oprot.writeStructBegin('p4_program_config_add_result')
    if self.success is not None:
      oprot.writeFieldBegin('success', TType.I32, 0)
      oprot.writeI32(self.success)
      oprot.writeFieldEnd()
    oprot.writeFieldStop()
    oprot.writeStructEnd()

  def validate(self):
    return


  def __hash__(self):
    value = 17
    value = (value * 31) ^ hash(self.success)
    return value

  def __repr__(self):
    L = ['%s=%r' % (key, value)
      for key, value in self.__dict__.iteritems()]
    return '%s(%s)' % (self.__class__.__name__, ', '.join(L))

  def __eq__(self, other):
    return isinstance(other, self.__class__) and self.__dict__ == other.__dict__

  def __ne__(self, other):
    return not (self == other)

class p4_program_config_delete_args:
  """
  Attributes:
   - program_name
  """

  thrift_spec = (
    None, # 0
    (1, TType.STRING, 'program_name', None, None, ), # 1
  )

  def __init__(self, program_name=None,):
    self.program_name = program_name

  def read(self, iprot):
    if iprot.__class__ == TBinaryProtocol.TBinaryProtocolAccelerated and isinstance(iprot.trans, TTransport.CReadableTransport) and self.thrift_spec is not None and fastbinary is not None:
      fastbinary.decode_binary(self, iprot.trans, (self.__class__, self.thrift_spec))
      return
    iprot.readStructBegin()
    while True:
      (fname, ftype, fid) = iprot.readFieldBegin()
      if ftype == TType.STOP:
        break
      if fid == 1:
        if ftype == TType.STRING:
          self.program_name = iprot.readString();
        else:
          iprot.skip(ftype)
      else:
        iprot.skip(ftype)
      iprot.readFieldEnd()
    iprot.readStructEnd()

  def write(self, oprot):
    if oprot.__class__ == TBinaryProtocol.TBinaryProtocolAccelerated and self.thrift_spec is not None and fastbinary is not None:
      oprot.trans.write(fastbinary.encode_binary(self, (self.__class__, self.thrift_spec)))
      return
    oprot.writeStructBegin('p4_program_config_delete_args')
    if self.program_name is not None:
      oprot.writeFieldBegin('program_name', TType.STRING, 1)
      oprot.writeString(self.program_name)
      oprot.writeFieldEnd()
    oprot.writeFieldStop()
    oprot.writeStructEnd()

  def validate(self):
    return


  def __hash__(self):
    value = 17
    value = (value * 31) ^ hash(self.program_name)
    return value

  def __repr__(self):
    L = ['%s=%r' % (key, value)
      for key, value in self.__dict__.iteritems()]
    return '%s(%s)' % (self.__class__.__name__, ', '.join(L))

  def __eq__(self, other):
    return isinstance(other, self.__class__) and self.__dict__ == other.__dict__

  def __ne__(self, other):
    return not (self == other)

class p4_program_config_delete_result:
  """
  Attributes:
   - success
  """

  thrift_spec = (
    (0, TType.I32, 'success', None, None, ), # 0
  )

  def __init__(self, success=None,):
    self.success = success

  def read(self, iprot):
    if iprot.__class__ == TBinaryProtocol.TBinaryProtocolAccelerated and isinstance(iprot.trans, TTransport.CReadableTransport) and self.thrift_spec is not None and fastbinary is not None:
      fastbinary.decode_binary(self, iprot.trans, (self.__class__, self.thrift_spec))
      return
    iprot.readStructBegin()
    while True:
      (fname, ftype, fid) = iprot.readFieldBegin()
      if ftype == TType.STOP:
        break
      if fid == 0:
        if ftype == TType.I32:
          self.success = iprot.readI32();
        else:
          iprot.skip(ftype)
      else:
        iprot.skip(ftype)
      iprot.readFieldEnd()
    iprot.readStructEnd()

  def write(self, oprot):
    if oprot.__class__ == TBinaryProtocol.TBinaryProtocolAccelerated and self.thrift_spec is not None and fastbinary is not None:
      oprot.trans.write(fastbinary.encode_binary(self, (self.__class__, self.thrift_spec)))
      return
    oprot.writeStructBegin('p4_program_config_delete_result')
    if self.success is not None:
      oprot.writeFieldBegin('success', TType.I32, 0)
      oprot.writeI32(self.success)
      oprot.writeFieldEnd()
    oprot.writeFieldStop()
    oprot.writeStructEnd()

  def validate(self):
    return


  def __hash__(self):
    value = 17
    value = (value * 31) ^ hash(self.success)
    return value

  def __repr__(self):
    L = ['%s=%r' % (key, value)
      for key, value in self.__dict__.iteritems()]
    return '%s(%s)' % (self.__class__.__name__, ', '.join(L))

  def __eq__(self, other):
    return isinstance(other, self.__class__) and self.__dict__ == other.__dict__

  def __ne__(self, other):
    return not (self == other)
