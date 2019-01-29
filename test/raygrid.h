// include me in tests

static
RayGrid::ray_pair_vector_t make_raypairs()
{
    RayGrid::ray_pair_vector_t raypairs;

    // corners
    Vector ll(0,0,0), lr(0,0,100), ul(0,100,0), ur(0,100,100);
    

    // horizontal bounds
    raypairs.push_back(make_pair( Ray(ll, lr), Ray(ul, ur) ));

    // vertical bounds
    raypairs.push_back(make_pair( Ray(ll, ul), Ray(lr, ur) ));

    // pitch1
    Vector p1(0, 10, 10);
    raypairs.push_back(make_pair( Ray(ul, lr), Ray(ul+p1, lr+p1) ));

    // pitch2
    Vector p2(0, -10, 10);
    raypairs.push_back(make_pair( Ray(ll, ur), Ray(ll+p2, ur+p2) ));

    // pitch3
    Vector p3(0, 0, 10);
    raypairs.push_back(make_pair( Ray(0.5*(ll+lr), 0.5*(ul+ur)), Ray(0.5*(ll+lr)+p3, 0.5*(ul+ur)+p3)));
    return raypairs;
}
