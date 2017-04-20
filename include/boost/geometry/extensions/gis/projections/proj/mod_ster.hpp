#ifndef BOOST_GEOMETRY_PROJECTIONS_MOD_STER_HPP
#define BOOST_GEOMETRY_PROJECTIONS_MOD_STER_HPP

// Boost.Geometry - extensions-gis-projections (based on PROJ4)
// This file is automatically generated. DO NOT EDIT.

// Copyright (c) 2008-2015 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2017.
// Modifications copyright (c) 2017, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// This file is converted from PROJ4, http://trac.osgeo.org/proj
// PROJ4 is originally written by Gerald Evenden (then of the USGS)
// PROJ4 is maintained by Frank Warmerdam
// PROJ4 is converted to Boost.Geometry by Barend Gehrels

// Last updated version of proj: 4.9.1

// Original copyright notice:

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#include <boost/geometry/util/math.hpp>
#include <boost/math/special_functions/hypot.hpp>

#include <boost/geometry/extensions/gis/projections/impl/base_static.hpp>
#include <boost/geometry/extensions/gis/projections/impl/base_dynamic.hpp>
#include <boost/geometry/extensions/gis/projections/impl/projects.hpp>
#include <boost/geometry/extensions/gis/projections/impl/factory_entry.hpp>
#include <boost/geometry/extensions/gis/projections/impl/aasincos.hpp>
#include <boost/geometry/extensions/gis/projections/impl/pj_zpoly1.hpp>

namespace boost { namespace geometry { namespace projections
{
    struct mil_os {};
    struct lee_os {};
    struct gs48 {};
    struct alsk {};
    struct gs50 {};

    #ifndef DOXYGEN_NO_DETAIL
    namespace detail
    {
        namespace mod_ster
        {

            static const double EPSLN = 1e-10;

            struct par_mod_ster
            {
                COMPLEX    *zcoeff;
                double    cchio, schio;
                int        n;
            };

            /* based upon Snyder and Linck, USGS-NMD */

