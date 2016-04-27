#-----------------------------------------------------------------------------
# File:        TBGeomtryUtil.py
# Description: TB 2016 simple HGC test beam geometry utilities
# Created:     April-2016 Harrison B. Prosper
#-----------------------------------------------------------------------------
import sys, os, re
from string import atof, lower, replace, strip, split, joinfields, find
from array import array
from math import *
from ROOT import *
#------------------------------------------------------------------------------
def computeBinVertices(side, cell):
    x0, y0, posid = cell.x, cell.y, cell.posid
    # construct (x,y) vertices of a hexagon or half-hexagon, 
    # centered at (x0,y0)
    S = float(side)
    H = S*sqrt(3.0)/2  # center to side distance
    x = array('d')
    y = array('d')
    if   posid == 0:
        x.append(x0-S/2); y.append(y0-H)
        x.append(x0-S);   y.append(y0)
        x.append(x0-S/2); y.append(y0+H)
        x.append(x0+S/2); y.append(y0+H)
        x.append(x0+S);   y.append(y0)
        x.append(x0+S/2); y.append(y0-H)
    elif posid == 1:
        x.append(x0-S/2); y.append(y0-H)
        x.append(x0-S);   y.append(y0)
        x.append(x0-S/2); y.append(y0+H)
        x.append(x0+S/2); y.append(y0-H)
    elif posid == 2:
        x.append(x0-S/2); y.append(y0-H)
        x.append(x0-S);   y.append(y0)
        x.append(x0+S);   y.append(y0)
        x.append(x0+S/2); y.append(y0-H)
    elif posid == 3:
        x.append(x0-S/2); y.append(y0-H)
        x.append(x0+S/2); y.append(y0+H)
        x.append(x0+S);   y.append(y0)
        x.append(x0+S/2); y.append(y0-H)
    elif posid == 4:
        x.append(x0-S/2); y.append(y0+H)
        x.append(x0+S/2); y.append(y0+H)
        x.append(x0+S);   y.append(y0)
        x.append(x0+S/2); y.append(y0-H)
    elif posid == 5:
        x.append(x0-S);   y.append(y0)
        x.append(x0-S/2); y.append(y0+H)
        x.append(x0+S/2); y.append(y0+H)
        x.append(x0+S);   y.append(y0)
    elif posid == 6:
        x.append(x0-S/2); y.append(y0-H)
        x.append(x0-S);   y.append(y0)
        x.append(x0-S/2); y.append(y0+H)
        x.append(x0+S/2); y.append(y0+H)    
    return (x, y)
#------------------------------------------------------------------------------
def computeHexVertices(side):
    # construct (x,y) vertices of a hexagon centered at the origin
    S = side
    H = S*sqrt(3)/2  # center to side distance
    x = array('d')
    y = array('d')
    x.append(-S/2); y.append(-H)
    x.append(-S);   y.append(0)
    x.append(-S/2); y.append( H)
    x.append( S/2); y.append( H)
    x.append( S);   y.append(0)
    x.append( S/2); y.append(-H)
    return (x, y)
#------------------------------------------------------------------------------
def computeSquareVertices(side):
    # construct (x,y) vertices of a hexagon centered at the origin
    S = side
    H = S/2  # center to side distance
    x = array('d')
    y = array('d')
    x.append(-H); y.append(-H)
    x.append(-H); y.append( H)
    x.append( H); y.append( H)
    x.append( H); y.append(-H)
    return (x, y)
#------------------------------------------------------------------------------
def createGeometry(geometry="TBGeometry_2016_04"):
    from copy import copy
    cmd = 'from HGCal.TBStandaloneSimulator.%s import Components, Geometry'\
        % geometry
    exec(cmd)

    tprev = 0.0
    layer = 0
    z = 0.0
    geometry  = []
    sensitive = {}
    for part in Geometry:
        print part
        comp = copy(Components[part])
        # check for modules
        if type(comp) == type([]):
            for subpart in comp:
                print '\t%s' % subpart
                component = copy(Components[subpart])
                t    = component['thickness']
                side = component['side']
                z += (t + tprev)/2
                component['z'] = z
                tprev = t
                if component.has_key('sensitive'):
                    layer += 1
                    component['layer'] = layer
                    sensitive[layer] = component
                geometry.append(component)
        else:
            t    = comp['thickness']
            side = comp['side']
            z += (t + tprev)/2
            comp['z'] = z
            tprev = t
            geometry.append(comp)
    return (geometry, sensitive)
#------------------------------------------------------------------------------
def main():
    # get test beam geometry
    geom, sensitive  = createGeometry(geometry="TBGeometry_2016_04")
    from pprint import PrettyPrinter
    pp = PrettyPrinter()
    pp.pprint(geom)

if __name__ == "__main__": main()
