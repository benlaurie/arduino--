import pysimulavr

class SimulavrAdapter(object):

  DEFAULT_CYCLE_LENGTH = 62 # 62ns or about 16MHz
  
  def loadDevice(self, t, e, cycleLength = DEFAULT_CYCLE_LENGTH):
    pysimulavr.DumpManager.Instance().SetSingleDeviceApp()
    self.__sc = pysimulavr.SystemClock.Instance()
    self.__sc.ResetClock()
    dev = pysimulavr.AvrFactory.instance().makeDevice(t)
    dev.Load(e)
    self.cycleLength = cycleLength
    dev.SetClockFreq(cycleLength)
    self.__sc.Add(dev)
    return dev

  def time(self):
    return self.__sc.GetCurrentTime()

  def doRun(self, n = None):
    ct = self.__sc.GetCurrentTime
    while n is None or ct() < n:
      res = self.__sc.Step()
      if res:
        return res
    return 0

  def doStep(self, stepcount = 1):
    while stepcount > 0:
      res = self.__sc.Step()
      if res is not 0: return res
      stepcount -= 1
    return 0
    
  def getCurrentTime(self):
    return self.__sc.GetCurrentTime()
    
  def getAllRegisteredTraceValues(self):
    os = pysimulavr.ostringstream()
    pysimulavr.DumpManager.Instance().save(os)
    return filter(None, [i.strip() for i in os.str().split("\n")])

  def dmanStart(self):
    pysimulavr.DumpManager.Instance().start()
  
  def Reset(self):
    pysimulavr.DumpManager.Instance().stopApplication()
    pysimulavr.DumpManager.Instance().Reset()
    self.__sc.ResetClock()
  
  def setVCDDump(self, vcdname, signals, rstrobe = False, wstrobe = False):
    dman = pysimulavr.DumpManager.Instance()
    sigs = ["+ " + i for i in signals]
    dman.addDumpVCD(vcdname, "\n".join(sigs), "ns", rstrobe, wstrobe)
    
  def getWordByName(self, dev, label):
    addr = dev.data.GetAddressAtSymbol(label)
    v = dev.getRWMem(addr)
    addr += 1
    v = (dev.getRWMem(addr) << 8) + v
    return v
