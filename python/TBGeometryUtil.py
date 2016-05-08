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
def nameonly(s):
    import posixpath
    return posixpath.splitext(posixpath.split(s)[1])[0]
#---------------------------------------------------------------------------
RED    ="\x1b[0;31;48m"
GREEN  ="\x1b[0;32;48m"
YELLOW ="\x1b[0;33;48m"
BLUE   ="\x1b[0;34;48m"
MAGENTA="\x1b[0;35;48m"
CYAN   ="\x1b[0;36;48m"

BOLDRED    ="\x1b[1;31;48m"
BOLDGREEN  ="\x1b[1;32;48m"
BOLDYELLOW ="\x1b[1;33;48m"
BOLDBLUE   ="\x1b[1;34;48m"
BOLDMAGENTA="\x1b[1;35;48m"
BOLDCYAN   ="\x1b[1;36;48m"

RESETCOLOR ="\x1b[0m"    # reset to default foreground color 

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
    geometry = nameonly(geometry)
    cmd = 'from HGCal.TBStandaloneSimulator.%s import  '\
        'World, Components, Geometry'\
        % geometry
    exec(cmd)

    tprev = 0.0
    layer = 0
    # location of first layer of first module (SamplingSection)
    x0 = 0.0
    y0 = 0.0
    z0 = 0.0
    z  = 0.0
    header = None
    geometry  = []
    for part in Geometry:
        # check for header
        if type(part) == type({}):
            header= copy(part)
            units = header['units']
            # convert to mm
            if   units == 'm':
                scale = 1000.0
            elif units == 'cm':
                scale = 10.0
            else:
                scale = 1.0
            x0 = scale*header['x']
            y0 = scale*header['y']
            z0 = scale*header['z']
            header['x'] = x0
            header['y'] = y0
            header['z'] = z0
            header['units'] = 'mm'
            continue

        print part
        comp = copy(Components[part])
        # check for modules (SamplingSections)
        if type(comp) == type([]):
            for ii, subpart in enumerate(comp):
                print '\t%s' % subpart
                component = copy(Components[subpart])
                t    = component['thickness']
                side = component['side']
                x  = component['x']
                y  = component['y']
                z += (t + tprev)/2
                tprev = t
                component['x'] = x0 + x
                component['y'] = y0 + y
                component['z'] = z0 + z

                if not component.has_key('sensitive'):
                    component['sensitive'] = False
                component['first'] = ii == 0
                component['last']  = ii == len(comp)-1
                geometry.append(component)
        else:
            t    = comp['thickness']
            side = comp['side']
            x  = comp['x']
            y  = comp['y']
            z += (t + tprev)/2
            tprev = t
            comp['x'] = x0 + x
            comp['y'] = y0 + y
            comp['z'] = z0 + z
            comp['sensitive'] = False
            comp['first'] = True
            comp['last']  = True
            geometry.append(comp)


    #from pprint import PrettyPrinter
    #pp = PrettyPrinter()
    #pp.pprint(geometry)

    # produce a map from sensitive layer number
    # to index into the geometry list
    sensitive = {}
    layer = 0
    for index, c in enumerate(geometry):
        if c['sensitive']:
            layer += 1
            sensitive[layer] = index

    return {'world'    : World,
            'header'   : header,  
            'geometry' : geometry, 
            'sensitive': sensitive}
#------------------------------------------------------------------------------
def divideCanvas(nlayers, canvas):
    n = int(sqrt(nlayers+1))
    xdiv = n
    ydiv = n
    if xdiv*ydiv < nlayers: ydiv += 1
    nplots = min(xdiv*ydiv, nlayers)
    canvas.Divide(xdiv, ydiv)
    return nplots    
#------------------------------------------------------------------------------
def main():
    # get test beam geometry
    geom, sensitive  = createGeometry(geometry="TBGeometry_2016_04")
    from pprint import PrettyPrinter
    pp = PrettyPrinter()
    pp.pprint(geom)

if __name__ == "__main__": main()
