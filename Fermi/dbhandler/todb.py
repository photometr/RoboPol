#!/usr/bin/python

import sys
import MySQLdb
import time

objname = sys.argv[1]

objects = {
  "BLLac":      {"dbname" : "BL Lac",      "spi" : 0.03569},#logpar
  "S51803+784": {"dbname" : "S5 1803+784", "spi" : 0.03046},
  "S41849+67":  {"dbname" : "S4 1849+67",  "spi" : 0.03737},#logpar
  }


class point():
  def __init__(self):
    self.MJD = 0
    self.Flux = 0
    self.Flux_Err = 0
    self.Index = 0
    self.IdxErr = 0
    self.UpperLimit = 0
    self.TS_VALUE = 0
    self.Prefactor = 0
    self.Pref_Err = 0
    self.T_START = 0
    self.T_STOP = 0
    self.NPRED = 0
  def setSpIndErr(self,spidxerr):
    if spidxerr < 1e-6:
      self.IdxErr = objects[objname]["spi"]

def Log(msg):
  fop = open(sys.argv[0]+'.log','a')
  fop.write(time.strftime("%d.%m %H:%M:%S ")+str(msg)+"\n")
  fop.close()

def parsefile():
  data = []
  fop = open(objname+".dat")
  for line in fop.readlines():
    p = point()
    if line.startswith("#"):
      continue
    if line.strip() == "":
      continue
    sl = line.split()
    if sl[2] == "ERROR!":
      continue
    if sl[6] == "Error":
      continue
    p.MJD        = float(sl[1])
    p.Flux       = float(sl[2])
    p.Flux_Err   = float(sl[3])
    p.Index      = float(sl[4])
    p.setSpIndErr( float(sl[5]) )
    if sl[6] == "Ignored":
      p.UpperLimit = None
    else:
      p.UpperLimit = float(sl[6])
    p.TS_VALUE   = float(sl[7])
    p.Prefactor  = float(sl[8])
    p.Pref_Err   = float(sl[9])
    p.T_START    = float(sl[10])
    p.T_STOP     = float(sl[11])
    p.NPRED      = float(sl[12])
    data.append(p)
  fop.close()
  return data

def GetBlazarIds(cursor):
  blazars = {}
  sql = "SELECT id,name FROM Fermi_objects"
  cursor.execute(sql)
  data = cursor.fetchall()
  for line in data:
    blazars[line[1]] = line[0]
  return blazars

def Replace(db,cursor,data,blazar_id):
  fields = []
  for p in data:
    fields.append((blazar_id,p.MJD,p.Flux,p.Flux_Err,p.Index,p.IdxErr,p.UpperLimit,p.TS_VALUE,p.Prefactor,p.Pref_Err,p.T_START,p.T_STOP,p.NPRED))

  #try:
    #sql = "DELETE FROM Fermi_data WHERE blazar_id = " + str(blazar_id) + " ;"
    #cursor.execute(sql)
  #except:
    #errmsg = "Smth wrong on delete of " + objname
    #Log(errmsg)
    #return 1

  try:
    cursor.executemany("""REPLACE INTO Fermi_data (blazar_id, jd, flux, flux_err, ind, index_err, upp_lim, ts_value, prefactor, pref_err, t_start, t_stop,npred) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)""", fields)
    db.commit()
  except:
    db.rollback()
    errmsg = "Smth wrong on update of " + objname
    Log(errmsg)
    return 1
  return 0

def writeToDB(data):
  db = MySQLdb.connect(host="localhost", user="", passwd="", db="", charset='latin1')
  cursor = db.cursor()
  blazars = GetBlazarIds(cursor)
  try:
    blazar_id = blazars[objects[objname]["dbname"]]
  except KeyError:
    errmsg = "Object " + objname + " is not in the DB"
    Log(errmsg)
    return 1
  Replace(db,cursor,data,blazar_id)

if __name__ == "__main__":
  data = parsefile()
  writeToDB(data)
