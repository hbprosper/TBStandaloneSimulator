#--------------------------------------------------------------------------
# Basic assumption: a test beam detector can be modeled as a sequence of
# elements along the z axis (beam axis).
#--------------------------------------------------------------------------
# List all components from which a test beam geometry can be built.
# (x, y, z) is the location of the center of the component
# Then specify the geometry using these components.
#--------------------------------------------------------------------------
Components = {'W21': {'shape': 'square',
                      'material': 'W',
                      'units': 'mm',
                      'side': 140.0,
                      'thickness': 2.1,
                      'x': 0.0,
                      'y': 0.0,
                      'z': 0.0},
              
              'W42':  {'shape': 'square',
                       'material': 'W',
                       'units': 'mm',
                       'side': 140.0,
                       'thickness': 4.2,
                       'x': 0.0,
                       'y': 0.0,
                       'z': 0.0},

              'Cu60': {'shape': 'hexagon',
                       'material': 'Cu',
                       'units': 'mm',
                       'side': 71.4598,
                       'thickness': 6.0,
                       'x': 0.0,
                       'y': 0.0,
                       'z': 0.0},

              'WCu06': {'shape': 'hexagon',
                        'material': 'WCu',
                        'units': 'mm',
                        'side': 71.4598,
                        'thickness': 0.6,
                        'x': 0.0,
                        'y': 0.0,
                        'z': 0.0},

              'Air30': {'shape': 'square',
                        'material': 'Air',
                        'units': 'mm',
                        'side': 71.4598,
                        'thickness': 3.0,
                        'x': 0.0,
                        'y': 0.0,
                        'z': 0.0},

              'Air60': {'shape': 'square',
                        'material': 'Air',
                        'units': 'mm',
                        'side': 71.4598,
                        'thickness': 6.0,
                        'x': 0.0,
                        'y': 0.0,
                        'z': 0.0},
              
              'Kapton':{'shape': 'hexagon',
                        'material': 'Air',
                        'units': 'mm',
                        'side': 71.4598,
                        'thickness': 0.01,
                        'x': 0.0,
                        'y': 0.0,
                        'z': 0.0},

              'Si020': {'shape': 'hexagon',
                        'material': 'Si',
                        'units': 'mm',
                        'sensitive': True,
                        'side': 71.4598,
                        'cellsize': 6.496345,
                        'thickness': 0.20,
                        'x': 0.0,
                        'y': 0.0,
                        'z': 0.0},

              'Si012': {'shape': 'hexagon',
                        'material': 'Si',
                        'units': 'mm',
                        'side': 71.4598,
                        'thickness': 0.12,
                        'x': 0.0,
                        'y': 0.0,
                        'z': 0.0},

              # These are mapped to SamplingSections in the standalone code
              'module2016_04': ['WCu06',
                                'Cu60',
                                'WCu06',
                                'Kapton',
                                'Si020',
                                'Si012'],

              'W42_Air60': ['W42',
                            'Air60'],

              'W21_Air60': ['W21',
                            'Air60'],

              'W21_Air30': ['W21',
                            'Air30']              
              }
#--------------------------------------------------------------------------
# World defines the overarching volume within which is placed a detector
# world volume, which in turn contains the detector.
#--------------------------------------------------------------------------
World = {'shape': 'box',
         'units': 'm',
         'xside': 0.5,
         'yside': 0.5,
         'zside': 1.0
         }

#--------------------------------------------------------------------------
# Geometry is specified as an ordered list of elements.
# An element is an instance of a component.
#--------------------------------------------------------------------------
# The Geometry block starts with a header containing the
# model, version number, and the desired location of the first 
# layer of the first element.
#
# It can also contain info specific to the model.

Geometry=[{'model':   5,
           'version': 1,
           'units': 'cm',
           'x':   0.0,   
           'y':   0.0,
           'z':  10.0
           },

          'W42_Air60',
          'W42_Air60',
          'W42_Air60',

          'W21_Air60',
          'W21_Air60',
          'W21_Air30',

          'module2016_04']
