#!/usr/bin/python
import sys, getopt, numpy

def trunc(f, n):
    '''Truncates/pads a float f to n decimal places without rounding'''
    return ('%.*f' % (n + 1, f))[:-1]

def main(argv):
   inputfile = ''
   outputfile = ''
   try:
      opts, args = getopt.getopt(argv,"hi:",["ifile="])
   except getopt.GetoptError:
      print 'test.py -i <inputfile>'
      sys.exit(2)
   for opt, arg in opts:
      if opt == '-h':
         print 'test.py -i <inputfile>'
         sys.exit()
      elif opt in ("-i", "--ifile"):
         inputfile = arg

   with open(inputfile) as f:
    array = []
    for line in f:
       array.append([int(x) for x in line.split()])

    avg=trunc(numpy.average(array,axis=0), 4)
    std=trunc(numpy.std(array,axis=0), 4)
    
    print 'Average: '+str(avg)+'   Standard deviation: '+str(std)

if __name__ == "__main__":
   main(sys.argv[1:])
