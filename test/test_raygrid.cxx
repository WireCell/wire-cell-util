#include "WireCellUtil/RayGrid.h"

#include "WireCellUtil/Testing.h"

#include "TCanvas.h"
#include "TMarker.h"
#include "TText.h"
#include "TLine.h"

#include <iostream>
#include <string>

using namespace WireCell;
using namespace std;

void draw_ray(const Ray& ray, int color=1)
{
    TLine l;
    l.SetLineColor(color);
    l.DrawLine(ray.first.z(), ray.first.y(),
               ray.second.z(), ray.second.y());
}

void draw_point(const Point& p, float size=1, int style=20, int color=1);
void draw_point(const Point& p, float size, int style, int color)
{
    TMarker m;
    m.SetMarkerColor(color);
    m.SetMarkerSize(size);
    m.SetMarkerStyle(style);
    m.DrawMarker(p.z(), p.y());
}
void draw_text(const Point&p, std::string text)
{
    TText t;
    t.SetTextFont(82);
    t.SetTextSize(0.03);
    t.DrawText(p.z(), p.y(), text.c_str());
}
void draw_zero_crossing(const RayGrid& rg, RayGrid::rccs_index_t il, RayGrid::rccs_index_t im)
{
    auto p = rg.zero_crossing(il, im);
    draw_point(p, 1, 24);

    const auto& cs = rg.centers();
    const auto& c1 = cs[il];
    const auto& c2 = cs[im];
    draw_point(c1, 0.5, 20, 2);
    draw_point(c2, 0.5, 20, 4);

    const auto& rjs = rg.ray_jumps();
    const auto& j1 = rjs(il,im);
    const auto& j2 = rjs(im,il);

    draw_ray(Ray(c1, c1+j1), 2);
    draw_ray(Ray(c2, c2+j2), 4);

    std::stringstream ss;
    ss << "(" << il << "," << im << ")";

    draw_text(p, ss.str());
}


void draw_pairs(const RayGrid::ray_pair_vector_t& raypairs)
{
    for (const auto& rp : raypairs) {
        draw_ray(rp.first);
        draw_ray(rp.second);
    }
}

void draw(std::string fname, const RayGrid& rg, const RayGrid::ray_pair_vector_t& raypairs)
{

    TCanvas canvas("test_raygrid","Ray Grid", 500, 500);
    auto draw_print = [&](std::string extra="") { canvas.Print((fname + extra).c_str(), "pdf"); };

    auto draw_frame = [&]() { canvas.DrawFrame(-10,-10,110,110);};
    
    draw_print("[");

    const size_t nbounds = raypairs.size();
    for (RayGrid::rccs_index_t il=0; il < nbounds; ++il) {
        for (RayGrid::rccs_index_t im=0; im < nbounds; ++im) {
            if (il < im) {
                draw_frame();
                draw_pairs(raypairs);
                draw_zero_crossing(rg, il, im);

                for (RayGrid::grid_index_t ip = 0; ip < 10; ++ip) {
                    for (RayGrid::grid_index_t jp = 0; jp < 10; ++jp) {
                        RayGrid::ray_address_t one{il, ip}, two{im, jp};
                        auto p = rg.ray_crossing(one, two);
                        draw_point(p, 1, 7);
                        cout << p<< " " << one.rccs << ":" << one.grid << " , " << two.rccs << ":" << two.grid <<  endl;
                    }
                }

                draw_print();
            }

            for (size_t in=0; in < nbounds; ++in) {
            }
        }
    }

    draw_print("]");
}


int main(int argc, char *argv[])
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

    RayGrid rg(raypairs);

    Assert(rg.nrccs() == raypairs.size());
    
    for (size_t ind=0; ind<rg.nrccs(); ++ind) {
        cout << ind
             << " r1=" << raypairs[ind].first
             << " r2=" << raypairs[ind].second
             << " p=" << rg.pitch_mags().at(ind) << " " << rg.pitch_dirs().at(ind)
             << " c=" << rg.centers().at(ind) << endl;
    }

    std::string fname = argv[0];
    fname += ".pdf";
    draw(fname, rg, raypairs);
    return 0;

}

