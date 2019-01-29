// include me in tests

RayGrid::ray_pair_vector_t make_raypairs(double width=100, double height=100, double pitch_mag = 5);
RayGrid::ray_pair_vector_t make_raypairs(double width, double height, double pitch_mag)
{
    RayGrid::ray_pair_vector_t raypairs;

    // corners
    const Vector ll(0,0,0), lr(0,0,width), ul(0,height,0), ur(0,height,width);

    // wire directions
    const Vector eckx(1,0,0);
    const Vector why(0,1,0);
    const Vector zee(0,0,1);

    // /-wires
    const Vector du = (ur - ll).norm();
    const Vector pu = eckx.cross(du); 
    cerr << "pu=" << pu << endl;
    const Ray rayu0(ul + why*(why.dot(0.5*pitch_mag*pu)),
                    ul + zee*(zee.dot(0.5*pitch_mag*pu)));
    const Ray rayu1(ul + why*(why.dot(1.5*pitch_mag*pu)),
                    ul + zee*(zee.dot(1.5*pitch_mag*pu)));
    
    // \-wires
    const Vector dv = (ul - lr).norm();    
    const Vector pv = eckx.cross(dv);
    cerr << "pv=" << pv << endl;
    const Ray rayv0(ll + why*(why.dot(0.5*pitch_mag*pv)),
                    ll + zee*(zee.dot(0.5*pitch_mag*pv)));
    const Ray rayv1(ll + why*(why.dot(1.5*pitch_mag*pv)),
                    ll + zee*(zee.dot(1.5*pitch_mag*pv)));


    // |-wires
    const Vector dw = why;
    const Vector pw = zee;
    const Vector cw = 0.5*(ul + ll);
    const Ray rayw0(ll, ul);
    const Ray rayw1(ll + pitch_mag*pw, ul+pitch_mag*pw);    


    // horizontal bounds
    raypairs.push_back(make_pair( Ray(ll, lr), Ray(ul, ur) ));

    // vertical bounds
    raypairs.push_back(make_pair( Ray(ll, ul), Ray(lr, ur) ));

    // pitch1
    raypairs.push_back(make_pair( rayu0, rayu1) );

    // pitch2
    raypairs.push_back(make_pair( rayv0, rayv1) );

    // pitch3
    raypairs.push_back(make_pair( rayw0, rayw1) );
    return raypairs;
}
