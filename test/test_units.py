#!/usr/bin/env python

def test_import():
    import ROOT

    myunits = ROOT.MyUnits
    myunits.dummy()
    print 'MyUnits::yyy=', myunits.yyy
    print 'MyUnits::yyy*1=', myunits.yyy*1.0

    units = ROOT.units
    units.dummy()
    print 'units=',units
    print 'xxx=',units.xxx
    print 'xxx=',units.xxx*1.0
    print 1.0*units.pi

if '__main__' == __name__:
    test_import()
