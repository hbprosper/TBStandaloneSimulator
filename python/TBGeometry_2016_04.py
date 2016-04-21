#--------------------------------------------------------------------------
# List all components from which geometry is built.
# (x, y, z) is the location of the center of the component
#--------------------------------------------------------------------------
Components = {'W21': {'shape': 'square',
                      'material': 'W',
                      'length units': 'mm',
                      'side': 72.0,
                      'thickness': 2.1,
                      'x': 0.0,
                      'y': 0.0,
                      'z': 0.0},
              
              'W42':  {'shape': 'square',
                       'material': 'W',
                       'length units': 'mm',
                       'side': 72.0,
                       'thickness': 4.2,
                       'x': 0.0,
                       'y': 0.0,
                       'z': 0.0},

              'Cu60': {'shape': 'hexagon',
                       'material': 'Cu',
                       'length units': 'mm',
                       'side': 71.4598,
                       'thickness': 6.0,
                       'x': 0.0,
                       'y': 0.0,
                       'z': 0.0},

              'WCu06': {'shape': 'hexagon',
                        'material': 'WCu',
                        'length units': 'mm',
                        'side': 71.4598,
                        'thickness': 0.6,
                        'x': 0.0,
                        'y': 0.0,
                        'z': 0.0},

              'Air30': {'shape': 'square',
                        'material': 'Air',
                        'length units': 'mm',
                        'side': 71.4598,
                        'thickness': 3.0,
                        'x': 0.0,
                        'y': 0.0,
                        'z': 0.0},

              'Air60': {'shape': 'square',
                        'material': 'Air',
                        'length units': 'mm',
                        'side': 71.4598,
                        'thickness': 6.0,
                        'x': 0.0,
                        'y': 0.0,
                        'z': 0.0},
              
              'Kapton':{'shape': 'hexagon',
                        'material': 'Air',
                        'length units': 'mm',
                        'side': 71.4598,
                        'thickness': 0.01,
                        'x': 0.0,
                        'y': 0.0,
                        'z': 0.0},

              'Si020': {'shape': 'hexagon',
                        'material': 'Si',
                        'length units': 'mm',
                        'sensitive': True,
                        'side': 71.4598,
                        'cellside': 6.496345,
                        'thickness': 0.20,
                        'x': 0.0,
                        'y': 0.0,
                        'z': 0.0},

              'Si012': {'shape': 'hexagon',
                        'material': 'Si',
                        'length units': 'mm',
                        'side': 71.4598,
                        'thickness': 0.12,
                        'x': 0.0,
                        'y': 0.0,
                        'z': 0.0},

              'module1': ['WCu06',
                          'Cu60',
                          'WCu06',
                          'Kapton',
                          'Si020',
                          'Si012']              
              }
#--------------------------------------------------------------------------
# Geometry is specified as an ordered list of components
#--------------------------------------------------------------------------
Geometry=['W42',
          'Air60',

          'W42',
          'Air60',

          'W42',
          'Air60',

          'W21',
          'Air60',

          'W21',
          'Air60',

          'W21',
          'Air30',

          'module1']
