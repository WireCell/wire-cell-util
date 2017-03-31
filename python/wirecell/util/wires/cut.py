
class Wire(namedtuple("Wire","ident plane face channel segment tail head")):
    '''
    A wire segment.

    :param int ident: numerical identifier unique to this wire in the context of the detector.
    :param int face: the anode face (0 for "front" and 1 for "back").
    :param int channel: the numerical identifier for the channel to which this wire contributes.
    :param int segment: the number of wire segments between this one and the input to the conductor.
    :param triple tail: the (x y z) face-based coordinates of the starting point of the wire.
    :param triple head: the (x y z) face-based coordinates of the ending point of the wire.
    '''

class Plane(namedtuple("Plane","ident wires")):
    '''
    A "plane" here holds the wires making up the wire plane and info about the channel plane.

    The channel plane is identified with the that of all segment-zero wires.

    :param int ident: numerical identifier unique to this plane in the context of the detector.
    :param list wires: the wires in this plane

    :param int anode: logical anode plane number (aka APA number).


    '''

..................................












class Point(namedtuple("Point", "x y z")):
    '''
    A point in 3 space.

    Each point is in terms of the coordinate system (as described
    below for each coordinate) of its drift cell.  This means that for
    double-sided anode planes one must understand the context in which
    the point is referenced to know where a point is in some external
    coordinate system in which both drift cells are related.

    :param float x: Measure along the X-axis which points counter to
        the electron drift direction.
    :param float y: Measure along the Y-axis which points counter to
        the force of gravity.
    :param float z: Measure along the Z-axis which is X cross Y.
    '''
    __slots__ = ()


class Wire(namedtuple("Wire","ident tail head")):
    '''
    A Wire object holds information about one physical wire segment.

    A wire is a ray which points in the direction that signals flow
    toward the electronics.

    :param int ident: numerical identifier unique to this wire in the
        anode plane.
    :param int tailid: reference to the tail end point of the wire in
        the coordinate system for the anode face.
    :param int headid: reference to the head end point of the wire in
        the coordinate system for the anode face.
    '''
    __slots__ = ()


class Conductor(namedtuple("Conductor","ident chid segids")):
    '''
    A Conductor object associates an ordered list of wire segments
    with a electronics channel.

    :param int ident: numerical identifier unique to this conductor in
        the anode plane.
    :param int chid: numerical identifier for the electronics channel
        to which the conductor is connected.  This number has no
        specific semantic meaning to the toolkit.  However it will be
        passed to some components to identify a channel in some
        components so may be used by the implementations to look up
        related properties.
    :param list segids: list of Wire.ident ordered by increasing
        distance from the electronics.  That is, first segment is
        directly connected to the import of the channel.
    '''
    __slots__ = ()


class WirePlane(namedtuple("WirePlane", "ident wireids")):
    '''
    A WirePlane object collects the wires that make up one physical
    plane.

    :param int ident: numerical identifier unique to the anode plane.
    :param list wireids: list of Wire.ident making up the plane.
    '''
    __slots__ = ()


class ConductorPlane(namedtuple("ConductorPlane", "ident conductorids")):
    '''
    A ConductorPlane object collects the conductors that begin one
    physical plane.

    :param int ident: numerical identifier unique to the anode plane.
    :param list conductorids: list of Conductor.ident making up the
        plane.
    '''
    __slots__ = ()
    

class AnodeFace(namedtuple("AnodeFace", "ident wpids cpids")):
    '''
    An AnodeFace collects the wire and conductor planes making up one
    face of an anode plane.

    :param int ident: numerical identifier unique to the anode plane.
    :param list wpids: list of WirePlane.ident making up this face.
    :param list cpids: list of ConductorPlane.ident making up this
        face.
    '''
    __slots__ = ()


class AnodePlane(namedtuple("AnodePlane","ident faceids")):
    '''
    An AnodePlane object collects together (usually just one or two)
    AnodeFaces.

    An anode plane is the "APA" of DUNE.  It includes all wires,
    conductors and channels.

    :param int ident: some opaque numerical identifier for this anode
        plane.
    :param list faceids: list of AnodeFace.ident numbers making up
        this anode plane.  The first face listed is considered the
        "front" face.  If a second face is given it is considered the
        "back" face.  No other semantic interpretation of these terms
        is specified here.
    '''
    __slots__ = ()    


class Store(namedtuple("Store","aps afs cps wps conductors wires points")):
    '''
    A store of collections of the objects of this schema.

    :param list aps: list of the AnodePlane objects.
    :param list afs: list of the AnodeFace objects
    :param list cps: list of the ConductorPlane objects.
    :param list pws: list of the WirePlane objects.
    :param list conductors: list of the Conductor objects.
    :param list wires: list of the Wire objects.
    :param list points: list of the Point objects.

    '''
    __slots__ = ()