            // template class, using CRTP to implement forward/inverse
            template <typename CalculationType, typename Parameters>
            struct base_mod_ster_ellipsoid : public base_t_fi<base_mod_ster_ellipsoid<CalculationType, Parameters>,
                     CalculationType, Parameters>
            {

                typedef CalculationType geographic_type;
                typedef CalculationType cartesian_type;

                par_mod_ster m_proj_parm;

                inline base_mod_ster_ellipsoid(const Parameters& par)
                    : base_t_fi<base_mod_ster_ellipsoid<CalculationType, Parameters>,
                     CalculationType, Parameters>(*this, par) {}

                // FORWARD(e_forward)  ellipsoid
                // Project coordinates from geographic (lon, lat) to cartesian (x, y)
                inline void fwd(geographic_type& lp_lon, geographic_type& lp_lat, cartesian_type& xy_x, cartesian_type& xy_y) const
                {
                    double sinlon, coslon, esphi, chi, schi, cchi, s;
                    COMPLEX p;

                    sinlon = sin(lp_lon);
                    coslon = cos(lp_lon);
                    esphi = this->m_par.e * sin(lp_lat);
                    chi = 2. * atan(tan((geometry::math::half_pi<double>() + lp_lat) * .5) *
                        pow((1. - esphi) / (1. + esphi), this->m_par.e * .5)) - geometry::math::half_pi<double>();
                    schi = sin(chi);
                    cchi = cos(chi);
                    s = 2. / (1. + this->m_proj_parm.schio * schi + this->m_proj_parm.cchio * cchi * coslon);
                    p.r = s * cchi * sinlon;
                    p.i = s * (this->m_proj_parm.cchio * schi - this->m_proj_parm.schio * cchi * coslon);
                    p = pj_zpoly1(p, this->m_proj_parm.zcoeff, this->m_proj_parm.n);
                    xy_x = p.r;
                    xy_y = p.i;
                }

                // INVERSE(e_inverse)  ellipsoid
                // Project coordinates from cartesian (x, y) to geographic (lon, lat)
                inline void inv(cartesian_type& xy_x, cartesian_type& xy_y, geographic_type& lp_lon, geographic_type& lp_lat) const
                {
                    int nn;
                    COMPLEX p, fxy, fpxy, dp;
                    double den, rh = 0, z, sinz = 0, cosz = 0, chi, phi = 0, dphi, esphi;

                    p.r = xy_x;
                    p.i = xy_y;
                    for (nn = 20; nn ;--nn) {
                        fxy = pj_zpolyd1(p, this->m_proj_parm.zcoeff, this->m_proj_parm.n, &fpxy);
                        fxy.r -= xy_x;
                        fxy.i -= xy_y;
                        den = fpxy.r * fpxy.r + fpxy.i * fpxy.i;
                        dp.r = -(fxy.r * fpxy.r + fxy.i * fpxy.i) / den;
                        dp.i = -(fxy.i * fpxy.r - fxy.r * fpxy.i) / den;
                        p.r += dp.r;
                        p.i += dp.i;
                        if ((fabs(dp.r) + fabs(dp.i)) <= EPSLN)
                            break;
                    }
                    if (nn) {
                        rh = boost::math::hypot(p.r, p.i);
                        z = 2. * atan(.5 * rh);
                        sinz = sin(z);
                        cosz = cos(z);
                        lp_lon = this->m_par.lam0;
                        if (fabs(rh) <= EPSLN) {
                            lp_lat = this->m_par.phi0;
                            return;
                        }
                        chi = aasin(cosz * this->m_proj_parm.schio + p.i * sinz * this->m_proj_parm.cchio / rh);
                        phi = chi;
                        for (nn = 20; nn ;--nn) {
                            esphi = this->m_par.e * sin(phi);
                            dphi = 2. * atan(tan((geometry::math::half_pi<double>() + chi) * .5) *
                                pow((1. + esphi) / (1. - esphi), this->m_par.e * .5)) - geometry::math::half_pi<double>() - phi;
                            phi += dphi;
                            if (fabs(dphi) <= EPSLN)
                                break;
                        }
                    }
                    if (nn) {
                        lp_lat = phi;
                        lp_lon = atan2(p.r * sinz, rh * this->m_proj_parm.cchio * cosz - p.i *
                            this->m_proj_parm.schio * sinz);
                    } else
                        lp_lon = lp_lat = HUGE_VAL;
                }

                static inline std::string get_name()
                {
                    return "mod_ster_ellipsoid";
                }

            };

            template <typename Parameters>
            void setup(Parameters& par, par_mod_ster& proj_parm)  /* general initialization */
            {
                double esphi, chio;

                if (par.es) {
                    esphi = par.e * sin(par.phi0);
                    chio = 2. * atan(tan((geometry::math::half_pi<double>() + par.phi0) * .5) *
                        pow((1. - esphi) / (1. + esphi), par.e * .5)) - geometry::math::half_pi<double>();
                } else
                    chio = par.phi0;
                proj_parm.schio = sin(chio);
                proj_parm.cchio = cos(chio);
            }


            // Miller Oblated Stereographic
            template <typename Parameters>
            void setup_mil_os(Parameters& par, par_mod_ster& proj_parm)
            {
                static COMPLEX /* Miller Oblated Stereographic */
            AB[] = {
                {0.924500,    0.},
                {0.,            0.},
                {0.019430,    0.}
            };

                proj_parm.n = 2;
                par.lam0 = geometry::math::d2r<double>() * 20.;
                par.phi0 = geometry::math::d2r<double>() * 18.;
                proj_parm.zcoeff = AB;
                par.es = 0.;
                setup(par, proj_parm);
            }

