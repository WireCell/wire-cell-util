#!/usr/bin/env python
'''
A tvtk based converter for JSON file dumped from tests

'''
import math
import json
import numpy
from tvtk.api import tvtk, write_data
from tvtk.common import configure_input


def orderpoints(pointset):
    c = [0.0,0.0,0.0]
    for p in pointset:
        for i in range(3):
            c[i] += p[i]
    n = len(pointset)
    for i in range(3):
        c[i] /= n

    byang = list()
    for p in pointset:
        ang = math.atan2(p[2]-c[2], p[1]-c[1]);
        byang.append((ang, p))
    byang.sort()
    return [p for a,p in byang]

def extrude(pts, dx):
    '''
    make a 3d set of cells based on ring of pts extruded along X axis by dx

    Return points and "relative cells"


    '''
    pts2 = [ [pt[0]+dx,pt[1],pt[2]] for pt in pts] # the other face
    all_pts = pts + pts2

    n = len(pts)
    top_cell = range(n)
    bot_cell = range(n, 2*n)
    cells = [top_cell, bot_cell]

    # enumerate the sides
    for ind in range(n):
        ind2 = (ind+1)%n
        cell = [top_cell[ind], top_cell[ind2], bot_cell[ind2], bot_cell[ind]]
        cells.append(cell)

    return all_pts, cells
        


def ugrid_blobs(blobs):
    '''
    Given a data structure which is a list of blobs, each blob is a dict:

    { 
      points=[[x1,y1,z1], [x2,y2,z2],...], 
      values=dict(name1=val1, name2=val2), 
      thickness=1.0, # optional
    }

    return an unstructured grid
    '''
    datasetnames = set()
    all_points = list()
    blob_cells = []
    for blob in blobs:
        myvalnames = list(blob['values'].keys())
        datasetnames = datasetnames.union(myvalnames)
        pts = orderpoints(blob['points'])
        pts,cells = extrude(pts, blob.get('thickness', 1.0))
        all_points += pts
        blob_cells.append((len(pts),cells))

    ugrid = tvtk.UnstructuredGrid(points=all_points)

    ptype = tvtk.Polyhedron().cell_type

    offset = 0
    for npts,cells in blob_cells:
        cell_ids = [len(cells)]
        for cell in cells:
            cell_ids.append(len(cell))
            cell_ids += [offset+cid for cid in cell]
        ugrid.insert_next_cell(ptype, cell_ids)
        offset += npts

    ugrid.cell_data.scalars = list(range(len(blobs)))
    ugrid.cell_data.scalars.name = "indices"

    print (datasetnames)
    narrays = 1
    for datasetname in sorted(datasetnames):
        arr = [b["values"].get(datasetname, 0.0) for b in blobs]
        ugrid.cell_data.add_array(arr)
        ugrid.cell_data.get_array(narrays).name = datasetname
        narrays += 1

    return ugrid

def main(infile, outfile):
    jblobs = json.loads(open(infile).read())
    #print (jblobs)
    ugrid = ugrid_blobs(jblobs)
    w = tvtk.UnstructuredGridWriter(file_name=outfile)
    configure_input(w, ugrid)
    w.write()
    

def test(filename = "blob2tvtk_test.vtk"):
    points = numpy.asarray([
        [0.0,0.0,0.0],
        [0.0,0.0,1.0],
        [0.0,1.0,1.0],
        [0.0,1.0,0.0],

        [1.0,0.0,0.0],
        [1.0,0.0,1.0],
        [1.0,1.0,1.0],
        [1.0,1.0,0.0],

    ])

    ugrid = tvtk.UnstructuredGrid(points=points)

    # #faces,  #pts, pt1, pt2, ...  #pts, pt1, pt2, ..., ...
    pt_ids = [6]
    pt_ids.append(4)
    pt_ids += list(range(4))
    pt_ids.append(4)
    pt_ids += list(range(4,8))
    for ind in range(4):
        ind2 = (ind+1)%4
        pt_ids.append(4)
        pt_ids += [ind, ind2, 4+ind2, 4+ind]
    print (pt_ids)

    ptype = tvtk.Polyhedron().cell_type
    ugrid.insert_next_cell(ptype, pt_ids)

    ugrid.cell_data.scalars = [1.0]
    ugrid.cell_data.scalars.name = "unity"
    # exercise multiple values
    ugrid.cell_data.add_array([2.0])
    ugrid.cell_data.get_array(1).name = "duality"
    ugrid.cell_data.add_array([3.0])
    ugrid.cell_data.get_array(2).name = "trinity"
    #w = tvtk.XMLUnstructuredGridWriter(file_name=filename)
    w = tvtk.UnstructuredGridWriter(file_name=filename)
    configure_input(w, ugrid)
    w.write()
    
if '__main__' == __name__:
    import sys
    main(*sys.argv[1:])

    
