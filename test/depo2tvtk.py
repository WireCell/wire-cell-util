#!/usr/bin/env python
'''
Convert a Numpy file produced by NumpyDepoSaver into a vtk file which
can be visualized with paraview.
'''
import os
import math
import numpy
from wirecell import units
from collections import defaultdict
from tvtk.api import tvtk, write_data
from tvtk.common import configure_input

import rendertvtk;

    
def main(infile, outname=None, toffset=1*units.us, speed=1.6*units.mm/units.us):
    if outname is None or outname == "-":
        outname = os.path.splitext(infile)[0]

    if type(speed) == type(""):
        speed = eval(speed)
    if type(toffset) == type(""):
        toffset = eval(toffset)


    print("toffset=%f us, speed=%f mm/us" %
          (toffset/units.us, speed/(units.mm/units.us)))

    arrs = numpy.load(infile)
    depos = arrs['depo_data_0']


    # depos is a 7xN array
    # (t,q,x,y,z,dl,dt)

    # 
    # pack into array of : dict(
    #   point= [x,y,z],
    #   values=dict(name1=val1, name2=val2, ...)
    # )

    points = list()
    for one in depos.T:
        t,q,x,y,z,dl,dt = one
        rx = x + (t-toffset)*speed
        # size=(dl,dt,dt)
        p = dict(point=[-rx,y,z], values=dict(time=t,charge=q))
        points.append(p)

    pointdata = rendertvtk.points(points)
    ofile = outname + "-depos.vtp"
    print(ofile)
    write_data(pointdata, ofile)


if '__main__' == __name__:
    import sys
    main(*sys.argv[1:])

    