            // Lee Oblated Stereographic
            template <typename Parameters>
            void setup_lee_os(Parameters& par, par_mod_ster& proj_parm)
            {
                static COMPLEX /* Lee Oblated Stereographic */
            AB[] = {
                {0.721316,    0.},
                {0.,            0.},
                    {-0.0088162,     -0.00617325}
            };

                proj_parm.n = 2;
                par.lam0 = geometry::math::d2r<double>() * -165.;
                par.phi0 = geometry::math::d2r<double>() * -10.;
                proj_parm.zcoeff = AB;
                par.es = 0.;
                setup(par, proj_parm);
            }

            // Mod. Stererographics of 48 U.S.
            template <typename Parameters>
            void setup_gs48(Parameters& par, par_mod_ster& proj_parm)
            {
                static COMPLEX /* 48 United States */
            AB[] = {
                {0.98879,    0.},
                {0.,        0.},
                {-0.050909,    0.},
                {0.,        0.},
                    {0.075528,    0.}
            };

                proj_parm.n = 4;
                par.lam0 = geometry::math::d2r<double>() * -96.;
                par.phi0 = geometry::math::d2r<double>() * -39.;
                proj_parm.zcoeff = AB;
                par.es = 0.;
                par.a = 6370997.;
                setup(par, proj_parm);
            }

            // Mod. Stererographics of Alaska
            template <typename Parameters>
            void setup_alsk(Parameters& par, par_mod_ster& proj_parm)
            {
                static COMPLEX
            ABe[] = { /* Alaska ellipsoid */
                {.9945303,    0.},
                {.0052083,    -.0027404},
                {.0072721,    .0048181},
                {-.0151089,    -.1932526},
                {.0642675,    -.1381226},
                {.3582802,    -.2884586}},
            ABs[] = { /* Alaska sphere */
                {.9972523,    0.},
                {.0052513,    -.0041175},
                {.0074606,    .0048125},
                {-.0153783,    -.1968253},
                {.0636871,    -.1408027},
                    {.3660976,    -.2937382}
            };

                proj_parm.n = 5;
                par.lam0 = geometry::math::d2r<double>() * -152.;
                par.phi0 = geometry::math::d2r<double>() * 64.;
                if (par.es) { /* fixed ellipsoid/sphere */
                    proj_parm.zcoeff = ABe;
                    par.a = 6378206.4;
                    par.e = sqrt(par.es = 0.00676866);
                } else {
                    proj_parm.zcoeff = ABs;
                    par.a = 6370997.;
                }
                setup(par, proj_parm);
            }

            // Mod. Stererographics of 50 U.S.
            template <typename Parameters>
            void setup_gs50(Parameters& par, par_mod_ster& proj_parm)
            {
                static COMPLEX
            ABe[] = { /* GS50 ellipsoid */
                {.9827497,    0.},
                {.0210669,    .0053804},
                {-.1031415,    -.0571664},
                {-.0323337,    -.0322847},
                {.0502303,    .1211983},
                {.0251805,    .0895678},
                {-.0012315,    -.1416121},
                {.0072202,    -.1317091},
                {-.0194029,    .0759677},
                    {-.0210072,    .0834037}
            },
            ABs[] = { /* GS50 sphere */
                {.9842990,    0.},
                {.0211642,    .0037608},
                {-.1036018,    -.0575102},
                {-.0329095,    -.0320119},
                {.0499471,    .1223335},
                {.0260460,    .0899805},
                {.0007388,    -.1435792},
                {.0075848,    -.1334108},
                {-.0216473,    .0776645},
                    {-.0225161,    .0853673}
            };

                proj_parm.n = 9;
                par.lam0 = geometry::math::d2r<double>() * -120.;
                par.phi0 = geometry::math::d2r<double>() * 45.;
                if (par.es) { /* fixed ellipsoid/sphere */
                    proj_parm.zcoeff = ABe;
                    par.a = 6378206.4;
                    par.e = sqrt(par.es = 0.00676866);
                } else {
                    proj_parm.zcoeff = ABs;
                    par.a = 6370997.;
                }
                setup(par, proj_parm);
            }

        } // namespace mod_ster

        /*!
            \brief Miller Oblated Stereographic projection
            \ingroup projections
            \tparam Geographic latlong point type
            \tparam Cartesian xy point type
            \tparam Parameters parameter type
            \par Projection characteristics
             - Azimuthal (mod)
            \par Example
            \image html ex_mil_os.gif
        */
        template <typename CalculationType, typename Parameters = parameters>
        struct mil_os_ellipsoid : public detail::mod_ster::base_mod_ster_ellipsoid<CalculationType, Parameters>
        {
            inline mil_os_ellipsoid(const Parameters& par) : detail::mod_ster::base_mod_ster_ellipsoid<CalculationType, Parameters>(par)
            {
                detail::mod_ster::setup_mil_os(this->m_par, this->m_proj_parm);
            }
        };

        /*!
            \brief Lee Oblated Stereographic projection
            \ingroup projections
            \tparam Geographic latlong point type
            \tparam Cartesian xy point type
            \tparam Parameters parameter type
            \par Projection characteristics
             - Azimuthal (mod)
            \par Example
            \image html ex_lee_os.gif
        */
        template <typename CalculationType, typename Parameters = parameters>
        struct lee_os_ellipsoid : public detail::mod_ster::base_mod_ster_ellipsoid<CalculationType, Parameters>
        {
            inline lee_os_ellipsoid(const Parameters& par) : detail::mod_ster::base_mod_ster_ellipsoid<CalculationType, Parameters>(par)
            {
                detail::mod_ster::setup_lee_os(this->m_par, this->m_proj_parm);
            }
        };

        /*!
            \brief Mod. Stererographics of 48 U.S. projection
            \ingroup projections
            \tparam Geographic latlong point type
            \tparam Cartesian xy point type
            \tparam Parameters parameter type
            \par Projection characteristics
             - Azimuthal (mod)
            \par Example
            \image html ex_gs48.gif
        */
        template <typename CalculationType, typename Parameters = parameters>
        struct gs48_ellipsoid : public detail::mod_ster::base_mod_ster_ellipsoid<CalculationType, Parameters>
        {
            inline gs48_ellipsoid(const Parameters& par) : detail::mod_ster::base_mod_ster_ellipsoid<CalculationType, Parameters>(par)
            {
                detail::mod_ster::setup_gs48(this->m_par, this->m_proj_parm);
            }
        };

        /*!
            \brief Mod. Stererographics of Alaska projection
            \ingroup projections
            \tparam Geographic latlong point type
            \tparam Cartesian xy point type
            \tparam Parameters parameter type
            \par Projection characteristics
             - Azimuthal (mod)
            \par Example
            \image html ex_alsk.gif
        */
        template <typename CalculationType, typename Parameters = parameters>
        struct alsk_ellipsoid : public detail::mod_ster::base_mod_ster_ellipsoid<CalculationType, Parameters>
        {
            inline alsk_ellipsoid(const Parameters& par) : detail::mod_ster::base_mod_ster_ellipsoid<CalculationType, Parameters>(par)
            {
                detail::mod_ster::setup_alsk(this->m_par, this->m_proj_parm);
            }
        };

        /*!
            \brief Mod. Stererographics of 50 U.S. projection
            \ingroup projections
            \tparam Geographic latlong point type
            \tparam Cartesian xy point type
            \tparam Parameters parameter type
            \par Projection characteristics
             - Azimuthal (mod)
            \par Example
            \image html ex_gs50.gif
        */
        template <typename CalculationType, typename Parameters = parameters>
        struct gs50_ellipsoid : public detail::mod_ster::base_mod_ster_ellipsoid<CalculationType, Parameters>
        {
            inline gs50_ellipsoid(const Parameters& par) : detail::mod_ster::base_mod_ster_ellipsoid<CalculationType, Parameters>(par)
            {
                detail::mod_ster::setup_gs50(this->m_par, this->m_proj_parm);
            }
        };

        // Static projection
        BOOST_GEOMETRY_PROJECTIONS_DETAIL_STATIC_PROJECTION(projections::mil_os, mil_os_ellipsoid, mil_os_ellipsoid)
        BOOST_GEOMETRY_PROJECTIONS_DETAIL_STATIC_PROJECTION(projections::lee_os, lee_os_ellipsoid, lee_os_ellipsoid)
        BOOST_GEOMETRY_PROJECTIONS_DETAIL_STATIC_PROJECTION(projections::gs48, gs48_ellipsoid, gs48_ellipsoid)
        BOOST_GEOMETRY_PROJECTIONS_DETAIL_STATIC_PROJECTION(projections::alsk, alsk_ellipsoid, alsk_ellipsoid)
        BOOST_GEOMETRY_PROJECTIONS_DETAIL_STATIC_PROJECTION(projections::gs50, gs50_ellipsoid, gs50_ellipsoid)

        // Factory entry(s)
        template <typename CalculationType, typename Parameters>
        class mil_os_entry : public detail::factory_entry<CalculationType, Parameters>
        {
            public :
                virtual base_v<CalculationType, Parameters>* create_new(const Parameters& par) const
                {
                    return new base_v_fi<mil_os_ellipsoid<CalculationType, Parameters>, CalculationType, Parameters>(par);
                }
        };

        template <typename CalculationType, typename Parameters>
        class lee_os_entry : public detail::factory_entry<CalculationType, Parameters>
        {
            public :
                virtual base_v<CalculationType, Parameters>* create_new(const Parameters& par) const
                {
                    return new base_v_fi<lee_os_ellipsoid<CalculationType, Parameters>, CalculationType, Parameters>(par);
                }
        };

        template <typename CalculationType, typename Parameters>
        class gs48_entry : public detail::factory_entry<CalculationType, Parameters>
        {
            public :
                virtual base_v<CalculationType, Parameters>* create_new(const Parameters& par) const
                {
                    return new base_v_fi<gs48_ellipsoid<CalculationType, Parameters>, CalculationType, Parameters>(par);
                }
        };

        template <typename CalculationType, typename Parameters>
        class alsk_entry : public detail::factory_entry<CalculationType, Parameters>
        {
            public :
                virtual base_v<CalculationType, Parameters>* create_new(const Parameters& par) const
                {
                    return new base_v_fi<alsk_ellipsoid<CalculationType, Parameters>, CalculationType, Parameters>(par);
                }
        };

        template <typename CalculationType, typename Parameters>
        class gs50_entry : public detail::factory_entry<CalculationType, Parameters>
        {
            public :
                virtual base_v<CalculationType, Parameters>* create_new(const Parameters& par) const
                {
                    return new base_v_fi<gs50_ellipsoid<CalculationType, Parameters>, CalculationType, Parameters>(par);
                }
        };

        template <typename CalculationType, typename Parameters>
        inline void mod_ster_init(detail::base_factory<CalculationType, Parameters>& factory)
        {
            factory.add_to_factory("mil_os", new mil_os_entry<CalculationType, Parameters>);
            factory.add_to_factory("lee_os", new lee_os_entry<CalculationType, Parameters>);
            factory.add_to_factory("gs48", new gs48_entry<CalculationType, Parameters>);
            factory.add_to_factory("alsk", new alsk_entry<CalculationType, Parameters>);
            factory.add_to_factory("gs50", new gs50_entry<CalculationType, Parameters>);
        }

    } // namespace detail
    #endif // doxygen

}}} // namespace boost::geometry::projections

#endif // BOOST_GEOMETRY_PROJECTIONS_MOD_STER_HPP

